#include "sbsat.h"
#include "sbsat_solver.h"
#include "solver.h"

// Smurf State

void initSmurfStateType() {
   arrStatesTypeSize[FN_SMURF] = sizeof(SmurfStateEntry);
   arrApplyInferenceToState[FN_SMURF] = ApplyInferenceToSmurf;
   arrPrintStateEntry[FN_SMURF] = PrintSmurfStateEntry;
   arrPrintStateEntry_dot[FN_SMURF] = PrintSmurfStateEntry_dot;
   arrFreeStateEntry[FN_SMURF] = FreeSmurfStateEntry;
   arrCalculateStateHeuristic[FN_SMURF] = CalculateSmurfLSGBHeuristic;
   arrSetStateHeuristicScore[FN_SMURF] = LSGBSmurfSetHeurScore;
   arrGetStateHeuristicScore[FN_SMURF] = LSGBSumNodeWeights;
}

void PrintSmurfStateEntry(void *pState) {
   SmurfStateEntry *pSmurfState = (SmurfStateEntry *)pState;
	d9_printf9("SM Var=%d, v=T:%p, v=F:%p, TWght=%4.6f, FWght=%4.6f, NextGT=%p, NextLT=%p, Next=%p\n",
              pSmurfState->nTransitionVar, 
              (void *)pSmurfState->pVarIsTrueTransition,
              (void *)pSmurfState->pVarIsFalseTransition,
              pSmurfState->fHeurWghtofTrueTransition,
              pSmurfState->fHeurWghtofFalseTransition,
              (void *)pSmurfState->pNextVarInThisStateGT,
              (void *)pSmurfState->pNextVarInThisStateLT,
              (void *)pSmurfState->pNextVarInThisState);
}

void PrintSmurfStateEntry_dot(void *pState) {
	SmurfStateEntry *pSmurfState = (SmurfStateEntry *)pState;
   SmurfStateEntry *pTempState = pSmurfState;
	while(pTempState != NULL) {
		fprintf(stdout, " b%p->b%p [style=solid,fontname=\"Helvetica\",label=\"%s\"]\n", 
              (void *)pState, 
              (void *)pTempState->pVarIsTrueTransition, 
              s_name(arrSimpleSolver2IteVarMap[pTempState->nTransitionVar]));
		fprintf(stdout, " b%p->b%p [style=dotted,fontname=\"Helvetica\",label=\"-%s\"]\n",
              (void *)pState,
              (void *)pTempState->pVarIsFalseTransition,
              s_name(arrSimpleSolver2IteVarMap[pTempState->nTransitionVar]));
//		fprintf(stdout, "%p [
		pTempState = (SmurfStateEntry *)pTempState->pNextVarInThisState;
   }
}

void FreeSmurfStateEntry(void *pState) {

}

// Inference State

void initInferenceStateType() {
   arrStatesTypeSize[FN_INFERENCE] = sizeof(InferenceStateEntry);
   arrApplyInferenceToState[FN_INFERENCE] = NULL;//ApplyInferenceToInference; //SEAN!!! Might be good to have this
   arrPrintStateEntry[FN_INFERENCE] = PrintInferenceStateEntry;
   arrPrintStateEntry_dot[FN_INFERENCE] = PrintInferenceStateEntry_dot;
   arrFreeStateEntry[FN_INFERENCE] = FreeInferenceStateEntry;
   arrCalculateStateHeuristic[FN_INFERENCE] = NULL;
   arrSetStateHeuristicScore[FN_INFERENCE] = LSGBInferenceSetHeurScore;
   arrGetStateHeuristicScore[FN_INFERENCE] = LSGBInferenceGetHeurScore;
}

void PrintInferenceStateEntry(void *pState) {
   InferenceStateEntry *pInferenceState = (InferenceStateEntry *)pState;
	d9_printf4("IN Var=%d, Inf=%p, Polarity=%d\n",
              pInferenceState->nTransitionVar,
              (void *)pInferenceState->pVarTransition,
              pInferenceState->bPolarity);
}

void PrintInferenceStateEntry_dot(void *pState) {
   InferenceStateEntry *pInferenceState = (InferenceStateEntry *)pState;
	if(pInferenceState->bPolarity)
	  fprintf(stdout, " b%p->b%p [style=solid,fontname=\"Helvetica\",label=\"%s\"]\n", 
             (void *)pInferenceState, 
             (void *)pInferenceState->pVarTransition, 
             s_name(arrSimpleSolver2IteVarMap[pInferenceState->nTransitionVar]));
	else
	  fprintf(stdout, " b%p->b%p [style=dotted,fontname=\"Helvetica\",label=\"-%s\"]\n", 
             (void *)pInferenceState, 
             (void *)pInferenceState->pVarTransition, 
             s_name(arrSimpleSolver2IteVarMap[pInferenceState->nTransitionVar]));
	fprintf(stdout, " b%p [shape=\"ellipse\", label=\"I\"]\n", (void *)pInferenceState);
}

void FreeInferenceStateEntry(void *pState) {

}
