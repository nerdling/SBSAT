#include "sbsat.h"
#include "sbsat_solver.h"
#include "solver.h"

void PrintXORStateEntry(XORStateEntry *ssEntry) {
	d9_printf2("Parity=%d ", ssEntry->bParity);
	d9_printf1("Vars=");
	for(int x = 0; x < ssEntry->nSize; x++)
	  d9_printf2("%d ", ssEntry->pnTransitionVars[x]);
	d9_printf2("Size=%d\n", ssEntry->nSize);
}

void PrintXORStateEntry_dot(XORStateEntry *ssEntry) {
	d9_printf2("Parity=%d ", ssEntry->bParity);
	d9_printf1("Vars=");
	for(int x = 0; x < ssEntry->nSize; x++)
	  d9_printf2("%d ", ssEntry->pnTransitionVars[x]);
	d9_printf2("Size=%d\n", ssEntry->nSize);
}

void PrintXORCounterStateEntry(XORCounterStateEntry *ssEntry) {
	d9_printf4("Next=%x Head=%x Size=%d\n", ssEntry->pTransition, ssEntry->pXORState, ssEntry->nSize);
}

void PrintXORCounterStateEntry_dot(XORCounterStateEntry *ssEntry) {
	d9_printf4("Next=%x Head=%x Size=%d\n", ssEntry->pTransition, ssEntry->pXORState, ssEntry->nSize);
}

void PrintXORGElimStateEntry(XORGElimStateEntry *ssEntry) {
	d9_printf2("Parity=%d ", ((int *)ssEntry->pVector)[0]&1);
	d9_printf1("Vars=");
	for(int x = 0; x < ssEntry->nSize; x++)
	  d9_printf2("%d ", ssEntry->pnTransitionVars[x]);
	d9_printf2("Size=%d ", ssEntry->nSize);
	d9_printf1("Vector= ");
	PrintXORGElimVector(ssEntry->pVector);
	d9_printf1("\n");
}

void PrintXORGElimStateEntry_dot(XORGElimStateEntry *ssEntry) {
	d9_printf2("Parity=%d ", ((int *)ssEntry->pVector)[0]&1);
	d9_printf1("Vars=");
	for(int x = 0; x < ssEntry->nSize; x++)
	  d9_printf2("%d ", ssEntry->pnTransitionVars[x]);
	d9_printf2("Size=%d ", ssEntry->nSize);
	d9_printf1("Vector= ");
	PrintXORGElimVector(ssEntry->pVector);
	d9_printf1("\n");
}

void PrintXORStateEntry_formatted(XORStateEntry *ssEntry) {
	d2_printf2("equ(%c ", ssEntry->bParity?'T':'F');
	d2_printf1("xor(");
	for(int x = 0; x < ssEntry->nSize; x++) {
		if(SimpleSmurfProblemState->arrInferenceDeclaredAtLevel[ssEntry->pnTransitionVars[x]] == SimpleSmurfProblemState->nNumVars)
		  d2_printf2("%d ", ssEntry->pnTransitionVars[x]);
	}
	d2_printf1("))\n");
}

void FreeXORStateEntry(XORStateEntry *ssEntry) {
	ite_free((void **)&ssEntry->pnTransitionVars);	
}

void FreeXORCounterStateEntry(XORCounterStateEntry *ssEntry) {
	
}

void FreeXORGElimStateEntry(XORGElimStateEntry *ssEntry) {
	ite_free((void **)&ssEntry->pnTransitionVars);
	ite_free((void **)&ssEntry->pVector);
}
