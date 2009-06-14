#include "sbsat.h"
#include "sbsat_solver.h"
#include "solver.h"

void *CreateXORState(int *arrElts, int nNumElts, BDDNode *pCurrentBDD, XORStateEntry *pStartState) {
	check_SmurfStatesTableSize(sizeof(XORStateEntry));
	ite_counters[SMURF_STATES]+=1;

	pStartState = (XORStateEntry *)SimpleSmurfProblemState->pSmurfStatesTableTail;
	SimpleSmurfProblemState->pSmurfStatesTableTail = (void *)(pStartState + 1);
	pStartState->cType = FN_XOR;
	pStartState->pnTransitionVars = arrElts;
   pStartState->nSize = nNumElts;
	BDDNode *tmp_bdd;
	for(tmp_bdd = pCurrentBDD; !IS_TRUE_FALSE(tmp_bdd); tmp_bdd = tmp_bdd->thenCase){}
	pStartState->bParity = tmp_bdd == false_ptr;
	if((nNumElts&0x1) == 1)
	  pStartState->bParity -= 1;

//	fprintf(stderr, "\nb=%d\n", pStartState->bParity);
//	printBDDerr(pCurrentBDD);
//	fprintf(stderr, "\n");
//	PrintAllSmurfStateEntries();

	pCurrentBDD->pState = (void *)pStartState;
   pStartState->pXORStateBDD = pCurrentBDD;
	
	if(nNumElts == 2) return (void *)pStartState;

	XORCounterStateEntry *pCurrXORCounter=NULL;
	void *pPrevXORCounter = (void *)pStartState;
	for(int x = 2; x < nNumElts; x++) {
//		fprintf(stderr, "%d, %d, %d: ", x, arrElts[x], arrSimpleSolver2IteVarMap[arrElts[x]]);
		check_SmurfStatesTableSize(sizeof(XORCounterStateEntry));
		ite_counters[SMURF_STATES]+=1;
		pCurrXORCounter = (XORCounterStateEntry *)SimpleSmurfProblemState->pSmurfStatesTableTail;
		SimpleSmurfProblemState->pSmurfStatesTableTail = (void *)(pCurrXORCounter + 1);
		pCurrXORCounter->cType = FN_XOR_COUNTER;
		pCurrXORCounter->pTransition = pPrevXORCounter;
		pCurrXORCounter->pXORState = pStartState;
		pCurrXORCounter->nSize = x+1;
		pPrevXORCounter = (void *)pCurrXORCounter;
	}

	return pCurrentBDD->pState = (void *)pCurrXORCounter;
}

void *CreateXORGElimState(int *arrElts, int nNumElts, BDDNode *pCurrentBDD, XORGElimStateEntry *pStartState) {
	check_SmurfStatesTableSize(sizeof(XORGElimStateEntry));
	ite_counters[SMURF_STATES]+=1;
	
	pStartState = (XORGElimStateEntry *)SimpleSmurfProblemState->pSmurfStatesTableTail;
	SimpleSmurfProblemState->pSmurfStatesTableTail = (void *)(pStartState + 1);
	pStartState->cType = FN_XOR_GELIM;
	pStartState->pnTransitionVars = arrElts;
   pStartState->nSize = nNumElts;
	pStartState->pXORGElimStateBDD = pCurrentBDD;
	BDDNode *tmp_bdd;
	for(tmp_bdd = pCurrentBDD; !IS_TRUE_FALSE(tmp_bdd); tmp_bdd = tmp_bdd->thenCase){}
	bool bParity = tmp_bdd == false_ptr;
	if((nNumElts&0x1) == 1)
	  bParity -= 1;

	pStartState->pVector = createXORGElimTableVector(nNumElts, arrElts, bParity);
	
	//	fprintf(stderr, "\nb=%d\n", pStartState->bParity);
	//	printBDDerr(pCurrentBDD);
	//	fprintf(stderr, "\n");
	//	PrintAllSmurfStateEntries();
	
	return pCurrentBDD->pState = (void *)pStartState;
}
