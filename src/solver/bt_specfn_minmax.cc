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

#include "ite.h"
#include "solver.h"

ITE_INLINE void FillLemmaWithReversedPolarities(LemmaBlock *pLemma);

ITE_INLINE void
InferNLits_MINMAX(SpecialFunc *pSpecialFunc, int nNumRHSUnknowns, int value)
{
   int nNumElts = pSpecialFunc->rhsVbles.nNumElts;
   int *arrElts = pSpecialFunc->rhsVbles.arrElts;
   int arrNewPolar[BOOL_MAX_SBSAT];
   if (value == BOOL_TRUE) {
      arrNewPolar[BOOL_TRUE] = BOOL_TRUE;
      arrNewPolar[BOOL_FALSE] = BOOL_FALSE;
   } else {
      arrNewPolar[BOOL_TRUE] = BOOL_FALSE;
      arrNewPolar[BOOL_FALSE] = BOOL_TRUE;
   }
   if (NO_LEMMAS == 1) 
   {
      for (int j = 0; j < nNumElts; j++)
      {
         int vble = arrElts[j];
         if (arrSolution[vble] == BOOL_UNKNOWN)
         {
            InferLiteral(vble, arrNewPolar[pSpecialFunc->arrRHSPolarities[j]],
                  false, NULL, NULL, 1);
         }
      }
   } else {
      int *arrLits = (int*)ite_calloc(nNumElts-nNumRHSUnknowns+1, sizeof(int), 
            9, "arrlits for minmax infer lemma");
      int nLitsPos = 0;
      for(int j=0; j<nNumElts; j++)
      {
         int vble = arrElts[j];
         if (arrSolution[vble]!=BOOL_UNKNOWN)
         {
            arrLits[nLitsPos++] = vble * (arrSolution[vble]==pSpecialFunc->arrRHSPolarities[j]?-1:1);
            assert(arrSolution[vble] == (arrLits[nLitsPos-1] > 0 ? BOOL_FALSE : BOOL_TRUE));
         }
      }
      assert(nLitsPos == nNumElts-nNumRHSUnknowns);
      LemmaBlock *pLemma = NULL;
      LemmaInfoStruct *pLemmaInfo = NULL;
      for (int j = 0; j < nNumElts; j++)
      {
         int vble = arrElts[j];
         if (arrSolution[vble] == BOOL_UNKNOWN)
         {
            arrLits[nLitsPos] = vble * (arrNewPolar[pSpecialFunc->arrRHSPolarities[j]]==BOOL_TRUE?1:-1);
            pLemmaInfo=AddLemma(nNumElts-nNumRHSUnknowns+1, arrLits, false, NULL, NULL);
            pLemma = pLemmaInfo->pLemma;
            InferLiteral(vble, arrNewPolar[pSpecialFunc->arrRHSPolarities[j]],
                  false, pLemma, pLemmaInfo, 1);
         }
      }
      free(arrLits);
   }
}


ITE_INLINE int
UpdateSpecialFunction_MINMAX(IndexRoleStruct *pIRS)
{
   assert(pIRS->nRHSIndex >= 0); // no LHS literal in MINMAX 
   int nSpecFuncIndex = pIRS->nSpecFuncIndex;
   SpecialFunc *pSpecialFunc = arrSpecialFuncs + nSpecFuncIndex;

   // need to retrieve the counter too to be effective
   //int nNumRHSUnknowns = arrNumRHSUnknownsNew[nSpecFuncIndex];
   int counter=0;
   int nNumRHSUnknowns = 0;
   int nNumElts = pSpecialFunc->rhsVbles.nNumElts;
   int *arrElts = pSpecialFunc->rhsVbles.arrElts;
   for (int j = 0; j < nNumElts; j++) 
   {
      int vble = arrElts[j];
    //  fprintf(stderr, "\nvble(%d) value(%d) pola(%d)", vble, arrSolution[vble], pSpecialFunc->arrRHSPolarities[j]);
      if (arrSolution[vble]==BOOL_UNKNOWN)
      {
         nNumRHSUnknowns++;
      }
      else if (arrSolution[vble] == pSpecialFunc->arrRHSPolarities[j])
      {
         counter++;
      }
   }
   //fprintf(stderr, "\nUnknowns(%d), counter(%d)\n", nNumRHSUnknowns, counter);

   if (nNumRHSUnknowns == 0 || counter > pSpecialFunc->max || 
         (counter >= pSpecialFunc->min && (counter+nNumRHSUnknowns)<=pSpecialFunc->max) ||
         ((counter+nNumRHSUnknowns)< pSpecialFunc->min))
   {
      // check for conflict? 
      assert(pSpecialFunc->nLHSVble == 0 && 
            pSpecialFunc->nLHSPolarity == BOOL_FALSE);
      if (counter < pSpecialFunc->min || counter > pSpecialFunc->max)
      {
         // Conflict -- backtrack.
         // create lemma 
         if (NO_LEMMAS == 0) {
            //assert(pSpecialFunc->pLongLemma);
            //FillLemmaWithReversedPolarities(pSpecialFunc->pLongLemma);
            //pConflictLemma = pSpecialFunc->pLongLemma;
            
            /* assign it to pConflictLemmaInfo so we can free it */
            int *arrLits = (int*)ite_calloc(nNumElts-nNumRHSUnknowns, sizeof(int), 
                  9, "arrlits for minmax conflict lemma");
            int nLitsPos=0;
            for(int j=0; j<nNumElts; j++)
            {
               int vble = arrElts[j];
               if (arrSolution[vble]!=BOOL_UNKNOWN)
               {
                  // reverse all polarities
                  arrLits[nLitsPos++] = vble * (arrSolution[vble]==pSpecialFunc->arrRHSPolarities[j]?-1:1);
                  assert(arrSolution[vble] == (arrLits[nLitsPos-1] > 0 ? BOOL_FALSE : BOOL_TRUE));
               }
            }
            pConflictLemmaInfo = AddLemma(nLitsPos, arrLits, false, NULL, NULL);
            pConflictLemma = pConflictLemmaInfo->pLemma;
            free(arrLits);
         }
         // goto_Backtrack;
         return ERR_BT_SPEC_FN_MINMAX;
      }

      // The function is now satisfied.
      nNumRHSUnknowns = 0;
      nNumUnresolvedFunctions--;

   } // nNumRHSUnknowns == 0 
   else
      if ((counter+nNumRHSUnknowns) == pSpecialFunc->min) {
         // infer all remaining to TRUE
         ite_counters[INF_SPEC_FN_MINMAX] += nNumRHSUnknowns;
         InferNLits_MINMAX(pSpecialFunc, nNumRHSUnknowns, BOOL_TRUE); // uses arrShortLemmas
         nNumRHSUnknowns = 0;
         nNumUnresolvedFunctions--;
      }
      else if (counter == pSpecialFunc->max)
      {
         // infer all remaining to FALSE
         ite_counters[INF_SPEC_FN_MINMAX] += nNumRHSUnknowns;
         InferNLits_MINMAX(pSpecialFunc, nNumRHSUnknowns, BOOL_FALSE); // uses arrShortLemmas
         nNumRHSUnknowns = 0;
         nNumUnresolvedFunctions--;
      }

   arrNumRHSUnknowns[nSpecFuncIndex] = nNumRHSUnknowns;
   arrRHSCounter[nSpecFuncIndex] = counter;
   return NO_ERROR;
}
