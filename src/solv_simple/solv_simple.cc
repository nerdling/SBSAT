#include "sbsat.h"
#include "sbsat_solver.h"
#include "solver.h"
#include <omp.h>

int nSimpleSolver_Reset=0;
int nInfQueueStart=0;
int solver_polarity_presets_count=0;
int simple_solver_reset_level=-1;
int add_one_display=0;
int solutions_overflow=0;

ITE_INLINE
void save_solution_simple(void) {
	d7_printf1("      Solution found\n");

	if (result_display_type) {
		ite_counters[NUM_SOLUTIONS]++;
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

		int nCurrInfLevel = SimpleSmurfProblemState->arrSmurfStack[SimpleSmurfProblemState->nCurrSearchTreeLevel].nNumFreeVars;
		
		for (int i = 0; i<SimpleSmurfProblemState->nNumVars; i++) {
			if(SimpleSmurfProblemState->arrInferenceDeclaredAtLevel[i] >= nCurrInfLevel)
			  tmp_solution_info->arrElts[arrSimpleSolver2IteVarMap[abs(SimpleSmurfProblemState->arrInferenceQueue[i])]] = BOOL_UNKNOWN;
			else
			  tmp_solution_info->arrElts[arrSimpleSolver2IteVarMap[abs(SimpleSmurfProblemState->arrInferenceQueue[i])]] = (SimpleSmurfProblemState->arrInferenceQueue[i]>0)?BOOL_TRUE:BOOL_FALSE;
		}
	} else {
		int num_vars = 0;
		LONG64 save_sol = ite_counters[NUM_SOLUTIONS];

		//Handle nForceBackjumpLevel
		if (arrVarChoiceLevels && nForceBackjumpLevel>=0 && nForceBackjumpLevel<nVarChoiceLevelsNum) {
			int nCurrInfLevel = SimpleSmurfProblemState->arrSmurfStack[SimpleSmurfProblemState->nCurrSearchTreeLevel].nNumFreeVars;
			int level=SimpleSmurfProblemState->arrSmurfStack[SimpleSmurfProblemState->nCurrSearchTreeLevel].nVarChoiceCurrLevel;
			for(;level<nVarChoiceLevelsNum;level++) {
				if(nForceBackjumpLevel < level) break;
				int j=SimpleSmurfProblemState->arrSmurfStack[SimpleSmurfProblemState->nCurrSearchTreeLevel].nHeuristicPlaceholder;
				while(arrVarChoiceLevels[level][j] != 0) {
					int i=arrVarChoiceLevels[level][j];
					if(SimpleSmurfProblemState->arrInferenceDeclaredAtLevel[i] >= nCurrInfLevel) {
						//double # of solutions
						num_vars++;
					}
					j++;
				}
			}
			ite_counters[NUM_SOLUTIONS]+=((LONG64)1)<<((LONG64)(num_vars));
		} else {
			ite_counters[NUM_SOLUTIONS]+=
			  ((LONG64)1)<<((LONG64)(SimpleSmurfProblemState->nNumVars-1 - SimpleSmurfProblemState->arrSmurfStack[SimpleSmurfProblemState->nCurrSearchTreeLevel].nNumFreeVars));
			
			num_vars = SimpleSmurfProblemState->nNumVars-1 - SimpleSmurfProblemState->arrSmurfStack[SimpleSmurfProblemState->nCurrSearchTreeLevel].nNumFreeVars;
		}

		if((num_vars > 63)
			|| ite_counters[NUM_SOLUTIONS]<(LONG64)0
			|| ite_counters[NUM_SOLUTIONS]<(LONG64)save_sol
			) {
			solutions_overflow=1;
			d7_printf1("Number of solutions is >= 2^64\n");
		}
	}
}


ITE_INLINE
void Calculate_Heuristic_Values() {

	memset(SimpleSmurfProblemState->arrPosVarHeurWghts, 0, sizeof(double)*SimpleSmurfProblemState->nNumVars);
	memset(SimpleSmurfProblemState->arrNegVarHeurWghts, 0, sizeof(double)*SimpleSmurfProblemState->nNumVars);
	
	void **arrSmurfStates = SimpleSmurfProblemState->arrSmurfStack[SimpleSmurfProblemState->nCurrSearchTreeLevel].arrSmurfStates;

//#pragma omp parallel num_threads(2)
	  {

//#pragma omp for schedule(static, 1)
	for(int nSmurfIndex = 0; nSmurfIndex < SimpleSmurfProblemState->nNumSmurfs; nSmurfIndex++) {
		//TrueState
		if(arrSmurfStates[nSmurfIndex] == pTrueSimpleSmurfState) continue;
		void *pState;
		int nCurrInfLevel = SimpleSmurfProblemState->arrSmurfStack[SimpleSmurfProblemState->nCurrSearchTreeLevel].nNumFreeVars;
		int numfound = 0;
		//FN_SMURF
		switch(((TypeStateEntry *)arrSmurfStates[nSmurfIndex])->cType) {
		 case FN_SMURF:
			pState = arrSmurfStates[nSmurfIndex];
//#pragma omp atomic
			SimpleSmurfProblemState->arrPosVarHeurWghts[((SmurfStateEntry *)pState)->nTransitionVar] +=
			  ((SmurfStateEntry *)pState)->fHeurWghtofTrueTransition;
//#pragma omp atomic
			SimpleSmurfProblemState->arrNegVarHeurWghts[((SmurfStateEntry *)pState)->nTransitionVar] +=  
			  ((SmurfStateEntry *)pState)->fHeurWghtofFalseTransition;
			while (((SmurfStateEntry *)pState)->pNextVarInThisState != NULL) {
				pState = ((SmurfStateEntry *)pState)->pNextVarInThisState;
//#pragma omp atomic
				SimpleSmurfProblemState->arrPosVarHeurWghts[((SmurfStateEntry *)pState)->nTransitionVar] +=
				  ((SmurfStateEntry *)pState)->fHeurWghtofTrueTransition;
//#pragma omp atomic
				SimpleSmurfProblemState->arrNegVarHeurWghts[((SmurfStateEntry *)pState)->nTransitionVar] +=  
				  ((SmurfStateEntry *)pState)->fHeurWghtofFalseTransition;
			}
			break;
		 case FN_OR_COUNTER:
			pState = arrSmurfStates[nSmurfIndex];
			for(int index = 0; numfound < ((ORCounterStateEntry *)pState)->nSize; index++) {
				if(SimpleSmurfProblemState->arrInferenceDeclaredAtLevel[((ORCounterStateEntry *)pState)->pORState->pnTransitionVars[index]] >= nCurrInfLevel) {
					numfound++;
					if(((ORCounterStateEntry *)pState)->pORState->bPolarity[index] == BOOL_TRUE) {
//#pragma omp atomic
						SimpleSmurfProblemState->arrPosVarHeurWghts[((ORCounterStateEntry *)pState)->pORState->pnTransitionVars[index]] += JHEURISTIC_K_TRUE;
//#pragma omp atomic
						SimpleSmurfProblemState->arrNegVarHeurWghts[((ORCounterStateEntry *)pState)->pORState->pnTransitionVars[index]] += LSGBORCounterGetHeurNeg(((ORCounterStateEntry *)pState));
					} else {
//#pragma omp atomic
						SimpleSmurfProblemState->arrPosVarHeurWghts[((ORCounterStateEntry *)pState)->pORState->pnTransitionVars[index]] += LSGBORCounterGetHeurNeg(((ORCounterStateEntry *)pState));
//#pragma omp atomic
						SimpleSmurfProblemState->arrNegVarHeurWghts[((ORCounterStateEntry *)pState)->pORState->pnTransitionVars[index]] += JHEURISTIC_K_TRUE;
					}
				}
			}
			break;
		 case FN_XOR_COUNTER:
			pState = arrSmurfStates[nSmurfIndex];
			for(int index = 0; numfound < ((XORCounterStateEntry *)pState)->nSize; index++) {
				if(SimpleSmurfProblemState->arrInferenceDeclaredAtLevel[((XORCounterStateEntry *)pState)->pXORState->pnTransitionVars[index]] >= nCurrInfLevel) {
					numfound++;
//#pragma omp atomic
					SimpleSmurfProblemState->arrPosVarHeurWghts[((XORCounterStateEntry *)pState)->pXORState->pnTransitionVars[index]] += LSGBXORCounterGetHeurScoreTrans(((XORCounterStateEntry *)pState));
//#pragma omp atomic
					SimpleSmurfProblemState->arrNegVarHeurWghts[((XORCounterStateEntry *)pState)->pXORState->pnTransitionVars[index]] += LSGBXORCounterGetHeurScoreTrans(((XORCounterStateEntry *)pState));
				}
			}
			break;
		 case FN_OR:
			pState = arrSmurfStates[nSmurfIndex];
			for(int index = 0; numfound < 2; index++) {
				if(SimpleSmurfProblemState->arrInferenceDeclaredAtLevel[((ORStateEntry *)pState)->pnTransitionVars[index]] >= nCurrInfLevel) {
					numfound++;
					if(((ORStateEntry *)pState)->bPolarity[index] == BOOL_TRUE) {
//#pragma omp atomic
						SimpleSmurfProblemState->arrPosVarHeurWghts[((ORStateEntry *)pState)->pnTransitionVars[index]] += JHEURISTIC_K_TRUE;
//#pragma omp atomic
						SimpleSmurfProblemState->arrNegVarHeurWghts[((ORStateEntry *)pState)->pnTransitionVars[index]] += LSGBORGetHeurNeg(((ORStateEntry *)pState));
					} else {
//#pragma omp atomic
						SimpleSmurfProblemState->arrPosVarHeurWghts[((ORStateEntry *)pState)->pnTransitionVars[index]] += LSGBORGetHeurNeg(((ORStateEntry *)pState));
//#pragma omp atomic
						SimpleSmurfProblemState->arrNegVarHeurWghts[((ORStateEntry *)pState)->pnTransitionVars[index]] += JHEURISTIC_K_TRUE;
					}
				}
			}
			break;
		 case FN_XOR:
			pState = arrSmurfStates[nSmurfIndex];
			for(int index = 0; numfound < 2; index++) {
				if(SimpleSmurfProblemState->arrInferenceDeclaredAtLevel[((XORStateEntry *)pState)->pnTransitionVars[index]] >= nCurrInfLevel) {
					numfound++;
//#pragma omp atomic
					SimpleSmurfProblemState->arrPosVarHeurWghts[((XORStateEntry *)pState)->pnTransitionVars[index]] += LSGBXORGetHeurScoreTrans(((XORStateEntry *)pState));
//#pragma omp atomic
					SimpleSmurfProblemState->arrNegVarHeurWghts[((XORStateEntry *)pState)->pnTransitionVars[index]] += LSGBXORGetHeurScoreTrans(((XORStateEntry *)pState));
				}
			}
			break;
		 case FN_XOR_GELIM:
			pState = arrSmurfStates[nSmurfIndex];
			for(int index = 0; index < ((XORGElimStateEntry *)pState)->nSize; index++) {
				if(SimpleSmurfProblemState->arrInferenceDeclaredAtLevel[((XORGElimStateEntry *)pState)->pnTransitionVars[index]] >= nCurrInfLevel) {
					numfound++;
				}
			}
			if(numfound == 0) arrSmurfStates[nSmurfIndex] = pTrueSimpleSmurfState;
			else {
				for(int index = 0; index < ((XORGElimStateEntry *)pState)->nSize; index++) {				  
					if(SimpleSmurfProblemState->arrInferenceDeclaredAtLevel[((XORGElimStateEntry *)pState)->pnTransitionVars[index]] >= nCurrInfLevel) {
//#pragma omp atomic
						SimpleSmurfProblemState->arrPosVarHeurWghts[((XORGElimStateEntry *)pState)->pnTransitionVars[index]] += LSGBarrXORWeightTrans(numfound);
//#pragma omp atomic
						SimpleSmurfProblemState->arrNegVarHeurWghts[((XORGElimStateEntry *)pState)->pnTransitionVars[index]] += LSGBarrXORWeightTrans(numfound);
					}
				}
			}
			break;
		 default: break;
		}
	}
   } //end omp pragma

	Calculate_Heuristic_Values_Hooks();
	
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
						if(fVbleWeight >= fMaxWeight) {
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
	
	// Search through the remaining uninstantiated variables.
	for(int i = nBestVble + 1; i < SimpleSmurfProblemState->nNumVars; i++) {
		if(SimpleSmurfProblemState->arrInferenceDeclaredAtLevel[i] >= nCurrInfLevel) {
			fVbleWeight = (1+SimpleSmurfProblemState->arrPosVarHeurWghts[i]) *
			  (1+SimpleSmurfProblemState->arrNegVarHeurWghts[i]);
			if(fVbleWeight >= fMaxWeight) {
				fMaxWeight = fVbleWeight;
				nBestVble = i;
			}
		}
	}
	
   if(nBestVble == 0) {
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

int EnqueueInference(int nInfVar, bool bInfPolarity) {
	//Try to insert inference into the inference Queue
	int nInfQueueHead = SimpleSmurfProblemState->arrSmurfStack[SimpleSmurfProblemState->nCurrSearchTreeLevel].nNumFreeVars;
	int nPrevInfLevel = /*abs(*/SimpleSmurfProblemState->arrInferenceDeclaredAtLevel[nInfVar];
	//Sure, nPrevInfLevel could be zero, but only if it was a choicepoint and 
	//I think it's impossible for a prior choicepoint to be inferred here.
	d7_printf4("      Inferring %d at Level %d (prior level = %d)\n",
				  bInfPolarity?nInfVar:-nInfVar, nInfQueueHead, nPrevInfLevel);
	assert(nPrevInfLevel > 0);
	
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
int ApplyInferenceToStates(int nBranchVar, bool bBVPolarity) {
	int ret = ApplyInference_Hooks(nBranchVar, bBVPolarity);
	if(ret == 0) return 0;
	
	void **arrSmurfStates = SimpleSmurfProblemState->arrSmurfStack[SimpleSmurfProblemState->nCurrSearchTreeLevel].arrSmurfStates;
	d7_printf2("  Transitioning Smurfs using %d\n", bBVPolarity?nBranchVar:-nBranchVar);
	for(int i = SimpleSmurfProblemState->arrVariableOccursInSmurf[nBranchVar][0]; i > 0; i--) {
		//SEAN IDEA: Clauses are watched on literals, so only ~half of clauses are checked each time.
		int nSmurfNumber = SimpleSmurfProblemState->arrVariableOccursInSmurf[nBranchVar][i];
		d7_printf3("    Checking Smurf %d (State %x)\n", nSmurfNumber, arrSmurfStates[nSmurfNumber]);
		if ((nSmurfNumber >> 31) == 1) continue; //Skip Non-Watched Variables
		//if ((nSmurfNumber & 0x40000000) == 0x40000000) continue;
		if(arrSmurfStates[nSmurfNumber] == pTrueSimpleSmurfState) continue;
		ret = ((TypeStateEntry *)arrSmurfStates[nSmurfNumber])->ApplyInferenceToState(nBranchVar, bBVPolarity, nSmurfNumber, arrSmurfStates);
		if(ret == 0) return 0;
	}
	return ret;
}

ITE_INLINE
void SmurfStates_Push(int destination) {

	//destination=nCurrSearchTreeLevel+1, except in the case of a nSimpleSolver_Reset then destination=0
	if(SimpleSmurfProblemState->arrSmurfStack[destination].arrSmurfStates == NULL) {
		Alloc_SmurfStack(destination);
	}

	memcpy_ite(SimpleSmurfProblemState->arrSmurfStack[destination].arrSmurfStates,
				  SimpleSmurfProblemState->arrSmurfStack[SimpleSmurfProblemState->nCurrSearchTreeLevel].arrSmurfStates,
				  SimpleSmurfProblemState->nNumSmurfs*sizeof(void *));

//	for(int i = 0; i < SimpleSmurfProblemState->nNumSmurfs; i++) {
//		SimpleSmurfProblemState->arrSmurfStack[destination].arrSmurfStates[i] = 
//		  SimpleSmurfProblemState->arrSmurfStack[SimpleSmurfProblemState->nCurrSearchTreeLevel].arrSmurfStates[i];
//	}

	SimpleSmurfProblemState->arrSmurfStack[destination].nVarChoiceCurrLevel =
	  SimpleSmurfProblemState->arrSmurfStack[SimpleSmurfProblemState->nCurrSearchTreeLevel].nVarChoiceCurrLevel;

	SimpleSmurfProblemState->arrSmurfStack[destination].nNumSmurfsSatisfied =
	  SimpleSmurfProblemState->arrSmurfStack[SimpleSmurfProblemState->nCurrSearchTreeLevel].nNumSmurfsSatisfied;

	SimpleSmurfProblemState->arrSmurfStack[destination].nNumFreeVars =
	  SimpleSmurfProblemState->arrSmurfStack[SimpleSmurfProblemState->nCurrSearchTreeLevel].nNumFreeVars;

	SimpleSmurfProblemState->arrSmurfStack[destination].nHeuristicPlaceholder =
	  SimpleSmurfProblemState->arrSmurfStack[SimpleSmurfProblemState->nCurrSearchTreeLevel].nHeuristicPlaceholder;

	SimpleSmurfProblemState->arrSmurfStack[destination].nWatchedListStackTop =
	  SimpleSmurfProblemState->arrSmurfStack[SimpleSmurfProblemState->nCurrSearchTreeLevel].nWatchedListStackTop;
	
	SmurfStates_Push_Hooks(SimpleSmurfProblemState->nCurrSearchTreeLevel, destination);
	
	SimpleSmurfProblemState->nCurrSearchTreeLevel = destination;
}

ITE_INLINE
int SmurfStates_Pop() {
	SimpleSmurfProblemState->nCurrSearchTreeLevel--;
	
	if(SimpleSmurfProblemState->nCurrSearchTreeLevel < 0) {
		if(ite_counters[NUM_SOLUTIONS] == 0) return SOLV_UNSAT; //return Unsatisifable
		else return SOLV_SAT;
	}

	SmurfStates_Pop_Hooks();
	
	return SOLV_UNKNOWN;
}

ITE_INLINE
int Backtrack() {
	//Pop stack
	ite_counters[ERR_BT_SMURF]++;
	ite_counters[NUM_BACKTRACKS]++;

	d7_printf1("  Conflict - Backtracking\n");

	d9_printf2("\nStarting backtrack # %ld\n", (long)ite_counters[NUM_BACKTRACKS]);

	int ret = Backtrack_Hooks();
	
	int nInfQueueTail = SimpleSmurfProblemState->arrSmurfStack[SimpleSmurfProblemState->nCurrSearchTreeLevel].nNumFreeVars;	  
	int nWatchedListStackTail = SimpleSmurfProblemState->arrSmurfStack[SimpleSmurfProblemState->nCurrSearchTreeLevel].nWatchedListStackTop;
	
	int nPop = SmurfStates_Pop();
	if(nPop != SOLV_UNKNOWN) return nPop;

	int nInfQueueHead = SimpleSmurfProblemState->arrSmurfStack[SimpleSmurfProblemState->nCurrSearchTreeLevel].nNumFreeVars;
	int nWatchedListStackHead = SimpleSmurfProblemState->arrSmurfStack[SimpleSmurfProblemState->nCurrSearchTreeLevel].nWatchedListStackTop;
	
	//Empty the inference queue & reverse polarity of cp var
	//Clear Inference Queue

	//Using the inference queue to avoid a linear check over all variables.
	for(int i = nInfQueueHead; i < nInfQueueTail; i++) {
		int nBranchLit = abs(SimpleSmurfProblemState->arrInferenceQueue[i]);
		d7_printf3("      Resetting level of variable %d to %d\n", nBranchLit, SimpleSmurfProblemState->nNumVars);
		SimpleSmurfProblemState->arrInferenceDeclaredAtLevel[nBranchLit] = SimpleSmurfProblemState->nNumVars;
	}

	//Repair the watched variable lists
	for(int i = nWatchedListStackHead; i < nWatchedListStackTail; i++) {
		(*SimpleSmurfProblemState->arrWatchedListStack[i]) &= 0x7FFFFFFF; //Unset the top bit.
	}
	
	return ret;
}

int SimpleBrancher() {

	int ret = SOLV_UNKNOWN;
	
	SimpleSmurfProblemState->nCurrSearchTreeLevel = 0;

	fSimpleSolverStartTime = get_runtime();

	int max_solutions_simple = max_solutions>0?max_solutions:(max_solutions==0?-1:0);
	
	while(ite_counters[NUM_SOLUTIONS] != max_solutions_simple) {
		bool bBVPolarity; 
		int nBranchLit, nInfQueueHead, nPrevInfLevel, nBranchVar;
		  
		while(SimpleSmurfProblemState->arrSmurfStack[SimpleSmurfProblemState->nCurrSearchTreeLevel].nNumFreeVars < SimpleSmurfProblemState->nNumVars-1
				&& (result_display_type || SimpleSmurfProblemState->arrSmurfStack[SimpleSmurfProblemState->nCurrSearchTreeLevel].nNumSmurfsSatisfied < SimpleSmurfProblemState->nNumSmurfs)
				) {
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
					switch(Backtrack()) {
					 case SOLV_UNSAT: return SOLV_UNSAT;
					 case SOLV_SAT: return SOLV_SAT;
					 case SOLV_LIMIT: return ret;
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

		ret = SOLV_SAT;
		
		if(ite_counters[NUM_SOLUTIONS] != max_solutions_simple) {
			do {
				if(nForceBackjumpLevel < SimpleSmurfProblemState->arrSmurfStack[SimpleSmurfProblemState->nCurrSearchTreeLevel].nVarChoiceCurrLevel)
				  d7_printf2("Forcing a backjump to level %d\n", nForceBackjumpLevel);
				switch(Backtrack()) {
				 case SOLV_UNSAT: return SOLV_UNSAT;
				 case SOLV_SAT: return SOLV_SAT;
				 case SOLV_LIMIT: return ret;
				 default: break;
				}
			} while(nForceBackjumpLevel < SimpleSmurfProblemState->arrSmurfStack[SimpleSmurfProblemState->nCurrSearchTreeLevel].nVarChoiceCurrLevel);
			goto find_more_solutions;
		}
	}

	return ret;
}


//Main solving function - calls the initialization functions, the brancher, and the cleanup
int simpleSolve() {
	int nForceBackjumpLevel_old = nForceBackjumpLevel;
   if(nForceBackjumpLevel < 0) nForceBackjumpLevel = nVarChoiceLevelsNum+1;
	
	simple_solver_reset_level=-1;
	
	int ret = Init_SimpleSmurfSolver();

	nSimpleSolver_Reset=0;
	nInfQueueStart=0;
	solver_polarity_presets_count=0;
	add_one_display=0;
	solutions_overflow=0;
	
	if(ret != SOLV_UNKNOWN) return ret;
	
	ret = SimpleBrancher();
	
	nForceBackjumpLevel = nForceBackjumpLevel_old;
	
	Final_SimpleSmurfSolver();
	
	return ret;
}
