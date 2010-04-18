#include "sbsat.h"
#include "sbsat_solver.h"
#include "solver.h"

//MINMAX State

void initMINMAXStateType() {
   arrStatesTypeSize[FN_MINMAX] = sizeof(MINMAXStateEntry);
   arrSetVisitedState[FN_MINMAX] = SetVisitedMINMAXState;
   arrApplyInferenceToState[FN_MINMAX] = ApplyInferenceToMINMAX;
   arrPrintStateEntry[FN_MINMAX] = PrintMINMAXStateEntry;
   arrPrintStateEntry_dot[FN_MINMAX] = NULL;
   arrFreeStateEntry[FN_MINMAX] = FreeMINMAXStateEntry;
   arrCalculateStateHeuristic[FN_MINMAX] = CalculateMINMAXLSGBHeuristic;
   arrSetStateHeuristicScore[FN_MINMAX] = LSGBMINMAXStateSetHeurScore;
   arrGetStateHeuristicScore[FN_MINMAX] = LSGBMINMAXStateGetHeurScore;
}

void PrintMINMAXStateEntry(void *pState) {
   MINMAXStateEntry *pMINMAXState = (MINMAXStateEntry *)pState;
	d9_printf2("MM %d <= ", pMINMAXState->nMin);
	for(int x = 0; x < pMINMAXState->nSize; x++)
	  d9_printf2("%d ", pMINMAXState->pnTransitionVars[x]);
	d9_printf2("<= %d ", pMINMAXState->nMax);
	d9_printf2("Size=%d\n", pMINMAXState->nSize);
}

void PrintMINMAXStateEntry_dot(void *pState) {
	return;
}

void FreeMINMAXStateEntry(void *pState) {
   MINMAXStateEntry *pMINMAXState = (MINMAXStateEntry *)pState;
	ite_free((void **)&pMINMAXState->pnTransitionVars);
   if(pMINMAXState->pMINMAXStateBDD != NULL) {
      pMINMAXState->pMINMAXStateBDD->pState = NULL;
   }
}

//MINMAX Counter State

void initMINMAXCounterStateType() {
   arrStatesTypeSize[FN_MINMAX_COUNTER] = sizeof(MINMAXCounterStateEntry);
   arrSetVisitedState[FN_MINMAX_COUNTER] = SetVisitedMINMAXCounterState;
   arrApplyInferenceToState[FN_MINMAX_COUNTER] = ApplyInferenceToMINMAXCounter;
   arrPrintStateEntry[FN_MINMAX_COUNTER] = PrintMINMAXCounterStateEntry;
   arrPrintStateEntry_dot[FN_MINMAX_COUNTER] = PrintMINMAXCounterStateEntry_dot;
   arrFreeStateEntry[FN_MINMAX_COUNTER] = FreeMINMAXCounterStateEntry;
   arrCalculateStateHeuristic[FN_MINMAX_COUNTER] = CalculateMINMAXCounterLSGBHeuristic;
   arrSetStateHeuristicScore[FN_MINMAX_COUNTER] = LSGBMINMAXCounterStateSetHeurScore;
   arrGetStateHeuristicScore[FN_MINMAX_COUNTER] = LSGBMINMAXCounterStateGetHeurScore;
}

void PrintMINMAXCounterStateEntry(void *pState) {
   MINMAXCounterStateEntry *pMINMAXCounterState = (MINMAXCounterStateEntry *)pState;
	d9_printf5("MC Next=%p Head=%p VarsLeft=%d NumTrue=%d\n", 
              (void *)pMINMAXCounterState->pTransition,
              (void *)pMINMAXCounterState->pMINMAXState, 
              pMINMAXCounterState->nVarsLeft, 
              pMINMAXCounterState->nNumTrue);
}

void PrintMINMAXCounterStateEntry_dot(void *pState) {
	MINMAXCounterStateEntry *pMINMAXCounterState = (MINMAXCounterStateEntry *)pState;
	MINMAXStateEntry *pMINMAXState = (MINMAXStateEntry *)pMINMAXCounterState->pMINMAXState;
	uint8_t print_simple = 0;

	if(print_simple) {
		if(pMINMAXCounterState->nVarsLeft == 1) {
			fprintf(stdout, " b%p->b%p [style=dashed,label=\" c1 : x1\\n c2 : -x1\\n c3\"]\n", (void *)pMINMAXCounterState, (void *)pTrueSimpleSmurfState);
		} else {
			fprintf(stdout, " b%p->b%p [style=solid,label=\" x : nT++\"]\n", (void *)pMINMAXCounterState, (void *)(pMINMAXCounterState->pTransition));
			fprintf(stdout, " b%p->b%p [style=dotted,label=\" -x\"]\n", (void *)pMINMAXCounterState, (void *)(pMINMAXCounterState->pTransition));
			fprintf(stdout, " b%p->b%p [style=dashed,label=\" c1 : x1..x%d\\n c2 : -x1..-x%d\\n c3\"]\n", (void *)pMINMAXCounterState, (void *)pTrueSimpleSmurfState,
					 pMINMAXCounterState->nVarsLeft, pMINMAXCounterState->nVarsLeft);
		}
	} else {
		if(pMINMAXCounterState->nVarsLeft == 1) {
			fprintf(stdout, " b%p->b%p [style=dashed,label=\" nT<%d & %d=(%d-nT) : x1\\n nT=%d : -x1\\n %d<=nT>=%d & %d<=(%d-nT)\"]\n", (void *)pMINMAXCounterState, (void *)pTrueSimpleSmurfState,
					  pMINMAXState->nMin, pMINMAXCounterState->nVarsLeft, pMINMAXState->nMin, pMINMAXState->nMax, pMINMAXState->nMin, pMINMAXState->nMax, pMINMAXCounterState->nVarsLeft, pMINMAXState->nMax);
		} else {
			fprintf(stdout, " b%p->b%p [style=solid,label=\" x : nT++\"]\n", (void *)pMINMAXCounterState, (void *)(pMINMAXCounterState->pTransition));
			fprintf(stdout, " b%p->b%p [style=dotted,label=\" -x\"]\n", (void *)pMINMAXCounterState, (void *)(pMINMAXCounterState->pTransition));
			fprintf(stdout, " b%p->b%p [style=dashed,label=\" nT<%d & %d=(%d-nT) : x1..x%d\\n nT=%d : -x1..-x%d\\n %d<=nT>=%d & %d<=(%d-nT)\"]\n", (void *)pMINMAXCounterState, (void *)pTrueSimpleSmurfState,
					  pMINMAXState->nMin, pMINMAXCounterState->nVarsLeft, pMINMAXState->nMin, pMINMAXCounterState->nVarsLeft, pMINMAXState->nMax, pMINMAXCounterState->nVarsLeft, pMINMAXState->nMin, pMINMAXState->nMax, pMINMAXCounterState->nVarsLeft, pMINMAXState->nMax);
		}
	}
	
	fprintf(stdout, " b%p [shape=\"ellipse\", label=\"MINMAX, m=%d, M=%d, n=%d\"]\n", (void *)pMINMAXCounterState, pMINMAXState->nMin, pMINMAXState->nMax, pMINMAXCounterState->nVarsLeft);
	arrPrintStateEntry_dot[(int)((TypeStateEntry *)(pMINMAXCounterState->pTransition))->cType](pMINMAXCounterState->pTransition);
}

void FreeMINMAXCounterStateEntry(void *pState) {
		
}
