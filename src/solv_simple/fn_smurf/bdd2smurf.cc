#include "sbsat.h"
#include "sbsat_solver.h"
#include "solver.h"

void fillHEAP(int index, int size, int *arrElts) {
	SmurfStateEntry *CurrState = (SmurfStateEntry *)SimpleSmurfProblemState->pSmurfStatesTableTail;
	SimpleSmurfProblemState->nNumSmurfStateEntries++;
	SimpleSmurfProblemState->pSmurfStatesTableTail = (void *)(CurrState + 1);
	CurrState->cType = FN_SMURF;
	CurrState->ApplyInferenceToState = ApplyInferenceToSmurf;
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

void *CreateWatchedListState(BDDNode *pNextBDD, int *arrElts_curr, int nNumElts_curr) {
	

	
	
}

void *CreateSmurfState(int *arrElts, int nNumElts, BDDNode *pCurrentBDD, SmurfStateEntry *pStartState) {
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
		//fprintf(stderr, "%d, %d, %d: ", nVbleIndex, arrElts[nVbleIndex], arrSimpleSolver2IteVarMap[arrElts[nVbleIndex]]);
		//    printBDDerr(pCurrentBDD);
		//    fprintf(stderr, "\n");
		//    PrintAllSmurfStateEntries();
		   
		SmurfStateEntry *CurrState = findStateInHEAP(pStartState, arrElts[nVbleIndex]);
		
		assert(CurrState->nTransitionVar == arrElts[nVbleIndex]);

		/*
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
		*/

		//Compute the SmurfState w/ nTransitionVar = False
		//Create Inference Transitions
		BDDNode *infBDD = pCurrentBDD;
		infBDD = set_variable(infBDD, arrSimpleSolver2IteVarMap[CurrState->nTransitionVar], 0);
		SmurfStateEntry *NextState = CurrState;
		InferenceStateEntry *NextInfState = NULL;
		//Follow positive and negative inferences
		infer *inferences = infBDD->inferences;
		bool bIsInference = 0;
		while(inferences!=NULL) { 
			if(inferences->nums[1] != 0) {
				inferences = inferences->next; continue; 
			}
			
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
				NextInfState->nTransitionVar = arrIte2SimpleSolverVarMap[inferences->nums[0]];
				NextInfState->bPolarity = 1;
				infBDD = set_variable(infBDD, arrSimpleSolver2IteVarMap[NextInfState->nTransitionVar], 1);
			} else {						
				NextInfState->nTransitionVar = arrIte2SimpleSolverVarMap[-inferences->nums[0]];
				NextInfState->bPolarity = 0;
				infBDD = set_variable(infBDD, arrSimpleSolver2IteVarMap[NextInfState->nTransitionVar], 0);
			}			
			inferences = infBDD->inferences;
		}

		//Determine variable drops (bad comment - fix later)
		
		int nNumElts_next = 0;
		unravelBDD(&nNumElts_next, &tempint_max, &tempint, infBDD);
		int *arrElts_next = (int *)ite_calloc(nNumElts_next, sizeof(int), 9, "arrElts_next");
		for (int i=0;i<nNumElts_next;i++) {
			if (tempint[i]==0 || arrIte2SimpleSolverVarMap[tempint[i]]==0) {
				dE_printf1("\nassigned variable in a BDD in the solver");
				dE_printf3("\nvariable id: %d, true_false=%d\n", tempint[i], variablelist[tempint[i]].true_false);
				exit(1);
			}
			arrElts_next[i] = arrIte2SimpleSolverVarMap[tempint[i]];
		}
		qsort(arrElts_next, nNumElts_next, sizeof(int), revcompfunc);

		
		if(infBDD->pState != NULL) {
			ite_free((void **)&arrElts_next);
			if(bIsInference == 0) NextState->pVarIsFalseTransition = infBDD->pState; //The transition is False
			else NextInfState->pVarTransition = infBDD->pState;
		} else {
			//Recurse on nTransitionVar == False transition
			void *pNext = ReadSmurfStateIntoTable(infBDD, arrElts_next, nNumElts_next); //HERE
			assert(pNext!=NULL);
			if(bIsInference == 0) NextState->pVarIsFalseTransition = pNext; //The transition is False
			else NextInfState->pVarTransition = pNext;
		}
		
		//Compute the SmurfState w/ nTransitionVar = True
		//Create Inference Transitions
		infBDD = pCurrentBDD;
		infBDD = set_variable(infBDD, arrSimpleSolver2IteVarMap[CurrState->nTransitionVar], 1);
		NextState = CurrState;
		NextInfState = NULL;
		//Follow positive and negative inferences
		inferences = infBDD->inferences;
		bIsInference = 0;
		while(inferences!=NULL) {
			if(inferences->nums[1] != 0) {
				inferences = inferences->next; continue; 
			}
			
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
				NextInfState->nTransitionVar = arrIte2SimpleSolverVarMap[inferences->nums[0]];
				NextInfState->bPolarity = 1;
				infBDD = set_variable(infBDD, arrSimpleSolver2IteVarMap[NextInfState->nTransitionVar], 1);
			} else {						
				NextInfState->nTransitionVar = arrIte2SimpleSolverVarMap[-inferences->nums[0]];
				NextInfState->bPolarity = 0;
				infBDD = set_variable(infBDD, arrSimpleSolver2IteVarMap[NextInfState->nTransitionVar], 0);
			}
			inferences = infBDD->inferences;
		}
		
		if(infBDD->pState != NULL) {
			if(bIsInference == 0) NextState->pVarIsTrueTransition = infBDD->pState; //The transition is True
			else NextInfState->pVarTransition = infBDD->pState;
		} else {
			//Recurse on nTransitionVar == True transition
			void *pNext = ReadSmurfStateIntoTable(infBDD, NULL, 0); //HERE
			assert(pNext!=NULL);
			if(bIsInference == 0) NextState->pVarIsTrueTransition = pNext; //The transition is True
			else NextInfState->pVarTransition = pNext;
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
