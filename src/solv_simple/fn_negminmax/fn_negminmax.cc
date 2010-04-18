#include "sbsat.h"
#include "sbsat_solver.h"
#include "solver.h"

//NEGMINMAX State

void initNEGMINMAXStateType() {
   arrStatesTypeSize[FN_NEGMINMAX] = sizeof(NEGMINMAXStateEntry);
   arrSetVisitedState[FN_NEGMINMAX] = SetVisitedNEGMINMAXState;
   arrApplyInferenceToState[FN_NEGMINMAX] = ApplyInferenceToNEGMINMAX;
   arrPrintStateEntry[FN_NEGMINMAX] = PrintNEGMINMAXStateEntry;
	arrPrintStateEntry_dot[FN_NEGMINMAX] = PrintNEGMINMAXStateEntry_dot;
   arrFreeStateEntry[FN_NEGMINMAX] = FreeNEGMINMAXStateEntry;
   arrCalculateStateHeuristic[FN_NEGMINMAX] = CalculateNEGMINMAXLSGBHeuristic;
   arrSetStateHeuristicScore[FN_NEGMINMAX] = LSGBNEGMINMAXStateSetHeurScore;
   arrGetStateHeuristicScore[FN_NEGMINMAX] = LSGBNEGMINMAXStateGetHeurScore;
}

void PrintNEGMINMAXStateEntry(void *pState) {
   NEGMINMAXStateEntry *pNEGMINMAXState = (NEGMINMAXStateEntry *)pState;
	d9_printf2("NMM %d <= ", pNEGMINMAXState->nMin);
	for(int x = 0; x < pNEGMINMAXState->nSize; x++)
	  d9_printf2("%d ", pNEGMINMAXState->pnTransitionVars[x]);
	d9_printf2("<= %d ", pNEGMINMAXState->nMax);
	d9_printf2("Size=%d\n", pNEGMINMAXState->nSize);
}

void PrintNEGMINMAXStateEntry_dot(void *pState) {
	return;
}

void FreeNEGMINMAXStateEntry(void *pState) {
   NEGMINMAXStateEntry *pNEGMINMAXState = (NEGMINMAXStateEntry *)pState;
	ite_free((void **)&pNEGMINMAXState->pnTransitionVars);
   if(pNEGMINMAXState->pNEGMINMAXStateBDD != NULL) {
      pNEGMINMAXState->pNEGMINMAXStateBDD->pState = NULL;
   }           
}

//NEGMINMAX Counter State

void initNEGMINMAXCounterStateType() {
   arrStatesTypeSize[FN_NEGMINMAX_COUNTER] = sizeof(NEGMINMAXCounterStateEntry);
   arrSetVisitedState[FN_NEGMINMAX_COUNTER] = SetVisitedNEGMINMAXCounterState;
   arrApplyInferenceToState[FN_NEGMINMAX_COUNTER] = ApplyInferenceToNEGMINMAXCounter;
   arrPrintStateEntry[FN_NEGMINMAX_COUNTER] = PrintNEGMINMAXCounterStateEntry;
   arrPrintStateEntry_dot[FN_NEGMINMAX_COUNTER] = PrintNEGMINMAXCounterStateEntry_dot;
   arrFreeStateEntry[FN_NEGMINMAX_COUNTER] = FreeNEGMINMAXCounterStateEntry;
   arrCalculateStateHeuristic[FN_NEGMINMAX_COUNTER] = CalculateNEGMINMAXCounterLSGBHeuristic;
   arrSetStateHeuristicScore[FN_NEGMINMAX_COUNTER] = LSGBNEGMINMAXCounterStateSetHeurScore;
   arrGetStateHeuristicScore[FN_NEGMINMAX_COUNTER] = LSGBNEGMINMAXCounterStateGetHeurScore;
}

void PrintNEGMINMAXCounterStateEntry(void *pState) {
   NEGMINMAXCounterStateEntry *pNEGMINMAXCounterState = (NEGMINMAXCounterStateEntry *)pState;
	d9_printf5("NMC Next=%p Head=%p VarsLeft=%d NumTrue=%d\n", 
              (void *)pNEGMINMAXCounterState->pTransition,
              (void *)pNEGMINMAXCounterState->pNEGMINMAXState,
              pNEGMINMAXCounterState->nVarsLeft,
              pNEGMINMAXCounterState->nNumTrue);
}

void PrintNEGMINMAXCounterStateEntry_dot(void *pState) {
	NEGMINMAXCounterStateEntry *pNEGMINMAXCounterState = (NEGMINMAXCounterStateEntry *)pState;
	NEGMINMAXStateEntry *pNEGMINMAXState = (NEGMINMAXStateEntry *)pNEGMINMAXCounterState->pNEGMINMAXState;
	uint8_t print_simple = 0;
	
	if(print_simple) {
		if(pNEGMINMAXCounterState->nVarsLeft == 1) {
			fprintf(stdout, " b%p->b%p [style=dashed,label=\" c1 : x1\\n c2: -x1\\n c3\\n c4\"]\n", (void *)pNEGMINMAXCounterState, (void *)pTrueSimpleSmurfState);
		} else {
			fprintf(stdout, " b%p->b%p [style=solid,label=\" x : nT++\"]\n", (void *)pNEGMINMAXCounterState, (void *)(pNEGMINMAXCounterState->pTransition));
			fprintf(stdout, " b%p->b%p [style=dotted,label=\" -x\"]\n", (void *)pNEGMINMAXCounterState, (void *)(pNEGMINMAXCounterState->pTransition));
			fprintf(stdout, " b%p->b%p [style=dashed,label=\" c1 : x1..x%d\\n c2 : -x1..-x%d\\n c3\\n c4\"]\n", (void *)pNEGMINMAXCounterState, (void *)pTrueSimpleSmurfState,
					  pNEGMINMAXCounterState->nVarsLeft, pNEGMINMAXCounterState->nVarsLeft);
		}
	} else {
		if(pNEGMINMAXCounterState->nVarsLeft == 1) {
			fprintf(stdout, " b%p->b%p [style=dashed,label=\" nT>=%d & %d=(%d-nT) : x1\\n nT=%d & %d<=(%d-nT) : -x1\\n nT>%d\\n %d>(nT+%d)\"]\n", (void *)pNEGMINMAXCounterState, (void *)pTrueSimpleSmurfState,
					  pNEGMINMAXState->nMin, pNEGMINMAXCounterState->nVarsLeft, pNEGMINMAXState->nMax+1, pNEGMINMAXState->nMin-1, pNEGMINMAXCounterState->nVarsLeft, pNEGMINMAXState->nMax,
					  pNEGMINMAXState->nMax, pNEGMINMAXState->nMin, pNEGMINMAXCounterState->nVarsLeft);
		} else {
			fprintf(stdout, " b%p->b%p [style=solid,label=\" x : nT++\"]\n", (void *)pNEGMINMAXCounterState, (void *)(pNEGMINMAXCounterState->pTransition));
			fprintf(stdout, " b%p->b%p [style=dotted,label=\" -x\"]\n", (void *)pNEGMINMAXCounterState, (void *)(pNEGMINMAXCounterState->pTransition));
			fprintf(stdout, " b%p->b%p [style=dashed,label=\" nT>=%d & %d=(%d-nT) : x1..x%d\\n nT=%d & %d=(%d-nT) : -x1..-x%d\\n nT>%d\\n %d>(nT+%d)\"]\n", (void *)pNEGMINMAXCounterState, (void *)pTrueSimpleSmurfState,
					  pNEGMINMAXState->nMin, pNEGMINMAXCounterState->nVarsLeft, pNEGMINMAXState->nMax+1, pNEGMINMAXCounterState->nVarsLeft,
					  pNEGMINMAXState->nMin-1, pNEGMINMAXCounterState->nVarsLeft, pNEGMINMAXState->nMax, pNEGMINMAXCounterState->nVarsLeft,
					  pNEGMINMAXState->nMax, pNEGMINMAXState->nMin, pNEGMINMAXCounterState->nVarsLeft);
		}
	}
	
	fprintf(stdout, " b%p [shape=\"ellipse\", label=\"NEGMINMAX, m=%d, M=%d, n=%d\"]\n", (void *)pNEGMINMAXCounterState, pNEGMINMAXState->nMin, pNEGMINMAXState->nMax, pNEGMINMAXCounterState->nVarsLeft);
	arrPrintStateEntry_dot[(int)((TypeStateEntry *)(pNEGMINMAXCounterState->pTransition))->cType](pNEGMINMAXCounterState->pTransition);
}

void FreeNEGMINMAXCounterStateEntry(void *pState) {
		
}
