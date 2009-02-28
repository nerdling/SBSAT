#include "sbsat.h"
#include "sbsat_solver.h"
#include "solver.h"

ITE_INLINE
void create_clause_from_SmurfState(int nInfVar, TypeStateEntry *pSmurfState, int nNumVarsInSmurf,
										Cls **clause, int *lits_max_size) {
	Lit ** p;
	
	if((*clause)->used == 1) return; //Clause is already in the queue.
	
	if(nNumVarsInSmurf > (*lits_max_size)) {
		(*clause) = (Cls *)realloc((*clause), bytes_clause(nNumVarsInSmurf, 0));
		(*lits_max_size) = nNumVarsInSmurf;
	}

	d7_printf2("        Lemma: %d", nInfVar);
	
	p = (*clause)->lits;
	(*p++) = int2lit(nInfVar);
	int nNumVars = 1;
	while(pSmurfState != NULL) {
		d7_printf3(" %d(%x)", pSmurfState->nLemmaLiteral, pSmurfState); //Negate literals in the path.
		(*p++) = int2lit(pSmurfState->nLemmaLiteral);
		pSmurfState = (TypeStateEntry *)pSmurfState->pPreviousState;
		nNumVars++;
	}
	d7_printf1("\n");
	(*clause)->size = nNumVars;
}

ITE_INLINE
int SmurfState_InferWithLemma(SmurfStateEntry *pTopState, void *pNextState, int nInfVar, int nSmurfNumber) {
	int nInfQueueHead = SimpleSmurfProblemState->arrSmurfStack[SimpleSmurfProblemState->nCurrSearchTreeLevel].nNumFreeVars;
	int nPrevInfLevel = abs(SimpleSmurfProblemState->arrInferenceDeclaredAtLevel[nInfVar]);
	if(nPrevInfLevel < nInfQueueHead) {
		if((SimpleSmurfProblemState->arrInferenceQueue[nPrevInfLevel] > 0) != ((InferenceStateEntry *)pNextState)->bPolarity) {
			//Conflict
			//Add a lemma to reference this conflict.
			create_clause_from_SmurfState((((InferenceStateEntry *)pNextState)->bPolarity)?nInfVar:-nInfVar,
													(TypeStateEntry *)pTopState,
													SimpleSmurfProblemState->arrReverseOccurenceList[nSmurfNumber][0].var,
													&(SimpleSmurfProblemState->pConflictClause.clause),
													&(SimpleSmurfProblemState->pConflictClause.max_size));
			int ret = EnqueueInference(nInfVar, ((InferenceStateEntry *)pNextState)->bPolarity, INF_SMURF);
			assert(ret == 0);
			return 0;
		} else {
			//Lit already assigned.
			d7_printf2("      Inference %d already inferred\n", (((InferenceStateEntry *)pNextState)->bPolarity)?nInfVar:-nInfVar);
			return 1;
		} 
	} else {
		//Add a lemma as reference to this inference.
		create_clause_from_SmurfState((((InferenceStateEntry *)pNextState)->bPolarity > 0)?nInfVar:-nInfVar,
												(TypeStateEntry *)pTopState,
												SimpleSmurfProblemState->arrReverseOccurenceList[nSmurfNumber][0].var,
												&(SimpleSmurfProblemState->arrInferenceLemmas[nInfVar].clause),
												&(SimpleSmurfProblemState->arrInferenceLemmas[nInfVar].max_size));
		
		if(EnqueueInference(nInfVar, ((InferenceStateEntry *)pNextState)->bPolarity, INF_SMURF) == 0) return 0;
	}
	return 1;
}

ITE_INLINE
int TransitionInference(int nSmurfNumber, void **arrSmurfStates) {
	void *pNextState = arrSmurfStates[nSmurfNumber];
	assert(((TypeStateEntry *)pNextState)->cType == FN_INFERENCE);

	void *pPrevState = NULL;
	while(pNextState!=NULL && ((TypeStateEntry *)pNextState)->cType == FN_INFERENCE) {
		int nInfVar = ((InferenceStateEntry *)pNextState)->nTransitionVar;
		
		if(use_lemmas) {
			if(SmurfState_InferWithLemma(NULL, pNextState, nInfVar, nSmurfNumber) == 0) return 0;
		} else {
			if(EnqueueInference(nInfVar, ((InferenceStateEntry *)pNextState)->bPolarity, INF_SMURF) == 0) return 0;
		}
		
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
		((TypeStateEntry *)pNextState)->pPreviousState = arrSmurfStates[nSmurfNumber];
		((TypeStateEntry *)pNextState)->pStateOwner = nSmurfNumber;
	}

	arrSmurfStates[nSmurfNumber] = pNextState;
	
	//This is slightly abusive, but needed to catch linear functions for the GE table
	int ret = ApplyInferenceToSmurf_Hooks(0, 1, nSmurfNumber, arrSmurfStates);

	d7_printf3("      Smurf %d transitioned to state %x\n", nSmurfNumber, arrSmurfStates[nSmurfNumber]);
	d7_printf3("      Smurf %d previously was %x\n", nSmurfNumber, ((SmurfStateEntry *)arrSmurfStates[nSmurfNumber])->pPreviousState);

	return ret;
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

				int nInfVar = ((InferenceStateEntry *)pNextState)->nTransitionVar;

				if(use_lemmas) {
					if(SmurfState_InferWithLemma((SmurfStateEntry *)arrSmurfStates[nSmurfNumber], pNextState, nInfVar, nSmurfNumber) == 0) return 0;
				} else {
					if(EnqueueInference(nInfVar, ((InferenceStateEntry *)pNextState)->bPolarity, INF_SMURF) == 0) return 0;
				}

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

