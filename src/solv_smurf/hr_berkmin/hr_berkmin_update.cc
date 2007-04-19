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

double *arrBerkMinHeurScoresPos;
double *arrBerkMinHeurScoresNeg;
extern int *arrLemmaVbleCountsPos;
extern int *arrLemmaVbleCountsNeg;
extern int *arrLastLemmaVbleCountsPos;
extern int *arrLastLemmaVbleCountsNeg;

ITE_INLINE
void
InitBerkMinHeurArrays (int nMaxVbleIndex)
{
   arrBerkMinHeurScoresPos = (double*)ite_calloc(nMaxVbleIndex+1, sizeof(double), 9, "arrBerkMinHeurScoresPos");
   arrBerkMinHeurScoresNeg = (double*)ite_calloc(nMaxVbleIndex+1, sizeof(double), 9, "arrBerkMinHeurScoresNeg");
   arrLemmaVbleCountsPos = (int*)ite_calloc(nMaxVbleIndex+1, sizeof(int), 9, "arrLemmaVbleCountsPos");
   arrLemmaVbleCountsNeg = (int*)ite_calloc(nMaxVbleIndex+1, sizeof(int), 9, "arrLemmaVbleCountsNeg");
   arrLastLemmaVbleCountsPos = (int*)ite_calloc(nMaxVbleIndex+1, sizeof(int), 9, "arrLastLemmaVbleCountsPos");
   arrLastLemmaVbleCountsNeg = (int*)ite_calloc(nMaxVbleIndex+1, sizeof(int), 9, "arrLastLemmaVbleCountsNeg");

   for (int i = 0; i <= nMaxVbleIndex; i++)
   {
      arrBerkMinHeurScoresPos[i] = 0.0;
      arrBerkMinHeurScoresNeg[i] = 0.0;
      arrLemmaVbleCountsPos[i] = 1; // to satisfy the heuristic
      arrLemmaVbleCountsNeg[i] = 1; // to satisfy the heuristic
      arrLastLemmaVbleCountsPos[i] = 0;
      arrLastLemmaVbleCountsNeg[i] = 0;
   }

}

ITE_INLINE
void
DeleteBerkMinHeurArrays ()
{
   ite_free((void**)&arrBerkMinHeurScoresPos);
   ite_free((void**)&arrBerkMinHeurScoresNeg);
   ite_free((void**)&arrLemmaVbleCountsPos);
   ite_free((void**)&arrLemmaVbleCountsNeg);
   ite_free((void**)&arrLastLemmaVbleCountsPos);
   ite_free((void**)&arrLastLemmaVbleCountsNeg);
}

//#define fWght 1000;
//
ITE_INLINE void
AddBerkMinSpaceHeuristicInfluence(int *arr, int num)
{
 //  fprintf(stderr, "num=%d\n", num);
   for(int i=0; i<num; i++)
   {
      int nLiteral = arr[i];
//      fprintf(stderr, "lit=%d, ", nLiteral);
      if (nLiteral >= 0)
      {
         //arrBerkMinHeurScoresPos[nLiteral] += fWght;
         arrLemmaVbleCountsPos[nLiteral]++;
      }
      else
      {
         //arrBerkMinHeurScoresNeg[-nLiteral] += fWght;
         arrLemmaVbleCountsNeg[-nLiteral]++;
      }
   }
}

ITE_INLINE void
AddBerkMinBlockHeuristicInfluence(LemmaBlock *pLemmaBlock)
{
   int *arrLits = pLemmaBlock->arrLits;
   int nLemmaLength = arrLits[0];
   int nLiteral;
   int nLitIndex, nLitIndexInBlock;

   //double fWght = arrAndEqFalseWght[nLemmaLength].fPos * 10000;
   //double fWght = 1000;

   for (nLitIndex = 1, nLitIndexInBlock = 1;
         nLitIndex <= nLemmaLength; nLitIndex++, nLitIndexInBlock++)
   {
      if (nLitIndexInBlock == LITS_PER_LEMMA_BLOCK)
      {
         nLitIndexInBlock = 0;
         pLemmaBlock = pLemmaBlock->pNext;
         arrLits = pLemmaBlock->arrLits;
      }
      nLiteral = arrLits[nLitIndexInBlock];
      if (nLiteral >= 0)
      {
         //arrBerkMinHeurScoresPos[nLiteral] += fWght;
         arrLemmaVbleCountsPos[nLiteral]++;
      }
      else
      {
         //arrBerkMinHeurScoresNeg[-nLiteral] += fWght;
         arrLemmaVbleCountsNeg[-nLiteral]++;
      }
   }
}

ITE_INLINE void
AddBerkMinHeuristicInfluence(LemmaInfoStruct * pLemmaInfo)
{
   LemmaBlock *pLemmaBlock = pLemmaInfo->pLemma;
   AddBerkMinBlockHeuristicInfluence(pLemmaBlock);
}


ITE_INLINE void
RemoveBerkMinHeuristicInfluence (LemmaInfoStruct * pLemmaInfo)
   // Updates : arrBerkMinHeurScoresPos[], arrBerkMinHeurScoresNeg[].
   // Remove the lemma's influence on the search heuristic.
{
   LemmaBlock *pLemmaBlock = pLemmaInfo->pLemma;
   int *arrLits = pLemmaBlock->arrLits;
   int nLemmaLength = arrLits[0];
   int nLiteral;
   int nLitIndex, nLitIndexInBlock;

   //double fWght = arrAndEqFalseWght[nLemmaLength].fPos * 10000;
   //double fWght = 1000;

   for (nLitIndex = 1, nLitIndexInBlock = 1;
         nLitIndex <= nLemmaLength; nLitIndex++, nLitIndexInBlock++)
   {
      if (nLitIndexInBlock == LITS_PER_LEMMA_BLOCK)
      {
         nLitIndexInBlock = 0;
         pLemmaBlock = pLemmaBlock->pNext;
         arrLits = pLemmaBlock->arrLits;
      }
      nLiteral = arrLits[nLitIndexInBlock];
      if (nLiteral >= 0)
      {
         //arrBerkMinHeurScoresPos[nLiteral] -= fWght;
         arrLemmaVbleCountsPos[nLiteral]--;
      }
      else
      {
         //arrBerkMinHeurScoresNeg[-nLiteral] -= fWght;
         arrLemmaVbleCountsNeg[-nLiteral]--;
      }
   }
}
