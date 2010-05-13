#include "sbsat.h"
#include "sbsat_solver.h"
#include "solver.h"

// Smurf State

void initSmurfStateType() {
   arrStatesTypeSize[FN_SMURF] = sizeof(SmurfStateEntry);
   arrSetVisitedState[FN_SMURF] = SetVisitedSmurfState;
   arrApplyInferenceToState[FN_SMURF] = ApplyInferenceToSmurf;
   arrPrintStateEntry[FN_SMURF] = PrintSmurfStateEntry;
// arrPrintStateEntry_dot[FN_SMURF] = PrintSmurfStateEntry_dot;
	arrPrintStateEntry_dot[FN_SMURF] = PrintSmurfStateEntry_inferences_compressed_dot;
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
	if(pState == (void *)pTrueSimpleSmurfState) return;
	SmurfStateEntry *pSmurfState = (SmurfStateEntry *)pState;
   SmurfStateEntry *pTempState = pSmurfState;
	while(pTempState != NULL) {
		fprintf(stdout, " b%p->b%p [style=solid,label=\"%s\"]\n", 
              (void *)pState, 
              (void *)pTempState->pVarIsTrueTransition, 
              s_name(arrSimpleSolver2IteVarMap[pTempState->nTransitionVar]));
		fprintf(stdout, " b%p->b%p [style=dotted,label=\"-%s\"]\n",
              (void *)pState,
              (void *)pTempState->pVarIsFalseTransition,
              s_name(arrSimpleSolver2IteVarMap[pTempState->nTransitionVar]));
//		fprintf(stdout, "%p [
		pTempState = (SmurfStateEntry *)pTempState->pNextVarInThisState;
   }
}

void PrintSmurfStateEntry_inferences_compressed_dot(void *pState) {
	if(pState == (void *)pTrueSimpleSmurfState) return;
	SmurfStateEntry *pSmurfState = (SmurfStateEntry *)pState;
   SmurfStateEntry *pTempState = pSmurfState;
	while(pTempState != NULL) {
		TypeStateEntry *pNextState = (TypeStateEntry *)pTempState->pVarIsTrueTransition;
		fprintf(stdout, " b%p->", (void *)pState);
		if(pNextState->cType == FN_INFERENCE) {
			pNextState = pGetNextSmurfStateFromInference((InferenceStateEntry *)pNextState);
			fprintf(stdout, "b%p [style=solid,label=\"%s :",
					  (void *)pNextState,
					  s_name(arrSimpleSolver2IteVarMap[pTempState->nTransitionVar]));
			PrintInferenceChain_dot((InferenceStateEntry *)pTempState->pVarIsTrueTransition);
			fprintf(stdout, " \"]\n");
		} else {
			fprintf(stdout, "b%p [style=solid,label=\"%s \"]\n",
					  (void *)pNextState,
					  s_name(arrSimpleSolver2IteVarMap[pTempState->nTransitionVar]));
		}

		pNextState = (TypeStateEntry *)pTempState->pVarIsFalseTransition;
		fprintf(stdout, " b%p->", (void *)pState);
		if(pNextState->cType == FN_INFERENCE) {
			pNextState = pGetNextSmurfStateFromInference((InferenceStateEntry *)pNextState);
			fprintf(stdout, "b%p [style=dotted,label=\"-%s :",
					  (void *)pNextState,
					  s_name(arrSimpleSolver2IteVarMap[pTempState->nTransitionVar]));
			PrintInferenceChain_dot((InferenceStateEntry *)pTempState->pVarIsFalseTransition);
			fprintf(stdout, " \"]\n");
		} else {
			fprintf(stdout, "b%p [style=dotted,label=\"-%s \"]\n",
					  (void *)pNextState,
					  s_name(arrSimpleSolver2IteVarMap[pTempState->nTransitionVar]));
		}
		pTempState = (SmurfStateEntry *)pTempState->pNextVarInThisState;
   }
}

void FreeSmurfStateEntry(void *pState) {
   SmurfStateEntry *pSmurfState = (SmurfStateEntry *)pState;
   if(pSmurfState->pSmurfBDD != NULL) {
      pSmurfState->pSmurfBDD->pState = NULL;
   }
}
