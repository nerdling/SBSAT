#include "sbsat.h"
#include "sbsat_solver.h"
#include "solver.h"

ITE_INLINE
int ApplyInferenceToSmurf(int nBranchVar, bool bBVPolarity, int nSmurfNumber, void **arrSmurfStates) {
	SmurfStateEntry *pSmurfState = (SmurfStateEntry *)arrSmurfStates[nSmurfNumber];
	do {
		if(pSmurfState->nTransitionVar < nBranchVar)
		  pSmurfState = (SmurfStateEntry *)pSmurfState->pNextVarInThisStateGT;
		else if(pSmurfState->nTransitionVar > nBranchVar)
		  pSmurfState = (SmurfStateEntry *)pSmurfState->pNextVarInThisStateLT;
		else {
			//(pSmurfState->nTransitionVar == nBranchVar) {
			//Follow this transition and apply all inferences found.
			void *pNextState = bBVPolarity?pSmurfState->pVarIsTrueTransition:pSmurfState->pVarIsFalseTransition;
			if(((TypeStateEntry *)pNextState)->cType == FN_WATCHED_LIST) {
//				SimpleSmurfProblemState->arrWatchedListStack[SimpleSmurfProblemState->arrSmurfStack[SimpleSmurfProblemState->nCurrSearchTreeLevel].nWatchedListStackTop] =
//				  SimpleSmurfProblemState->arrReverseOccurenceList[x][y]
				
//				int length = ((WatchedListStateEntry *)pNextState);
//				for(int i=0; i < length; i++) {
//          0x80000000
					
//				}
				pNextState = ((WatchedListStateEntry *)pNextState)->pTransition;
			}
			while(((TypeStateEntry *)pNextState)->cType == FN_INFERENCE) {
				if(EnqueueInference(((InferenceStateEntry *)pNextState)->nTransitionVar, ((InferenceStateEntry *)pNextState)->bPolarity > 0) == 0) return 0;
				//Follow the transtion to the next SmurfState
				pNextState = ((InferenceStateEntry *)pNextState)->pVarTransition;
			}
			if(((TypeStateEntry *)pNextState)->cType == FN_WATCHED_LIST) {
				//SEAN!!! Remove nSmurfNumber from all vars watched lists, add info to stack.
				pNextState = ((WatchedListStateEntry *)pNextState)->pTransition;
			}

			//Record the transition.
			arrSmurfStates[nSmurfNumber] = pNextState;
			break;
		}
	} while (pSmurfState != NULL);

	int ret = ApplyInferenceToSmurf_Hooks(nBranchVar, bBVPolarity, nSmurfNumber, arrSmurfStates);
	
	d7_printf3("      Smurf %d transitioned to state %x\n", nSmurfNumber, arrSmurfStates[nSmurfNumber]);

	return ret;
}

