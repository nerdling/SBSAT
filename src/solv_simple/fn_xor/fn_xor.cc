#include "sbsat.h"
#include "sbsat_solver.h"
#include "solver.h"

// XOR State

void initXORStateType() {
	arrStatesTypeSize[FN_XOR] = sizeof(XORStateEntry);
   arrSetVisitedState[FN_XOR] = SetVisitedXORState;
	arrApplyInferenceToState[FN_XOR] = ApplyInferenceToXOR;
	arrPrintStateEntry[FN_XOR] = PrintXORStateEntry;
	arrPrintStateEntry_dot[FN_XOR] = PrintXORStateEntry_dot;
	arrFreeStateEntry[FN_XOR] = FreeXORStateEntry;
	arrCalculateStateHeuristic[FN_XOR] = CalculateXORLSGBHeuristic;
   arrSetStateHeuristicScore[FN_XOR] = LSGBXORStateSetHeurScore;
   arrGetStateHeuristicScore[FN_XOR] = LSGBXORStateGetHeurScore;
}

void PrintXORStateEntry(void *pState) {
	XORStateEntry *pXORState = (XORStateEntry *)pState;
	d9_printf2("XOR Parity=%d ", pXORState->bParity);
	d9_printf1("Vars=");
	for(int x = 0; x < pXORState->nSize; x++)
	  d9_printf2("%d ", pXORState->pnTransitionVars[x]);
	d9_printf2("Size=%d\n", pXORState->nSize);
}

void PrintXORStateEntry_dot(void *pState) {
	XORStateEntry *pXORState = (XORStateEntry *)pState;
   fprintf(stdout, " b%p->b%p [style=solid, fontname=\"Helvetica\",label=\"n--\"]\n", (void *)pXORState, (void *)pXORState);
	fprintf(stdout, " b%p->i%p [style=solid, fontname=\"Helvetica\",label=\"If n==1\"]\n", (void *)pXORState, (void *)pXORState);
	fprintf(stdout, " i%p->b%p [style=solid, fontname=\"Helvetica\",label=\"v=p\"]\n", (void *)pXORState, pTrueSimpleSmurfState);
	
	fprintf(stdout, " i%p [shape=\"ellipse\", label=\"I\"]\n", (void *)pXORState);
	fprintf(stdout, " b%p [shape=\"ellipse\", label=\"XOR, n=%d\"]\n", pXORState, pXORState->nSize);
}

void PrintXORStateEntry_formatted(void *pState) {
	XORStateEntry *pXORState = (XORStateEntry *)pState;
	d2_printf2("equ(%c ", pXORState->bParity?'T':'F');
	d2_printf1("xor(");
	for(int x = 0; x < pXORState->nSize; x++) {
		if(SimpleSmurfProblemState->arrInferenceDeclaredAtLevel[pXORState->pnTransitionVars[x]] == SimpleSmurfProblemState->nNumVars)
		  d2_printf2("%d ", pXORState->pnTransitionVars[x]);
	}
	d2_printf1("))\n");
}

void FreeXORStateEntry(void *pState) {
	XORStateEntry *pXORState = (XORStateEntry *)pState;
	ite_free((void **)&pXORState->pnTransitionVars);
   if(pXORState->pXORStateBDD != NULL) {
      pXORState->pXORStateBDD->pState = NULL;
   }
}

// XOR Counter State

void initXORCounterStateType() {
	arrStatesTypeSize[FN_XOR_COUNTER] = sizeof(XORCounterStateEntry);
   arrSetVisitedState[FN_XOR_COUNTER] = SetVisitedXORCounterState;
	arrApplyInferenceToState[FN_XOR_COUNTER] = ApplyInferenceToXORCounter;
	arrPrintStateEntry[FN_XOR_COUNTER] = PrintXORCounterStateEntry;
	arrPrintStateEntry_dot[FN_XOR_COUNTER] = PrintXORCounterStateEntry_dot;
	arrFreeStateEntry[FN_XOR_COUNTER] = FreeXORCounterStateEntry;
	arrCalculateStateHeuristic[FN_XOR_COUNTER] = CalculateXORCounterLSGBHeuristic;
   arrSetStateHeuristicScore[FN_XOR_COUNTER] = LSGBXORCounterStateSetHeurScore;
   arrGetStateHeuristicScore[FN_XOR_COUNTER] = LSGBXORCounterStateGetHeurScore;
}

void PrintXORCounterStateEntry(void *pState) {
	XORCounterStateEntry *pXORCounterState = (XORCounterStateEntry *)pState;
	d9_printf4("XOR Next=%p Head=%p Size=%d\n", (void *)pXORCounterState->pTransition, (void *)pXORCounterState->pXORState, pXORCounterState->nSize);
}

void PrintXORCounterStateEntry_dot(void *pState) {
	XORCounterStateEntry *pXORCounterState = (XORCounterStateEntry *)pState;
   fprintf(stdout, " b%p->b%p [style=solid, fontname=\"Helvetica\",label=\"n--\"]\n", (void *)pXORCounterState, (void *)pXORCounterState);
	fprintf(stdout, " b%p->i%p [style=solid, fontname=\"Helvetica\",label=\"If n==1\"]\n", (void *)pXORCounterState, (void *)pXORCounterState);
	fprintf(stdout, " i%p->b%p [style=solid, fontname=\"Helvetica\",label=\"v=p\"]\n", (void *)pXORCounterState, pTrueSimpleSmurfState);
	
	fprintf(stdout, " i%p [shape=\"ellipse\", label=\"I\"]\n", (void *)pXORCounterState);
	fprintf(stdout, " b%p [shape=\"ellipse\", label=\"XOR, n=%d\"]\n", (void *)pXORCounterState, pXORCounterState->pXORState->nSize);
}

void FreeXORCounterStateEntry(void *pState) {

}

// Gaussian Elimination State

int return_1(int nBranchVar, bool bBVPolarity, int nSmurfNumber, void **arrSmurfStates) {
	return 1;
}

void initXORGElimStateType() {
	arrStatesTypeSize[FN_XOR_GELIM] = sizeof(XORGElimStateEntry);
   arrSetVisitedState[FN_XOR_GELIM] = SetVisitedXORGElimState;
	arrApplyInferenceToState[FN_XOR_GELIM] = return_1;
	arrPrintStateEntry[FN_XOR_GELIM] = PrintXORGElimStateEntry;
	arrPrintStateEntry_dot[FN_XOR_GELIM] = PrintXORGElimStateEntry_dot;
	arrFreeStateEntry[FN_XOR_GELIM] = FreeXORGElimStateEntry;
	arrCalculateStateHeuristic[FN_XOR_GELIM] = CalculateXORGElimLSGBHeuristic;
   arrSetStateHeuristicScore[FN_XOR_GELIM] = LSGBXORStateSetHeurScore;
   arrGetStateHeuristicScore[FN_XOR_GELIM] = LSGBarrXORWeight;
}

void PrintXORGElimStateEntry(void *pState) {
	XORGElimStateEntry *pXORGElimState = (XORGElimStateEntry *)pState;
	d9_printf2("XOR Parity=%d ", ((int *)pXORGElimState->pVector)[0]&1);
	d9_printf1("Vars=");
	for(int x = 0; x < pXORGElimState->nSize; x++)
	  d9_printf2("%d ", pXORGElimState->pnTransitionVars[x]);
	d9_printf2("Size=%d ", pXORGElimState->nSize);
	d9_printf1("Vector= ");
	PrintXORGElimVector(pXORGElimState->pVector);
	d9_printf1("\n");
}

void PrintXORGElimStateEntry_dot(void *pState) {
	XORGElimStateEntry *pXORGElimState = (XORGElimStateEntry *)pState;
   fprintf(stdout, " b%p->b%p [style=solid, fontname=\"Helvetica\",label=\"n--\"]\n", (void *)pXORGElimState, (void *)pXORGElimState);
	fprintf(stdout, " b%p->i%p [style=solid, fontname=\"Helvetica\",label=\"If n==1\"]\n", (void *)pXORGElimState, (void *)pXORGElimState);
	fprintf(stdout, " i%p->b%p [style=solid, fontname=\"Helvetica\",label=\"v=p\"]\n", (void *)pXORGElimState, pTrueSimpleSmurfState);
	
	fprintf(stdout, " i%p [shape=\"ellipse\", label=\"I\"]\n", (void *)pXORGElimState);
	fprintf(stdout, " b%p [shape=\"ellipse\", label=\"GE XOR, n=%d\"]\n", (void *)pXORGElimState, pXORGElimState->nSize);
}

void FreeXORGElimStateEntry(void *pState) {
	XORGElimStateEntry *pXORGElimState = (XORGElimStateEntry *)pState;
	ite_free((void **)&pXORGElimState->pnTransitionVars);
	ite_free((void **)&pXORGElimState->pVector);

	if(pXORGElimState->pXORGElimStateBDD != NULL) {
      pXORGElimState->pXORGElimStateBDD->pState = NULL;
   }
}
