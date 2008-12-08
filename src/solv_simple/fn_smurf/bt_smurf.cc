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
				  pSmurfState->pVarIsTrueTransition = ReadSmurfStateIntoTable(set_variable(pSmurfState->pSmurfBDD, arrSimpleSolver2IteVarMap[nBranchVar], 1), NULL, 0);
				pNextState = pSmurfState->pVarIsTrueTransition;
			} else {
				if(pSmurfState->pVarIsFalseTransition == NULL)
				  pSmurfState->pVarIsFalseTransition = ReadSmurfStateIntoTable(set_variable(pSmurfState->pSmurfBDD, arrSimpleSolver2IteVarMap[nBranchVar], 0), NULL, 0);
				pNextState = pSmurfState->pVarIsFalseTransition;
			}
			void *pPrevState = NULL;
			while(pNextState!=NULL && ((TypeStateEntry *)pNextState)->cType == FN_INFERENCE) {
				if(EnqueueInference(((InferenceStateEntry *)pNextState)->nTransitionVar, ((InferenceStateEntry *)pNextState)->bPolarity > 0) == 0) return 0;
				//Follow the transtion to the next SmurfState
				pPrevState = pNextState;
				pNextState = ((InferenceStateEntry *)pNextState)->pVarTransition;
			}

			if(pNextState == NULL) {
				assert(((TypeStateEntry *)pPrevState)->cType == FN_INFERENCE);
				((InferenceStateEntry *)pPrevState)->pVarTransition = ReadSmurfStateIntoTable(
																						    set_variable(((InferenceStateEntry *)pPrevState)->pInferenceBDD,
																							 arrSimpleSolver2IteVarMap[((InferenceStateEntry *)pPrevState)->nTransitionVar],
																							 ((InferenceStateEntry *)pPrevState)->bPolarity),
																							 NULL,
																							 0);
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

//Seems to be working great...so far.
ITE_INLINE
int ApplyInferenceToWatchedSmurf(int nBranchVar, bool bBVPolarity, int nSmurfNumber, void **arrSmurfStates) {
//int ApplyInferenceToSmurf(int nBranchVar, bool bBVPolarity, int nSmurfNumber, void **arrSmurfStates) {	
	int nInfQueueHead = SimpleSmurfProblemState->arrSmurfStack[SimpleSmurfProblemState->nCurrSearchTreeLevel].nNumFreeVars;
	//SEAN!!! inferences ahead of current position may be inferred here - change the functionality a little.
	
	SmurfStateEntry *pSmurfState = (SmurfStateEntry *)arrSmurfStates[nSmurfNumber];

	do {
		//The abs() is necessary because old choicepoints are negative
		int nPrevInfLevel = abs(SimpleSmurfProblemState->arrInferenceDeclaredAtLevel[pSmurfState->nTransitionVar]);

		if(nPrevInfLevel < nInfQueueHead) { //Apply this inference
			int nInfVar = pSmurfState->nTransitionVar;
			bool bInfPolarity = (SimpleSmurfProblemState->arrInferenceQueue[nPrevInfLevel] > 0);

			d7_printf3("      Handling inference %c%d\n", bInfPolarity?'+':'-', nInfVar);
			
			void *pNextState;
			if(bInfPolarity) {
				if(pSmurfState->pVarIsTrueTransition == NULL)
				  pSmurfState->pVarIsTrueTransition = ReadSmurfStateIntoTable(set_variable(pSmurfState->pSmurfBDD, arrSimpleSolver2IteVarMap[nInfVar], 1), NULL, 0);
				pNextState = pSmurfState->pVarIsTrueTransition;
			} else {
				if(pSmurfState->pVarIsFalseTransition == NULL)
				  pSmurfState->pVarIsFalseTransition = ReadSmurfStateIntoTable(set_variable(pSmurfState->pSmurfBDD, arrSimpleSolver2IteVarMap[nInfVar], 0), NULL, 0);
				pNextState = pSmurfState->pVarIsFalseTransition;
			}
			void *pPrevState = NULL;
			while(pNextState!=NULL && ((TypeStateEntry *)pNextState)->cType == FN_INFERENCE) {
				if(EnqueueInference(((InferenceStateEntry *)pNextState)->nTransitionVar, ((InferenceStateEntry *)pNextState)->bPolarity > 0) == 0) return 0;
				//Follow the transtion to the next SmurfState
				pPrevState = pNextState;
				pNextState = ((InferenceStateEntry *)pNextState)->pVarTransition;
			}
			
			if(pNextState == NULL) {
				assert(((TypeStateEntry *)pPrevState)->cType == FN_INFERENCE);
					((InferenceStateEntry *)pPrevState)->pVarTransition = ReadSmurfStateIntoTable(
																								 set_variable(((InferenceStateEntry *)pPrevState)->pInferenceBDD,
																								 arrSimpleSolver2IteVarMap[((InferenceStateEntry *)pPrevState)->nTransitionVar],
																								 ((InferenceStateEntry *)pPrevState)->bPolarity),
																								 NULL,
																								 0);
				pNextState = ((void *)((InferenceStateEntry *)pPrevState)->pVarTransition);
			}
			
			d7_printf3("      Smurf %d transitioned to state %x\n", nSmurfNumber, pNextState);

			arrSmurfStates[nSmurfNumber] = pNextState;
			if(((TypeStateEntry *)pNextState)->cType == FN_SMURF) {
				//Record the transition.
				pSmurfState = (SmurfStateEntry *)pNextState;
			} else break; //New functionType detected
		} else {
			pSmurfState = (SmurfStateEntry *)pSmurfState->pNextVarInThisState;			  
		}
		
	} while (pSmurfState != NULL && pSmurfState!=pTrueSimpleSmurfState);
	
	if(((TypeStateEntry *)arrSmurfStates[nSmurfNumber])->cType != FN_SMURF) {		  
		if(((TypeStateEntry *)arrSmurfStates[nSmurfNumber])->ApplyInferenceToState(nBranchVar, bBVPolarity, nSmurfNumber, arrSmurfStates) == 0)
		  return 0;
	} else {
		if(arrSmurfStates[nSmurfNumber] == pTrueSimpleSmurfState) {
			SimpleSmurfProblemState->arrSmurfStack[SimpleSmurfProblemState->nCurrSearchTreeLevel].nNumSmurfsSatisfied++;
		} else {
			//Setup a new watched variable - possibly more ?!??
		}
	}
		
	int ret = ApplyInferenceToSmurf_Hooks(nBranchVar, bBVPolarity, nSmurfNumber, arrSmurfStates);
	
	return ret;
}

