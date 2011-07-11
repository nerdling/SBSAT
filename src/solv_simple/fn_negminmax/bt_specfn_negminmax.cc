#include "sbsat.h"
#include "sbsat_solver.h"
#include "solver.h"

void SetVisitedNEGMINMAXState(void *pState, int value) {
   NEGMINMAXStateEntry *pNEGMINMAXState = (NEGMINMAXStateEntry *)pState;
   assert(pNEGMINMAXState->cType == FN_NEGMINMAX);
   if(pNEGMINMAXState->visited != value) {
      d7_printf3("Marking visited=%d of NEGMINMAX State %p\n", value, pNEGMINMAXState);
      pNEGMINMAXState->visited = value;
   }
}

void SetVisitedNEGMINMAXCounterState(void *pState, int value) {
   NEGMINMAXCounterStateEntry *pNEGMINMAXCounterState = (NEGMINMAXCounterStateEntry *)pState;
   assert(pNEGMINMAXCounterState->cType == FN_NEGMINMAX_COUNTER);
   SetVisitedNEGMINMAXState(pNEGMINMAXCounterState->pNEGMINMAXState, value);
   NEGMINMAXCounterStateEntry *tmp_negminmax = pNEGMINMAXCounterState;
   while(tmp_negminmax != NULL && tmp_negminmax->cType == FN_NEGMINMAX_COUNTER) {
      d7_printf3("Marking visited=%d of NEGMINMAXCounter State %p\n", value, tmp_negminmax);
      tmp_negminmax->visited = value;
      tmp_negminmax = (NEGMINMAXCounterStateEntry *)tmp_negminmax->pTransition;
   }
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
         if(TypeState_InferVar((TypeStateEntry *)arrSmurfStates[nSmurfNumber],
                               pNEGMINMAXCounterState->pNEGMINMAXState->pnTransitionVars[x],
                               1,
                               nSmurfNumber,
                               INF_SPEC_FN_NEGMINMAX) == 0) return 0;			
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
         if(TypeState_InferVar((TypeStateEntry *)arrSmurfStates[nSmurfNumber],
                               pNEGMINMAXCounterState->pNEGMINMAXState->pnTransitionVars[x],
                               0,
                               nSmurfNumber,
                               INF_SPEC_FN_NEGMINMAX) == 0) return 0;
		}
		arrSmurfStates[nSmurfNumber] = pTrueSimpleSmurfState;
		SimpleSmurfProblemState->arrSmurfStack[SimpleSmurfProblemState->nCurrSearchTreeLevel].nNumSmurfsSatisfied++;
	} else {
		pNEGMINMAXCounterState->pPreviousState = (NEGMINMAXCounterStateEntry *)arrSmurfStates[nSmurfNumber]; //This line can be precomputed -- SEAN!!!
		pNEGMINMAXCounterState->pStateOwner = nSmurfNumber;
		arrSmurfStates[nSmurfNumber] = pNEGMINMAXCounterState;
	}
	
	d7_printf3("      NEGMINMAXCounterSmurf %d transitioned to state %p\n", nSmurfNumber, arrSmurfStates[nSmurfNumber]);
	return 1;
}
