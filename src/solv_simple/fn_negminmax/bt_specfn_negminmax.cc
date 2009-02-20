#include "sbsat.h"
#include "sbsat_solver.h"
#include "solver.h"

ITE_INLINE
int NEGMINMAXState_InferWithLemma(NEGMINMAXCounterStateEntry *pNEGMINMAXCounterState, int infer_var, bool bPolarity, int nSmurfNumber) {
	int nInfVar = pNEGMINMAXCounterState->pNEGMINMAXState->pnTransitionVars[infer_var];//, pNEGMINMAXState->bPolarity[infer_var]
	int nInfQueueHead = SimpleSmurfProblemState->arrSmurfStack[SimpleSmurfProblemState->nCurrSearchTreeLevel].nNumFreeVars;
	int nPrevInfLevel = abs(SimpleSmurfProblemState->arrInferenceDeclaredAtLevel[nInfVar]);
	if(nPrevInfLevel < nInfQueueHead) {
		if((SimpleSmurfProblemState->arrInferenceQueue[nPrevInfLevel] > 0) != bPolarity) {
			//Conflict
			//Add a lemma to reference this conflict.
			create_clause_from_SmurfState(bPolarity?nInfVar:-nInfVar, (TypeStateEntry *)pNEGMINMAXCounterState,
													SimpleSmurfProblemState->arrReverseOccurenceList[nSmurfNumber][0].var,
													&(SimpleSmurfProblemState->pConflictClause.clause),
													&(SimpleSmurfProblemState->pConflictClause.max_size));
			int ret = EnqueueInference(nInfVar, bPolarity, INF_SPEC_FN_NEG_MINMAX);
			assert(ret == 0);
			return 0;
		} else {
			//Lit already assigned.
			d7_printf2("      Inference %d already inferred\n", bPolarity?nInfVar:-nInfVar);
			return 1;
		}
	} else {
		//Add a lemma as reference to this inference.
		create_clause_from_SmurfState(bPolarity?nInfVar:-nInfVar, (TypeStateEntry *)pNEGMINMAXCounterState,
												SimpleSmurfProblemState->arrReverseOccurenceList[nSmurfNumber][0].var,
												&(SimpleSmurfProblemState->arrInferenceLemmas[nInfVar].clause),
												&(SimpleSmurfProblemState->arrInferenceLemmas[nInfVar].max_size));
		if(EnqueueInference(nInfVar, bPolarity, INF_SPEC_FN_NEG_MINMAX) == 0) return 0;
	}
	return 1;
}


int ApplyInferenceToNEGMINMAX(int nBranchVar, bool bBVPolarity, int nSmurfNumber, void **arrSmurfStates) {
	//I don't think this should happen w/ the current setup...yet - use for watched literals
	assert(0);
}

int ApplyInferenceToNEGMINMAXCounter(int nBranchVar, bool bBVPolarity, int nSmurfNumber, void **arrSmurfStates) {
	NEGMINMAXCounterStateEntry *pNEGMINMAXCounterState = (NEGMINMAXCounterStateEntry *)arrSmurfStates[nSmurfNumber];
	NEGMINMAXStateEntry *pNEGMINMAXState = (NEGMINMAXStateEntry *)pNEGMINMAXCounterState->pNEGMINMAXState;
	
	assert(pNEGMINMAXCounterState->pStateOwner == nSmurfNumber);
	
	int index = 0;
	int size = pNEGMINMAXState->nSize;
	int prev = 0;
	int var = pNEGMINMAXState->pnTransitionVars[index+(size/2)];
	while(var != nBranchVar) {
		//fprintf(stderr, "%d(%d) - ", pNEGMINMAXState->pnTransitionVars[index+(size/2)], size);
		if(var == prev) break;
		prev = var;
		if(var < nBranchVar) {
			size = size/2;
		} else {
			index = index+(size/2)+1;
			size = (size-(size/2))-1;
			if(index+(size/2) == pNEGMINMAXState->nSize) break;
		}
		var = pNEGMINMAXState->pnTransitionVars[index+(size/2)];
	}
	if(var != nBranchVar) return 1; //Var does not exist in this Smurf
	index = index+(size/2);

	if(bBVPolarity) {
		((NEGMINMAXCounterStateEntry *)(pNEGMINMAXCounterState->pTransition))->nNumTrue = pNEGMINMAXCounterState->nNumTrue+1;
	} else {
		((NEGMINMAXCounterStateEntry *)(pNEGMINMAXCounterState->pTransition))->nNumTrue = pNEGMINMAXCounterState->nNumTrue;
	}	

	pNEGMINMAXCounterState->nLemmaLiteral = bBVPolarity?-nBranchVar:nBranchVar; //Polarity for lemma literals is reversed
	pNEGMINMAXCounterState = (NEGMINMAXCounterStateEntry *)(pNEGMINMAXCounterState->pTransition);
	
//	PrintNEGMINMAXCounterStateEntry(pNEGMINMAXCounterState);
//	PrintNEGMINMAXStateEntry(pNEGMINMAXState);

	if(pNEGMINMAXCounterState->nNumTrue > pNEGMINMAXState->nMax ||
      pNEGMINMAXState->nMin > (pNEGMINMAXCounterState->nNumTrue + pNEGMINMAXCounterState->nVarsLeft)) {
		arrSmurfStates[nSmurfNumber] = pTrueSimpleSmurfState;
		SimpleSmurfProblemState->arrSmurfStack[SimpleSmurfProblemState->nCurrSearchTreeLevel].nNumSmurfsSatisfied++;
	} else if((pNEGMINMAXCounterState->nNumTrue >= pNEGMINMAXState->nMin) &&
				 (pNEGMINMAXCounterState->nVarsLeft == ((pNEGMINMAXState->nMax+1) - pNEGMINMAXCounterState->nNumTrue))) {
		//Infer remaining variables to True
		int nInfQueueLevel = abs(SimpleSmurfProblemState->arrInferenceDeclaredAtLevel[nBranchVar]);
		for(int x = 0; x < pNEGMINMAXState->nSize; x++) {
			int nPrevInfLevel = SimpleSmurfProblemState->arrInferenceDeclaredAtLevel[pNEGMINMAXState->pnTransitionVars[x]];
			if(nPrevInfLevel <= nInfQueueLevel)	continue;
			//Inference is not in inference queue, insert it.
			if(use_lemmas) {
				if(NEGMINMAXState_InferWithLemma((NEGMINMAXCounterStateEntry *)arrSmurfStates[nSmurfNumber], x, 1, nSmurfNumber) == 0) return 0;
			} else {
				if(EnqueueInference(pNEGMINMAXState->pnTransitionVars[x], 1, INF_SPEC_FN_NEG_MINMAX) == 0) return 0;
			}
		}
		arrSmurfStates[nSmurfNumber] = pTrueSimpleSmurfState;
		SimpleSmurfProblemState->arrSmurfStack[SimpleSmurfProblemState->nCurrSearchTreeLevel].nNumSmurfsSatisfied++;
	} else if((pNEGMINMAXCounterState->nNumTrue == (pNEGMINMAXState->nMin-1)) &&
             (pNEGMINMAXCounterState->nVarsLeft <= (pNEGMINMAXState->nMax - pNEGMINMAXCounterState->nNumTrue))) {
		//Infer remaining variables to False
		int nInfQueueLevel = abs(SimpleSmurfProblemState->arrInferenceDeclaredAtLevel[nBranchVar]);
		for(int x = 0; x < pNEGMINMAXState->nSize; x++) {
			int nPrevInfLevel = SimpleSmurfProblemState->arrInferenceDeclaredAtLevel[pNEGMINMAXState->pnTransitionVars[x]];
			if(nPrevInfLevel <= nInfQueueLevel)	continue;
			//Inference is not in inference queue, insert it.
			if(use_lemmas) {
				if(NEGMINMAXState_InferWithLemma((NEGMINMAXCounterStateEntry *)arrSmurfStates[nSmurfNumber], x, 0, nSmurfNumber) == 0) return 0;
			} else {
				if(EnqueueInference(pNEGMINMAXState->pnTransitionVars[x], 0, INF_SPEC_FN_NEG_MINMAX) == 0) return 0;
			}
		}
		arrSmurfStates[nSmurfNumber] = pTrueSimpleSmurfState;
		SimpleSmurfProblemState->arrSmurfStack[SimpleSmurfProblemState->nCurrSearchTreeLevel].nNumSmurfsSatisfied++;
	} else {
		pNEGMINMAXCounterState->pPreviousState = (NEGMINMAXCounterStateEntry *)arrSmurfStates[nSmurfNumber]; //This line can be precomputed -- SEAN!!!
		pNEGMINMAXCounterState->pStateOwner = nSmurfNumber;
		arrSmurfStates[nSmurfNumber] = pNEGMINMAXCounterState;
	}
	
	d7_printf3("      NEGMINMAXCounterSmurf %d transitioned to state %x\n", nSmurfNumber, arrSmurfStates[nSmurfNumber]);
	return 1;
}
