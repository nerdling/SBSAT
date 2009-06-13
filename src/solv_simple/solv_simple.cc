#include "sbsat.h"
#include "sbsat_solver.h"
#include "solver.h"
//#include <omp.h>

int nSimpleSolver_Reset=0;
int nInfQueueStart=0;
int solver_polarity_presets_count=0;
int simple_solver_reset_level=-1;
int add_one_display=0;
int solutions_overflow=0;
long nNextRestart = 0;
int MAX_RESTARTS = 10;
int nNumRestarts = 0;
int backtrack_level = 0;

ITE_INLINE 
void create_clause_from_SBSAT_solution(int *arrInferenceQueue, SmurfStack *arrSmurfStack,
													int nCurrSearchTreeLevel, Cls **clause, int *lits_max_size) {
	Lit ** p;
	int i;
	
	assert((*clause)->used == 0);
	
	if(nCurrSearchTreeLevel > (*lits_max_size)) {
		(*clause) = (Cls *)realloc((*clause), bytes_clause(nCurrSearchTreeLevel, 0));
		(*lits_max_size) = nCurrSearchTreeLevel;
	}
	
	(*clause)->size = nCurrSearchTreeLevel;
	p = (*clause)->lits;
	for(i = nCurrSearchTreeLevel-1; i >= 0; i--) {
		//    fprintf(stderr, "(%d)", -arrInferenceQueue[arrSmurfStack[i].nNumFreeVars]);
		(*p++) = int2lit(-arrInferenceQueue[arrSmurfStack[i].nNumFreeVars]);
	}
}

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
		tmp_solution_info->nNumElts = (int)numinp+1;//SimpleSmurfProblemState->nNumVars+1;
		tmp_solution_info->arrElts = new int[numinp+1];//new int[SimpleSmurfProblemState->nNumVars+2];

		int nCurrInfLevel = SimpleSmurfProblemState->arrSmurfStack[SimpleSmurfProblemState->nCurrSearchTreeLevel].nNumFreeVars;
		
		for (int i = 1; i<SimpleSmurfProblemState->nNumVars; i++) {
			int nVarLevel = abs(SimpleSmurfProblemState->arrInferenceDeclaredAtLevel[i]);
			if(nVarLevel >= nCurrInfLevel)
			  tmp_solution_info->arrElts[arrSimpleSolver2IteVarMap[i]] = BOOL_UNKNOWN;
			else
			  tmp_solution_info->arrElts[arrSimpleSolver2IteVarMap[i]] = (SimpleSmurfProblemState->arrInferenceQueue[nVarLevel]>0)?BOOL_TRUE:BOOL_FALSE;
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
					if(abs(SimpleSmurfProblemState->arrInferenceDeclaredAtLevel[i]) >= nCurrInfLevel) {
						//double # of solutions
						num_vars++;
					}
					j++;
				}
			}
			if(use_XORGElim==1) num_vars-=nNumActiveXORGElimVectors(SimpleSmurfProblemState->arrSmurfStack[SimpleSmurfProblemState->nCurrSearchTreeLevel].XORGElimTable);
			ite_counters[NUM_SOLUTIONS]+=((LONG64)1)<<((LONG64)(num_vars));
		} else {
			num_vars = SimpleSmurfProblemState->nNumVars-1 - SimpleSmurfProblemState->arrSmurfStack[SimpleSmurfProblemState->nCurrSearchTreeLevel].nNumFreeVars;
			if(use_XORGElim==1) num_vars-=nNumActiveXORGElimVectors(SimpleSmurfProblemState->arrSmurfStack[SimpleSmurfProblemState->nCurrSearchTreeLevel].XORGElimTable);
			ite_counters[NUM_SOLUTIONS]+=((LONG64)1)<<((LONG64)(num_vars));
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

	int nCurrInfLevel = SimpleSmurfProblemState->arrSmurfStack[SimpleSmurfProblemState->nCurrSearchTreeLevel].nNumFreeVars;
	for(int nSmurfIndex = 0; nSmurfIndex < SimpleSmurfProblemState->nNumSmurfs; nSmurfIndex++) {
		//TrueState
		if(arrSmurfStates[nSmurfIndex] == pTrueSimpleSmurfState) continue;
		arrCalculateStateHeuristic[(int)((TypeStateEntry *)arrSmurfStates[nSmurfIndex])->cType](arrSmurfStates[nSmurfIndex], nCurrInfLevel);
	}
	Calculate_Heuristic_Values_Hooks();
	
	d7_printf1("JHeuristic values:\n");
	for(int nVar = 1; nVar < SimpleSmurfProblemState->nNumVars; nVar++) {
		d7_printf3(" %d: %4.6f\n", nVar, SimpleSmurfProblemState->arrPosVarHeurWghts[nVar]);
		d7_printf3("-%d: %4.6f\n", nVar, SimpleSmurfProblemState->arrNegVarHeurWghts[nVar]);
	}
	d7_printf1("\n");
}

ITE_INLINE
int (*Simple_Solver_Heuristic) () = NULL;

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
				if(abs(SimpleSmurfProblemState->arrInferenceDeclaredAtLevel[i]) >= nCurrInfLevel) {
					nBestVble = i;
					fMaxWeight = (1+SimpleSmurfProblemState->arrPosVarHeurWghts[i]) *
					  (1+SimpleSmurfProblemState->arrNegVarHeurWghts[i]);
					SimpleSmurfProblemState->arrSmurfStack[SimpleSmurfProblemState->nCurrSearchTreeLevel].nHeuristicPlaceholder = j;
					break;
				}
				j++;
			}
			if(SimpleSmurfProblemState->arrSmurfStack[SimpleSmurfProblemState->nCurrSearchTreeLevel].nVarChoiceCurrLevel != level)
			  SimpleSmurfProblemState->arrSmurfStack[SimpleSmurfProblemState->nCurrSearchTreeLevel].nHeuristicPlaceholder=0;

			if (nBestVble > 0) {
				while(arrVarChoiceLevels[level][j] != 0) {
					int i=arrVarChoiceLevels[level][j];
					if(abs(SimpleSmurfProblemState->arrInferenceDeclaredAtLevel[i]) >= nCurrInfLevel) {
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
				d9_printf4("Choosing %d from arrVarChoiceLevels[%d][%d]\n", nBestVble, level, SimpleSmurfProblemState->arrSmurfStack[SimpleSmurfProblemState->nCurrSearchTreeLevel].nHeuristicPlaceholder);
				return (((SimpleSmurfProblemState->arrPosVarHeurWghts[nBestVble]*arrVarTrueInfluences[nBestVble]) >=
							(SimpleSmurfProblemState->arrNegVarHeurWghts[nBestVble]*(1-arrVarTrueInfluences[nBestVble])))
							?nBestVble:-nBestVble);
			}
		}
	}

	for(int i = SimpleSmurfProblemState->arrSmurfStack[SimpleSmurfProblemState->nCurrSearchTreeLevel].nHeuristicPlaceholder+1; i < SimpleSmurfProblemState->nNumVars; i++) {
		if(abs(SimpleSmurfProblemState->arrInferenceDeclaredAtLevel[i]) >= nCurrInfLevel) {
			nBestVble = i;
			fMaxWeight = (1+SimpleSmurfProblemState->arrPosVarHeurWghts[i]) *
			  (1+SimpleSmurfProblemState->arrNegVarHeurWghts[i]);
			SimpleSmurfProblemState->arrSmurfStack[SimpleSmurfProblemState->nCurrSearchTreeLevel].nHeuristicPlaceholder = i-1;
			break;
		}
   }
	
	// Search through the remaining uninstantiated variables.
	for(int i = nBestVble + 1; i < SimpleSmurfProblemState->nNumVars; i++) {
		if(abs(SimpleSmurfProblemState->arrInferenceDeclaredAtLevel[i]) >= nCurrInfLevel) {
			fVbleWeight = (1+SimpleSmurfProblemState->arrPosVarHeurWghts[i]) *
			  (1+SimpleSmurfProblemState->arrNegVarHeurWghts[i]);
			if(fVbleWeight > fMaxWeight) {
				fMaxWeight = fVbleWeight;
				nBestVble = i;
			}
		}
	}
	
   if(nBestVble == 0) {
		dE_printf1 ("Error in heuristic routine:  No uninstantiated variable found\n");
		assert(0);
		exit (1);
   }
	
	SimpleSmurfProblemState->arrSmurfStack[SimpleSmurfProblemState->nCurrSearchTreeLevel].nVarChoiceCurrLevel = nVarChoiceLevelsNum;
		return (((SimpleSmurfProblemState->arrPosVarHeurWghts[nBestVble]*arrVarTrueInfluences[nBestVble]) >=
					(SimpleSmurfProblemState->arrNegVarHeurWghts[nBestVble]*(1-arrVarTrueInfluences[nBestVble])))
					?nBestVble:-nBestVble);
}

ITE_INLINE
int Simple_PMVSIDS_Heuristic() {
	
   int nBestVble = 0;
   int nMaxWeight = 0;
   int nVbleWeight = 0;
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
				if(abs(SimpleSmurfProblemState->arrInferenceDeclaredAtLevel[i]) >= nCurrInfLevel) {
					nBestVble = i;
					nMaxWeight = arrPMVSIDS[i];
					SimpleSmurfProblemState->arrSmurfStack[SimpleSmurfProblemState->nCurrSearchTreeLevel].nHeuristicPlaceholder = j;
					break;
				}
				j++;
			}
			if(SimpleSmurfProblemState->arrSmurfStack[SimpleSmurfProblemState->nCurrSearchTreeLevel].nVarChoiceCurrLevel != level)
			  SimpleSmurfProblemState->arrSmurfStack[SimpleSmurfProblemState->nCurrSearchTreeLevel].nHeuristicPlaceholder=0;
			
			if (nBestVble > 0) {
				while(arrVarChoiceLevels[level][j] != 0) {
					int i=arrVarChoiceLevels[level][j];
					if(abs(SimpleSmurfProblemState->arrInferenceDeclaredAtLevel[i]) >= nCurrInfLevel) {
						nVbleWeight = arrPMVSIDS[i];
						if(nVbleWeight > nMaxWeight) {
							nMaxWeight = nVbleWeight;
							nBestVble = i;
						}
					}
					j++;
				}
				SimpleSmurfProblemState->arrSmurfStack[SimpleSmurfProblemState->nCurrSearchTreeLevel].nVarChoiceCurrLevel = level;
				return arrVarTrueInfluences[nBestVble]>0.5?nBestVble:-nBestVble;
			}
		}
	}

	for(int i = SimpleSmurfProblemState->arrSmurfStack[SimpleSmurfProblemState->nCurrSearchTreeLevel].nHeuristicPlaceholder+1; i < SimpleSmurfProblemState->nNumVars; i++) {
		if(abs(SimpleSmurfProblemState->arrInferenceDeclaredAtLevel[i]) >= nCurrInfLevel) {
			nBestVble = i;
			nMaxWeight = arrPMVSIDS[i];
			SimpleSmurfProblemState->arrSmurfStack[SimpleSmurfProblemState->nCurrSearchTreeLevel].nHeuristicPlaceholder = i-1;
			break;
		}
   }

	// Search through the remaining uninstantiated variables.
	for(int i = nBestVble + 1; i < SimpleSmurfProblemState->nNumVars; i++) {
		if(abs(SimpleSmurfProblemState->arrInferenceDeclaredAtLevel[i]) >= nCurrInfLevel) {
			nVbleWeight = arrPMVSIDS[i];
			if(nVbleWeight > nMaxWeight) {
				nMaxWeight = nVbleWeight;
				nBestVble = i;
			}
		}
	}
	
   if(nBestVble == 0) {
		dE_printf1 ("Error in heuristic routine:  No uninstantiated variable found\n");
		assert(0);
		exit (1);
   }
	
	SimpleSmurfProblemState->arrSmurfStack[SimpleSmurfProblemState->nCurrSearchTreeLevel].nVarChoiceCurrLevel = nVarChoiceLevelsNum;
	assert(nBestVble>0 && nBestVble<SimpleSmurfProblemState->nNumVars);
	return arrVarTrueInfluences[nBestVble]>0.5?nBestVble:-nBestVble;
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
				if(abs(SimpleSmurfProblemState->arrInferenceDeclaredAtLevel[i]) >= nCurrInfLevel) {
					nBestVble = i;
					SimpleSmurfProblemState->arrSmurfStack[SimpleSmurfProblemState->nCurrSearchTreeLevel].nHeuristicPlaceholder = j;
					break;
				}
				j++;
			}
			if(SimpleSmurfProblemState->arrSmurfStack[SimpleSmurfProblemState->nCurrSearchTreeLevel].nVarChoiceCurrLevel != level)
			  SimpleSmurfProblemState->arrSmurfStack[SimpleSmurfProblemState->nCurrSearchTreeLevel].nHeuristicPlaceholder=0;
			if (nBestVble > 0) {
				SimpleSmurfProblemState->arrSmurfStack[SimpleSmurfProblemState->nCurrSearchTreeLevel].nVarChoiceCurrLevel = level;
				d9_printf4("Choosing %d from arrVarChoiceLevels[%d][%d]\n", nBestVble, level, SimpleSmurfProblemState->arrSmurfStack[SimpleSmurfProblemState->nCurrSearchTreeLevel].nHeuristicPlaceholder);
				return arrVarTrueInfluences[nBestVble]>0.5?nBestVble:-nBestVble;
			}
		}
	}

	for(int i = SimpleSmurfProblemState->arrSmurfStack[SimpleSmurfProblemState->nCurrSearchTreeLevel].nHeuristicPlaceholder+1; i < SimpleSmurfProblemState->nNumVars; i++) {
		if(abs(SimpleSmurfProblemState->arrInferenceDeclaredAtLevel[i]) >= nCurrInfLevel) {
			nBestVble = i;
			SimpleSmurfProblemState->arrSmurfStack[SimpleSmurfProblemState->nCurrSearchTreeLevel].nHeuristicPlaceholder = i-1;
			break;
		}
   }
	
   if(nBestVble == 0) {
		dE_printf1 ("Error in heuristic routine:  No uninstantiated variable found\n");
		assert(0);
		exit (1);
   }
	
	SimpleSmurfProblemState->arrSmurfStack[SimpleSmurfProblemState->nCurrSearchTreeLevel].nVarChoiceCurrLevel = nVarChoiceLevelsNum;
	assert(nBestVble>0 && nBestVble<SimpleSmurfProblemState->nNumVars);
	return arrVarTrueInfluences[nBestVble]>0.5?nBestVble:-nBestVble;
}

ITE_INLINE
int SimpleHeuristic() {
	int nBranchLit;
	
	//if(ite_counters[NUM_CHOICE_POINTS] %128 == 64)
	//if(SimpleSmurfProblemState->nCurrSearchTreeLevel > 10)
	//nBranchLit = Simple_DC_Heuristic(); //Don't Care - Choose the first unset variable found.
	//else 
	//nBranchLit = Simple_LSGB_Heuristic();
	nBranchLit = Simple_Solver_Heuristic();
	
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
		//		fprintf(stderr, "here");
	}
	
	ite_counters[NUM_CHOICE_POINTS]++;
	ite_counters[NO_ERROR]++;
	
	return nBranchLit;
}

int EnqueueInference_lemmas_hook(int nBranchVar, bool bBVPolarity) {
	//Try to insert inference into the inference Queue
	int nInfQueueHead = SimpleSmurfProblemState->arrSmurfStack[SimpleSmurfProblemState->nCurrSearchTreeLevel].nNumFreeVars;
	int nPrevInfLevel = abs(SimpleSmurfProblemState->arrInferenceDeclaredAtLevel[nBranchVar]);
	d7_printf4("      Lemma cache inferring %d at Level %d (prior level = %d)\n",
				  bBVPolarity?nBranchVar:-nBranchVar, nInfQueueHead, nPrevInfLevel);
	assert(use_lemmas || use_SmurfWatchedLists || nPrevInfLevel >= 0);
	
	if(nPrevInfLevel < nInfQueueHead) {
		//Inference already in queue
		assert((SimpleSmurfProblemState->arrInferenceQueue[nPrevInfLevel] > 0) == bBVPolarity);
		//Lemmas shouldn't be able to infer confliting variables.

		//Value is already inferred the correct value
		//Do nothing
		d7_printf2("      Inference %d already inferred\n", bBVPolarity?nBranchVar:-nBranchVar); 
		return 2;
	} else {
		//Inference is not in inference queue, insert it.
		SimpleSmurfProblemState->arrInferenceQueue[nInfQueueHead] = bBVPolarity?nBranchVar:-nBranchVar;
		SimpleSmurfProblemState->arrInferenceDeclaredAtLevel[nBranchVar] = nInfQueueHead;
		SimpleSmurfProblemState->arrSmurfStack[SimpleSmurfProblemState->nCurrSearchTreeLevel].nNumFreeVars++;
      ite_counters[INF_LEMMA]++;
      ite_counters[NUM_INFERENCES]++;
	}
	return 1;
}

int EnqueueInference(int nBranchVar, bool bBVPolarity, int inf_function_type) {
	//Try to insert inference into the inference Queue
	int nInfQueueHead = SimpleSmurfProblemState->arrSmurfStack[SimpleSmurfProblemState->nCurrSearchTreeLevel].nNumFreeVars;
	int nPrevInfLevel = abs(SimpleSmurfProblemState->arrInferenceDeclaredAtLevel[nBranchVar]);
	d7_printf4("      Inferring %d at Level %d (prior level = %d)\n",
				  bBVPolarity?nBranchVar:-nBranchVar, nInfQueueHead, nPrevInfLevel);
	assert(use_lemmas || use_SmurfWatchedLists || nPrevInfLevel >= 0);
	
	if(nPrevInfLevel < nInfQueueHead) {
		//Inference already in queue
		if((SimpleSmurfProblemState->arrInferenceQueue[nPrevInfLevel] > 0) != bBVPolarity) {
			//Conflict Detected;
			d7_printf2("      Conflict when adding %d to the inference queue\n", bBVPolarity?nBranchVar:-nBranchVar);
         ite_counters[inf_function_type+1]++; //ERR_BT should follow INF -- see sbsat/include/sbsat_vars.h
			if(use_lemmas) {
				if(backtrack_level < SimpleSmurfProblemState->nCurrSearchTreeLevel) {
					return 0;
				}
				backtrack_level = picosat_bcp(); //Empty the lemma database bcp queue
				if(backtrack_level < SimpleSmurfProblemState->nCurrSearchTreeLevel) { 
					return 0;
				}
				d7_printf2("      Applying conflict %d to lemma database\n", bBVPolarity?nBranchVar:-nBranchVar);
				backtrack_level = picosat_apply_inference(bBVPolarity?nBranchVar:-nBranchVar, SimpleSmurfProblemState->pConflictClause.clause);
			}
			return 0;
		} else {
			//Value is already inferred the correct value
			//Do nothing
			d7_printf2("      Inference %d already inferred\n", bBVPolarity?nBranchVar:-nBranchVar); 
		   return 2;
		}
	} else {
		//Inference is not in inference queue, insert it.
		SimpleSmurfProblemState->arrInferenceQueue[nInfQueueHead] = bBVPolarity?nBranchVar:-nBranchVar;
		SimpleSmurfProblemState->arrInferenceDeclaredAtLevel[nBranchVar] = nInfQueueHead;
		SimpleSmurfProblemState->arrSmurfStack[SimpleSmurfProblemState->nCurrSearchTreeLevel].nNumFreeVars++;
		ite_counters[inf_function_type]++;
      ite_counters[NUM_INFERENCES]++;

		if(use_lemmas) {
			d7_printf2("  Applying %d to lemma database\n", bBVPolarity?nBranchVar:-nBranchVar);
			backtrack_level = picosat_apply_inference(bBVPolarity?nBranchVar:-nBranchVar, SimpleSmurfProblemState->arrInferenceLemmas[nBranchVar].clause);
			if(backtrack_level < SimpleSmurfProblemState->nCurrSearchTreeLevel) {
            ite_counters[ERR_BT_LEMMA]++;
				return 0;
			}
		}
	}
	return 1;
}

ITE_INLINE
int ApplyInferenceToStates(int nBranchVar, bool bBVPolarity) {
	d7_printf2("  Handling inference %d\n", bBVPolarity?nBranchVar:-nBranchVar);

	int ret = ApplyInference_Hooks(nBranchVar, bBVPolarity);
	if(ret == 0) return 0;

	if(use_lemmas) {
		d7_printf1("  Calling lemma database bcp\n");
//		assert(backtrack_level >= SimpleSmurfProblemState);
		if(backtrack_level < SimpleSmurfProblemState->nCurrSearchTreeLevel) {
//			fprintf(stderr, "Don't really think this can happen\n");
//			assert(0);
			return 0;
		}
		backtrack_level = picosat_bcp();
		if(backtrack_level < SimpleSmurfProblemState->nCurrSearchTreeLevel) {
         ite_counters[ERR_BT_LEMMA]++;
         return 0;
      }
	}

	d7_printf2("  Transitioning Smurfs using %d\n", bBVPolarity?nBranchVar:-nBranchVar);
	void **arrSmurfStates = SimpleSmurfProblemState->arrSmurfStack[SimpleSmurfProblemState->nCurrSearchTreeLevel].arrSmurfStates;
	int i = SimpleSmurfProblemState->arrVariableOccursInSmurf[nBranchVar][0];
	int *arrVarOccursInSmurf = &(SimpleSmurfProblemState->arrVariableOccursInSmurf[nBranchVar][i]);
	for(; i > 0; i--) {
//	for(int i = SimpleSmurfProblemState->arrVariableOccursInSmurf[nBranchVar][0]; i > 0; i--) {
		int nSmurfNumber = *(arrVarOccursInSmurf--);
//		int nSmurfNumber = SimpleSmurfProblemState->arrVariableOccursInSmurf[nBranchVar][i];
		//SmurfNumber 0 always has all it's variables watched. No big deal really.
		if (nSmurfNumber < 0) continue; //Skip Non-Watched Variables
		d7_printf3("    Checking Smurf %d (State %p)\n", nSmurfNumber, (void *)arrSmurfStates[nSmurfNumber]);
		if(arrSmurfStates[nSmurfNumber] == pTrueSimpleSmurfState) continue;
		ret = arrApplyInferenceToState[(int)((TypeStateEntry *)arrSmurfStates[nSmurfNumber])->cType](nBranchVar, bBVPolarity, nSmurfNumber, arrSmurfStates);
//		ret = ((TypeStateEntry *)arrSmurfStates[nSmurfNumber])->ApplyInferenceToState(nBranchVar, bBVPolarity, nSmurfNumber, arrSmurfStates);
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

	if(use_SmurfWatchedLists) {
		SimpleSmurfProblemState->arrSmurfStack[destination].arrSmurfStates = 
		  SimpleSmurfProblemState->arrSmurfStack[SimpleSmurfProblemState->nCurrSearchTreeLevel].arrSmurfStates;
	} else  {
		memcpy_ite(SimpleSmurfProblemState->arrSmurfStack[destination].arrSmurfStates,
					  SimpleSmurfProblemState->arrSmurfStack[SimpleSmurfProblemState->nCurrSearchTreeLevel].arrSmurfStates,
					  SimpleSmurfProblemState->nNumSmurfs*sizeof(void *));
	}
		
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

	SmurfStates_Push_Hooks(SimpleSmurfProblemState->nCurrSearchTreeLevel, destination);
	
	SimpleSmurfProblemState->nCurrSearchTreeLevel = destination;
}

ITE_INLINE
int SmurfStates_Pop(int pop_to) {

	int nInfQueueTail = SimpleSmurfProblemState->arrSmurfStack[SimpleSmurfProblemState->nCurrSearchTreeLevel].nNumFreeVars;
	
	while (SimpleSmurfProblemState->nCurrSearchTreeLevel>pop_to) {
		
		SimpleSmurfProblemState->nCurrSearchTreeLevel--;
		
		if(SimpleSmurfProblemState->nCurrSearchTreeLevel < 0 || simple_solver_reset_level!=-1) {
			if(ite_counters[NUM_SOLUTIONS] == 0) return SOLV_UNSAT; //return Unsatisifable
			else return SOLV_SAT;
		}
		
		SmurfStates_Pop_Hooks();
	}

	int nInfQueueHead = SimpleSmurfProblemState->arrSmurfStack[SimpleSmurfProblemState->nCurrSearchTreeLevel].nNumFreeVars;
	
	//Clear Inference Queue
	
	//Using the inference queue to avoid a linear check over all variables.
	for(int i = nInfQueueHead; i < nInfQueueTail; i++) {
		int nBranchLit = abs(SimpleSmurfProblemState->arrInferenceQueue[i]);
		d7_printf3("      Resetting level of variable %d to %d\n", nBranchLit, SimpleSmurfProblemState->nNumVars);
		SimpleSmurfProblemState->arrInferenceDeclaredAtLevel[nBranchLit] = SimpleSmurfProblemState->nNumVars;
	}
	
	return SOLV_UNKNOWN;
}

ITE_INLINE
int Backtrack() {
	//Pop stack
	ite_counters[NUM_BACKTRACKS]++;

	d7_printf1("  Conflict - Backtracking\n");

	d9_printf2("\nStarting backtrack # %ld\n", (long)ite_counters[NUM_BACKTRACKS]);

	int ret = Backtrack_Hooks();
	
	int nPop = SmurfStates_Pop(SimpleSmurfProblemState->nCurrSearchTreeLevel-1);
	if(nPop != SOLV_UNKNOWN) return nPop;

	return ret;
}

int SimpleBrancher() {

	int ret = SOLV_UNKNOWN;
	bool bBVPolarity; 
	int nBranchLit, nInfQueueHead, nPrevInfLevel, nBranchVar;

	backtrack_level = SimpleSmurfProblemState->nNumVars;
	
	LONG64 max_solutions_simple = max_solutions>0?max_solutions:(max_solutions==0?-1:0);

	nInfQueueHead = 0;
	
	fSimpleSolverStartTime = get_runtime();

	//Clear out any initial inferences
	while(nInfQueueHead != SimpleSmurfProblemState->arrSmurfStack[SimpleSmurfProblemState->nCurrSearchTreeLevel].nNumFreeVars) {
		//Get inference
		nBranchLit = SimpleSmurfProblemState->arrInferenceQueue[nInfQueueHead];
		nInfQueueHead++;
		bBVPolarity = (nBranchLit < 0)?0:1;
		nBranchVar = bBVPolarity?nBranchLit:-nBranchLit;
		
		//apply inference to all involved smurfs
		if(ApplyInferenceToStates(nBranchVar, bBVPolarity) == 0)
		  return SOLV_UNSAT;
	}
	
	while((max_solutions>0)?(ite_counters[NUM_SOLUTIONS]<max_solutions_simple):1) {
		while(SimpleSmurfProblemState->arrSmurfStack[SimpleSmurfProblemState->nCurrSearchTreeLevel].nNumFreeVars < SimpleSmurfProblemState->nNumVars-1
				&& (result_display_type || simple_solver_reset_level!=-1 || (SimpleSmurfProblemState->arrSmurfStack[SimpleSmurfProblemState->nCurrSearchTreeLevel].nNumSmurfsSatisfied < SimpleSmurfProblemState->nNumSmurfs))
				) {

			if(use_RapidRestarts && nNumRestarts<MAX_RESTARTS && ite_counters[NUM_BACKTRACKS]>nNextRestart) {
				nNextRestart+=Simple_nextRestart();
				nNumRestarts++;
				d3_printf1("Restarting\n");
				ite_counters[NUM_SOLUTIONS]=0;
				nSimpleSolver_Reset=0;
				nInfQueueStart=0;
				solver_polarity_presets_count=0;
				add_one_display=0;
				solutions_overflow=0;
				SmurfStates_Pop(0);
			}

			//Update heuristic values
		
			//Call Heuristic to get variable and polarity
			d7_printf2("Calling heuristic to choose choice point #%lld\n", ite_counters[NUM_CHOICE_POINTS]);
			nBranchLit = SimpleHeuristic();

         GarbageCollectSmurfStatesTable(0); //Garbage collect the state machines if necessary
         
         //Push stack
			if(nSimpleSolver_Reset) { SmurfStates_Push(0); nSimpleSolver_Reset = 0; }
			else SmurfStates_Push(SimpleSmurfProblemState->nCurrSearchTreeLevel+1); //Normal condition
         
			//Insert heuristic var into inference queue
			nInfQueueHead = SimpleSmurfProblemState->arrSmurfStack[SimpleSmurfProblemState->nCurrSearchTreeLevel].nNumFreeVars;
			//First clear out old inference
			//SimpleSmurfProblemState->arrInferenceDeclaredAtLevel[abs(SimpleSmurfProblemState->arrInferenceQueue[nInfQueueHead])] =
			//SimpleSmurfProblemState->nNumVars;
			//Then insert new inference
			nPrevInfLevel = abs(SimpleSmurfProblemState->arrInferenceDeclaredAtLevel[abs(nBranchLit)]);
			SimpleSmurfProblemState->arrInferenceQueue[nInfQueueHead] = nBranchLit;
			SimpleSmurfProblemState->arrInferenceDeclaredAtLevel[abs(nBranchLit)] = nInfQueueHead;
			SimpleSmurfProblemState->arrSmurfStack[SimpleSmurfProblemState->nCurrSearchTreeLevel].nNumFreeVars++;
			d7_printf5("Inferring %c%d (choice point) at Level %d (prior level = %d)\n",
						  nBranchLit>0?'+':'-', abs(nBranchLit), nInfQueueHead, nPrevInfLevel);
			//    d7_printf3("Choice Point #%d = %d\n", , nBranchLit);

			if(use_lemmas) {
				//Apply choice point to the lemma table
				d7_printf2("  Applying choice %d to lemma database\n", nBranchLit);
				backtrack_level = picosat_apply_inference(nBranchLit, NULL);
			}
			
			//While inference queue != empty {
			while(nInfQueueHead != SimpleSmurfProblemState->arrSmurfStack[SimpleSmurfProblemState->nCurrSearchTreeLevel].nNumFreeVars) {
				//Get inference
				nBranchLit = SimpleSmurfProblemState->arrInferenceQueue[nInfQueueHead];
				nInfQueueHead++;
				bBVPolarity = (nBranchLit < 0)?0:1;
				nBranchVar = bBVPolarity?nBranchLit:-nBranchLit;
				
				//apply inference to all involved smurfs
				if(ApplyInferenceToStates(nBranchVar, bBVPolarity) == 0) {
					//A conflict occured
					if(use_lemmas) {
find_more_solutions_lemmas: ;
						if(backtrack_level >= SimpleSmurfProblemState->nCurrSearchTreeLevel) {
							d7_printf1("  Backtracking the lemma database and resolving new conflict clauses\n");							  
							backtrack_level = picosat_bcp();
						}
						if(SimpleSmurfProblemState->nCurrSearchTreeLevel == 0) {
							if(ite_counters[NUM_SOLUTIONS] == 0) return SOLV_UNSAT; //return Unsatisifable
							else return SOLV_SAT;
						}
						assert(backtrack_level < SimpleSmurfProblemState->nCurrSearchTreeLevel);
						d7_printf3("  Backtrack from level %d to level %d\n", SimpleSmurfProblemState->nCurrSearchTreeLevel, backtrack_level);
						
						do {
							switch(Backtrack()) {
							 case SOLV_UNSAT: return SOLV_UNSAT;
							 case SOLV_SAT: return SOLV_SAT;
							 case SOLV_LIMIT: return ret;
							 default: break;
							}
						} while (backtrack_level < SimpleSmurfProblemState->nCurrSearchTreeLevel);

						nInfQueueHead = SimpleSmurfProblemState->arrSmurfStack[SimpleSmurfProblemState->nCurrSearchTreeLevel].nNumFreeVars;
						d7_printf1("  Calling lemma database to infer the UIP literal\n");
						backtrack_level = picosat_bcp();
						//The lemma table has inferred at least one UIP variable
						
						//Negate this value to flag variable as an old choicepoint
						if(SimpleSmurfProblemState->arrInferenceDeclaredAtLevel[abs(SimpleSmurfProblemState->arrInferenceQueue[nInfQueueHead])] == 0)
						  add_one_display = 1;
						else SimpleSmurfProblemState->arrInferenceDeclaredAtLevel[abs(SimpleSmurfProblemState->arrInferenceQueue[nInfQueueHead])] = -SimpleSmurfProblemState->arrInferenceDeclaredAtLevel[abs(SimpleSmurfProblemState->arrInferenceQueue[nInfQueueHead])];
						continue; //Go check the queue.
					}
					
					//To maintain consistency (and being able to find ALL solutions over months of running)
					//I'll need this literal phase consistency heuristic thing Knot did. So, after backtracking --
					//I think this will work...

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
		
		if((max_solutions>0)?(ite_counters[NUM_SOLUTIONS]<max_solutions_simple):1) {
			if(use_lemmas) {
				if(nForceBackjumpLevel < SimpleSmurfProblemState->arrSmurfStack[SimpleSmurfProblemState->nCurrSearchTreeLevel].nVarChoiceCurrLevel) {
					d7_printf2("Forcing a backjump to at least level %d\n", nForceBackjumpLevel);
					while(nForceBackjumpLevel < SimpleSmurfProblemState->arrSmurfStack[SimpleSmurfProblemState->nCurrSearchTreeLevel].nVarChoiceCurrLevel) {
						switch(Backtrack()) {
						 case SOLV_UNSAT: return SOLV_UNSAT;
						 case SOLV_SAT: return SOLV_SAT;
						 case SOLV_LIMIT: return ret;
						 default: break;
						}
					}
				}
				//Add conflict lemma
				if(SimpleSmurfProblemState->nCurrSearchTreeLevel == 0) return SOLV_SAT;
//				fprintf(stderr, "conflict decision == %d\n", -SimpleSmurfProblemState->arrInferenceQueue[SimpleSmurfProblemState->arrSmurfStack[SimpleSmurfProblemState->nCurrSearchTreeLevel-1].nNumFreeVars]);
				create_clause_from_SBSAT_solution(SimpleSmurfProblemState->arrInferenceQueue,
															 SimpleSmurfProblemState->arrSmurfStack,
															 SimpleSmurfProblemState->nCurrSearchTreeLevel,
															 &(SimpleSmurfProblemState->pConflictClause.clause),
															 &(SimpleSmurfProblemState->pConflictClause.max_size));
				backtrack_level = picosat_apply_inference(-SimpleSmurfProblemState->arrInferenceQueue[SimpleSmurfProblemState->arrSmurfStack[SimpleSmurfProblemState->nCurrSearchTreeLevel-1].nNumFreeVars],
																		SimpleSmurfProblemState->pConflictClause.clause);
				goto find_more_solutions_lemmas;
			}
			
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
	int DO_EQUIVALENCES_OLD = DO_EQUIVALENCES;
	DO_EQUIVALENCES = 0;
	
	if(ret != SOLV_UNKNOWN) return ret;
	
	ret = SimpleBrancher();
	
	nForceBackjumpLevel = nForceBackjumpLevel_old;
	
	Final_SimpleSmurfSolver();

	DO_EQUIVALENCES = DO_EQUIVALENCES_OLD;
	
	return ret;
}
