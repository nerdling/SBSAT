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
FnLemmaAuInit() {
   // not used
	return 1;
}

#if 0
ITE_INLINE int
FnLemmaInit() {
   InitLemmaSpacePool(0);
   InitLemmaInfoArray();

   /* Backtrack arrays */
   arrUnsetLemmaFlagVars = (int*)ite_calloc(gnMaxVbleIndex, sizeof(int),
         9, "arrUnsetLemmaFlagVars");
   arrTempLemma = (int*)ite_calloc(gnMaxVbleIndex, sizeof(int),
         9, "arrTempLemma");
   arrLemmaFlag = (bool*)ite_calloc(gnMaxVbleIndex+1, sizeof(bool),
         9, "arrLemmaFlag");

   return NO_ERROR;
}


ITE_INLINE void 
//LemmaCreateAFS(int nFnId, int nVarId, int nAFSIndex)
LemmaCreateAFS(int nVarId)
{
//   arrAFS[nVarId].arrOneAFS[nAFSIndex].nFnId = nFnId;
//   arrAFS[nVarId].arrOneAFS[nAFSIndex].nType = 0; //FN_SMURF;
   arrAFS[nVarId].LemmasWherePosTail[0].pNextLemma[0] = &(arrAFS[nVarId].LemmasWherePos[0]);
   arrAFS[nVarId].LemmasWherePosTail[1].pNextLemma[1] = &(arrAFS[nVarId].LemmasWherePos[1]);
   arrAFS[nVarId].LemmasWhereNegTail[0].pNextLemma[0] = &(arrAFS[nVarId].LemmasWhereNeg[0]);
   arrAFS[nVarId].LemmasWhereNegTail[1].pNextLemma[1] = &(arrAFS[nVarId].LemmasWhereNeg[1]);
}


/*
void LoadLemmas(char *filename);

   gnNumLemmas = 0;
   InitLemmaSpacePool(0);
   InitLemmaInfoArray();
   if (*lemma_in_file) LoadLemmas(lemma_in_file);

  FreeLemmaInfoArray();
  FreeLemmaSpacePool();
  */

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
#endif
