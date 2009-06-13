#include "sbsat.h"
#include "sbsat_solver.h"
#include "solver.h"

void SetVisitedSmurfState(void *pState, int value) {
   SmurfStateEntry *pSmurfState = (SmurfStateEntry *)pState;
   assert(pSmurfState->cType == FN_SMURF);

   while(pSmurfState!=NULL && pSmurfState->visited != value) {
      d7_printf3("Marking visited=%d of Smurf State %p\n", value, pSmurfState);
      pSmurfState->visited = value;
      bdd_flag_nodes(pSmurfState->pSmurfBDD);
//      if(pSmurfState->pVarIsTrueTransition != NULL && ((TypeStateEntry *)(pSmurfState->pVarIsTrueTransition))->cType == FN_INFERENCE)
//        SetVisitedInferenceState(pSmurfState->pVarIsTrueTransition, value);
//Commented out to work w/ garbage collection compression (may result in slighly slower search.
//      if((TypeStateEntry *)pSmurfState->pVarIsTrueTransition->visited != value)
        pSmurfState->pVarIsTrueTransition = NULL;
//      if(pSmurfState->pVarIsFalseTransition != NULL && ((TypeStateEntry *)(pSmurfState->pVarIsFalseTransition))->cType == FN_INFERENCE)
//        SetVisitedInferenceState(pSmurfState->pVarIsFalseTransition, value);
//Commented out to work w/ garbage collection compression (may result in slighly slower search.
//      if((TypeStateEntry *)pSmurfState->pVarIsFalseTransition->visited != value)
        pSmurfState->pVarIsFalseTransition = NULL;
      pSmurfState = (SmurfStateEntry *)(pSmurfState->pNextVarInThisState);
   }
}

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
				if(pSmurfState->pVarIsTrueTransition == NULL) {
               pSmurfState->pVarIsTrueTransition = ReadSmurfStateIntoTable(set_variable(pSmurfState->pSmurfBDD, arrSimpleSolver2IteVarMap[nBranchVar], 1), NULL, 0);
               assert(((TypeStateEntry *)(pSmurfState->pVarIsTrueTransition))->cType!=FN_FREE_STATE);
            }
            pNextState = pSmurfState->pVarIsTrueTransition;
			} else {
				if(pSmurfState->pVarIsFalseTransition == NULL) {
               pSmurfState->pVarIsFalseTransition = ReadSmurfStateIntoTable(set_variable(pSmurfState->pSmurfBDD, arrSimpleSolver2IteVarMap[nBranchVar], 0), NULL, 0);
               assert(((TypeStateEntry *)(pSmurfState->pVarIsFalseTransition))->cType!=FN_FREE_STATE);
            }
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
            assert(((TypeStateEntry *)pNextState)->cType!=FN_FREE_STATE);
			}
			
			//Record the transition.
			if(((SmurfStateEntry *)pNextState) == pTrueSimpleSmurfState) {
				SimpleSmurfProblemState->arrSmurfStack[SimpleSmurfProblemState->nCurrSearchTreeLevel].nNumSmurfsSatisfied++;
			} else if(pNextState == arrSmurfStates[((TypeStateEntry *)pNextState)->pStateOwner]) {
				d7_printf3("      State %p currently owned by Smurf %d, transitioning to True\n", pNextState, ((TypeStateEntry *)pNextState)->pStateOwner);
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
	
	d7_printf3("      Smurf %d transitioned to state %p\n", nSmurfNumber, arrSmurfStates[nSmurfNumber]);
	d7_printf3("      Smurf %d previously was %p\n", nSmurfNumber, ((SmurfStateEntry *)arrSmurfStates[nSmurfNumber])->pPreviousState);
	
	return ret;
}

ITE_INLINE
int ApplyInferenceToWatchedSmurf(int nBranchVar, bool bBVPolarity, int nSmurfNumber, void **arrSmurfStates) {
	return 0;
}

