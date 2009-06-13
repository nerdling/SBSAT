#include "sbsat.h"
#include "sbsat_solver.h"
#include "solver.h"

ITE_INLINE
void create_clause_from_TypeState(int nInfVar, TypeStateEntry *pState, int nNumVarsInSmurf,
                                  Cls **clause, int *lits_max_size) {
   Lit ** p;
   if((*clause)->used == 1) return; //Clause is already in the queue.
   
   if(nNumVarsInSmurf > (*lits_max_size)) {
      (*clause) = (Cls *)realloc((*clause), bytes_clause(nNumVarsInSmurf, 0));
      (*lits_max_size) = nNumVarsInSmurf;
   }
   
   d7_printf2("        Lemma: %d", nInfVar);
   
   p = (*clause)->lits;
   (*p++) = int2lit(nInfVar);
   int nNumVars = 1;
   while(pState != NULL) {
      d7_printf3(" %d(%p)", pState->nLemmaLiteral, pState); //Negate literals in the path.
      (*p++) = int2lit(pState->nLemmaLiteral);
      pState = (TypeStateEntry *)pState->pPreviousState;
      nNumVars++;
   }

   d7_printf1("\n");
   (*clause)->size = nNumVars;
}

ITE_INLINE
int TypeState_InferVar(TypeStateEntry *pState, int nInfVar, bool bPolarity, int nSmurfNumber, int INF_TYPE) {
   if(use_lemmas) {
      int nInfQueueHead = SimpleSmurfProblemState->arrSmurfStack[SimpleSmurfProblemState->nCurrSearchTreeLevel].nNumFreeVars;
      int nPrevInfLevel = abs(SimpleSmurfProblemState->arrInferenceDeclaredAtLevel[nInfVar]);
      if(nPrevInfLevel < nInfQueueHead) {
         if((SimpleSmurfProblemState->arrInferenceQueue[nPrevInfLevel] > 0) != bPolarity) {
            //Conflict
            //Add a lemma to reference this conflict.
            create_clause_from_TypeState(bPolarity?nInfVar:-nInfVar,
                                         pState,
                                         SimpleSmurfProblemState->arrReverseOccurenceList[nSmurfNumber][0].var,
                                         &(SimpleSmurfProblemState->pConflictClause.clause),
                                         &(SimpleSmurfProblemState->pConflictClause.max_size));
            int ret = EnqueueInference(nInfVar, bPolarity, INF_TYPE);
            assert(ret == 0);
            return 0;
         } else {
            //Lit already assigned.
            d7_printf2("      Inference %d already inferred\n", (bPolarity?nInfVar:-nInfVar));
            return 1;
         }
      } else {
         //Add a lemma as reference to this inference.
         create_clause_from_TypeState(bPolarity?nInfVar:-nInfVar,
                                      pState,
                                      SimpleSmurfProblemState->arrReverseOccurenceList[nSmurfNumber][0].var,
                                      &(SimpleSmurfProblemState->arrInferenceLemmas[nInfVar].clause),
                                      &(SimpleSmurfProblemState->arrInferenceLemmas[nInfVar].max_size));
         if(EnqueueInference(nInfVar, bPolarity, INF_TYPE) == 0) return 0;
      }
   } else {
      //No Lemmas
      if(EnqueueInference(nInfVar, bPolarity, INF_TYPE) == 0) return 0;
   }
   return 1;
}

ITE_INLINE
void SetVisitedInferenceState(void *pState, int value) {
   InferenceStateEntry *pInferenceState = (InferenceStateEntry *)pState;
   assert(pInferenceState->cType == FN_INFERENCE);
   
   InferenceStateEntry *pPreviousInference = NULL;
   while(pInferenceState!=NULL && (pInferenceState->cType == FN_INFERENCE) && pInferenceState->visited != value) {
      d7_printf3("Marking visited=%d of Inference State %p\n", value, pInferenceState);
      pInferenceState->visited = value;
      bdd_flag_nodes(pInferenceState->pInferenceBDD);
      pPreviousInference = pInferenceState;
      pInferenceState = (InferenceStateEntry *)(pInferenceState->pVarTransition);
   }
//Commented out to work w/ garbage collection compression (may result in slighly slower search.
   if(pInferenceState!=NULL) // && (TypeStateEntry *)pInferenceState->visited != value)
     pPreviousInference->pVarTransition = NULL;
}

ITE_INLINE
int TransitionInference(int nSmurfNumber, void **arrSmurfStates) {
   void *pNextState = arrSmurfStates[nSmurfNumber];
   assert(((TypeStateEntry *)pNextState)->cType == FN_INFERENCE);
   
   void *pPrevState = NULL;
   while(pNextState!=NULL && ((TypeStateEntry *)pNextState)->cType == FN_INFERENCE) {
      int nInfVar = ((InferenceStateEntry *)pNextState)->nTransitionVar;

      if(TypeState_InferVar(NULL,
                            nInfVar,
                            ((InferenceStateEntry *)pNextState)->bPolarity,
                            nSmurfNumber,
                            INF_SMURF) == 0) return 0;
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
             //pNextState = ((void *)((InferenceStateEntry *)pPrevState)->pVarTransition);
      assert(((TypeStateEntry *)pNextState)->cType==FN_FREE_STATE);
   }
   //Record the transition.
   if(((SmurfStateEntry *)pNextState) == pTrueSimpleSmurfState) {
      SimpleSmurfProblemState->arrSmurfStack[SimpleSmurfProblemState->nCurrSearchTreeLevel].nNumSmurfsSatisfied++;
   } else if(pNextState == arrSmurfStates[((TypeStateEntry *)pNextState)->pStateOwner]) {
      d7_printf3("      State %p currently owned by Smurf %d, transitioning to True\n", pNextState, ((TypeStateEntry *)pNextState)->pStateOwner);
      pNextState = pTrueSimpleSmurfState;
   } else {
      ((TypeStateEntry *)pNextState)->pPreviousState = arrSmurfStates[nSmurfNumber];
      ((TypeStateEntry *)pNextState)->pStateOwner = nSmurfNumber;
   }
   
   arrSmurfStates[nSmurfNumber] = pNextState;
   
   //This is slightly abusive, but needed to catch linear functions for the GE table
   int ret = ApplyInferenceToSmurf_Hooks(0, 1, nSmurfNumber, arrSmurfStates);
   
   d7_printf3("      Smurf %d transitioned to state %p\n", nSmurfNumber, arrSmurfStates[nSmurfNumber]);
   d7_printf3("      Smurf %d previously was %p\n", nSmurfNumber, ((SmurfStateEntry *)arrSmurfStates[nSmurfNumber])->pPreviousState);
   
   return ret;
}
