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

double ***arrNEGMINMAXWghts = NULL;
int *arrMaxNEGMINMAXTrue = NULL;
int *arrMaxNEGMINMAXFalse = NULL;

int nNEGMINMAXWghtsSize = 0;

//---------------------------------------------------------------

ITE_INLINE void LSGBNEGMINMAXStateSetHeurScore(void *pState) {
	
	NEGMINMAXStateEntry *pNEGMINMAXState = (NEGMINMAXStateEntry *)pState;
	
   // Find out the length of the diff (max-min)

   int minmax_diff = pNEGMINMAXState->nMax - pNEGMINMAXState->nMin;

   if(nNEGMINMAXWghtsSize < minmax_diff+1) {
      arrNEGMINMAXWghts = (double***)ite_recalloc(arrNEGMINMAXWghts, nNEGMINMAXWghtsSize, minmax_diff+1, sizeof(double**), 2, "arrNEGMINMAXWghts");
      arrMaxNEGMINMAXTrue = (int*)ite_recalloc(arrMaxNEGMINMAXTrue, nNEGMINMAXWghtsSize, minmax_diff+1, sizeof(int), 2, "arrMaxNEGMINMAXTrue");
      arrMaxNEGMINMAXFalse = (int*)ite_recalloc(arrMaxNEGMINMAXFalse, nNEGMINMAXWghtsSize, minmax_diff+1, sizeof(int), 2, "arrMaxNEGMINMAXFalse");

      nNEGMINMAXWghtsSize = minmax_diff+1;
   }
   
   int recompute_true = 0;
   int old_true = 0;
   if (arrMaxNEGMINMAXTrue[minmax_diff] < (pNEGMINMAXState->nMax + 2)) {
      recompute_true = 1;
      old_true = arrMaxNEGMINMAXTrue[minmax_diff];
      arrMaxNEGMINMAXTrue[minmax_diff] = pNEGMINMAXState->nMax + 2;
   }

   int recompute_false = 0;
   int old_false = 0;
   if (arrMaxNEGMINMAXFalse[minmax_diff] < ((pNEGMINMAXState->nSize - pNEGMINMAXState->nMin) + 2)) {
      recompute_false = 1;
      old_false = arrMaxNEGMINMAXFalse[minmax_diff];
      arrMaxNEGMINMAXFalse[minmax_diff] = (pNEGMINMAXState->nSize - pNEGMINMAXState->nMin) + 2;
   }

   if(recompute_true>0 || recompute_false>0) {
      int i = minmax_diff;
      if(recompute_true > 0)
        arrNEGMINMAXWghts[i] = (double**)ite_recalloc(arrNEGMINMAXWghts[i], old_true, arrMaxNEGMINMAXTrue[i], sizeof(double*), 2, "arrNEGMINMAXWghts[i]");
      
      for(int j=0; j<arrMaxNEGMINMAXTrue[i]; j++) {
         arrNEGMINMAXWghts[i][j] = (double*)ite_recalloc(arrNEGMINMAXWghts[i][j], old_false, arrMaxNEGMINMAXFalse[i], sizeof(double), 2, "arrNEGMINMAXWghts[i][j]");
         arrNEGMINMAXWghts[i][j][0] = JHEURISTIC_K_TRUE; // diff = i
         if (j==0) {
            for(int m=old_false==0?1:old_false; m<arrMaxNEGMINMAXFalse[i]; m++) {
               arrNEGMINMAXWghts[i][0][m] = JHEURISTIC_K_TRUE;
            }
         } else {
            for(int m=old_false==0?1:old_false; m<arrMaxNEGMINMAXFalse[i]; m++) {
					if(j < i+2) {
						if(m < i+2) {
							arrNEGMINMAXWghts[i][j][m] = JHEURISTIC_K_TRUE;
						} else if (m == i+2) {
							arrNEGMINMAXWghts[i][j][m] = JHEURISTIC_K_INF*(double)j;
						} else arrNEGMINMAXWghts[i][j][m] = (arrNEGMINMAXWghts[i][j-1][m] + arrNEGMINMAXWghts[i][j][m-1]) / (2.0*JHEURISTIC_K);
					} else if (m < i+2 && j == i+2) {
						arrNEGMINMAXWghts[i][j][m] = JHEURISTIC_K_INF*(double)m;
					} else arrNEGMINMAXWghts[i][j][m] = (arrNEGMINMAXWghts[i][j-1][m] + arrNEGMINMAXWghts[i][j][m-1]) / (2.0*JHEURISTIC_K);
            }
         }
      }
   }

   // print it -- debug
   // 
   //for(int i=0; i<nNEGMINMAXWghtsSize; i++) {
   //if (arrMaxNEGMINMAXTrue[i] == 0) continue;
   //fprintf(stderr, "\nNEGMINMAX Diff = %d: \n", i);
   //for(int j=0; j < arrMaxNEGMINMAXTrue[i]; j++) {
   //for(int m=0; m<arrMaxNEGMINMAXFalse[i]; m++) {
   //fprintf(stderr, " %2.6f ", arrNEGMINMAXWghts[i][j][m]);
   //}
   //fprintf(stderr, "\n");
   //}
   //fprintf(stderr, "\n");
   //}
}

ITE_INLINE void LSGBNEGMINMAXCounterStateSetHeurScore(void *pState) {
	LSGBNEGMINMAXStateSetHeurScore(((NEGMINMAXCounterStateEntry *)pState)->pNEGMINMAXState);
}

ITE_INLINE double LSGBNEGMINMAXStateGetHeurScore(void *pState) {
   NEGMINMAXStateEntry *pNEGMINMAXState = (NEGMINMAXStateEntry *)pState;
   return arrNEGMINMAXWghts[pNEGMINMAXState->nMax - pNEGMINMAXState->nMin][pNEGMINMAXState->nMax][pNEGMINMAXState->nSize - pNEGMINMAXState->nMin];
}

ITE_INLINE double LSGBNEGMINMAXCounterStateGetHeurScore(void *pState) {
	return LSGBNEGMINMAXStateGetHeurScore(((NEGMINMAXCounterStateEntry *)pState)->pNEGMINMAXState);
}

ITE_INLINE double LSGBNEGMINMAXCounterGetHeurScorePos(NEGMINMAXCounterStateEntry *pState) {
   NEGMINMAXStateEntry *pNEGMINMAXState = pState->pNEGMINMAXState;
   int max = pNEGMINMAXState->nMax - (pState->nNumTrue+1);
   int min = pNEGMINMAXState->nMin - (pState->nNumTrue+1);
   return arrNEGMINMAXWghts[pNEGMINMAXState->nMax - pNEGMINMAXState->nMin][max+1][((pState->nVarsLeft-1) - min)+1];
}

ITE_INLINE double LSGBNEGMINMAXCounterGetHeurScoreNeg(NEGMINMAXCounterStateEntry *pState) {
   NEGMINMAXStateEntry *pNEGMINMAXState = pState->pNEGMINMAXState;
   int max = pNEGMINMAXState->nMax - pState->nNumTrue;
   int min = pNEGMINMAXState->nMin - pState->nNumTrue;
   return arrNEGMINMAXWghts[pNEGMINMAXState->nMax - pNEGMINMAXState->nMin][max+1][((pState->nVarsLeft-1) - min)+1];
}

ITE_INLINE void LSGBNEGMINMAXFree() {
   for(int i = 0; i < nNEGMINMAXWghtsSize; i++) {
      if(arrMaxNEGMINMAXTrue[i] == 0) continue;
      for(int j=0; j < arrMaxNEGMINMAXTrue[i]; j++)
        ite_free((void **)&arrNEGMINMAXWghts[i][j]);
      ite_free((void **)&arrNEGMINMAXWghts[i]);
   }
   
   if(arrNEGMINMAXWghts!=NULL) ite_free((void **)&arrNEGMINMAXWghts);
   if(arrMaxNEGMINMAXTrue!=NULL) ite_free((void **)&arrMaxNEGMINMAXTrue);
   if(arrMaxNEGMINMAXFalse!=NULL) ite_free((void **)&arrMaxNEGMINMAXFalse);
   
   nNEGMINMAXWghtsSize = 0;
}
