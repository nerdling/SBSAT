#include "sbsat.h"
#include "sbsat_solver.h"
#include "solver.h"

/********** Included in include/sbsat_solver.h *********************

struct TypeStateEntry {
   char cType;
}

 struct SmurfStateEntry{
   char cType; //FN_SMURF
   int nTransitionVar;
	void *pVarIsTrueTransition;
	void *pVarIsFalseTransition;
	double fHeurWghtofTrueTransition;
	double fHeurWghtofFalseTransition;
	//This is 1 if nTransitionVar should be inferred True,
	//       -1 if nTransitionVar should be inferred False,
	//        0 if nTransitionVar should not be inferred.
	void *pNextVarInThisStateGT; //There are n SmurfStateEntries linked together
 	void *pNextVarInThisStateLT; //in the structure of a heap,
	                             //where n is the number of variables in this SmurfStateEntry.
                          	     //All of these SmurfStateEntries represent the same function,
                                //but a different variable (nTransitionVar) is
                                //highlighted for each link in the heap.
                                //If this is 0, we have reached a leaf node.
	void *pNextVarInThisState;   //Same as above except linked linearly, instead of a heap.
                                //Used for computing the heuristic of a state.
 };

struct InferenceStateEntry {
   char cType; //FN_INFERENCE
   int nTransitionVar;
   void *pVarTransition;
   bool bPolarity
 };

struct ORStateEntry {
   char cType; //FN_OR
   int *nTransitionVars;
   bool *bPolarity;
   int nSize;
};

struct ORCounterStateEntry {
   char cType; //FN_OR_COUNTER
   void *pTransition;
   int nSize;
   ORStateEntry *pORState;
};

struct XORStateEntry {
   char cType; //FN_OR
   int *nTransitionVars;
   bool parity;
   int nSize;
};

struct XORCounterStateEntry {
   char cType; //FN_OR_COUNTER
   void *pTransition;
   int nSize;
   XORStateEntry *pXORState; //For heuristic purposes
};

struct SmurfStack{
	int nNumFreeVars;
   int nHeuristicPlaceholder;
   int nVarChoiceCurrLevel;   //Index to array of size nNumVars
   void **arrSmurfStates;     //Pointer to array of size nNumSmurfs
};

struct SmurfStatesTableStruct {
   int curr_size;
   int max_size;
   void **arrStatesTable; //Pointer to a table of smurf states.
   SmurfStatesTableStruct *pNext;
};

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

SmurfStateEntry *pTrueSimpleSmurfState;

//#define HEUR_MULT 10000
#define SMURF_TABLE_SIZE 1000000

ProblemState *SimpleSmurfProblemState;

double fSimpleSolverStartTime;
double fSimpleSolverEndTime;
double fSimpleSolverPrevEndTime;

int add_one_display=0;

int smurfs_share_paths=1;

int nSimpleSolver_Reset=0;
int nInfQueueStart=0;
int solver_polarity_presets_count=0;
int simple_solver_reset_level=-1;

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

void PrintSmurfStateEntry(SmurfStateEntry *ssEntry) {
	d9_printf9("Var=%d, v=T:%x, v=F:%x, TWght=%4.6f, FWght=%4.6f, NextGT=%x, NextLT=%x, Next=%x\n", ssEntry->nTransitionVar, ssEntry->pVarIsTrueTransition, ssEntry->pVarIsFalseTransition, ssEntry->fHeurWghtofTrueTransition, ssEntry->fHeurWghtofFalseTransition, ssEntry->pNextVarInThisStateGT, ssEntry->pNextVarInThisStateLT, ssEntry->pNextVarInThisState);
}

void PrintInferenceStateEntry(InferenceStateEntry *ssEntry) {
	d9_printf4("Var=%d, Inf=%x, Polarity=%d\n", ssEntry->nTransitionVar, ssEntry->pVarTransition, ssEntry->bPolarity);
}

void PrintORStateEntry(ORStateEntry *ssEntry) {
	d9_printf1("Vars=");
	for(int x = 0; x < ssEntry->nSize; x++)
	  d9_printf2("%d ", ssEntry->bPolarity[x]==1?ssEntry->nTransitionVars[x]:-ssEntry->nTransitionVars[x]);
	d9_printf2("Size=%d\n", ssEntry->nSize);
}

void PrintORCounterStateEntry(ORCounterStateEntry *ssEntry) {
	d9_printf4("Next=%x Head=%x Size=%d\n", ssEntry->pTransition, ssEntry->pORState, ssEntry->nSize);
}

void PrintAllSmurfStateEntries() {
	SmurfStatesTableStruct *arrSmurfStatesTable = SimpleSmurfProblemState->arrSmurfStatesTableHead;
	int state_num = 0;
	while(arrSmurfStatesTable != NULL) {
		void *arrSmurfStates = arrSmurfStatesTable->arrStatesTable;		
		int size = 0;
		for(int x = 0; x < arrSmurfStatesTable->curr_size; x+=size) {
			state_num++;
			d9_printf3("State %d(%x), ", state_num, arrSmurfStates);
			if(((TypeStateEntry *)arrSmurfStates)->cType == FN_SMURF) {
				PrintSmurfStateEntry((SmurfStateEntry *)arrSmurfStates);
				arrSmurfStates = (void *)(((SmurfStateEntry *)arrSmurfStates) + 1);
				size = sizeof(SmurfStateEntry);
			} else if(((TypeStateEntry *)arrSmurfStates)->cType == FN_INFERENCE) {
				PrintInferenceStateEntry((InferenceStateEntry *)arrSmurfStates);
				arrSmurfStates = (void *)(((InferenceStateEntry *)arrSmurfStates) + 1);
				size = sizeof(InferenceStateEntry);
			} else if(((TypeStateEntry *)arrSmurfStates)->cType == FN_OR) {
				PrintORStateEntry((ORStateEntry *)arrSmurfStates);
				arrSmurfStates = (void *)(((ORStateEntry *)arrSmurfStates) + 1);
				size = sizeof(ORStateEntry);
			} else if(((TypeStateEntry *)arrSmurfStates)->cType == FN_OR_COUNTER) {
				PrintORCounterStateEntry((ORCounterStateEntry *)arrSmurfStates);
				arrSmurfStates = (void *)(((ORCounterStateEntry *)arrSmurfStates) + 1);
				size = sizeof(ORCounterStateEntry);
			}
	   }
		arrSmurfStatesTable = arrSmurfStatesTable->pNext;
	}
}

ITE_INLINE
void save_solution_simple(void) {
	d7_printf1("      Solution found\n");
	if (result_display_type) {
		/* create another node in solution chain */
		d7_printf1("      Recording solution\n");
		t_solution_info *tmp_solution_info;
		tmp_solution_info = (t_solution_info*)calloc(1, sizeof(t_solution_info));
		
		if (solution_info_head == NULL) {
			solution_info = tmp_solution_info;
			solution_info_head = solution_info;
		} else {
			solution_info->next = (struct _t_solution_info*)tmp_solution_info;
			solution_info = (t_solution_info*)(solution_info->next);
		}
		tmp_solution_info->nNumElts = numinp+1;//SimpleSmurfProblemState->nNumVars+1;
		tmp_solution_info->arrElts = new int[numinp+1];//new int[SimpleSmurfProblemState->nNumVars+2];
		
		for (int i = 0; i<SimpleSmurfProblemState->nNumVars; i++) {
			tmp_solution_info->arrElts[arrSolver2IteVarMap[abs(SimpleSmurfProblemState->arrInferenceQueue[i])]] = (SimpleSmurfProblemState->arrInferenceQueue[i]>0)?BOOL_TRUE:BOOL_FALSE;
		}
	}
}

ITE_INLINE
void CalculateSimpleSolverProgress(int *_whereAmI, int *_total) {
	int whereAmI=0;
	int total=0;
	int soft_count=14;
	int hard_count=28;
	int count=0;
	int nInfQueueTail = 0;
	
	if(SimpleSmurfProblemState->nCurrSearchTreeLevel >= 0)
	  nInfQueueTail = SimpleSmurfProblemState->arrSmurfStack[SimpleSmurfProblemState->nCurrSearchTreeLevel].nNumFreeVars;
	
	int nCurrSearchTreeLevel = 0;
	for(int i = nInfQueueStart; i < nInfQueueTail && (count<soft_count || (count < hard_count && whereAmI==0)); i++) {
		int nBranchLit = SimpleSmurfProblemState->arrInferenceQueue[i];
		if(SimpleSmurfProblemState->arrInferenceDeclaredAtLevel[abs(nBranchLit)] < 0 ||
		   (SimpleSmurfProblemState->arrInferenceDeclaredAtLevel[abs(nBranchLit)] == 0 && add_one_display==1)) {
			//This variable is an old choicepoint
			whereAmI *= 2;
			whereAmI += 1;
			total *= 2;
			total += 1;
			count++;
		} else if(nBranchLit == SimpleSmurfProblemState->arrInferenceQueue[SimpleSmurfProblemState->arrSmurfStack[nCurrSearchTreeLevel].nNumFreeVars]) {
			//This variable is a current choicepoint
			whereAmI *= 2;
			total *= 2;
			total += 1;
			nCurrSearchTreeLevel++;
			count++;
		}
	}
	
	*_whereAmI = whereAmI;
	*_total    = total;
}

ITE_INLINE
void DisplaySimpleSolverBacktrackInfo(double &fSimpleSolverPrevEndTime, double &fSimpleSolverStartTime) {
	double fSimpleSolverEndTime = ite_counters_f[CURRENT_TIME] = get_runtime();
	double fTotalDurationInSecs = fSimpleSolverEndTime - fSimpleSolverStartTime;
	double fDurationInSecs = fSimpleSolverEndTime - fSimpleSolverPrevEndTime;
	double fBacktracksPerSec = BACKTRACKS_PER_STAT_REPORT / (fDurationInSecs>0?fDurationInSecs:0.001);
	fSimpleSolverPrevEndTime = fSimpleSolverEndTime;
	
	d2_printf2("Time: %4.3fs. ", fTotalDurationInSecs);
	d2_printf3("Backtracks: %ld (%4.3f per sec) ",
				  (long)ite_counters[NUM_BACKTRACKS], fBacktracksPerSec);
	
	int whereAmI = 0;
	int total = 0;
	double progress = 0.0;
	CalculateSimpleSolverProgress(&whereAmI, &total);
	if (total == 0) total=1;
	progress = ite_counters_f[PROGRESS] = (float)whereAmI*100/total;
	//d2_printf3("Progress: %x/%x        ", whereAmI, total);
	d2_printf1("Progress: ");
	char number[10];
	char back[10] = "\b\b\b\b\b\b\b\b\b";
	sprintf(number, "% 3.2f%%", progress);
	//sprintf(number, " ?");
	back[strlen(number)]=0;
	if ((DEBUG_LVL&15) == 1) {
		fprintf(stderr, "%s%s", number, back);
	} else {
		D_1(
			 d0_printf3("%s%s", number, back);
			 fflush(stddbg);
		)
	}
	
	d2_printf1("\n Choices (total, dependent" );
	dC_printf4("c %4.3fs. Progress %s Choices: %lld\n", fTotalDurationInSecs, number, ite_counters[NO_ERROR]);
	if (backjumping) d2_printf1(", backjumped");
	d2_printf3("): (%lld, %lld", ite_counters[NUM_CHOICE_POINTS], ite_counters[HEU_DEP_VAR]);
	if (backjumping) d2_printf2(", %lld", ite_counters[NUM_TOTAL_BACKJUMPS]);
	d2_printf1(")");
	
	if (NO_LEMMAS == 0)
	  d2_printf6("\n Lemmas (0, 1, 2, non-cached, added): (%d, %d, %d, %d, %d)",
					 nNumCachedLemmas[0], nNumCachedLemmas[1], nNumCachedLemmas[2],
					 gnNumLemmas - (nNumCachedLemmas[0]+nNumCachedLemmas[1]+nNumCachedLemmas[2]),
					 nCallsToAddLemma
	  );
	
	d2_printf1("\n");
	d2_printf1(" Inferences by ");
	d2_printf2("smurfs: %lld; ", ite_counters[INF_SMURF]);
	d2_printf2("ANDs: %lld; ", ite_counters[INF_SPEC_FN_AND]);
	d2_printf2("XORs: %lld; ", ite_counters[INF_SPEC_FN_XOR]);
	d2_printf2("MINMAXs: %lld; ", ite_counters[INF_SPEC_FN_MINMAX]);
	if (NO_LEMMAS == 0) d2_printf2("lemmas: %lld; ", ite_counters[INF_LEMMA]);
	d2_printf1("\n");
	
	d2_printf1(" Backtracks by ");
	d2_printf2("smurfs: %lld; ", ite_counters[ERR_BT_SMURF]);
	d2_printf2("ANDs: %lld; ", ite_counters[ERR_BT_SPEC_FN_AND]);
	d2_printf2("XORs: %lld; ", ite_counters[ERR_BT_SPEC_FN_XOR]);
	d2_printf2("MINMAXs: %lld; ", ite_counters[ERR_BT_SPEC_FN_MINMAX]);
	if (NO_LEMMAS == 0) d2_printf2("lemmas: %lld; ", ite_counters[ERR_BT_LEMMA]);
	d2_printf1("\n");
	if (backjumping) d2_printf3(" Backjumps: %ld (avg bj len: %.1f)\n",
										 (long)ite_counters[NUM_TOTAL_BACKJUMPS],
										 (float)ite_counters[NUM_TOTAL_BACKJUMPS]/(1+ite_counters[NUM_BACKJUMPS]));
	if (autarky) d2_printf3(" Autarkies: %ld (avg au len: %.1f)\n",
									(long)ite_counters[NUM_TOTAL_AUTARKIES],
									(float)ite_counters[NUM_TOTAL_AUTARKIES]/(1+ite_counters[NUM_AUTARKIES]));
	if (max_solutions != 1) d2_printf3(" Solutions found: %ld/%ld\n",
												  (long)ite_counters[NUM_SOLUTIONS], (long)max_solutions);
	
	d2_printf1("\n");
}

ITE_INLINE
void DisplaySimpleSolverBacktrackInfo_gnuplot(double &fSimpleSolverPrevEndTime, double &fSimpleSolverStartTime) {
	double fSimpleSolverEndTime = ite_counters_f[CURRENT_TIME];
	double fTotalDurationInSecs = fSimpleSolverEndTime - fSimpleSolverStartTime;
	double fDurationInSecs = fSimpleSolverEndTime - fSimpleSolverPrevEndTime;
	double fBacktracksPerSec = BACKTRACKS_PER_STAT_REPORT / (fDurationInSecs>0?fDurationInSecs:0.001);
	
	fprintf(fd_csv_trace_file, "%4.3f ", fTotalDurationInSecs);
	fprintf(fd_csv_trace_file, "%ld %4.3f ",
				  (long)ite_counters[NUM_BACKTRACKS], fBacktracksPerSec);
	
	int whereAmI = 0;
	int total = 0;
	double progress = 0.0;
	CalculateSimpleSolverProgress(&whereAmI, &total);
	if (total == 0) total=1;
	progress = ite_counters_f[PROGRESS] = (float)whereAmI*100/total;
	fprintf(fd_csv_trace_file, "%4.3f ", progress);
	
	fprintf(fd_csv_trace_file, "%lld %lld ", ite_counters[NUM_CHOICE_POINTS], ite_counters[HEU_DEP_VAR]);
	fprintf(fd_csv_trace_file, "%lld ", ite_counters[INF_SMURF]);
	fprintf(fd_csv_trace_file, "%lld ", ite_counters[ERR_BT_SMURF]);
	fprintf(fd_csv_trace_file, "%ld %ld\n", (long)ite_counters[NUM_SOLUTIONS], (long)max_solutions);
}


//struct AndEqFalseWghtStruct *arrAndEqFalseWght = NULL;
HWEIGHT K = JHEURISTIC_K;

// We need nMaxRHSSize to be at least one to insure that entry 1 exists
// and we don't overrun the arrays.

extern int nMaxRHSSize;

ITE_INLINE void LSGBORStateSetHeurScores(ORStateEntry *pState) {
	int size = pState->nSize;

	if(size > nMaxRHSSize) {
		size+=20;
		if(nMaxRHSSize == 1) {
			arrAndEqFalseWght = (AndEqFalseWghtStruct*)ite_calloc(size+1, sizeof(AndEqFalseWghtStruct), 9, "arrAndEqFalseWght");
			arrAndEqFalseWght[2].fPos = JHEURISTIC_K_TRUE+JHEURISTIC_K_INF;
			arrAndEqFalseWght[2].fFmla = (arrAndEqFalseWght[2].fPos + JHEURISTIC_K_TRUE) / (2*K);
			nMaxRHSSize = 2;
		} else
		  arrAndEqFalseWght = (AndEqFalseWghtStruct*)ite_recalloc(arrAndEqFalseWght, nMaxRHSSize, size+1, sizeof(AndEqFalseWghtStruct), 9, "arrAndEqFalseWght");

		for (int i = nMaxRHSSize+1; i <= size; i++) {
			arrAndEqFalseWght[i].fPos = arrAndEqFalseWght[i - 1].fFmla;
			arrAndEqFalseWght[i].fFmla = (arrAndEqFalseWght[i].fPos + JHEURISTIC_K_TRUE) / (2*K);
		}
		nMaxRHSSize = size;
	}
}

ITE_INLINE void LSGBORCounterStateSetHeurScores(ORCounterStateEntry *pState) {
	LSGBORStateSetHeurScores(pState->pORState);
}

ITE_INLINE double LSGBSumNodeWeights(SmurfStateEntry *pState) {
	if (pState == pTrueSimpleSmurfState) return JHEURISTIC_K_TRUE;
	
	int num_variables = 0;
	double fTotalTransitions = 0.0;
	while(pState!=NULL) {
		if (pState->cType != FN_SMURF) return 0;
		num_variables++;
		/* ----- POSITIVE TRANSITION ------ */
		fTotalTransitions += pState->fHeurWghtofTrueTransition;
		
		/* ----- NEGATIVE TRANSITION ------ */
		fTotalTransitions += pState->fHeurWghtofFalseTransition;
		
		pState = (SmurfStateEntry *)pState->pNextVarInThisState;
	}
	return fTotalTransitions / (((double)num_variables) * 2.0 * JHEURISTIC_K);
}

ITE_INLINE double LSGBORGetHeurScore(ORStateEntry *pState) {
	return arrAndEqFalseWght[2].fFmla;
}

ITE_INLINE double LSGBORGetHeurPos(ORStateEntry *pState) {
	return arrAndEqFalseWght[2].fPos;
}

ITE_INLINE double LSGBORCounterGetHeurScore(ORCounterStateEntry *pState) {
	return arrAndEqFalseWght[pState->nSize].fFmla;
}

ITE_INLINE double LSGBORCounterGetHeurPos(ORCounterStateEntry *pState) {
	return arrAndEqFalseWght[pState->nSize].fPos;
}

ITE_INLINE double LSGBGetHeurScoreTransition(SmurfStateEntry *pState, int nPolarity) {
	int num_inferences = 0;
	//pState = (SmurfStateEntry *) (nPolarity>0?pState->pVarIsTrueTransition:pState->pVarIsFalseTransition);
	InferenceStateEntry *pInference = (InferenceStateEntry *) (nPolarity>0?pState->pVarIsTrueTransition:pState->pVarIsFalseTransition);
	while(pInference->cType == FN_INFERENCE) {
		num_inferences++;
		pInference = (InferenceStateEntry *) pInference->pVarTransition;
	}
	double fInferenceWeights = JHEURISTIC_K_INF * num_inferences;

	if (pInference->cType == FN_SMURF) {
		return fInferenceWeights + LSGBSumNodeWeights((SmurfStateEntry *)pInference);
	} else if (pInference->cType == FN_OR) {
		return fInferenceWeights + LSGBORGetHeurScore((ORStateEntry *)pInference);
	} else if (pInference->cType == FN_OR_COUNTER) {
		return fInferenceWeights + LSGBORCounterGetHeurScore((ORCounterStateEntry *)pInference);
	}
	return 0;
}

ITE_INLINE void
LSGBSmurfSetHeurScores(SmurfStateEntry *pState) {
	if (pState == pTrueSimpleSmurfState) return;
	
	while(pState!=NULL) {
		if (pState->cType != FN_SMURF) return;
		/* ----- POSITIVE TRANSITION ------ */
		pState->fHeurWghtofTrueTransition = LSGBGetHeurScoreTransition(pState, BOOL_TRUE);
		
		/* ----- NEGATIVE TRANSITION ------ */
		pState->fHeurWghtofFalseTransition = LSGBGetHeurScoreTransition(pState, BOOL_FALSE);
		
		pState = (SmurfStateEntry *)pState->pNextVarInThisState;
	}
}

void fillHEAP(int index, int size, int *arrElts) {
	SmurfStateEntry *CurrState = (SmurfStateEntry *)SimpleSmurfProblemState->pSmurfStatesTableTail;
	SimpleSmurfProblemState->nNumSmurfStateEntries++;
	SimpleSmurfProblemState->pSmurfStatesTableTail = (void *)(CurrState + 1);
	CurrState->cType = FN_SMURF;
	CurrState->nTransitionVar = arrElts[index+(size/2)];
	//fprintf(stderr, "%d(%d) - ", arrElts[index+(size/2)], size);
	if(size<=1) return;
	CurrState->pNextVarInThisStateGT = SimpleSmurfProblemState->pSmurfStatesTableTail;
	fillHEAP(index, size/2, arrElts);
	if(size<=2) return;
	CurrState->pNextVarInThisStateLT = SimpleSmurfProblemState->pSmurfStatesTableTail;
	fillHEAP(index+(size/2)+1, (size-(size/2))-1, arrElts);
	assert(CurrState->pNextVarInThisStateGT != CurrState->pNextVarInThisStateLT);
}

SmurfStateEntry *findStateInHEAP(SmurfStateEntry *pStartState, int var) {
	while(pStartState->nTransitionVar != var) {
		//fprintf(stderr, "|%d, %d|", pStartState->nTransitionVar, var);
		if(var > pStartState->nTransitionVar) {
			pStartState = (SmurfStateEntry *)pStartState->pNextVarInThisStateGT;
		} else {
			pStartState = (SmurfStateEntry *)pStartState->pNextVarInThisStateLT;
		}
	}
	//fprintf(stderr, "return %d\n", pStartState->nTransitionVar);
	return pStartState;
}

int *tempint;
long tempint_max = 0;

void *ReadSmurfStateIntoTable(BDDNode *pCurrentBDD);

void *ReadORState(int *arrElts, int nNumElts, BDDNode *pCurrentBDD, ORStateEntry *pStartState) {
	check_SmurfStatesTableSize(sizeof(ORStateEntry));
	ite_counters[SMURF_STATES]+=1;

	pStartState = (ORStateEntry *)SimpleSmurfProblemState->pSmurfStatesTableTail;
	SimpleSmurfProblemState->pSmurfStatesTableTail = (void *)(pStartState + 1);
	pStartState->cType = FN_OR;
	pStartState->nTransitionVars = arrElts;
   pStartState->nSize = nNumElts;
	pStartState->bPolarity = (bool *)ite_calloc(nNumElts, sizeof(bool), 9, "bPolarity");
	for(int x = 0; x < nNumElts; x++) {
//		fprintf(stderr, "%d, %d, %d: ", x, arrElts[x], arrSolver2IteVarMap[arrElts[x]]);
		if(set_variable(pCurrentBDD, arrSolver2IteVarMap[arrElts[x]], 1) == true_ptr)
		  pStartState->bPolarity[x] = 1;
	}
//	printBDDerr(pCurrentBDD);
//	fprintf(stderr, "\n");
//	PrintAllSmurfStateEntries();

	if(nNumElts == 2) return (void *)pStartState;

	ORCounterStateEntry *pCurrORCounter;
	void *pPrevORCounter = (void *)pStartState;
	for(int x = 2; x < nNumElts; x++) {
//		fprintf(stderr, "%d, %d, %d: ", x, arrElts[x], arrSolver2IteVarMap[arrElts[x]]);
		check_SmurfStatesTableSize(sizeof(ORCounterStateEntry));
		ite_counters[SMURF_STATES]+=1;
		pCurrORCounter = (ORCounterStateEntry *)SimpleSmurfProblemState->pSmurfStatesTableTail;
		SimpleSmurfProblemState->pSmurfStatesTableTail = (void *)(pCurrORCounter + 1);
		pCurrORCounter->cType = FN_OR_COUNTER;
		pCurrORCounter->pTransition = pPrevORCounter;
		pCurrORCounter->pORState = pStartState;
		pCurrORCounter->nSize = x+1;
		pPrevORCounter = (void *)pCurrORCounter;
	}

	pCurrentBDD->pState = (void *)pCurrORCounter;
	return (void *)pCurrORCounter;
}
	
void *ReadSmurfState(int *arrElts, int nNumElts, BDDNode *pCurrentBDD, SmurfStateEntry *pStartState) {
	check_SmurfStatesTableSize(sizeof(SmurfStateEntry)*nNumElts);
	ite_counters[SMURF_STATES]+=nNumElts;
	pCurrentBDD->pState = SimpleSmurfProblemState->pSmurfStatesTableTail;
	
	pStartState = (SmurfStateEntry *)SimpleSmurfProblemState->pSmurfStatesTableTail;
	fillHEAP(0, nNumElts, arrElts);
	
	for(int nVbleIndex = 0; nVbleIndex < nNumElts-1; nVbleIndex++) {
		SmurfStateEntry *CurrState = (pStartState+nVbleIndex);
		CurrState->pNextVarInThisState = (void *)(pStartState+nVbleIndex+1);
	}
	
	for(int nVbleIndex = 0; nVbleIndex < nNumElts; nVbleIndex++) {
//		fprintf(stderr, "%d, %d, %d: ", nVbleIndex, arrElts[nVbleIndex], arrSolver2IteVarMap[arrElts[nVbleIndex]]);
//		printBDDerr(pCurrentBDD);
//		fprintf(stderr, "\n");
//		PrintAllSmurfStateEntries();
			
		SmurfStateEntry *CurrState = findStateInHEAP(pStartState, arrElts[nVbleIndex]);
		
		assert(CurrState->nTransitionVar == arrElts[nVbleIndex]);
		
		//Determine and record if nTransitionVar is safe
		BDDNode *pSafeBDD = false_ptr;//safe_assign(pCurrentBDD->pFunc, CurrState->nTransitionVar);
		if(pSafeBDD == false_ptr)
		  CurrState->bVarIsSafe = 0;
		else if(pSafeBDD->thenCase == true_ptr && pSafeBDD->elseCase == false_ptr) {
			CurrState->bVarIsSafe = 1;
			fprintf(stderr, "{+1}");
		} else if(pSafeBDD->thenCase == false_ptr && pSafeBDD->elseCase == true_ptr) {
			CurrState->bVarIsSafe = -1;
			fprintf(stderr, "{-1}");
		}
		
		//Compute the SmurfState w/ nTransitionVar = False
		//Create Inference Transitions
		BDDNode *infBDD = pCurrentBDD;
		infBDD = set_variable(infBDD, arrSolver2IteVarMap[CurrState->nTransitionVar], 0);
		SmurfStateEntry *NextState = CurrState;
		InferenceStateEntry *NextInfState = NULL;
		//Follow positive and negative inferences
		infer *inferences = infBDD->inferences;
		bool bIsInference = 0;
		while(inferences!=NULL) {
			if(inferences->nums[1] != 0) { inferences = inferences->next; continue; }
			//Add a SmurfStateEntry into the table
			if(infBDD->pState != NULL) break;
			check_SmurfStatesTableSize(sizeof(InferenceStateEntry));
			ite_counters[SMURF_STATES]++;
			infBDD->pState = SimpleSmurfProblemState->pSmurfStatesTableTail;
			if(bIsInference == 0) NextState->pVarIsFalseTransition = infBDD->pState; //The transition is False
			else NextInfState->pVarTransition = infBDD->pState;
			bIsInference = 1;
			assert(infBDD->pState!=NULL);
			NextInfState = (InferenceStateEntry *)SimpleSmurfProblemState->pSmurfStatesTableTail;
			SimpleSmurfProblemState->nNumSmurfStateEntries++;
			SimpleSmurfProblemState->pSmurfStatesTableTail = (void *)(NextInfState + 1);
			NextInfState->cType = FN_INFERENCE;
			if(inferences->nums[0] > 0) { 
				NextInfState->nTransitionVar = arrIte2SolverVarMap[inferences->nums[0]];
				NextInfState->bPolarity = 1;
				infBDD = set_variable(infBDD, arrSolver2IteVarMap[NextInfState->nTransitionVar], 1);
			} else {
				NextInfState->nTransitionVar = arrIte2SolverVarMap[-inferences->nums[0]];
				NextInfState->bPolarity = 0;
				infBDD = set_variable(infBDD, arrSolver2IteVarMap[NextInfState->nTransitionVar], 0);
			}
			inferences = infBDD->inferences;
		}
		if(infBDD->pState != NULL) {
			if(bIsInference == 0) NextState->pVarIsFalseTransition = infBDD->pState; //The transition is False
			else NextInfState->pVarTransition = infBDD->pState;
		} else {
			//Recurse on nTransitionVar == False transition
			void *pNext = ReadSmurfStateIntoTable(infBDD);
			assert(pNext!=NULL);
			if(bIsInference == 0) NextState->pVarIsFalseTransition = pNext; //The transition is False
			else NextInfState->pVarTransition = pNext;
		}
		
		//Compute the SmurfState w/ nTransitionVar = True
		//Create Inference Transitions
		infBDD = pCurrentBDD;
		infBDD = set_variable(infBDD, arrSolver2IteVarMap[CurrState->nTransitionVar], 1);
		NextState = CurrState;
		NextInfState = NULL;
		//Follow positive and negative inferences
		inferences = infBDD->inferences;
		bIsInference = 0;
		while(inferences!=NULL) {
			if(inferences->nums[1] != 0) { inferences = inferences->next; continue; }
			//Add a SmurfStateEntry into the table
			if(infBDD->pState != NULL) break;
			check_SmurfStatesTableSize(sizeof(InferenceStateEntry));
			ite_counters[SMURF_STATES]++;
			infBDD->pState = SimpleSmurfProblemState->pSmurfStatesTableTail;
			if(bIsInference == 0) NextState->pVarIsTrueTransition = infBDD->pState; //The transition is True
			else NextInfState->pVarTransition = infBDD->pState;
			bIsInference = 1;
			assert(infBDD->pState!=NULL);
			NextInfState = (InferenceStateEntry *)SimpleSmurfProblemState->pSmurfStatesTableTail;
			SimpleSmurfProblemState->nNumSmurfStateEntries++;
			SimpleSmurfProblemState->pSmurfStatesTableTail = (void *)(NextInfState + 1);
			NextInfState->cType = FN_INFERENCE;
			if(inferences->nums[0] > 0) {
				NextInfState->nTransitionVar = arrIte2SolverVarMap[inferences->nums[0]];
				NextInfState->bPolarity = 1;
				infBDD = set_variable(infBDD, arrSolver2IteVarMap[NextInfState->nTransitionVar], 1);
			} else {
				NextInfState->nTransitionVar = arrIte2SolverVarMap[-inferences->nums[0]];
				NextInfState->bPolarity = 0;
				infBDD = set_variable(infBDD, arrSolver2IteVarMap[NextInfState->nTransitionVar], 0);
			}
			inferences = infBDD->inferences;
		}
		if(infBDD->pState != NULL) {
			if(bIsInference == 0) NextState->pVarIsTrueTransition = infBDD->pState; //The transition is True
			else NextInfState->pVarTransition = infBDD->pState;
		} else {
			//Recurse on nTransitionVar == True transition
			void *pNext = ReadSmurfStateIntoTable(infBDD);
			assert(pNext!=NULL);
			if(bIsInference == 0) NextState->pVarIsTrueTransition = pNext; //The transition is True
			else NextInfState->pVarTransition = pNext;
		}
	}
	return (void *)pStartState;
}

//This BDD, pCurrentBDD, cannot have any inferences, and has not been visited
void *ReadSmurfStateIntoTable(BDDNode *pCurrentBDD) {
	void *pStartState = pCurrentBDD->pState;
	if(pCurrentBDD != true_ptr && pCurrentBDD->pState==NULL) {
		//If this is the first transition in a SmurfState, mark this SmurfState as Visited      

		long nNumElts = 0;
		unravelBDD(&nNumElts, &tempint_max, &tempint, pCurrentBDD);
		int *arrElts = (int *)ite_calloc(nNumElts, sizeof(int), 9, "arrElts");
		for (int i=0;i<nNumElts;i++) {
			if (tempint[i]==0 ||
				 arrIte2SolverVarMap[tempint[i]]==0) {
				dE_printf1("\nassigned variable in a BDD in the solver");
				dE_printf3("\nvariable id: %d, true_false=%d\n",
							  tempint[i],
							  variablelist[tempint[i]].true_false);
				//exit(1);
			}
			arrElts[i] = arrIte2SolverVarMap[tempint[i]];
		}
		qsort(arrElts, nNumElts, sizeof(int), revcompfunc);

		int equ_var;
		if(nNumElts >= 2 &&//nNumElts >= functionTypeLimits[PLAINOR] &&
			isOR(pCurrentBDD)) {
			//FN_OR
			pStartState = ReadORState(arrElts, nNumElts, pCurrentBDD, (ORStateEntry *)pStartState);
			
			if(((TypeStateEntry *)pStartState)->cType == FN_OR)
			  LSGBORStateSetHeurScores((ORStateEntry *)pStartState);
			else if(((TypeStateEntry *)pStartState)->cType == FN_OR_COUNTER)
			  LSGBORCounterStateSetHeurScores((ORCounterStateEntry *)pStartState);
			
//			fprintf(stderr, "v");
		} else if(0 && 
					 nNumElts >= 3 &&//nNumElts >= functionTypeLimits[PLAINXOR] &&
					 isXOR(pCurrentBDD)) {
			//FN_XOR
			//pStartState = ReadSmurfState(arrElts, nNumElts, pCurrentBDD, (SmurfStateEntry *)pStartState);			
			//LSGBSmurfSetHeurScores((SmurfStateEntry *)pStartState);
//			fprintf(stderr, "+");
			pCurrentBDD->pState = pTrueSimpleSmurfState;
			pStartState = pTrueSimpleSmurfState;
			ite_free((void **)&arrElts);
		} else if(0 && 
					 nNumElts >= 3 &&//nNumElts >= functionTypeLimits[PLAINAND] &&
					 (equ_var = isAND_EQU(pCurrentBDD, tempint, nNumElts))!=0) {
			//FN_AND_EQU / FN_OR_EQU
			//pStartState = ReadSmurfState(arrElts, nNumElts, pCurrentBDD, (SmurfStateEntry *)pStartState);			
			//LSGBSmurfSetHeurScores((SmurfStateEntry *)pStartState);
//			fprintf(stderr, "=");
			pCurrentBDD->pState = pTrueSimpleSmurfState;
			pStartState = pTrueSimpleSmurfState;
			ite_free((void **)&arrElts);
		} else if(0 && 
					 nNumElts >= 3 &&// &&//nNumElts >= functionTypeLimits[PLAINAND] &&
					 isMIN_MAX(pCurrentBDD, tempint, nNumElts)) {
			//FN_MINMAX
			//pStartState = ReadSmurfState(arrElts, nNumElts, pCurrentBDD, (SmurfStateEntry *)pStartState);			
			//LSGBSmurfSetHeurScores((SmurfStateEntry *)pStartState);
//			fprintf(stderr, "M");
			pCurrentBDD->pState = pTrueSimpleSmurfState;
			pStartState = pTrueSimpleSmurfState;
			ite_free((void **)&arrElts);
		} else if(0 && 
					 nNumElts >= 3 &&//nNumElts >= functionTypeLimits[PLAINAND] &&
			       isNEG_MIN_MAX(pCurrentBDD, tempint, nNumElts)) {
			//FN_NEG_MINMAX
			//pStartState = ReadSmurfState(arrElts, nNumElts, pCurrentBDD, (SmurfStateEntry *)pStartState);			
			//LSGBSmurfSetHeurScores((SmurfStateEntry *)pStartState);
//			fprintf(stderr, "m");
			pCurrentBDD->pState = pTrueSimpleSmurfState;
			pStartState = pTrueSimpleSmurfState;
			ite_free((void **)&arrElts);
		} else {
			//FN_SMURF
			pStartState = ReadSmurfState(arrElts, nNumElts, pCurrentBDD, (SmurfStateEntry *)pStartState);
			LSGBSmurfSetHeurScores((SmurfStateEntry *)pStartState);
			ite_free((void **)&arrElts);
		}
	}
	return pStartState;
}

void ReadAllSmurfsIntoTable() {
	//Create the pTrueSimpleSmurfState entry
	SimpleSmurfProblemState = (ProblemState *)ite_calloc(1, sizeof(ProblemState), 9, "SimpleSmurfProblemState");
	SimpleSmurfProblemState->nNumSmurfs = nmbrFunctions;
	SimpleSmurfProblemState->nNumVars = nNumVariables;
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
				 arrIte2SolverVarMap[tempint[i]]==0) {
				dE_printf1("\nassigned variable in a BDD in the solver");
				dE_printf3("\nvariable id: %d, true_false=%d\n",
							  tempint[i],
							  variablelist[tempint[i]].true_false);
				//exit(1);
			}
			tempint[i] = arrIte2SolverVarMap[tempint[i]];
		}
//		qsort(tempint, nNumElts, sizeof(int), revcompfunc);

		for(int y = 0; y < nNumElts; y++)
		  temp_varcount[tempint[y]]++;
		
	}
	
	for(int x = 0; x < SimpleSmurfProblemState->nNumVars; x++) {
		SimpleSmurfProblemState->arrSmurfStack[x].arrSmurfStates
		  = (void **)ite_calloc(SimpleSmurfProblemState->nNumSmurfs, sizeof(int *), 9, "arrSmurfStates");
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
				 arrIte2SolverVarMap[tempint[i]]==0) {
				dE_printf1("\nassigned variable in a BDD in the solver");
				dE_printf3("\nvariable id: %d, true_false=%d\n",
							  tempint[i],
							  variablelist[tempint[i]].true_false);
				//exit(1);
			}
			tempint[i] = arrIte2SolverVarMap[tempint[i]];
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

ITE_INLINE
void Calculate_Heuristic_Values() {

	memset(SimpleSmurfProblemState->arrPosVarHeurWghts, 0, sizeof(double)*SimpleSmurfProblemState->nNumVars);
	memset(SimpleSmurfProblemState->arrNegVarHeurWghts, 0, sizeof(double)*SimpleSmurfProblemState->nNumVars);
	
	void **arrSmurfStates = SimpleSmurfProblemState->arrSmurfStack[SimpleSmurfProblemState->nCurrSearchTreeLevel].arrSmurfStates;
	for(int nSmurfIndex = 0; nSmurfIndex < SimpleSmurfProblemState->nNumSmurfs; nSmurfIndex++) {
		//TrueState
		if(arrSmurfStates[nSmurfIndex] == pTrueSimpleSmurfState) continue;
		//FN_SMURF
		if (((TypeStateEntry *)arrSmurfStates[nSmurfIndex])->cType == FN_SMURF) {
			SmurfStateEntry *pState = (SmurfStateEntry *)arrSmurfStates[nSmurfIndex];
			SimpleSmurfProblemState->arrPosVarHeurWghts[pState->nTransitionVar] += 
			  pState->fHeurWghtofTrueTransition;
			SimpleSmurfProblemState->arrNegVarHeurWghts[pState->nTransitionVar] +=  
			  pState->fHeurWghtofFalseTransition;
			while (pState->pNextVarInThisState != NULL) {
				pState = (SmurfStateEntry *)pState->pNextVarInThisState;
				SimpleSmurfProblemState->arrPosVarHeurWghts[pState->nTransitionVar] += 
				  pState->fHeurWghtofTrueTransition;
				SimpleSmurfProblemState->arrNegVarHeurWghts[pState->nTransitionVar] +=  
				  pState->fHeurWghtofFalseTransition;
			}
		//FN_OR
		} else if (((TypeStateEntry *)arrSmurfStates[nSmurfIndex])->cType == FN_OR) {
			ORStateEntry *pORState = (ORStateEntry *)arrSmurfStates[nSmurfIndex];
			int nCurrInfLevel = SimpleSmurfProblemState->arrSmurfStack[SimpleSmurfProblemState->nCurrSearchTreeLevel].nNumFreeVars;
			int numfound = 0;
			for(int index = 0; numfound < 2; index++) {
				if(SimpleSmurfProblemState->arrInferenceDeclaredAtLevel[pORState->nTransitionVars[index]] >= nCurrInfLevel) {
					numfound++;
					if(pORState->bPolarity[index] == BOOL_TRUE) {
						SimpleSmurfProblemState->arrPosVarHeurWghts[pORState->nTransitionVars[index]] += JHEURISTIC_K_TRUE;
						SimpleSmurfProblemState->arrNegVarHeurWghts[pORState->nTransitionVars[index]] += LSGBORGetHeurPos(pORState);
					} else {
						SimpleSmurfProblemState->arrPosVarHeurWghts[pORState->nTransitionVars[index]] += LSGBORGetHeurPos(pORState);
						SimpleSmurfProblemState->arrNegVarHeurWghts[pORState->nTransitionVars[index]] += JHEURISTIC_K_TRUE;
					}
				}
			}
		//FN_OR_COUNTER
		} else if (((TypeStateEntry *)arrSmurfStates[nSmurfIndex])->cType == FN_OR_COUNTER) {
			ORCounterStateEntry *pORCounterState = (ORCounterStateEntry *)arrSmurfStates[nSmurfIndex];
			int nCurrInfLevel = SimpleSmurfProblemState->arrSmurfStack[SimpleSmurfProblemState->nCurrSearchTreeLevel].nNumFreeVars;
			int numfound = 0;
			for(int index = 0; numfound < pORCounterState->nSize; index++) {
				if(SimpleSmurfProblemState->arrInferenceDeclaredAtLevel[pORCounterState->pORState->nTransitionVars[index]] >= nCurrInfLevel) {
					numfound++;
					if(pORCounterState->pORState->bPolarity[index] == BOOL_TRUE) {
						SimpleSmurfProblemState->arrPosVarHeurWghts[pORCounterState->pORState->nTransitionVars[index]] += JHEURISTIC_K_TRUE;
						SimpleSmurfProblemState->arrNegVarHeurWghts[pORCounterState->pORState->nTransitionVars[index]] += LSGBORCounterGetHeurPos(pORCounterState);
					} else {
						SimpleSmurfProblemState->arrPosVarHeurWghts[pORCounterState->pORState->nTransitionVars[index]] += LSGBORCounterGetHeurPos(pORCounterState);
						SimpleSmurfProblemState->arrNegVarHeurWghts[pORCounterState->pORState->nTransitionVars[index]] += JHEURISTIC_K_TRUE;
					}
				}
			}
		}
	}
	
	d7_printf1("JHeuristic values:\n");
	for(int nVar = 1; nVar < SimpleSmurfProblemState->nNumVars; nVar++) {
		d7_printf3(" %d: %4.6f\n", nVar, SimpleSmurfProblemState->arrPosVarHeurWghts[nVar]);
		d7_printf3("-%d: %4.6f\n", nVar, SimpleSmurfProblemState->arrNegVarHeurWghts[nVar]);
	}
	d7_printf1("\n");
}

ITE_INLINE
int Simple_LSGB_Heuristic() {
	
   Calculate_Heuristic_Values();
	
   int nBestVble = 0;
   double fMaxWeight = 0;
   double fVbleWeight = 0;
   int nCurrInfLevel = SimpleSmurfProblemState->arrSmurfStack[SimpleSmurfProblemState->nCurrSearchTreeLevel].nNumFreeVars;

   // Determine the variable with the highest weight:
   // 
   // Initialize to the lowest indexed variable whose value is uninstantiated.
	
	if (arrVarChoiceLevels) {
		int level=SimpleSmurfProblemState->arrSmurfStack[SimpleSmurfProblemState->nCurrSearchTreeLevel].nVarChoiceCurrLevel;
		for(;level<nVarChoiceLevelsNum;level++) {
			int j=SimpleSmurfProblemState->arrSmurfStack[SimpleSmurfProblemState->nCurrSearchTreeLevel].nHeuristicPlaceholder;
			while(arrVarChoiceLevels[level][j] != 0) {
				int i=arrVarChoiceLevels[level][j];
				if(SimpleSmurfProblemState->arrInferenceDeclaredAtLevel[i] >= nCurrInfLevel) {
					nBestVble = i;
					fMaxWeight = (1+SimpleSmurfProblemState->arrPosVarHeurWghts[i]) *
					  (1+SimpleSmurfProblemState->arrNegVarHeurWghts[i]);
					SimpleSmurfProblemState->arrSmurfStack[SimpleSmurfProblemState->nCurrSearchTreeLevel].nHeuristicPlaceholder = j;
					break;
				}
				j++;
			}
			if (nBestVble > 0) {
				while(arrVarChoiceLevels[level][j] != 0) {
					int i=arrVarChoiceLevels[level][j];
					if(SimpleSmurfProblemState->arrInferenceDeclaredAtLevel[i] >= nCurrInfLevel) {
						fVbleWeight = (1+SimpleSmurfProblemState->arrPosVarHeurWghts[i]) *
						  (1+SimpleSmurfProblemState->arrNegVarHeurWghts[i]);
						if(fVbleWeight > fMaxWeight) {
							fMaxWeight = fVbleWeight;
							nBestVble = i;
						}
					}
					j++;
				}
				SimpleSmurfProblemState->arrSmurfStack[SimpleSmurfProblemState->nCurrSearchTreeLevel].nVarChoiceCurrLevel = level;
				return (SimpleSmurfProblemState->arrPosVarHeurWghts[nBestVble] >= SimpleSmurfProblemState->arrNegVarHeurWghts[nBestVble]?nBestVble:-nBestVble);
			}
		}
	}

	for(int i = SimpleSmurfProblemState->arrSmurfStack[SimpleSmurfProblemState->nCurrSearchTreeLevel].nHeuristicPlaceholder+1; i < SimpleSmurfProblemState->nNumVars; i++) {
		if(SimpleSmurfProblemState->arrInferenceDeclaredAtLevel[i] >= nCurrInfLevel) {
			nBestVble = i;
			fMaxWeight = (1+SimpleSmurfProblemState->arrPosVarHeurWghts[i]) *
			  (1+SimpleSmurfProblemState->arrNegVarHeurWghts[i]);
			SimpleSmurfProblemState->arrSmurfStack[SimpleSmurfProblemState->nCurrSearchTreeLevel].nHeuristicPlaceholder = i-1;
			break;
		}
   }
	
   if(nBestVble > 0) {
		// Search through the remaining uninstantiated variables.
		for(int i = nBestVble + 1; i < SimpleSmurfProblemState->nNumVars; i++) {
			if(SimpleSmurfProblemState->arrInferenceDeclaredAtLevel[i] >= nCurrInfLevel) {
				fVbleWeight = (1+SimpleSmurfProblemState->arrPosVarHeurWghts[i]) *
				  (1+SimpleSmurfProblemState->arrNegVarHeurWghts[i]);
				if(fVbleWeight > fMaxWeight) {
					fMaxWeight = fVbleWeight;
					nBestVble = i;
				}
			}
		}
   } else {
		dE_printf1 ("Error in heuristic routine:  No uninstantiated variable found\n");
		exit (1);
   }
	
	SimpleSmurfProblemState->arrSmurfStack[SimpleSmurfProblemState->nCurrSearchTreeLevel].nVarChoiceCurrLevel = nVarChoiceLevelsNum;
   return (SimpleSmurfProblemState->arrPosVarHeurWghts[nBestVble] >= SimpleSmurfProblemState->arrNegVarHeurWghts[nBestVble]?nBestVble:-nBestVble);
}

ITE_INLINE
int Simple_DC_Heuristic() { //Simple Don't Care Heuristic
   int nBestVble = 0;
   int nCurrInfLevel = SimpleSmurfProblemState->arrSmurfStack[SimpleSmurfProblemState->nCurrSearchTreeLevel].nNumFreeVars;

   // Determine the variable with the highest weight:
   // 
   // Initialize to the lowest indexed variable whose value is uninstantiated.

	if (arrVarChoiceLevels) {
		int level=SimpleSmurfProblemState->arrSmurfStack[SimpleSmurfProblemState->nCurrSearchTreeLevel].nVarChoiceCurrLevel;
		for(;level<nVarChoiceLevelsNum;level++) {
			int j=SimpleSmurfProblemState->arrSmurfStack[SimpleSmurfProblemState->nCurrSearchTreeLevel].nHeuristicPlaceholder;
			while(arrVarChoiceLevels[level][j] != 0) {
				int i=arrVarChoiceLevels[level][j];
				if(SimpleSmurfProblemState->arrInferenceDeclaredAtLevel[i] >= nCurrInfLevel) {
					nBestVble = i;
					SimpleSmurfProblemState->arrSmurfStack[SimpleSmurfProblemState->nCurrSearchTreeLevel].nHeuristicPlaceholder = j;
					break;
				}
				j++;
			}
			if (nBestVble > 0) {
				SimpleSmurfProblemState->arrSmurfStack[SimpleSmurfProblemState->nCurrSearchTreeLevel].nVarChoiceCurrLevel = level;
				return -nBestVble;
			}
		}
	}

	for(int i = SimpleSmurfProblemState->arrSmurfStack[SimpleSmurfProblemState->nCurrSearchTreeLevel].nHeuristicPlaceholder+1; i < SimpleSmurfProblemState->nNumVars; i++) {
		if(SimpleSmurfProblemState->arrInferenceDeclaredAtLevel[i] >= nCurrInfLevel) {
			nBestVble = i;
			SimpleSmurfProblemState->arrSmurfStack[SimpleSmurfProblemState->nCurrSearchTreeLevel].nHeuristicPlaceholder = i-1;
			break;
		}
   }
	
   if(nBestVble == 0) {
		dE_printf1 ("Error in heuristic routine:  No uninstantiated variable found\n");
		exit (1);
   }
	
	SimpleSmurfProblemState->arrSmurfStack[SimpleSmurfProblemState->nCurrSearchTreeLevel].nVarChoiceCurrLevel = nVarChoiceLevelsNum;
	assert(nBestVble>0 && nBestVble<SimpleSmurfProblemState->nNumVars);
   return -nBestVble;
}

ITE_INLINE
int SimpleHeuristic() {
	int nBranchLit;
	
	//if(ite_counters[NUM_CHOICE_POINTS] %128 == 64)
	//if(SimpleSmurfProblemState->nCurrSearchTreeLevel > 10)
	//nBranchLit = Simple_DC_Heuristic(); //Don't Care - Choose the first unset variable found.
	//else 
	nBranchLit = Simple_LSGB_Heuristic();
	
	if(solver_polarity_presets_length > solver_polarity_presets_count) {
		d7_printf3("solver_polarity_presets forcing choice point at level %d to take value %c\n", SimpleSmurfProblemState->nCurrSearchTreeLevel, solver_polarity_presets[SimpleSmurfProblemState->nCurrSearchTreeLevel]);
		if(nBranchLit < 0) {
			if(solver_polarity_presets[solver_polarity_presets_count]=='+')
			  nBranchLit = -nBranchLit;
		} else {
			if(solver_polarity_presets[solver_polarity_presets_count]=='-')
			  nBranchLit = -nBranchLit;
		}
		solver_polarity_presets_count++;
	}
	
	if(simple_solver_reset_level == SimpleSmurfProblemState->nCurrSearchTreeLevel) {
		simple_solver_reset_level=-1;
		nSimpleSolver_Reset=1;
		nInfQueueStart = SimpleSmurfProblemState->arrSmurfStack[SimpleSmurfProblemState->nCurrSearchTreeLevel].nNumFreeVars+1;
		d7_printf2("Resetting solver at level %d\n", SimpleSmurfProblemState->nCurrSearchTreeLevel);
		fprintf(stderr, "here");
	}
	
	ite_counters[NUM_CHOICE_POINTS]++;
	ite_counters[NO_ERROR]++;
	
	return nBranchLit;
}

ITE_INLINE
int EnqueueInference(int nInfVar, bool bInfPolarity) {
	//Try to insert inference into the inference Queue
	int nInfQueueHead = SimpleSmurfProblemState->arrSmurfStack[SimpleSmurfProblemState->nCurrSearchTreeLevel].nNumFreeVars;
	int nPrevInfLevel = /*abs(*/SimpleSmurfProblemState->arrInferenceDeclaredAtLevel[nInfVar];
	//Sure, nPrevInfLevel could be zero, but only if it was a choicepoint and 
	//I think it's impossible for a prior choicepoint to be inferred here.
	assert(nPrevInfLevel > 0);
	d7_printf4("      Inferring %d at Level %d (prior level = %d)\n",
				  bInfPolarity?nInfVar:-nInfVar, nInfQueueHead, nPrevInfLevel);
	
	if(nPrevInfLevel < nInfQueueHead) {
		//Inference already in queue
		if((SimpleSmurfProblemState->arrInferenceQueue[nPrevInfLevel] > 0) != bInfPolarity) {
			//Conflict Detected;
			d7_printf2("      Conflict when adding %d to inference queue\n", bInfPolarity?nInfVar:-nInfVar); 
			return 0;
		} else {
			//Value is already inferred the correct value
			//Do nothing
			d7_printf2("      Inference %d already inferred\n", bInfPolarity?nInfVar:-nInfVar); 
		}
	} else {
		//Inference is not in inference queue, insert it.
		SimpleSmurfProblemState->arrInferenceQueue[nInfQueueHead] = bInfPolarity?nInfVar:-nInfVar;
		SimpleSmurfProblemState->arrInferenceDeclaredAtLevel[nInfVar] = nInfQueueHead;
		SimpleSmurfProblemState->arrSmurfStack[SimpleSmurfProblemState->nCurrSearchTreeLevel].nNumFreeVars++;
		ite_counters[INF_SMURF]++;
	}
	return 1;
}

ITE_INLINE
int ApplyInferenceToSmurfs(int nBranchVar, bool bBVPolarity, int nSmurfNumber, void **arrSmurfStates) {
	SmurfStateEntry *pSmurfState = (SmurfStateEntry *)arrSmurfStates[nSmurfNumber];
	do {
		if(pSmurfState->nTransitionVar < nBranchVar)
		  pSmurfState = (SmurfStateEntry *)pSmurfState->pNextVarInThisStateGT;
		else if(pSmurfState->nTransitionVar > nBranchVar)
		  pSmurfState = (SmurfStateEntry *)pSmurfState->pNextVarInThisStateLT;
		else { //(pSmurfState->nTransitionVar == nBranchVar) {
			//Follow this transition and apply all inferences found.
			InferenceStateEntry *pInference = (InferenceStateEntry *)(bBVPolarity?pSmurfState->pVarIsTrueTransition:pSmurfState->pVarIsFalseTransition);
			while(pInference->cType == FN_INFERENCE) {
				
				if(EnqueueInference(pInference->nTransitionVar, pInference->bPolarity > 0) == 0) return 0;
				
				//Follow the transtion to the next SmurfState
				pInference = (InferenceStateEntry *)pInference->pVarTransition;
			}
			//assert(pInference->cType == FN_SMURF);
			//pSmurfState = (SmurfStateEntry *)pInference;
			//Record the transition.
			arrSmurfStates[nSmurfNumber] = pInference;//pSmurfState;
			
			break;
		}
	} while (pSmurfState != NULL);
	d7_printf3("      Smurf %d transitioned to state %x\n", nSmurfNumber, arrSmurfStates[nSmurfNumber]);
	return 1;
}

int ApplyInferenceToOR(int nBranchVar, bool bBVPolarity, int nSmurfNumber, void **arrSmurfStates) {
	ORStateEntry *pORState = (ORStateEntry *)arrSmurfStates[nSmurfNumber];	
	
	//Check to see if nBranchVar is inferred correctly.
	int index = 0;
	int size = pORState->nSize;
	int prev = 0;
	int var = pORState->nTransitionVars[index+(size/2)];
	while(var != nBranchVar) {
		//fprintf(stderr, "%d(%d) - ", pORState->nTransitionVars[index+(size/2)], size);
		if(var == prev) break;
		prev = var;
		if(var < nBranchVar) {
			size = size/2;
		} else {
			index = index+(size/2)+1;
			size = (size-(size/2))-1;
			if(index+(size/2) == pORState->nSize) break;
		}
		var = pORState->nTransitionVars[index+(size/2)];
	}
	if(var != nBranchVar) return 1; //Var does not exist in this Smurf
	index = index+(size/2);
	
	if(pORState->bPolarity[index] == bBVPolarity) arrSmurfStates[nSmurfNumber] = pTrueSimpleSmurfState;
	else {
		int nInfQueueLevel = abs(SimpleSmurfProblemState->arrInferenceDeclaredAtLevel[nBranchVar]);
		for(int x = 0; x < pORState->nSize; x++) {
			int nPrevInfLevel = SimpleSmurfProblemState->arrInferenceDeclaredAtLevel[pORState->nTransitionVars[x]];
//			fprintf(stderr, "%d %d<%d x=%d var=%d pol=%d\n", nBranchVar, nPrevInfLevel, nInfQueueLevel, x, pORState->nTransitionVars[x], pORState->bPolarity[x]);
//			if(x == index) continue;
			if(nPrevInfLevel <= 0) continue;
			if(nPrevInfLevel <= nInfQueueLevel) {
				assert((SimpleSmurfProblemState->arrInferenceQueue[nPrevInfLevel] == 0) ||
						 ((SimpleSmurfProblemState->arrInferenceQueue[nPrevInfLevel] > 0) != pORState->bPolarity[x]));
				continue;
			}
			
			//Inference is not in inference queue, insert it.
			if(EnqueueInference(pORState->nTransitionVars[x], pORState->bPolarity[x]) == 0) return 0;
			arrSmurfStates[nSmurfNumber] = pTrueSimpleSmurfState;
			break;
		}
	}
	d7_printf3("      ORSmurf %d transitioned to state %x\n", nSmurfNumber, arrSmurfStates[nSmurfNumber]);
	return 1;
}

int ApplyInferenceToORCounter(int nBranchVar, bool bBVPolarity, int nSmurfNumber, void **arrSmurfStates) {
	ORCounterStateEntry *pORCounterState = (ORCounterStateEntry *)arrSmurfStates[nSmurfNumber];
	ORStateEntry *pORState = (ORStateEntry *)pORCounterState->pORState;
	
	int index = 0;
	int size = pORState->nSize;
	int prev = 0;
	int var = pORState->nTransitionVars[index+(size/2)];
	while(var != nBranchVar) {
		//fprintf(stderr, "%d(%d) - ", pORState->nTransitionVars[index+(size/2)], size);
		if(var == prev) break;
		prev = var;
		if(var < nBranchVar) {
			size = size/2;
		} else {
			index = index+(size/2)+1;
			size = (size-(size/2))-1;
			if(index+(size/2) == pORState->nSize) break;
		}
		var = pORState->nTransitionVars[index+(size/2)];
	}
	if(var != nBranchVar) return 1; //Var does not exist in this Smurf
	index = index+(size/2);
	
	if(pORState->bPolarity[index] == bBVPolarity) arrSmurfStates[nSmurfNumber] = pTrueSimpleSmurfState;
	else {
		arrSmurfStates[nSmurfNumber] = pORCounterState->pTransition;
	}	
	d7_printf3("      ORCounterSmurf %d transitioned to state %x\n", nSmurfNumber, arrSmurfStates[nSmurfNumber]);
	return 1;
}

ITE_INLINE
int ApplyInferenceToStates(int nBranchVar, bool bBVPolarity) {
	void **arrSmurfStates = SimpleSmurfProblemState->arrSmurfStack[SimpleSmurfProblemState->nCurrSearchTreeLevel].arrSmurfStates;
	d7_printf2("  Transitioning Smurfs using %d\n", bBVPolarity?nBranchVar:-nBranchVar);
	for(int i = 0; SimpleSmurfProblemState->arrVariableOccursInSmurf[nBranchVar][i] != -1; i++) {
		int nSmurfNumber = SimpleSmurfProblemState->arrVariableOccursInSmurf[nBranchVar][i];
		d7_printf3("    Checking Smurf %d (State %x)\n", nSmurfNumber, arrSmurfStates[nSmurfNumber]);
		if(arrSmurfStates[nSmurfNumber] == pTrueSimpleSmurfState) continue;
		void *pState = arrSmurfStates[nSmurfNumber];
		int ret = 1;
		if(((TypeStateEntry *)pState)->cType == FN_SMURF) {
			ret = ApplyInferenceToSmurfs(nBranchVar, bBVPolarity, nSmurfNumber, arrSmurfStates);
		} else if(((TypeStateEntry *)pState)->cType == FN_OR_COUNTER) {
			ret = ApplyInferenceToORCounter(nBranchVar, bBVPolarity, nSmurfNumber, arrSmurfStates);
		} else if(((TypeStateEntry *)pState)->cType == FN_OR) {
			ret = ApplyInferenceToOR(nBranchVar, bBVPolarity, nSmurfNumber, arrSmurfStates);
		}
		if(ret == 0) return 0;
	}
	return 1;
}

int Init_SimpleSmurfSolver() {
	//Clear all FunctionTypes
	for(int x = 0; x < nmbrFunctions; x++)
	  functionType[x] = UNSURE;
	
	InitVarMap();
	InitVariables();
	//InitFunctions();
	/* Convert bdds to smurfs */
	int ret = SOLV_UNKNOWN;// = CreateFunctions();
	if (ret != SOLV_UNKNOWN) return ret;
	
	ReadAllSmurfsIntoTable();
	
	FreeFunctions();
	FreeVariables();
	//FreeVarMap(); //This is freed at the end of simpleSolve(). 
	                //arrVarChoiceLevels and arrSolver2IteVarMap are needed during search.

	simple_solver_reset_level = solver_reset_level-1;
	
	return ret;
}

ITE_INLINE
void SmurfStates_Push(int destination) {
	//destination=nCurrSearchTreeLevel+1, except in the case of a nSimpleSolver_Reset then destination=0
	
	memcpy_ite(SimpleSmurfProblemState->arrSmurfStack[destination].arrSmurfStates,
				  SimpleSmurfProblemState->arrSmurfStack[SimpleSmurfProblemState->nCurrSearchTreeLevel].arrSmurfStates,
				  SimpleSmurfProblemState->nNumSmurfs*sizeof(int));

//	for(int i = 0; i < SimpleSmurfProblemState->nNumSmurfs; i++) {
//		SimpleSmurfProblemState->arrSmurfStack[destination].arrSmurfStates[i] = 
//		  SimpleSmurfProblemState->arrSmurfStack[SimpleSmurfProblemState->nCurrSearchTreeLevel].arrSmurfStates[i];
//	}
	
	SimpleSmurfProblemState->arrSmurfStack[destination].nNumFreeVars =
	  SimpleSmurfProblemState->arrSmurfStack[SimpleSmurfProblemState->nCurrSearchTreeLevel].nNumFreeVars;

	SimpleSmurfProblemState->arrSmurfStack[destination].nVarChoiceCurrLevel =
	  SimpleSmurfProblemState->arrSmurfStack[SimpleSmurfProblemState->nCurrSearchTreeLevel].nVarChoiceCurrLevel;
	
	SimpleSmurfProblemState->arrSmurfStack[destination].nHeuristicPlaceholder =
	  SimpleSmurfProblemState->arrSmurfStack[SimpleSmurfProblemState->nCurrSearchTreeLevel].nHeuristicPlaceholder;
	
	SimpleSmurfProblemState->nCurrSearchTreeLevel = destination;
}

ITE_INLINE
int SmurfStates_Pop() {
	d7_printf1("  Conflict - Backtracking\n");

	ite_counters[NUM_BACKTRACKS]++;
	d9_printf2("\nStarting backtrack # %ld\n", (long)ite_counters[NUM_BACKTRACKS]);
	if (ite_counters[NUM_BACKTRACKS] % BACKTRACKS_PER_STAT_REPORT == 0) {
		DisplaySimpleSolverBacktrackInfo(fSimpleSolverPrevEndTime, fSimpleSolverStartTime);
      if (fd_csv_trace_file) {
			DisplaySimpleSolverBacktrackInfo_gnuplot(fSimpleSolverPrevEndTime, fSimpleSolverStartTime);
		}
	}
		
	SimpleSmurfProblemState->nCurrSearchTreeLevel--;
	
	if(SimpleSmurfProblemState->nCurrSearchTreeLevel < 0)
	  return 0;
	return 1;  
}

ITE_INLINE
int backtrack() {
	//Pop stack
	ite_counters[ERR_BT_SMURF]++;
	
	int nInfQueueTail = SimpleSmurfProblemState->arrSmurfStack[SimpleSmurfProblemState->nCurrSearchTreeLevel].nNumFreeVars;
	
	if(SmurfStates_Pop() == 0) {
		//We are at the top of the stack
		if(ite_counters[NUM_SOLUTIONS] == 0) return SOLV_UNSAT; //return Unsatisifable
		else return SOLV_SAT;
	}

	int nInfQueueHead = SimpleSmurfProblemState->arrSmurfStack[SimpleSmurfProblemState->nCurrSearchTreeLevel].nNumFreeVars;
	
	//Empty the inference queue & reverse polarity of cp var
	//Clear Inference Queue

	//Using the inference queue to avoid a linear check over all variables.
	for(int i = nInfQueueHead; i < nInfQueueTail; i++) {
		int nBranchLit = abs(SimpleSmurfProblemState->arrInferenceQueue[i]);
		d7_printf3("      Resetting level of variable %d to %d\n", nBranchLit, SimpleSmurfProblemState->nNumVars);
		SimpleSmurfProblemState->arrInferenceDeclaredAtLevel[nBranchLit] = SimpleSmurfProblemState->nNumVars;
	}
	  
/*	for(int i = 1; i < SimpleSmurfProblemState->nNumVars; i++)
	  if(abs(SimpleSmurfProblemState->arrInferenceDeclaredAtLevel[i]) > SimpleSmurfProblemState->arrSmurfStack[SimpleSmurfProblemState->nCurrSearchTreeLevel].nNumFreeVars ||
		  abs(SimpleSmurfProblemState->arrInferenceQueue[abs(SimpleSmurfProblemState->arrInferenceDeclaredAtLevel[i])]) != i) {
        //d7_printf3("      Resetting level of variable %d to %d\n", i, SimpleSmurfProblemState->nNumVars);
		  assert(abs(SimpleSmurfProblemState->arrInferenceDeclaredAtLevel[i]) == SimpleSmurfProblemState->nNumVars);
	  }
*/

	return SOLV_UNKNOWN;
}

int SimpleBrancher() {

	int ret = SOLV_UNKNOWN;
	
	SimpleSmurfProblemState->nCurrSearchTreeLevel = 0;

	fSimpleSolverStartTime = get_runtime();

	int max_solutions_simple = max_solutions>0?max_solutions:(max_solutions==0?-1:0);
	
	while(ite_counters[NUM_SOLUTIONS] != max_solutions_simple) {
		bool bBVPolarity; 
		int nBranchLit, nInfQueueHead, nPrevInfLevel, nBranchVar;
		  
		while(SimpleSmurfProblemState->arrSmurfStack[SimpleSmurfProblemState->nCurrSearchTreeLevel].nNumFreeVars < SimpleSmurfProblemState->nNumVars-1) {
			//Update heuristic values
			
			//Call Heuristic to get variable and polarity
			d7_printf2("Calling heuristic to choose choice point #%lld\n", ite_counters[NUM_CHOICE_POINTS]);
			nBranchLit = SimpleHeuristic();
			
			//Push stack
			if(nSimpleSolver_Reset) { SmurfStates_Push(0); nSimpleSolver_Reset = 0; }
			else SmurfStates_Push(SimpleSmurfProblemState->nCurrSearchTreeLevel+1); //Normal condition
			
			//Insert heuristic var into inference queue
			nInfQueueHead = SimpleSmurfProblemState->arrSmurfStack[SimpleSmurfProblemState->nCurrSearchTreeLevel].nNumFreeVars;
			//First clear out old inference
			//SimpleSmurfProblemState->arrInferenceDeclaredAtLevel[abs(SimpleSmurfProblemState->arrInferenceQueue[nInfQueueHead])] =
			//SimpleSmurfProblemState->nNumVars;
			//Then insert new inference
			nPrevInfLevel = SimpleSmurfProblemState->arrInferenceDeclaredAtLevel[abs(nBranchLit)];
			SimpleSmurfProblemState->arrInferenceQueue[nInfQueueHead] = nBranchLit;
			SimpleSmurfProblemState->arrInferenceDeclaredAtLevel[abs(nBranchLit)] = nInfQueueHead;
			SimpleSmurfProblemState->arrSmurfStack[SimpleSmurfProblemState->nCurrSearchTreeLevel].nNumFreeVars++;
			d7_printf5("Inferring %c%d (choice point) at Level %d (prior level = %d)\n",
						  nBranchLit>0?'+':'-', abs(nBranchLit), nInfQueueHead, nPrevInfLevel);
			//    d7_printf3("Choice Point #%d = %d\n", , nBranchLit);

			//While inference queue != empty {
			while(nInfQueueHead != SimpleSmurfProblemState->arrSmurfStack[SimpleSmurfProblemState->nCurrSearchTreeLevel].nNumFreeVars) {
				//Get inference
				nBranchLit = SimpleSmurfProblemState->arrInferenceQueue[nInfQueueHead];
				nInfQueueHead++;
				bBVPolarity = 1;
				if(nBranchLit < 0) bBVPolarity = 0;
				nBranchVar = bBVPolarity?nBranchLit:-nBranchLit;
				
				//apply inference to all involved smurfs
				if(ApplyInferenceToStates(nBranchVar, bBVPolarity) == 0) {
					//A conflict occured
					
					switch(backtrack()) {
					 case SOLV_UNSAT: return SOLV_UNSAT;
					 case SOLV_SAT: return SOLV_SAT;
					 default: break;
					}
					
find_more_solutions: ;
					
					nBranchLit = SimpleSmurfProblemState->arrInferenceQueue[SimpleSmurfProblemState->arrSmurfStack[SimpleSmurfProblemState->nCurrSearchTreeLevel].nNumFreeVars];
					SimpleSmurfProblemState->arrInferenceDeclaredAtLevel[abs(nBranchLit)] = -SimpleSmurfProblemState->arrSmurfStack[SimpleSmurfProblemState->nCurrSearchTreeLevel].nNumFreeVars; //Negate this value to flag variable as an old choicepoint
					if(SimpleSmurfProblemState->arrInferenceDeclaredAtLevel[abs(nBranchLit)] == 0)
					  add_one_display = 1;
					SimpleSmurfProblemState->arrInferenceQueue[SimpleSmurfProblemState->arrSmurfStack[SimpleSmurfProblemState->nCurrSearchTreeLevel].nNumFreeVars] = -nBranchLit;
					nInfQueueHead = SimpleSmurfProblemState->arrSmurfStack[SimpleSmurfProblemState->nCurrSearchTreeLevel].nNumFreeVars;
					SimpleSmurfProblemState->arrSmurfStack[SimpleSmurfProblemState->nCurrSearchTreeLevel].nNumFreeVars++;
					d7_printf3("Flipping Value of Choicepoint at level %d to %d\n", nInfQueueHead, -nBranchLit);
					//continue the loop
				}
			}
			d7_printf1("\n");
		}

		//Record solution

		save_solution_simple();

		ite_counters[NUM_SOLUTIONS]++;
		ret = SOLV_SAT;
		
		if(ite_counters[NUM_SOLUTIONS] != max_solutions_simple) {
			do {
				if(nForceBackjumpLevel < SimpleSmurfProblemState->arrSmurfStack[SimpleSmurfProblemState->nCurrSearchTreeLevel].nVarChoiceCurrLevel)
				  d7_printf2("Forcing a backjump to level %d\n", nForceBackjumpLevel);
				switch(backtrack()) {
				 case SOLV_UNSAT: return SOLV_UNSAT;
				 case SOLV_SAT: return SOLV_SAT;
				 default: break;
				}
			} while(nForceBackjumpLevel < SimpleSmurfProblemState->arrSmurfStack[SimpleSmurfProblemState->nCurrSearchTreeLevel].nVarChoiceCurrLevel);
			goto find_more_solutions;
		}
	}

	return ret;
}

int simpleSolve() {
	int nForceBackjumpLevel_old = nForceBackjumpLevel;
	if(nForceBackjumpLevel < 0) nForceBackjumpLevel = nVarChoiceLevelsNum+1;
	//Clear function type:
	for(int x = 0; x < nmbrFunctions; x++)
	  functionType[x] = UNSURE;
	int ret = Init_SimpleSmurfSolver();
	if(ret != SOLV_UNKNOWN) return ret;
	if(*csv_trace_file)
	  fd_csv_trace_file = fopen(csv_trace_file, "w");
	ret = SimpleBrancher();  
	DisplayStatistics(ite_counters[NUM_CHOICE_POINTS], ite_counters[NUM_BACKTRACKS], ite_counters[NUM_BACKJUMPS]);
	if (fd_csv_trace_file) {
		DisplaySimpleSolverBacktrackInfo_gnuplot(fSimpleSolverPrevEndTime, fSimpleSolverStartTime);
		fclose(fd_csv_trace_file);
	}
	//Still need to do some backend stuff like free memory.
	nForceBackjumpLevel = nForceBackjumpLevel_old;
	FreeVarMap();
	return ret;
	//Hmmm I didn't free ANY of that memory. SEAN!!! FIX THIS!!!
}

