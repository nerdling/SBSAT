#include "sbsat.h"
#include "sbsat_solver.h"
#include "solver.h"

ITE_INLINE
int MINMAXState_InferWithLemma(MINMAXCounterStateEntry *pMINMAXCounterState, int infer_var, bool bPolarity, int nSmurfNumber) {
	int nInfVar = pMINMAXCounterState->pMINMAXState->pnTransitionVars[infer_var];//, pMINMAXState->bPolarity[infer_var]
	int nInfQueueHead = SimpleSmurfProblemState->arrSmurfStack[SimpleSmurfProblemState->nCurrSearchTreeLevel].nNumFreeVars;
	int nPrevInfLevel = abs(SimpleSmurfProblemState->arrInferenceDeclaredAtLevel[nInfVar]);
	if(nPrevInfLevel < nInfQueueHead) {
		if((SimpleSmurfProblemState->arrInferenceQueue[nPrevInfLevel] > 0) != bPolarity) {
			//Conflict
			//Add a lemma to reference this conflict.
			create_clause_from_SmurfState(bPolarity?nInfVar:-nInfVar, (TypeStateEntry *)pMINMAXCounterState,
													SimpleSmurfProblemState->arrReverseOccurenceList[nSmurfNumber][0].var,
													&(SimpleSmurfProblemState->pConflictClause.clause),
													&(SimpleSmurfProblemState->pConflictClause.max_size));
			int ret = EnqueueInference(nInfVar, bPolarity, INF_SPEC_FN_MINMAX);
			assert(ret == 0);
			return 0;
		} else {
			//Lit already assigned.
			d7_printf2("      Inference %d already inferred\n", bPolarity?nInfVar:-nInfVar);
			return 1;
		}
	} else {
		//Add a lemma as reference to this inference.
		create_clause_from_SmurfState(bPolarity?nInfVar:-nInfVar, (TypeStateEntry *)pMINMAXCounterState,
												SimpleSmurfProblemState->arrReverseOccurenceList[nSmurfNumber][0].var,
												&(SimpleSmurfProblemState->arrInferenceLemmas[nInfVar].clause),
												&(SimpleSmurfProblemState->arrInferenceLemmas[nInfVar].max_size));
		if(EnqueueInference(nInfVar, bPolarity, INF_SPEC_FN_MINMAX) == 0) return 0;
	}
	return 1;
}


int ApplyInferenceToMINMAX(int nBranchVar, bool bBVPolarity, int nSmurfNumber, void **arrSmurfStates) {
	//I don't think this should happen w/ the current setup...yet - use for watched literals
	assert(0);
}

int ApplyInferenceToMINMAXCounter(int nBranchVar, bool bBVPolarity, int nSmurfNumber, void **arrSmurfStates) {
	MINMAXCounterStateEntry *pMINMAXCounterState = (MINMAXCounterStateEntry *)arrSmurfStates[nSmurfNumber];
	MINMAXStateEntry *pMINMAXState = (MINMAXStateEntry *)pMINMAXCounterState->pMINMAXState;
	
	assert(pMINMAXCounterState->pStateOwner == nSmurfNumber);
	
	int index = 0;
	int size = pMINMAXState->nSize;
	int prev = 0;
	int var = pMINMAXState->pnTransitionVars[index+(size/2)];
	while(var != nBranchVar) {
		//fprintf(stderr, "%d(%d) - ", pMINMAXState->pnTransitionVars[index+(size/2)], size);
		if(var == prev) break;
		prev = var;
		if(var < nBranchVar) {
			size = size/2;
		} else {
			index = index+(size/2)+1;
			size = (size-(size/2))-1;
			if(index+(size/2) == pMINMAXState->nSize) break;
		}
		var = pMINMAXState->pnTransitionVars[index+(size/2)];
	}
	if(var != nBranchVar) return 1; //Var does not exist in this Smurf
	index = index+(size/2);

	if(bBVPolarity) {
		((MINMAXCounterStateEntry *)(pMINMAXCounterState->pTransition))->nNumTrue = pMINMAXCounterState->nNumTrue+1;
	} else {
		((MINMAXCounterStateEntry *)(pMINMAXCounterState->pTransition))->nNumTrue = pMINMAXCounterState->nNumTrue;
	}	

	pMINMAXCounterState->nLemmaLiteral = bBVPolarity?-nBranchVar:nBranchVar; //Polarity for lemma literals is reversed
	pMINMAXCounterState = (MINMAXCounterStateEntry *)(pMINMAXCounterState->pTransition);
	
//	PrintMINMAXCounterStateEntry(pMINMAXCounterState);
//	PrintMINMAXStateEntry(pMINMAXState);

	if(pMINMAXCounterState->nNumTrue >= pMINMAXState->nMin && pMINMAXCounterState->nNumTrue <= pMINMAXState->nMax &&
		(pMINMAXCounterState->nVarsLeft <= (pMINMAXState->nMax - pMINMAXCounterState->nNumTrue))) {
		arrSmurfStates[nSmurfNumber] = pTrueSimpleSmurfState;
		SimpleSmurfProblemState->arrSmurfStack[SimpleSmurfProblemState->nCurrSearchTreeLevel].nNumSmurfsSatisfied++;
	} else if((pMINMAXCounterState->nNumTrue < pMINMAXState->nMin) &&
				 (pMINMAXCounterState->nVarsLeft == (pMINMAXState->nMin - pMINMAXCounterState->nNumTrue))) {
		//Infer remaining variables to True
		int nInfQueueLevel = abs(SimpleSmurfProblemState->arrInferenceDeclaredAtLevel[nBranchVar]);
		for(int x = 0; x < pMINMAXState->nSize; x++) {
			int nPrevInfLevel = SimpleSmurfProblemState->arrInferenceDeclaredAtLevel[pMINMAXState->pnTransitionVars[x]];
			if(nPrevInfLevel <= nInfQueueLevel)	continue;
			//Inference is not in inference queue, insert it.
			if(use_lemmas) {
				if(MINMAXState_InferWithLemma((MINMAXCounterStateEntry *)arrSmurfStates[nSmurfNumber], x, 1, nSmurfNumber) == 0) return 0;
			} else {
				if(EnqueueInference(pMINMAXState->pnTransitionVars[x], 1, INF_SPEC_FN_MINMAX) == 0) return 0;
			}

		}
		arrSmurfStates[nSmurfNumber] = pTrueSimpleSmurfState;
		SimpleSmurfProblemState->arrSmurfStack[SimpleSmurfProblemState->nCurrSearchTreeLevel].nNumSmurfsSatisfied++;
	} else if(pMINMAXCounterState->nNumTrue == pMINMAXState->nMax) {
		//Infer remaining variables to False
		int nInfQueueLevel = abs(SimpleSmurfProblemState->arrInferenceDeclaredAtLevel[nBranchVar]);
		for(int x = 0; x < pMINMAXState->nSize; x++) {
			int nPrevInfLevel = SimpleSmurfProblemState->arrInferenceDeclaredAtLevel[pMINMAXState->pnTransitionVars[x]];
			if(nPrevInfLevel <= nInfQueueLevel)	continue;
			//Inference is not in inference queue, insert it.

			if(use_lemmas) {
				if(MINMAXState_InferWithLemma((MINMAXCounterStateEntry *)arrSmurfStates[nSmurfNumber], x, 0, nSmurfNumber) == 0) return 0;
			} else {
				if(EnqueueInference(pMINMAXState->pnTransitionVars[x], 0, INF_SPEC_FN_MINMAX) == 0) return 0;
			}
		}
		arrSmurfStates[nSmurfNumber] = pTrueSimpleSmurfState;
		SimpleSmurfProblemState->arrSmurfStack[SimpleSmurfProblemState->nCurrSearchTreeLevel].nNumSmurfsSatisfied++;
	} else {
		pMINMAXCounterState->pPreviousState = (MINMAXCounterStateEntry *)arrSmurfStates[nSmurfNumber]; //This line can be precomputed -- SEAN!!!
		pMINMAXCounterState->pStateOwner = nSmurfNumber;
		arrSmurfStates[nSmurfNumber] = pMINMAXCounterState;
	}
	
	d7_printf3("      MINMAXCounterSmurf %d transitioned to state %x\n", nSmurfNumber, arrSmurfStates[nSmurfNumber]);
	return 1;
}
