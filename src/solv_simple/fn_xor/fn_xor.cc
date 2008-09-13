#include "sbsat.h"
#include "sbsat_solver.h"
#include "solver.h"

void PrintXORStateEntry(XORStateEntry *ssEntry) {
	d9_printf2("XOR Parity=%d ", ssEntry->bParity);
	d9_printf1("Vars=");
	for(int x = 0; x < ssEntry->nSize; x++)
	  d9_printf2("%d ", ssEntry->pnTransitionVars[x]);
	d9_printf2("Size=%d\n", ssEntry->nSize);
}

void PrintXORStateEntry_dot(XORStateEntry *ssEntry) {
   fprintf(stdout, " b%x->b%x [style=solid, fontname=\"Helvetica\",label=\"n--\"]\n", ssEntry, ssEntry);
	fprintf(stdout, " b%x->i%x [style=solid, fontname=\"Helvetica\",label=\"If n==1\"]\n", ssEntry, ssEntry);
	fprintf(stdout, " i%x->b%x [style=solid, fontname=\"Helvetica\",label=\"v=p\"]\n", ssEntry, pTrueSimpleSmurfState);
	
	fprintf(stdout, " i%x [shape=\"ellipse\", label=\"I\"]\n", ssEntry);
	fprintf(stdout, " b%x [shape=\"ellipse\", label=\"XOR, n=%d\"]\n", ssEntry, ssEntry->nSize);
}

void PrintXORCounterStateEntry(XORCounterStateEntry *ssEntry) {
	d9_printf4("XOR Next=%x Head=%x Size=%d\n", ssEntry->pTransition, ssEntry->pXORState, ssEntry->nSize);
}

void PrintXORCounterStateEntry_dot(XORCounterStateEntry *ssEntry) {
   fprintf(stdout, " b%x->b%x [style=solid, fontname=\"Helvetica\",label=\"n--\"]\n", ssEntry, ssEntry);
	fprintf(stdout, " b%x->i%x [style=solid, fontname=\"Helvetica\",label=\"If n==1\"]\n", ssEntry, ssEntry);
	fprintf(stdout, " i%x->b%x [style=solid, fontname=\"Helvetica\",label=\"v=p\"]\n", ssEntry, pTrueSimpleSmurfState);
	
	fprintf(stdout, " i%x [shape=\"ellipse\", label=\"I\"]\n", ssEntry);
	fprintf(stdout, " b%x [shape=\"ellipse\", label=\"XOR, n=%d\"]\n", ssEntry, ssEntry->pXORState->nSize);
}

void PrintXORGElimStateEntry(XORGElimStateEntry *ssEntry) {
	d9_printf2("XOR Parity=%d ", ((int *)ssEntry->pVector)[0]&1);
	d9_printf1("Vars=");
	for(int x = 0; x < ssEntry->nSize; x++)
	  d9_printf2("%d ", ssEntry->pnTransitionVars[x]);
	d9_printf2("Size=%d ", ssEntry->nSize);
	d9_printf1("Vector= ");
	PrintXORGElimVector(ssEntry->pVector);
	d9_printf1("\n");
}

void PrintXORGElimStateEntry_dot(XORGElimStateEntry *ssEntry) {
   fprintf(stdout, " b%x->b%x [style=solid, fontname=\"Helvetica\",label=\"n--\"]\n", ssEntry, ssEntry);
	fprintf(stdout, " b%x->i%x [style=solid, fontname=\"Helvetica\",label=\"If n==1\"]\n", ssEntry, ssEntry);
	fprintf(stdout, " i%x->b%x [style=solid, fontname=\"Helvetica\",label=\"v=p\"]\n", ssEntry, pTrueSimpleSmurfState);
	
	fprintf(stdout, " i%x [shape=\"ellipse\", label=\"I\"]\n", ssEntry);
	fprintf(stdout, " b%x [shape=\"ellipse\", label=\"GE XOR, n=%d\"]\n", ssEntry, ssEntry->nSize);
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
