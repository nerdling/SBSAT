#include "sbsat.h"
#include "sbsat_solver.h"
#include "solver.h"

ITE_INLINE
int ApplyInferenceToSmurf(int nBranchVar, bool bBVPolarity, int nSmurfNumber, void **arrSmurfStates) {
	SmurfStateEntry *pSmurfState = (SmurfStateEntry *)arrSmurfStates[nSmurfNumber];
	SmurfStateEntry *pTopState = pSmurfState;
	void *pNextState;
	do {
		if(pSmurfState->nTransitionVar < nBranchVar)
		  pSmurfState = (SmurfStateEntry *)pSmurfState->pNextVarInThisStateGT;
		else if(pSmurfState->nTransitionVar > nBranchVar)
		  pSmurfState = (SmurfStateEntry *)pSmurfState->pNextVarInThisStateLT;
		else {
			pTopState->nLemmaLiteral = bBVPolarity?-nBranchVar:nBranchVar;
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

				//SimpleSmurfProblemState->arrInferenceLemmas[nInfLiteral].lits;
				//SimpleSmurfProblemState->arrInferenceLemmas[nInfLiteral].max_size;

				int nInfVar = ((InferenceStateEntry *)pNextState)->nTransitionVar;
				
				D_7(
					 SmurfStateEntry *pTempState = pTopState;
					 d7_printf2("        Lemma: %d", (((InferenceStateEntry *)pNextState)->bPolarity > 0)?
									nInfVar:-nInfVar);
					 while(pTempState!=NULL) {
						 d7_printf3(" %d(%x)", pTempState->nLemmaLiteral, pTempState); //Negate literals in the path.
						 pTempState = (SmurfStateEntry *)pTempState->pPreviousState;
					 }
					 d7_printf1("\n");
					 );
				
				//Add a lemma as reference to this inference.
				create_clause_from_Smurf((((InferenceStateEntry *)pNextState)->bPolarity > 0)?nInfVar:-nInfVar,
												 SimpleSmurfProblemState->arrReverseOccurenceList[nSmurfNumber][0].var,
												 pTopState, 
												 &(SimpleSmurfProblemState->arrInferenceLemmas[nInfVar].clause),
												 &(SimpleSmurfProblemState->arrInferenceLemmas[nInfVar].max_size));
				
				if(EnqueueInference(nInfVar, ((InferenceStateEntry *)pNextState)->bPolarity > 0) == 0) return 0;

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
				//pNextState = ((void *)((InferenceStateEntry *)pPrevState)->pVarTransition);
			}
			
			//Record the transition.
			if(((SmurfStateEntry *)pNextState) == pTrueSimpleSmurfState) {
				SimpleSmurfProblemState->arrSmurfStack[SimpleSmurfProblemState->nCurrSearchTreeLevel].nNumSmurfsSatisfied++;
			} else if(pNextState == arrSmurfStates[((TypeStateEntry *)pNextState)->pStateOwner]) {
				d7_printf3("      State %x currently owned by Smurf %d, transitioning to True\n", pNextState, ((TypeStateEntry *)pNextState)->pStateOwner);
				pNextState = pTrueSimpleSmurfState;
			} else {
				((TypeStateEntry *)pNextState)->pPreviousState = pTopState;
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

//Seems to be working great...so far.
ITE_INLINE
int ApplyInferenceToWatchedSmurf(int nBranchVar, bool bBVPolarity, int nSmurfNumber, void **arrSmurfStates) {
//int ApplyInferenceToSmurf(int nBranchVar, bool bBVPolarity, int nSmurfNumber, void **arrSmurfStates) {	
	int nInfQueueHead = SimpleSmurfProblemState->arrSmurfStack[SimpleSmurfProblemState->nCurrSearchTreeLevel].nNumFreeVars;
	//SEAN!!! inferences ahead of current position may be inferred here - changes the functionality a little. ???

	SmurfStateEntry *pSmurfState = (SmurfStateEntry *)arrSmurfStates[nSmurfNumber];
	SmurfStateEntry *pTopSmurf = pSmurfState;
	
	do {
		//The abs() is necessary because old choicepoints are negative
		int nInfVar = pSmurfState->nTransitionVar;
		int nPrevInfLevel = abs(SimpleSmurfProblemState->arrInferenceDeclaredAtLevel[nInfVar]);
		
		if(nPrevInfLevel < nInfQueueHead) { //Apply this inference
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

			//arrSmurfStates[nSmurfNumber] = pNextState;
			pTopSmurf = (SmurfStateEntry *)pNextState;
			if(((TypeStateEntry *)pNextState)->cType == FN_SMURF || ((TypeStateEntry *)pNextState)->cType == FN_WATCHED_SMURF) {
				//Record the transition.
				pSmurfState = (SmurfStateEntry *)pNextState;
			} else {
				break; //New functionType detected
			}
		} else {
			pSmurfState = (SmurfStateEntry *)pSmurfState->pNextVarInThisState;
		}
	} while (pSmurfState != NULL && pSmurfState!=(SmurfStateEntry *)pTrueSimpleSmurfState);

//	if(arrSmurfStates[nSmurfNumber] == pTrueSimpleSmurfState) {
//		SimpleSmurfProblemState->arrSmurfStack[SimpleSmurfProblemState->nCurrSearchTreeLevel].nNumSmurfsSatisfied++;
	if(pTopSmurf == pTrueSimpleSmurfState) {
		d9_printf2("      Smurf %d transitioned to True\n", nSmurfNumber);
	} else if(((TypeStateEntry *)arrSmurfStates[nSmurfNumber])->cType != FN_WATCHED_SMURF) {
		//This doesn't make sense unless all special smurfs are watched.
		if(((TypeStateEntry *)arrSmurfStates[nSmurfNumber])->ApplyInferenceToState(nBranchVar, bBVPolarity, nSmurfNumber, arrSmurfStates) == 0)
		  return 0;
	} else {
		//Setup a new watched variables

		//Refresh nInfQueueHead because inferences could have been made by this watched smurf.
		nInfQueueHead = SimpleSmurfProblemState->arrSmurfStack[SimpleSmurfProblemState->nCurrSearchTreeLevel].nNumFreeVars;

		int nNumWatchedNeeded_NotSet;
		int nNumWatchedNeeded_Set;
		int nNumRevListVars = SimpleSmurfProblemState->arrReverseOccurenceList[nSmurfNumber][0].var;
		int nNumSmurfVars = 0;

		d9_printf3("      Current watches for Smurf %d (%d):", nSmurfNumber, nInfQueueHead);

		for(int x = 1; x <= nNumRevListVars; x++) {
			int nInfVar = SimpleSmurfProblemState->arrReverseOccurenceList[nSmurfNumber][x].var;
			if(abs(SimpleSmurfProblemState->arrInferenceDeclaredAtLevel[nInfVar]) >= nInfQueueHead) nNumSmurfVars++;
			D_9(
			  if((*SimpleSmurfProblemState->arrReverseOccurenceList[nSmurfNumber][x].loc)>=0) {
				  d9_printf3("*%d(%d) ", nInfVar, abs(SimpleSmurfProblemState->arrInferenceDeclaredAtLevel[nInfVar]));
			  } else {
				  d9_printf3("%d(%d) ", nInfVar, abs(SimpleSmurfProblemState->arrInferenceDeclaredAtLevel[nInfVar]));
			  }
			);
		}
		d9_printf1("\n");
		
		if(nNumSmurfVars >= numSmurfWatchedVars) {
			nNumWatchedNeeded_NotSet = numSmurfWatchedVars;
			nNumWatchedNeeded_Set = 0;
		} else {
			nNumWatchedNeeded_NotSet = nNumSmurfVars;
			nNumWatchedNeeded_Set = numSmurfWatchedVars - nNumSmurfVars;
		}

		for(int x = 1; x <= nNumRevListVars; x++) {
			int nInfVar = SimpleSmurfProblemState->arrReverseOccurenceList[nSmurfNumber][x].var;
			if(abs(SimpleSmurfProblemState->arrInferenceDeclaredAtLevel[nInfVar]) < nInfQueueHead && (nNumWatchedNeeded_Set > 0)) {
				//Variable has been assigned
				if((*SimpleSmurfProblemState->arrReverseOccurenceList[nSmurfNumber][x].loc)<0) {
					(*SimpleSmurfProblemState->arrReverseOccurenceList[nSmurfNumber][x].loc) = -(*SimpleSmurfProblemState->arrReverseOccurenceList[nSmurfNumber][x].loc);
					d9_printf2("      Adding watch on fixed variable %d\n", nInfVar);
				}
				nNumWatchedNeeded_Set--;
			} else if(abs(SimpleSmurfProblemState->arrInferenceDeclaredAtLevel[nInfVar]) >= nInfQueueHead && (nNumWatchedNeeded_NotSet > 0)) {
				//Variable has not been assigned
				if((*SimpleSmurfProblemState->arrReverseOccurenceList[nSmurfNumber][x].loc)<0) {
					(*SimpleSmurfProblemState->arrReverseOccurenceList[nSmurfNumber][x].loc) = -(*SimpleSmurfProblemState->arrReverseOccurenceList[nSmurfNumber][x].loc);
					d9_printf2("      Adding watch on free variable %d\n", nInfVar);
				}
				nNumWatchedNeeded_NotSet--;
			} else if((*SimpleSmurfProblemState->arrReverseOccurenceList[nSmurfNumber][x].loc)>0) {
				(*SimpleSmurfProblemState->arrReverseOccurenceList[nSmurfNumber][x].loc) = -(*SimpleSmurfProblemState->arrReverseOccurenceList[nSmurfNumber][x].loc);
				d9_printf2("      Removing watch on fixed variable %d\n", nInfVar);
			}
		}
	}

	int ret = ApplyInferenceToSmurf_Hooks(nBranchVar, bBVPolarity, nSmurfNumber, arrSmurfStates);

	return ret;
}

