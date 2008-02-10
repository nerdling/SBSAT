#include "sbsat.h"
#include "sbsat_solver.h"
#include "solver.h"

/********** Included in include/sbsat_solver.h *********************

struct TypeStateEntry {
   char cType;
}

struct ORStateEntry {
   char cType; //FN_OR
   int *nTransitionVars;
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
	d9_printf1("Vars=");
	for(int x = 0; x < ssEntry->nSize; x++)
	  d9_printf2("%d ", ssEntry->bPolarity[x]==1?ssEntry->nTransitionVars[x]:-ssEntry->nTransitionVars[x]);
	d9_printf2("Size=%d\n", ssEntry->nSize);
}

void PrintORCounterStateEntry(ORCounterStateEntry *ssEntry) {
	d9_printf4("Next=%x Head=%x Size=%d\n", ssEntry->pTransition, ssEntry->pORState, ssEntry->nSize);
}
