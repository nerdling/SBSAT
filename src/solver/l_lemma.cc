/* =========FOR INTERNAL USE ONLY. NO DISTRIBUTION PLEASE ========== */

/*********************************************************************
 Copyright 1999-2003, University of Cincinnati.  All rights reserved.
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

double *arrLemmaHeurScoresPos;
double *arrLemmaHeurScoresNeg;
int *arrLemmaVbleCountsPos;
int *arrLemmaVbleCountsNeg;
int *arrLastLemmaVbleCountsPos;
int *arrLastLemmaVbleCountsNeg;

ITE_INLINE
void
InitLemmaHeurArrays (int nMaxVbleIndex)
{
   ITE_NEW_CATCH(
         arrLemmaHeurScoresPos = new double[nMaxVbleIndex + 1];
         arrLemmaHeurScoresNeg = new double[nMaxVbleIndex + 1];
         arrLemmaVbleCountsPos = new int[nMaxVbleIndex + 1];
         arrLemmaVbleCountsNeg = new int[nMaxVbleIndex + 1];
         arrLastLemmaVbleCountsPos = new int[nMaxVbleIndex + 1];
         arrLastLemmaVbleCountsNeg = new int[nMaxVbleIndex + 1];,
         "InitLemmaHeurArrays");

   for (int i = 0; i <= nMaxVbleIndex; i++)
   {
      arrLemmaHeurScoresPos[i] = 0.0;
      arrLemmaHeurScoresNeg[i] = 0.0;
      arrLemmaVbleCountsPos[i] = 0;
      arrLemmaVbleCountsNeg[i] = 0;
      arrLastLemmaVbleCountsPos[i] = 0;
      arrLastLemmaVbleCountsNeg[i] = 0;
   }

}

ITE_INLINE
void
DeleteLemmaHeurArrays ()
{
   delete arrLemmaHeurScoresPos;
   delete arrLemmaHeurScoresNeg;
   delete arrLemmaVbleCountsPos;
   delete arrLemmaVbleCountsNeg;
   delete arrLastLemmaVbleCountsPos;
   delete arrLastLemmaVbleCountsNeg;

}

//#define fWght 1000;

ITE_INLINE
void
UpdateHeuristicWithLemma (int nNumLiterals, int arrLiterals[])
   // Updates : arrLemmaHeurScoresPos[], arrLemmaHeurScoresNeg[].
   // arrLiterals holds a lemma of length nNumLiterals.
   // This routine adds the heuristic weight for each literal
   // in the lemma into the heuristic weight for the variable
   // mentioned in the literal.
   // Since the lemma is a disjunction of literals, and since
   // the we use the And-Equals-False construct to precompute
   // the heuristic weights of literals in such disjunctions,
   // we get the weight out of the array arrAndEqFalseWght.
   // If the literal is positive, then we add nothing to the
   // positive weight of the variable.  (Setting the variable
   // to true would satisfy the lemma, and we award zero weight
   // to an assignment which satisfies a constraint.)
   // Similarly, if the literal is negative, then we add nothing
   // to the negative weight of the variable.
{
   // Get the weight which is to be added to each variable appearing
   // in the lemma.
   //double fWght = arrAndEqFalseWght[nNumLiterals].fPos * 10000;
   //double fWght = 1000;
   int nLiteral;

   //static double fMaxWeight = 0.0;

   //if(fWght > fMaxWeight) {
   // fMaxWeight = fWght;
   // fprintf(stdout, "\nfMaxWeight = %f\n", fWght);    
   //}

   // Scan through the lemma and add the weight to each variable therein.
   for (int i = 0; i < nNumLiterals; i++)
   {
      nLiteral = arrLiterals[i];
      if (nLiteral >= 0)
      {
         //arrLemmaHeurScoresPos[nLiteral] += fWght;
         arrLemmaVbleCountsPos[nLiteral]++;
      }
      else
      {
         //arrLemmaHeurScoresNeg[-nLiteral] += fWght;
         arrLemmaVbleCountsNeg[-nLiteral]++;
      }
   }
}

ITE_INLINE
void
RemoveLemmasHeuristicInfluence (LemmaInfoStruct * pLemmaInfo)
   // Updates : arrLemmaHeurScoresPos[], arrLemmaHeurScoresNeg[].
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
         //arrLemmaHeurScoresPos[nLiteral] -= fWght;
         arrLemmaVbleCountsPos[nLiteral]--;
      }
      else
      {
         //arrLemmaHeurScoresNeg[-nLiteral] -= fWght;
         arrLemmaVbleCountsNeg[-nLiteral]--;
      }
   }
}
