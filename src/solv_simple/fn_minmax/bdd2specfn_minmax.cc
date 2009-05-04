#include "sbsat.h"
#include "sbsat_solver.h"
#include "solver.h"

void *CreateMINMAXState(int *arrElts, int nNumElts, BDDNode *pCurrentBDD, MINMAXStateEntry *pStartState) {
	check_SmurfStatesTableSize(sizeof(MINMAXStateEntry));
	ite_counters[SMURF_STATES]+=1;

	pStartState = (MINMAXStateEntry *)SimpleSmurfProblemState->pSmurfStatesTableTail;
	SimpleSmurfProblemState->pSmurfStatesTableTail = (void *)(pStartState + 1);

	//assert(nNumElts > 2); //OR and XOR cover all 2 variable Boolean functions w/ no immediate inferences.

	int max = -1;
	BDDNode *tmp_bdd;
	for(tmp_bdd = pCurrentBDD; !IS_TRUE_FALSE(tmp_bdd); tmp_bdd = tmp_bdd->thenCase)
	  max++;
	
	assert(tmp_bdd == false_ptr);
	
	int min = -1;
	for(tmp_bdd = pCurrentBDD; !IS_TRUE_FALSE(tmp_bdd); tmp_bdd = tmp_bdd->elseCase)
	  min++;
	if(tmp_bdd == true_ptr) min = 0;
	else {
		assert(tmp_bdd == false_ptr);
		min = nNumElts-min;
	}

	assert(min <= max);
	
	pStartState->cType = FN_MINMAX;
   pStartState->nSize = nNumElts;
	pStartState->nMin = min;
	pStartState->nMax = max;
	pStartState->pnTransitionVars = arrElts;
   
	MINMAXCounterStateEntry *pCurrMINMAXCounter=NULL;
	void *pPrevMINMAXCounter = (void *)pTrueSimpleSmurfState;//pStartState;
	for(int x = 1; x <= nNumElts; x++) {
//		fprintf(stderr, "%d, %d, %d: ", x, arrElts[x], arrSimpleSolver2IteVarMap[arrElts[x]]);
		check_SmurfStatesTableSize(sizeof(MINMAXCounterStateEntry));
		ite_counters[SMURF_STATES]+=1;
		
		pCurrMINMAXCounter = (MINMAXCounterStateEntry *)SimpleSmurfProblemState->pSmurfStatesTableTail;
		SimpleSmurfProblemState->pSmurfStatesTableTail = (void *)(pCurrMINMAXCounter + 1);

		pCurrMINMAXCounter->cType = FN_MINMAX_COUNTER;

		pCurrMINMAXCounter->nVarsLeft = x;
		pCurrMINMAXCounter->nNumTrue = 0; //Dynamic
		pCurrMINMAXCounter->pTransition = pPrevMINMAXCounter;
		pCurrMINMAXCounter->pMINMAXState = pStartState;
		pPrevMINMAXCounter = (void *)pCurrMINMAXCounter;
	}

	pCurrentBDD->pState = (void *)pCurrMINMAXCounter;
   pStartState->pMINMAXStateBDD = pCurrentBDD;
   
	return (void *)pCurrMINMAXCounter;
}
