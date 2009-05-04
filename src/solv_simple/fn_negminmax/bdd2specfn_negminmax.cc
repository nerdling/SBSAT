#include "sbsat.h"
#include "sbsat_solver.h"
#include "solver.h"

void *CreateNEGMINMAXState(int *arrElts, int nNumElts, BDDNode *pCurrentBDD, NEGMINMAXStateEntry *pStartState) {
	check_SmurfStatesTableSize(sizeof(NEGMINMAXStateEntry));
	ite_counters[SMURF_STATES]+=1;

	pStartState = (NEGMINMAXStateEntry *)SimpleSmurfProblemState->pSmurfStatesTableTail;
	SimpleSmurfProblemState->pSmurfStatesTableTail = (void *)(pStartState + 1);

	//assert(nNumElts > 2); //OR and XOR cover all 2 variable Boolean functions w/ no immediate inferences.

	int max = -1;
	BDDNode *tmp_bdd;
	for(tmp_bdd = pCurrentBDD; !IS_TRUE_FALSE(tmp_bdd); tmp_bdd = tmp_bdd->thenCase)
	  max++;
	
	assert(tmp_bdd == true_ptr);
	
	int min = -1;
	for(tmp_bdd = pCurrentBDD; !IS_TRUE_FALSE(tmp_bdd); tmp_bdd = tmp_bdd->elseCase)
	  min++;
	if(tmp_bdd == false_ptr) min = 0;
	else {
		assert(tmp_bdd == true_ptr);
		min = nNumElts-min;
	}

	assert(min <= max);
	
	pStartState->cType = FN_NEGMINMAX;
   pStartState->nSize = nNumElts;
	pStartState->nMin = min;
	pStartState->nMax = max;
	pStartState->pnTransitionVars = arrElts;
   
	NEGMINMAXCounterStateEntry *pCurrNEGMINMAXCounter=NULL;
	void *pPrevNEGMINMAXCounter = (void *)pTrueSimpleSmurfState;//pStartState;
	for(int x = 1; x <= nNumElts; x++) {
//		fprintf(stderr, "%d, %d, %d: ", x, arrElts[x], arrSimpleSolver2IteVarMap[arrElts[x]]);
		check_SmurfStatesTableSize(sizeof(NEGMINMAXCounterStateEntry));
		ite_counters[SMURF_STATES]+=1;
		
		pCurrNEGMINMAXCounter = (NEGMINMAXCounterStateEntry *)SimpleSmurfProblemState->pSmurfStatesTableTail;
		SimpleSmurfProblemState->pSmurfStatesTableTail = (void *)(pCurrNEGMINMAXCounter + 1);

		pCurrNEGMINMAXCounter->cType = FN_NEGMINMAX_COUNTER;

		pCurrNEGMINMAXCounter->nVarsLeft = x;
		pCurrNEGMINMAXCounter->nNumTrue = 0; //Dynamic
		pCurrNEGMINMAXCounter->pTransition = pPrevNEGMINMAXCounter;
		pCurrNEGMINMAXCounter->pNEGMINMAXState = pStartState;
		pPrevNEGMINMAXCounter = (void *)pCurrNEGMINMAXCounter;
	}

	pCurrentBDD->pState = (void *)pCurrNEGMINMAXCounter;
   pStartState->pNEGMINMAXStateBDD = pCurrentBDD;
   
	return (void *)pCurrNEGMINMAXCounter;
}
