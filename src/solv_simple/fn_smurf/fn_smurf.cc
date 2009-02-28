#include "sbsat.h"
#include "sbsat_solver.h"
#include "solver.h"

void initSmurfStateType() {
   arrStatesTypeSize[FN_SMURF] = sizeof(SmurfStateEntry);
   arrApplyInferenceToState[FN_SMURF] = ApplyInferenceToSmurf;
   arrPrintStateEntry[FN_SMURF] = PrintSmurfStateEntry;
   arrPrintStateEntry_dot[FN_SMURF] = PrintSmurfStateEntry_dot;
   arrFreeStateEntry[FN_SMURF] = FreeSmurfStateEntry;
   arrCalculateStateHeuristic[FN_SMURF] = NULL; //SEAN!!! Create this function
}

void PrintSmurfStateEntry(void *pState) {
   SmurfStateEntry *pSmurfState = (SmurfStateEntry *)pState;
	d9_printf9("SM Var=%d, v=T:%x, v=F:%x, TWght=%4.6f, FWght=%4.6f, NextGT=%x, NextLT=%x, Next=%x\n",
              pSmurfState->nTransitionVar, 
              (uintptr_t)pSmurfState->pVarIsTrueTransition,
              (uintptr_t)pSmurfState->pVarIsFalseTransition,
              pSmurfState->fHeurWghtofTrueTransition,
              pSmurfState->fHeurWghtofFalseTransition,
              (uintptr_t)pSmurfState->pNextVarInThisStateGT,
              (uintptr_t)pSmurfState->pNextVarInThisStateLT,
              (uintptr_t)pSmurfState->pNextVarInThisState);
}

void PrintSmurfStateEntry_dot(void *pState) {
	SmurfStateEntry *pSmurfState = (SmurfStateEntry *)pState;
   SmurfStateEntry *pTempState = pSmurfState;
	while(pTempState != NULL) {
		fprintf(stdout, " b%x->b%x [style=solid,fontname=\"Helvetica\",label=\"%s\"]\n", 
              (uintptr_t)pState, 
              (uintptr_t)pTempState->pVarIsTrueTransition, 
              s_name(arrSimpleSolver2IteVarMap[pTempState->nTransitionVar]));
		fprintf(stdout, " b%x->b%x [style=dotted,fontname=\"Helvetica\",label=\"-%s\"]\n",
              (uintptr_t)pState,
              (uintptr_t)pTempState->pVarIsFalseTransition,
              s_name(arrSimpleSolver2IteVarMap[pTempState->nTransitionVar]));
//		fprintf(stdout, "%x [
		pTempState = (SmurfStateEntry *)pTempState->pNextVarInThisState;
   }
}

void PrintInferenceStateEntry(void *pState) {
   InferenceStateEntry *pInferenceState = (InferenceStateEntry *)pState;
	d9_printf4("IN Var=%d, Inf=%x, Polarity=%d\n",
              pInferenceState->nTransitionVar,
              (uintptr_t)pInferenceState->pVarTransition,
              pInferenceState->bPolarity);
}

void PrintInferenceStateEntry_dot(void *pState) {
   InferenceStateEntry *pInferenceState = (InferenceStateEntry *)pState;
	if(pInferenceState->bPolarity)
	  fprintf(stdout, " b%x->b%x [style=solid,fontname=\"Helvetica\",label=\"%s\"]\n", 
             (uintptr_t)pInferenceState, 
             (uintptr_t)pInferenceState->pVarTransition, 
             s_name(arrSimpleSolver2IteVarMap[pInferenceState->nTransitionVar]));
	else
	  fprintf(stdout, " b%x->b%x [style=dotted,fontname=\"Helvetica\",label=\"-%s\"]\n", 
             (uintptr_t)pInferenceState, 
             (uintptr_t)pInferenceState->pVarTransition, 
             s_name(arrSimpleSolver2IteVarMap[pInferenceState->nTransitionVar]));
	fprintf(stdout, " b%x [shape=\"ellipse\", label=\"I\"]\n", (uintptr_t)pInferenceState);
}

void FreeSmurfStateEntry(void *pState) {

}

void FreeInferenceStateEntry(void *pState) {

}
