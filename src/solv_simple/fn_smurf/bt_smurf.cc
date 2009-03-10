#include "sbsat.h"
#include "sbsat_solver.h"
#include "solver.h"

ITE_INLINE
int ApplyInferenceToSmurf(int nBranchVar, bool bBVPolarity, int nSmurfNumber, void **arrSmurfStates) {
	SmurfStateEntry *pSmurfState = (SmurfStateEntry *)arrSmurfStates[nSmurfNumber];

	void *pNextState;
	do {
		if(pSmurfState->nTransitionVar < nBranchVar)
		  pSmurfState = (SmurfStateEntry *)pSmurfState->pNextVarInThisStateGT;
		else if(pSmurfState->nTransitionVar > nBranchVar)
		  pSmurfState = (SmurfStateEntry *)pSmurfState->pNextVarInThisStateLT;
		else {
			((TypeStateEntry *)arrSmurfStates[nSmurfNumber])->nLemmaLiteral = bBVPolarity?-nBranchVar:nBranchVar; //Polarity for lemma literals is reversed
			//(pSmurfState->nTransitionVar == nBranchVar) {
			//Follow this transition and apply all inferences found.
//			void *pNextState = bBVPolarity?pSmurfState->pVarIsTrueTransition:pSmurfState->pVarIsFalseTransition;
			if(bBVPolarity) {
				if(pSmurfState->pVarIsTrueTransition == NULL)
				  pSmurfState->pVarIsTrueTransition = ReadSmurfStateIntoTable(set_variable(pSmurfState->pSmurfBDD, arrSimpleSolver2IteVarMap[nBranchVar], 1), NULL, 0);
				pNextState = pSmurfState->pVarIsTrueTransition;
			} else {
				if(pSmurfState->pVarIsFalseTransition == NULL)
				  pSmurfState->pVarIsFalseTransition = ReadSmurfStateIntoTable(set_variable(pSmurfState->pSmurfBDD, arrSimpleSolver2IteVarMap[nBranchVar], 0), NULL, 0);
				pNextState = pSmurfState->pVarIsFalseTransition;
			}
			void *pPrevState = NULL;
			while(pNextState!=NULL && ((TypeStateEntry *)pNextState)->cType == FN_INFERENCE) {
            if(TypeState_InferVar((TypeStateEntry *)arrSmurfStates[nSmurfNumber], 
                                  ((InferenceStateEntry *)pNextState)->nTransitionVar,
                                  ((InferenceStateEntry *)pNextState)->bPolarity,
                                  nSmurfNumber,
                                  INF_SMURF) == 0) return 0;
            
				//Follow the transtion to the next SmurfState
				pPrevState = pNextState;
				pNextState = ((InferenceStateEntry *)pNextState)->pVarTransition;
			}

			if(pNextState == NULL) {
				assert(((TypeStateEntry *)pPrevState)->cType == FN_INFERENCE);
				((InferenceStateEntry *)pPrevState)->pVarTransition = pNextState = ReadSmurfStateIntoTable(
																						    set_variable(((InferenceStateEntry *)pPrevState)->pInferenceBDD,
																							 arrSimpleSolver2IteVarMap[((InferenceStateEntry *)pPrevState)->nTransitionVar],
																							 ((InferenceStateEntry *)pPrevState)->bPolarity),
																							 NULL, 0);
			}
			
			//Record the transition.
			if(((SmurfStateEntry *)pNextState) == pTrueSimpleSmurfState) {
				SimpleSmurfProblemState->arrSmurfStack[SimpleSmurfProblemState->nCurrSearchTreeLevel].nNumSmurfsSatisfied++;
			} else if(pNextState == arrSmurfStates[((TypeStateEntry *)pNextState)->pStateOwner]) {
				d7_printf3("      State %x currently owned by Smurf %d, transitioning to True\n", pNextState, ((TypeStateEntry *)pNextState)->pStateOwner);
				pNextState = pTrueSimpleSmurfState;
			} else {
				((TypeStateEntry *)pNextState)->pPreviousState = arrSmurfStates[nSmurfNumber];
				((TypeStateEntry *)pNextState)->pStateOwner = nSmurfNumber;
			}

			arrSmurfStates[nSmurfNumber] = pNextState;
			break;
		}
	} while (pSmurfState != NULL);

	int ret = ApplyInferenceToSmurf_Hooks(nBranchVar, bBVPolarity, nSmurfNumber, arrSmurfStates);
	
	d7_printf3("      Smurf %d transitioned to state %x\n", nSmurfNumber, arrSmurfStates[nSmurfNumber]);
	d7_printf3("      Smurf %d previously was %x\n", nSmurfNumber, ((SmurfStateEntry *)arrSmurfStates[nSmurfNumber])->pPreviousState);
	
	return ret;
}

ITE_INLINE
int ApplyInferenceToWatchedSmurf(int nBranchVar, bool bBVPolarity, int nSmurfNumber, void **arrSmurfStates) {
	return 0;
}

