#include "sbsat.h"
#include "sbsat_solver.h"
#include "solver.h"

int ApplyInferenceToXOR(int nBranchVar, bool bBVPolarity, int nSmurfNumber, void **arrSmurfStates) {
	XORStateEntry *pXORState = (XORStateEntry *)arrSmurfStates[nSmurfNumber];	

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
	int nInference = -1;
	for(int x = 0; x < pXORState->nSize; x++) {
		int nPrevInfLevel = SimpleSmurfProblemState->arrInferenceDeclaredAtLevel[pXORState->pnTransitionVars[x]];
		//fprintf(stderr, "%d %d<%d x=%d var=%d pol=%d\n", nBranchVar, nPrevInfLevel, nInfQueueLevel, x, pXORState->pnTransitionVars[x], pXORState->bPolarity[x]);
		//if(x == index) continue;
		if(nPrevInfLevel <= 0) {
			if(SimpleSmurfProblemState->arrInferenceQueue[-nPrevInfLevel] > 0) bParity = 1-bParity;
		} else if(nPrevInfLevel <= nInfQueueLevel) {
			assert(SimpleSmurfProblemState->arrInferenceQueue[nPrevInfLevel] != 0);
			if(SimpleSmurfProblemState->arrInferenceQueue[nPrevInfLevel] > 0) bParity = 1-bParity;
		} else {
			//if(nInference != 0) return 1;
			//assert(nInference==-1); //A variable was earlier inferred that does not exist in this part of the Smurf.
			//if(nInference != -1) {return 1;}
			nInference = x;
		}
	}

	//assert(nInference != 0 || bParity);
	//if(nInference == -1) {return 1;}
	
	//Inference is not in inference queue, insert it.
	if(EnqueueInference(pXORState->pnTransitionVars[nInference], bParity) == 0) return 0;
	arrSmurfStates[nSmurfNumber] = pTrueSimpleSmurfState;
	SimpleSmurfProblemState->arrSmurfStack[SimpleSmurfProblemState->nCurrSearchTreeLevel].nNumSmurfsSatisfied++;
	
	d7_printf3("      XORSmurf %d transitioned to state %x\n", nSmurfNumber, arrSmurfStates[nSmurfNumber]);
	return 1;
}

int ApplyInferenceToXORCounter(int nBranchVar, bool bBVPolarity, int nSmurfNumber, void **arrSmurfStates) {
//	arrSmurfStates[nSmurfNumber] = ((XORCounterStateEntry *)arrSmurfStates[nSmurfNumber])->pTransition;

	XORCounterStateEntry *pXORCounterState = (XORCounterStateEntry *)arrSmurfStates[nSmurfNumber];
	XORStateEntry *pXORState = (XORStateEntry *)pXORCounterState->pXORState;
	
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

	arrSmurfStates[nSmurfNumber] = pXORCounterState->pTransition;

//	if(arrSmurfStates[nSmurfNumber] == pTrueSimpleSmurfState) //Can't happen here
//	  SimpleSmurfProblemState->arrSmurfStack[SimpleSmurfProblemState->nCurrSearchTreeLevel].nNumSmurfsSatisfied++;
	
	d7_printf3("      XORCounterSmurf %d transitioned to state %x\n", nSmurfNumber, arrSmurfStates[nSmurfNumber]);
	return 1;
}
