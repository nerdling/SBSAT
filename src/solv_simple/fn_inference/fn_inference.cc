#include "sbsat.h"
#include "sbsat_solver.h"
#include "solver.h"

// Inference State
void initInferenceStateType() {
   arrStatesTypeSize[FN_INFERENCE] = sizeof(InferenceStateEntry);
   arrSetVisitedState[FN_INFERENCE] = SetVisitedInferenceState;
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
   InferenceStateEntry *pInferenceState = (InferenceStateEntry *)pState;
   if(pInferenceState->pInferenceBDD!=NULL) {
      pInferenceState->pInferenceBDD->pState = NULL;
   }
}

