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

ITE_INLINE int
CheckSmurfInferences(int nSmurfIndex, int *arrInferences, int nNumInferences, 
      LemmaBlock **arrInferenceLemmas, int value)
{
   for (int j = 0; j < nNumInferences; j++)
   {
      int nNewInferredAtom = arrInferences[j];
      int nCurrentAtomValue = arrSolution[nNewInferredAtom];

      if (nCurrentAtomValue == value) continue;

      LemmaBlock *pLemma = NULL;
      LemmaInfoStruct *pLemmaInfo = NULL;
      if (arrInferenceLemmas == NULL) {
         if (NO_LEMMAS == 0) {
            /* create lemma */
            arrSmurfPath[nSmurfIndex].literals[arrSmurfPath[nSmurfIndex].idx] 
               = nNewInferredAtom * (value==BOOL_FALSE?-1:1);

#ifdef MATCH_ORIGINAL_LEMMA_ORDER
            int *arrLits = (int*)ite_calloc(arrSmurfPath[nSmurfIndex].idx+1, sizeof(int),
                  9, "arrLits");
            for (int i=0; i<arrSmurfPath[nSmurfIndex].idx+1;i++)
               arrLits[arrSmurfPath[nSmurfIndex].idx+1-i-1] = arrSmurfPath[nSmurfIndex].literals[i];

            pLemmaInfo=AddLemma(arrSmurfPath[nSmurfIndex].idx+1,
                  arrLits/*arrSmurfPath[nSmurfIndex].literals*/,
                  false, NULL, NULL);
            pLemma = pLemmaInfo->pLemma;
            free(arrLits);
#else
            pLemmaInfo=AddLemma(arrSmurfPath[nSmurfIndex].idx+1,
                  arrSmurfPath[nSmurfIndex].literals, false, NULL, NULL);
            pLemma = pLemmaInfo->pLemma;
#endif
            //DisplayLemmaStatus(pLemma, arrSolution);
         } else {
            //pLemma = NULL;
            //pLemmaInfo = NULL;
         }
      } else {
         pLemma = arrInferenceLemmas[j];
         //pLemmaInfo = NULL;
      }

      if (nCurrentAtomValue == BOOL_UNKNOWN)
      {
         ite_counters[INF_SMURF]++;
         InferLiteral(nNewInferredAtom, value, false, pLemma, pLemmaInfo, 1);
      }
      else // if (nCurrentAtomValue != value)
      {
         // Conflict -- backtrack.
         TB_9(
               d9_printf1("Conflict:  goto Backtrack\n");
            )

         pConflictLemma = pLemma;
         pConflictLemmaInfo = pLemmaInfo; /* so we can free it */
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

   d9_printf2("Visiting Regular Smurf #%d\n", nSmurfIndex);

   pState = arrCurrentStates[nSmurfIndex];

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
      assert(pTransition);
      assert(pTransition->pNextState != pState);

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
            TB_9(
                  d9_printf3("Decremented nNumUnresolvedFunctions to %d due to smurf # %d\n",
                     nNumUnresolvedFunctions, nSmurfIndex);
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

