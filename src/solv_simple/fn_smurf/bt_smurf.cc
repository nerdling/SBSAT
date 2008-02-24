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
			InferenceStateEntry *pInference = (InferenceStateEntry *)(bBVPolarity?pSmurfState->pVarIsTrueTransition:pSmurfState->pVarIsFalseTransition);
			while(pInference->cType == FN_INFERENCE) {
				if(EnqueueInference(pInference->nTransitionVar, pInference->bPolarity > 0) == 0) return 0;
				//Follow the transtion to the next SmurfState
				pInference = (InferenceStateEntry *)pInference->pVarTransition;
			}
			//assert(pInference->cType == FN_SMURF);
			//pSmurfState = (SmurfStateEntry *)pInference;
			//Record the transition.
			arrSmurfStates[nSmurfNumber] = pInference;//pSmurfState;
			break;
		}
	} while (pSmurfState != NULL);

	int ret = ApplyInferenceToSmurf_Hooks(nBranchVar, bBVPolarity, nSmurfNumber, arrSmurfStates);
	
	d7_printf3("      Smurf %d transitioned to state %x\n", nSmurfNumber, arrSmurfStates[nSmurfNumber]);

	return ret;
}

