#include "sbsat.h"
#include "sbsat_solver.h"
#include "solver.h"

ITE_INLINE int
FnLemmaAuInit() {
   // not used
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
