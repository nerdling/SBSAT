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
#include "solver.h"

ITE_INLINE void
J_Update_RHS_AND(SpecialFunc * pSpecialFunc, HWEIGHT fPosDelta, HWEIGHT fNegDelta);

ITE_INLINE int
CheckSmurfInferences(int nSmurfIndex, int *arrInferences, int nNumInferences, 
      LemmaBlock **arrInferenceLemmas, int value)
{
   for (int j = 0; j < nNumInferences; j++)
   {
      int nNewInferredAtom = arrInferences[j];
      int nCurrentAtomValue = arrSolution[nNewInferredAtom];

      if (nCurrentAtomValue == value) continue;

      LemmaBlock *pLemma;
      LemmaInfoStruct *pUnitLemmaListTail;
      if (arrInferenceLemmas == NULL) {
         if (NO_LEMMAS == 0) {
            /* create lemma */
            arrSmurfPath[nSmurfIndex].literals[arrSmurfPath[nSmurfIndex].idx] 
               = nNewInferredAtom * (value==BOOL_FALSE?-1:1);

            int *arrLits = (int*)ite_calloc(arrSmurfPath[nSmurfIndex].idx+1, sizeof(int),
                  9, "arrLits");
            for (int i=0; i<arrSmurfPath[nSmurfIndex].idx+1;i++)
               arrLits[arrSmurfPath[nSmurfIndex].idx+1-i-1] = arrSmurfPath[nSmurfIndex].literals[i];

            pUnitLemmaListTail = NULL;
            pUnitLemmaList->pNextLemma[0] = NULL;
            pLemma=AddLemma(arrSmurfPath[nSmurfIndex].idx+1,
                  arrLits/*arrSmurfPath[nSmurfIndex].literals*/,
                  false,
                  pUnitLemmaList, //m lemma is added in here 
                  pUnitLemmaListTail //m and here 
                  );
            pUnitLemmaList->pNextLemma[0] = NULL;
            free(arrLits);
            /* 
             LemmaBlock *pLemmaLastBlock;
             int nNumBlocks;
             EnterIntoLemmaSpace(arrSmurfPath[nSmurfIndex].idx+1,
             arrSmurfPath[nSmurfIndex].literals,
             true,
             pLemma, pLemmaLastBlock, nNumBlocks);
             */ 

            //DisplayLemmaStatus(pLemma, arrSolution);
         } else {
            pLemma = NULL;
            pUnitLemmaListTail = NULL;
         }
      } else {
         pLemma = arrInferenceLemmas[j];
         pUnitLemmaListTail = NULL;
      }

      if (nCurrentAtomValue == BOOL_UNKNOWN)
      {
         ite_counters[INF_SMURF]++;
         InferLiteral(nNewInferredAtom, value, false, pLemma, pUnitLemmaListTail, 1);
      }
      else // if (nCurrentAtomValue != value)
      {
         // Conflict -- backtrack.
         D_9(
               if (nNumBacktracks >= TRACE_START)
               {
               d9_printf1("Conflict:  goto Backtrack\n");
               }
            )

         pConflictLemma = pLemma;
         pConflictLemmaInfo = pUnitLemmaListTail; /* so we can free it */
         //goto_Backtrack;
         return ERR_BT_SMURF;
      }
   }
   return 0;
}


extern int *arrChangedSmurfs;

inline
ITE_INLINE int 
UpdateRegularSmurf(int nSmurfIndex)
{
   SmurfState *pState;
   SmurfState *pOldState;

      d9_printf2("Visiting Regular Smurf #%d\n", nSmurfIndex);

      pOldState = pState = arrCurrentStates[nSmurfIndex];

      while(1) 
      {
         int vble = 0, k;
         int nNumElts = pState->vbles.nNumElts;
         int *arrElts = pState->vbles.arrElts;
         for (k = 0; k < nNumElts; k++) 
         {
            vble = arrElts[k];
            if (arrSolution[vble]!=BOOL_UNKNOWN)  break;
         }

         /* if no more instantiated variables */
         if (k == nNumElts) break;

         //Save_arrCurrentStates(nSmurfIndex);

         if (compress_smurfs && NO_LEMMAS == 0) {
            // keep track of the path for lemmas
            // the atoms in the path is reversed for lemmas
            arrSmurfPath[nSmurfIndex].literals[arrSmurfPath[nSmurfIndex].idx++] 
               = arrElts[k] * (arrSolution[vble]==BOOL_FALSE?1:-1);
         }

         // Get the transition.
         Transition *pTransition = FindTransition(pState, k, vble, arrSolution[vble]);
         assert (pTransition);

         if (pTransition->positiveInferences.nNumElts &&
               CheckSmurfInferences(
                  nSmurfIndex,
                  pTransition->positiveInferences.arrElts,
                  pTransition->positiveInferences.nNumElts,
                  pTransition->arrPosInferenceLemmas,
                  BOOL_TRUE)) return ERR_BT_SMURF;

         if (pTransition->negativeInferences.nNumElts &&
               CheckSmurfInferences(
                  nSmurfIndex,
                  pTransition->negativeInferences.arrElts,
                  pTransition->negativeInferences.nNumElts,
                  pTransition->arrNegInferenceLemmas,
                  BOOL_FALSE)) return ERR_BT_SMURF;

         arrCurrentStates[nSmurfIndex] = 
            pState = pTransition->pNextState;
         if (pState == pTrueSmurfState)
         {
            nNumUnresolvedFunctions--;
            D_9(
                  if (nNumBacktracks >= TRACE_START)
                  {
                  d9_printf3("Decremented nNumUnresolvedFunctions to %d due to smurf # %d\n",
                     nNumUnresolvedFunctions, nSmurfIndex);
                  }
               )
               break; // break the for all vbles in smurf loop
         }

      } // while (1) there is a instantiated variable

   return NO_ERROR;
}

ITE_INLINE int
UpdateEachAffectedRegularSmurf(AffectedFuncsStruct *pAFS)
{
   int nSmurfIndex;

   // Determine the potentially affected regular Smurfs.
   int nNumRegSmurfsAffected = pAFS->nNumRegSmurfsAffected;
   int *arrRegSmurfsAffected = pAFS->arrRegSmurfsAffected;

   // Update each affected regular Smurf.
   for (int i = 0; i < nNumRegSmurfsAffected; i++)
   {
      nSmurfIndex = arrRegSmurfsAffected[i];
      if ((arrChangedSmurfs[nSmurfIndex]&1) == 0) continue;

      // Update the Smurf state.
      arrChangedSmurfs[nSmurfIndex] = 2;

      if (arrCurrentStates[nSmurfIndex] == pTrueSmurfState) continue;

      int ret = UpdateRegularSmurf(nSmurfIndex);
      if (ret != NO_ERROR) return ret;
      
   } // for each affected regular Smurf

   return NO_ERROR;
}


ITE_INLINE void
J_UpdateHeuristicSmurf(SmurfState *pOldState, SmurfState *pState, int nSmurfIndex) {
   int k,j;
   int *arrElts;
   int n;
   int neg_idx;
   SpecialFunc *pSpecialFunc;
   HWEIGHT fScorePos = 0;
   HWEIGHT fScoreNeg = 0;

   // remove heuristic influence 
   arrElts  = pOldState->vbles.arrElts;
   j=0;
   if (arrSmurfChain[nSmurfIndex].specfn == -1) {
      for (k = 0; k < pOldState->vbles.nNumElts; k++) {
         int nVble = arrElts[k];
         Save_arrHeurScores(nVble);
         arrHeurScores[nVble].Pos -= pOldState->arrTransitions[j+BOOL_TRUE].fHeuristicWeight;
         arrHeurScores[nVble].Neg -= pOldState->arrTransitions[j+BOOL_FALSE].fHeuristicWeight;
         j+=2;
      }
   } else {
      pSpecialFunc = arrSpecialFuncs + arrSmurfChain[nSmurfIndex].specfn;
      n = arrPrevNumRHSUnknowns[arrSmurfChain[nSmurfIndex].specfn];
      if (n == 0)  {
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
         for (k = 0; k < pOldState->vbles.nNumElts; k++) {
            int nVble = arrElts[k];
            Save_arrHeurScores(nVble);
            arrHeurScores[nVble].Pos -= 
               pOldState->arrTransitions[j+BOOL_TRUE].pNextState->arrHeuristicXors[n];
            arrHeurScores[nVble].Neg -= 
               pOldState->arrTransitions[j+BOOL_FALSE].pNextState->arrHeuristicXors[n];
            j+=2;
         }
      }
      // update special function 
      if (pOldState == pTrueSmurfState) {
         fScorePos = 
         fScoreNeg = -pOldState->arrHeuristicXors[n];
      } else if (n > 2) {
         fScorePos = 
         fScoreNeg = -pOldState->arrHeuristicXors[n-1];
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
         neg_idx = (pSpecialFunc->arrRHSPolarities[0]+counter) % 2;
         fScorePos = -pOldState->arrHeuristicXors[1-neg_idx]; 
         fScoreNeg = -pOldState->arrHeuristicXors[neg_idx];
      }
   }

   // if (pState == pTrueSmurfState) return;

   // add heuristic influence 
   arrElts  = pState->vbles.arrElts;
   j=0;
   if (arrSmurfChain[nSmurfIndex].specfn == -1) {
      for (k = 0; k < pState->vbles.nNumElts; k++) {
         int nVble = arrElts[k];
         arrHeurScores[nVble].Pos+= pState->arrTransitions[j+BOOL_TRUE].fHeuristicWeight;
         arrHeurScores[nVble].Neg += pState->arrTransitions[j+BOOL_FALSE].fHeuristicWeight;
         j+=2;
      }
   } else {
      if (n == 0) {
         for (k = 0; k < pState->vbles.nNumElts; k++) {
            int nVble = arrElts[k];
            arrHeurScores[nVble].Pos += 
               pState->arrTransitions[j+BOOL_TRUE].fHeuristicWeight;
            arrHeurScores[nVble].Neg += 
               pState->arrTransitions[j+BOOL_FALSE].fHeuristicWeight;
            j+=2;
         }
      } else {
         for (k = 0; k < pState->vbles.nNumElts; k++) {
            int nVble = arrElts[k];
            arrHeurScores[nVble].Pos += 
               pState->arrTransitions[j+BOOL_TRUE].pNextState->arrHeuristicXors[n];
            arrHeurScores[nVble].Neg += 
               pState->arrTransitions[j+BOOL_FALSE].pNextState->arrHeuristicXors[n];
            j+=2;
         }
      }

      // update special function 
      if (pState == pTrueSmurfState) {
         fScorePos += pState->arrHeuristicXors[n];
         fScoreNeg += pState->arrHeuristicXors[n];
      } else if (n > 2) {
         fScorePos += pState->arrHeuristicXors[n-1];
         fScoreNeg += pState->arrHeuristicXors[n-1];
      } else {
         fScorePos += pState->arrHeuristicXors[1-neg_idx]; 
         fScoreNeg += pState->arrHeuristicXors[neg_idx];
      }
      J_Update_RHS_AND(pSpecialFunc, fScorePos, fScoreNeg);
   }
}

