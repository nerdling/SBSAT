/* =========FOR INTERNAL USE ONLY. NO DISTRIBUTION PLEASE ========== */

/*********************************************************************
 Copyright 1999-2003, University of Cincinnati.  All rights reserved.
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

#include "ite.h"
#include "solver.h"

// External variables.
long gnTotalBytesForTransitions = 0;

ITE_INLINE
SmurfState *
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

   if (SFADDONS(pFunc->addons)->pState) {
      return SFADDONS(pFunc->addons)->pState;
   }

   SmurfState *pSmurfState = AllocateSmurfState();

   ite_counters[SMURF_NODE_NEW]++;

   pSmurfState->pFunc = pFunc;
   SFADDONS(pFunc->addons)->pState = pSmurfState;

   // get all the variables
   long tempint_max = 0;
   long y=0;
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

   int i=0;
   int nSolverVble, nVble;
   for (i=0;i<pSmurfState->vbles.nNumElts;i++)
   {

      nSolverVble = pSmurfState->vbles.arrElts[i];
      nVble = arrSolver2IteVarMap[nSolverVble];

      // Compute transition that occurs when vble is set to true.
      BDDNodeStruct *pFuncEvaled = set_variable(pFunc, nVble, 1);
      InitializeAddons(pFuncEvaled);
      assert(pFuncEvaled != pFunc);
      SmurfState *pSmurfStateOfEvaled = BDD2Smurf(pFuncEvaled);
      AddStateTransition(pSmurfState, i, nSolverVble, BOOL_TRUE,
            pFuncEvaled, pSmurfStateOfEvaled);

      // Compute transition that occurs when vble is set to false.
      pFuncEvaled = set_variable(pFunc, nVble, 0);
      InitializeAddons(pFuncEvaled);
      assert(pFuncEvaled != pFunc);
      pSmurfStateOfEvaled = BDD2Smurf(pFuncEvaled);

      AddStateTransition(pSmurfState, i, nSolverVble, BOOL_FALSE,
            pFuncEvaled, pSmurfStateOfEvaled);
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
   SFADDONS(pFunc->addons)->pReduct = set_variable_all_infs(pFunc);
   InitializeAddons(SFADDONS(pFunc->addons)->pReduct);

   return ComputeSmurfOfNormalized(SFADDONS(pFunc->addons)->pReduct);
}
