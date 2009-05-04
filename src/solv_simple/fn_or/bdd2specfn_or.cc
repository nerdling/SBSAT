#include "sbsat.h"
#include "sbsat_solver.h"
#include "solver.h"

/********** Included in include/sbsat_solver.h *********************

struct TypeStateEntry {
   char cType;
}

struct ORStateEntry {
   char cType; //FN_OR
   int *pnTransitionVars;
   bool *bPolarity;
   int nSize;
};

struct ORCounterStateEntry {
   char cType; //FN_OR_COUNTER
   void *pTransition;
   int nSize;
   ORStateEntry *pORState;
};

***********************************************************************/

void *CreateORState(int *arrElts, int nNumElts, BDDNode *pCurrentBDD, ORStateEntry *pStartState) {
	check_SmurfStatesTableSize(sizeof(ORStateEntry));
	ite_counters[SMURF_STATES]+=1;

	pStartState = (ORStateEntry *)SimpleSmurfProblemState->pSmurfStatesTableTail;
	SimpleSmurfProblemState->pSmurfStatesTableTail = (void *)(pStartState + 1);
	pStartState->cType = FN_OR;
	pStartState->pnTransitionVars = arrElts;
   pStartState->nSize = nNumElts;
	pStartState->bPolarity = (bool *)ite_calloc(nNumElts, sizeof(bool), 9, "bPolarity");
	for(int x = 0; x < nNumElts; x++) {
//		fprintf(stderr, "%d, %d, %d: ", x, arrElts[x], arrSimpleSolver2IteVarMap[arrElts[x]]);
		if(set_variable(pCurrentBDD, arrSimpleSolver2IteVarMap[arrElts[x]], 1) == true_ptr)
		  pStartState->bPolarity[x] = 1;
	}
   
//	printBDDerr(pCurrentBDD);
//	fprintf(stderr, "\n");
//	PrintAllSmurfStateEntries();

	pCurrentBDD->pState = (void *)pStartState;
   pStartState->pORStateBDD = pCurrentBDD;
	
	if(nNumElts == 2)	return (void *)pStartState;

	ORCounterStateEntry *pCurrORCounter=NULL;
	void *pPrevORCounter = (void *)pStartState;
	for(int x = 2; x < nNumElts; x++) {
//		fprintf(stderr, "%d, %d, %d: ", x, arrElts[x], arrSimpleSolver2IteVarMap[arrElts[x]]);
		check_SmurfStatesTableSize(sizeof(ORCounterStateEntry));
		ite_counters[SMURF_STATES]+=1;
		pCurrORCounter = (ORCounterStateEntry *)SimpleSmurfProblemState->pSmurfStatesTableTail;
		SimpleSmurfProblemState->pSmurfStatesTableTail = (void *)(pCurrORCounter + 1);
		pCurrORCounter->cType = FN_OR_COUNTER;
		pCurrORCounter->pTransition = pPrevORCounter;
//		pCurrORCounter->pTransition->pPreviousState = pCurrORCounter;
		pCurrORCounter->pORState = pStartState;
		pCurrORCounter->nSize = x+1;
		pPrevORCounter = (void *)pCurrORCounter;
	}

	pCurrentBDD->pState = (void *)pCurrORCounter;
	pStartState->pORCounterState = pCurrORCounter;
	return (void *)pCurrORCounter;
}
