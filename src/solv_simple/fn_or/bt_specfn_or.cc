#include "sbsat.h"
#include "sbsat_solver.h"
#include "solver.h"

void SetVisitedORState(void *pState, int value) {
   ORStateEntry *pORState = (ORStateEntry *)pState;
   assert(pORState->cType == FN_OR);
   if(pORState->visited != value) {
      d7_printf3("Marking visited=%d of OR State %p\n", value, pORState);
      pORState->visited = value;
   }   
}

void SetVisitedORCounterState(void *pState, int value) {
   ORCounterStateEntry *pORCounterState = (ORCounterStateEntry *)pState;
   assert(pORCounterState->cType == FN_OR_COUNTER);
   SetVisitedORState(pORCounterState->pORState, value);
   ORCounterStateEntry *tmp_or = pORCounterState;
   while(tmp_or != NULL && tmp_or->cType == FN_OR_COUNTER) {
      d7_printf3("Marking visited=%d of ORCounter State %p\n", value, tmp_or);
      tmp_or->visited = value;
      tmp_or = (ORCounterStateEntry *)tmp_or->pTransition;
   }
}

int ApplyInferenceToOR(int nBranchVar, bool bBVPolarity, int nSmurfNumber, void **arrSmurfStates) {
	ORStateEntry *pORState = (ORStateEntry *)arrSmurfStates[nSmurfNumber];	

	assert(pORState->pStateOwner == nSmurfNumber);
	
	//Check to see if nBranchVar is inferred correctly.
	int index = 0;
	int size = pORState->nSize;
	int prev = 0;
	int var = pORState->pnTransitionVars[index+(size/2)];
	while(var != nBranchVar) {
		//fprintf(stderr, "%d(%d) - ", pORState->pnTransitionVars[index+(size/2)], size);
		if(var == prev) break;
		prev = var;
		if(var < nBranchVar) {
			size = size/2;
		} else {
			index = index+(size/2)+1;
			size = (size-(size/2))-1;
			if(index+(size/2) == pORState->nSize) break;
		}
		var = pORState->pnTransitionVars[index+(size/2)];
	}
	if(var != nBranchVar) return 1; //Var does not exist in this Smurf
	index = index+(size/2);
	
	if(pORState->bPolarity[index] == bBVPolarity) { 
		arrSmurfStates[nSmurfNumber] = pTrueSimpleSmurfState;
		SimpleSmurfProblemState->arrSmurfStack[SimpleSmurfProblemState->nCurrSearchTreeLevel].nNumSmurfsSatisfied++;
	} else {
		int nInfQueueLevel = abs(SimpleSmurfProblemState->arrInferenceDeclaredAtLevel[nBranchVar]);
		int infer_var = 0;
		for(int x = 0; x < pORState->nSize; x++) {
			int nPrevInfLevel = abs(SimpleSmurfProblemState->arrInferenceDeclaredAtLevel[pORState->pnTransitionVars[x]]);
//			fprintf(stderr, "%d %d<%d x=%d var=%d pol=%d\n", nBranchVar, nPrevInfLevel, nInfQueueLevel, x, pORState->pnTransitionVars[x], pORState->bPolarity[x]);
//			if(x == index) continue;
			if(nPrevInfLevel <= nInfQueueLevel) {
				assert((SimpleSmurfProblemState->arrInferenceQueue[nPrevInfLevel] == 0) ||
						 ((SimpleSmurfProblemState->arrInferenceQueue[nPrevInfLevel] > 0) != pORState->bPolarity[x]));
				continue;
			}

			infer_var = x;
		}
		//Inference is not in inference queue, insert it.

		pORState->nLemmaLiteral = bBVPolarity?-nBranchVar:nBranchVar; //Polarity for lemma literals is reversed

		if(TypeState_InferVar((TypeStateEntry *)pORState,
                            pORState->pnTransitionVars[infer_var],
                            pORState->bPolarity[infer_var],
                            nSmurfNumber,
                            INF_SPEC_FN_OR) == 0) return 0;

		arrSmurfStates[nSmurfNumber] = pTrueSimpleSmurfState;
		SimpleSmurfProblemState->arrSmurfStack[SimpleSmurfProblemState->nCurrSearchTreeLevel].nNumSmurfsSatisfied++;
	}

	d7_printf3("      ORSmurf %d transitioned to state %p\n", nSmurfNumber, arrSmurfStates[nSmurfNumber]);
	return 1;
}

int ApplyInferenceToORCounter(int nBranchVar, bool bBVPolarity, int nSmurfNumber, void **arrSmurfStates) {
	ORCounterStateEntry *pORCounterState = (ORCounterStateEntry *)arrSmurfStates[nSmurfNumber];
	ORStateEntry *pORState = (ORStateEntry *)pORCounterState->pORState;

	assert(pORCounterState->pStateOwner == nSmurfNumber);
	
	int index = 0;
	int size = pORState->nSize;
	int prev = 0;
	int var = pORState->pnTransitionVars[index+(size/2)];
	while(var != nBranchVar) {
		//fprintf(stderr, "%d(%d) - ", pORState->pnTransitionVars[index+(size/2)], size);
		if(var == prev) break;
		prev = var;
		if(var < nBranchVar) {
			size = size/2;
		} else {
			index = index+(size/2)+1;
			size = (size-(size/2))-1;
			if(index+(size/2) == pORState->nSize) break;
		}
		var = pORState->pnTransitionVars[index+(size/2)];
	}
	if(var != nBranchVar) return 1; //Var does not exist in this Smurf
	index = index+(size/2);
	
	if(pORState->bPolarity[index] == bBVPolarity) { 
		arrSmurfStates[nSmurfNumber] = pTrueSimpleSmurfState;
		SimpleSmurfProblemState->arrSmurfStack[SimpleSmurfProblemState->nCurrSearchTreeLevel].nNumSmurfsSatisfied++;
	} else {
		pORCounterState->nLemmaLiteral = bBVPolarity?-nBranchVar:nBranchVar; //Polarity for lemma literals is reversed
		((TypeStateEntry *)pORCounterState->pTransition)->pPreviousState = pORCounterState; //This line can be precomputed -- SEAN!!!
		((TypeStateEntry *)pORCounterState->pTransition)->pStateOwner = nSmurfNumber;
		arrSmurfStates[nSmurfNumber] = pORCounterState->pTransition;
	}	
	d7_printf3("      ORCounterSmurf %d transitioned to state %p\n", nSmurfNumber, arrSmurfStates[nSmurfNumber]);
	return 1;
}
