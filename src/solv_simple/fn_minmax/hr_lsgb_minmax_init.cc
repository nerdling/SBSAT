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

double ***arrMINMAXWghts = NULL;
int *arrMaxMINMAXTrue = NULL;
int *arrMaxMINMAXFalse = NULL;

int nMINMAXWghtsSize = 0;

//---------------------------------------------------------------

ITE_INLINE void LSGBMINMAXStateSetHeurScore(void *pState) {

	MINMAXStateEntry *pMINMAXState = (MINMAXStateEntry *)pState;
	
   // Find out the length of the diff (max-min)

   int minmax_diff = pMINMAXState->nMax - pMINMAXState->nMin;

   if(nMINMAXWghtsSize < minmax_diff+1) {
      arrMINMAXWghts = (double***)ite_recalloc(arrMINMAXWghts, nMINMAXWghtsSize, minmax_diff+1, sizeof(double**), 2, "arrMINMAXWghts");
      arrMaxMINMAXTrue = (int*)ite_recalloc(arrMaxMINMAXTrue, nMINMAXWghtsSize, minmax_diff+1, sizeof(int), 2, "arrMaxMINMAXTrue");
      arrMaxMINMAXFalse = (int*)ite_recalloc(arrMaxMINMAXFalse, nMINMAXWghtsSize, minmax_diff+1, sizeof(int), 2, "arrMaxMINMAXFalse");

      nMINMAXWghtsSize = minmax_diff+1;
   }
   
   int recompute_true = 0;
   int old_true = 0;
   if (arrMaxMINMAXTrue[minmax_diff] < (pMINMAXState->nMax + 1)) {
      recompute_true = 1;
      old_true = arrMaxMINMAXTrue[minmax_diff];
      arrMaxMINMAXTrue[minmax_diff] = pMINMAXState->nMax + 1;
   }

   int recompute_false = 0;
   int old_false = 0;
   if (arrMaxMINMAXFalse[minmax_diff] < ((pMINMAXState->nSize - pMINMAXState->nMin) + 1)) {
      recompute_false = 1;
      old_false = arrMaxMINMAXFalse[minmax_diff];
      arrMaxMINMAXFalse[minmax_diff] = (pMINMAXState->nSize - pMINMAXState->nMin) + 1;
   }

   if(recompute_true>0 || recompute_false>0) {
      int i = minmax_diff;
      if(recompute_true > 0)
        arrMINMAXWghts[i] = (double**)ite_recalloc(arrMINMAXWghts[i], old_true, arrMaxMINMAXTrue[i], sizeof(double*), 2, "arrMINMAXWghts[i]");
      
      for(int j=0; j<arrMaxMINMAXTrue[i]; j++) {
         arrMINMAXWghts[i][j] = (double*)ite_recalloc(arrMINMAXWghts[i][j], old_false, arrMaxMINMAXFalse[i], sizeof(double), 2, "arrMINMAXWghts[i][j]");
         arrMINMAXWghts[i][j][0] = (j-i <= 0?JHEURISTIC_K_TRUE:(JHEURISTIC_K_INF * (double)(j-i))); // diff = i, j - LeftToSetTrue, 0 - LeftToSetFalse
         if (j==0) {
            for(int m=old_false==0?1:old_false; m<arrMaxMINMAXFalse[i]; m++) {
               arrMINMAXWghts[i][0][m] = (m-i <= 0?JHEURISTIC_K_TRUE:(JHEURISTIC_K_INF * (double)(m-i)));
            }
         } else {
            for(int m=old_false==0?1:old_false; m<arrMaxMINMAXFalse[i]; m++) {
               arrMINMAXWghts[i][j][m] = (arrMINMAXWghts[i][j-1][m] + arrMINMAXWghts[i][j][m-1]) / (2.0*JHEURISTIC_K);
            }
         }
      }
   }

   // print it -- debug
   // 
   //for(int i=0; i<nMINMAXWghtsSize; i++) {
   //if (arrMaxMINMAXTrue[i] == 0) continue;
   //fprintf(stderr, "\nMINMAX Diff = %d: \n", i);
   //for(int j=0; j < arrMaxMINMAXTrue[i]; j++) {
   //for(int m=0; m<arrMaxMINMAXFalse[i]; m++) {
   //fprintf(stderr, " %2.6f ", arrMINMAXWghts[i][j][m]);
   //}
   //fprintf(stderr, "\n");
   //}
   //fprintf(stderr, "\n");
   //}
}

ITE_INLINE void LSGBMINMAXCounterStateSetHeurScore(void *pState) {	
	LSGBMINMAXStateSetHeurScore(((MINMAXCounterStateEntry *)pState)->pMINMAXState);
}

ITE_INLINE double LSGBMINMAXStateGetHeurScore(void *pState) {
   MINMAXStateEntry *pMINMAXState = (MINMAXStateEntry *)pState;
   return arrMINMAXWghts[pMINMAXState->nMax - pMINMAXState->nMin][pMINMAXState->nMax][pMINMAXState->nSize - pMINMAXState->nMin];
}

ITE_INLINE double LSGBMINMAXCounterStateGetHeurScore(void *pState) {
	return LSGBMINMAXStateGetHeurScore(((MINMAXCounterStateEntry *)pState)->pMINMAXState);
}

ITE_INLINE double LSGBMINMAXCounterGetHeurScorePos(MINMAXCounterStateEntry *pState) {
   MINMAXStateEntry *pMINMAXState = pState->pMINMAXState;
   int max = pMINMAXState->nMax - (pState->nNumTrue+1);
   int min = pMINMAXState->nMin - (pState->nNumTrue+1);
   return arrMINMAXWghts[pMINMAXState->nMax - pMINMAXState->nMin][max][(pState->nVarsLeft-1) - min];
}

ITE_INLINE double LSGBMINMAXCounterGetHeurScoreNeg(MINMAXCounterStateEntry *pState) {
   MINMAXStateEntry *pMINMAXState = pState->pMINMAXState;
   int max = pMINMAXState->nMax - pState->nNumTrue;
   int min = pMINMAXState->nMin - pState->nNumTrue;
   return arrMINMAXWghts[pMINMAXState->nMax - pMINMAXState->nMin][max][(pState->nVarsLeft-1) - min];
}

ITE_INLINE void LSGBMINMAXFree() {
   for(int i = 0; i < nMINMAXWghtsSize; i++) {
      if(arrMaxMINMAXTrue[i] == 0) continue;
      for(int j=0; j < arrMaxMINMAXTrue[i]; j++)
        ite_free((void **)&arrMINMAXWghts[i][j]);
      ite_free((void **)&arrMINMAXWghts[i]);
   }
   
   if(arrMINMAXWghts!=NULL) ite_free((void **)&arrMINMAXWghts);
   if(arrMaxMINMAXTrue!=NULL) ite_free((void **)&arrMaxMINMAXTrue);
   if(arrMaxMINMAXFalse!=NULL) ite_free((void **)&arrMaxMINMAXFalse);
   
   nMINMAXWghtsSize = 0;
}



/*

double ***arrMinmaxWghts = NULL;
int max_minmax_diff=0;
int *arrMaxMinmaxTrue = NULL;
int *arrMaxMinmaxFalse = NULL;

extern int arrFnMinMaxTypes[];

ITE_INLINE void LSGBMinMaxInitHeuristicTables();
ITE_INLINE void LSGBMinMaxFreeHeuristicTables();

void
HrLSGBFnMinMaxInit()
{
   for(int j=0; arrFnMinMaxTypes[j] != 0; j++)
   {
      int i=arrFnMinMaxTypes[j];
      procHeurGetScores[i] = LSGBMinMaxGetHeurScores;
      procHeurUpdateFunctionInfEnd[i] = LSGBMinMaxUpdateFunctionInfEnd;
   }
}


ITE_INLINE void
LSGBMinMaxInitHeuristicTables()
{
   HWEIGHT K = JHEURISTIC_K;


   // For every different max-min a special table is needed
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
   for(int i=0; i<nNumFuncs; i++) 
      if (arrSolverFunctions[i].nType == MINMAX) {
         if (max_minmax_diff < (arrSolverFunctions[i].fn_minmax.max - arrSolverFunctions[i].fn_minmax.min)) {
            max_minmax_diff = (arrSolverFunctions[i].fn_minmax.max - arrSolverFunctions[i].fn_minmax.min);
         }
      }

   arrMinmaxWghts = (double***)ite_calloc(max_minmax_diff+1, sizeof(double**), 2, "arrMinmaxWghts");
   arrMaxMinmaxTrue = (int*)ite_calloc(max_minmax_diff+1, sizeof(int), 2, "arrMaxMinmaxSet");
   arrMaxMinmaxFalse = (int*)ite_calloc(max_minmax_diff+1, sizeof(int), 2, "arrMaxMinmaxUnset");

   // Find out the max bounds for each diff (min,max) array
   for(int i=0; i<nNumFuncs; i++) 
      if (arrSolverFunctions[i].nType == MINMAX) {
         int minmax_diff = (arrSolverFunctions[i].fn_minmax.max - arrSolverFunctions[i].fn_minmax.min);
         if (arrMaxMinmaxTrue[minmax_diff] < arrSolverFunctions[i].fn_minmax.max)
            arrMaxMinmaxTrue[minmax_diff] = arrSolverFunctions[i].fn_minmax.max;
         if (arrMaxMinmaxFalse[minmax_diff] < arrSolverFunctions[i].fn_minmax.rhsVbles.nNumElts - arrSolverFunctions[i].fn_minmax.min)
            arrMaxMinmaxFalse[minmax_diff] = arrSolverFunctions[i].fn_minmax.rhsVbles.nNumElts - arrSolverFunctions[i].fn_minmax.min;
      }

   // create arrays and compute the weights
   for(int i=0; i<=max_minmax_diff; i++) {
      if (arrMaxMinmaxTrue[i] == 0) continue;
      arrMinmaxWghts[i] = (double**)ite_calloc(arrMaxMinmaxTrue[i]+1, sizeof(double*), 2, "arrMinmaxWghts[]");
      for(int j=0; j<=arrMaxMinmaxTrue[i]; j++) {
         arrMinmaxWghts[i][j] = (double*)ite_calloc(arrMaxMinmaxFalse[i]+1, sizeof(double), 2, "arrMinmaxWghts[][]");
         arrMinmaxWghts[i][j][0] = (j-i < 0? 0: j-i); // diff = i, j - LeftToSetTrue, 0 - LeftToSetFalse
         if (j==0) {
            for(int m=1; m<=arrMaxMinmaxFalse[i]; m++) {
               arrMinmaxWghts[i][0][m] = (m-i < 0? 0: m-i);
            }
         } else {
            for(int m=1; m<=arrMaxMinmaxFalse[i]; m++) {
               arrMinmaxWghts[i][j][m] = (arrMinmaxWghts[i][j-1][m] + arrMinmaxWghts[i][j][m-1]) / (2*K);
            }
         }
      }
   }

   // print it -- debug
  
 //for(int i=0; i<=max_minmax_diff; i++) {
 //if (arrMaxMinmaxTrue[i] == 0) continue;
 //fprintf(stderr, "\nMINMAX Diff = %d: \n", i);
 //for(int j=arrMaxMinmaxTrue[i]; j>=0; j--) {
 //for(int m=0; m<=arrMaxMinmaxTrue[i]; m++) {
 //fprintf(stderr, " %2.4f ", arrMinmaxWghts[i][j][m]);
 //}
 //fprintf(stderr, "\n");
 //}
 //fprintf(stderr, "\n");
 //}
}

ITE_INLINE void
LSGBMinMaxFreeHeuristicTables()
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
LSGBMinMaxGetHeurScores(int nFnId)
{
   if (arrMinmaxWghts == NULL) LSGBMinMaxInitHeuristicTables();

   int minmax_diff = arrSolverFunctions[nFnId].fn_minmax.max - arrSolverFunctions[nFnId].fn_minmax.min;
   int left_to_set_true = arrSolverFunctions[nFnId].fn_minmax.max;
   int left_to_set_false = arrSolverFunctions[nFnId].fn_minmax.rhsVbles.nNumElts - arrSolverFunctions[nFnId].fn_minmax.min;
   double fPosDelta = arrMinmaxWghts[minmax_diff][left_to_set_true-1][left_to_set_false];
   double fNegDelta = arrMinmaxWghts[minmax_diff][left_to_set_true][left_to_set_false-1];

   J_Update_RHS_AND(
         arrSolverFunctions[nFnId].fn_minmax.rhsVbles.nNumElts,
         arrSolverFunctions[nFnId].fn_minmax.rhsVbles.arrElts,
         arrSolverFunctions[nFnId].fn_minmax.arrRHSPolarities,
         fPosDelta, fNegDelta);
}

ITE_INLINE void
LSGBWMinMaxGetHeurScores(int nFnId)
{
   fprintf(stderr, "MinMax Error: variable weights are not yet supported\n");
   exit(1);
}

ITE_INLINE void
LSGBMinMaxUpdateFunctionInfEnd(int nFnId)
{
   int nOldNumRHSUnknowns = arrSolverFunctions[nFnId].fn_minmax.nNumRHSUnknownsPrev;
   int nNumRHSUnknowns = arrSolverFunctions[nFnId].fn_minmax.nNumRHSUnknowns;
   int nOldRHSCounter = arrSolverFunctions[nFnId].fn_minmax.nRHSCounterPrev;
   int nRHSCounter = arrSolverFunctions[nFnId].fn_minmax.nRHSCounter;

   int minmax_diff = arrSolverFunctions[nFnId].fn_minmax.max - arrSolverFunctions[nFnId].fn_minmax.min;
   int old_left_to_set_true = arrSolverFunctions[nFnId].fn_minmax.max-nOldRHSCounter;
   int old_left_to_set_false = (arrSolverFunctions[nFnId].fn_minmax.rhsVbles.nNumElts - arrSolverFunctions[nFnId].fn_minmax.min) -
      ((arrSolverFunctions[nFnId].fn_minmax.rhsVbles.nNumElts - nOldNumRHSUnknowns) - nOldRHSCounter);

   int left_to_set_true = arrSolverFunctions[nFnId].fn_minmax.max-nRHSCounter;
   int left_to_set_false = (arrSolverFunctions[nFnId].fn_minmax.rhsVbles.nNumElts - arrSolverFunctions[nFnId].fn_minmax.min) -
      ((arrSolverFunctions[nFnId].fn_minmax.rhsVbles.nNumElts - nNumRHSUnknowns) - nRHSCounter);
   double fPosDelta, fNegDelta;
   if (nNumRHSUnknowns == 0) {
      fPosDelta = -arrMinmaxWghts[minmax_diff][old_left_to_set_true==0?0:old_left_to_set_true-1][old_left_to_set_false];
      fNegDelta = -arrMinmaxWghts[minmax_diff][old_left_to_set_true][old_left_to_set_false==0?0:old_left_to_set_false-1];
   } else {
      fPosDelta = arrMinmaxWghts[minmax_diff][left_to_set_true-1][left_to_set_false] -
         arrMinmaxWghts[minmax_diff][old_left_to_set_true-1][old_left_to_set_false];
      fNegDelta = arrMinmaxWghts[minmax_diff][left_to_set_true][left_to_set_false-1] -
         arrMinmaxWghts[minmax_diff][old_left_to_set_true][old_left_to_set_false-1];
   }

   J_Update_RHS_AND(
         arrSolverFunctions[nFnId].fn_minmax.rhsVbles.nNumElts,
         arrSolverFunctions[nFnId].fn_minmax.rhsVbles.arrElts,
         arrSolverFunctions[nFnId].fn_minmax.arrRHSPolarities,
         fPosDelta, fNegDelta);
}

ITE_INLINE void
LSGBWMinMaxUpdateHeuristic(int nFnId, 
      int nOldNumRHSUnknowns, int nNumRHSUnknowns, 
      double fOldSumRHSUnknowns, double fSumRHSUnknowns,
      int counter)
{
   fprintf(stderr, "MinMax Error: variable weights are not yet supported\n");
   exit(1);
}


*/
