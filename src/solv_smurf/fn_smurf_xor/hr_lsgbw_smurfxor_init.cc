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

#include "sbsat.h"
#include "sbsat_solver.h"
#include "solver.h"

void
HrLSGBWFnSmurfInit()
{
   for(int i=0;i<MAX_FUNC;i++)
   {
      // can reuse LSGB Update instead of LSGBW
      procHeurUpdateFunctionInfEnd[i] = LSGBSmurfUpdateFunctionInfEnd; 
      procHeurGetScores[i] = LSGBWSmurfGetHeurScores;
   }
}
/*
ITE_INLINE double
LSGBSmurfGetHeurWeight(SmurfState *pState, int i, int nVble, int nValue)
   // Returns the heuristic weight of a transition.
{
   Transition *pTransition;

   if (pState == pTrueSmurfState) return 0;
   assert(i<pState->vbles.nNumElts && pState->vbles.arrElts[i] == nVble);
   assert(pState->arrTransitions != 0);
   if ((pTransition=FindTransition(pState, i, nVble, nValue))==NULL) return 0;

   return pTransition->fHeuristicWeight;
}
*/

ITE_INLINE double
LSGBWSumInferenceWeights(Transition *pTransition)
{
  int i=0;
  double fSum = 0;


  for(i=0;i<pTransition->positiveInferences.nNumElts;i++)
    fSum += arrJWeights[pTransition->positiveInferences.arrElts[i]];

  for(i=0;i<pTransition->negativeInferences.nNumElts;i++)
    fSum += arrJWeights[pTransition->negativeInferences.arrElts[i]];

  return fSum;
}

ITE_INLINE double
LSGBWSetHeurScoreTransition(SmurfState *pState, int i, Transition *pTransition, int nRegSmurfIndex, int nNumXors, int polarity)
{
   double fTransitionWeight = LSGBWSumInferenceWeights(pTransition) +
                          pTransition->pNextState->fNodeHeuristicWeight;
   pTransition->fHeuristicWeight = fTransitionWeight;
/*
   if (nNumXors) {
        if (pState->vbles.arrElts[i] != arrSolverFunctions[nRegSmurfIndex].fn_smurf.nSmurfEqualityVble) {
           for (int j=3;j<nNumXors;j++) {
                  pState->arrHeuristicXors[j] += pTransition->pNextState->arrHeuristicXors[j];
        }
      } else {
        pState->arrHeuristicXors[polarity] = pTransition->fHeuristicWeight;
      }
   }
   */
   return fTransitionWeight;
}

/*
ITE_INLINE void
J_SetHeurScoresForSmurfs_Counting(int nRegSmurfIndex, SmurfState *pState, int nNumXors)
{
   if (pState == pTrueSmurfState) {
      return;
   }

   // FIND OUT IF THE HEUR ALREADY COMPUTED 
   if (pState->cFlag == 2) return;
   pState->cFlag = 2;

   double fTotalCount = 0;

   for (int i=0;i<pState->vbles.nNumElts;i++)
   {
      // ----- POSITIVE TRANSITIONS ------ 
      {
         Transition *pTransition = FindTransition(pState, i, pState->vbles.arrElts[i], BOOL_TRUE);
         J_SetHeurScoresForSmurfs_Counting(nRegSmurfIndex, pTransition->pNextState, nNumXors);
         fTotalCount += pTransition->pNextState->fNodeHeuristicWeight;
      }

      // ----- NEGATIVE TRANSITIONS ------ 
      {
         Transition *pTransition = FindTransition(pState, i, pState->vbles.arrElts[i], BOOL_FALSE);
         J_SetHeurScoresForSmurfs_Counting(nRegSmurfIndex, pTransition->pNextState, nNumXors);
         fTotalCount += pTransition->pNextState->fNodeHeuristicWeight;
      }
   }

   pState->fNodeHeuristicWeight = 1+fTotalCount;

   for (int i=0;i<pState->vbles.nNumElts;i++)
   {
      Transition *pTransition;
      pTransition = FindTransition(pState, i, pState->vbles.arrElts[i], BOOL_TRUE);
      pTransition->fHeuristicWeight = 
         pState->fNodeHeuristicWeight - pTransition->pNextState->fNodeHeuristicWeight;

      pTransition = FindTransition(pState, i, pState->vbles.arrElts[i], BOOL_FALSE);
      pTransition->fHeuristicWeight =
         pState->fNodeHeuristicWeight - pTransition->pNextState->fNodeHeuristicWeight;
   }
}
*/

ITE_INLINE void
LSGBWSmurfSetHeurScores(int nRegSmurfIndex, SmurfState *pState, int nNumXors)
{
   if (pState == pTrueSmurfState) {
      /*
      if (pTrueSmurfState->nNumHeuristicXors < nNumXors)
          J_SetHeurWeightsTrueState(nNumXors);
          */
      return;
   }

   // FIND OUT IF THE HEUR ALREADY COMPUTED 
   if (pState->cFlag == 2 && pState->nNumHeuristicXors >= nNumXors) return;
   pState->cFlag = 2;
/*
   if (pState->nNumHeuristicXors < nNumXors) {
      ite_free((void**)&pState->arrHeuristicXors);
      pState->arrHeuristicXors = (double*)ite_calloc(nNumXors, sizeof(double),
            9, "pState->arrHeuristicXors");
      pState->nNumHeuristicXors = nNumXors;
   }
   */

   double fTotalTransitions  = 0;
   for (int i=0;i<pState->vbles.nNumElts;i++)
   {
      /* ----- POSITIVE TRANSITIONS ------ */
      {
         Transition *pTransition = FindTransition(pState, i, pState->vbles.arrElts[i], BOOL_TRUE);
         LSGBWSmurfSetHeurScores(nRegSmurfIndex, pTransition->pNextState, nNumXors);
         fTotalTransitions  += LSGBWSetHeurScoreTransition(pState, i, pTransition, nRegSmurfIndex, nNumXors, 1);
      }

      /* ----- NEGATIVE TRANSITIONS ------ */
      {
         Transition *pTransition = FindTransition(pState, i, pState->vbles.arrElts[i], BOOL_FALSE);
         LSGBWSmurfSetHeurScores(nRegSmurfIndex, pTransition->pNextState, nNumXors);
         fTotalTransitions  += LSGBWSetHeurScoreTransition(pState, i, pTransition, nRegSmurfIndex, nNumXors, 0);
      }
   }

   pState->fNodeHeuristicWeight = fTotalTransitions / (pState->vbles.nNumElts * 2 * JHEURISTIC_K);
/*
   if (nNumXors) {
      // only for base case 
      pState->arrHeuristicXors[2] = pState->fNodeHeuristicWeight;
      for (int j=3;j<nNumXors;j++) {
          pState->arrHeuristicXors[j] += (j-1)*2* pState->arrHeuristicXors[j-1];
          pState->arrHeuristicXors[j] /= (2.0 * (pState->vbles.nNumElts-1+j-1) * JHEURISTIC_K);
      }
   }
   */

}

/*
ITE_INLINE void
J_SetHeuristicScoresForSmurfXor(int nFnId)
{
   int nRHSXorVbles = 0;
   int specfn = arrSmurfChain[i].specfn;
   if (specfn != -1)  nRHSXorVbles = arrSpecialFuncs[specfn].rhsVbles.nNumElts+1;
   if (sHeuristic[1] == 'C') {
      assert(nRHSXorVbles == 0);
      J_SetHeurScoresForSmurfs_Counting(i, arrRegSmurfInitialStates[i], nRHSXorVbles);
   } else {
      J_SetHeurScoresForSmurfs(i, arrRegSmurfInitialStates[i], nRHSXorVbles);
   }
}

ITE_INLINE void
J_SetHeurWeightsTrueState(int nNumXors)
{
   if (pTrueSmurfState->nNumHeuristicXors < nNumXors) {
      pTrueSmurfState->arrHeuristicXors 
         = (double*)realloc(pTrueSmurfState->arrHeuristicXors, nNumXors*sizeof(double));
      pTrueSmurfState->nNumHeuristicXors = nNumXors;
      pTrueSmurfState->arrHeuristicXors[0] = 0;       
      pTrueSmurfState->arrHeuristicXors[1] = 0;      
      pTrueSmurfState->arrHeuristicXors[2] = 1;
      for(int j=3;j<nNumXors;j++) {
         pTrueSmurfState->arrHeuristicXors[j] = arrXorEqWght[j-1];
      }
   };
}


*/

