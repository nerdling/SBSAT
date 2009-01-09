#include "sbsat.h"
#include "sbsat_solver.h"
#include "solver.h"

ITE_INLINE
int ORState_InferWithLemma(ORStateEntry *pORState, int infer_var, int nSmurfNumber) {
	int nInfVar = pORState->pnTransitionVars[infer_var];//, pORState->bPolarity[infer_var]
	int nInfQueueHead = SimpleSmurfProblemState->arrSmurfStack[SimpleSmurfProblemState->nCurrSearchTreeLevel].nNumFreeVars;
	int nPrevInfLevel = abs(SimpleSmurfProblemState->arrInferenceDeclaredAtLevel[nInfVar]);
	if(nPrevInfLevel < nInfQueueHead) {
		if((SimpleSmurfProblemState->arrInferenceQueue[nPrevInfLevel] > 0) != pORState->bPolarity[infer_var]) {
			//Conflict
			//Add a lemma to reference this conflict.
			create_clause_from_SmurfState(pORState->bPolarity[infer_var]?nInfVar:-nInfVar, (TypeStateEntry *)pORState,
												SimpleSmurfProblemState->arrReverseOccurenceList[nSmurfNumber][0].var,
												&(SimpleSmurfProblemState->pConflictClause.clause),
												&(SimpleSmurfProblemState->pConflictClause.max_size));
			int ret = EnqueueInference(nInfVar, pORState->bPolarity[infer_var]);
			assert(ret == 0);
			return 0;
		} else {
			//Lit already assigned.
			d7_printf2("      Inference %d already inferred\n", pORState->bPolarity[infer_var]?nInfVar:-nInfVar);
			return 1;
		}
	} else {
		//Add a lemma as reference to this inference.
		create_clause_from_SmurfState(pORState->bPolarity[infer_var]?nInfVar:-nInfVar, (TypeStateEntry *)pORState,
											SimpleSmurfProblemState->arrReverseOccurenceList[nSmurfNumber][0].var,
											&(SimpleSmurfProblemState->arrInferenceLemmas[nInfVar].clause),
											&(SimpleSmurfProblemState->arrInferenceLemmas[nInfVar].max_size));
		if(EnqueueInference(nInfVar, pORState->bPolarity[infer_var]) == 0) return 0;
	}
	return 1;
}

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
	
	if(pORState->bPolarity[index] == bBVPolarity) { 
		arrSmurfStates[nSmurfNumber] = pTrueSimpleSmurfState;
		SimpleSmurfProblemState->arrSmurfStack[SimpleSmurfProblemState->nCurrSearchTreeLevel].nNumSmurfsSatisfied++;
	} else {
		int nInfQueueLevel = abs(SimpleSmurfProblemState->arrInferenceDeclaredAtLevel[nBranchVar]);
		int infer_var;
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
		if(use_lemmas) {
			if(ORState_InferWithLemma(pORState, infer_var, nSmurfNumber) == 0) return 0;
		} else {			  
			if(EnqueueInference(pORState->pnTransitionVars[infer_var], pORState->bPolarity[infer_var]) == 0) return 0;
		}
		
		arrSmurfStates[nSmurfNumber] = pTrueSimpleSmurfState;
		SimpleSmurfProblemState->arrSmurfStack[SimpleSmurfProblemState->nCurrSearchTreeLevel].nNumSmurfsSatisfied++;
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
	
	if(pORState->bPolarity[index] == bBVPolarity) { 
		arrSmurfStates[nSmurfNumber] = pTrueSimpleSmurfState;
		SimpleSmurfProblemState->arrSmurfStack[SimpleSmurfProblemState->nCurrSearchTreeLevel].nNumSmurfsSatisfied++;
	} else {
		pORCounterState->nLemmaLiteral = bBVPolarity?-nBranchVar:nBranchVar; //Polarity for lemma literals is reversed
		((TypeStateEntry *)pORCounterState->pTransition)->pPreviousState = pORCounterState; //This line can be precomputed -- SEAN!!!
		((TypeStateEntry *)pORCounterState->pTransition)->pStateOwner = nSmurfNumber;
		arrSmurfStates[nSmurfNumber] = pORCounterState->pTransition;
	}	
	d7_printf3("      ORCounterSmurf %d transitioned to state %x\n", nSmurfNumber, arrSmurfStates[nSmurfNumber]);
	return 1;
}
