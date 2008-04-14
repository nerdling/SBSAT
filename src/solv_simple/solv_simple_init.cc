#include "sbsat.h"
#include "sbsat_solver.h"
#include "solver.h"

//A flag that determines whether or not smurf states are shared between smurfs.
int smurfs_share_paths=1;

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
long tempint_max = 0;

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

//This checks the size of the current block of the smurf states table.
//If the table is full, we will allocate a new block via allocate_new_SmurfStatesTable(int size)
void check_SmurfStatesTableSize(int size) {
	SimpleSmurfProblemState->arrCurrSmurfStates->curr_size+=size;
	if(SimpleSmurfProblemState->arrCurrSmurfStates->curr_size >= SimpleSmurfProblemState->arrCurrSmurfStates->max_size) {
		SimpleSmurfProblemState->arrCurrSmurfStates->curr_size-=size;
		//fprintf(stderr, "Increasing size %x\n", SimpleSmurfProblemState->arrCurrSmurfStates);
		allocate_new_SmurfStatesTable(size);
		SimpleSmurfProblemState->arrCurrSmurfStates = SimpleSmurfProblemState->arrCurrSmurfStates->pNext;
		SimpleSmurfProblemState->pSmurfStatesTableTail = SimpleSmurfProblemState->arrCurrSmurfStates->arrStatesTable;
		SimpleSmurfProblemState->arrCurrSmurfStates->curr_size+=size;
		if(SimpleSmurfProblemState->arrCurrSmurfStates->curr_size > SimpleSmurfProblemState->arrCurrSmurfStates->max_size) {
			fprintf(stderr, "Increase SMURF_TABLE_SIZE to larger than %d", size);
			exit(0);
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
	for(int i = 0; i < SimpleSmurfProblemState->nNumVars; i++)
	  if(SimpleSmurfProblemState->arrSmurfStack[i].arrSmurfStates != NULL)
		 ite_free((void **)&SimpleSmurfProblemState->arrSmurfStack[i].arrSmurfStates);
	ite_free((void **)&SimpleSmurfProblemState->arrSmurfStack);
}

//This function initializes a lot of memory, e.g. creating the smurfs from the BDDs.
void Init_SimpleSmurfProblemState() {
	//Create the pTrueSimpleSmurfState entry
	SimpleSmurfProblemState = (ProblemState *)ite_calloc(1, sizeof(ProblemState), 9, "SimpleSmurfProblemState");
	SimpleSmurfProblemState->nNumSmurfStateEntries = 2;
	int size = SMURF_TABLE_SIZE;
	SimpleSmurfProblemState->arrSmurfStatesTableHead = (SmurfStatesTableStruct *)ite_calloc(1, sizeof(SmurfStatesTableStruct), 9, "SimpleSmurfProblemState->arrSmurfStatesTableHead");
	SimpleSmurfProblemState->arrSmurfStatesTableHead->arrStatesTable = (void **)ite_calloc(size, 1, 9, "SimpleSmurfProblemState->arrSmurfStatesTableHead->arrStatesTable");
	SimpleSmurfProblemState->arrSmurfStatesTableHead->curr_size = sizeof(SmurfStateEntry);
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
	SimpleSmurfProblemState->arrPosVarHeurWghts = (double *)ite_calloc(SimpleSmurfProblemState->nNumVars, sizeof(double), 9, "arrPosVarHeurWghts");
	SimpleSmurfProblemState->arrNegVarHeurWghts = (double *)ite_calloc(SimpleSmurfProblemState->nNumVars, sizeof(double), 9, "arrNegVarHeurWghts");
	SimpleSmurfProblemState->arrInferenceQueue = (int *)ite_calloc(SimpleSmurfProblemState->nNumVars, sizeof(int), 9, "arrInferenceQueue");
	SimpleSmurfProblemState->arrInferenceDeclaredAtLevel = (int *)ite_calloc(SimpleSmurfProblemState->nNumVars, sizeof(int), 9, "arrInferenceDeclaredAtLevel");

	Init_Solver_PreSmurfs_Hooks();
	
	//Count the number of functions every variable occurs in.
	int *temp_varcount = (int *)ite_calloc(SimpleSmurfProblemState->nNumVars, sizeof(int), 9, "temp_varcount");
	for(int x = 0; x < SimpleSmurfProblemState->nNumSmurfs; x++) {
		long nNumElts = 0;
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
		
	}

	Alloc_SmurfStack(0); //Alloc some of the Smurf Stack
	
	for(int x = 0; x < SimpleSmurfProblemState->nNumVars; x++) {
		SimpleSmurfProblemState->arrVariableOccursInSmurf[x] = (int *)ite_calloc(temp_varcount[x]+1, sizeof(int), 9, "arrVariableOccursInSmurf[x]");
		SimpleSmurfProblemState->arrVariableOccursInSmurf[x][temp_varcount[x]] = -1;
		SimpleSmurfProblemState->arrInferenceDeclaredAtLevel[x] = SimpleSmurfProblemState->nNumVars;
		temp_varcount[x] = 0;
	}
	
	//Fill in the variable occurance arrays for each function
	for(int x = 0; x < SimpleSmurfProblemState->nNumSmurfs; x++) {
		long nNumElts = 0;
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
		
		for(int y = 0; y < nNumElts; y++) {
			int nVar = tempint[y];
			SimpleSmurfProblemState->arrVariableOccursInSmurf[nVar][temp_varcount[nVar]] = x;
			temp_varcount[nVar]++;
		}
	}
	
	ite_free((void **)&temp_varcount);
	
	//Create the rest of the SmurfState entries
	char p[256]; int str_length;
	D_3(
		 d3_printf1("Building Smurfs: ");
		 sprintf(p, "{0/%d}",  SimpleSmurfProblemState->nNumSmurfs);
		 str_length = strlen(p);
		 d3_printf1(p);
		 );
	for(int nSmurfIndex = 0; nSmurfIndex < SimpleSmurfProblemState->nNumSmurfs; nSmurfIndex++) {
		D_3(
			 if (nSmurfIndex % ((SimpleSmurfProblemState->nNumSmurfs/100)+1) == 0) {
				 for(int iter = 0; iter<str_length; iter++)
					d3_printf1("\b");
				 sprintf(p, "{%d/%d}", nSmurfIndex, SimpleSmurfProblemState->nNumSmurfs);
				 str_length = strlen(p);
				 d3_printf1(p);
			 }
			 );
		BDDNode *pInitialBDD = functions[nSmurfIndex];
		if(pInitialBDD->pState != NULL && smurfs_share_paths) {
			SimpleSmurfProblemState->arrSmurfStack[0].arrSmurfStates[nSmurfIndex] = pInitialBDD->pState;
		} else {
			//LSGBSmurfSetHeurScores(nSmurfIndex, pInitialState);
			if(!smurfs_share_paths) { clear_all_bdd_pState(); true_ptr->pState = pTrueSimpleSmurfState; 	bdd_gc();}
			SimpleSmurfProblemState->arrSmurfStack[0].arrSmurfStates[nSmurfIndex] =	ReadSmurfStateIntoTable(pInitialBDD);
		}
		Init_Solver_MidSmurfs_Hooks(nSmurfIndex, SimpleSmurfProblemState->arrSmurfStack[0].arrSmurfStates);
	}
	D_3(
		 for(int iter = 0; iter<str_length; iter++)
		 d3_printf1("\b");
		 sprintf(p, "{%d/%d}\n", SimpleSmurfProblemState->nNumSmurfs, SimpleSmurfProblemState->nNumSmurfs);
		 str_length = strlen(p);
		 d3_printf1(p);
		 d3_printf2("%d SmurfStates Used\n", SimpleSmurfProblemState->nNumSmurfStateEntries);  
		 );

	D_9(PrintAllSmurfStateEntries(););
	
	return Init_Solver_PostSmurfs_Hooks(SimpleSmurfProblemState->arrSmurfStack[0].arrSmurfStates);
}

void FreeSmurfSolverVars() {
	for(int i = 0; i < SimpleSmurfProblemState->nNumVars; i++)
	  if(SimpleSmurfProblemState->arrVariableOccursInSmurf[i]!=NULL)
		 ite_free((void **)&SimpleSmurfProblemState->arrVariableOccursInSmurf[i]);
	ite_free((void **)&SimpleSmurfProblemState->arrVariableOccursInSmurf);
	
	ite_free((void **)&SimpleSmurfProblemState->arrPosVarHeurWghts);
	ite_free((void **)&SimpleSmurfProblemState->arrNegVarHeurWghts);
	ite_free((void **)&SimpleSmurfProblemState->arrInferenceQueue);
	ite_free((void **)&SimpleSmurfProblemState->arrInferenceDeclaredAtLevel);
}

void FreeSmurfStateEntries() {
	SmurfStatesTableStruct *arrSmurfStatesTable = SimpleSmurfProblemState->arrSmurfStatesTableHead;
	int state_num = 0;
	while(arrSmurfStatesTable != NULL) {
		void *arrSmurfStates = arrSmurfStatesTable->arrStatesTable;
		int size = 0;
		for(int x = 0; x < arrSmurfStatesTable->curr_size; x+=size)	{
			state_num++;
			//d9_printf3("State %d(%x), ", state_num, arrSmurfStates);
			if(((TypeStateEntry *)arrSmurfStates)->cType == FN_SMURF) {
				FreeSmurfStateEntry((SmurfStateEntry *)arrSmurfStates);
				arrSmurfStates = (void *)(((SmurfStateEntry *)arrSmurfStates) + 1);
				size = sizeof(SmurfStateEntry);
			} else if(((TypeStateEntry *)arrSmurfStates)->cType == FN_INFERENCE) {							  
				FreeInferenceStateEntry((InferenceStateEntry *)arrSmurfStates);
				arrSmurfStates = (void *)(((InferenceStateEntry *)arrSmurfStates) + 1);
				size = sizeof(InferenceStateEntry);
			} else if(((TypeStateEntry *)arrSmurfStates)->cType == FN_OR) {							  
				FreeORStateEntry((ORStateEntry *)arrSmurfStates);
				arrSmurfStates = (void *)(((ORStateEntry *)arrSmurfStates) + 1);
				size = sizeof(ORStateEntry);
			} else if(((TypeStateEntry *)arrSmurfStates)->cType == FN_OR_COUNTER) {
				FreeORCounterStateEntry((ORCounterStateEntry *)arrSmurfStates);
				arrSmurfStates = (void *)(((ORCounterStateEntry *)arrSmurfStates) + 1);
				size = sizeof(ORCounterStateEntry);
			} else if(((TypeStateEntry *)arrSmurfStates)->cType == FN_XOR) {
				FreeXORStateEntry((XORStateEntry *)arrSmurfStates);
				arrSmurfStates = (void *)(((XORStateEntry *)arrSmurfStates) + 1);
				size = sizeof(XORStateEntry);
			} else if(((TypeStateEntry *)arrSmurfStates)->cType == FN_XOR_COUNTER) {
				FreeXORCounterStateEntry((XORCounterStateEntry *)arrSmurfStates);
				arrSmurfStates = (void *)(((XORCounterStateEntry *)arrSmurfStates) + 1);
				size = sizeof(XORCounterStateEntry);
			} else if(((TypeStateEntry *)arrSmurfStates)->cType == FN_XOR_GELIM) {
				FreeXORGElimStateEntry((XORGElimStateEntry *)arrSmurfStates);
				arrSmurfStates = (void *)(((XORGElimStateEntry *)arrSmurfStates) + 1);
				size = sizeof(XORGElimStateEntry);
			}
		}
		arrSmurfStatesTable = arrSmurfStatesTable->pNext;
	}
}

//This initializes a few things, then calls the main initialization
//  function - ReadAllSmurfsIntoTable(nNumVars);
int Init_SimpleSmurfSolver() {
	//Clear all FunctionTypes
	for(int x = 0; x < nmbrFunctions; x++)
	  functionType[x] = UNSURE;

	simple_solver_reset_level = solver_reset_level-1;
	
	int nNumVars = InitSimpleVarMap();
	/* Convert bdds to smurfs */
	int ret = SOLV_UNKNOWN;// = CreateFunctions();
	if (ret != SOLV_UNKNOWN) return ret;

	if(*csv_trace_file)
	  fd_csv_trace_file = fopen(csv_trace_file, "w");
	
	ret = ReadAllSmurfsIntoTable(nNumVars);

	return ret;
}

//Here we do the clean up after the brancher is done - freeing memory etc.
void Final_SimpleSmurfSolver() {

	DisplaySimpleStatistics(ite_counters[NUM_CHOICE_POINTS], ite_counters[NUM_BACKTRACKS], ite_counters[NUM_BACKJUMPS]);
	if (fd_csv_trace_file) {
		DisplaySimpleSolverBacktrackInfo_gnuplot(fSimpleSolverPrevEndTime, fSimpleSolverStartTime);
		fclose(fd_csv_trace_file);
	}
	//Still need to do some backend stuff like free memory.

	Final_Solver_Hooks();

	FreeSimpleVarMap();

	FreeSmurfStateEntries();
	FreeSmurfStatesTable();

	FreeSmurfStack();
	FreeSmurfSolverVars();

	ite_free((void **)&SimpleSmurfProblemState);
	
	ite_free((void **)&tempint);
	tempint_max = 0;
	
	//Hmmm I didn't free A LOT of that memory. SEAN!!! FIX THIS!!!
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
			fprintf(stdout, " b%x [shape=box fontname=""Helvetica"",fontsize=""16"",label=""T""];\n", pTrueSimpleSmurfState);
			PrintSmurf_dot(ReadSmurfStateIntoTable(bdds[i]));
			fprintf(stdout, "}\n");
		}
	}
	
	FreeSimpleVarMap();
	FreePrintSmurfs();

	ite_free((void **)&arrSimpleSolver2IteVarMap);
	ite_free((void **)&arrIte2SimpleSolverVarMap);
}
