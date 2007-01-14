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

ITE_INLINE void
LSGBSmurfXorGetHeurScores(int nFnId)
{
   LSGBSmurfXorSetHeuristicScores(nFnId);

   // Get a ptr to the Smurf state.
   SmurfState *pState = arrSolverFunctions[nFnId].fn_smurf.pInitialState;
                     
   // Do nothing if constraint is trivial.
   if (pState == pTrueSmurfState) return;

   int *arrElts = pState->vbles.arrElts;
   int j=0;
   int k;
   int n = arrSolverFunctions[functionProps[nFnId].bdd_part_bddxor.xor_part].fn_xor.nNumRHSUnknowns;
   for (k = 0; k < pState->vbles.nNumElts; k++) {
      int nVble = arrElts[k];
      arrHeurScores[nVble].Pos +=
         pState->arrTransitions[j+BOOL_TRUE].pNextState->arrHeuristicXors[n];
      arrHeurScores[nVble].Neg +=
         pState->arrTransitions[j+BOOL_FALSE].pNextState->arrHeuristicXors[n];
      j+=2;
   }
}

ITE_INLINE void
LSGBSmurfXorUpdateFunctionInfEnd(int nFnId) {
   SmurfState *pOldState = arrSolverFunctions[nFnId].fn_smurf.pPrevState; 
   SmurfState *pState = arrSolverFunctions[nFnId].fn_smurf.pCurrentState;
   int k,j;
   int *arrElts;
   int neg_idx=0;
   HWEIGHT fScorePos = 0;
   HWEIGHT fScoreNeg = 0;

   // remove heuristic influence 
   arrElts  = pOldState->vbles.arrElts;
   j=0;
   int xor_part = functionProps[nFnId].bdd_part_bddxor.xor_part;
   int nPrevNumRHSUnknowns = arrSolverFunctions[xor_part].fn_xor.nNumRHSUnknownsPrev;
   if (nPrevNumRHSUnknowns == 0)  {
      for (k = 0; k < pOldState->vbles.nNumElts; k++) {
         int nVble = arrElts[k];
         Save_arrHeurScores(nVble);
         arrHeurScores[nVble].Pos -= 
            pOldState->arrTransitions[j+BOOL_TRUE].fHeuristicWeight;
         arrHeurScores[nVble].Neg -= 
            pOldState->arrTransitions[j+BOOL_FALSE].fHeuristicWeight;
         j+=2;
      }
   } else {
      // nPrevNumRHSUnknowns > 0 
      for (k = 0; k < pOldState->vbles.nNumElts; k++) {
         int nVble = arrElts[k];
         Save_arrHeurScores(nVble);
         arrHeurScores[nVble].Pos -= 
            pOldState->arrTransitions[j+BOOL_TRUE].pNextState->arrHeuristicXors[nPrevNumRHSUnknowns];
         arrHeurScores[nVble].Neg -= 
            pOldState->arrTransitions[j+BOOL_FALSE].pNextState->arrHeuristicXors[nPrevNumRHSUnknowns];
         j+=2;
      }
   }
   // update special function 
   if (pOldState == pTrueSmurfState) {
      fScorePos = 
         fScoreNeg = -pOldState->arrHeuristicXors[nPrevNumRHSUnknowns];
   } else if (nPrevNumRHSUnknowns > 2) {
      fScorePos = 
         fScoreNeg = -pOldState->arrHeuristicXors[nPrevNumRHSUnknowns-1];
   } else {
      int counter=0;
      int nNumElts = arrSolverFunctions[xor_part].fn_xor.rhsVbles.nNumElts;
      int *arrElts = arrSolverFunctions[xor_part].fn_xor.rhsVbles.arrElts;
      for (int j = 0; j < nNumElts; j++)
      {
         int vble = arrElts[j];
         if (arrSolution[vble]!=BOOL_UNKNOWN &&
               arrSolution[vble] == arrSolverFunctions[xor_part].fn_xor.arrRHSPolarities[j])
            counter++;
      }
      neg_idx = (arrSolverFunctions[xor_part].fn_xor.arrRHSPolarities[0]+counter) % 2;
      fScorePos = -pOldState->arrHeuristicXors[1-neg_idx]; 
      fScoreNeg = -pOldState->arrHeuristicXors[neg_idx];
   }

   // can't return because the XOR part of the BROKEN-XOR-SMURF might not be true YET
   //if (pState == pTrueSmurfState) return;
   
   // add heuristic influence 
   arrElts  = pState->vbles.arrElts;
   j=0;
   if (nPrevNumRHSUnknowns == 0) {
      for (k = 0; k < pState->vbles.nNumElts; k++) {
         int nVble = arrElts[k];
         arrHeurScores[nVble].Pos += 
            pState->arrTransitions[j+BOOL_TRUE].fHeuristicWeight;
         arrHeurScores[nVble].Neg += 
            pState->arrTransitions[j+BOOL_FALSE].fHeuristicWeight;

         j+=2;
      }
   } else {
      // nPrevNumRHSUnknowns > 0 
      for (k = 0; k < pState->vbles.nNumElts; k++) {
         int nVble = arrElts[k];
         arrHeurScores[nVble].Pos += 
            pState->arrTransitions[j+BOOL_TRUE].pNextState->arrHeuristicXors[nPrevNumRHSUnknowns];
         arrHeurScores[nVble].Neg += 
            pState->arrTransitions[j+BOOL_FALSE].pNextState->arrHeuristicXors[nPrevNumRHSUnknowns];
         j+=2;
      }
   }

   // update special function 
   if (pState == pTrueSmurfState) {
      fScorePos += pState->arrHeuristicXors[nPrevNumRHSUnknowns];
      fScoreNeg += pState->arrHeuristicXors[nPrevNumRHSUnknowns];
   } else if (nPrevNumRHSUnknowns > 2) {
      fScorePos += pState->arrHeuristicXors[nPrevNumRHSUnknowns-1];
      fScoreNeg += pState->arrHeuristicXors[nPrevNumRHSUnknowns-1];
   } else {
      fScorePos += pState->arrHeuristicXors[1-neg_idx]; 
      fScoreNeg += pState->arrHeuristicXors[neg_idx];
   }
   J_Update_RHS_AND(arrSolverFunctions[xor_part].fn_xor.rhsVbles.nNumElts, arrSolverFunctions[xor_part].fn_xor.rhsVbles.arrElts, arrSolverFunctions[xor_part].fn_xor.arrRHSPolarities, fScorePos, fScoreNeg);
}

