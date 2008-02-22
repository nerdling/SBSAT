#include "sbsat.h"
#include "sbsat_solver.h"
#include "solver.h"

/*********************************************************************
struct ProblemState{
	// Static
	int nNumSmurfs;
	int nNumVars;
	int nNumSmurfStateEntries;
   SmurfStatesTableStruct *arrSmurfStatesTableHead; //Pointer to the table of all smurf states
   SmurfStatesTableStruct *arrCurrSmurfStates;      //Pointer to the current table of smurf states
   void *pSmurfStatesTableTail;                     //Pointer to the next open block of the arrSmurfStatesTable
	int **arrVariableOccursInSmurf; //Pointer to lists of Smurfs, indexed by variable number, that contain that variable.
	                                //Max size would be nNumSmurfs * nNumVars, but this would only happen if every
               	                 //Smurf contained every variable. Each list is terminated by a -1 element.
	// Dynamic
	int nCurrSearchTreeLevel;
   double *arrPosVarHeurWghts;       //Pointer to array of size nNumVars
	double *arrNegVarHeurWghts;       //Pointer to array of size nNumVars
	int *arrInferenceQueue;           //Pointer to array of size nNumVars (dynamically indexed by arrSmurfStack[level].nNumFreeVars
   int *arrInferenceDeclaredAtLevel; //Pointer to array of size nNumVars
	SmurfStack *arrSmurfStack;        //Pointer to array of size nNumVars
};
***********************************************************************/

int smurfs_share_paths=1;

ProblemState *SimpleSmurfProblemState;
SmurfStateEntry *pTrueSimpleSmurfState;

double fSimpleSolverStartTime;
double fSimpleSolverEndTime;
double fSimpleSolverPrevEndTime;

int *tempint;
long tempint_max = 0;

void allocate_new_SmurfStatesTable(int size) {
	size = SMURF_TABLE_SIZE;
	SimpleSmurfProblemState->arrCurrSmurfStates->pNext = (SmurfStatesTableStruct *)ite_calloc(1, sizeof(SmurfStatesTableStruct), 9, "SimpleSmurfProblemState->arrCurrSmurfStates->pNext");
	SimpleSmurfProblemState->arrCurrSmurfStates->pNext->arrStatesTable = (void **)ite_calloc(size, 1, 9, "SimpleSmurfProblemState->arrCurrSmurfStates->pNext->arrStatesTable");
	SimpleSmurfProblemState->arrCurrSmurfStates->pNext->curr_size = 0;
	SimpleSmurfProblemState->arrCurrSmurfStates->pNext->max_size = size;
	SimpleSmurfProblemState->arrCurrSmurfStates->pNext->pNext = NULL;
}

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

void Alloc_SmurfStack(int destination) {
	//fprintf(stdout, "increasing stack size\n");
	for(int i = destination; i < destination + SMURF_STATES_INCREASE_SIZE && i < SimpleSmurfProblemState->nNumVars; i++) {		  
		SimpleSmurfProblemState->arrSmurfStack[i].arrSmurfStates
		  = (void **)ite_calloc(SimpleSmurfProblemState->nNumSmurfs, sizeof(int *), 9, "arrSmurfStates");
		//fprintf(stderr, "alloc %d\n", i);
	}
	Alloc_SmurfStack_Hooks(destination);
}

void ReadAllSmurfsIntoTable(int nNumVars) {
	//Create the pTrueSimpleSmurfState entry
	SimpleSmurfProblemState = (ProblemState *)ite_calloc(1, sizeof(ProblemState), 9, "SimpleSmurfProblemState");
	SimpleSmurfProblemState->nNumSmurfs = nmbrFunctions;
	SimpleSmurfProblemState->nNumVars = nNumVars;
	SimpleSmurfProblemState->nNumSmurfStateEntries = 2;
	//ite_counters[SMURF_STATES] = DetermineNumOfSmurfStates();
	int size = SMURF_TABLE_SIZE;
	SimpleSmurfProblemState->arrSmurfStatesTableHead = (SmurfStatesTableStruct *)ite_calloc(1, sizeof(SmurfStatesTableStruct), 9, "SimpleSmurfProblemState->arrSmurfStatesTableHead");
	SimpleSmurfProblemState->arrSmurfStatesTableHead->arrStatesTable = (void **)ite_calloc(size, 1, 9, "SimpleSmurfProblemState->arrSmurfStatesTableHead->arrStatesTable");
	SimpleSmurfProblemState->arrSmurfStatesTableHead->curr_size = sizeof(SmurfStateEntry);
	SimpleSmurfProblemState->arrSmurfStatesTableHead->max_size = size;
	SimpleSmurfProblemState->arrSmurfStatesTableHead->pNext = NULL;
	SimpleSmurfProblemState->arrCurrSmurfStates = SimpleSmurfProblemState->arrSmurfStatesTableHead;
	SimpleSmurfProblemState->arrSmurfStack = (SmurfStack *)ite_calloc(SimpleSmurfProblemState->nNumVars, sizeof(SmurfStack), 9, "arrSmurfStack");
	SimpleSmurfProblemState->arrVariableOccursInSmurf = (int **)ite_calloc(SimpleSmurfProblemState->nNumVars, sizeof(int *), 9, "arrVariablesOccursInSmurf");
	SimpleSmurfProblemState->arrPosVarHeurWghts = (double *)ite_calloc(SimpleSmurfProblemState->nNumVars, sizeof(double), 9, "arrPosVarHeurWghts");
	SimpleSmurfProblemState->arrNegVarHeurWghts = (double *)ite_calloc(SimpleSmurfProblemState->nNumVars, sizeof(double), 9, "arrNegVarHeurWghts");
	SimpleSmurfProblemState->arrInferenceQueue = (int *)ite_calloc(SimpleSmurfProblemState->nNumVars, sizeof(int), 9, "arrInferenceQueue");
	SimpleSmurfProblemState->arrInferenceDeclaredAtLevel = (int *)ite_calloc(SimpleSmurfProblemState->nNumVars, sizeof(int), 9, "arrInferenceDeclaredAtLevel");
	
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
		SimpleSmurfProblemState->arrVariableOccursInSmurf[x] = (int *)ite_calloc(temp_varcount[x]+1, sizeof(int *), 9, "arrVariableOccursInSmurf[x]");
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
	
	//arrSmurfStatesTable[0] is reserved for the pTrueSimpleSmurfState
	pTrueSimpleSmurfState = (SmurfStateEntry *)SimpleSmurfProblemState->arrCurrSmurfStates->arrStatesTable;
	pTrueSimpleSmurfState->nTransitionVar = 0;
	pTrueSimpleSmurfState->pVarIsTrueTransition = (void *)pTrueSimpleSmurfState;
	pTrueSimpleSmurfState->pVarIsFalseTransition = (void *)pTrueSimpleSmurfState;
	pTrueSimpleSmurfState->fHeurWghtofTrueTransition = 0;
	pTrueSimpleSmurfState->fHeurWghtofFalseTransition = 0;
	pTrueSimpleSmurfState->bVarIsSafe = 0;
	pTrueSimpleSmurfState->pNextVarInThisStateGT = NULL;
	pTrueSimpleSmurfState->pNextVarInThisStateLT = NULL;
	pTrueSimpleSmurfState->pNextVarInThisState = NULL;
	
	SimpleSmurfProblemState->pSmurfStatesTableTail = (void *)(pTrueSimpleSmurfState + 1);
	
	clear_all_bdd_pState();
	true_ptr->pState = pTrueSimpleSmurfState;
	
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
			if(!smurfs_share_paths) { clear_all_bdd_pState(); true_ptr->pState = pTrueSimpleSmurfState; }
			SimpleSmurfProblemState->arrSmurfStack[0].arrSmurfStates[nSmurfIndex] =	ReadSmurfStateIntoTable(pInitialBDD);
			bdd_gc();
		}
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
}

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
	
	Init_Solver_PreSmurfs_Hooks();
	
	ReadAllSmurfsIntoTable(nNumVars);

	ret = Init_Solver_PostSmurfs_Hooks();
	
	return ret;
}

void Final_SimpleSmurfSolver() {

	DisplaySimpleStatistics(ite_counters[NUM_CHOICE_POINTS], ite_counters[NUM_BACKTRACKS], ite_counters[NUM_BACKJUMPS]);
	if (fd_csv_trace_file) {
		DisplaySimpleSolverBacktrackInfo_gnuplot(fSimpleSolverPrevEndTime, fSimpleSolverStartTime);
		fclose(fd_csv_trace_file);
	}
	//Still need to do some backend stuff like free memory.

	Final_Solver_Hooks();
	
	FreeSimpleVarMap();

	//Hmmm I didn't free A LOT of that memory. SEAN!!! FIX THIS!!!
}

int simpleSolve() {
	int nForceBackjumpLevel_old = nForceBackjumpLevel;
	if(nForceBackjumpLevel < 0) nForceBackjumpLevel = nVarChoiceLevelsNum+1;
	
	int ret = Init_SimpleSmurfSolver();
	if(ret != SOLV_UNKNOWN) return ret;

	ret = SimpleBrancher();
	
	nForceBackjumpLevel = nForceBackjumpLevel_old;

	Final_SimpleSmurfSolver();
	
	return ret;
}
