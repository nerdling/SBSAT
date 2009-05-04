#include "sbsat.h"
#include "sbsat_solver.h"
#include "solver.h"

void *CreateInferenceStates(BDDNode *infBDD) {
   void *pNextState = NULL;
   InferenceStateEntry *pNextInfState = NULL;
   infer *inferences = infBDD->inferences;
   int nNumInferences = 0;
   while(inferences!=NULL) {
      if(inferences->nums[1] != 0) {
         inferences = inferences->next; continue;
      }
      
      //Add a SmurfStateEntry into the table
      if(infBDD->pState != NULL) break;
      check_SmurfStatesTableSize(sizeof(InferenceStateEntry));
      ite_counters[SMURF_STATES]++;
      infBDD->pState = SimpleSmurfProblemState->pSmurfStatesTableTail;
      if(nNumInferences == 0) pNextState = infBDD->pState;
      else pNextInfState->pVarTransition = infBDD->pState;
      nNumInferences++;
      assert(infBDD->pState!=NULL);
      pNextInfState = (InferenceStateEntry *)SimpleSmurfProblemState->pSmurfStatesTableTail;
      SimpleSmurfProblemState->nNumSmurfStateEntries++;
      SimpleSmurfProblemState->pSmurfStatesTableTail = (void *)(pNextInfState + 1);
      pNextInfState->cType = FN_INFERENCE;
      pNextInfState->pInferenceBDD = infBDD;
      if(inferences->nums[0] > 0) {
         pNextInfState->nTransitionVar = arrIte2SimpleSolverVarMap[inferences->nums[0]];
         pNextInfState->bPolarity = 1;
         infBDD = set_variable(infBDD, arrSimpleSolver2IteVarMap[pNextInfState->nTransitionVar], 1);
      } else {
         pNextInfState->nTransitionVar = arrIte2SimpleSolverVarMap[-inferences->nums[0]];
         pNextInfState->bPolarity = 0;
         infBDD = set_variable(infBDD, arrSimpleSolver2IteVarMap[pNextInfState->nTransitionVar], 0);
      }
      inferences = infBDD->inferences;
   }
   
   if(infBDD->pState != NULL) {
      if(nNumInferences == 0) pNextState = infBDD->pState; //The transition is False
      else pNextInfState->pVarTransition = infBDD->pState;
   } else {
      //Recurse on nTransitionVar == False transition
      void *pNext = NULL;
      if(precompute_smurfs) {
         pNext = ReadSmurfStateIntoTable(infBDD, NULL, 0);
         assert(pNext!=NULL);
      }
      if(nNumInferences == 0) pNextState = pNext;
      else pNextInfState->pVarTransition = pNext;
   }
   assert(precompute_smurfs==0 || pNextState != NULL);
   return pNextState;
}

