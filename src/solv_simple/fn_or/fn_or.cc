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

void PrintORStateEntry(ORStateEntry *ssEntry) {
	d9_printf1("OR Vars=");
	for(int x = 0; x < ssEntry->nSize; x++)
	  d9_printf2("%d ", ssEntry->bPolarity[x]==1?ssEntry->pnTransitionVars[x]:-ssEntry->pnTransitionVars[x]);
	d9_printf2("Size=%d\n", ssEntry->nSize);
}

void PrintORStateEntry_dot(ORStateEntry *ssEntry) {
	fprintf(stdout, " b%x->b%x [style=solid, fontname=\"Helvetica\",label=\"If v!=p, n--\"]\n", ssEntry, ssEntry);
	fprintf(stdout, " b%x->b%x [style=solid, fontname=\"Helvetica\",label=\"If v==p\"]\n", ssEntry, pTrueSimpleSmurfState);
	fprintf(stdout, " b%x->i%x [style=solid, fontname=\"Helvetica\",label=\"If n==1\"]\n", ssEntry, ssEntry);
	fprintf(stdout, " i%x->b%x [style=solid, fontname=\"Helvetica\",label=\"v=p\"]\n", ssEntry, pTrueSimpleSmurfState);

	fprintf(stdout, " i%x [shape=\"ellipse\", label=\"I\"]\n", ssEntry);
	fprintf(stdout, " b%x [shape=\"ellipse\", label=\"OR, n=%d\"]\n", ssEntry, ssEntry->nSize);
}

void PrintORCounterStateEntry(ORCounterStateEntry *ssEntry) {
	d9_printf4("OR Next=%x Head=%x Size=%d\n", ssEntry->pTransition, ssEntry->pORState, ssEntry->nSize);
}

void PrintORCounterStateEntry_dot(ORCounterStateEntry *ssEntry) {
	fprintf(stdout, " b%x->b%x [style=solid, fontname=\"Helvetica\",label=\"If v!=p, n--\"]\n", ssEntry, ssEntry);
	fprintf(stdout, " b%x->b%x [style=solid, fontname=\"Helvetica\",label=\"If v==p\"]\n", ssEntry, pTrueSimpleSmurfState);
	fprintf(stdout, " b%x->i%x [style=solid, fontname=\"Helvetica\",label=\"If n==1\"]\n", ssEntry, ssEntry);
	fprintf(stdout, " i%x->b%x [style=solid, fontname=\"Helvetica\",label=\"v=p\"]\n", ssEntry, pTrueSimpleSmurfState);

	fprintf(stdout, " i%x [shape=\"ellipse\", label=\"I\"]\n", ssEntry);
	fprintf(stdout, " b%x [shape=\"ellipse\", label=\"OR, n=%d\"]\n", ssEntry, ssEntry->pORState->nSize);
}

void FreeORStateEntry(ORStateEntry *ssEntry) {
	ite_free((void **)&ssEntry->bPolarity);
	ite_free((void **)&ssEntry->pnTransitionVars);
}

void FreeORCounterStateEntry(ORCounterStateEntry *ssEntry){
	
}
