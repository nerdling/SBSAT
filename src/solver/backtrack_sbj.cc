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
void SelectNewBranchPoint ();
void push_smurf_states_onto_stack();
void push_special_fn_onto_stack();

/* 
 * input: pConflictLemma (used in ConstructTempLemma )
 * output:
 */

//#define LEMMA_CONSTANT 10.0

//LemmaBlock      * pConflictLemma=NULL;
//LemmaInfoStruct * pConflictLemmaInfo=NULL;
//bool *arrLemmaFlag=NULL;
//int  *arrTempLemma=NULL;

ITE_INLINE
int
BackTrack_SBJ()
{
   int nOldBacktrackStackIndex=0;
   LemmaInfoStruct *pUnitLemmaListTail = NULL;
   LemmaInfoStruct pUnitLemmaList;
   int nUnsetLemmaFlagIndex = 0; /* literals that we need to unset after bt */
   LemmaBlock *pNewLemma=NULL; /* last lemma added */
   int nTempLemmaIndex = 0; /* the length of the new/temporary lemma */
   int nNumForcedInfsBelowCurrentCP = 0;
   int nInferredAtom; /* the choice point */
   int nInferredValue; /* the value of the choice point */
   int _num_backjumps = 0;

	int highest_uip_level = gnMaxVbleIndex + 1;
	int highest_uip_var0 = 0;
	int highest_uip_var1 = 0;

	
   ///assert(pUnitLemmaList.pNextLemma[0] == NULL);
   pUnitLemmaList.pNextLemma[0] = NULL;

   // Copy the conflict lemma into arrTempLemma.
   nTempLemmaIndex = ConstructTempLemma();

   /* backjumping loop */
   do {

      // Pop the choice point stack.
      pChoicePointTop--;

      if (pChoicePointTop < arrChoicePointStack)
      {
         // return ERR_
         return 1;
      }
		
      nInferredAtom = pChoicePointTop->nBranchVble;
      nInferredValue = arrSolution[nInferredAtom];

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

      // Pop heuristic scores.
      if (nHeuristic == JOHNSON_HEURISTIC || nHeuristic == STATE_HEURISTIC)
         J_PopHeuristicScores();

      // Pop the backtrack stack until we pop the branch atom.
      while (1)
      {
         pBacktrackTop--;
         nBacktrackStackIndex--;
         assert(nBacktrackStackIndex >= 0);

         if (pBacktrackTop->pLemmaInfo && 
               pBacktrackTop->pLemmaInfo->nBacktrackStackReferences > 0)
         {
            // The lemma for this backtrack stack entry was a cached lemma.
            // Decrement the reference count for it.
            // (When the count reaches zero, the lemma can be recycled.)
            (pBacktrackTop->pLemmaInfo->nBacktrackStackReferences)--;

            // Move lemma to front of lemma priority queue.
            MoveToFrontOfLPQ(pBacktrackTop->pLemmaInfo);

            pBacktrackTop->pLemmaInfo = NULL;
         }

         /*m invalidating arrBacktrackStackIndex but keep the prev value */
         int nBacktrackAtom = pBacktrackTop->nAtom;
         int nBacktrackAtomStackIndex = arrBacktrackStackIndex[nBacktrackAtom];
         arrBacktrackStackIndex[nBacktrackAtom] = gnMaxVbleIndex + 1;


         //Handle old choicepoints
         /*m is atom relevant */
         if(arrLemmaFlag[nBacktrackAtom]) {
            nNumForcedInfsBelowCurrentCP--;
         }

         if (pBacktrackTop->bWasChoicePoint == true || 
			//if(pBacktrackTop->bWasChoicePoint != true && 
				(nNumForcedInfsBelowCurrentCP == 0 ||
				 nBacktrackAtom == nInferredAtom))
         {
            //nBacktrackAtom is a UIP-unique implication point
            if (pBacktrackTop->bWasChoicePoint == false) {
               pBacktrackTop->pUnitLemmaList = NULL;
            }

            pBacktrackTop->bWasChoicePoint = false;

				//s Do this only if we need more lemmas. ie. # of uip lemmas
				//  found so far is less than uip_lemma_max (or something)
				//  Or do this if we are at the choicepoint?
            //  Maybe once i hit the limit of uip lemmas I can speed
				//  through the backtrack loop to the choiepoint without
				//  worrying about doing all the resolutions?
            if (arrLemmaFlag[nBacktrackAtom])
            {
               // Resolve out lemma literals which are not needed.
               // Do this by checking the level at which each literal
               // was inferred.
               int nTempLemmaLiteral;
               int nTempLemmaVble;

               int nLemmaScore=0;  //Used for scoring lemmas

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
                     nLemmaScore += arrBacktrackStackIndex[nTempLemmaVble];
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
                  return 1; // return ERR_
               }

               /* COMPUTE nLemmaScore where lemmas with lower level literal 
                * are WORSE!!! 
                */ 

               bool bFlag = false;
               if (MAX_NUM_CACHED_LEMMAS) bFlag = true;
               /*
                if (MAX_NUM_CACHED_LEMMAS && nBacktrackStackIndex > 0)
                {
                double fLemmaScore = 2.0 * nLemmaScore/
                (nBacktrackStackIndex*(nBacktrackStackIndex+1));

                if (fLemmaScore < LEMMA_CONSTANT) bFlag = true;
                }
                */

               assert(IsInLemmaList(pUnitLemmaListTail,
                        &pUnitLemmaList));	  
					
               pNewLemma=AddLemma(nTempLemmaIndex,
                     arrTempLemma,
                     bFlag,
                     &pUnitLemmaList, /*m lemma is added in here */
                     &pUnitLemmaListTail /*m and here */
                     )->pLemma;

					LemmaInfoStruct *p =	pUnitLemmaList.pNextLemma[0];
					if(p->nWatchedVble[1] == 0) {
						highest_uip_level = arrBacktrackStackIndex[0];
					} else if(arrBacktrackStackIndex[p->nWatchedVble[1]] < highest_uip_level) {
						highest_uip_level = arrBacktrackStackIndex[p->nWatchedVble[1]];
						highest_uip_var0 = p->nWatchedVble[0];
						highest_uip_var1 = p->nWatchedVble[1];
					}
            }

            /*m while (1) - the end of backtrack */
            if(nBacktrackAtom == nInferredAtom) break; 


            if(pBacktrackTop->pUnitLemmaList != NULL) 
            {
					//I do need this cause of backjumping???
					//Maybe i don't need this?
               /*m join pUnitLemmaList with the pBacktrackTop->pUnitLemmaList */
					assert(IsInLemmaList(pBacktrackTop->pUnitLemmaListTail,
                        pBacktrackTop->pUnitLemmaList));
               pBacktrackTop->pUnitLemmaListTail->pNextLemma[0]
                  = pUnitLemmaList.pNextLemma[0];
               pUnitLemmaList.pNextLemma[0] = pBacktrackTop->pUnitLemmaList;
               if(pUnitLemmaListTail == NULL) 
                  pUnitLemmaListTail = pBacktrackTop->pUnitLemmaListTail;
               assert(IsInLemmaList(pUnitLemmaListTail, &pUnitLemmaList));
            }

         } // if(pBacktrackTop->bWasChoicePoint == true || ... )

#ifdef DISPLAY_TRACE
         TB_9(
            cout << "Examining lemma:" << endl;
            DisplayLemmaStatus(pBacktrackTop->pLemma, arrSolution);
            cout << "which witnesses inference X"
               << pBacktrackTop->nAtom << " = "
               << arrSolution[pBacktrackTop->nAtom]
               << endl;
         )
#endif

         // Check whether the atom at the top of the backtrack stack
         // is relevent to the resolution done so far.
         if (arrLemmaFlag[nBacktrackAtom])
         {
            // The backtrack atom is relevant to the resolution done so far.
            // Therefore, include its attached lemma in the resolution process.
# ifdef DISPLAY_TRACE
            TB_9(
                  cout << "Lemma relevant to contradiction." << endl;
                );
# endif


            //m copy all literals not marked in arrLemmaFlag into arrTempLemma
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
                  if(arrBacktrackStackIndex[nLemmaVble] > arrBacktrackStackIndex[nInferredAtom]) nNumForcedInfsBelowCurrentCP++;
                  assert(arrSolution[nLemmaVble] 
                        == (nLemmaLiteral > 0 ? BOOL_FALSE : BOOL_TRUE));
               }
            }
         }
#ifdef DISPLAY_TRACE
         else TB_9(
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
			
         if (pBacktrackTop->pLemmaInfo) {
            FreeLemma(pBacktrackTop->pLemmaInfo, false);
         }
			
         arrSolution[nBacktrackAtom] = BOOL_UNKNOWN;
      };  //  while (1)

      arrSolution[nInferredAtom] = BOOL_UNKNOWN;
      _num_backjumps++;

	}  // backjumping loop
   while (arrLemmaFlag[nInferredAtom] == false);

   _num_backjumps--; /*m last backjump was not just backtrack */

   /*m clean up */
   for(int i = 0; i < nUnsetLemmaFlagIndex; i++)
      arrLemmaFlag[arrUnsetLemmaFlagVars[i]] = false;
   for(int i = 0; i < nTempLemmaIndex; i++) 
      arrLemmaFlag[abs(arrTempLemma[i])] = false;

   // Flush the inference queue.
   pInferenceQueueNextElt = pInferenceQueueNextEmpty = arrInferenceQueue;
   pFnInferenceQueueNextElt = pFnInferenceQueueNextEmpty = arrFnInferenceQueue;

   //Backjump to the highest uip point.

	int nCP_BSI = nOldBacktrackStackIndex;
	int nAtTop = 0;
	while(highest_uip_level < nOldBacktrackStackIndex) {
		// Pop the choice point stack.
		pChoicePointTop--;
		if (pChoicePointTop < arrChoicePointStack) {
			pChoicePointTop++;
			nAtTop = 1;
			break;
		}
      nInferredAtom = pChoicePointTop->nBranchVble;
      nInferredValue = arrSolution[nInferredAtom];

      nOldBacktrackStackIndex = arrBacktrackStackIndex[nInferredAtom];
		nCP_BSI = nOldBacktrackStackIndex;
      // Pop the num unresolved functions 
      nNumUnresolvedFunctions = pChoicePointTop->nNumUnresolved;
		
		pop_state_information(1);
		// Pop heuristic scores.
		if (nHeuristic == JOHNSON_HEURISTIC || nHeuristic == STATE_HEURISTIC)
		  J_PopHeuristicScores();

		// Pop the backtrack stack until we pop the branch atom.
		while (1)
		  {
			  pBacktrackTop--;
			  nBacktrackStackIndex--;
			  assert(nBacktrackStackIndex >= 0);
			  if (pBacktrackTop->pLemmaInfo && pBacktrackTop->pLemmaInfo->nBacktrackStackReferences > 0) {
				  // The lemma for this backtrack stack entry was a cached lemma.
				  // Decrement the reference count for it.
				  // (When the count reaches zero, the lemma can be recycled.)
				  (pBacktrackTop->pLemmaInfo->nBacktrackStackReferences)--;
				  
				  // Move lemma to front of lemma priority queue.
				  MoveToFrontOfLPQ(pBacktrackTop->pLemmaInfo);
					  
				  pBacktrackTop->pLemmaInfo = NULL;
			  }
			  if (pBacktrackTop->pLemmaInfo) FreeLemma(pBacktrackTop->pLemmaInfo, false);
			  
			  /*m invalidating arrBacktrackStackIndex but keep the prev value */
			  int nBacktrackAtom = pBacktrackTop->nAtom;
			  arrBacktrackStackIndex[nBacktrackAtom] = gnMaxVbleIndex + 1;
			  arrSolution[nBacktrackAtom] = BOOL_UNKNOWN;
			  if(nBacktrackAtom == nInferredAtom) break;
		  };  //  while (1)
		
		arrSolution[nInferredAtom] = BOOL_UNKNOWN;
      _num_backjumps++;
		
	}  // backjumping loop

   if (_num_backjumps) {
      ite_counters[NUM_BACKJUMPS]++;
      ite_counters[NUM_TOTAL_BACKJUMPS] += _num_backjumps;
   }
	
	LemmaInfoStruct *previous = &pUnitLemmaList;
	for (LemmaInfoStruct *p = previous->pNextLemma[0]; p; p = p->pNextLemma[0])
   {
      //m WatchedVble 0 and 1 have the highest BSI in the whole clause
		if(p->nWatchedVble[1] == 0) {
			//This lemma is a unit lemma and should be stuck in the inference
			//queue. This lemma will cause us to backtrack to the very
			//top of the search tree as well.
			assert(arrSolution[p->nWatchedVble[0]] == BOOL_UNKNOWN);
			
         InferLiteral(p->nWatchedVble[0], p->nWatchedVblePolarity[0],
               false, p->pLemma, NULL, 1);
		} else if(nAtTop==1 && arrBacktrackStackIndex[p->nWatchedVble[1]] < nCP_BSI){
			if(arrSolution[p->nWatchedVble[0]] == BOOL_UNKNOWN) {//Only want to apply CP once
				InferLiteral(p->nWatchedVble[0], p->nWatchedVblePolarity[0],
								 false, p->pLemma, NULL, 1);
			}
		}
		previous->pNextLemma[0] = p->pNextLemma[0];
		if (p == pUnitLemmaListTail) pUnitLemmaListTail = previous;
		
		AddLemmaIntoCache(p);
		
		p = previous;			
	} // for (LemmaInfoStruct *p = pUnitLemmaList->pNextLemma[0]; ; p = p->pNextLemma[0])

   BacktrackStackEntry *pBacktrackStackOldBranchPoint = pBacktrackTop;
   pBacktrackStackOldBranchPoint->pUnitLemmaList = NULL;
   pBacktrackStackOldBranchPoint->pUnitLemmaListTail = NULL;
	//I may not need to maintain pBacktrackStackOldBranchPoint...

	ite_counters[NO_ERROR]--;
	//Re-apply the choicepoint.
   //Heuristic does this automatically.

#ifdef DISPLAY_TRACE
   TB_9(
      cout << "Reversed polarity of X" << nInferredAtom
         << " to " << nInferredValue << endl;
      //DisplayAllBrancherLemmas();
      cout << "Would DisplayAllBrancherLemmas here." << endl;
   )
#endif

   // Get the consequences of the branch atoms new value.
   return 0; /* NO_ERROR */
}
