#include "sbsat.h"
#include "sbsat_solver.h"
#include "solver.h"

/********** Included in include/sbsat_solver.h *********************

struct TypeStateEntry {
   char cType;
}

struct ORStateEntry {
   char cType; //FN_OR
   int *pnTransitionVars;
   bool *bPolarity;
   int nSize;
};

struct ORCounterStateEntry {
   char cType; //FN_OR_COUNTER
   void *pTransition;
   int nSize;
   ORStateEntry *pORState;
};

***********************************************************************/

int ApplyInferenceToOR(int nBranchVar, bool bBVPolarity, int nSmurfNumber, void **arrSmurfStates) {
	ORStateEntry *pORState = (ORStateEntry *)arrSmurfStates[nSmurfNumber];	
	
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
	
	if(pORState->bPolarity[index] == bBVPolarity) arrSmurfStates[nSmurfNumber] = pTrueSimpleSmurfState;
	else {
		int nInfQueueLevel = abs(SimpleSmurfProblemState->arrInferenceDeclaredAtLevel[nBranchVar]);
		for(int x = 0; x < pORState->nSize; x++) {
			int nPrevInfLevel = SimpleSmurfProblemState->arrInferenceDeclaredAtLevel[pORState->pnTransitionVars[x]];
//			fprintf(stderr, "%d %d<%d x=%d var=%d pol=%d\n", nBranchVar, nPrevInfLevel, nInfQueueLevel, x, pORState->pnTransitionVars[x], pORState->bPolarity[x]);
//			if(x == index) continue;
			if(nPrevInfLevel <= 0) continue;
			if(nPrevInfLevel <= nInfQueueLevel) {
				assert((SimpleSmurfProblemState->arrInferenceQueue[nPrevInfLevel] == 0) ||
						 ((SimpleSmurfProblemState->arrInferenceQueue[nPrevInfLevel] > 0) != pORState->bPolarity[x]));
				continue;
			}
			
			//Inference is not in inference queue, insert it.
			if(EnqueueInference(pORState->pnTransitionVars[x], pORState->bPolarity[x]) == 0) return 0;
			arrSmurfStates[nSmurfNumber] = pTrueSimpleSmurfState;
			break;
		}
	}
	d7_printf3("      ORSmurf %d transitioned to state %x\n", nSmurfNumber, arrSmurfStates[nSmurfNumber]);
	return 1;
}

int ApplyInferenceToORCounter(int nBranchVar, bool bBVPolarity, int nSmurfNumber, void **arrSmurfStates) {
	ORCounterStateEntry *pORCounterState = (ORCounterStateEntry *)arrSmurfStates[nSmurfNumber];
	ORStateEntry *pORState = (ORStateEntry *)pORCounterState->pORState;
	
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
	
	if(pORState->bPolarity[index] == bBVPolarity) arrSmurfStates[nSmurfNumber] = pTrueSimpleSmurfState;
	else {
		arrSmurfStates[nSmurfNumber] = pORCounterState->pTransition;
	}	
	d7_printf3("      ORCounterSmurf %d transitioned to state %x\n", nSmurfNumber, arrSmurfStates[nSmurfNumber]);
	return 1;
}
