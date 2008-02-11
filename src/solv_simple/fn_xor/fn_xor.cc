#include "sbsat.h"
#include "sbsat_solver.h"
#include "solver.h"

void PrintXORStateEntry(XORStateEntry *ssEntry) {
	d9_printf2("Parity=%d ", ssEntry->bParity);
	d9_printf1("Vars=");
	for(int x = 0; x < ssEntry->nSize; x++)
	  d9_printf2("%d ", ssEntry->nTransitionVars[x]);
	d9_printf2("Size=%d\n", ssEntry->nSize);
}

void PrintXORCounterStateEntry(XORCounterStateEntry *ssEntry) {
	d9_printf4("Next=%x Head=%x Size=%d\n", ssEntry->pTransition, ssEntry->pXORState, ssEntry->nSize);
}
