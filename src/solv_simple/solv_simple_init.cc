#include "sbsat.h"
#include "sbsat_solver.h"
#include "solver.h"

//The main data structure holding information about the current state used
// by the brancher.
ProblemState *SimpleSmurfProblemState;

//The True smurf state
SmurfStateEntry *pTrueSimpleSmurfState;

//Variables for runtime data
double fSimpleSolverStartTime;
double fSimpleSolverEndTime;
double fSimpleSolverPrevEndTime;

//Just a dummy variable for holding temp memory allocations
int *tempint;
int tempint_max = 0;

//Arrays to handle state-type specific operations.
int *arrStatesTypeSize = NULL;
SetVisitedState *arrSetVisitedState;
ApplyInferenceToState *arrApplyInferenceToState = NULL;
PrintStateEntry *arrPrintStateEntry;
PrintStateEntry_dot *arrPrintStateEntry_dot;
FreeStateEntry *arrFreeStateEntry;
CalculateStateHeuristic *arrCalculateStateHeuristic;
SetStateHeuristicScore *arrSetStateHeuristicScore;
GetStateHeuristicScore *arrGetStateHeuristicScore;

//This allocates a new block of smurfs states and attaches them to the previous block
//The blocks are connected by a linked list - accessed through ->pNext
void allocate_new_SmurfStatesTable(int size) {
	size = SMURF_TABLE_SIZE;
	SimpleSmurfProblemState->arrCurrSmurfStates->pNext = (SmurfStatesTableStruct *)ite_calloc(1, sizeof(SmurfStatesTableStruct), 9, "SimpleSmurfProblemState->arrCurrSmurfStates->pNext");
	SimpleSmurfProblemState->arrCurrSmurfStates->pNext->arrStatesTable = (void **)ite_calloc(size, 1, 9, "SimpleSmurfProblemState->arrCurrSmurfStates->pNext->arrStatesTable");
	SimpleSmurfProblemState->arrCurrSmurfStates->pNext->curr_size = 0;
	SimpleSmurfProblemState->arrCurrSmurfStates->pNext->max_size = size;
	SimpleSmurfProblemState->arrCurrSmurfStates->pNext->pNext = NULL;
}

void FreeSmurfStatesTable() {
	SmurfStatesTableStruct *pTemp;
	for(SmurfStatesTableStruct *pIter = SimpleSmurfProblemState->arrSmurfStatesTableHead; pIter != NULL;) {
		pTemp = pIter;
		pIter = pIter->pNext;
		ite_free((void **)&pTemp->arrStatesTable);
		ite_free((void **)&pTemp);
	}
	
	SimpleSmurfProblemState->arrSmurfStatesTableHead = NULL;
	SimpleSmurfProblemState->arrCurrSmurfStates = NULL;
	SimpleSmurfProblemState->pSmurfStatesTableTail = NULL;
}

void ClearAllStatesVisitedFlag() {
	for(SmurfStatesTableStruct *pIter = SimpleSmurfProblemState->arrSmurfStatesTableHead; pIter != NULL;) {
      int x=0;
      while(1) {
         int size = arrStatesTypeSize[(unsigned char)((TypeStateEntry *)(((char *)pIter->arrStatesTable) + x))->cType];
         if(((TypeStateEntry *)(((char *)pIter->arrStatesTable) + x))->cType!=FN_FREE_STATE && ((TypeStateEntry *)(((char *)pIter->arrStatesTable) + x))->visited == 1) {
            //Clear out this smurf
            ((TypeStateEntry *)(((char *)pIter->arrStatesTable) + x))->visited = 0;
         }
         x+=size;
         if(x>=SimpleSmurfProblemState->arrCurrSmurfStates->max_size) break;
      }
		pIter = pIter->pNext;
	}
}

void GarbageCollectSmurfStatesTable(int force) {
   //This may have to be better in the future.

   if(force == 0 && bdd_gc_test() == 0) return;

   d4_printf2("SMURF_GC START (free %d) ", 0); //SEAN!!! Some indicator for progress?
   
   pTrueSimpleSmurfState->visited = 1;
 
   for(int nSmurfStackIdx = 0; nSmurfStackIdx <= SimpleSmurfProblemState->nCurrSearchTreeLevel; nSmurfStackIdx++) {
      for(int nSmurfIndex = 0; nSmurfIndex < SimpleSmurfProblemState->nNumSmurfs; nSmurfIndex++) {
         TypeStateEntry *pState = (TypeStateEntry *)SimpleSmurfProblemState->arrSmurfStack[nSmurfStackIdx].arrSmurfStates[nSmurfIndex];
         while(pState != NULL) {
            arrSetVisitedState[(int)pState->cType](pState, 1); //Set visited flag to 1
            pState = (TypeStateEntry *)pState->pPreviousState;
         }
      }
   }

   //Free up extra state machine memory
	for(SmurfStatesTableStruct *pIter = SimpleSmurfProblemState->arrSmurfStatesTableHead; pIter != NULL; pIter = pIter->pNext) {
      int x=0;
      while(1) {
         int size = arrStatesTypeSize[(unsigned char)((TypeStateEntry *)(((char *)pIter->arrStatesTable) + x))->cType];//arrStatesTypeSize[(int)pIter->arrStatesTable[x]];
         if(((TypeStateEntry *)(((char *)pIter->arrStatesTable) + x))->cType!=FN_FREE_STATE && ((TypeStateEntry *)(((char *)pIter->arrStatesTable) + x))->visited == 0) {
            //Free extra memory from this smurf, also clear the BDD -> SMURF pointer.
            arrFreeStateEntry[(unsigned char)((TypeStateEntry *)(((char *)pIter->arrStatesTable) + x))->cType](((char *)pIter->arrStatesTable) + x);
         }
         x+=size;
         if(x>=SimpleSmurfProblemState->arrCurrSmurfStates->max_size) break;
      }
	}
   
   bdd_gc(1); //Force a BDD garbage collect
   
   //Clear the states table
	for(SmurfStatesTableStruct *pIter = SimpleSmurfProblemState->arrSmurfStatesTableHead; pIter != NULL; pIter = pIter->pNext) {
      int x=0;
      while(1) {
         int size = arrStatesTypeSize[(unsigned char)((TypeStateEntry *)(((char *)pIter->arrStatesTable) + x))->cType];//arrStatesTypeSize[(int)pIter->arrStatesTable[x]];
         if(((TypeStateEntry *)(((char *)pIter->arrStatesTable) + x))->cType!=FN_FREE_STATE && ((TypeStateEntry *)(((char *)pIter->arrStatesTable) + x))->visited == 0) {
            //Clear out this smurf
            memset(((char *)pIter->arrStatesTable) + x, FN_FREE_STATE, arrStatesTypeSize[(unsigned char)((TypeStateEntry *)(((char *)pIter->arrStatesTable) + x))->cType]);
         }
         x+=size;
         if(x>=SimpleSmurfProblemState->arrCurrSmurfStates->max_size) break;
      }
      pIter->curr_size = 0;
	}

   SimpleSmurfProblemState->arrCurrSmurfStates = SimpleSmurfProblemState->arrSmurfStatesTableHead;
	SimpleSmurfProblemState->pSmurfStatesTableTail = (void *)(pTrueSimpleSmurfState + 1);
   SimpleSmurfProblemState->arrSmurfStatesTableHead->curr_size = sizeof(SmurfStateEntry); //For pTrueSimpleSmurfState
   
   ClearAllStatesVisitedFlag();
}

//void CompressSmurfStatesTable() { ??

//This checks the size of the current block of the smurf states table.
//If the table is full, we will allocate a new block via allocate_new_SmurfStatesTable(int size)
void check_SmurfStatesTableSize(int size) {
   assert(size >= (int)sizeof(TypeStateEntry));
   d9_printf2("enter %d\n", size);
   while(1) {
      d9_printf4("%d %d %p\n", SimpleSmurfProblemState->arrCurrSmurfStates->curr_size,
              arrStatesTypeSize[(unsigned char)((TypeStateEntry *)SimpleSmurfProblemState->pSmurfStatesTableTail)->cType],
              SimpleSmurfProblemState->pSmurfStatesTableTail);
      if(SimpleSmurfProblemState->arrCurrSmurfStates->curr_size+size >= SimpleSmurfProblemState->arrCurrSmurfStates->max_size) {
         //d9_printf2("Increasing size %p\n", SimpleSmurfProblemState->arrCurrSmurfStates);
         if(SimpleSmurfProblemState->arrCurrSmurfStates->pNext == NULL) {
            allocate_new_SmurfStatesTable(size);
            SimpleSmurfProblemState->arrCurrSmurfStates = SimpleSmurfProblemState->arrCurrSmurfStates->pNext;
            SimpleSmurfProblemState->pSmurfStatesTableTail = SimpleSmurfProblemState->arrCurrSmurfStates->arrStatesTable;
            if(SimpleSmurfProblemState->arrCurrSmurfStates->curr_size+size > SimpleSmurfProblemState->arrCurrSmurfStates->max_size) {
               fprintf(stderr, "Increase SMURF_TABLE_SIZE to larger than %d", size);
               exit(0);
            }
         } else {
            SimpleSmurfProblemState->arrCurrSmurfStates = SimpleSmurfProblemState->arrCurrSmurfStates->pNext;
            SimpleSmurfProblemState->pSmurfStatesTableTail = SimpleSmurfProblemState->arrCurrSmurfStates->arrStatesTable;
         }
      } else if (((TypeStateEntry *)SimpleSmurfProblemState->pSmurfStatesTableTail)->cType == FN_FREE_STATE) {
         int all_clear = 1;
         char *pTempSSTT = (char *)SimpleSmurfProblemState->pSmurfStatesTableTail;
         int blanks_size = 0;
         for(int x = 1; x < size; x++) {
            pTempSSTT += 1;
            blanks_size++;
            if(((TypeStateEntry *)pTempSSTT)->cType != FN_FREE_STATE) {
               blanks_size += arrStatesTypeSize[(unsigned char)((TypeStateEntry *)pTempSSTT)->cType];
               all_clear = 0;
               break;
            }
         }

         if(all_clear==1) {
            SimpleSmurfProblemState->arrCurrSmurfStates->curr_size+=size;
            break;
         }
         
         SimpleSmurfProblemState->arrCurrSmurfStates->curr_size+=blanks_size;
         SimpleSmurfProblemState->pSmurfStatesTableTail = (void *)(((char *)SimpleSmurfProblemState->pSmurfStatesTableTail) + blanks_size);
      } else {
         assert(arrStatesTypeSize[(unsigned char)((TypeStateEntry *)SimpleSmurfProblemState->pSmurfStatesTableTail)->cType] > 1);
         SimpleSmurfProblemState->arrCurrSmurfStates->curr_size+=
           arrStatesTypeSize[(unsigned char)((TypeStateEntry *)SimpleSmurfProblemState->pSmurfStatesTableTail)->cType];
         SimpleSmurfProblemState->pSmurfStatesTableTail = (void *)(((char *)SimpleSmurfProblemState->pSmurfStatesTableTail) +
           arrStatesTypeSize[(unsigned char)((TypeStateEntry *)SimpleSmurfProblemState->pSmurfStatesTableTail)->cType]);
      }
   }
}

//This function increases the size of the smurf stack.
//The smurf stack stores all the state specific information
//  relevant to the search at a specific decision level
void Alloc_SmurfStack(int destination) {
	//fprintf(stdout, "increasing stack size\n");
	for(int i = destination; i < destination + SMURF_STATES_INCREASE_SIZE && i < SimpleSmurfProblemState->nNumVars; i++) {		  
		SimpleSmurfProblemState->arrSmurfStack[i].arrSmurfStates
		  = (void **)ite_calloc(SimpleSmurfProblemState->nNumSmurfs, sizeof(void *), 9, "arrSmurfStates");
		//fprintf(stderr, "alloc %d\n", i);
		Alloc_SmurfStack_Hooks(i);
	}
}

void FreeSmurfStack() {
	if(use_SmurfWatchedLists) ite_free((void **)&SimpleSmurfProblemState->arrSmurfStack[0].arrSmurfStates);
	else {
		for(int i = 0; i < SimpleSmurfProblemState->nNumVars; i++)
		  if(SimpleSmurfProblemState->arrSmurfStack[i].arrSmurfStates != NULL) {
           ite_free((void **)&SimpleSmurfProblemState->arrSmurfStack[i].arrSmurfStates);
           Free_SmurfStack_Hooks(i);
        }
	}
	ite_free((void **)&SimpleSmurfProblemState->arrSmurfStack);
}

void Init_SimpleSmurfProblemState() {
   arrStatesTypeSize = (int *)ite_calloc(NUM_SMURF_TYPES, sizeof(int), 9, "arrStatesTypeSize");
   arrStatesTypeSize[FN_FREE_STATE] = sizeof(char);
   arrStatesTypeSize[FN_TYPE_STATE] = sizeof(TypeStateEntry);

   arrSetVisitedState = (SetVisitedState *)ite_calloc(NUM_SMURF_TYPES, sizeof(SetVisitedState), 9, "arrSetVisitedState");
   arrApplyInferenceToState = (ApplyInferenceToState *)ite_calloc(NUM_SMURF_TYPES, sizeof(ApplyInferenceToState), 9, "arrApplyInferenceToState");
   arrPrintStateEntry = (PrintStateEntry *)ite_calloc(NUM_SMURF_TYPES, sizeof(PrintStateEntry), 9, "arrPrintStateEntry");
   arrPrintStateEntry_dot = (PrintStateEntry_dot *)ite_calloc(NUM_SMURF_TYPES, sizeof(PrintStateEntry_dot), 9, "arrPrintStateEntry_dot");
   arrFreeStateEntry = (FreeStateEntry *)ite_calloc(NUM_SMURF_TYPES, sizeof(FreeStateEntry), 9, "arrFreeStateEntry");
   arrCalculateStateHeuristic = (CalculateStateHeuristic *)ite_calloc(NUM_SMURF_TYPES, sizeof(CalculateStateHeuristic), 9, "arrCalculateStateHeuristic");
	arrSetStateHeuristicScore = (SetStateHeuristicScore *)ite_calloc(NUM_SMURF_TYPES, sizeof(SetStateHeuristicScore), 9, "arrSetStateHeuristicScore");
   arrGetStateHeuristicScore = (GetStateHeuristicScore *)ite_calloc(NUM_SMURF_TYPES, sizeof(GetStateHeuristicScore), 9, "arrGetStateHeuristicScore");

	initSmurfStateType();
	initORStateType();
	initORCounterStateType();
	initXORStateType();
	initXORCounterStateType();
	initXORGElimStateType();
	initMINMAXStateType();
	initMINMAXCounterStateType();
	initNEGMINMAXStateType();
	initNEGMINMAXCounterStateType();
	initInferenceStateType();
   
   //Create the pTrueSimpleSmurfState entry
	
   SimpleSmurfProblemState = (ProblemState *)ite_calloc(1, sizeof(ProblemState), 9, "SimpleSmurfProblemState");
	SimpleSmurfProblemState->nNumSmurfStateEntries = 2;
	int size = SMURF_TABLE_SIZE;
   SimpleSmurfProblemState->arrSmurfStatesTableHead = (SmurfStatesTableStruct *)ite_calloc(1, sizeof(SmurfStatesTableStruct), 9, "SimpleSmurfProblemState->arrSmurfStatesTableHead");
	SimpleSmurfProblemState->arrSmurfStatesTableHead->arrStatesTable = (void **)ite_calloc(size, 1, 9, "SimpleSmurfProblemState->arrSmurfStatesTableHead->arrStatesTable");
	SimpleSmurfProblemState->arrSmurfStatesTableHead->curr_size = sizeof(SmurfStateEntry); //For pTrueSimpleSmurfState
	SimpleSmurfProblemState->arrSmurfStatesTableHead->max_size = size;
	SimpleSmurfProblemState->arrSmurfStatesTableHead->pNext = NULL;
	SimpleSmurfProblemState->arrCurrSmurfStates = SimpleSmurfProblemState->arrSmurfStatesTableHead;
   
	//arrSmurfStatesTable[0] is reserved for the pTrueSimpleSmurfState
	pTrueSimpleSmurfState = (SmurfStateEntry *)SimpleSmurfProblemState->arrCurrSmurfStates->arrStatesTable;
	pTrueSimpleSmurfState->cType = FN_SMURF;
	pTrueSimpleSmurfState->nTransitionVar = 0;
	pTrueSimpleSmurfState->pVarIsTrueTransition = (void *)pTrueSimpleSmurfState;
	pTrueSimpleSmurfState->pVarIsFalseTransition = (void *)pTrueSimpleSmurfState;
	pTrueSimpleSmurfState->fHeurWghtofTrueTransition = 0;
	pTrueSimpleSmurfState->fHeurWghtofFalseTransition = 0;
	//pTrueSimpleSmurfState->bVarIsSafe = 0;
	pTrueSimpleSmurfState->pNextVarInThisStateGT = NULL;
	pTrueSimpleSmurfState->pNextVarInThisStateLT = NULL;
	pTrueSimpleSmurfState->pNextVarInThisState = NULL;
	
	SimpleSmurfProblemState->pSmurfStatesTableTail = (void *)(pTrueSimpleSmurfState + 1);
	
	clear_all_bdd_pState();
	true_ptr->pState = pTrueSimpleSmurfState;
}

//This function initializes a lot of memory, e.g. creating the smurfs from the BDDs.
int ReadAllSmurfsIntoTable(int nNumVars) {
	Init_SimpleSmurfProblemState();
	SimpleSmurfProblemState->nNumSmurfs = nmbrFunctions;
	SimpleSmurfProblemState->nNumVars = nNumVars;
	
	SimpleSmurfProblemState->arrSmurfStack = (SmurfStack *)ite_calloc(SimpleSmurfProblemState->nNumVars, sizeof(SmurfStack), 9, "arrSmurfStack");
	SimpleSmurfProblemState->arrVariableOccursInSmurf = (int **)ite_calloc(SimpleSmurfProblemState->nNumVars, sizeof(int *), 9, "arrVariablesOccursInSmurf");
	SimpleSmurfProblemState->arrReverseOccurenceList = (int_p **)ite_calloc(SimpleSmurfProblemState->nNumSmurfs, sizeof(int_p *), 9, "arrReverseOccurenceList");
	SimpleSmurfProblemState->arrPosVarHeurWghts = (double *)ite_calloc(SimpleSmurfProblemState->nNumVars, sizeof(double), 9, "arrPosVarHeurWghts");
	SimpleSmurfProblemState->arrNegVarHeurWghts = (double *)ite_calloc(SimpleSmurfProblemState->nNumVars, sizeof(double), 9, "arrNegVarHeurWghts");
	SimpleSmurfProblemState->arrInferenceQueue = (int *)ite_calloc(SimpleSmurfProblemState->nNumVars, sizeof(int), 9, "arrInferenceQueue");
	SimpleSmurfProblemState->arrInferenceDeclaredAtLevel = (int *)ite_calloc(SimpleSmurfProblemState->nNumVars, sizeof(int), 9, "arrInferenceDeclaredAtLevel");

	SimpleSmurfProblemState->nCurrSearchTreeLevel = 0;
	
	Init_Solver_PreSmurfs_Hooks();
	
	//Count the number of functions every variable occurs in.
	int *temp_varcount = (int *)ite_calloc(SimpleSmurfProblemState->nNumVars, sizeof(int), 9, "temp_varcount");
	for(int x = 0; x < SimpleSmurfProblemState->nNumSmurfs; x++) {
		int nNumElts = 0;
		unravelBDD(&nNumElts, &tempint_max, &tempint, functions[x]);
		for (int i=0;i<nNumElts;i++) {
			if (tempint[i]==0 ||
				 arrIte2SimpleSolverVarMap[tempint[i]]==0) {
				dE_printf1("\nassigned variable in a BDD in the solver");
				dE_printf3("\nvariable id: %d, true_false=%d\n",
							  tempint[i],
							  variablelist[tempint[i]].true_false);
				//exit(1);
			}
			tempint[i] = arrIte2SimpleSolverVarMap[tempint[i]];
		}
//		qsort(tempint, nNumElts, sizeof(int), revcompfunc);

		for(int y = 0; y < nNumElts; y++)
		  temp_varcount[tempint[y]]++;
		SimpleSmurfProblemState->arrReverseOccurenceList[x] = (int_p *)ite_calloc(nNumElts+1, sizeof(int_p), 9, "arrReverseOccurenceList[x]");
		SimpleSmurfProblemState->arrReverseOccurenceList[x][0].var = nNumElts;
	}

	Alloc_SmurfStack(0); //Alloc some of the Smurf Stack
	
	for(int x = 0; x < SimpleSmurfProblemState->nNumVars; x++) {
		SimpleSmurfProblemState->arrVariableOccursInSmurf[x] = (int *)ite_calloc(temp_varcount[x]+1, sizeof(int), 9, "arrVariableOccursInSmurf[x]");
		SimpleSmurfProblemState->arrVariableOccursInSmurf[x][0] = temp_varcount[x];
		SimpleSmurfProblemState->arrInferenceDeclaredAtLevel[x] = SimpleSmurfProblemState->nNumVars;
		temp_varcount[x] = 1;
	}

	//Fill in the variable occurance arrays for each function
	for(int x = 0; x < SimpleSmurfProblemState->nNumSmurfs; x++) {
		int nNumElts = 0;
		unravelBDD(&nNumElts, &tempint_max, &tempint, functions[x]);
		for (int i=0;i<nNumElts;i++) {
			if (tempint[i]==0 ||
				 arrIte2SimpleSolverVarMap[tempint[i]]==0) {
				dE_printf1("\nassigned variable in a BDD in the solver");
				dE_printf3("\nvariable id: %d, true_false=%d\n",
							  tempint[i],
							  variablelist[tempint[i]].true_false);
				//exit(1);
			}
			tempint[i] = arrIte2SimpleSolverVarMap[tempint[i]];
		}
		qsort(tempint, nNumElts, sizeof(int), revcompfunc); //Necessary
		
		for(int y = 0; y < nNumElts; y++) {
			int nVar = tempint[y];
			SimpleSmurfProblemState->arrVariableOccursInSmurf[nVar][temp_varcount[nVar]] = x;
			SimpleSmurfProblemState->arrReverseOccurenceList[x][y+1].var = nVar;
			SimpleSmurfProblemState->arrReverseOccurenceList[x][y+1].loc = &(SimpleSmurfProblemState->arrVariableOccursInSmurf[nVar][temp_varcount[nVar]]);
//			fprintf(stderr, "|%d %d|", x, *SimpleSmurfProblemState->arrReverseOccurenceList[x][y+1]);
//			SimpleSmurfProblemState->arrVariableOccursInSmurf[nVar][temp_varcount[nVar]] = 7;
//			fprintf(stderr, "|%d %d|", x, *SimpleSmurfProblemState->arrReverseOccurenceList[x][y+1]);
//			(*SimpleSmurfProblemState->arrReverseOccurenceList[x][y+1])--;
//			fprintf(stderr, "|%d %d|", x, *SimpleSmurfProblemState->arrReverseOccurenceList[x][y+1]);
//			(*SimpleSmurfProblemState->arrReverseOccurenceList[x][y+1])=1;
//			fprintf(stderr, "|%d %d|", x, *SimpleSmurfProblemState->arrReverseOccurenceList[x][y+1]);

			if(use_SmurfWatchedLists && y > (numSmurfWatchedVars==0?nNumElts:numSmurfWatchedVars-1)) {
//				fprintf(stderr, "/%d ", SimpleSmurfProblemState->arrVariableOccursInSmurf[nVar][temp_varcount[nVar]]);
				SimpleSmurfProblemState->arrVariableOccursInSmurf[nVar][temp_varcount[nVar]] = -SimpleSmurfProblemState->arrVariableOccursInSmurf[nVar][temp_varcount[nVar]];
//				fprintf(stderr, "%d\\", SimpleSmurfProblemState->arrVariableOccursInSmurf[nVar][temp_varcount[nVar]]);
			}
			
			temp_varcount[nVar]++;
		}
	}

	ite_free((void **)&temp_varcount);
	
	//Create the rest of the SmurfState entries
	char p[256]; int str_length=0;
	D_3(
		 d3_printf1("Building Smurfs: ");
		 sprintf(p, "{0/%d}",  SimpleSmurfProblemState->nNumSmurfs);
		 str_length = (int)strlen(p);
		 d3_printf1(p);
		 );
	for(int nSmurfIndex = 0; nSmurfIndex < SimpleSmurfProblemState->nNumSmurfs; nSmurfIndex++) {
		D_3(
			 if (nSmurfIndex % ((SimpleSmurfProblemState->nNumSmurfs/100)+1) == 0) {
				 for(int iter = 0; iter<str_length; iter++)
					d3_printf1("\b");
				 sprintf(p, "{%d/%d}", nSmurfIndex, SimpleSmurfProblemState->nNumSmurfs);
				 str_length = (int)strlen(p);
				 d3_printf1(p);
			 }
			 );
		BDDNode *pInitialBDD = functions[nSmurfIndex];
		if(nSmurfIndex > 0 && pInitialBDD->pState != NULL && smurfs_share_states &&
         SimpleSmurfProblemState->arrSmurfStack[0].arrSmurfStates[((TypeStateEntry *)pInitialBDD->pState)->pStateOwner] == pInitialBDD->pState) { //Duplicate Smurf
         //This really shouldn't happen because the BDD preprocessor should have already removed all duplicates.
         d7_printf2("Removing duplicate Smurf #%d\n", nSmurfIndex);
			SimpleSmurfProblemState->arrSmurfStack[0].arrSmurfStates[nSmurfIndex] = pTrueSimpleSmurfState;
		} else {
			if(!smurfs_share_states) {
				clear_all_bdd_pState(); 
				true_ptr->pState = pTrueSimpleSmurfState;
				if(precompute_smurfs == 1) bdd_gc(); //MUST NOT garbage collect BDDs when building smurfs on the fly.
			}
         SimpleSmurfProblemState->arrSmurfStack[0].arrSmurfStates[nSmurfIndex] =	ReadSmurfStateIntoTable(pInitialBDD, NULL, 0);
			//Setting the ownership of this Smurf
			d7_printf3("Setting owner of Smurf #%d to %d\n", nSmurfIndex, nSmurfIndex);
			assert(((TypeStateEntry *)SimpleSmurfProblemState->arrSmurfStack[0].arrSmurfStates[nSmurfIndex])->pPreviousState == NULL);
			((TypeStateEntry *)SimpleSmurfProblemState->arrSmurfStack[0].arrSmurfStates[nSmurfIndex])->pStateOwner = nSmurfIndex;
			if(SimpleSmurfProblemState->arrSmurfStack[0].arrSmurfStates[nSmurfIndex] == pTrueSimpleSmurfState)
			  SimpleSmurfProblemState->arrSmurfStack[0].nNumSmurfsSatisfied++;
		}
		Init_Solver_MidSmurfs_Hooks(nSmurfIndex, SimpleSmurfProblemState->arrSmurfStack[0].arrSmurfStates);
	}

   D_3(
		 for(int iter = 0; iter<str_length; iter++)
		 d3_printf1("\b");
		 sprintf(p, "{%d/%d}\n", SimpleSmurfProblemState->nNumSmurfs, SimpleSmurfProblemState->nNumSmurfs);
		 str_length = (int)strlen(p);
		 d3_printf1(p);
		 d3_printf2("%d SmurfStates Used\n", SimpleSmurfProblemState->nNumSmurfStateEntries);  
		 );

	D_9(PrintAllSmurfStateEntries(););

	return Init_Solver_PostSmurfs_Hooks(SimpleSmurfProblemState->arrSmurfStack[0].arrSmurfStates);
}

void FreeSmurfSolverVars() {
	for(int i = 0; i < SimpleSmurfProblemState->nNumSmurfs; i++)
	  if(SimpleSmurfProblemState->arrReverseOccurenceList[i]!=NULL)
		 ite_free((void **)&SimpleSmurfProblemState->arrReverseOccurenceList[i]);
	ite_free((void **)&SimpleSmurfProblemState->arrReverseOccurenceList);
		  
	for(int i = 0; i < SimpleSmurfProblemState->nNumVars; i++)
	  if(SimpleSmurfProblemState->arrVariableOccursInSmurf[i]!=NULL)
		 ite_free((void **)&SimpleSmurfProblemState->arrVariableOccursInSmurf[i]);
	ite_free((void **)&SimpleSmurfProblemState->arrVariableOccursInSmurf);
	
	ite_free((void **)&SimpleSmurfProblemState->arrPosVarHeurWghts);
	ite_free((void **)&SimpleSmurfProblemState->arrNegVarHeurWghts);
	ite_free((void **)&SimpleSmurfProblemState->arrInferenceQueue);
	ite_free((void **)&SimpleSmurfProblemState->arrInferenceDeclaredAtLevel);

	LSGBORFree();
	LSGBXORFree();
   LSGBMINMAXFree();
   LSGBNEGMINMAXFree();
}

void FreeSmurfStateEntries() {
	for(SmurfStatesTableStruct *pIter = SimpleSmurfProblemState->arrSmurfStatesTableHead; pIter != NULL;) {
      int x=0;
      while(1) {
         int size = arrStatesTypeSize[(unsigned char)((TypeStateEntry *)(((char *)pIter->arrStatesTable) + x))->cType];
         if(((TypeStateEntry *)(((char *)pIter->arrStatesTable) + x))->cType!=FN_FREE_STATE && ((TypeStateEntry *)(((char *)pIter->arrStatesTable) + x))->visited == 1) {
            //Free this state
            arrFreeStateEntry[(unsigned char)((TypeStateEntry *)(((char *)pIter->arrStatesTable) + x))->cType](((char *)pIter->arrStatesTable) + x);
         }
         x+=size;
         if(x>=SimpleSmurfProblemState->arrCurrSmurfStates->max_size) break;
      }
		pIter = pIter->pNext;
	}
}

int use_poor_mans_vsids = 0;
int *arrPMVSIDS = NULL;
//This initializes a few things, then calls the main initialization
//  function - ReadAllSmurfsIntoTable(nNumVars);
int Init_SimpleSmurfSolver() {
	//Clear all FunctionTypes
	for(int x = 0; x < nmbrFunctions; x++)
	  functionType[x] = UNSURE;

	simple_solver_reset_level = solver_reset_level-1;
	
	int nNumVars = InitSimpleVarMap();

	if(*csv_trace_file)
	  fd_csv_trace_file = fopen(csv_trace_file, "w");

	//Set up heuristic
	switch (sHeuristic[0]) {
	 case 'n': Simple_Solver_Heuristic = Simple_DC_Heuristic; break;
	 case 'j': Simple_Solver_Heuristic = Simple_LSGB_Heuristic; break;
	 case 'p': if(use_lemmas == 0) {
		          dE_printf1("Error: 'lemmas' must be enabled to use the PicoSAT heuristic\n");
                exit(0);
	           } else Simple_Solver_Heuristic = picosat_decide_for_SBSAT; break;
    case 'v': Simple_Solver_Heuristic = Simple_PMVSIDS_Heuristic; 
		        arrPMVSIDS = (int *)ite_calloc(nNumVars, sizeof(int), 9, "arrPMVSIDS");
		        use_poor_mans_vsids=1;
		        break;
	 default:
		 dE_printf2("Error: Unknown heuristic type %c\n", sHeuristic[0]);
		exit(0);
		break;
	}

	//Set up restarts
	switch (sRestartHeuristic[0]) {
	 case '0': use_RapidRestarts = 0; break;
	 case 'm': use_RapidRestarts = 1;
		        Simple_initRestart = MiniSAT_initRestart;
		        Simple_nextRestart = MiniSAT_nextRestart; break;
	 case 'p': use_RapidRestarts = 1;
		        Simple_initRestart = PicoSAT_initRestart;
		        Simple_nextRestart = PicoSAT_nextRestart; break;
	 case 'l': use_RapidRestarts = 1;
		        Simple_initRestart = Luby_initRestart;
		        Simple_nextRestart = Luby_nextRestart; break;
	 default:
		 dE_printf2("Error: Unknown restart heuristic type %c\n", sRestartHeuristic[0]);
		exit(0);
		break;
	}

	if(use_RapidRestarts) {
		Simple_initRestart();
		nNextRestart = Simple_nextRestart();
	}
	
	//Compute SMURFS	
	int ret = ReadAllSmurfsIntoTable(nNumVars);
   
	return ret;
}

//Here we do the clean up after the brancher is done - freeing memory etc.
void Final_SimpleSmurfSolver() {

	DisplaySimpleStatistics(ite_counters[NUM_CHOICE_POINTS], ite_counters[NUM_BACKTRACKS], ite_counters[NUM_BACKJUMPS]);
	if (fd_csv_trace_file) {
		DisplaySimpleSolverBacktrackInfo_gnuplot();
		fclose(fd_csv_trace_file);
	}

	Final_Solver_Hooks();

	FreeSimpleVarMap();

	FreeSmurfStateEntries();
	FreeSmurfStatesTable();

	FreeSmurfStack();
	FreeSmurfSolverVars();

	ite_free((void **)&arrPMVSIDS);

   ite_free((void **)&arrStatesTypeSize);
   
	ite_free((void **)&SimpleSmurfProblemState);
	
	ite_free((void **)&tempint);
	tempint_max = 0;
	
}

// Functions for printing (via dot) the corresponding SMURFS for sets of BDDs.

void FreePrintSmurfs() {
	FreeSmurfStateEntries();
	FreeSmurfStatesTable();
	ite_free((void **)&SimpleSmurfProblemState);
}

void PrintSmurfs(BDDNode **bdds, int size) {
	
   int temp_numinp = 0;
	for (int i = 0; i < size; i++) {
		if (bdds[i]->variable > temp_numinp)
		  temp_numinp = bdds[i]->variable;
	}	
	
	arrSimpleSolver2IteVarMap = (int *)ite_calloc(temp_numinp+1, sizeof(int), 9, "solver mapping(s2i)");
	arrIte2SimpleSolverVarMap = (int *)ite_calloc(temp_numinp+1, sizeof(int), 9, "solver mapping(i2s)");

	for(int i = 0; i <= temp_numinp; i++)
		arrSimpleSolver2IteVarMap[i] = arrIte2SimpleSolverVarMap[i] = i;
	
	Init_SimpleSmurfProblemState();

	//Need to start up the variable map
	
	for(int i = 0; i < size; i++) {
		if(bdds[i]->pState != NULL || bdds[i] == false_ptr){
			continue;
		} else {
			fprintf(stdout, "digraph Smurf {\n");
			fprintf(stdout, " graph [concentrate=true, nodesep=\"0.30\", ordering=in, rankdir=TB, ranksep=\"2.25\"];\n");
			fprintf(stdout, " b%p [shape=box fontname=""Helvetica"",label=""T""];\n", (void *)pTrueSimpleSmurfState);
			PrintSmurf_dot(ReadSmurfStateIntoTable(bdds[i], NULL, 0));
         fprintf(stdout, "}\n");
		}
	}
	
	FreeSimpleVarMap();
	FreePrintSmurfs();

	ite_free((void **)&arrSimpleSolver2IteVarMap);
	ite_free((void **)&arrIte2SimpleSolverVarMap);
}
