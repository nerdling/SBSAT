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
extern int nRegSmurfIndex;

// Functions.
ITE_INLINE
SmurfState *
ComputeSmurfAndInferences(BDDNodeStruct *pFunc, PartialAssignmentEncoding encoding);

ITE_INLINE
SmurfState *
ComputeSmurfOfNormalized(BDDNodeStruct *pFunc, PartialAssignmentEncoding encoding)
   // Precondition:  *pFunc is 'normalized', i.e., no literal
   // is logically implied by *pFunc.
   // Creates Smurf states to represent the function and its children.
   // Also computes the heuristic value of the Smurf state and its transitions,
   // based on the "JohnsonVillage" heuristic.
{
   assert(pFunc != false_ptr);

   ite_counters[SMURF_NODE_FIND]++;

   // Collapse 'true' Smurf states.
   if (pFunc == true_ptr)
   {
      // 'True' Smurf state was initialized at the beginning of SmurfFactory().
      assert(pTrueSmurfState);
      return pTrueSmurfState;
   }

   if (compress_smurfs && pFunc->addons && pFunc->addons->pState) {
         return pFunc->addons->pState;
   }

   SmurfState *pSmurfState;
   // Collapse Smurf states iff
   // (1) They were created from the same constraint
   // and (2) the literal assignment (path) taken to reach the different states
   // are permutations of each other.  With respect to clause (2)
   // we do not count forced assignments as part of the path.
   if (encoding.FindOrAddState(&pSmurfState))
   { 
      /* state already present */
      return pSmurfState;
   }

   ite_counters[SMURF_NODE_NEW]++;

   pSmurfState->pFunc = pFunc;
   pFunc->addons->pState = pSmurfState;

   ComputeVbleSet(pFunc);

   pFunc->addons
      ->pVbles->StoreAsArrayBasedSet(pSmurfState->vbles, NULL);
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
   int nVble;
   for (i=0;i<pSmurfState->vbles.nNumElts;i++)
   {

      // pSmurfState->vbles.arrElts[i];
      nVble = arrSolver2IteVarMap[pSmurfState->vbles.arrElts[i]];
      //d9_printf2("Building Smurf State x with %d\n", nVble);

      // Compute transition that occurs when vble is set to true.
      BDDNodeStruct *pFuncEvaled = EvalBdd(pFunc, nVble, true);
      SmurfState *pSmurfStateOfEvaled
         = ComputeSmurfAndInferences(pFuncEvaled, encoding.AddLiteral(nVble, true));
      AddStateTransition(pSmurfState, i, arrIte2SolverVarMap[nVble], BOOL_TRUE,
            pFuncEvaled, pSmurfStateOfEvaled);

      // Compute transition that occurs when vble is set to false.
      pFuncEvaled = EvalBdd(pFunc, nVble, false);
      pSmurfStateOfEvaled 
         = ComputeSmurfAndInferences(pFuncEvaled, encoding.AddLiteral(nVble, false));

      AddStateTransition(pSmurfState, i, arrIte2SolverVarMap[nVble], BOOL_FALSE,
            pFuncEvaled, pSmurfStateOfEvaled);
   }

   return pSmurfState;
}

ITE_INLINE
void
ComputeImpliedLiterals(BDDNodeStruct *pFunc)
{
   if (pFunc->addons->pImplied)
   {
      // Inference set has already been computed.
      return;
   }

   ITE_NEW_CATCH(
         pFunc->addons->pImplied = new LiteralSet();,
         "pFunc->addons->pImplied");
   LiteralSet *pImplied = pFunc->addons->pImplied;
   if (pFunc == false_ptr)
   {
      pImplied->SetToInconsistent();
      return;
   }

   if (pFunc == true_ptr)
   {
      assert(pImplied->IsEmpty());
      return;
   }

   ComputeImpliedLiterals(pFunc->thenCase);
   ComputeImpliedLiterals(pFunc->elseCase);
   ComputeIntersection(*(pFunc->thenCase->addons->pImplied),
         *(pFunc->elseCase->addons->pImplied),
         *pImplied);
   if (pFunc->thenCase == false_ptr)
   {
      pImplied->PushElement(pFunc->variable, false);
   }
   else if (pFunc->elseCase == false_ptr)
   {
      pImplied->PushElement(pFunc->variable, true);
   }
}

ITE_INLINE
void
ComputeReduct(BDDNodeStruct *pFunc)
{ 
   if (pFunc->addons->pReduct) return;

   int nVble;
   bool bValueOfVble;
   BDDNodeStruct *pReduct = pFunc;

   LiteralSetIterator litsetNext(*(pFunc->addons->pImplied));
   while (litsetNext(nVble, bValueOfVble))
   {
      pReduct = EvalBdd(pReduct, nVble, bValueOfVble);
   }
   pFunc->addons->pReduct = pReduct;
}

ITE_INLINE
SmurfState *
ComputeSmurfAndInferences(BDDNodeStruct *pFunc, PartialAssignmentEncoding encoding)
{
   // Non-special function -- represent with regular state machine.
   ComputeImpliedLiterals(pFunc);
   ComputeReduct(pFunc);

   return ComputeSmurfOfNormalized(pFunc->addons->pReduct, encoding);
}

ITE_INLINE
SmurfState *
BDD2Smurf(BDDNodeStruct *pFunc)
   // Constructs a representation for the constraint *pFunc.
   // The representation may be as a regular Smurf or as a special function.
   // Returns 0 if constraint pointed to by pFunc is unsatisfiable
   // or if the constraint is represented by a special function.
   // Otherwise, returns a pointer to the initial Smurf state.
{

   /* start the recursion */
   PartialAssignmentEncoding encoding;

   ComputeVbleSet(pFunc); /* LOOK -- should it be here? */
   ComputeVariableMapping(*(pFunc->addons->pVbles));

   return
      ComputeSmurfAndInferences(pFunc, encoding);
}
