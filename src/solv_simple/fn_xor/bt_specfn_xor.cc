#include "sbsat.h"
#include "sbsat_solver.h"
#include "solver.h"

void SetVisitedXORState(void *pState, int value) {
   XORStateEntry *pXORState = (XORStateEntry *)pState;
   assert(pXORState->cType == FN_XOR);
   if(pXORState->visited != value) {
      d7_printf3("Marking visited=%d of XOR State %p\n", value, pXORState);
      pXORState->visited = value;
   }
}

void SetVisitedXORCounterState(void *pState, int value) {
   XORCounterStateEntry *pXORCounterState = (XORCounterStateEntry *)pState;
   assert(pXORCounterState->cType == FN_XOR_COUNTER);
   SetVisitedXORState(pXORCounterState->pXORState, value);
   XORCounterStateEntry *tmp_xor = pXORCounterState;
   while(tmp_xor != NULL && tmp_xor->cType == FN_XOR_COUNTER) {
      d7_printf3("Marking visited=%d of XORCounter State %p\n", value, tmp_xor);
      tmp_xor->visited = value;
      tmp_xor = (XORCounterStateEntry *)tmp_xor->pTransition;
   }
}

void SetVisitedXORGElimState(void *pState, int value) {
   XORGElimStateEntry *pXORGElimState = (XORGElimStateEntry *)pState;
   assert(pXORGElimState->cType == FN_XOR_GELIM);
   if(pXORGElimState->visited != value) {
      d7_printf3("Marking visited=%d of XORGElim State %p\n", value, pXORGElimState);
      pXORGElimState->visited = value;
   }
}

int ApplyInferenceToXOR(int nBranchVar, bool bBVPolarity, int nSmurfNumber, void **arrSmurfStates) {
	XORStateEntry *pXORState = (XORStateEntry *)arrSmurfStates[nSmurfNumber];	

	assert(pXORState->pStateOwner == nSmurfNumber);
	
	//Check to see if nBranchVar exists in this constraint.
	int index = 0;
	int size = pXORState->nSize;
	int prev = 0;
	int var = pXORState->pnTransitionVars[index+(size/2)];
	while(var != nBranchVar) {
		//fprintf(stderr, "%d(%d) - ", pORState->pnTransitionVars[index+(size/2)], size);
		if(var == prev) break;
		prev = var;
		if(var < nBranchVar) {
			size = size/2;
		} else {
			index = index+(size/2)+1;
			size = (size-(size/2))-1;
			if(index+(size/2) == pXORState->nSize) break;
		}
		var = pXORState->pnTransitionVars[index+(size/2)];
	}
	if(var != nBranchVar) return 1; //Var does not exist in this Smurf

	int nInfQueueLevel = abs(SimpleSmurfProblemState->arrInferenceDeclaredAtLevel[nBranchVar]);

	bool bParity = pXORState->bParity;
	int infer_var = 0;
	for(int x = 0; x < pXORState->nSize; x++) {
		int nPrevInfLevel = abs(SimpleSmurfProblemState->arrInferenceDeclaredAtLevel[pXORState->pnTransitionVars[x]]);
		//fprintf(stderr, "%d %d<%d x=%d var=%d pol=%d\n", nBranchVar, nPrevInfLevel, nInfQueueLevel, x, pXORState->pnTransitionVars[x], pXORState->bPolarity[x]);
		//if(x == index) continue;
		if(nPrevInfLevel <= nInfQueueLevel) {
			assert(SimpleSmurfProblemState->arrInferenceQueue[nPrevInfLevel] != 0);
			if(SimpleSmurfProblemState->arrInferenceQueue[nPrevInfLevel] > 0) bParity = 1-bParity;
		} else {
			//if(infer_var != 0) return 1;
			//assert(infer_var==-1); //A variable was earlier inferred that does not exist in this part of the Smurf.
			//if(infer_var != -1) {return 1;}
			infer_var = x;
		}
	}

	//assert(infer_var != 0 || bParity);
	//if(infer_var == -1) {return 1;}
	
	//Inference is not in inference queue, insert it.
	pXORState->nLemmaLiteral = bBVPolarity?-nBranchVar:nBranchVar; //Polarity for lemma literals is reversed

	if(TypeState_InferVar((TypeStateEntry *)pXORState,
                         pXORState->pnTransitionVars[infer_var],
                         bParity,
                         nSmurfNumber,
                         INF_SPEC_FN_XOR) == 0) return 0;

	arrSmurfStates[nSmurfNumber] = pTrueSimpleSmurfState;
	SimpleSmurfProblemState->arrSmurfStack[SimpleSmurfProblemState->nCurrSearchTreeLevel].nNumSmurfsSatisfied++;
	
	d7_printf3("      XORSmurf %d transitioned to state %p\n", nSmurfNumber, arrSmurfStates[nSmurfNumber]);
	return 1;
}

int ApplyInferenceToXORCounter(int nBranchVar, bool bBVPolarity, int nSmurfNumber, void **arrSmurfStates) {
//	arrSmurfStates[nSmurfNumber] = ((XORCounterStateEntry *)arrSmurfStates[nSmurfNumber])->pTransition;

	XORCounterStateEntry *pXORCounterState = (XORCounterStateEntry *)arrSmurfStates[nSmurfNumber];
	XORStateEntry *pXORState = (XORStateEntry *)pXORCounterState->pXORState;

	assert(pXORCounterState->pStateOwner == nSmurfNumber);
	
	int index = 0;
	int size = pXORState->nSize;
	int prev = 0;
	int var = pXORState->pnTransitionVars[index+(size/2)];
	while(var != nBranchVar) {
		//fprintf(stderr, "%d(%d) - ", pXORState->pnTransitionVars[index+(size/2)], size);
		if(var == prev) break;
		prev = var;
		if(var < nBranchVar) {
			size = size/2;
		} else {
			index = index+(size/2)+1;
			size = (size-(size/2))-1;
			if(index+(size/2) == pXORState->nSize) break;
		}
		var = pXORState->pnTransitionVars[index+(size/2)];
	}
	if(var != nBranchVar) return 1; //Var does not exist in this Smurf

	pXORCounterState->nLemmaLiteral = bBVPolarity?-nBranchVar:nBranchVar; //Polarity for lemma literals is reversed
	((TypeStateEntry *)pXORCounterState->pTransition)->pPreviousState = pXORCounterState; //This line can be precomputed -- SEAN!!!
	((TypeStateEntry *)pXORCounterState->pTransition)->pStateOwner = nSmurfNumber;
	arrSmurfStates[nSmurfNumber] = pXORCounterState->pTransition;

//	if(arrSmurfStates[nSmurfNumber] == pTrueSimpleSmurfState) //Can't happen here
//	  SimpleSmurfProblemState->arrSmurfStack[SimpleSmurfProblemState->nCurrSearchTreeLevel].nNumSmurfsSatisfied++;
	
	d7_printf3("      XORCounterSmurf %d transitioned to state %p\n", nSmurfNumber, arrSmurfStates[nSmurfNumber]);
	return 1;
}
