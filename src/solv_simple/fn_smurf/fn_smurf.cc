#include "sbsat.h"
#include "sbsat_solver.h"
#include "solver.h"

void PrintSmurfStateEntry(SmurfStateEntry *ssEntry) {
	d9_printf9("Var=%d, v=T:%x, v=F:%x, TWght=%4.6f, FWght=%4.6f, NextGT=%x, NextLT=%x, Next=%x\n", ssEntry->nTransitionVar, ssEntry->pVarIsTrueTransition, ssEntry->pVarIsFalseTransition, ssEntry->fHeurWghtofTrueTransition, ssEntry->fHeurWghtofFalseTransition, ssEntry->pNextVarInThisStateGT, ssEntry->pNextVarInThisStateLT, ssEntry->pNextVarInThisState);
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
		fprintf(stdout, " b%x->b%x [style=solid,fontname=\"Helvetica\",fontsize=\"8\",label=\"%s\"]\n", ssEntry, pState->pVarIsTrueTransition, s_name(arrSimpleSolver2IteVarMap[pState->nTransitionVar]));
		fprintf(stdout, " b%x->b%x [style=dotted,fontname=\"Helvetica\",fontsize=\"8\",label=\"-%s\"]\n", ssEntry, pState->pVarIsFalseTransition, s_name(arrSimpleSolver2IteVarMap[pState->nTransitionVar]));
//		fprintf(stdout, "%x [
		pState = (SmurfStateEntry *)pState->pNextVarInThisState;
   }
}

void PrintInferenceStateEntry(InferenceStateEntry *ssEntry) {
	d9_printf4("Var=%d, Inf=%x, Polarity=%d\n", ssEntry->nTransitionVar, ssEntry->pVarTransition, ssEntry->bPolarity);
}

void PrintInferenceStateEntry_dot(InferenceStateEntry *ssEntry) {
	if(ssEntry->bPolarity)
	  fprintf(stdout, " b%x->b%x [style=solid,fontname=\"Helvetica\",fontsize=\"8\",label=\"%s\"]\n", ssEntry, ssEntry->pVarTransition, s_name(arrSimpleSolver2IteVarMap[ssEntry->nTransitionVar]));
	else
	  fprintf(stdout, " b%x->b%x [style=dotted,fontname=\"Helvetica\",fontsize=\"8\",label=\"-%s\"]\n", ssEntry, ssEntry->pVarTransition, s_name(arrSimpleSolver2IteVarMap[ssEntry->nTransitionVar]));
	fprintf(stdout, "b%x [shape=\"circle\", label=\"I\"]\n", ssEntry);
}

void FreeSmurfStateEntry(SmurfStateEntry *ssEntry) {

}

void FreeInferenceStateEntry(InferenceStateEntry *ssEntry) {

}
