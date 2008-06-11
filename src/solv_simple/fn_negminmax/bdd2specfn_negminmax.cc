#include "sbsat.h"
#include "sbsat_solver.h"
#include "solver.h"

void *CreateMINMAXState(int *arrElts, int nNumElts, BDDNode *pCurrentBDD, MINMAXStateEntry *pStartState) {
	check_SmurfStatesTableSize(sizeof(MINMAXStateEntry));
	ite_counters[SMURF_STATES]+=1;

	pStartState = (MINMAXStateEntry *)SimpleSmurfProblemState->pSmurfStatesTableTail;
	SimpleSmurfProblemState->pSmurfStatesTableTail = (void *)(pStartState + 1);
	
	if(IS_TRUE_FALSE(bdd)) assert(0);
	int max = -1;
	BDDNode *tmp_bdd;
	for(tmp_bdd = pCurrentBDD; !IS_TRUE_FALSE(tmp_bdd); tmp_bdd = tmp_bdd->thenCase)
	  max++;
	if(tmp_bdd != false_ptr) assert(0);
	
	int min = -1;
	for(tmp_bdd = pCurrentBDD; !IS_TRUE_FALSE(tmp_bdd); tmp_bdd = tmp_bdd->elseCase)
	  min++;
	if(tmp_bdd == true_ptr) min = 0;
	else if(tmp_bdd != false_ptr) assert(0);
	else min = bdd_len-min;
	
	if(min > max) assert(0);
	
	pStartState->cType = FN_MINMAX;
   pStartState->nSize = nNumElts;
	pStartState->nMin = min;
	pStartState->nMax = max;
	pStartState->pnTransitionVars = arrElts;

	pCurrentBDD->pState = (void *)pStartState;
	
	if(nNumElts == 2) return (void *)pStartState;

	MINMAXStateEntry *pCurrMINMAXCounter;
	void *pPrevMINMAXCounter = (void *)pTrueSimpleSmurfState;//pStartState;
	for(int x = 1; x < nNumElts; x++) {
//		fprintf(stderr, "%d, %d, %d: ", x, arrElts[x], arrSimpleSolver2IteVarMap[arrElts[x]]);
		check_SmurfStatesTableSize(sizeof(MINMAXCounterStateEntry));
		ite_counters[SMURF_STATES]+=1;
		
		pCurrMINMAXCounter = (MINMAXCounterStateEntry *)SimpleSmurfProblemState->pSmurfStatesTableTail;
		SimpleSmurfProblemState->pSmurfStatesTableTail = (void *)(pCurrMINMAXCounter + 1);

		pCurrMINMAXCounter->cType = FN_MINMAX_COUNTER;
		pCurrMINMAXCounter->ApplyInferenceToState = ApplyInferenceToMINMAXCounter;

		pCurrMINMAXCounter->bTop = 0;
		pCurrMINMAXCounter->nVarsLeft = x;
		pCurrMINMAXCounter->nNumTrue = 0;
		pCurrMINMAXCounter->pTransition = pPrevMINMAXCounter;
		pCurrMINMAXCounter->pMINMAXState = pStartState;
		pPrevMINMAXCounter = (void *)pCurrMINMAXCounter;
	}

	pCurrMINMAXCounter->bTop = 1;
	pCurrentBDD->pState = (void *)pCurrMINMAXCounter;
	return (void *)pCurrMINMAXCounter;
}
