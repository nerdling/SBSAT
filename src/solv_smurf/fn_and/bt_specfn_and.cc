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

ITE_INLINE int
UpdateSpecialFunction_AND(int nFnId)
{
   int nNumRHSUnknowns = 0;
   int nNumLHSUnknowns = arrSolverFunctions[nFnId].fn_and.nNumLHSUnknownsNew;
   double fSumRHSUnknowns = 0;
   int nNumElts = arrSolverFunctions[nFnId].fn_and.rhsVbles.nNumElts;
   int *arrElts = arrSolverFunctions[nFnId].fn_and.rhsVbles.arrElts;
   int RHSValue = 1;
   int falseRHSIdx = -1;
   int unknownRHSIdx = -1;
   for (int j = 0; j < nNumElts; j++) {
      int vble = arrElts[j];
      if (arrSolution[vble]==BOOL_UNKNOWN) 
      {
         nNumRHSUnknowns++;
         unknownRHSIdx = j;
         if (arrJWeights) fSumRHSUnknowns += arrJWeights[vble];
      }
      else
         if (arrSolution[vble] != arrSolverFunctions[nFnId].fn_and.arrRHSPolarities[j]) 
         { 
            RHSValue = 0;
            falseRHSIdx = j;
         }
   }
   d9_printf4("RHSValue: %d; falseRHSIdx: %d; nNumRHSUnknowns: %d\n",
         RHSValue,     falseRHSIdx,     nNumRHSUnknowns);

   if (nNumRHSUnknowns == arrSolverFunctions[nFnId].fn_and.nNumRHSUnknowns &&
         arrSolution[arrSolverFunctions[nFnId].fn_and.nLHSVble] == BOOL_UNKNOWN) return NO_ERROR;

   if (arrSolution[arrSolverFunctions[nFnId].fn_and.nLHSVble] == 
         arrSolverFunctions[nFnId].fn_and.nLHSPolarity)
   {
      if (RHSValue == 0) 
      {
         /* conflict */
         assert(falseRHSIdx != -1);
         d9_printf1("LHS is true RHS is false -> conflict\n");
         pConflictLemma = arrSolverFunctions[nFnId].fn_and.arrShortLemmas[falseRHSIdx];
         // Conflict -- backtrack.
         return ERR_BT_SPEC_FN_AND;
      }

      /* infer n atoms on RHS to true 
       * where n = nNumRHSUnknowns
       */
      d9_printf1("LHS is true -> inferring RHS unknowns \n");
      InferNLits(arrSolverFunctions[nFnId].fn_and.rhsVbles.nNumElts, 
            arrSolverFunctions[nFnId].fn_and.rhsVbles.arrElts, 
            arrSolverFunctions[nFnId].fn_and.arrRHSPolarities,
            arrSolverFunctions[nFnId].fn_and.arrShortLemmas,
            nNumRHSUnknowns);
      nNumRHSUnknowns = 0;
      nNumLHSUnknowns = 0;
      fSumRHSUnknowns = 0;
      nNumUnresolvedFunctions--;
   }
   else 
      if (RHSValue == 0) 
      {
         d9_printf1("RHS is false \n");
         if (arrSolution[arrSolverFunctions[nFnId].fn_and.nLHSVble] == BOOL_UNKNOWN)
         {
            /* infer LHS to false */
            assert(falseRHSIdx != -1);
            d9_printf1("RHS is false -> inferring LHS unknown \n");
            ite_counters[INF_SPEC_FN_AND]++;
            InferLiteral(arrSolverFunctions[nFnId].fn_and.nLHSVble, 
                  BOOL_NEG(arrSolverFunctions[nFnId].fn_and.nLHSPolarity),
                  false, 
                  arrSolverFunctions[nFnId].fn_and.arrShortLemmas[falseRHSIdx],
                  NULL, 1);
         }
         nNumRHSUnknowns = 0;
         nNumLHSUnknowns = 0;
         fSumRHSUnknowns = 0;
         nNumUnresolvedFunctions--;
      }
      else 
         if (nNumRHSUnknowns == 1 && arrSolution[arrSolverFunctions[nFnId].fn_and.nLHSVble] != BOOL_UNKNOWN)
         {
            /* LHS is false!!! */
            /* need to use the long lemma */
            /* infer n atoms on RHS to arrSolution[pSpecialFunc->nLHSVble] 
             * where n = 1
             */
            assert(unknownRHSIdx != -1);
            d9_printf1("Inferring RHS to LHS\n");
            ite_counters[INF_SPEC_FN_AND]++;
            InferLiteral(arrElts[unknownRHSIdx],
                  BOOL_NEG(arrSolverFunctions[nFnId].fn_and.arrRHSPolarities[unknownRHSIdx]),
                  false, 
                  arrSolverFunctions[nFnId].fn_and.pLongLemma, 
                  NULL, 1);
            nNumRHSUnknowns = 0;
            fSumRHSUnknowns = 0;
            nNumUnresolvedFunctions--;
         }
         else
            if (nNumRHSUnknowns == 0 && arrSolution[arrSolverFunctions[nFnId].fn_and.nLHSVble] == BOOL_UNKNOWN)
            {
               /* infer LHS to true using long lemma */
               d9_printf1("Inferring LHS to RHS\n");
               ite_counters[INF_SPEC_FN_AND]++;
               InferLiteral(arrSolverFunctions[nFnId].fn_and.nLHSVble, 
                     arrSolverFunctions[nFnId].fn_and.nLHSPolarity,
                     false, 
                     arrSolverFunctions[nFnId].fn_and.pLongLemma, 
                     NULL, 1);
               nNumRHSUnknowns = 0;
               nNumLHSUnknowns = 0;
               fSumRHSUnknowns = 0;
               nNumUnresolvedFunctions--;
            }
            else
               if (nNumRHSUnknowns == 0 && RHSValue == 1)
               {
                  /* conflict */
                  d9_printf1("Conflict - RHS is true, LHS is false\n");
                  pConflictLemma = arrSolverFunctions[nFnId].fn_and.pLongLemma;
                  // Conflict -- backtrack.
                  return ERR_BT_SPEC_FN_AND;
               }

   //Save_arrNumRHSUnknowns(nFnId);
   arrSolverFunctions[nFnId].fn_and.nNumRHSUnknowns = nNumRHSUnknowns;
   arrSolverFunctions[nFnId].fn_and.nNumLHSUnknowns = nNumLHSUnknowns;
   arrSolverFunctions[nFnId].fn_and.fSumRHSUnknowns = fSumRHSUnknowns;
/*
   if (nHeuristic != JOHNSON_HEURISTIC) 
   {
      arrPrevNumRHSUnknowns[nFnId] = arrNumRHSUnknowns[nFnId];
      arrPrevNumLHSUnknowns[nFnId] = arrNumLHSUnknowns[nFnId];
      arrPrevSumRHSUnknowns[nFnId] = arrSumRHSUnknowns[nFnId];
      arrPrevRHSCounter[nFnId]     = arrRHSCounter[nFnId];
   }
   */

   return NO_ERROR;
}

int AndUpdateAffectedFunction(int nFnId)
{
   // big update
   if (arrSolverFunctions[nFnId].fn_and.nNumRHSUnknowns <= 0) return NO_ERROR ;
   /* the following depends on the polarity -- opportunity for fine tuning
   if (arrSolverFunctions[nFnId].fn_and.nNumRHSUnknownsNew>0
   && arrSolverFunctions[nFnId].fn_and.nNumLHSUnknownsNew>0)
       return NO_ERROR; 
    */
   return UpdateSpecialFunction_AND(nFnId);
}

int AndUpdateAffectedFunction_Infer(void *oneafs, int x)
{
   // small update
   if (((OneAFS*)oneafs)->fn_and.nRHSPos >= 0) 
      arrSolverFunctions[((OneAFS*)oneafs)->nFnId].fn_and.nNumRHSUnknownsNew--;
   else
      arrSolverFunctions[((OneAFS*)oneafs)->nFnId].fn_and.nNumLHSUnknownsNew--;
   return NO_ERROR;
}

int AndSave2Stack(int nFnId, void *one_stack)
{
   ((FnStack*)one_stack)->fn_and.rhs_unknowns = 
      arrSolverFunctions[nFnId].fn_and.nNumRHSUnknowns;
   ((FnStack*)one_stack)->fn_and.lhs_unknowns = 
      arrSolverFunctions[nFnId].fn_and.nNumLHSUnknowns;
   ((FnStack*)one_stack)->fn_and.rhs_sum = 
      arrSolverFunctions[nFnId].fn_and.fSumRHSUnknowns;
   return NO_ERROR;
}

int AndRestoreFromStack(void *one_stack)
{
   int nFnId = ((FnStack*)one_stack)->nFnId;
   arrSolverFunctions[nFnId].fn_and.nNumRHSUnknowns =
      arrSolverFunctions[nFnId].fn_and.nNumRHSUnknownsNew =
      arrSolverFunctions[nFnId].fn_and.nNumRHSUnknownsPrev =
      ((FnStack*)one_stack)->fn_and.rhs_unknowns;
   arrSolverFunctions[nFnId].fn_and.nNumLHSUnknowns =
      arrSolverFunctions[nFnId].fn_and.nNumLHSUnknownsNew =
      arrSolverFunctions[nFnId].fn_and.nNumLHSUnknownsPrev =
      ((FnStack*)one_stack)->fn_and.lhs_unknowns;
   arrSolverFunctions[nFnId].fn_and.fSumRHSUnknowns =
      arrSolverFunctions[nFnId].fn_and.fSumRHSUnknownsNew =
      arrSolverFunctions[nFnId].fn_and.fSumRHSUnknownsPrev =
      ((FnStack*)one_stack)->fn_and.rhs_sum;

   return NO_ERROR;
}

void AndUpdateFunctionInfEnd(int nFnId)
{
   arrSolverFunctions[nFnId].fn_and.nNumRHSUnknownsNew =
      arrSolverFunctions[nFnId].fn_and.nNumRHSUnknownsPrev =
      arrSolverFunctions[nFnId].fn_and.nNumRHSUnknowns;
   arrSolverFunctions[nFnId].fn_and.nNumLHSUnknownsNew =
      arrSolverFunctions[nFnId].fn_and.nNumLHSUnknownsPrev =
      arrSolverFunctions[nFnId].fn_and.nNumLHSUnknowns;
   arrSolverFunctions[nFnId].fn_and.fSumRHSUnknownsNew =
      arrSolverFunctions[nFnId].fn_and.fSumRHSUnknownsPrev =
      arrSolverFunctions[nFnId].fn_and.fSumRHSUnknowns;
}
