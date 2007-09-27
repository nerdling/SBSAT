#include "sbsat.h"
#include "sbsat_solver.h"
#include "solver.h"

/********** Included in include/sbsat_solver.h *********************
struct SmurfStateEntry{
	// Static
   char cType;
   int nTransitionVar;
	void *nVarIsTrueTransition;
	void *nVarIsFalseTransition;
	double nHeurWghtofTrueTransition;
	double nHeurWghtofFalseTransition;
	char nVarIsAnInference;
	//This is 1 if nTransitionVar should be inferred True,
	//       -1 if nTransitionVar should be inferred False,
	//        0 if nTransitionVar should not be inferred.
	void *nNextVarInThisStateGT; //There are n SmurfStateEntries linked together
 	void *nNextVarInThisStateLT; //in the structure of a heap,
	                             //where n is the number of variables in this SmurfStateEntry.
                          	     //All of these SmurfStateEntries represent the same function,
                                //but a different variable (nTransitionVar) is
                                //highlighted for each link in the heap.
                                //If this is 0, we have reached a leaf node.
	void *nNextVarInThisState;   //Same as above except linked linearly, instead of a heap.
                                //Used for computing the heuristic of a state.
 };

struct SmurfStack{
	int nNumFreeVars;
   int nHeuristicPlaceholder;
   int nVarChoiceCurrLevel;   //Index to array of size nNumVars
   void **arrSmurfStates;     //Pointer to array of size nNumSmurfs
};

struct ProblemState{
	// Static
	int nNumSmurfs;
	int nNumVars;
	int nNumSmurfStateEntries;
   void *vSmurfStatesTableTail;
   void **arrSmurfStatesTable;     //Pointer to the table of all smurf states.
	                                //Will be of size nNumSmurfStateEntries
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

SmurfStateEntry *TrueSimpleSmurfState;

#define HEUR_MULT 10000
#define FN_SMURF 0

ProblemState *SimpleSmurfProblemState;

double fSimpleSolverStartTime;
double fSimpleSolverEndTime;
double fSimpleSolverPrevEndTime;

int add_one_display=0;

int smurfs_share_paths=1;

void PrintSmurfStateEntry(SmurfStateEntry *ssEntry) {
	d9_printf10("Var=%d, v=T:%d, v=F:%d, TWght=%4.6f, FWght=%4.6f, Inf=%d, NextGT=%d, NextLT=%d, Next=%d\n", ssEntry->nTransitionVar, ssEntry->nVarIsTrueTransition, ssEntry->nVarIsFalseTransition, ssEntry->nHeurWghtofTrueTransition, ssEntry->nHeurWghtofFalseTransition, ssEntry->nVarIsAnInference, ssEntry->nNextVarInThisStateGT, ssEntry->nNextVarInThisStateLT, ssEntry->nNextVarInThisState);
}

void PrintAllSmurfStateEntries() {
	SmurfStateEntry *arrSmurfStatesTable = (SmurfStateEntry *)SimpleSmurfProblemState->arrSmurfStatesTable;
	for(int x = 0; x < SimpleSmurfProblemState->nNumSmurfStateEntries; x++) {
		d9_printf2("State %d, ", x);
		PrintSmurfStateEntry(&arrSmurfStatesTable[x]);
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

	int nInfQueueTail = SimpleSmurfProblemState->arrSmurfStack[SimpleSmurfProblemState->nCurrSearchTreeLevel].nNumFreeVars;
	
	int nCurrSearchTreeLevel = 0;
	for(int i = 0; i < nInfQueueTail && (count<soft_count || (count < hard_count && whereAmI==0)); i++) {
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
	progress = (float)whereAmI*100/total;
	fprintf(fd_csv_trace_file, "%4.3f ", progress);
	
	fprintf(fd_csv_trace_file, "%lld %lld ", ite_counters[NUM_CHOICE_POINTS], ite_counters[HEU_DEP_VAR]);
	fprintf(fd_csv_trace_file, "%lld ", ite_counters[INF_SMURF]);
	fprintf(fd_csv_trace_file, "%lld ", ite_counters[ERR_BT_SMURF]);
	fprintf(fd_csv_trace_file, "%ld %ld\n", (long)ite_counters[NUM_SOLUTIONS], (long)max_solutions);
}

int NumOfSmurfStatesInSmurf(SmurfState *pCurrentState) {
	int num_states = 0;
	if(pCurrentState != pTrueSmurfState) {
		pCurrentState->pFunc->flag = SimpleSmurfProblemState->nNumSmurfStateEntries;
		for(int nVbleIndex = 0; nVbleIndex < pCurrentState->vbles.nNumElts; nVbleIndex++) {
			BDDNode *infBDD = pCurrentState->pFunc;
			num_states++;
			infBDD = set_variable_noflag(infBDD, arrSolver2IteVarMap[pCurrentState->vbles.arrElts[nVbleIndex]], 0);
			Transition *pTransition = &pCurrentState->arrTransitions[2*nVbleIndex];
			//Follow Positive inferences
			for(int infIdx = 0; infIdx < pTransition->positiveInferences.nNumElts && infBDD->flag == 0; infIdx++) {
				infBDD->flag = 1;
				num_states++;
				infBDD = set_variable_noflag(infBDD, arrSolver2IteVarMap[pTransition->positiveInferences.arrElts[infIdx]], 1);
			}
			//Follow Negative inferences
			for(int infIdx = 0; infIdx < pTransition->negativeInferences.nNumElts && infBDD->flag == 0; infIdx++) {
				infBDD->flag = 1;
				num_states++;
				infBDD = set_variable_noflag(infBDD, arrSolver2IteVarMap[pTransition->negativeInferences.arrElts[infIdx]], 0);
			}
			if(infBDD->flag == 0) {
				assert(pTransition->pNextState->pFunc == infBDD);
				num_states += NumOfSmurfStatesInSmurf(pTransition->pNextState);
			}
			
			infBDD = pCurrentState->pFunc;
			infBDD = set_variable_noflag(infBDD, arrSolver2IteVarMap[pCurrentState->vbles.arrElts[nVbleIndex]], 1);
			pTransition = &pCurrentState->arrTransitions[2*nVbleIndex+1];
			//Follow Negative inferences
			for(int infIdx = 0; infIdx < pTransition->negativeInferences.nNumElts && infBDD->flag == 0; infIdx++) {
				infBDD->flag = 1;
				num_states++;
				infBDD = set_variable_noflag(infBDD, arrSolver2IteVarMap[pTransition->negativeInferences.arrElts[infIdx]], 0);
				
			}
			//Follow Positive inferences
			for(int infIdx = 0; infIdx < pTransition->positiveInferences.nNumElts && infBDD->flag == 0; infIdx++) {
				infBDD->flag = 1;
				num_states++;
				infBDD = set_variable_noflag(infBDD, arrSolver2IteVarMap[pTransition->positiveInferences.arrElts[infIdx]], 1);
			}
			if(infBDD->flag == 0) {
				assert(pTransition->pNextState->pFunc == infBDD);
				num_states +=NumOfSmurfStatesInSmurf(pTransition->pNextState);
			}
		}
	}
	return num_states;
}

int DetermineNumOfSmurfStates() {
	clear_all_bdd_flags();
	true_ptr->flag = 1;
	//Create the rest of the SmurfState entries
	int num_states = 0;
	for(int nSmurfIndex = 0; nSmurfIndex < SimpleSmurfProblemState->nNumSmurfs; nSmurfIndex++) {
		SmurfState *pInitialState = arrSolverFunctions[nSmurfIndex].fn_smurf.pInitialState;
		if(!smurfs_share_paths) { clear_all_bdd_flags(); true_ptr->flag = 1; }
		if(pInitialState->pFunc->flag == 0) {
			num_states += NumOfSmurfStatesInSmurf(pInitialState);
		}
	}
	d7_printf2("NumSmurfStates = %d\n", num_states);
	return num_states;
}

void fillHEAP(int index, int size, int *vbles) {
	SmurfStateEntry *CurrState = (SmurfStateEntry *)SimpleSmurfProblemState->vSmurfStatesTableTail;
	SimpleSmurfProblemState->nNumSmurfStateEntries++;
	SimpleSmurfProblemState->vSmurfStatesTableTail = (void *)(CurrState + 1);
	CurrState->nTransitionVar = vbles[index+(size/2)];
	//fprintf(stderr, "%d(%d) - ", vbles[index+(size/2)], size);
	if(size<=1) return;
	CurrState->nNextVarInThisStateGT = SimpleSmurfProblemState->vSmurfStatesTableTail;
	fillHEAP(index, size/2, vbles);
	if(size<=2) return;
	CurrState->nNextVarInThisStateLT = SimpleSmurfProblemState->vSmurfStatesTableTail;
	fillHEAP(index+(size/2)+1, (size-(size/2))-1, vbles);
	assert(CurrState->nNextVarInThisStateGT != CurrState->nNextVarInThisStateLT);
}

SmurfStateEntry *findStateInHEAP(SmurfStateEntry *pStartState, int var) {
	while(pStartState->nTransitionVar != var) {
		//fprintf(stderr, "|%d, %d|", pStartState->nTransitionVar, var);
		if(var > pStartState->nTransitionVar) {
			pStartState = (SmurfStateEntry *)pStartState->nNextVarInThisStateGT;
		} else {
			pStartState = (SmurfStateEntry *)pStartState->nNextVarInThisStateLT;
		}
	}
	//fprintf(stderr, "return %d\n", pStartState->nTransitionVar);
	return pStartState;
}

//This state, pCurrentState, cannot have any inferences, and has not been visited
void ReadSmurfStateIntoTable(SmurfState *pCurrentState) {
	if(pCurrentState != pTrueSmurfState && pCurrentState->pFunc->pState==NULL) {
		//If this is the first transition in a SmurfState, mark this SmurfState as Visited      
		pCurrentState->pFunc->pState = SimpleSmurfProblemState->vSmurfStatesTableTail;

		//This sort was already done in src/solv_smurf/fn_smurf/bdd2smurf.cc: 
		//qsort(pCurrentState->vbles.arrElts, pCurrentState->vbles.nNumElts, sizeof(int), revcompfunc);
		
		SmurfStateEntry *pStartState = (SmurfStateEntry *)SimpleSmurfProblemState->vSmurfStatesTableTail;
		fillHEAP(0, pCurrentState->vbles.nNumElts, pCurrentState->vbles.arrElts);

		for(int nVbleIndex = 0; nVbleIndex < pCurrentState->vbles.nNumElts-1; nVbleIndex++) {
			SmurfStateEntry *CurrState = (SmurfStateEntry *)(pStartState+nVbleIndex);
			CurrState->nNextVarInThisState = (void *)(pStartState+nVbleIndex+1);
		}
		
		for(int nVbleIndex = 0; nVbleIndex < pCurrentState->vbles.nNumElts; nVbleIndex++) {
			//fprintf(stderr, "%d, %d, %d: ", nVbleIndex, pCurrentState->vbles.arrElts[nVbleIndex], arrSolver2IteVarMap[pCurrentState->vbles.arrElts[nVbleIndex]]);
			//printBDDerr(pCurrentState->pFunc);
			//fprintf(stderr, "\n");
			//PrintAllSmurfStateEntries();

			
			//	SmurfStateEntry *pStartState = &(SimpleSmurfProblemState->arrSmurfStatesTable[nStartState]);
			
			SmurfStateEntry *CurrState = findStateInHEAP(pStartState, pCurrentState->vbles.arrElts[nVbleIndex]);

			assert(CurrState->nTransitionVar == pCurrentState->vbles.arrElts[nVbleIndex]);
			CurrState->nVarIsAnInference = 0;
			CurrState->nHeurWghtofFalseTransition
			  = pCurrentState->arrTransitions[2*nVbleIndex].fHeuristicWeight;
			//= (int)(pCurrentState->arrTransitions[2*nVbleIndex].fHeuristicWeight * HEUR_MULT); //???
			CurrState->nHeurWghtofTrueTransition
			  = pCurrentState->arrTransitions[2*nVbleIndex+1].fHeuristicWeight;
			//= (int)(pCurrentState->arrTransitions[2*nVbleIndex+1].fHeuristicWeight * HEUR_MULT); //???

			//Determine and record if nTransitionVar is safe
			BDDNode *pSafeBDD = false_ptr;//safe_assign(pCurrentState->pFunc, CurrState->nTransitionVar);
			if(pSafeBDD == false_ptr)
			  CurrState->cVarIsSafe = 0;
			else if(pSafeBDD->thenCase == true_ptr && pSafeBDD->elseCase == false_ptr) {
				CurrState->cVarIsSafe = 1;
				fprintf(stderr, "{+1}");
			} else if(pSafeBDD->thenCase == false_ptr && pSafeBDD->elseCase == true_ptr) {
				CurrState->cVarIsSafe = -1;
				fprintf(stderr, "{-1}");
			}
			
			//Compute the SmurfState w/ nTransitionVar = False
			//Create Inference Transitions
			BDDNode *infBDD = pCurrentState->pFunc;
			int nTransition_polarity = 0;
			infBDD = set_variable_noflag(infBDD, arrSolver2IteVarMap[CurrState->nTransitionVar], nTransition_polarity);
			Transition *pTransition = &(pCurrentState->arrTransitions[2*nVbleIndex]);
			SmurfStateEntry *NextState = CurrState;
			//Follow Positive inferences
			for(int infIdx = 0; infIdx < pTransition->positiveInferences.nNumElts && infBDD->pState == NULL; infIdx++) {
				//Add a SmurfStateEntry into the table
				if(infBDD->pState != NULL) break;
				infBDD->pState = SimpleSmurfProblemState->vSmurfStatesTableTail;
				if(nTransition_polarity) NextState->nVarIsTrueTransition = infBDD->pState; //The transition is True
				else NextState->nVarIsFalseTransition = infBDD->pState; //The transition is False
				NextState = (SmurfStateEntry *)SimpleSmurfProblemState->vSmurfStatesTableTail;
				SimpleSmurfProblemState->nNumSmurfStateEntries++;
				SimpleSmurfProblemState->vSmurfStatesTableTail = (void *)(NextState + 1);
				NextState->nTransitionVar = pTransition->positiveInferences.arrElts[infIdx];
				NextState->nVarIsAnInference = 1;
				nTransition_polarity = 1;
				infBDD = set_variable_noflag(infBDD, arrSolver2IteVarMap[NextState->nTransitionVar], nTransition_polarity);
			}
			//Follow Negative inferences
			for(int infIdx = 0; infIdx < pTransition->negativeInferences.nNumElts && infBDD->pState == NULL; infIdx++) {
				//Add a SmurfStateEntry into the table
				if(infBDD->pState != NULL) break;
				infBDD->pState = SimpleSmurfProblemState->vSmurfStatesTableTail;
				if(nTransition_polarity) NextState->nVarIsTrueTransition = infBDD->pState; //The transition is True
				else NextState->nVarIsFalseTransition = infBDD->pState; //The transition is False
				NextState = (SmurfStateEntry *)SimpleSmurfProblemState->vSmurfStatesTableTail;
				SimpleSmurfProblemState->nNumSmurfStateEntries++;
				SimpleSmurfProblemState->vSmurfStatesTableTail = (void *)(NextState + 1);
				NextState->nTransitionVar = pTransition->negativeInferences.arrElts[infIdx];
				NextState->nVarIsAnInference = -1;
				nTransition_polarity = 0;
				infBDD = set_variable_noflag(infBDD, arrSolver2IteVarMap[NextState->nTransitionVar], nTransition_polarity);
			}
			if(infBDD->pState != NULL) {
				if(nTransition_polarity) NextState->nVarIsTrueTransition = infBDD->pState; //The transition is True
				else NextState->nVarIsFalseTransition = infBDD->pState; //The transition is False
			} else {
				assert(pTransition->pNextState->pFunc == infBDD);
				if(nTransition_polarity) NextState->nVarIsTrueTransition = (SmurfStateEntry *)SimpleSmurfProblemState->vSmurfStatesTableTail; //The transition is True
				else NextState->nVarIsFalseTransition = (SmurfStateEntry *)SimpleSmurfProblemState->vSmurfStatesTableTail; //The transition is False

				//Recurse on nTransitionVar == False transition
				ReadSmurfStateIntoTable(pTransition->pNextState);
			}
			
			//Compute the SmurfState w/ nTransitionVar = True
			//Create Inference Transitions
			infBDD = pCurrentState->pFunc;
			nTransition_polarity = 1;
			infBDD = set_variable_noflag(infBDD, arrSolver2IteVarMap[CurrState->nTransitionVar], nTransition_polarity);
			pTransition = &(pCurrentState->arrTransitions[2*nVbleIndex+1]);
			NextState = CurrState;
			//Follow Negative inferences
			for(int infIdx = 0; infIdx < pTransition->negativeInferences.nNumElts && infBDD->pState == NULL; infIdx++) {
				//Add a SmurfStateEntry into the table
				if(infBDD->pState != NULL) break;
				infBDD->pState = SimpleSmurfProblemState->vSmurfStatesTableTail;
				if(nTransition_polarity) NextState->nVarIsTrueTransition = infBDD->pState; //The transition is True
				else NextState->nVarIsFalseTransition = infBDD->pState; //The transition is False
				NextState = (SmurfStateEntry *)SimpleSmurfProblemState->vSmurfStatesTableTail;
				SimpleSmurfProblemState->nNumSmurfStateEntries++;
				SimpleSmurfProblemState->vSmurfStatesTableTail = (void *)(NextState + 1);
				NextState->nTransitionVar = pTransition->negativeInferences.arrElts[infIdx];
				NextState->nVarIsAnInference = -1;
				nTransition_polarity = 0;
				infBDD = set_variable_noflag(infBDD, arrSolver2IteVarMap[NextState->nTransitionVar], nTransition_polarity);
			}
			//Follow Positive inferences
			for(int infIdx = 0; infIdx < pTransition->positiveInferences.nNumElts && infBDD->pState == NULL; infIdx++) {
				//Add a SmurfStateEntry into the table
				if(infBDD->pState != NULL) break;
				infBDD->pState = SimpleSmurfProblemState->vSmurfStatesTableTail;
				if(nTransition_polarity) NextState->nVarIsTrueTransition = infBDD->pState; //The transition is True
				else NextState->nVarIsFalseTransition = infBDD->pState; //The transition is False
				NextState = (SmurfStateEntry *)SimpleSmurfProblemState->vSmurfStatesTableTail;
				SimpleSmurfProblemState->nNumSmurfStateEntries++;
				SimpleSmurfProblemState->vSmurfStatesTableTail = (void *)(NextState + 1);
				NextState->nTransitionVar = pTransition->positiveInferences.arrElts[infIdx];
				NextState->nVarIsAnInference = 1;
				nTransition_polarity = 1;
				infBDD = set_variable_noflag(infBDD, arrSolver2IteVarMap[NextState->nTransitionVar], nTransition_polarity);
			}
			if(infBDD->pState != NULL) {
				if(nTransition_polarity) NextState->nVarIsTrueTransition = infBDD->pState; //The transition is True
				else NextState->nVarIsFalseTransition = infBDD->pState; //The transition is False
			} else {
				assert(pTransition->pNextState->pFunc == infBDD);
				if(nTransition_polarity) NextState->nVarIsTrueTransition = (SmurfStateEntry *)SimpleSmurfProblemState->vSmurfStatesTableTail; //The transition is True
				else NextState->nVarIsFalseTransition = (SmurfStateEntry *)SimpleSmurfProblemState->vSmurfStatesTableTail; //The transition is False

				//Recurse on nTransitionVar == False transition
				ReadSmurfStateIntoTable(pTransition->pNextState);
			}
		}
	}
}

void ReadAllSmurfsIntoTable() {
	//Create the TrueSimpleSmurfState entry
	SimpleSmurfProblemState = (ProblemState *)ite_calloc(1, sizeof(ProblemState), 9, "SimpleSmurfProblemState");
	SimpleSmurfProblemState->nNumSmurfs = nmbrFunctions;
	SimpleSmurfProblemState->nNumVars = nNumVariables;
	SimpleSmurfProblemState->nNumSmurfStateEntries = 2;
	ite_counters[SMURF_STATES] = DetermineNumOfSmurfStates();
	SimpleSmurfProblemState->arrSmurfStatesTable = (void **)ite_calloc(ite_counters[SMURF_STATES]+3, sizeof(SmurfStateEntry), 9, "arrSmurfStatesTable");
	SimpleSmurfProblemState->arrSmurfStack = (SmurfStack *)ite_calloc(SimpleSmurfProblemState->nNumVars, sizeof(SmurfStack), 9, "arrSmurfStack");
	SimpleSmurfProblemState->arrVariableOccursInSmurf = (int **)ite_calloc(SimpleSmurfProblemState->nNumVars, sizeof(int *), 9, "arrVariablesOccursInSmurf");
	SimpleSmurfProblemState->arrPosVarHeurWghts = (double *)ite_calloc(SimpleSmurfProblemState->nNumVars, sizeof(double), 9, "arrPosVarHeurWghts");
	SimpleSmurfProblemState->arrNegVarHeurWghts = (double *)ite_calloc(SimpleSmurfProblemState->nNumVars, sizeof(double), 9, "arrNegVarHeurWghts");
	SimpleSmurfProblemState->arrInferenceQueue = (int *)ite_calloc(SimpleSmurfProblemState->nNumVars, sizeof(int), 9, "arrInferenceQueue");
	SimpleSmurfProblemState->arrInferenceDeclaredAtLevel = (int *)ite_calloc(SimpleSmurfProblemState->nNumVars, sizeof(int), 9, "arrInferenceDeclaredAtLevel");
	
	//Count the number of functions every variable occurs in.
	int *temp_varcount = (int *)ite_calloc(SimpleSmurfProblemState->nNumVars, sizeof(int), 9, "temp_varcount");
	for(int x = 0; x < SimpleSmurfProblemState->nNumSmurfs; x++)
	  for(int y = 0; y < arrSolverFunctions[x].fn_smurf.pInitialState->vbles.nNumElts; y++)
		 temp_varcount[arrSolverFunctions[x].fn_smurf.pInitialState->vbles.arrElts[y]]++;
	
	for(int x = 0; x < SimpleSmurfProblemState->nNumVars; x++) {
		SimpleSmurfProblemState->arrSmurfStack[x].arrSmurfStates
		  = (void **)ite_calloc(SimpleSmurfProblemState->nNumSmurfs, sizeof(int), 9, "arrSmurfStates");
		SimpleSmurfProblemState->arrVariableOccursInSmurf[x] = (int *)ite_calloc(temp_varcount[x]+1, sizeof(int), 9, "arrVariableOccursInSmurf[x]");
		SimpleSmurfProblemState->arrVariableOccursInSmurf[x][temp_varcount[x]] = -1;
		SimpleSmurfProblemState->arrInferenceDeclaredAtLevel[x] = SimpleSmurfProblemState->nNumVars;
		temp_varcount[x] = 0;
	}
	
	//Fill in the variable occurance arrays for each function
	for(int x = 0; x < SimpleSmurfProblemState->nNumSmurfs; x++) {
		for(int y = 0; y < arrSolverFunctions[x].fn_smurf.pInitialState->vbles.nNumElts; y++) {
			int nVar = arrSolverFunctions[x].fn_smurf.pInitialState->vbles.arrElts[y];
			SimpleSmurfProblemState->arrVariableOccursInSmurf[nVar][temp_varcount[nVar]] = x;
			temp_varcount[nVar]++;
		}
	}
	
	ite_free((void **)&temp_varcount);
	
	//arrSmurfStatesTable[1] is reserved for the TrueSimpleSmurfState
	TrueSimpleSmurfState = (SmurfStateEntry *)SimpleSmurfProblemState->arrSmurfStatesTable;
	TrueSimpleSmurfState->nTransitionVar = 0;
	TrueSimpleSmurfState->nVarIsTrueTransition = (void *)TrueSimpleSmurfState;
	TrueSimpleSmurfState->nVarIsFalseTransition = (void *)TrueSimpleSmurfState;
	TrueSimpleSmurfState->nHeurWghtofTrueTransition = 0;
	TrueSimpleSmurfState->nHeurWghtofFalseTransition = 0;
	TrueSimpleSmurfState->cVarIsSafe = 0;
	TrueSimpleSmurfState->nVarIsAnInference = 0;
	TrueSimpleSmurfState->nNextVarInThisStateGT = NULL;
	TrueSimpleSmurfState->nNextVarInThisStateLT = NULL;
	TrueSimpleSmurfState->nNextVarInThisState = NULL;

	SimpleSmurfProblemState->vSmurfStatesTableTail = (void *)(TrueSimpleSmurfState + 1);
	
	clear_all_bdd_flags();
	clear_all_bdd_pState();
	//true_ptr->flag = 1; //Necessary???
	true_ptr->pState = TrueSimpleSmurfState;
	
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
		SmurfState *pInitialState = arrSolverFunctions[nSmurfIndex].fn_smurf.pInitialState;
		if(pInitialState->pFunc->pState != NULL && smurfs_share_paths) {
			SimpleSmurfProblemState->arrSmurfStack[0].arrSmurfStates[nSmurfIndex] = pInitialState->pFunc->pState;
		} else {
			SimpleSmurfProblemState->arrSmurfStack[0].arrSmurfStates[nSmurfIndex] = SimpleSmurfProblemState->vSmurfStatesTableTail;
			LSGBSmurfSetHeurScores(nSmurfIndex, pInitialState);
			if(!smurfs_share_paths) { clear_all_bdd_flags(); clear_all_bdd_pState(); true_ptr->pState = TrueSimpleSmurfState; }
			ReadSmurfStateIntoTable(pInitialState);
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
	assert(SimpleSmurfProblemState->nNumSmurfStateEntries == ite_counters[SMURF_STATES]+2);
}

ITE_INLINE
void Calculate_Heuristic_Values() {

	memset(SimpleSmurfProblemState->arrPosVarHeurWghts, 0, sizeof(double)*SimpleSmurfProblemState->nNumVars);
	memset(SimpleSmurfProblemState->arrNegVarHeurWghts, 0, sizeof(double)*SimpleSmurfProblemState->nNumVars);
	
	void **arrSmurfStates = SimpleSmurfProblemState->arrSmurfStack[SimpleSmurfProblemState->nCurrSearchTreeLevel].arrSmurfStates;
	for(int nSmurfIndex = 0; nSmurfIndex < SimpleSmurfProblemState->nNumSmurfs; nSmurfIndex++) {
		if(arrSmurfStates[nSmurfIndex] == TrueSimpleSmurfState) continue;
		SmurfStateEntry *pSmurfState = (SmurfStateEntry *)arrSmurfStates[nSmurfIndex];
		SimpleSmurfProblemState->arrPosVarHeurWghts[pSmurfState->nTransitionVar] += 
		  pSmurfState->nHeurWghtofTrueTransition;
		SimpleSmurfProblemState->arrNegVarHeurWghts[pSmurfState->nTransitionVar] +=  
		  pSmurfState->nHeurWghtofFalseTransition;
		while (pSmurfState->nNextVarInThisState != NULL) {
			pSmurfState = (SmurfStateEntry *)pSmurfState->nNextVarInThisState;
			SimpleSmurfProblemState->arrPosVarHeurWghts[pSmurfState->nTransitionVar] += 
			  pSmurfState->nHeurWghtofTrueTransition;
			SimpleSmurfProblemState->arrNegVarHeurWghts[pSmurfState->nTransitionVar] +=  
			  pSmurfState->nHeurWghtofFalseTransition;
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
int ApplyInferenceToSmurfs(int nBranchVar, bool bBVPolarity) {
	void **arrSmurfStates = SimpleSmurfProblemState->arrSmurfStack[SimpleSmurfProblemState->nCurrSearchTreeLevel].arrSmurfStates;
	d7_printf2("  Transitioning Smurfs using %d\n", bBVPolarity?nBranchVar:-nBranchVar);
	for(int i = 0; SimpleSmurfProblemState->arrVariableOccursInSmurf[nBranchVar][i] != -1; i++) {
		int nSmurfNumber = SimpleSmurfProblemState->arrVariableOccursInSmurf[nBranchVar][i];
		d7_printf3("    Checking Smurf %d (State %d)\n", nSmurfNumber, arrSmurfStates[nSmurfNumber]);
		if(arrSmurfStates[nSmurfNumber] == TrueSimpleSmurfState) continue;
		SmurfStateEntry *pSmurfState = (SmurfStateEntry *)arrSmurfStates[nSmurfNumber];
		do {
			if(pSmurfState->nTransitionVar < nBranchVar)
			  pSmurfState = (SmurfStateEntry *)pSmurfState->nNextVarInThisStateGT;
			else if(pSmurfState->nTransitionVar > nBranchVar)
			  pSmurfState = (SmurfStateEntry *)pSmurfState->nNextVarInThisStateLT;
			else { //(pSmurfState->nTransitionVar == nBranchVar) {
				//Follow this transition and apply all inferences found.
				pSmurfState = (SmurfStateEntry *)(bBVPolarity?pSmurfState->nVarIsTrueTransition:pSmurfState->nVarIsFalseTransition);
				while(pSmurfState->nVarIsAnInference != 0) {
					int nInfVar = pSmurfState->nTransitionVar;
					bool bInfPolarity = pSmurfState->nVarIsAnInference > 0;
					
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
					
					//Follow the transtion to the next SmurfState
					pSmurfState = (SmurfStateEntry *)(bInfPolarity?pSmurfState->nVarIsTrueTransition:pSmurfState->nVarIsFalseTransition);
				}
				
				//Record the transition.
				arrSmurfStates[nSmurfNumber] = pSmurfState;
				
				break;
			}
		} while (pSmurfState != NULL);
		d7_printf3("      Smurf %d transitioned to state %x\n", nSmurfNumber, arrSmurfStates[nSmurfNumber]);
	}
	return 1;
}

int Init_SimpleSmurfSolver() {
	//Clear all FunctionTypes
	for(int x = 0; x < nmbrFunctions; x++)
	  functionType[x] = UNSURE;
	
	InitVarMap();
	InitVariables();
	InitFunctions();
	/* Convert bdds to smurfs */
	int ret = CreateFunctions();
	if (ret != SOLV_UNKNOWN) return ret;
	
	ReadAllSmurfsIntoTable();
	
	FreeFunctions();
	FreeVariables();
	//FreeVarMap(); //This is freed at the end of simpleSolve(). 
	                //arrVarChoiceLevels and arrSolver2IteVarMap are needed during search.
	
	return ret;
}

ITE_INLINE
void SmurfStates_Push() {
	
//	memcpy_ite(SimpleSmurfProblemState->arrSmurfStack[SimpleSmurfProblemState->nCurrSearchTreeLevel+1].arrSmurfStates,
//				  SimpleSmurfProblemState->arrSmurfStack[SimpleSmurfProblemState->nCurrSearchTreeLevel].arrSmurfStates,
//				  SimpleSmurfProblemState->nNumSmurfs*sizeof(int));
	for(int i = 0; i < SimpleSmurfProblemState->nNumSmurfs; i++) {
		SimpleSmurfProblemState->arrSmurfStack[SimpleSmurfProblemState->nCurrSearchTreeLevel+1].arrSmurfStates[i] = 
		  SimpleSmurfProblemState->arrSmurfStack[SimpleSmurfProblemState->nCurrSearchTreeLevel].arrSmurfStates[i];
	}
	
	SimpleSmurfProblemState->arrSmurfStack[SimpleSmurfProblemState->nCurrSearchTreeLevel+1].nNumFreeVars =
	  SimpleSmurfProblemState->arrSmurfStack[SimpleSmurfProblemState->nCurrSearchTreeLevel].nNumFreeVars;

	SimpleSmurfProblemState->arrSmurfStack[SimpleSmurfProblemState->nCurrSearchTreeLevel+1].nVarChoiceCurrLevel =
	  SimpleSmurfProblemState->arrSmurfStack[SimpleSmurfProblemState->nCurrSearchTreeLevel].nVarChoiceCurrLevel;
	
	SimpleSmurfProblemState->arrSmurfStack[SimpleSmurfProblemState->nCurrSearchTreeLevel+1].nHeuristicPlaceholder =
	  SimpleSmurfProblemState->arrSmurfStack[SimpleSmurfProblemState->nCurrSearchTreeLevel].nHeuristicPlaceholder;
	
	SimpleSmurfProblemState->nCurrSearchTreeLevel++;
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
			//if(ite_counters[NUM_CHOICE_POINTS] %128 == 64)
			//if(SimpleSmurfProblemState->nCurrSearchTreeLevel > 10)
			//nBranchLit = Simple_DC_Heuristic(); //Don't Care - Choose the first unset variable found.
			//else 
			  nBranchLit = Simple_LSGB_Heuristic();

			ite_counters[NUM_CHOICE_POINTS]++;
			ite_counters[NO_ERROR]++;
			
			//Push stack
			SmurfStates_Push();
			
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
				if(ApplyInferenceToSmurfs(nBranchVar, bBVPolarity) == 0) {
					//A conflict occured
					
					switch(backtrack()) {
					 case SOLV_UNSAT: return SOLV_UNSAT;
					 case SOLV_SAT: return SOLV_SAT;
					 default: break;
					}
					
find_more_solutions: ;
					
					nBranchLit = SimpleSmurfProblemState->arrInferenceQueue[SimpleSmurfProblemState->arrSmurfStack[SimpleSmurfProblemState->nCurrSearchTreeLevel].nNumFreeVars];
					SimpleSmurfProblemState->arrInferenceDeclaredAtLevel[abs(nBranchLit)] = -SimpleSmurfProblemState->nCurrSearchTreeLevel; //Negate this value to flag variable as an old choicepoint
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
	int bxors_old = BREAK_XORS;
	BREAK_XORS = 0;
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
	BREAK_XORS = bxors_old;
	FreeVarMap();
	return ret;
	//Hmmm I didn't free ANY of that memory. SEAN!!! FIX THIS!!!
}

