#include "sbsat.h"
#include "sbsat_solver.h"
#include "solver.h"

ITE_INLINE
int XORState_InferWithLemma(XORStateEntry *pXORState, int infer_var, bool bParity, int nSmurfNumber) {
	int nInfVar = pXORState->pnTransitionVars[infer_var];
	int nInfQueueHead = SimpleSmurfProblemState->arrSmurfStack[SimpleSmurfProblemState->nCurrSearchTreeLevel].nNumFreeVars;
	int nPrevInfLevel = abs(SimpleSmurfProblemState->arrInferenceDeclaredAtLevel[nInfVar]);
	if(nPrevInfLevel < nInfQueueHead) {
		if((SimpleSmurfProblemState->arrInferenceQueue[nPrevInfLevel] > 0) != bParity) {
			//Conflict
			//Add a lemma to reference this conflict.
			create_clause_from_SmurfState(bParity?nInfVar:-nInfVar, (TypeStateEntry *)pXORState,
												SimpleSmurfProblemState->arrReverseOccurenceList[nSmurfNumber][0].var,
												&(SimpleSmurfProblemState->pConflictClause.clause),
												&(SimpleSmurfProblemState->pConflictClause.max_size));
			int ret = EnqueueInference(nInfVar, bParity);
			assert(ret == 0);
			return 0;
		} else {
			//Lit already assigned.
			d7_printf2("      Inference %d already inferred\n", bParity?nInfVar:-nInfVar);
			return 1;
		}
	} else {
		//Add a lemma as reference to this inference.
		create_clause_from_SmurfState(bParity?nInfVar:-nInfVar, (TypeStateEntry *)pXORState,
											SimpleSmurfProblemState->arrReverseOccurenceList[nSmurfNumber][0].var,
											&(SimpleSmurfProblemState->arrInferenceLemmas[nInfVar].clause),
											&(SimpleSmurfProblemState->arrInferenceLemmas[nInfVar].max_size));
		if(EnqueueInference(nInfVar, bParity) == 0) return 0;
	}
	
	return 1;
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
	int infer_var = -1;
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
	if(use_lemmas) {
		if(XORState_InferWithLemma(pXORState, infer_var, bParity, nSmurfNumber) == 0) return 0;
	} else {
		if(EnqueueInference(pXORState->pnTransitionVars[infer_var], bParity) == 0) return 0;
	}
	arrSmurfStates[nSmurfNumber] = pTrueSimpleSmurfState;
	SimpleSmurfProblemState->arrSmurfStack[SimpleSmurfProblemState->nCurrSearchTreeLevel].nNumSmurfsSatisfied++;
	
	d7_printf3("      XORSmurf %d transitioned to state %x\n", nSmurfNumber, arrSmurfStates[nSmurfNumber]);
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
	
	d7_printf3("      XORCounterSmurf %d transitioned to state %x\n", nSmurfNumber, arrSmurfStates[nSmurfNumber]);
	return 1;
}
