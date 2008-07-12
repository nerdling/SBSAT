#include "sbsat.h"
#include "sbsat_solver.h"
#include "solver.h"

void fillHEAP(int index, int size, int *arrElts) {
	SmurfStateEntry *pCurrState = (SmurfStateEntry *)SimpleSmurfProblemState->pSmurfStatesTableTail;
	SimpleSmurfProblemState->nNumSmurfStateEntries++;
	SimpleSmurfProblemState->pSmurfStatesTableTail = (void *)(pCurrState + 1);
	pCurrState->cType = FN_SMURF;
	pCurrState->ApplyInferenceToState = ApplyInferenceToSmurf;
	pCurrState->nTransitionVar = arrElts[index+(size/2)];
	//fprintf(stderr, "%d(%d) - ", arrElts[index+(size/2)], size);
	if(size<=1) return;
	pCurrState->pNextVarInThisStateGT = SimpleSmurfProblemState->pSmurfStatesTableTail;
	fillHEAP(index, size/2, arrElts);
	if(size<=2) return;
	pCurrState->pNextVarInThisStateLT = SimpleSmurfProblemState->pSmurfStatesTableTail;
	fillHEAP(index+(size/2)+1, (size-(size/2))-1, arrElts);
	assert(pCurrState->pNextVarInThisStateGT != pCurrState->pNextVarInThisStateLT);
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

WatchedListStateEntry *CreateWatchedListState(int *arrWatchedList, int nWatchedListSize) {
	check_SmurfStatesTableSize(sizeof(WatchedListStateEntry));
	ite_counters[SMURF_STATES]++;
	assert(SimpleSmurfProblemState->pSmurfStatesTableTail != NULL);
	WatchedListStateEntry *pWLState = (WatchedListStateEntry *)SimpleSmurfProblemState->pSmurfStatesTableTail;	
	SimpleSmurfProblemState->nNumSmurfStateEntries++;
	SimpleSmurfProblemState->pSmurfStatesTableTail = (void *)(pWLState + 1);
	pWLState->cType = FN_WATCHED_LIST;
	pWLState->nWatchedListSize = nWatchedListSize;
	pWLState->arrWatchedList = arrWatchedList;
	return pWLState;
}

void *CreateSmurfState(int *arrElts, int nNumElts, BDDNode *pCurrentBDD, SmurfStateEntry *pStartState) {
	check_SmurfStatesTableSize(sizeof(SmurfStateEntry)*nNumElts);
	ite_counters[SMURF_STATES]+=nNumElts;
	pCurrentBDD->pState = SimpleSmurfProblemState->pSmurfStatesTableTail;
	
	pStartState = (SmurfStateEntry *)SimpleSmurfProblemState->pSmurfStatesTableTail;
	fillHEAP(0, nNumElts, arrElts);
	
	for(int nVbleIndex = 0; nVbleIndex < nNumElts-1; nVbleIndex++) {
		SmurfStateEntry *pCurrState = (pStartState+nVbleIndex);
		pCurrState->pNextVarInThisState = (void *)(pStartState+nVbleIndex+1);
	}

	for(int nVbleIndex = 0; nVbleIndex < nNumElts; nVbleIndex++) {
		//fprintf(stderr, "%d, %d, %d: ", nVbleIndex, arrElts[nVbleIndex], arrSimpleSolver2IteVarMap[arrElts[nVbleIndex]]);
		//    printBDDerr(pCurrentBDD);
		//    fprintf(stderr, "\n");
		//    PrintAllSmurfStateEntries();
		   
		SmurfStateEntry *pCurrState = findStateInHEAP(pStartState, arrElts[nVbleIndex]);
		
		assert(pCurrState->nTransitionVar == arrElts[nVbleIndex]);

		/*
			//Determine and record if nTransitionVar is safe
			BDDNode *pSafeBDD = false_ptr;//safe_assign(pCurrentBDD->pFunc, pCurrState->nTransitionVar);
			if(pSafeBDD == false_ptr)
			  pCurrState->bVarIsSafe = 0;
			else if(pSafeBDD->thenCase == true_ptr && pSafeBDD->elseCase == false_ptr) {
				pCurrState->bVarIsSafe = 1;
				fprintf(stderr, "{+1}");
			} else if(pSafeBDD->thenCase == false_ptr && pSafeBDD->elseCase == true_ptr) {
				pCurrState->bVarIsSafe = -1;
				fprintf(stderr, "{-1}");
			}
		*/

		//Compute the SmurfState w/ nTransitionVar = False
		//Create Inference Transitions
		BDDNode *infBDD = pCurrentBDD;
		infBDD = set_variable(infBDD, arrSimpleSolver2IteVarMap[pCurrState->nTransitionVar], 0);
		SmurfStateEntry *pNextState = pCurrState;
		InferenceStateEntry *pNextInfState = NULL;
		//Follow positive and negative inferences
		infer *inferences = infBDD->inferences;
		infer *save_inferences = inferences;
		int nNumInferences = 0;
		while(inferences!=NULL) { 
			if(inferences->nums[1] != 0) {
				inferences = inferences->next; continue; 
			}

			//Add a SmurfStateEntry into the table
			if(infBDD->pState != NULL) break;
			check_SmurfStatesTableSize(sizeof(InferenceStateEntry));
			ite_counters[SMURF_STATES]++;
			infBDD->pState = SimpleSmurfProblemState->pSmurfStatesTableTail;
			if(nNumInferences == 0) pNextState->pVarIsFalseTransition = infBDD->pState; //The transition is False
			else pNextInfState->pVarTransition = infBDD->pState;
			nNumInferences++;
			assert(infBDD->pState!=NULL);
			pNextInfState = (InferenceStateEntry *)SimpleSmurfProblemState->pSmurfStatesTableTail;
			SimpleSmurfProblemState->nNumSmurfStateEntries++;
			SimpleSmurfProblemState->pSmurfStatesTableTail = (void *)(pNextInfState + 1);
			pNextInfState->cType = FN_INFERENCE;
			if(inferences->nums[0] > 0) {
				pNextInfState->nTransitionVar = arrIte2SimpleSolverVarMap[inferences->nums[0]];
				pNextInfState->bPolarity = 1;
				infBDD = set_variable(infBDD, arrSimpleSolver2IteVarMap[pNextInfState->nTransitionVar], 1);
			} else {						
				pNextInfState->nTransitionVar = arrIte2SimpleSolverVarMap[-inferences->nums[0]];
				pNextInfState->bPolarity = 0;
				infBDD = set_variable(infBDD, arrSimpleSolver2IteVarMap[pNextInfState->nTransitionVar], 0);
			}
			inferences = infBDD->inferences;
//			inferences = inferences->next;
		}

		//Determine if we need to create some WatchedListStates

		WatchedListStateEntry *pNextWatchedListState;
		int nNumElts_next;
		int *arrElts_next = NULL;
		int nWatchedListSize = 0;
		if(infBDD != true_ptr && use_SmurfWatchedLists) {
			nNumElts_next = 0;
			unravelBDD(&nNumElts_next, &tempint_max, &tempint, infBDD);
			arrElts_next = (int *)ite_calloc(nNumElts_next, sizeof(int), 9, "arrElts_next");
			for (int i=0;i<nNumElts_next;i++) {
				if (tempint[i]==0 || arrIte2SimpleSolverVarMap[tempint[i]]==0) {
					dE_printf1("\nassigned variable in a BDD in the solver");
					dE_printf3("\nvariable id: %d, true_false=%d\n", tempint[i], variablelist[tempint[i]].true_false);
					exit(1);
				}
				arrElts_next[i] = arrIte2SimpleSolverVarMap[tempint[i]];
			}
			qsort(arrElts_next, nNumElts_next, sizeof(int), revcompfunc);
			
			nWatchedListSize = nNumElts - (nNumElts_next+nNumInferences+1);
		
			if(nWatchedListSize > 0) { //Some variables dropped out
				int *arrWatchedList = (int *)ite_calloc(nWatchedListSize, sizeof(int), 9, "nWatchedListSize");
				int next=0;
				int wl=0;
				for(int curr=0; curr < nNumElts; curr++) {
					if(arrElts[curr] == arrElts_next[next]) {
						next++; continue;
					}
					if(arrElts[curr] == pCurrState->nTransitionVar) continue;
					int cont = 0;
					inferences = save_inferences;
					while(inferences!=NULL) { 
						if(inferences->nums[1] != 0) {
							inferences = inferences->next; continue;
						}
						if(arrElts[curr] == abs(inferences->nums[0])) {
							cont=1; break;
						}
						inferences = inferences->next;						
					}
					if(cont == 1) continue;

					if(arrElts[curr] != arrElts_next[next]) 
					  arrWatchedList[wl++] = arrElts[curr];
				}
				assert(next == nNumElts_next);
				assert(wl == nWatchedListSize);
				
				pNextWatchedListState = CreateWatchedListState(arrWatchedList, nWatchedListSize);
				if(nNumInferences == 0)
				  pNextState->pVarIsFalseTransition = pNextWatchedListState;
				else {
					pNextWatchedListState->pVarTransition = pNextState->pVarIsFalseTransition;
					pNextState->pVarIsFalseTransition = pNextWatchedListState;
				}
			}
		}
		
		if(infBDD->pState != NULL) {
			ite_free((void **)&arrElts_next);
			if(nNumInferences == 0 && nWatchedListSize == 0) pNextState->pVarIsFalseTransition = infBDD->pState; //The transition is False
			else if(nNumInferences == 0) pNextWatchedListState->pVarTransition = infBDD->pState;
			else pNextInfState->pVarTransition = infBDD->pState;
		} else {
			//Recurse on nTransitionVar == False transition
			void *pNext = ReadSmurfStateIntoTable(infBDD, arrElts_next, nNumElts_next);
			assert(pNext!=NULL);
			if(nNumInferences == 0 && nWatchedListSize == 0) pNextState->pVarIsFalseTransition = pNext; //The transition is False
			else if(nNumInferences == 0) pNextWatchedListState->pVarTransition = pNext;
			else pNextInfState->pVarTransition = pNext;
		}

		//Compute the SmurfState w/ nTransitionVar = True
		//Create Inference Transitions
		infBDD = pCurrentBDD;
		infBDD = set_variable(infBDD, arrSimpleSolver2IteVarMap[pCurrState->nTransitionVar], 1);
		pNextState = pCurrState;
		pNextInfState = NULL;
		//Follow positive and negative inferences
		inferences = infBDD->inferences;
		save_inferences = inferences;
		nNumInferences = 0;
		while(inferences!=NULL) {
			if(inferences->nums[1] != 0) {
				inferences = inferences->next; continue; 
			}
			
			//Add a SmurfStateEntry into the table
			if(infBDD->pState != NULL) break;
			check_SmurfStatesTableSize(sizeof(InferenceStateEntry));
			ite_counters[SMURF_STATES]++;
			infBDD->pState = SimpleSmurfProblemState->pSmurfStatesTableTail;
			if(nNumInferences == 0) pNextState->pVarIsTrueTransition = infBDD->pState; //The transition is True
			else pNextInfState->pVarTransition = infBDD->pState;
			nNumInferences++;
			assert(infBDD->pState!=NULL);
			pNextInfState = (InferenceStateEntry *)SimpleSmurfProblemState->pSmurfStatesTableTail;
			SimpleSmurfProblemState->nNumSmurfStateEntries++;
			SimpleSmurfProblemState->pSmurfStatesTableTail = (void *)(pNextInfState + 1);
			pNextInfState->cType = FN_INFERENCE;
			if(inferences->nums[0] > 0) {
				pNextInfState->nTransitionVar = arrIte2SimpleSolverVarMap[inferences->nums[0]];
				pNextInfState->bPolarity = 1;
				infBDD = set_variable(infBDD, arrSimpleSolver2IteVarMap[pNextInfState->nTransitionVar], 1);
			} else {						
				pNextInfState->nTransitionVar = arrIte2SimpleSolverVarMap[-inferences->nums[0]];
				pNextInfState->bPolarity = 0;
				infBDD = set_variable(infBDD, arrSimpleSolver2IteVarMap[pNextInfState->nTransitionVar], 0);
			}
			inferences = infBDD->inferences;
//			inferences = inferences->next;
		}

		//Determine if we need to create some WatchedListStates

		arrElts_next = NULL;
		nWatchedListSize = 0;
		if(infBDD != true_ptr && use_SmurfWatchedLists) {
			nNumElts_next = 0;
			unravelBDD(&nNumElts_next, &tempint_max, &tempint, infBDD);
			arrElts_next = (int *)ite_calloc(nNumElts_next, sizeof(int), 9, "arrElts_next");
			for (int i=0;i<nNumElts_next;i++) {
				if (tempint[i]==0 || arrIte2SimpleSolverVarMap[tempint[i]]==0) {
					dE_printf1("\nassigned variable in a BDD in the solver");
					dE_printf3("\nvariable id: %d, true_false=%d\n", tempint[i], variablelist[tempint[i]].true_false);
					exit(1);
				}
				arrElts_next[i] = arrIte2SimpleSolverVarMap[tempint[i]];
			}
			qsort(arrElts_next, nNumElts_next, sizeof(int), revcompfunc);
			
			nWatchedListSize = nNumElts - (nNumElts_next+nNumInferences+1);

			if(nWatchedListSize > 0) { //Some variables dropped out
				int *arrWatchedList = (int *)ite_calloc(nWatchedListSize, sizeof(int), 9, "nWatchedListSize");
				int next=0;
				int wl=0;
				for(int curr=0; curr < nNumElts; curr++) {
					if(arrElts[curr] == arrElts_next[next]) {
						next++; continue;
					}
					if(arrElts[curr] == pCurrState->nTransitionVar)	continue;
					int cont = 0;
					inferences = save_inferences;
					while(inferences!=NULL) { 
						if(inferences->nums[1] != 0) {
							inferences = inferences->next; continue;
						}
						if(arrElts[curr] == abs(inferences->nums[0])) {
							cont=1; break;
						}
						inferences = inferences->next;						
					}
					if(cont == 1) continue;
					
					if(arrElts[curr] != arrElts_next[next])
					  arrWatchedList[wl++] = arrElts[curr];
				}
				assert(next == nNumElts_next);
				assert(wl == nWatchedListSize);
				
				pNextWatchedListState = CreateWatchedListState(arrWatchedList, nWatchedListSize);
				if(nNumInferences == 0)
				  pNextState->pVarIsTrueTransition = pNextWatchedListState;
				else {
					pNextWatchedListState->pVarTransition = pNextState->pVarIsFalseTransition;
					pNextState->pVarIsTrueTransition = pNextWatchedListState;
				}
			}
		}

		if(infBDD->pState != NULL) {
			ite_free((void **)&arrElts_next);
			if(nNumInferences == 0 && nWatchedListSize == 0) pNextState->pVarIsTrueTransition = infBDD->pState; //The transition is True
			else if(nNumInferences == 0) pNextWatchedListState->pVarTransition = infBDD->pState;
			else pNextInfState->pVarTransition = infBDD->pState;
		} else {
			//Recurse on nTransitionVar == True transition
			void *pNext = ReadSmurfStateIntoTable(infBDD, arrElts_next, nNumElts_next);
			assert(pNext!=NULL);
			if(nNumInferences == 0 && nWatchedListSize == 0) pNextState->pVarIsTrueTransition = pNext; //The transition is True
			else if(nNumInferences == 0) pNextWatchedListState->pVarTransition = pNext;
			else pNextInfState->pVarTransition = pNext;
		}
	}
	return (void *)pStartState;
}

void *ReadSmurfStateIntoTable(BDDNode *pCurrentBDD, int *arrElts, int nNumElts) {
	void *pStartState = pCurrentBDD->pState;
	if(pCurrentBDD != true_ptr && pCurrentBDD->pState==NULL)  {
		//If this is the first transition into this SmurfState, mark this SmurfState as Visited      
		
		if(arrElts == NULL) {
			unravelBDD(&nNumElts, &tempint_max, &tempint, pCurrentBDD);
			arrElts = (int *)ite_calloc(nNumElts, sizeof(int), 9, "arrElts");
			for (int i=0;i<nNumElts;i++) {
				if (tempint[i]==0 || arrIte2SimpleSolverVarMap[tempint[i]]==0) {
					dE_printf1("\nassigned variable in a BDD in the solver");
					dE_printf3("\nvariable id: %d, true_false=%d\n", tempint[i], variablelist[tempint[i]].true_false);
					exit(1);
				}
				arrElts[i] = arrIte2SimpleSolverVarMap[tempint[i]];
			}
			qsort(arrElts, nNumElts, sizeof(int), revcompfunc);
		}
		
		int equ_var;
		if(nNumElts >= functionTypeLimits[PLAINOR] &&
			isOR(pCurrentBDD)) {
			//FN_OR
			pStartState = CreateORState(arrElts, nNumElts, pCurrentBDD, (ORStateEntry *)pStartState);
			
			if(((TypeStateEntry *)pStartState)->cType == FN_OR)
			  LSGBORStateSetHeurScores((ORStateEntry *)pStartState);
			else if(((TypeStateEntry *)pStartState)->cType == FN_OR_COUNTER)
			  LSGBORCounterStateSetHeurScores((ORCounterStateEntry *)pStartState);
			//       fprintf(stderr, "v");
		} else if(nNumElts >= functionTypeLimits[PLAINXOR] &&
					 isXOR(pCurrentBDD)) {
			//FN_XOR
			//pStartState = CreateSmurfState(arrElts, nNumElts, pCurrentBDD, (SmurfStateEntry *)pStartState);        
			//LSGBSmurfSetHeurScores((SmurfStateEntry *)pStartState);
			//fprintf(stderr, "+");
			if(use_XORGElim==1) {
				pStartState = CreateXORGElimState(arrElts, nNumElts, pCurrentBDD, (XORGElimStateEntry *)pStartState);
			} else {
				pStartState = CreateXORState(arrElts, nNumElts, pCurrentBDD, (XORStateEntry *)pStartState);
			}
			if(((TypeStateEntry *)pStartState)->cType == FN_XOR || ((TypeStateEntry *)pStartState)->cType == FN_XOR_GELIM)
			  LSGBXORStateSetHeurScores((XORStateEntry *)pStartState);
			else if(((TypeStateEntry *)pStartState)->cType == FN_XOR_COUNTER)
			  LSGBXORCounterStateSetHeurScores((XORCounterStateEntry *)pStartState);
		} else if(0 && nNumElts >= 3 &&//>= functionTypeLimits[PLAINAND] &&
					 (equ_var = isAND_EQU(pCurrentBDD, tempint, nNumElts))!=0) {
			//FN_AND_EQU / FN_OR_EQU
			//pStartState = CreateSmurfState(arrElts, nNumElts, pCurrentBDD, (SmurfStateEntry *)pStartState);        
			//LSGBSmurfSetHeurScores((SmurfStateEntry *)pStartState);
			//       fprintf(stderr, "=");
			pCurrentBDD->pState = pTrueSimpleSmurfState;
			pStartState = pTrueSimpleSmurfState;
			ite_free((void **)&arrElts);
		} else if(0 && nNumElts >= 3 &&// &&//nNumElts >= functionTypeLimits[PLAINAND] &&
					 isMIN_MAX(pCurrentBDD, tempint, nNumElts)) {
			//FN_MINMAX
			//pStartState = CreateSmurfState(arrElts, nNumElts, pCurrentBDD, (SmurfStateEntry *)pStartState);        
			//LSGBSmurfSetHeurScores((SmurfStateEntry *)pStartState);
			//       fprintf(stderr, "M");
			pCurrentBDD->pState = pTrueSimpleSmurfState;
			pStartState = pTrueSimpleSmurfState;
			ite_free((void **)&arrElts);
		}
		else if(0 && nNumElts >= 3 &&//nNumElts >= functionTypeLimits[PLAINAND] &&
				  isNEG_MIN_MAX(pCurrentBDD, tempint, nNumElts)) {
			//FN_NEG_MINMAX
			//pStartState = CreateSmurfState(arrElts, nNumElts, pCurrentBDD, (SmurfStateEntry *)pStartState);        
			//LSGBSmurfSetHeurScores((SmurfStateEntry *)pStartState);
			//       fprintf(stderr, "m");
			pCurrentBDD->pState = pTrueSimpleSmurfState;
			pStartState = pTrueSimpleSmurfState;
			ite_free((void **)&arrElts);
		} else {
			//FN_SMURF
			pStartState = CreateSmurfState(arrElts, nNumElts, pCurrentBDD, (SmurfStateEntry *)pStartState);
			LSGBSmurfSetHeurScores((SmurfStateEntry *)pStartState);
			ite_free((void **)&arrElts);
		}
	}
	return pStartState;
}
