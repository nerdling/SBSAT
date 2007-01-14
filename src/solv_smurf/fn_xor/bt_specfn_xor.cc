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

ITE_INLINE void FillLemmaWithReversedPolarities(LemmaBlock *pLemma);

ITE_INLINE int
UpdateSpecialFunction_XOR(int nFnId)
{
   int nNumRHSUnknowns = arrSolverFunctions[nFnId].fn_xor.nNumRHSUnknownsNew;
   int counter=0;
   if (nNumRHSUnknowns <= 1) 
   {
      nNumRHSUnknowns = 0;
      int unknownRHSIdx = -1;
      int nNumElts = arrSolverFunctions[nFnId].fn_xor.rhsVbles.nNumElts;
      int *arrElts = arrSolverFunctions[nFnId].fn_xor.rhsVbles.arrElts;
      for (int j = 0; j < nNumElts; j++) 
      {
         int vble = arrElts[j];
         if (arrSolution[vble]==BOOL_UNKNOWN)
         {
            nNumRHSUnknowns++;
            unknownRHSIdx = j;
         }
         else
            if (arrSolution[vble] == arrSolverFunctions[nFnId].fn_xor.arrRHSPolarities[j])
            {
               counter++;
            }
      }

      if (nNumRHSUnknowns == 0) 
      {
         /* check for conflict? */
         /* false *//* -False == true */
         if ((counter % 2)==0 /* even */)
         {
            // Conflict -- backtrack.
            /* create lemma */
            if (NO_LEMMAS == 0) {
               assert(arrSolverFunctions[nFnId].fn_xor.pLongLemma);
               FillLemmaWithReversedPolarities(arrSolverFunctions[nFnId].fn_xor.pLongLemma);
               pConflictLemma = arrSolverFunctions[nFnId].fn_xor.pLongLemma;
            }
            // goto_Backtrack;
            return ERR_BT_SPEC_FN_XOR;
         }

         // The function is now satisfied.
         //arrNumRHSUnknowns[nFnId] = 0;
         nNumUnresolvedFunctions--;
         //return NO_ERROR;

      } /* nNumRHSUnknowns == 0 */
      else
         if (nNumRHSUnknowns == 1)
         {
            assert(unknownRHSIdx != -1);
            assert(arrSolverFunctions[nFnId].fn_xor.pLongLemma);
            assert(arrSolution[arrElts[unknownRHSIdx]] == BOOL_UNKNOWN);
            /* assign this variable the negative of the desired value *
             * if the function is already sat (counter%2) (odd)
             * and it equals the polarity (BOOL_TRUE) the inferred literal should be
             * BOOL_FALSE (negated to BOOL_TRUE)
             * if the function is unsat (counter%2 == 0) (even)
             * and it equals the polarity (BOOL_FALSE) the inferred literal should be
             * BOOL_FALSE (negated to BOOL_TRUE)
             */
            int nNewInferredAtom = arrElts[unknownRHSIdx];
            int nNewInferredValue = (
                  (counter%2)==arrSolverFunctions[nFnId].fn_xor.arrRHSPolarities[unknownRHSIdx]
                  ?BOOL_FALSE:BOOL_TRUE);
            if (NO_LEMMAS == 0) {
               arrSolution[nNewInferredAtom] = BOOL_NEG(nNewInferredValue);
               FillLemmaWithReversedPolarities(arrSolverFunctions[nFnId].fn_xor.pLongLemma);
               arrSolution[nNewInferredAtom] = BOOL_UNKNOWN;
            }

            ite_counters[INF_SPEC_FN_XOR]++;
            InferLiteral(nNewInferredAtom, nNewInferredValue, false,
                  arrSolverFunctions[nFnId].fn_xor.pLongLemma, NULL, 1);

            // The function is now satisfied.
            //arrNumRHSUnknowns[nFnId] = 0;
            nNumRHSUnknowns = 0;
            nNumUnresolvedFunctions--;
            //return NO_ERROR;
         }
   }

   //Save_arrNumRHSUnknowns(nFnId);

   arrSolverFunctions[nFnId].fn_xor.nNumRHSUnknowns = nNumRHSUnknowns;
   /*
   if (nHeuristic != JOHNSON_HEURISTIC) 
   {
      arrPrevNumRHSUnknowns[nFnId] = arrNumRHSUnknowns[nFnId];
      arrPrevSumRHSUnknowns[nFnId] = arrSumRHSUnknowns[nFnId];
      arrPrevRHSCounter[nFnId]     = arrRHSCounter[nFnId];
   }
   */

   return NO_ERROR;
}

int XorUpdateAffectedFunction(int nFnId)
{
   // big update
   if (arrSolverFunctions[nFnId].fn_xor.nNumRHSUnknowns <= 0) return NO_ERROR ;
   return UpdateSpecialFunction_XOR(nFnId); 
}

int XorUpdateAffectedFunction_Infer(void *oneafs, int x)
{
   // small update
   arrSolverFunctions[((OneAFS*)oneafs)->nFnId].fn_xor.nNumRHSUnknownsNew--;
   return NO_ERROR;
}

int XorSave2Stack(int nFnId, void *one_stack)
{
   ((FnStack*)one_stack)->fn_xor.rhs_unknowns =
      arrSolverFunctions[nFnId].fn_xor.nNumRHSUnknowns;
   //((FnStack*)one_stack)->fn_xor.rhs_counter =
      //arrSolverFunctions[nFnId].fn_xor.nRHSCounter;
   return NO_ERROR;
}

int XorRestoreFromStack(void *one_stack)
{
   int nFnId = ((FnStack*)one_stack)->nFnId;
   arrSolverFunctions[nFnId].fn_xor.nNumRHSUnknowns =
      arrSolverFunctions[nFnId].fn_xor.nNumRHSUnknownsNew =
      arrSolverFunctions[nFnId].fn_xor.nNumRHSUnknownsPrev =
      ((FnStack*)one_stack)->fn_xor.rhs_unknowns;
   //arrSolverFunctions[nFnId].fn_xor.nRHSCounter =
      //arrSolverFunctions[nFnId].fn_xor.nRHSCounterNew =
      //arrSolverFunctions[nFnId].fn_xor.nRHSCounterPrev =
      //((FnStack*)one_stack)->fn_xor.rhs_counter;

   return NO_ERROR;
}

void XorUpdateFunctionInfEnd(int nFnId)
{
   arrSolverFunctions[nFnId].fn_xor.nNumRHSUnknownsNew =
      arrSolverFunctions[nFnId].fn_xor.nNumRHSUnknownsPrev =
      arrSolverFunctions[nFnId].fn_xor.nNumRHSUnknowns;
}
 

