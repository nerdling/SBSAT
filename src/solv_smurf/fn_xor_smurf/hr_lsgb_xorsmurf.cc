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

void
HrLSGBFnXorSmurfInit()
{
   for(int j=0; arrFnXorSmurfTypes[j] != 0; j++)
   {
      int i=arrFnXorSmurfTypes[j];
      procHeurGetScores[i] = LSGBXorSmurfGetHeurScores;
      procHeurUpdateFunctionInfEnd[i] = LSGBXorSmurfUpdateFunctionInfEnd;
   }
}

ITE_INLINE void
LSGBXorSmurfGetHeurScores(int nFnId)
{
   int nNumRHSUnknowns = arrSolverFunctions[nFnId].fn_xor.nNumRHSUnknowns;

   SmurfState *pState = arrSolverFunctions[functionProps[nFnId].xor_part_bddxor.bdd_part].fn_smurf.pCurrentState;
   assert(pState->arrHeuristicXors);

   if (pState == pTrueSmurfState) {
      HWEIGHT fScore = pState->arrHeuristicXors[nNumRHSUnknowns]; 
      J_Update_RHS_AND(arrSolverFunctions[nFnId].fn_xor.rhsVbles.nNumElts, arrSolverFunctions[nFnId].fn_xor.rhsVbles.arrElts, arrSolverFunctions[nFnId].fn_xor.arrRHSPolarities, fScore, fScore);
   } else if (nNumRHSUnknowns > 2) {
      HWEIGHT fScore = pState->arrHeuristicXors[nNumRHSUnknowns-1]; 
      J_Update_RHS_AND(arrSolverFunctions[nFnId].fn_xor.rhsVbles.nNumElts, arrSolverFunctions[nFnId].fn_xor.rhsVbles.arrElts, arrSolverFunctions[nFnId].fn_xor.arrRHSPolarities, fScore, fScore);
   } else {
      int counter=0;
      int nNumElts = arrSolverFunctions[nFnId].fn_xor.rhsVbles.nNumElts;
      int *arrElts = arrSolverFunctions[nFnId].fn_xor.rhsVbles.arrElts;
      for (int j = 0; j < nNumElts; j++)
      {
         int vble = arrElts[j];
         if (arrSolution[vble]!=BOOL_UNKNOWN &&
               arrSolution[vble] == arrSolverFunctions[nFnId].fn_xor.arrRHSPolarities[j])
            counter++;
      }
      int neg_idx = (arrSolverFunctions[nFnId].fn_xor.arrRHSPolarities[0]+counter) % 2;
      HWEIGHT fScorePos = pState->arrHeuristicXors[1-neg_idx];
      HWEIGHT fScoreNeg = pState->arrHeuristicXors[neg_idx];
      J_Update_RHS_AND(arrSolverFunctions[nFnId].fn_xor.rhsVbles.nNumElts, arrSolverFunctions[nFnId].fn_xor.rhsVbles.arrElts, arrSolverFunctions[nFnId].fn_xor.arrRHSPolarities, fScorePos, fScoreNeg);
   }
}


ITE_INLINE void
LSGBXorSmurfUpdateFunctionInfEnd(int nFnId)
{
   int nOldNumRHSUnknowns = arrSolverFunctions[nFnId].fn_xor.nNumRHSUnknownsPrev;
   int nNumRHSUnknowns = arrSolverFunctions[nFnId].fn_xor.nNumRHSUnknowns;
   int counter = -1; //arrSolverFunctions[nFnId].fn_xor.counter;

   // update special function 
   SmurfState *pState = arrSolverFunctions[functionProps[nFnId].xor_part_bddxor.bdd_part].fn_smurf.pPrevState;
   if (pState == pTrueSmurfState) {
      HWEIGHT fDelta = pState->arrHeuristicXors[nNumRHSUnknowns] -
         pState->arrHeuristicXors[nOldNumRHSUnknowns];
      J_Update_RHS_AND(arrSolverFunctions[nFnId].fn_xor.rhsVbles.nNumElts, arrSolverFunctions[nFnId].fn_xor.rhsVbles.arrElts, arrSolverFunctions[nFnId].fn_xor.arrRHSPolarities, fDelta, fDelta);
   } else {
      if (nNumRHSUnknowns > 2) {
         HWEIGHT fDelta = pState->arrHeuristicXors[nNumRHSUnknowns-1] -
            pState->arrHeuristicXors[nOldNumRHSUnknowns-1];
         J_Update_RHS_AND(arrSolverFunctions[nFnId].fn_xor.rhsVbles.nNumElts, arrSolverFunctions[nFnId].fn_xor.rhsVbles.arrElts, arrSolverFunctions[nFnId].fn_xor.arrRHSPolarities, fDelta, fDelta);
      } else {
         if (counter == -1) {
            counter=0;
            int nNumElts = arrSolverFunctions[nFnId].fn_xor.rhsVbles.nNumElts;
            int *arrElts = arrSolverFunctions[nFnId].fn_xor.rhsVbles.arrElts;
            for (int j = 0; j < nNumElts; j++)
            {
               int vble = arrElts[j];
               if (arrSolution[vble]!=BOOL_UNKNOWN &&
                     arrSolution[vble] == arrSolverFunctions[nFnId].fn_xor.arrRHSPolarities[j])
                  counter++;
            }
         }
         int neg_idx = (arrSolverFunctions[nFnId].fn_xor.arrRHSPolarities[0] + counter)%2;
         HWEIGHT fScorePos = pState->arrHeuristicXors[1-neg_idx] -
            pState->arrHeuristicXors[nOldNumRHSUnknowns-1];
         HWEIGHT fScoreNeg = pState->arrHeuristicXors[neg_idx] -
            pState->arrHeuristicXors[nOldNumRHSUnknowns-1];
         J_Update_RHS_AND(arrSolverFunctions[nFnId].fn_xor.rhsVbles.nNumElts, arrSolverFunctions[nFnId].fn_xor.rhsVbles.arrElts, arrSolverFunctions[nFnId].fn_xor.arrRHSPolarities, fScorePos, fScoreNeg);
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

