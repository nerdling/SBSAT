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

ITE_INLINE int
BackTrack()
{
   int nOldBacktrackStackIndex=0;
   LemmaInfoStruct *pUnitLemmaListTail;
   LemmaInfoStruct pUnitLemmaList;
   int nUnsetLemmaFlagIndex = 0; /* literals that we need to unset after bt */
   LemmaBlock *pNewLemma=NULL; /* last lemma added */
   int nTempLemmaIndex = 0; /* the length of the new/temporary lemma */
   int nNumForcedInfsBelowCurrentCP = 0;
   int nInferredAtom = 0; /* the choice point */
   int nInferredValue = 0; /* the choice point value */
   int _num_backjumps = 0; /* num of backjumps during this backtrack */

	pUnitLemmaList.pNextLemma[0] = NULL;
	pUnitLemmaListTail = NULL;
	
   // Copy the conflict lemma into arrTempLemma.
   nTempLemmaIndex = ConstructTempLemma();

   /* backjumping loop */
   do {

//#define MK_DISCOUNT_BACKJUMPS
#ifdef MK_DISCOUNT_BACKJUMPS
      if (nInferredAtom != 0) {
         /* not that great since the values get restored during backtrack */
         arrHeurScores[nInferredAtom].Pos /= 2;
         arrHeurScores[nInferredAtom].Neg /= 2;
      }
#endif
      if (nInferredAtom != 0) 
         var_stat[nInferredAtom].backjumped[nInferredValue]++;

		if(pChoicePointTop->pUnitLemmaList != NULL) {
			if(pUnitLemmaList.pNextLemma[0] == NULL) {
				pUnitLemmaList.pNextLemma[0] = pChoicePointTop->pUnitLemmaList;
				pUnitLemmaListTail = pChoicePointTop->pUnitLemmaListTail;
			} else {
				pUnitLemmaListTail->pNextLemma[0] = pChoicePointTop->pUnitLemmaList;
				pUnitLemmaListTail = pChoicePointTop->pUnitLemmaListTail;
				assert(pChoicePointTop->pUnitLemmaListTail);
			}
			pChoicePointTop->pUnitLemmaList = NULL;
			pChoicePointTop->pUnitLemmaListTail = NULL;
		}
		
      // Pop the choice point stack.
      pChoicePointTop--;

      if (pChoicePointTop < pStartChoicePointStack)
      {
         // return ERR_
         return 1;
      }
      nInferredAtom = pChoicePointTop->nBranchVble;

      /*
       * reconstruct nNumForcedInfsBelowCurrentCP
       */
      /* cp1 inf1_1 inf1_2 inf1_3 cp2 inf2_1 inf2_2 inf2_3 cp3 ... cp4 ... cp5 */
      nNumForcedInfsBelowCurrentCP = 0;
      for (int i=0; i<nTempLemmaIndex;i++) {
         int nConflictVble = abs(arrTempLemma[i]);
         /* for all lower than current cp */
         /* bigger than cpX .... */
         if(arrBacktrackStackIndex[nConflictVble] >
               arrBacktrackStackIndex[nInferredAtom] && (
                  /* and just those that I'm going to hit for sure on this level */
                  /* lower than last cp  =>  ... cp(X-1) */
                  nOldBacktrackStackIndex == 0 ||
                  arrBacktrackStackIndex[nConflictVble] < nOldBacktrackStackIndex))
            nNumForcedInfsBelowCurrentCP++;
      }
      nOldBacktrackStackIndex = arrBacktrackStackIndex[nInferredAtom];

      // Pop the num unresolved functions 
      nNumUnresolvedFunctions = pChoicePointTop->nNumUnresolved;

      pop_state_information(1);

      // Pop the backtrack stack until we pop the branch atom.
      // backtracking loop
      do {
         pBacktrackTop--;
         nBacktrackStackIndex--;
         assert(nBacktrackStackIndex >= 0);

         /*m invalidating arrBacktrackStackIndex but keep the prev value */
         int nBacktrackAtom = pBacktrackTop->nBranchVble;
         int nBacktrackAtomStackIndex = arrBacktrackStackIndex[nBacktrackAtom];
         arrBacktrackStackIndex[nBacktrackAtom] = gnMaxVbleIndex + 1;

         if (pBacktrackTop->pLemmaInfo && 
               pBacktrackTop->pLemmaInfo->nBacktrackStackReferences > 0)
         {
            // The lemma for this backtrack stack entry was a cached lemma.
            // Decrement the reference count for it.
            // (When the count reaches zero, the lemma can be recycled.)
            (pBacktrackTop->pLemmaInfo->nBacktrackStackReferences)--;

               // Move lemma to front of lemma priority queue.
#define MK_UP_RELEVANT_LEMMAS
#ifdef MK_UP_RELEVANT_LEMMAS
            if(arrLemmaFlag[nBacktrackAtom]) 
#endif
               MoveToFrontOfLPQ(pBacktrackTop->pLemmaInfo);

            pBacktrackTop->pLemmaInfo = NULL;
         }

         //Handle old choicepoints
         /*m is atom relevant */
         if(arrLemmaFlag[nBacktrackAtom]) {
            nNumForcedInfsBelowCurrentCP--;
         }

         if (pBacktrackTop->bWasChoicePoint == true || 
               nNumForcedInfsBelowCurrentCP == 0 ||
               nBacktrackAtom == nInferredAtom) 
         {
            //nBacktrackAtom is a UIP-unique implication point
				pBacktrackTop->bWasChoicePoint = false;
				
            if (arrLemmaFlag[nBacktrackAtom]) 
				{
               // Resolve out lemma literals which are not needed.
               // Do this by checking the level at which each literal
               // was inferred.
               int nTempLemmaLiteral;
               int nTempLemmaVble;
					
               int nCopytoIndex = 0;
               for (int i = 0; i < nTempLemmaIndex; i++)
               {
                  nTempLemmaLiteral = arrTempLemma[i];
                  nTempLemmaVble = abs(nTempLemmaLiteral);
                  if ((arrBacktrackStackIndex[nTempLemmaVble] 
                           <= nBacktrackAtomStackIndex) || 
                        (nTempLemmaVble == nBacktrackAtom))
                  {
                     arrTempLemma[nCopytoIndex++] = nTempLemmaLiteral;
                  }
                  else
                     arrUnsetLemmaFlagVars[nUnsetLemmaFlagIndex++] 
                        = nTempLemmaVble;
               }
               nTempLemmaIndex = nCopytoIndex;

               if (nTempLemmaIndex <= 0)
               {
                  // Lemma of length zero. The problem is unsat.
                  cout << "1: Lemma of length zero." << endl;
                  return 1; // goto_NoSolution;
               }

               bool bFlag = (MAX_NUM_CACHED_LEMMAS > 0);

               assert(IsInLemmaList(pUnitLemmaListTail,
                        &pUnitLemmaList));	  

               pNewLemma=AddLemma(nTempLemmaIndex,
                     arrTempLemma,
                     bFlag,
                     &pUnitLemmaList, /*m lemma is added in here */
                     &pUnitLemmaListTail /*m and here */
                     )->pLemma;

            }

            /*m while (1) - the end of backtrack */
            if(nBacktrackAtom == nInferredAtom) break; 
         } // if(pBacktrackTop->bWasChoicePoint == true || ... )
			
#ifdef DISPLAY_TRACE
         TB_9(
            cout << "Examining lemma:" << endl;
            DisplayLemmaStatus(pBacktrackTop->pLemma);
            cout << "which witnesses inference X"
               << pBacktrackTop->nBranchVble << " = "
               << arrSolution[pBacktrackTop->nBranchVble]
               << endl;
         )
#endif

         // Check whether the atom at the top of the backtrack stack
         // is relevent to the resolution done so far.
         if (arrLemmaFlag[nBacktrackAtom])
         {
            // The backtrack atom is relevant to the resolution done so far.
            // Therefore, include its attached lemma in the resolution process.
#ifdef DISPLAY_TRACE
            TB_9(
               cout << "Lemma relevant to contradiction." << endl;
            )
#endif

            // copy all literals not present in arrTempLemma into arrTempLemma
            // (not marked in arrLemmaFlag)
            LemmaBlock *pLemmaBlock = pBacktrackTop->pLemma;
            int *arrLits = pLemmaBlock->arrLits;
            int nLemmaLength = arrLits[0];
            int nLemmaLiteral;
            int nLemmaVble;

            for (int nLitIndex = 1, nLitIndexInBlock = 1;
                  nLitIndex <= nLemmaLength;
                  nLitIndex++, nLitIndexInBlock++)
            {
               if (nLitIndexInBlock == LITS_PER_LEMMA_BLOCK)
               {
                  nLitIndexInBlock = 0;
                  pLemmaBlock = pLemmaBlock->pNext;
                  arrLits = pLemmaBlock->arrLits;
               }
               nLemmaLiteral = arrLits[nLitIndexInBlock];
               nLemmaVble = abs(nLemmaLiteral);
               if (arrLemmaFlag[nLemmaVble] == false)
               {
                  arrLemmaFlag[nLemmaVble] = true;
                  arrTempLemma[nTempLemmaIndex++] = nLemmaLiteral;
                  if(arrBacktrackStackIndex[nLemmaVble] > arrBacktrackStackIndex[nInferredAtom]) 
                     nNumForcedInfsBelowCurrentCP++;
                  assert(arrSolution[nLemmaVble] 
                        == (nLemmaLiteral > 0 ? BOOL_FALSE : BOOL_TRUE));
               }
            }
         }
#ifdef DISPLAY_TRACE
         else 
            TB_9(
                  cout << "Lemma irrelevant to contradiction." << endl;
                );
         cout << "Star_Count = " << nNumForcedInfsBelowCurrentCP << endl;

         TB_9(
            cout << "Backtracking from forced assignment of X"
               << nBacktrackAtom << " equal to "
               << (arrSolution[nBacktrackAtom] == BOOL_TRUE ? "true" : "false")
               << "." << endl;
         )
#endif

         if (pBacktrackTop->pLemmaInfo) FreeLemma(pBacktrackTop->pLemmaInfo);

         arrSolution[nBacktrackAtom] = BOOL_UNKNOWN;

      } // bactracking loop
      while (1);

      /* remember the value so we can flip it */
      nInferredValue = arrSolution[nInferredAtom];
      arrSolution[nInferredAtom] = BOOL_UNKNOWN;
      _num_backjumps++;
   }  // backjumping loop
   while (arrLemmaFlag[nInferredAtom] == false);

   _num_backjumps--; /*m last backjump was not just backtrack */
   if (_num_backjumps) {
      ite_counters[NUM_TOTAL_BACKJUMPS]+=_num_backjumps;
      ite_counters[NUM_BACKJUMPS]++;
   }

   // Pop heuristic scores.
   if (procHeurBacktrack) procHeurBacktrack(_num_backjumps+1);

   /*m clean up */
   for(int i = 0; i < nUnsetLemmaFlagIndex; i++)
      arrLemmaFlag[arrUnsetLemmaFlagVars[i]] = false;
   for(int i = 0; i < nTempLemmaIndex; i++) 
      arrLemmaFlag[abs(arrTempLemma[i])] = false;

   // Flush the inference queue.
   pInferenceQueueNextElt = pInferenceQueueNextEmpty = arrInferenceQueue;
   pFnInferenceQueueNextElt = pFnInferenceQueueNextEmpty = arrFnInferenceQueue;
   pFnInfQueueUpdate = arrFnInferenceQueue;
   for(int i = 0; i < MAX_FN_PRIORITY; i++) {
      arrFnInfPriority[i].First = NULL;
      arrFnInfPriority[i].Last = NULL;
   }
   nLastFnInfPriority = 0;


   // Reverse the polarity of the branch atom.
   // the value of the choice point 
   // flip it
   nInferredValue = (nInferredValue == BOOL_TRUE ? BOOL_FALSE : BOOL_TRUE);
   InferLiteral(nInferredAtom, nInferredValue, true, pNewLemma, NULL, 1);

	assert(IsInLemmaList(pUnitLemmaListTail,
								&pUnitLemmaList));	  
	  
	// Add inferences from lemmas which became unit through the course of
   // the backtracking.
   int nInferredAtomLevel = arrBacktrackStackIndex[nInferredAtom];

   // A lemma in this UnitLemmaList will reverse the polarity of
   // the branch atom automatically.
   // A lemma here will witnesses that we have to reverse polarity.
   LemmaInfoStruct *previous = pUnitLemmaList.pNextLemma[0];
   assert(previous->nWatchedVble[0] == nInferredAtom);
	for (LemmaInfoStruct *p = previous->pNextLemma[0]; p; p = p->pNextLemma[0])
   {
      //m WatchedVble 1 and 2 have the highest BSI in the whole clause
      //m i.e. if Watched2 is < InferredAtomLevel => Watched1 is inferred
      if(arrBacktrackStackIndex[p->nWatchedVble[1]] >= nInferredAtomLevel || p->nWatchedVble[0] == 0)
      {
         //Remove lemma p from UnitLemmaList and add it into the lemma cache
			previous->pNextLemma[0] = p->pNextLemma[0];
			if (p == pUnitLemmaListTail) pUnitLemmaListTail = previous;

         if (p->bPutInCache) //Add p to the two corresponding lemma lists...
         {
            AddLemmaIntoCache(p);
         }
         else
         {
            // The lemma does not go into the lemma cache.
            // Recycle the lemma immediately.
            FreeLemma(p);
         }
			p = previous;
      }
      else
      {
         //It is the case that no two lemmas in the UnitLemmaList
         //will ever apply the same inference. Thus nWatchedVble[0] always
         //is the index of a variable which has not yet been branched on.
			assert(p->nWatchedVble[1] != nInferredAtom);

         //Apply the inference to nWatchedVble[0]

         assert(arrSolution[p->nWatchedVble[0]] == BOOL_UNKNOWN);

         InferLiteral(p->nWatchedVble[0], p->nWatchedVblePolarity[0],
               false, p->pLemma, NULL, 1);

			previous = previous->pNextLemma[0];
      }
   } // for (LemmaInfoStruct *p = pUnitLemmaList->pNextLemma[0]; ; p = p->pNextLemma[0])
   //cout << "Placing unit lemma list on the stack:" << endl;
   //DisplayLemmaList(pUnitLemmaList);
	
	if (pChoicePointTop >= pStartChoicePointStack) { //We are at the top of the tree
		if(pChoicePointTop->pUnitLemmaList == NULL) {
			pChoicePointTop->pUnitLemmaList = pUnitLemmaList.pNextLemma[0];
			pChoicePointTop->pUnitLemmaListTail = pUnitLemmaListTail;
		} else {
			pChoicePointTop->pUnitLemmaListTail->pNextLemma[0]
			  = pUnitLemmaList.pNextLemma[0];
			if(pUnitLemmaListTail != NULL)
			  pChoicePointTop->pUnitLemmaListTail = pUnitLemmaListTail;
		}
		assert(IsInLemmaList(pChoicePointTop->pUnitLemmaListTail, &pUnitLemmaList));		
	} else assert(pUnitLemmaList.pNextLemma[0]->pNextLemma[0] == NULL);
	
	// Get the consequences of the branch atoms new value.
   return 0; /* NO_ERROR */
}

