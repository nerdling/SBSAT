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
/* j-heuristic for special fns */

#include "ite.h"
#include "solver.h"

extern SpecialFunc *arrSpecialFuncs;
extern int *arrNumRHSUnknowns;

double ***arrMinmaxWghts = NULL;
int max_minmax_diff=0;
int *arrMaxMinmaxTrue = NULL;
int *arrMaxMinmaxFalse = NULL;

ITE_INLINE void InitHeuristicTablesForSpecialFuncs_MINMAX();
ITE_INLINE void FreeHeuristicTablesForSpecialFuncs_MINMAX();

ITE_INLINE void
InitHeuristicTablesForSpecialFuncs_MINMAX()
{
   HWEIGHT K = JHEURISTIC_K;


   // for every difference max-min a special table is needed
   //
   //
   // e.g. 2 [ 6 ] 4 difference is (4-2) = 2
   //
   // |   .
   // 6   .
   // |   3
   // 4   2
   // |   1  the rest is 
   // 2   0  (down + left) / 2K
   // |   0
   // 0   0 0 0 1 2 3 ..
   //   
   //     0---2---4---6---...
   //      <--- left potentially set to false
   //
   // 2 [ 6 ] 4 will start at x (left set to false) (6-2) and y (left set to true 4)
   // 12 [ 16 ] 14 will start at x (left set to false) (16-12) and y (left set to true 14)

   // arrMinmaxWghts [ diff=max-min ] [ left unset to True (start with max) ] [ left unset to False (start with n-min) ]

   // Find out the length of the diff (min,max) array
   if (arrSpecialFuncs)
      for(int i=0; arrSpecialFuncs[i].nFunctionType != 0; i++) 
         if (arrSpecialFuncs[i].nFunctionType == SFN_MINMAX) {
            if (max_minmax_diff < (arrSpecialFuncs[i].max - arrSpecialFuncs[i].min)) {
               max_minmax_diff = (arrSpecialFuncs[i].max - arrSpecialFuncs[i].min);
            }
         }

   arrMinmaxWghts = (double***)ite_calloc(max_minmax_diff+1, sizeof(double**), 2, "arrMinmaxWghts");
   arrMaxMinmaxTrue = (int*)ite_calloc(max_minmax_diff+1, sizeof(int), 2, "arrMaxMinmaxSet");
   arrMaxMinmaxFalse = (int*)ite_calloc(max_minmax_diff+1, sizeof(int), 2, "arrMaxMinmaxUnset");

   // Find out the max bounds for each diff (min,max) array
   if (arrSpecialFuncs)
      for(int i=0; arrSpecialFuncs[i].nFunctionType != 0; i++) 
         if (arrSpecialFuncs[i].nFunctionType == SFN_MINMAX) {
            int minmax_diff = (arrSpecialFuncs[i].max - arrSpecialFuncs[i].min);
            if (arrMaxMinmaxTrue[minmax_diff] < arrSpecialFuncs[i].max)
               arrMaxMinmaxTrue[minmax_diff] = arrSpecialFuncs[i].max;
            if (arrMaxMinmaxFalse[minmax_diff] < arrSpecialFuncs[i].rhsVbles.nNumElts - arrSpecialFuncs[i].min)
               arrMaxMinmaxFalse[minmax_diff] = arrSpecialFuncs[i].rhsVbles.nNumElts - arrSpecialFuncs[i].min;
         }

   // create arrays and compute the weights
   for(int i=0; i<=max_minmax_diff; i++) {
      if (arrMaxMinmaxTrue[i] == 0) continue;
      arrMinmaxWghts[i] = (double**)ite_calloc(arrMaxMinmaxTrue[i]+1, sizeof(double*), 2, "arrMinmaxWghts[]");
      for(int j=0; j<=arrMaxMinmaxTrue[i]; j++) {
         arrMinmaxWghts[i][j] = (double*)ite_calloc(arrMaxMinmaxFalse[i]+1, sizeof(double), 2, "arrMinmaxWghts[][]");
         arrMinmaxWghts[i][j][0] = (j-i < 0? 0: j-i); // diff = i, j - LeftToSetTrue, 0 - LeftToSetFalse
         if (j==0) {
            for(int m=1; m<=arrMaxMinmaxTrue[i]; m++) {
               arrMinmaxWghts[i][0][m] = (m-i < 0? 0: m-i);
            }
         } else {
            for(int m=1; m<=arrMaxMinmaxTrue[i]; m++) {
               arrMinmaxWghts[i][j][m] = (arrMinmaxWghts[i][j-1][m] + arrMinmaxWghts[i][j][m-1]) / (2*K);
            }
         }
      }
   }

   // print it -- debug
   /*
   for(int i=0; i<=max_minmax_diff; i++) {
      if (arrMaxMinmaxTrue[i] == 0) continue;
      fprintf(stderr, "\nMINMAX Diff = %d: \n", i);
      for(int j=arrMaxMinmaxTrue[i]; j>=0; j--) {
            for(int m=0; m<=arrMaxMinmaxTrue[i]; m++) {
               fprintf(stderr, " %2.4f ", arrMinmaxWghts[i][j][m]);
            }
            fprintf(stderr, "\n");
      }
      fprintf(stderr, "\n");
   }
   */
}

ITE_INLINE void
FreeHeuristicTablesForSpecialFuncs_MINMAX()
{
   for(int i=0; i<=max_minmax_diff; i++) {
      if (arrMinmaxWghts[i] == 0) continue;
      for(int j=0; j<=arrMaxMinmaxTrue[i]; j++) {
         ite_free((void**)&arrMinmaxWghts[i][j]);
      }
      ite_free((void**)&arrMinmaxWghts[i]);
   }
   ite_free((void**)&arrMinmaxWghts);
   ite_free((void**)&arrMaxMinmaxTrue);
   ite_free((void**)&arrMaxMinmaxFalse);
}

ITE_INLINE void
GetHeurScoresFromSpecialFunc_MINMAX(int nSpecFuncIndex)
{
   SpecialFunc *pSpecialFunc = arrSpecialFuncs + nSpecFuncIndex;
   int minmax_diff = pSpecialFunc->max - pSpecialFunc->min;
   int left_to_set_true = pSpecialFunc->max;
   int left_to_set_false = pSpecialFunc->rhsVbles.nNumElts - pSpecialFunc->min;
   double fPosDelta = arrMinmaxWghts[minmax_diff][left_to_set_true-1][left_to_set_false];
   double fNegDelta = arrMinmaxWghts[minmax_diff][left_to_set_true][left_to_set_false-1];

   J_Update_RHS_AND(pSpecialFunc, fPosDelta, fNegDelta);
}

ITE_INLINE void
GetHeurScoresFromSpecialFunc_MINMAX_C(int nSpecFuncIndex)
{
   // variable weights are not supported
}

ITE_INLINE void
J_UpdateHeuristic_MINMAX(SpecialFunc *pSpecialFunc, int nOldNumRHSUnknowns, int nNumRHSUnknowns, int nOldRHSCounter, int nRHSCounter)
{
   int minmax_diff = pSpecialFunc->max - pSpecialFunc->min;
   int old_left_to_set_true = pSpecialFunc->max-nOldRHSCounter;
   int old_left_to_set_false = (pSpecialFunc->rhsVbles.nNumElts - pSpecialFunc->min) -
      ((pSpecialFunc->rhsVbles.nNumElts - nOldNumRHSUnknowns) - nOldRHSCounter);

   int left_to_set_true = pSpecialFunc->max-nRHSCounter;
   int left_to_set_false = (pSpecialFunc->rhsVbles.nNumElts - pSpecialFunc->min) -
      ((pSpecialFunc->rhsVbles.nNumElts - nNumRHSUnknowns) - nRHSCounter);
   double fPosDelta, fNegDelta;
   if (nNumRHSUnknowns == 0) {
      fPosDelta = -arrMinmaxWghts[minmax_diff][old_left_to_set_true-1][old_left_to_set_false];
      fNegDelta = -arrMinmaxWghts[minmax_diff][old_left_to_set_true][old_left_to_set_false-1];
   } else {
      fPosDelta = arrMinmaxWghts[minmax_diff][left_to_set_true-1][left_to_set_false] -
         arrMinmaxWghts[minmax_diff][old_left_to_set_true-1][old_left_to_set_false];
      fNegDelta = arrMinmaxWghts[minmax_diff][left_to_set_true][left_to_set_false-1] -
         arrMinmaxWghts[minmax_diff][old_left_to_set_true][old_left_to_set_false-1];
   }

   J_Update_RHS_AND(pSpecialFunc, fPosDelta, fNegDelta);
}

ITE_INLINE void
J_UpdateHeuristic_MINMAX_C(SpecialFunc *pSpecialFunc, 
      int nOldNumRHSUnknowns, int nNumRHSUnknowns, 
      double fOldSumRHSUnknowns, double fSumRHSUnknowns,
      int counter)
{
   // variable weights are not supported
}
