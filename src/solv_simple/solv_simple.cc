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

int nSimpleSolver_Reset=0;
int nInfQueueStart=0;
int solver_polarity_presets_count=0;
int simple_solver_reset_level=-1;
int add_one_display=0;

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
			tmp_solution_info->arrElts[arrSimpleSolver2IteVarMap[abs(SimpleSmurfProblemState->arrInferenceQueue[i])]] = (SimpleSmurfProblemState->arrInferenceQueue[i]>0)?BOOL_TRUE:BOOL_FALSE;
		}
	}
}

ITE_INLINE
void Calculate_Heuristic_Values() {

	memset(SimpleSmurfProblemState->arrPosVarHeurWghts, 0, sizeof(double)*SimpleSmurfProblemState->nNumVars);
	memset(SimpleSmurfProblemState->arrNegVarHeurWghts, 0, sizeof(double)*SimpleSmurfProblemState->nNumVars);
	
	void **arrSmurfStates = SimpleSmurfProblemState->arrSmurfStack[SimpleSmurfProblemState->nCurrSearchTreeLevel].arrSmurfStates;
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
			SimpleSmurfProblemState->arrPosVarHeurWghts[((SmurfStateEntry *)pState)->nTransitionVar] +=
			  ((SmurfStateEntry *)pState)->fHeurWghtofTrueTransition;
			SimpleSmurfProblemState->arrNegVarHeurWghts[((SmurfStateEntry *)pState)->nTransitionVar] +=  
			  ((SmurfStateEntry *)pState)->fHeurWghtofFalseTransition;
			while (((SmurfStateEntry *)pState)->pNextVarInThisState != NULL) {
				((SmurfStateEntry *)pState) = ((SmurfStateEntry *)((SmurfStateEntry *)pState)->pNextVarInThisState);
				SimpleSmurfProblemState->arrPosVarHeurWghts[((SmurfStateEntry *)pState)->nTransitionVar] +=
				  ((SmurfStateEntry *)pState)->fHeurWghtofTrueTransition;
				SimpleSmurfProblemState->arrNegVarHeurWghts[((SmurfStateEntry *)pState)->nTransitionVar] +=  
				  ((SmurfStateEntry *)pState)->fHeurWghtofFalseTransition;
			}
			break;
		 case FN_OR_COUNTER:
			pState = arrSmurfStates[nSmurfIndex];
			for(int index = 0; numfound < ((ORCounterStateEntry *)pState)->nSize; index++) {
				if(SimpleSmurfProblemState->arrInferenceDeclaredAtLevel[((ORCounterStateEntry *)pState)->pORState->nTransitionVars[index]] >= nCurrInfLevel) {
					numfound++;
					if(((ORCounterStateEntry *)pState)->pORState->bPolarity[index] == BOOL_TRUE) {
						SimpleSmurfProblemState->arrPosVarHeurWghts[((ORCounterStateEntry *)pState)->pORState->nTransitionVars[index]] += JHEURISTIC_K_TRUE;
						SimpleSmurfProblemState->arrNegVarHeurWghts[((ORCounterStateEntry *)pState)->pORState->nTransitionVars[index]] += LSGBORCounterGetHeurNeg(((ORCounterStateEntry *)pState));
					} else {
						SimpleSmurfProblemState->arrPosVarHeurWghts[((ORCounterStateEntry *)pState)->pORState->nTransitionVars[index]] += LSGBORCounterGetHeurNeg(((ORCounterStateEntry *)pState));
						SimpleSmurfProblemState->arrNegVarHeurWghts[((ORCounterStateEntry *)pState)->pORState->nTransitionVars[index]] += JHEURISTIC_K_TRUE;
					}
				}
			}
			break;
		 case FN_XOR_COUNTER:
			pState = arrSmurfStates[nSmurfIndex];
			for(int index = 0; numfound < ((XORCounterStateEntry *)pState)->nSize; index++) {
				if(SimpleSmurfProblemState->arrInferenceDeclaredAtLevel[((XORCounterStateEntry *)pState)->pXORState->nTransitionVars[index]] >= nCurrInfLevel) {
					numfound++;
					SimpleSmurfProblemState->arrPosVarHeurWghts[((XORCounterStateEntry *)pState)->pXORState->nTransitionVars[index]] += LSGBXORCounterGetHeurScoreTrans(((XORCounterStateEntry *)pState));
					SimpleSmurfProblemState->arrNegVarHeurWghts[((XORCounterStateEntry *)pState)->pXORState->nTransitionVars[index]] += LSGBXORCounterGetHeurScoreTrans(((XORCounterStateEntry *)pState));
				}
			}
			break;
		 case FN_OR:
			pState = arrSmurfStates[nSmurfIndex];
			for(int index = 0; numfound < 2; index++) {
				if(SimpleSmurfProblemState->arrInferenceDeclaredAtLevel[((ORStateEntry *)pState)->nTransitionVars[index]] >= nCurrInfLevel) {
					numfound++;
					if(((ORStateEntry *)pState)->bPolarity[index] == BOOL_TRUE) {
						SimpleSmurfProblemState->arrPosVarHeurWghts[((ORStateEntry *)pState)->nTransitionVars[index]] += JHEURISTIC_K_TRUE;
						SimpleSmurfProblemState->arrNegVarHeurWghts[((ORStateEntry *)pState)->nTransitionVars[index]] += LSGBORGetHeurNeg(((ORStateEntry *)pState));
					} else {
						SimpleSmurfProblemState->arrPosVarHeurWghts[((ORStateEntry *)pState)->nTransitionVars[index]] += LSGBORGetHeurNeg(((ORStateEntry *)pState));
						SimpleSmurfProblemState->arrNegVarHeurWghts[((ORStateEntry *)pState)->nTransitionVars[index]] += JHEURISTIC_K_TRUE;
					}
				}
			}
			break;
		 case FN_XOR:
			pState = arrSmurfStates[nSmurfIndex];
			for(int index = 0; numfound < 2; index++) {
				if(SimpleSmurfProblemState->arrInferenceDeclaredAtLevel[((XORStateEntry *)pState)->nTransitionVars[index]] >= nCurrInfLevel) {
					numfound++;
					SimpleSmurfProblemState->arrPosVarHeurWghts[((XORStateEntry *)pState)->nTransitionVars[index]] += LSGBXORGetHeurScoreTrans(((XORStateEntry *)pState));
					SimpleSmurfProblemState->arrNegVarHeurWghts[((XORStateEntry *)pState)->nTransitionVars[index]] += LSGBXORGetHeurScoreTrans(((XORStateEntry *)pState));
				}
			}
			break;
		 default: break;
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
	
   if(nBestVble > 0) {
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
int ApplyInferenceToStates(int nBranchVar, bool bBVPolarity) {
	void **arrSmurfStates = SimpleSmurfProblemState->arrSmurfStack[SimpleSmurfProblemState->nCurrSearchTreeLevel].arrSmurfStates;
	d7_printf2("  Transitioning Smurfs using %d\n", bBVPolarity?nBranchVar:-nBranchVar);
	for(int i = 0; SimpleSmurfProblemState->arrVariableOccursInSmurf[nBranchVar][i] != -1; i++) {
		int nSmurfNumber = SimpleSmurfProblemState->arrVariableOccursInSmurf[nBranchVar][i];
		d7_printf3("    Checking Smurf %d (State %x)\n", nSmurfNumber, arrSmurfStates[nSmurfNumber]);
		if(arrSmurfStates[nSmurfNumber] == pTrueSimpleSmurfState) continue;
		void *pState = arrSmurfStates[nSmurfNumber];
		int ret = 1;

		switch(((TypeStateEntry *)pState)->cType) {
		 case FN_SMURF:
			ret = ApplyInferenceToSmurf(nBranchVar, bBVPolarity, nSmurfNumber, arrSmurfStates); break;
		 case FN_OR_COUNTER:
			ret = ApplyInferenceToORCounter(nBranchVar, bBVPolarity, nSmurfNumber, arrSmurfStates); break;
		 case FN_XOR_COUNTER:
			ret = ApplyInferenceToXORCounter(nBranchVar, bBVPolarity, nSmurfNumber, arrSmurfStates); break;
		 case FN_OR:
			ret = ApplyInferenceToOR(nBranchVar, bBVPolarity, nSmurfNumber, arrSmurfStates); break;
		 case FN_XOR:
			ret = ApplyInferenceToXOR(nBranchVar, bBVPolarity, nSmurfNumber, arrSmurfStates); break;
		 default: break;
		}
		if(ret == 0) return 0;
	}
	return ApplyInference_Hooks(nBranchVar, bBVPolarity);
}

ITE_INLINE
void SmurfStates_Push(int destination) {

	//destination=nCurrSearchTreeLevel+1, except in the case of a nSimpleSolver_Reset then destination=0
	if(SimpleSmurfProblemState->arrSmurfStack[destination].arrSmurfStates == NULL) {
		Alloc_SmurfStack(destination);
	}
		  
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
	
	SmurfStates_Push_Hooks();
	
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
	  
	int nPop = SmurfStates_Pop();
	if(nPop != SOLV_UNKNOWN) return nPop;

	int nInfQueueHead = SimpleSmurfProblemState->arrSmurfStack[SimpleSmurfProblemState->nCurrSearchTreeLevel].nNumFreeVars;
	
	//Empty the inference queue & reverse polarity of cp var
	//Clear Inference Queue

	//Using the inference queue to avoid a linear check over all variables.
	for(int i = nInfQueueHead; i < nInfQueueTail; i++) {
		int nBranchLit = abs(SimpleSmurfProblemState->arrInferenceQueue[i]);
		d7_printf3("      Resetting level of variable %d to %d\n", nBranchLit, SimpleSmurfProblemState->nNumVars);
		SimpleSmurfProblemState->arrInferenceDeclaredAtLevel[nBranchLit] = SimpleSmurfProblemState->nNumVars;
	}
	  
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

		ite_counters[NUM_SOLUTIONS]++;
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
