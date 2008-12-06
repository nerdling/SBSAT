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
//			void *pNextState = bBVPolarity?pSmurfState->pVarIsTrueTransition:pSmurfState->pVarIsFalseTransition;
			void *pNextState;
			if(bBVPolarity) {
				if(pSmurfState->pVarIsTrueTransition == NULL)
				  pSmurfState->pVarIsTrueTransition = ReadSmurfStateIntoTable(set_variable(pSmurfState->bdd, nBranchVar, 1), NULL, 0);
				pNextState = pSmurfState->pVarIsTrueTransition;
			} else {
				if(pSmurfState->pVarIsFalseTransition == NULL)
				  pSmurfState->pVarIsFalseTransition = ReadSmurfStateIntoTable(set_variable(pSmurfState->bdd, nBranchVar, 0), NULL, 0);
				pNextState = pSmurfState->pVarIsFalseTransition;
			}
			void *pPrevState;
			while(pNextState!=NULL && ((TypeStateEntry *)pNextState)->cType == FN_INFERENCE) {
				if(EnqueueInference(((InferenceStateEntry *)pNextState)->nTransitionVar, ((InferenceStateEntry *)pNextState)->bPolarity > 0) == 0) return 0;
				//Follow the transtion to the next SmurfState
				pPrevState = pNextState;
				pNextState = ((InferenceStateEntry *)pNextState)->pVarTransition;
			}

			if(pNextState == NULL) {
				assert(((TypeStateEntry *)pPrevState)->cType == FN_INFERENCE);
				((InferenceStateEntry *)pPrevState)->pVarTransition = ReadSmurfStateIntoTable(
																						   set_variable(((InferenceStateEntry *)pPrevState)->bdd,
																											 ((InferenceStateEntry *)pPrevState)->nTransitionVar,
																											 ((InferenceStateEntry *)pPrevState)->bPolarity), NULL, 0);
				pNextState = ((void *)((InferenceStateEntry *)pPrevState)->pVarTransition);
			}
			
			//Record the transition.
			arrSmurfStates[nSmurfNumber] = pNextState;
			break;
		}
	} while (pSmurfState != NULL);

	if(arrSmurfStates[nSmurfNumber] == pTrueSimpleSmurfState)
	  SimpleSmurfProblemState->arrSmurfStack[SimpleSmurfProblemState->nCurrSearchTreeLevel].nNumSmurfsSatisfied++;	  
	
	int ret = ApplyInferenceToSmurf_Hooks(nBranchVar, bBVPolarity, nSmurfNumber, arrSmurfStates);
	
	d7_printf3("      Smurf %d transitioned to state %x\n", nSmurfNumber, arrSmurfStates[nSmurfNumber]);

	return ret;
}

