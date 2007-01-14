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
LSGBSmurfGetHeurScores(int nFnId)
{
   LSGBSmurfSetHeurScores(nFnId, arrSolverFunctions[nFnId].fn_smurf.pInitialState);

   // Get a ptr to the Smurf state.
   SmurfState *pState = arrSolverFunctions[nFnId].fn_smurf.pInitialState;
                     
   // Do nothing if constraint is trivial.
   if (pState == pTrueSmurfState) return;

   int *arrElts = pState->vbles.arrElts;
   int j=0;
   int k;
   for (k = 0; k < pState->vbles.nNumElts; k++) {
      int nVble = arrElts[k];
      arrHeurScores[nVble].Pos += pState->arrTransitions[j+BOOL_TRUE].fHeuristicWeight;
      arrHeurScores[nVble].Neg += pState->arrTransitions[j+BOOL_FALSE].fHeuristicWeight;
      j+=2;
   }
}

ITE_INLINE void
LSGBSmurfUpdateFunctionInfEnd(int nFnId) {
   SmurfState *pOldState = arrSolverFunctions[nFnId].fn_smurf.pPrevState; 
   SmurfState *pState = arrSolverFunctions[nFnId].fn_smurf.pCurrentState;
   int k,j;
   int *arrElts;

   // remove heuristic influence 
   arrElts  = pOldState->vbles.arrElts;
   j=0;
   for (k = 0; k < pOldState->vbles.nNumElts; k++) {
      int nVble = arrElts[k];
      Save_arrHeurScores(nVble);
      arrHeurScores[nVble].Pos -= pOldState->arrTransitions[j+BOOL_TRUE].fHeuristicWeight;
      arrHeurScores[nVble].Neg -= pOldState->arrTransitions[j+BOOL_FALSE].fHeuristicWeight;
      j+=2;
   }

   if (pState == pTrueSmurfState) return;
   
   // add heuristic influence 
   arrElts  = pState->vbles.arrElts;
   j=0;
   for (k = 0; k < pState->vbles.nNumElts; k++) {
      int nVble = arrElts[k];
      arrHeurScores[nVble].Pos += pState->arrTransitions[j+BOOL_TRUE].fHeuristicWeight;
      arrHeurScores[nVble].Neg += pState->arrTransitions[j+BOOL_FALSE].fHeuristicWeight;
      pState->arrTransitions[j+BOOL_TRUE].pNextState->cVisited |= 2;
      pState->arrTransitions[j+BOOL_FALSE].pNextState->cVisited |= 2;
      j+=2;
   }
}

