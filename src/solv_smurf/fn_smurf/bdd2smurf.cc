/* =========FOR INTERNAL USE ONLY. NO DISTRIBUTION PLEASE ========== */

/*********************************************************************
 Copyright 1999-2007, University of Cincinnati.  All rights reserved.
 By using this software the USER indicates that he or she has read,
 understood and will comply with the following:

 --- University of Cincinnati hereby grants USER nonexclusive permission
 to use, copy and/or modify this software for internal, noncommercial,
 research purposes only. Any distribution, including commercial sale
 or license, of this software, copies of the software, its associated
 documentation and/or modifications of either is strictly prohibited
 without the prior consent of University of Cincinnati.  Title to copyright
 to this software and its associated documentation shall at all times
 remain with University of Cincinnati.  Appropriate copyright notice shall
 be placed on all software copies, and a complete copy of this notice
 shall be included in all copies of the associated documentation.
 No right is  granted to use in advertising, publicity or otherwise
 any trademark,  service mark, or the name of University of Cincinnati.


 --- This software and any associated documentation is provided "as is"

 UNIVERSITY OF CINCINNATI MAKES NO REPRESENTATIONS OR WARRANTIES, EXPRESS
 OR IMPLIED, INCLUDING THOSE OF MERCHANTABILITY OR FITNESS FOR A
 PARTICULAR PURPOSE, OR THAT  USE OF THE SOFTWARE, MODIFICATIONS, OR
 ASSOCIATED DOCUMENTATION WILL NOT INFRINGE ANY PATENTS, COPYRIGHTS,
 TRADEMARKS OR OTHER INTELLECTUAL PROPERTY RIGHTS OF A THIRD PARTY.

 University of Cincinnati shall not be liable under any circumstances for
 any direct, indirect, special, incidental, or consequential damages
 with respect to any claim by USER or any third party on account of
 or arising from the use, or inability to use, this software or its
 associated documentation, even if University of Cincinnati has been advised
 of the possibility of those damages.
*********************************************************************/

#include "sbsat.h"
#include "sbsat_solver.h"
#include "solver.h"

// External variables.
long gnTotalBytesForTransitions = 0;

ITE_INLINE Transition *
CreateTransition(SmurfState *pSmurfState, int i, int nSolverVble, int value)
{
   int nVble = arrSolver2IteVarMap[nSolverVble];

   // Compute transition that occurs when vble is set to true.
   BDDNodeStruct *pFuncEvaled = set_variable(pSmurfState->pFunc, nVble, value==BOOL_TRUE?1:0);
   SmurfState *pSmurfStateOfEvaled = BDD2Smurf(pFuncEvaled);
   assert(pSmurfStateOfEvaled != NULL);
   AddStateTransition(pSmurfState, i, nSolverVble, value,
         pFuncEvaled, pSmurfStateOfEvaled);
   Transition *transition = FindTransition(pSmurfState, i, nSolverVble, value);
   assert(transition->pNextState != NULL);
   return transition;
}

ITE_INLINE SmurfState *
ComputeSmurfOfNormalized(BDDNodeStruct *pFunc)
   // Precondition:  *pFunc is 'normalized', i.e., no literal
   // is logically implied by *pFunc.
   // Creates Smurf states to represent the function and its children.
   // Also computes the heuristic value of the Smurf state and its transitions,
   // based on the "JohnsonVillage" heuristic.
{
   assert(pFunc != false_ptr);

   ite_counters[SMURF_NODE_FIND]++;

   // Collapse 'true' Smurf states.
   if (pFunc == true_ptr) return pTrueSmurfState;

   if (pFunc->pState) return (SmurfState*)(pFunc->pState);

   SmurfState *pSmurfState = AllocateSmurfState();

   ite_counters[SMURF_NODE_NEW]++;

   pSmurfState->pFunc = pFunc;
   pFunc->pState = (void*)pSmurfState;

   // get all the variables
   int tempint_max = 0;
   int y=0;
   unravelBDD(&y, &tempint_max, &pSmurfState->vbles.arrElts, pFunc);
   pSmurfState->vbles.nNumElts = y;
   pSmurfState->vbles.arrElts = (int*)realloc(pSmurfState->vbles.arrElts, pSmurfState->vbles.nNumElts*sizeof(int));

   /* mapping ite->solver variables */
   for (int i=0;i<pSmurfState->vbles.nNumElts;i++) {
      if (pSmurfState->vbles.arrElts[i]==0 || 
            arrIte2SolverVarMap[pSmurfState->vbles.arrElts[i]]==0) {
         dE_printf1("\nassigned variable in a BDD in the solver");
         dE_printf3("\nvariable id: %d, true_false=%d\n", 
               pSmurfState->vbles.arrElts[i],
               variablelist[pSmurfState->vbles.arrElts[i]].true_false);
         //exit(1);
      }
      pSmurfState->vbles.arrElts[i] = arrIte2SolverVarMap[pSmurfState->vbles.arrElts[i]];
   }
   qsort(pSmurfState->vbles.arrElts, pSmurfState->vbles.nNumElts, sizeof(int), revcompfunc);

   /* allocate transitions */
   int nBytesForTransitions = pSmurfState->vbles.nNumElts * 2 * sizeof (Transition);
   pSmurfState->arrTransitions = (Transition *)ite_calloc(pSmurfState->vbles.nNumElts * 2, sizeof (Transition),
         9, "pSmurfState->arrTransitions");

   gnTotalBytesForTransitions += nBytesForTransitions;

   //if (nHeuristic == JOHNSON_HEURISTIC) 
   {
      for(int i=0;i<pSmurfState->vbles.nNumElts;i++)
      {
         int nSolverVble = pSmurfState->vbles.arrElts[i];

         // Compute transition that occurs when vble is set to true.
         CreateTransition(pSmurfState, i, nSolverVble, BOOL_TRUE);
         // Compute transition that occurs when vble is set to false.
         CreateTransition(pSmurfState, i, nSolverVble, BOOL_FALSE);
      }
   }

   return pSmurfState;
}

ITE_INLINE
SmurfState *
BDD2Smurf(BDDNodeStruct *pFunc)
   // Constructs a Smurf representation for the constraint *pFunc.
   // Returns 0 if constraint pointed to by pFunc is unsatisfiable
   // or if the constraint is represented by a special function.
   // Otherwise, returns a pointer to the initial Smurf state.
{
   // Non-special function -- represent with regular state machine.
   BDDNodeStruct *pReduct = set_variable_all_infs(pFunc);

   return ComputeSmurfOfNormalized(pReduct);
}

int SmurfCreateFunction(int nFnId, BDDNode *bdd, int nFnType, int eqVble)
{
   arrSolverFunctions[nFnId].nFnId = nFnId;
   arrSolverFunctions[nFnId].nType = nFnType;
   arrSolverFunctions[nFnId].fn_smurf.nSmurfEqualityVble = arrIte2SolverVarMap[abs(eqVble)];
   arrSolverFunctions[nFnId].fn_smurf.pInitialState = BDD2Smurf(bdd);
   arrSolverFunctions[nFnId].fn_smurf.pPrevState = arrSolverFunctions[nFnId].fn_smurf.pCurrentState = arrSolverFunctions[nFnId].fn_smurf.pInitialState;
   arrSolverFunctions[nFnId].fn_smurf.arrSmurfPath.literals = (int*)ite_calloc(arrSolverFunctions[nFnId].fn_smurf.pInitialState->vbles.nNumElts+1, sizeof(int),
               9, "arrSmurfPath[].literals");
   return 0;
}

void SmurfAffectedVarList(int nFnId, int **arr1, int *num1, int **arr2, int *num2)
{
   *num1 = arrSolverFunctions[nFnId].fn_smurf.pInitialState->vbles.nNumElts;
   *arr1 = arrSolverFunctions[nFnId].fn_smurf.pInitialState->vbles.arrElts;
   *num2 = 0;
}

void SmurfCreateAFS(int nFnId, int nVarId, int nAFSIndex)
{
   arrAFS[nVarId].arrOneAFS[nAFSIndex].nFnId = nFnId;
   arrAFS[nVarId].arrOneAFS[nAFSIndex].nType = 0; //FN_SMURF;
}

