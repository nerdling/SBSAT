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

ITE_INLINE
TypeStateEntry *pGetNextSmurfStateFromInference(InferenceStateEntry *pInferenceState) {
	void *pNextState = pInferenceState;
	void *pPrevState = NULL;
	while(pNextState!=NULL && ((TypeStateEntry *)pNextState)->cType == FN_INFERENCE) {
		//Follow the transtion to the next SmurfState
		pPrevState = pNextState;
		pNextState = ((InferenceStateEntry *)pNextState)->pVarTransition;
	}

	if(pNextState == NULL) {
		assert(((TypeStateEntry *)pPrevState)->cType == FN_INFERENCE);
		((InferenceStateEntry *)pPrevState)->pVarTransition = pNextState = ReadSmurfStateIntoTable(
             set_variable(((InferenceStateEntry *)pPrevState)->pInferenceBDD,
				 arrSimpleSolver2IteVarMap[((InferenceStateEntry *)pPrevState)->nTransitionVar],
				 ((InferenceStateEntry *)pPrevState)->bPolarity),
				 NULL, 0);
		assert(((TypeStateEntry *)pNextState)->cType==FN_FREE_STATE);
	}

	return (TypeStateEntry *)pNextState;
}

ITE_INLINE
void PrintInferenceChain_dot(InferenceStateEntry *pInferenceState) {
	void *pNextState = pInferenceState;
	while(pNextState!=NULL && ((TypeStateEntry *)pNextState)->cType == FN_INFERENCE) {
		//Follow the transtion to the next SmurfState
		int nInfVar = ((InferenceStateEntry *)pNextState)->nTransitionVar;
		bool bInfPolarity = ((InferenceStateEntry *)pNextState)->bPolarity;
		//SEAN!!! Use commas???
		if(bInfPolarity) {
			fprintf(stdout, " %s", s_name(arrSimpleSolver2IteVarMap[nInfVar]));			
		} else {
			fprintf(stdout, " -%s", s_name(arrSimpleSolver2IteVarMap[nInfVar]));
		}
		pNextState = ((InferenceStateEntry *)pNextState)->pVarTransition;
	}
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
     fprintf(stdout, " b%p->b%p [style=dashed,label=\": %s\"]\n",
             (void *)pInferenceState,
             (void *)pInferenceState->pVarTransition,
             s_name(arrSimpleSolver2IteVarMap[pInferenceState->nTransitionVar]));
   else
     fprintf(stdout, " b%p->b%p [style=dashed,label=\": -%s\"]\n",
             (void *)pInferenceState,
             (void *)pInferenceState->pVarTransition,
             s_name(arrSimpleSolver2IteVarMap[pInferenceState->nTransitionVar]));
   fprintf(stdout, " b%p [shape=\"ellipse\",label=\"I\"]\n", (void *)pInferenceState);
}

void FreeInferenceStateEntry(void *pState) {
   InferenceStateEntry *pInferenceState = (InferenceStateEntry *)pState;
   if(pInferenceState->pInferenceBDD!=NULL) {
      pInferenceState->pInferenceBDD->pState = NULL;
   }
}
