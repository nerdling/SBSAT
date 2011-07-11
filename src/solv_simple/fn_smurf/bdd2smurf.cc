#include "sbsat.h"
#include "sbsat_solver.h"
#include "solver.h"

ITE_INLINE
int BDDHasInferences(BDDNode *pCurrentBDD) {
	if(pCurrentBDD->inferences!=NULL) {		  
		infer *inferences = pCurrentBDD->inferences;
		while(inferences!=NULL) {
			if(inferences->nums[1] != 0) {
				inferences = inferences->next; continue;
			}
			return 1;
		}
	}
	return 0;
}

void fillHEAP(int index, int size, int *arrElts) {
	SmurfStateEntry *pCurrState = (SmurfStateEntry *)SimpleSmurfProblemState->pSmurfStatesTableTail;
	SimpleSmurfProblemState->nNumSmurfStateEntries++;
	SimpleSmurfProblemState->pSmurfStatesTableTail = (void *)(pCurrState + 1);
	pCurrState->cType = FN_SMURF;
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

void *CreateSmurfState(int *arrElts, int nNumElts, BDDNode *pCurrentBDD, SmurfStateEntry *pStartState) {
	check_SmurfStatesTableSize((int)sizeof(SmurfStateEntry)*nNumElts);
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
		pCurrState->pSmurfBDD = pCurrentBDD;
		
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
		BDDNode *pNextBDD = set_variable(pCurrentBDD, arrSimpleSolver2IteVarMap[pCurrState->nTransitionVar], 0);
		
		if(precompute_smurfs || BDDHasInferences(pNextBDD)) {
			pCurrState->pVarIsFalseTransition = ReadSmurfStateIntoTable(pNextBDD, NULL, 0);
		} else {			  
			pCurrState->pVarIsFalseTransition = NULL;
		}

		//Compute the SmurfState w/ nTransitionVar = True
		//Create Inference Transitions
		pNextBDD = set_variable(pCurrentBDD, arrSimpleSolver2IteVarMap[pCurrState->nTransitionVar], 1);
		
		if(precompute_smurfs || BDDHasInferences(pNextBDD)) {
			pCurrState->pVarIsTrueTransition = ReadSmurfStateIntoTable(pNextBDD, NULL, 0);//CreateInferenceStates(infBDD);
		} else {
			pCurrState->pVarIsTrueTransition = NULL;
		}
	}
	assert(((TypeStateEntry *)pStartState)->cType!=FN_FREE_STATE);
	return (void *)pStartState;
}

void *ReadSmurfStateIntoTable(BDDNode *pCurrentBDD, int *arrElts, int nNumElts) {
	void *pStartState = pCurrentBDD->pState;
	if(pCurrentBDD->pState!=NULL) assert(((TypeStateEntry *)pStartState)->cType!=FN_FREE_STATE);
	if(pCurrentBDD != true_ptr && pCurrentBDD->pState==NULL)  {
		//If this is the first transition into this SmurfState, mark this SmurfState as Visited
		
		int nInferences = BDDHasInferences(pCurrentBDD);

		if(arrElts == NULL && nInferences == 0) {
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
		
		if(nInferences == 1) {
			//FN_INFERENCE
			pStartState = CreateInferenceStates(pCurrentBDD);
			ite_free((void **)&arrElts);
		} else if(nNumElts >= functionTypeLimits[PLAINOR] &&
					 isOR(pCurrentBDD)) {
			//FN_OR
			pStartState = CreateORState(arrElts, nNumElts, pCurrentBDD, (ORStateEntry *)pStartState);
		} else if(nNumElts >= functionTypeLimits[PLAINXOR] &&
					 isXOR(pCurrentBDD)) {
			//FN_XOR
			if(use_XORGElim==1) {
				pStartState = CreateXORGElimState(arrElts, nNumElts, pCurrentBDD, (XORGElimStateEntry *)pStartState);
			} else {
				pStartState = CreateXORState(arrElts, nNumElts, pCurrentBDD, (XORStateEntry *)pStartState);
			}
		} else if(nNumElts >= 3 && nNumElts >= functionTypeLimits[MINMAX] &&
					 isMIN_MAX(pCurrentBDD, tempint, nNumElts)) {
			//FN_MINMAX
			pStartState = CreateMINMAXState(arrElts, nNumElts, pCurrentBDD, (MINMAXStateEntry *)pStartState);
		}
		else if(nNumElts >= 3 && nNumElts >= functionTypeLimits[NEG_MINMAX] &&
				  isNEG_MIN_MAX(pCurrentBDD, tempint, nNumElts)) {
			//FN_NEGMINMAX
			pStartState = CreateNEGMINMAXState(arrElts, nNumElts, pCurrentBDD, (NEGMINMAXStateEntry *)pStartState);
		} else {
			//FN_SMURF
			pStartState = CreateSmurfState(arrElts, nNumElts, pCurrentBDD, (SmurfStateEntry *)pStartState);
			ite_free((void **)&arrElts);
		}
		arrSetStateHeuristicScore[(int)((TypeStateEntry *)pStartState)->cType](pStartState);
	}

	return pStartState;
}
