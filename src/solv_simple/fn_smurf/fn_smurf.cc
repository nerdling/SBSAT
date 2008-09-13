#include "sbsat.h"
#include "sbsat_solver.h"
#include "solver.h"

void PrintSmurfStateEntry(SmurfStateEntry *ssEntry) {
	d9_printf9("SM Var=%d, v=T:%x, v=F:%x, TWght=%4.6f, FWght=%4.6f, NextGT=%x, NextLT=%x, Next=%x\n", ssEntry->nTransitionVar, ssEntry->pVarIsTrueTransition, ssEntry->pVarIsFalseTransition, ssEntry->fHeurWghtofTrueTransition, ssEntry->fHeurWghtofFalseTransition, ssEntry->pNextVarInThisStateGT, ssEntry->pNextVarInThisStateLT, ssEntry->pNextVarInThisState);
}

void PrintSmurfStateEntry_dot(SmurfStateEntry *ssEntry) {
	SmurfStateEntry *pState = ssEntry;

/*	fprintf(stdout, " { rank=same;\n");
	while(pState != NULL) {
		if(pState->pVarIsTrueTransition != pTrueSimpleSmurfState)
		  fprintf(stdout, "   b%x\n", pState->pVarIsTrueTransition);
		if(pState->pVarIsFalseTransition != pTrueSimpleSmurfState)
		  fprintf(stdout, "   b%x\n", pState->pVarIsFalseTransition);
//		fprintf(stdout, "%x [
		pState = (SmurfStateEntry *)pState->pNextVarInThisState;
   }
	fprintf(stdout, " }\n");
*/	
	pState = ssEntry;
	while(pState != NULL) {
		fprintf(stdout, " b%x->b%x [style=solid,fontname=\"Helvetica\",label=\"%s\"]\n", ssEntry, pState->pVarIsTrueTransition, s_name(arrSimpleSolver2IteVarMap[pState->nTransitionVar]));
		fprintf(stdout, " b%x->b%x [style=dotted,fontname=\"Helvetica\",label=\"-%s\"]\n", ssEntry, pState->pVarIsFalseTransition, s_name(arrSimpleSolver2IteVarMap[pState->nTransitionVar]));
//		fprintf(stdout, "%x [
		pState = (SmurfStateEntry *)pState->pNextVarInThisState;
   }
}

void PrintInferenceStateEntry(InferenceStateEntry *ssEntry) {
	d9_printf4("IN Var=%d, Inf=%x, Polarity=%d\n", ssEntry->nTransitionVar, ssEntry->pVarTransition, ssEntry->bPolarity);
}

void PrintInferenceStateEntry_dot(InferenceStateEntry *ssEntry) {
	if(ssEntry->bPolarity)
	  fprintf(stdout, " b%x->b%x [style=solid,fontname=\"Helvetica\",label=\"%s\"]\n", ssEntry, ssEntry->pVarTransition, s_name(arrSimpleSolver2IteVarMap[ssEntry->nTransitionVar]));
	else
	  fprintf(stdout, " b%x->b%x [style=dotted,fontname=\"Helvetica\",label=\"-%s\"]\n", ssEntry, ssEntry->pVarTransition, s_name(arrSimpleSolver2IteVarMap[ssEntry->nTransitionVar]));
	fprintf(stdout, " b%x [shape=\"ellipse\", label=\"I\"]\n", ssEntry);
}

void PrintWatchedListStateEntry(WatchedListStateEntry *ssEntry) {
   d9_printf2("WL Next=%x ", ssEntry->pTransition);
	d9_printf1("Vars=");
	for(int x = 0; x < ssEntry->nWatchedListSize; x++)
	  d9_printf2("%d ", ssEntry->pnWatchedList[x]);
	d9_printf2("Size=%d\n", ssEntry->nWatchedListSize);
}

void PrintWatchedListStateEntry_dot(WatchedListStateEntry *ssEntry) {

}

void FreeSmurfStateEntry(SmurfStateEntry *ssEntry) {

}

void FreeInferenceStateEntry(InferenceStateEntry *ssEntry) {

}

void FreeWatchedListStateEntry(WatchedListStateEntry *ssEntry) {
	ite_free((void **)&ssEntry->pnWatchedList);

}
