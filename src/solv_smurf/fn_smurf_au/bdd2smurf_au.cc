/* =========FOR INTERNAL USE ONLY. NO DISTRIBUTION PLEASE ========== */

/*********************************************************************
 Copyright 1999-2004, University of Cincinnati.  All rights reserved.
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
long gnTotalBytesForTransitionAus = 0;

ITE_INLINE TransitionAu *
CreateTransitionAu(SmurfAuState *pSmurfAuState, int i, int nSolverVble, int value)
{
   int nVble = arrSolver2IteVarMap[nSolverVble];

   // Compute transition that occurs when vble is set to true.
   BDDNodeStruct *pFuncEvaled = set_variable(pSmurfAuState->pFunc, nVble, value==BOOL_TRUE?1:0);
   SmurfAuState *pSmurfAuStateOfEvaled = BDD2SmurfAu(pFuncEvaled);
   assert(pSmurfAuStateOfEvaled != NULL);
   AddStateTransitionAu(pSmurfAuState, i, nSolverVble, value,
         pFuncEvaled, pSmurfAuStateOfEvaled);
   TransitionAu *transition = FindTransitionAu(pSmurfAuState, i, nSolverVble, value);
   assert(transition->pNextState != NULL);
   return transition;
}

ITE_INLINE SmurfAuState *
ComputeSmurfAuOfNormalized(BDDNodeStruct *pFunc)
   // Precondition:  *pFunc is 'normalized', i.e., no literal
   // is logically implied by *pFunc.
   // Creates SmurfAu states to represent the function and its children.
   // Also computes the heuristic value of the SmurfAu state and its transitions,
   // based on the "JohnsonVillage" heuristic.
{
   assert(pFunc != false_ptr);

   ite_counters[SMURF_NODE_FIND]++;

   // Collapse 'true' SmurfAu states.
   if (pFunc == true_ptr) return pTrueSmurfAuState;

   if (pFunc->pState) return (SmurfAuState*)(pFunc->pState);

   SmurfAuState *pSmurfAuState = AllocateSmurfAuState();

   ite_counters[SMURF_NODE_NEW]++;

   pSmurfAuState->pFunc = pFunc;
   pFunc->pState = (void*)pSmurfAuState;

   // get all the variables
   long tempint_max = 0;
   long y=0;
   unravelBDD(&y, &tempint_max, &pSmurfAuState->vbles.arrElts, pFunc);
   pSmurfAuState->vbles.nNumElts = y;
   pSmurfAuState->vbles.arrElts = (int*)realloc(pSmurfAuState->vbles.arrElts, pSmurfAuState->vbles.nNumElts*sizeof(int));

   /* mapping ite->solver variables */
   for (int i=0;i<pSmurfAuState->vbles.nNumElts;i++) {
      if (pSmurfAuState->vbles.arrElts[i]==0 || 
            arrIte2SolverVarMap[pSmurfAuState->vbles.arrElts[i]]==0) {
         dE_printf1("\nassigned variable in a BDD in the solver");
         dE_printf3("\nvariable id: %d, true_false=%d\n", 
               pSmurfAuState->vbles.arrElts[i],
               variablelist[pSmurfAuState->vbles.arrElts[i]].true_false);
         //exit(1);
      }
      pSmurfAuState->vbles.arrElts[i] = arrIte2SolverVarMap[pSmurfAuState->vbles.arrElts[i]];
   }
   qsort(pSmurfAuState->vbles.arrElts, pSmurfAuState->vbles.nNumElts, sizeof(int), revcompfunc);

   /* allocate transitions */
   int nBytesForTransitionAus = pSmurfAuState->vbles.nNumElts * 2 * sizeof (TransitionAu);
   pSmurfAuState->arrTransitionAus = (TransitionAu *)ite_calloc(pSmurfAuState->vbles.nNumElts * 2, sizeof (TransitionAu),
         9, "pSmurfAuState->arrTransitionAus");

   gnTotalBytesForTransitionAus += nBytesForTransitionAus;

   //if (nHeuristic == JOHNSON_HEURISTIC) 
   {
      for(int i=0;i<pSmurfAuState->vbles.nNumElts;i++)
      {
         int nSolverVble = pSmurfAuState->vbles.arrElts[i];

         // Compute transition that occurs when vble is set to true.
         CreateTransitionAu(pSmurfAuState, i, nSolverVble, BOOL_TRUE);
         // Compute transition that occurs when vble is set to false.
         CreateTransitionAu(pSmurfAuState, i, nSolverVble, BOOL_FALSE);
      }
   }

   return pSmurfAuState;
}

ITE_INLINE
SmurfAuState *
BDD2SmurfAu(BDDNodeStruct *pFunc)
   // Constructs a SmurfAu representation for the constraint *pFunc.
   // Returns 0 if constraint pointed to by pFunc is unsatisfiable
   // or if the constraint is represented by a special function.
   // Otherwise, returns a pointer to the initial SmurfAu state.
{
   // Non-special function -- represent with regular state machine.
   BDDNodeStruct *pReduct = set_variable_all_infs(pFunc);

   return ComputeSmurfAuOfNormalized(pReduct);
}

int SmurfAuCreateFunction(int nFnId, BDDNode *bdd, int nFnType, int eqVble)
{
   arrSolverFunctions[nFnId].nFnId = nFnId;
   arrSolverFunctions[nFnId].nType = nFnType;
   arrSolverFunctions[nFnId].fn_smurf_au.nSmurfAuEqualityVble = arrIte2SolverVarMap[abs(eqVble)];
   arrSolverFunctions[nFnId].fn_smurf_au.pInitialState = BDD2SmurfAu(bdd);
   arrSolverFunctions[nFnId].fn_smurf_au.pPrevState = arrSolverFunctions[nFnId].fn_smurf_au.pCurrentState = arrSolverFunctions[nFnId].fn_smurf_au.pInitialState;
   arrSolverFunctions[nFnId].fn_smurf_au.arrSmurfAuPath.literals = (int*)ite_calloc(arrSolverFunctions[nFnId].fn_smurf_au.pInitialState->vbles.nNumElts+1, sizeof(int),
               9, "arrSmurfAuPath[].literals");
   return 0;
}

void SmurfAuAffectedVarList(int nFnId, int **arr1, int *num1, int **arr2, int *num2)
{
   *num1 = arrSolverFunctions[nFnId].fn_smurf_au.pInitialState->vbles.nNumElts;
   *arr1 = arrSolverFunctions[nFnId].fn_smurf_au.pInitialState->vbles.arrElts;
   *num2 = 0;
}

void SmurfAuCreateAFS(int nFnId, int nVarId, int nAFSIndex)
{
   arrAFS[nVarId].arrOneAFS[nAFSIndex].nFnId = nFnId;
   arrAFS[nVarId].arrOneAFS[nAFSIndex].nType = 0; //FN_SMURF;
}

