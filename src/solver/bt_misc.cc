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

/* 
 * input: pConflictLemma (used in ConstructTempLemma )
 * output:
 */

LemmaBlock * pConflictLemma=NULL;
LemmaInfoStruct * pConflictLemmaInfo=NULL;
bool *arrLemmaFlag=NULL;
int  *arrTempLemma=NULL;

ITE_INLINE int
ConstructTempLemma()
{
   int nTempLemmaIndex=0;
   LemmaBlock *pConflictLemmaBlock = pConflictLemma;
   int *arrConflictLemmaLits = pConflictLemmaBlock->arrLits;
   int nConflictLemmaLength = arrConflictLemmaLits[0];

   for (int nLitIndex = 1, nLitIndexInBlock = 1;
         nLitIndex <= nConflictLemmaLength;
         nLitIndex++, nLitIndexInBlock++)
   {
      if (nLitIndexInBlock == LITS_PER_LEMMA_BLOCK)
      {
         nLitIndexInBlock = 0;
         pConflictLemmaBlock = pConflictLemmaBlock->pNext;
         arrConflictLemmaLits = pConflictLemmaBlock->arrLits;
      }
      int nConflictLiteral = arrConflictLemmaLits[nLitIndexInBlock];
      int nConflictVble = abs(nConflictLiteral);
      assert(arrSolution[nConflictVble] != BOOL_UNKNOWN);
      assert(arrSolution[nConflictVble]
            == (nConflictLiteral > 0 ? BOOL_FALSE : BOOL_TRUE));
      assert(!arrLemmaFlag[nConflictVble]);
      arrTempLemma[nTempLemmaIndex++] = nConflictLiteral;
      arrLemmaFlag[nConflictVble] = true;
   }
   if (pConflictLemmaInfo) {
      FreeLemma(pConflictLemmaInfo);
      pConflictLemmaInfo = NULL;
   }
   return nTempLemmaIndex;
}

ITE_INLINE
void
InferLiteral(int nInferredAtom,
      int nInferredValue,
      bool bWasChoicePoint,
      LemmaBlock *pLemma,
      LemmaInfoStruct *pCachedLemma,
      int infer)
{
   assert(nInferredValue == BOOL_TRUE || nInferredValue == BOOL_FALSE);
   TB_9(
         d9_printf3("Inferring %c%d ", (nInferredValue==BOOL_TRUE?'+':'-'),
            nInferredAtom);
         if (pLemma) {
         DisplayLemmaStatus(pLemma);
         } else {
         if (infer == 0)
         d9_printf1("(choice point)\n");
         }
      )

      /*
       * flag changed structures
       */
      AffectedFuncsStruct *pAFS = arrAFS + nInferredAtom;

   /* Flag all smurfs */
   // Determine the potentially affected regular Smurfs.
   IndexRoleStruct *pIRS = pAFS->arrSpecFuncsAffected;
   int nNumRegSmurfsAffected = pAFS->nNumRegSmurfsAffected;
   int *arrRegSmurfsAffected = pAFS->arrRegSmurfsAffected;
   int nNumSpecialFuncsAffected = pAFS->nNumSpecialFuncsAffected;

   // Update each affected regular Smurf.
   for (int i = 0; i < nNumRegSmurfsAffected; i++)
   {
      // already true?
      if (arrCurrentStates[arrRegSmurfsAffected[i]] == pTrueSmurfState) continue;
      // already changed on this level and not complete updated?
      if ((arrChangedSmurfs[arrRegSmurfsAffected[i]]&1) != 0) continue;
      // already changed on this level at all?
      if (arrChangedSmurfs[arrRegSmurfsAffected[i]] == 0) {
            /* add on the heuristic update stack */
            Save_arrCurrentStates(arrRegSmurfsAffected[i]);
            assert(pFnInferenceQueueNextElt - arrFnInferenceQueue < (nNumRegSmurfs+nNumSpecialFuncs));
            pFnInferenceQueueNextElt->fn_type = 0;
            pFnInferenceQueueNextElt->fn_id = arrRegSmurfsAffected[i];
            pFnInferenceQueueNextElt++;
      }
      arrChangedSmurfs[arrRegSmurfsAffected[i]]=3;
      // FIXME: if there are no inferences -> transition rightaway and don't flag it
      // FIXME: NEW: add to inferenced function stack (SMURF, i)
      //Save_arrCurrentStates(arrRegSmurfsAffected[i]);
   }

   /* Flag all specfns */
   // Determine the potentially affected special functions.

   // Update each affected special function.
   for (int i = 0; i < nNumSpecialFuncsAffected; i++, pIRS++)
   {
      if (arrNumRHSUnknowns[pIRS->nSpecFuncIndex] == 0) continue;

      int nSpecFuncIndex = pIRS->nSpecFuncIndex;

       
       //if (arrSpecialFuncs[nSpecFuncIndex].nFunctionType == XOR && 
       //arrNumRHSUnknownsNew[nSpecFuncIndex] > 2) {
       //Save_arrNumRHSUnknowns(nSpecFuncIndex);
       //arrNumRHSUnknowns[nSpecFuncIndex] = --arrNumRHSUnknownsNew[nSpecFuncIndex];
       //continue;
       //}
       
      if ((arrChangedSpecialFn[nSpecFuncIndex]&1)==0) {
         if (arrChangedSpecialFn[nSpecFuncIndex]==0) {
            /* add on the heuristic update stack */
            Save_arrNumRHSUnknowns(nSpecFuncIndex);
            assert(pFnInferenceQueueNextElt - arrFnInferenceQueue < (nNumRegSmurfs+nNumSpecialFuncs));
            pFnInferenceQueueNextElt->fn_type = 1;
            pFnInferenceQueueNextElt->fn_id = nSpecFuncIndex;
            pFnInferenceQueueNextElt++;
         }
         arrChangedSpecialFn[nSpecFuncIndex]=3;
      }
      // FIXME: if there are no inferences -> transition rightaway and don't flag it
      //
      // arrSpecialFuncs[nSpecFuncIndex].nFunctionType
      //Save_arrNumRHSUnknowns(nSpecFuncIndex);
      if (pIRS->nRHSIndex >= 0) {
         arrNumRHSUnknownsNew[nSpecFuncIndex]--;
         // FIXME: Add xor/minmax counter
      } else arrNumLHSUnknownsNew[nSpecFuncIndex]--;

      // FIXME: NEW: add to inferenced function stack (SPFN, i)
   }
   /*
    * End of flagging
    */

   assert (arrSolution[nInferredAtom] == BOOL_UNKNOWN);
   arrSolution[nInferredAtom] = nInferredValue;

   // Enqueue inference.
   *(pInferenceQueueNextEmpty++) = nInferredAtom;

   arrBacktrackStackIndex[nInferredAtom] = nBacktrackStackIndex++;

   pBacktrackTop->nBranchVble = nInferredAtom;
   pBacktrackTop->bWasChoicePoint = bWasChoicePoint;
   pBacktrackTop->pLemma = pLemma;
   pBacktrackTop->pLemmaInfo = pCachedLemma;
   pBacktrackTop++;
}

ITE_INLINE void
InferNLits(SpecialFunc *pSpecialFunc, int n)
{
   int nNumElts = pSpecialFunc->rhsVbles.nNumElts;
   int *arrElts = pSpecialFunc->rhsVbles.arrElts;
   for (int j = 0; j < nNumElts && n; j++)
   {
      if (arrSolution[arrElts[j]] == BOOL_UNKNOWN)
      {
         n--;
         InferLiteral(arrElts[j], pSpecialFunc->arrRHSPolarities[j],
               false,
               pSpecialFunc->arrShortLemmas[j],
               NULL, 1);
      }
   }
   assert(n==0);
}


