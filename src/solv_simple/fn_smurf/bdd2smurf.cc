#include "sbsat.h"
#include "sbsat_solver.h"
#include "solver.h"

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

void fillWatchedHEAP(int size, int *arrElts) {
	for(int x = 0; x < size; x++) {
		SmurfStateEntry *pCurrState = (SmurfStateEntry *)SimpleSmurfProblemState->pSmurfStatesTableTail;
		SimpleSmurfProblemState->nNumSmurfStateEntries++;
		SimpleSmurfProblemState->pSmurfStatesTableTail = (void *)(pCurrState + 1);
		pCurrState->cType = FN_WATCHED_SMURF;
		pCurrState->nTransitionVar = arrElts[x];
	}
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

void *CreateInferenceStates(BDDNode *infBDD) {
	void *pNextState = NULL;
	InferenceStateEntry *pNextInfState = NULL;
	infer *inferences = infBDD->inferences;
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
		if(nNumInferences == 0) pNextState = infBDD->pState;
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
	}
	
	if(infBDD->pState != NULL) {
		if(nNumInferences == 0) pNextState = infBDD->pState; //The transition is False
		else pNextInfState->pVarTransition = infBDD->pState;
	} else {
		//Recurse on nTransitionVar == False transition
		void *pNext = NULL;
		if(precompute_smurfs) {
			pNext = ReadSmurfStateIntoTable(infBDD, NULL, 0);
			assert(pNext!=NULL);
		}
		if(nNumInferences == 0) pNextState = pNext;
		else { pNextInfState->pVarTransition = pNext; pNextInfState->pInferenceBDD = infBDD; }
	}
	assert(precompute_smurfs==0 || pNextState != NULL);
	return pNextState;
}


void ComputeSmurfInferences(BDDNode *pCurrentBDD, SmurfStateEntry *pCurrState) {
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

	pCurrState->pVarIsFalseTransition = CreateInferenceStates(infBDD);
	
	//Compute the SmurfState w/ nTransitionVar = True
	//Create Inference Transitions
	infBDD = pCurrentBDD;
	infBDD = set_variable(infBDD, arrSimpleSolver2IteVarMap[pCurrState->nTransitionVar], 1);

	pCurrState->pVarIsTrueTransition = CreateInferenceStates(infBDD);
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

		ComputeSmurfInferences(pCurrentBDD, pCurrState);
		
	}
	return (void *)pStartState;
}

void *CreateWatchedSmurfState(int *arrElts, int nNumElts, BDDNode *pCurrentBDD, SmurfStateEntry *pStartState) {
	check_SmurfStatesTableSize((int)sizeof(SmurfStateEntry)*nNumElts);
	ite_counters[SMURF_STATES]+=nNumElts;
	pCurrentBDD->pState = SimpleSmurfProblemState->pSmurfStatesTableTail;
	
	pStartState = (SmurfStateEntry *)SimpleSmurfProblemState->pSmurfStatesTableTail;
	fillWatchedHEAP(nNumElts, arrElts);
	
	for(int nVbleIndex = 0; nVbleIndex < nNumElts-1; nVbleIndex++) {
		SmurfStateEntry *pCurrState = (pStartState+nVbleIndex);
		pCurrState->pNextVarInThisState = (void *)(pStartState+nVbleIndex+1);
	}
		
	for(int nVbleIndex = 0; nVbleIndex < nNumElts; nVbleIndex++) {
		SmurfStateEntry *pCurrState = (pStartState+nVbleIndex);
		pCurrState->pSmurfBDD = pCurrentBDD;
		
		assert(pCurrState->nTransitionVar == arrElts[nVbleIndex]);

      ComputeSmurfInferences(pCurrentBDD, pCurrState);
		
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

		int nInferences = 0;
		if(pCurrentBDD->inferences!=NULL) {
			infer *inferences = pCurrentBDD->inferences;
			while(inferences!=NULL) {
				if(inferences->nums[1] != 0) {
					inferences = inferences->next; continue; 
				}
				nInferences = 1;
				break;
			}
		}		
		
		//Handle any initial inferences
		if(nInferences == 1) {
			pStartState = CreateInferenceStates(pCurrentBDD);
			ite_free((void **)&arrElts);
		} else if(nNumElts >= functionTypeLimits[PLAINOR] &&
			isOR(pCurrentBDD)) {
			//FN_OR
			pStartState = CreateORState(arrElts, nNumElts, pCurrentBDD, (ORStateEntry *)pStartState);
			
			if(((TypeStateEntry *)pStartState)->cType == FN_OR)
			  LSGBORStateSetHeurScores((ORStateEntry *)pStartState);
			else if(((TypeStateEntry *)pStartState)->cType == FN_OR_COUNTER)
			  LSGBORCounterStateSetHeurScores((ORCounterStateEntry *)pStartState);
			//ite_free((void **)&arrElts); This is saved on the OR State
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
			//ite_free((void **)&arrElts); This is saved on the XOR State
		} else if(0 && nNumElts >= functionTypeLimits[PLAINAND] &&
					 (equ_var = isAND_EQU(pCurrentBDD, tempint, nNumElts))!=0) {
			//FN_AND_EQU / FN_OR_EQU
			//pStartState = CreateSmurfState(arrElts, nNumElts, pCurrentBDD, (SmurfStateEntry *)pStartState);        
			//LSGBSmurfSetHeurScores((SmurfStateEntry *)pStartState);
			//       fprintf(stderr, "=");
			pCurrentBDD->pState = pTrueSimpleSmurfState;
			pStartState = pTrueSimpleSmurfState;
			ite_free((void **)&arrElts);
		} else if(nNumElts >= 3 && nNumElts >= functionTypeLimits[MINMAX] &&
					 isMIN_MAX(pCurrentBDD, tempint, nNumElts)) {
			//FN_MINMAX
			pStartState = CreateMINMAXState(arrElts, nNumElts, pCurrentBDD, (MINMAXStateEntry *)pStartState);
			LSGBMINMAXCounterStateSetHeurScores((MINMAXCounterStateEntry *)pStartState);
			//       fprintf(stderr, "M");
		}
		else if(nNumElts >= 3 && nNumElts >= functionTypeLimits[NEG_MINMAX] &&
				  isNEG_MIN_MAX(pCurrentBDD, tempint, nNumElts)) {
			//FN_NEG_MINMAX
			pStartState = CreateNEGMINMAXState(arrElts, nNumElts, pCurrentBDD, (NEGMINMAXStateEntry *)pStartState);
			LSGBNEGMINMAXCounterStateSetHeurScores((NEGMINMAXCounterStateEntry *)pStartState);
			//       fprintf(stderr, "m");
		} else {
			//FN_SMURF
			if(use_SmurfWatchedLists) pStartState = CreateWatchedSmurfState(arrElts, nNumElts, pCurrentBDD, (SmurfStateEntry *)pStartState);
			else pStartState = CreateSmurfState(arrElts, nNumElts, pCurrentBDD, (SmurfStateEntry *)pStartState);
			LSGBSmurfSetHeurScores((SmurfStateEntry *)pStartState);
			ite_free((void **)&arrElts);
		}
	}

	return pStartState;
}
