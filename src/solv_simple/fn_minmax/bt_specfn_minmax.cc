#include "sbsat.h"
#include "sbsat_solver.h"
#include "solver.h"

void SetVisitedMINMAXState(void *pState, int value) {
   MINMAXStateEntry *pMINMAXState = (MINMAXStateEntry *)pState;
   assert(pMINMAXState->cType == FN_MINMAX);
   if(pMINMAXState->visited != value) {
      d7_printf3("Marking visited=%d of MINMAX State %p\n", value, pMINMAXState);
      pMINMAXState->visited = value;
   }
}

void SetVisitedMINMAXCounterState(void *pState, int value) {
   MINMAXCounterStateEntry *pMINMAXCounterState = (MINMAXCounterStateEntry *)pState;
   assert(pMINMAXCounterState->cType == FN_MINMAX_COUNTER);
   SetVisitedMINMAXState(pMINMAXCounterState->pMINMAXState, value);
   MINMAXCounterStateEntry *tmp_minmax = pMINMAXCounterState;
   while(tmp_minmax != NULL && tmp_minmax->cType == FN_MINMAX_COUNTER) {
      d7_printf3("Marking visited=%d of MINMAXCounter State %p\n", value, tmp_minmax);
      tmp_minmax->visited = value;
      tmp_minmax = (MINMAXCounterStateEntry *)tmp_minmax->pTransition;
   }
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
			if(TypeState_InferVar((TypeStateEntry *)arrSmurfStates[nSmurfNumber],
                               pMINMAXCounterState->pMINMAXState->pnTransitionVars[x],
                               1,
                               nSmurfNumber,
                               INF_SPEC_FN_MINMAX) == 0) return 0;
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

         if(TypeState_InferVar((TypeStateEntry *)arrSmurfStates[nSmurfNumber],
                               pMINMAXCounterState->pMINMAXState->pnTransitionVars[x],
                               0,
                               nSmurfNumber,
                               INF_SPEC_FN_MINMAX) == 0) return 0;
		}
		arrSmurfStates[nSmurfNumber] = pTrueSimpleSmurfState;
		SimpleSmurfProblemState->arrSmurfStack[SimpleSmurfProblemState->nCurrSearchTreeLevel].nNumSmurfsSatisfied++;
	} else {
		pMINMAXCounterState->pPreviousState = (MINMAXCounterStateEntry *)arrSmurfStates[nSmurfNumber]; //This line can be precomputed -- SEAN!!!
		pMINMAXCounterState->pStateOwner = nSmurfNumber;
		arrSmurfStates[nSmurfNumber] = pMINMAXCounterState;
	}
	
	d7_printf3("      MINMAXCounterSmurf %d transitioned to state %p\n", nSmurfNumber, arrSmurfStates[nSmurfNumber]);
	return 1;
}
