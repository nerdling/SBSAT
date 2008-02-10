#include "sbsat.h"
#include "sbsat_solver.h"
#include "solver.h"

void PrintSmurfStateEntry(SmurfStateEntry *ssEntry) {
	d9_printf9("Var=%d, v=T:%x, v=F:%x, TWght=%4.6f, FWght=%4.6f, NextGT=%x, NextLT=%x, Next=%x\n", ssEntry->nTransitionVar, ssEntry->pVarIsTrueTransition, ssEntry->pVarIsFalseTransition, ssEntry->fHeurWghtofTrueTransition, ssEntry->fHeurWghtofFalseTransition, ssEntry->pNextVarInThisStateGT, ssEntry->pNextVarInThisStateLT, ssEntry->pNextVarInThisState);
}

void PrintInferenceStateEntry(InferenceStateEntry *ssEntry) {
	d9_printf4("Var=%d, Inf=%x, Polarity=%d\n", ssEntry->nTransitionVar, ssEntry->pVarTransition, ssEntry->bPolarity);
}
