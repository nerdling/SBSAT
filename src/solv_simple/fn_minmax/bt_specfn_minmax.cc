#include "sbsat.h"
#include "sbsat_solver.h"
#include "solver.h"

int ApplyInferenceToMINMAX(int nBranchVar, bool bBVPolarity, int nSmurfNumber, void **arrSmurfStates) {
//I don't think this should happen w/ the current setup...yet - use for watched literals
assert(0);
	MINMAXStateEntry *pMINMAXState = (MINMAXStateEntry *)arrSmurfStates[nSmurfNumber];	
	
	//Check to see if nBranchVar is inferred correctly.
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
	
	if(pMINMAXState->bPolarity[index] == bBVPolarity) {
		arrSmurfStates[nSmurfNumber] = pTrueSimpleSmurfState;
		SimpleSmurfProblemState->arrSmurfStack[SimpleSmurfProblemState->nCurrSearchTreeLevel].nNumSmurfsSatisfied++;
	} else {
		int nInfQueueLevel = abs(SimpleSmurfProblemState->arrInferenceDeclaredAtLevel[nBranchVar]);
		for(int x = 0; x < pMINMAXState->nSize; x++) {
			int nPrevInfLevel = SimpleSmurfProblemState->arrInferenceDeclaredAtLevel[pMINMAXState->pnTransitionVars[x]];
//			fprintf(stderr, "%d %d<%d x=%d var=%d pol=%d\n", nBranchVar, nPrevInfLevel, nInfQueueLevel, x, pMINMAXState->pnTransitionVars[x], pMINMAXState->bPolarity[x]);
//			if(x == index) continue;
			if(nPrevInfLevel <= 0) continue;
			if(nPrevInfLevel <= nInfQueueLevel) {
				assert((SimpleSmurfProblemState->arrInferenceQueue[nPrevInfLevel] == 0) ||
						 ((SimpleSmurfProblemState->arrInferenceQueue[nPrevInfLevel] > 0) != pMINMAXState->bPolarity[x]));
				continue;
			}
			
			//Inference is not in inference queue, insert it.
			if(EnqueueInference(pMINMAXState->pnTransitionVars[x], pMINMAXState->bPolarity[x]) == 0) return 0;
			arrSmurfStates[nSmurfNumber] = pTrueSimpleSmurfState;
			SimpleSmurfProblemState->arrSmurfStack[SimpleSmurfProblemState->nCurrSearchTreeLevel].nNumSmurfsSatisfied++;
			break;
		}
	}
	d7_printf3("      MINMAXSmurf %d transitioned to state %x\n", nSmurfNumber, arrSmurfStates[nSmurfNumber]);
	return 1;
}

int ApplyInferenceToMINMAXCounter(int nBranchVar, bool bBVPolarity, int nSmurfNumber, void **arrSmurfStates) {
	MINMAXCounterStateEntry *pMINMAXCounterState = (MINMAXCounterStateEntry *)arrSmurfStates[nSmurfNumber];
	MINMAXStateEntry *pMINMAXState = (MINMAXStateEntry *)pMINMAXCounterState->pMINMAXState;

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
		pMINMAXCounterState->pTransition->nNumTrue = pMINMAXCounterState->nNumTrue+1;
	}

	pMINMAXCounterState = pMINMAXCounterState->pTransition;
	
	if(pMINMAXCounterState->nNumTrue >= pMINMAXState->nMin && pMINMAXCounterState->nNumTrue <= pMINMAXState->nMax &&
		(pMINMAXCounterState->nVarsLeft <= (pMINMAXState->nMax - pMINMAXState->nNumTrue))) {
		arrSmurfStates[nSmurfNumber] = pTrueSimpleSmurfState;
		SimpleSmurfProblemState->arrSmurfStack[SimpleSmurfProblemState->nCurrSearchTreeLevel].nNumSmurfsSatisfied++;
	} else if((pMINMAXCounterState->nNumTrue < pMINMAXState->nMin) &&
				 (pMINMAXCounterState->nVarsLeft == (pMINMAXState->nMin - pMINMAXCounterState->nNumTrue))) {
		//Infer remaining variables to True
		for(int x = 0; x < pMINMAXState->nSize; x++) {
			int nPrevInfLevel = SimpleSmurfProblemState->arrInferenceDeclaredAtLevel[pMINMAXState->pnTransitionVars[x]];
			if(nPrevInfLevel <= nInfQueueLevel)	continue;
			//Inference is not in inference queue, insert it.
			if(EnqueueInference(pMINMAXState->pnTransitionVars[x], 1) == 0) return 0;
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
			if(EnqueueInference(pMINMAXState->pnTransitionVars[x], 0) == 0) return 0;
		}
		arrSmurfStates[nSmurfNumber] = pTrueSimpleSmurfState;
		SimpleSmurfProblemState->arrSmurfStack[SimpleSmurfProblemState->nCurrSearchTreeLevel].nNumSmurfsSatisfied++;
	} else arrSmurfStates[nSmurfNumber] = pMINMAXCounterState;
	
	d7_printf3("      MINMAXCounterSmurf %d transitioned to state %x\n", nSmurfNumber, arrSmurfStates[nSmurfNumber]);
	return 1;
}
