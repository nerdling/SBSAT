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

/* conflict resolution */
extern LemmaBlock *pConflictLemma;

ITE_INLINE void FillLemmaWithReversedPolarities(LemmaBlock *pLemma);

ITE_INLINE
int
UpdateSpecialFunction_XOR(IndexRoleStruct *pIRS)
{
   assert(pIRS->nRHSIndex >= 0); /* no LHS literal in XOR */
   int nSpecFuncIndex = pIRS->nSpecFuncIndex;
   SpecialFunc *pSpecialFunc = arrSpecialFuncs + nSpecFuncIndex;

   int nNumRHSUnknowns = arrNumRHSUnknownsNew[nSpecFuncIndex];
   int counter=0;
   if (nNumRHSUnknowns <= 1) 
   {
      nNumRHSUnknowns = 0;
      int unknownRHSIdx = -1;
      int nNumElts = pSpecialFunc->rhsVbles.nNumElts;
      int *arrElts = pSpecialFunc->rhsVbles.arrElts;
      for (int j = 0; j < nNumElts; j++) 
      {
         int vble = arrElts[j];
         if (arrSolution[vble]==BOOL_UNKNOWN)
         {
            nNumRHSUnknowns++;
            unknownRHSIdx = j;
         }
         else
            if (arrSolution[vble] == pSpecialFunc->arrRHSPolarities[j])
            {
               counter++;
            }
      }

      if (nNumRHSUnknowns == 0) 
      {
         /* check for conflict? */
         /* false *//* -False == true */
         assert(pSpecialFunc->nLHSVble == 0 && 
               pSpecialFunc->nLHSPolarity == BOOL_FALSE);
         if ((counter % 2)==0 /* even */)
         {
            // Conflict -- backtrack.
            /* create lemma */
            if (NO_LEMMAS == 0) {
               assert(pSpecialFunc->pLongLemma);
               FillLemmaWithReversedPolarities(pSpecialFunc->pLongLemma);
               pConflictLemma = pSpecialFunc->pLongLemma;
            }
            // goto_Backtrack;
            return ERR_BT_SPEC_FN_XOR;
         }

         // The function is now satisfied.
         //arrNumRHSUnknowns[nSpecFuncIndex] = 0;
         nNumUnresolvedFunctions--;
         //return NO_ERROR;

      } /* nNumRHSUnknowns == 0 */
      else
         if (nNumRHSUnknowns == 1)
         {
            assert(unknownRHSIdx != -1);
            assert(pSpecialFunc->pLongLemma);
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
                  (counter%2)==pSpecialFunc->arrRHSPolarities[unknownRHSIdx]
                  ?BOOL_FALSE:BOOL_TRUE);
            if (NO_LEMMAS == 0) {
               arrSolution[nNewInferredAtom] = BOOL_NEG(nNewInferredValue);
               FillLemmaWithReversedPolarities(pSpecialFunc->pLongLemma);
               arrSolution[nNewInferredAtom] = BOOL_UNKNOWN;
            }

            ite_counters[INF_SPEC_FN_XOR]++;
            InferLiteral(nNewInferredAtom, nNewInferredValue, false,
                  pSpecialFunc->pLongLemma, NULL, 1);

            // The function is now satisfied.
            //arrNumRHSUnknowns[nSpecFuncIndex] = 0;
            nNumRHSUnknowns = 0;
            nNumUnresolvedFunctions--;
            //return NO_ERROR;
         }
   }

   //Save_arrNumRHSUnknowns(nSpecFuncIndex);

   arrNumRHSUnknowns[nSpecFuncIndex] = nNumRHSUnknowns;
   return NO_ERROR;
}


