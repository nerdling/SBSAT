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
/* j-heuristic for special fns */

#include "ite.h"
#include "solver.h"

extern SpecialFunc *arrSpecialFuncs;
extern SmurfState **arrRegSmurfInitialStates;
extern int *arrNumRHSUnknowns;

double *arrXorEqWght = NULL;
double *arrXorEqWghtD = NULL;

ITE_INLINE void InitHeuristicTablesForSpecialFuncs_XOR();
ITE_INLINE void FreeHeuristicTablesForSpecialFuncs_XOR();

ITE_INLINE
void
InitHeuristicTablesForSpecialFuncs_XOR()
{
  HWEIGHT K = JHEURISTIC_K;

  // We need nMaxRHSSize to be at least one to insure that entry 1 exists
  // and we don't overrun the arrays.
  int nMaxRHSSize = 1;
  if (arrSpecialFuncs)
     for(int i=0; arrSpecialFuncs[i].nFunctionType != 0; i++) 
        if (arrSpecialFuncs[i].nFunctionType == SFN_XOR)
           if (nMaxRHSSize < arrSpecialFuncs[i].rhsVbles.nNumElts)
              nMaxRHSSize = arrSpecialFuncs[i].rhsVbles.nNumElts;

  arrXorEqWght = (double*)ite_calloc(nMaxRHSSize+1, sizeof(double), 9, "arrXorEqWght");
  arrXorEqWghtD = (double*)ite_calloc(nMaxRHSSize+1, sizeof(double), 9, "arrXorEqWghtD");

  if (sHeuristic[1] == 'm') 
  {
     arrXorEqWght[0] = 0.0;
     for (int i = 1; i <= nMaxRHSSize; i++)
     {
        arrXorEqWght[1] = 1.0 / i;
     }
  } else {
     arrXorEqWght[0] = 0.0;
     arrXorEqWght[1] = JHEURISTIC_K_TRUE+JHEURISTIC_K_INF; // 1.0;
     for (int i = 2; i <= nMaxRHSSize; i++)
     {
        arrXorEqWght[i] = arrXorEqWght[i-1]/K;
        arrXorEqWghtD[i] = arrXorEqWghtD[i-1]*(i-1)/i*K;
     }
  }
}

ITE_INLINE void
FreeHeuristicTablesForSpecialFuncs_XOR()
{
  ite_free((void**)&arrXorEqWght);
  ite_free((void**)&arrXorEqWghtD);
}

ITE_INLINE
void
GetHeurScoresFromSpecialFunc_XOR (int nSpecFuncIndex)
{
   SpecialFunc *pSpecialFunc = arrSpecialFuncs + nSpecFuncIndex;
   int nNumRHSUnknowns = arrNumRHSUnknowns[nSpecFuncIndex];

   if (pSpecialFunc->LinkedSmurfs == -1) {
      HWEIGHT fScore = arrXorEqWght[nNumRHSUnknowns-1];
      J_Update_RHS_AND(pSpecialFunc, fScore, fScore);
   } else {
      SmurfState *pState = arrRegSmurfInitialStates[pSpecialFunc->LinkedSmurfs];
      assert(pState->arrHeuristicXors);

      if (pState == pTrueSmurfState) {
         HWEIGHT fScore = pState->arrHeuristicXors[nNumRHSUnknowns]; 
         J_Update_RHS_AND(pSpecialFunc, fScore, fScore);
      } else if (nNumRHSUnknowns > 2) {
         HWEIGHT fScore = pState->arrHeuristicXors[nNumRHSUnknowns-1]; 
         J_Update_RHS_AND(pSpecialFunc, fScore, fScore);
      } else {
         int counter=0;
         int nNumElts = pSpecialFunc->rhsVbles.nNumElts;
         int *arrElts = pSpecialFunc->rhsVbles.arrElts;
         for (int j = 0; j < nNumElts; j++)
         {
            int vble = arrElts[j];
            if (arrSolution[vble]!=BOOL_UNKNOWN &&
                  arrSolution[vble] == pSpecialFunc->arrRHSPolarities[j])
               counter++;
         }
         int neg_idx = (pSpecialFunc->arrRHSPolarities[0]+counter) % 2;
         HWEIGHT fScorePos = pState->arrHeuristicXors[1-neg_idx];
         HWEIGHT fScoreNeg = pState->arrHeuristicXors[neg_idx];
         J_Update_RHS_AND(pSpecialFunc, fScorePos, fScoreNeg);
      }
   }
}

ITE_INLINE void
GetHeurScoresFromSpecialFunc_XOR_C(int nSpecFuncIndex)
{
   SpecialFunc *pSpecialFunc = arrSpecialFuncs + nSpecFuncIndex;
   int nNumRHSUnknowns = arrNumRHSUnknowns[nSpecFuncIndex];
   double fSum = arrSumRHSUnknowns[nSpecFuncIndex];

   if (pSpecialFunc->LinkedSmurfs == -1) {
      J_Update_RHS_AND_C(pSpecialFunc, 
            0, 0, 0, 0, 0, // fLastSum, fLastConstPos, fLastMultiPos, fLastConstNeg, fLastMultiNeg, 
            fSum, 0, arrXorEqWghtD[nNumRHSUnknowns-1], 0, arrXorEqWghtD[nNumRHSUnknowns-1]);
   } else {
      SmurfState *pState = arrRegSmurfInitialStates[pSpecialFunc->LinkedSmurfs];
      assert(pState->arrHeuristicXors);

      if (pState == pTrueSmurfState) {
         HWEIGHT fScore = pState->arrHeuristicXors[nNumRHSUnknowns]; 
         J_Update_RHS_AND(pSpecialFunc, fScore, fScore);
      } else if (nNumRHSUnknowns > 2) {
         HWEIGHT fScore = pState->arrHeuristicXors[nNumRHSUnknowns-1]; 
         J_Update_RHS_AND(pSpecialFunc, fScore, fScore);
      } else {
         int counter=0;
         int nNumElts = pSpecialFunc->rhsVbles.nNumElts;
         int *arrElts = pSpecialFunc->rhsVbles.arrElts;
         for (int j = 0; j < nNumElts; j++)
         {
            int vble = arrElts[j];
            if (arrSolution[vble]!=BOOL_UNKNOWN &&
                  arrSolution[vble] == pSpecialFunc->arrRHSPolarities[j])
               counter++;
         }
         int neg_idx = (pSpecialFunc->arrRHSPolarities[0]+counter) % 2;
         HWEIGHT fScorePos = pState->arrHeuristicXors[1-neg_idx];
         HWEIGHT fScoreNeg = pState->arrHeuristicXors[neg_idx];
         J_Update_RHS_AND(pSpecialFunc, fScorePos, fScoreNeg);
      }
   }
}

ITE_INLINE void
J_UpdateHeuristic_XOR(SpecialFunc *pSpecialFunc, int nOldNumRHSUnknowns, int nNumRHSUnknowns, int counter)
{
   if (pSpecialFunc->LinkedSmurfs == -1) {
      HWEIGHT fDelta = arrXorEqWght[nNumRHSUnknowns-1] -
         arrXorEqWght[nOldNumRHSUnknowns-1];
      J_Update_RHS_AND(pSpecialFunc, fDelta, fDelta);
   } else {
      // update special function 
      SmurfState *pState = arrPrevStates[pSpecialFunc->LinkedSmurfs];
      if (pState == pTrueSmurfState) {
         HWEIGHT fDelta = pState->arrHeuristicXors[nNumRHSUnknowns] -
            pState->arrHeuristicXors[nOldNumRHSUnknowns];
         J_Update_RHS_AND(pSpecialFunc, fDelta, fDelta);
      } else {
         if (nNumRHSUnknowns > 2) {
            HWEIGHT fDelta = pState->arrHeuristicXors[nNumRHSUnknowns-1] -
               pState->arrHeuristicXors[nOldNumRHSUnknowns-1];
            J_Update_RHS_AND(pSpecialFunc, fDelta, fDelta);
         } else {
            if (counter == -1) {
               counter=0;
               int nNumElts = pSpecialFunc->rhsVbles.nNumElts;
               int *arrElts = pSpecialFunc->rhsVbles.arrElts;
               for (int j = 0; j < nNumElts; j++)
               {
                  int vble = arrElts[j];
                  if (arrSolution[vble]!=BOOL_UNKNOWN &&
                        arrSolution[vble] == pSpecialFunc->arrRHSPolarities[j])
                     counter++;
               }
            }
            int neg_idx = (pSpecialFunc->arrRHSPolarities[0] + counter)%2;
            HWEIGHT fScorePos = pState->arrHeuristicXors[1-neg_idx] -
               pState->arrHeuristicXors[nOldNumRHSUnknowns-1];
            HWEIGHT fScoreNeg = pState->arrHeuristicXors[neg_idx] -
               pState->arrHeuristicXors[nOldNumRHSUnknowns-1];
            J_Update_RHS_AND(pSpecialFunc, fScorePos, fScoreNeg);
         }

         // update smurf 
         int n = nOldNumRHSUnknowns;
         int m = nNumRHSUnknowns;
         int j = 0;
         int *arrElts = pState->vbles.arrElts;
         if (m==0) {
            for (int k = 0; k < pState->vbles.nNumElts; k++) {
               int nVble = arrElts[k];
               Save_arrHeurScores(nVble);
               arrHeurScores[nVble].Pos +=
                  pState->arrTransitions[j+BOOL_TRUE].fHeuristicWeight-
                  pState->arrTransitions[j+BOOL_TRUE].pNextState->arrHeuristicXors[n];
               arrHeurScores[nVble].Neg +=
                  pState->arrTransitions[j+BOOL_FALSE].fHeuristicWeight-
                  pState->arrTransitions[j+BOOL_FALSE].pNextState->arrHeuristicXors[n];
               j+=2;
            }
         } else {
            for (int k = 0; k < pState->vbles.nNumElts; k++) {
               int nVble = arrElts[k];
               Save_arrHeurScores(nVble);
               arrHeurScores[nVble].Pos +=
                  pState->arrTransitions[j+BOOL_TRUE].pNextState->arrHeuristicXors[m]-
                  pState->arrTransitions[j+BOOL_TRUE].pNextState->arrHeuristicXors[n];
               arrHeurScores[nVble].Neg +=
                  pState->arrTransitions[j+BOOL_FALSE].pNextState->arrHeuristicXors[m]-
                  pState->arrTransitions[j+BOOL_FALSE].pNextState->arrHeuristicXors[n];
               j+=2;
            }
         }
      }
   }
}

ITE_INLINE void
J_UpdateHeuristic_XOR_C(SpecialFunc *pSpecialFunc, 
      int nOldNumRHSUnknowns, int nNumRHSUnknowns, 
      double fOldSumRHSUnknowns, double fSumRHSUnknowns,
      int counter)
{
   if (pSpecialFunc->LinkedSmurfs == -1) {
      J_Update_RHS_AND_C(pSpecialFunc, 
            // fLastSum, fLastConstPos, fLastMultiPos, fLastConstNeg, fLastMultiNeg, 
            fOldSumRHSUnknowns, 0, arrXorEqWghtD[nOldNumRHSUnknowns-1], 0, arrXorEqWghtD[nOldNumRHSUnknowns-1],
            fSumRHSUnknowns, 0, arrXorEqWghtD[nNumRHSUnknowns-1], 0, arrXorEqWghtD[nNumRHSUnknowns-1]);
   } else {
      // update special function 
      J_UpdateHeuristic_XOR(pSpecialFunc, nOldNumRHSUnknowns, nNumRHSUnknowns, counter);
   }
}
