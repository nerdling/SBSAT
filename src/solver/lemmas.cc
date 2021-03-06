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

int nCallsToAddLemma =  0;
int gnNumLemmas = 0;

/*****************************************************/

ITE_INLINE
int lemma_compfunc(const void *x, const void *y) {
  int pp, qq;
  
  pp = *(const int *)x;
  qq = *(const int *)y;
  if (arrBacktrackStackIndex[pp] < arrBacktrackStackIndex[qq]) return -1;
  if (arrBacktrackStackIndex[pp] == arrBacktrackStackIndex[qq])
#ifndef FORCE_STABLE_QSORT
   return 0;
#else
  { if (x < y) return -1;
    else if (x > y) return 1;
    else return 0;
  }
#endif
  return 1;
}

/*****************************************************/


ITE_INLINE void
FreeLemma(LemmaInfoStruct *pLemmaInfo)
   // Frees the LemmaInfoStruct and LemmaBlocks for a lemma so that
   // they can be recycled for use in another lemma.
{
   gnNumLemmas--;

   assert(pLemmaInfo->nBacktrackStackReferences == 0);

   if (nHeuristic == C_LEMMA_HEURISTIC
#ifdef JOHNSON_HEURISTIC_LEMMA
         || nHeuristic == JOHNSON_HEURISTIC
#endif
         )
      RemoveLemmasHeuristicInfluence(pLemmaInfo);

   RemoveLemmaFromWatchedLits(pLemmaInfo);
   FreeLemmaBlocks(pLemmaInfo);
   FreeLemmaInfoStruct(pLemmaInfo);
}

ITE_INLINE LemmaInfoStruct *
AddLemma(int nNumLiterals, int arrLiterals[], bool bPutInCache, 
      LemmaInfoStruct *pUnitLemmaList, LemmaInfoStruct **pUnitLemmaListTail
      )
{
   gnNumLemmas++; 

   nCallsToAddLemma++;

   // Initialize lemma info struct.
   LemmaInfoStruct *pLemmaInfo = AllocateLemmaInfoStruct();
   //LPQEnqueue(pLemmaInfo); -- it might not be the cached lemma yet

   //Sort the lemma in the order of the branch process
   //  if(bPutInCache) qsort(arrLiterals, nNumLiterals, sizeof(int), lemma_compfunc);

   // Allocate the lemma blocks and store the literals there.
   EnterIntoLemmaSpace(nNumLiterals, arrLiterals, true,
         pLemmaInfo->pLemma, pLemmaInfo->pLemmaLastBlock,
         pLemmaInfo->nNumBlocks);

   pLemmaInfo->bPutInCache = bPutInCache;
   pLemmaInfo->nBacktrackStackReferences = 0;

   LemmaSetWatchedLits(pLemmaInfo, arrLiterals, nNumLiterals);

   if (pUnitLemmaList) {
      /* new lemma is put in the beginning of pUnitLemmaList */
      pLemmaInfo->pNextLemma[0] = pUnitLemmaList->pNextLemma[0];
      pUnitLemmaList->pNextLemma[0] = pLemmaInfo;

      if(*pUnitLemmaListTail == NULL)
         *pUnitLemmaListTail = pLemmaInfo;

      assert(IsInLemmaList(*pUnitLemmaListTail, pUnitLemmaList));
   }

   //Done updating brancher information for this variable in this lemma.

   TB_9(
      d9_printf1("Adding lemma: ");
      DisplayLemma(pLemmaInfo->pLemma);
      DisplayLemmaStatus(pLemmaInfo->pLemma);
      DisplayLemmaInfo(pLemmaInfo);
      d9_printf1("\n");
   )

   return pLemmaInfo;
}

