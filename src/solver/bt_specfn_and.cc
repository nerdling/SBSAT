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
#include "ite.h"
#include "solver.h"

/* param */
extern int *arrNumRHSUnknowns;
extern SpecialFunc *arrSpecialFuncs;
extern int nNumUnresolvedFunctions;
extern int *arrBacktrackStackIndex;

/* conflict resolution */
extern LemmaBlock *pConflictLemma;

ITE_INLINE void
InferNLits(SpecialFunc *pSpecialFunc, int n);

ITE_INLINE
int
UpdateSpecialFunction_AND(IndexRoleStruct *pIRS)
{
   int nSpecFuncIndex = pIRS->nSpecFuncIndex;
   int nNumRHSUnknowns = 0;
   int nNumLHSUnknowns = arrNumLHSUnknownsNew[nSpecFuncIndex];
   double fSumRHSUnknowns = 0;
   assert(arrNumRHSUnknowns[nSpecFuncIndex] > 0);

   SpecialFunc *pSpecialFunc = arrSpecialFuncs + nSpecFuncIndex;
   int nNumElts = pSpecialFunc->rhsVbles.nNumElts;
   int *arrElts = pSpecialFunc->rhsVbles.arrElts;
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
         if (arrSolution[vble] != pSpecialFunc->arrRHSPolarities[j]) 
         { 
            RHSValue = 0;
            falseRHSIdx = j;
         }
   }
   d9_printf4("RHSValue: %d; falseRHSIdx: %d; nNumRHSUnknowns: %d\n",
         RHSValue,     falseRHSIdx,     nNumRHSUnknowns);

   if (nNumRHSUnknowns == arrNumRHSUnknowns[nSpecFuncIndex] &&
         arrSolution[pSpecialFunc->nLHSVble] == BOOL_UNKNOWN) return NO_ERROR;

   if (arrSolution[pSpecialFunc->nLHSVble] == pSpecialFunc->nLHSPolarity)
   {
      if (RHSValue == 0) 
      {
         /* conflict */
         assert(falseRHSIdx != -1);
         d9_printf1("LHS is true RHS is false -> conflict\n");
         pConflictLemma = pSpecialFunc->arrShortLemmas[falseRHSIdx];
         // Conflict -- backtrack.
         return ERR_BT_SPEC_FN_AND;
      }

      /* infer n atoms on RHS to true 
       * where n = nNumRHSUnknowns
       */
      d9_printf1("LHS is true -> inferring RHS unknowns \n");
      InferNLits(pSpecialFunc, nNumRHSUnknowns);
      nNumRHSUnknowns = 0;
      nNumLHSUnknowns = 0;
      fSumRHSUnknowns = 0;
      nNumUnresolvedFunctions--;
   }
   else 
      if (RHSValue == 0) 
      {
         d9_printf1("RHS is false \n");
         if (arrSolution[pSpecialFunc->nLHSVble] == BOOL_UNKNOWN)
         {
            /* infer LHS to false */
            assert(falseRHSIdx != -1);
            d9_printf1("RHS is false -> inferring LHS unknown \n");
            ite_counters[INF_SPEC_FN_AND]++;
            InferLiteral(pSpecialFunc->nLHSVble, 
                  BOOL_NEG(pSpecialFunc->nLHSPolarity),
                  false, 
                  pSpecialFunc->arrShortLemmas[falseRHSIdx],
                  NULL, 1);
         }
         nNumRHSUnknowns = 0;
         nNumLHSUnknowns = 0;
         fSumRHSUnknowns = 0;
         nNumUnresolvedFunctions--;
      }
      else 
         if (nNumRHSUnknowns == 1 && arrSolution[pSpecialFunc->nLHSVble] != BOOL_UNKNOWN)
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
                  BOOL_NEG(pSpecialFunc->arrRHSPolarities[unknownRHSIdx]),
                  false, 
                  pSpecialFunc->pLongLemma, 
                  NULL, 1);
            nNumRHSUnknowns = 0;
            fSumRHSUnknowns = 0;
            nNumUnresolvedFunctions--;
         }
         else
            if (nNumRHSUnknowns == 0 && arrSolution[pSpecialFunc->nLHSVble] == BOOL_UNKNOWN)
            {
               /* infer LHS to true using long lemma */
               d9_printf1("Inferring LHS to RHS\n");
               ite_counters[INF_SPEC_FN_AND]++;
               InferLiteral(pSpecialFunc->nLHSVble, pSpecialFunc->nLHSPolarity,
                     false, 
                     pSpecialFunc->pLongLemma, 
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
                  pConflictLemma = pSpecialFunc->pLongLemma;
                  // Conflict -- backtrack.
                  return ERR_BT_SPEC_FN_AND;
               }

   //Save_arrNumRHSUnknowns(nSpecFuncIndex);
   arrNumRHSUnknowns[nSpecFuncIndex] = nNumRHSUnknowns;
   arrNumLHSUnknowns[nSpecFuncIndex] = nNumLHSUnknowns;
   arrSumRHSUnknowns[nSpecFuncIndex] = fSumRHSUnknowns;
   return NO_ERROR;
}
