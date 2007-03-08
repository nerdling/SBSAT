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

int USE_BACKBURNER1 = 0;
int USE_BACKBURNER2 = 1;

ITE_INLINE
void AFSMoveLemma(LemmaInfoStruct *previous, LemmaInfoStruct *pLemmaListDisagreed, LemmaInfoStruct *pLemmaListTail, int negativeLit, int nVble, int idx)
{
	previous->pNextLemma[idx]
      = pLemmaListDisagreed->pNextLemma[idx];
   if (pLemmaListDisagreed->pNextLemma[idx])
   {
      pLemmaListDisagreed->pNextLemma[idx]->pPrevLemma[idx]
         = previous;
   } else {
		pLemmaListTail->pNextLemma[idx] = pLemmaListDisagreed->pPrevLemma[idx];
	}

   /* Add him into his new list */
   {
      LemmaInfoStruct *pLemmasWhere;
      if (negativeLit) pLemmasWhere = &((arrAFS + nVble)->LemmasWhereNeg[idx]);
      else pLemmasWhere = &((arrAFS + nVble)->LemmasWherePos[idx]);

      pLemmaListDisagreed->pNextLemma[idx]
         = pLemmasWhere->pNextLemma[idx];
      if (pLemmaListDisagreed->pNextLemma[idx])
      {
         pLemmaListDisagreed->pNextLemma[idx]
            ->pPrevLemma[idx]
            = pLemmaListDisagreed;
		} else {
			AffectedFuncsStruct *pAFS = arrAFS + nVble;
			if(negativeLit) {
				pAFS->LemmasWhereNegTail[idx].pNextLemma[idx] = pLemmaListDisagreed;
			} else {
				pAFS->LemmasWherePosTail[idx].pNextLemma[idx] = pLemmaListDisagreed;
			}
		}

      pLemmasWhere->pNextLemma[idx]
         = pLemmaListDisagreed;
      pLemmaListDisagreed->pPrevLemma[idx]
         = pLemmasWhere;
      pLemmaListDisagreed->nWatchedVblePolarity[idx]
         = 1-negativeLit;
      pLemmaListDisagreed->nWatchedVble[idx]
         = nVble;
   }
} 

ITE_INLINE
void AFSMoveLemmaToBack(LemmaInfoStruct *previous, LemmaInfoStruct *pLemmaListDisagreed, LemmaInfoStruct *pLemmaListTail, int negativeLit, int nVble, int idx)
{
	previous->pNextLemma[idx]
      = pLemmaListDisagreed->pNextLemma[idx];
   if (pLemmaListDisagreed->pNextLemma[idx])
   {
      pLemmaListDisagreed->pNextLemma[idx]->pPrevLemma[idx]
         = previous;
   } else {
		pLemmaListTail->pNextLemma[idx] = pLemmaListDisagreed->pPrevLemma[idx];
	}

   /* Add him into his new list */
   {
		AffectedFuncsStruct *pAFS = arrAFS + nVble;
		if(negativeLit) {
			pAFS->LemmasWhereNegTail[idx].pNextLemma[idx]->pNextLemma[idx] = pLemmaListDisagreed;
			pLemmaListDisagreed->pPrevLemma[idx] = pAFS->LemmasWhereNegTail[idx].pNextLemma[idx];
			pAFS->LemmasWhereNegTail[idx].pNextLemma[idx] = pLemmaListDisagreed;
		} else {
			pAFS->LemmasWherePosTail[idx].pNextLemma[idx]->pNextLemma[idx] = pLemmaListDisagreed;
			pLemmaListDisagreed->pPrevLemma[idx] = pAFS->LemmasWherePosTail[idx].pNextLemma[idx];
			pAFS->LemmasWherePosTail[idx].pNextLemma[idx] = pLemmaListDisagreed;
		}

		pLemmaListDisagreed->pNextLemma[idx] = NULL;
      pLemmaListDisagreed->nWatchedVblePolarity[idx]
         = 1-negativeLit;
      pLemmaListDisagreed->nWatchedVble[idx]
         = nVble;
   }
} 

ITE_INLINE
int
UpdateEachAffectedLemma(AffectedFuncsStruct *pAFS, int nInferredValue)
{
   int nVble;
   // Update each affected lemma.
   LemmaInfoStruct *pLemmaListDisagreed;
	LemmaInfoStruct *pLemmaListTail;
	
   if (nInferredValue == BOOL_TRUE) {
      pLemmaListDisagreed = &(pAFS->LemmasWhereNeg[0]);
		pLemmaListTail = &(pAFS->LemmasWhereNegTail[0]);
	} else {
      pLemmaListDisagreed = &(pAFS->LemmasWherePos[0]);
		pLemmaListTail = &(pAFS->LemmasWherePosTail[0]);
	}

   // Visit lemmas which have literals which contradict the
   // new assignment.

   LemmaInfoStruct *previous = pLemmaListDisagreed;
   bool bContinue = false;
   LemmaBlock *pLemmaBlock;
   int *arrLits; // Literals in the current lemma block.
   int nLemmaLength; // # of literals in the lemma.
   int nLitIndex; // Index relative to the whole lemma.
   int nLitIndexInBlock; // Index relative to the individual
   // lemma block.  I.e., this is
   // an index into arrLits.
   int nLitIndexInBlockLen; 
   int nVbleValue;
   int negativeLit = 0;
   bool bWatchedVbleUnknown = false;

   for (pLemmaListDisagreed = pLemmaListDisagreed->pNextLemma[0];
         pLemmaListDisagreed;
         pLemmaListDisagreed = pLemmaListDisagreed->pNextLemma[0])
   {
      if(arrSolution[pLemmaListDisagreed->nWatchedVble[1]] == pLemmaListDisagreed->nWatchedVblePolarity[1])
      {
			//Lemma is currently satisfied by watched variable 1

			if(USE_BACKBURNER1) {
				//Search for the variable in this lemma that satisfies the lemma
				//and has the lowest BacktrackStackIndex (highest in the search tree)
				pLemmaBlock = pLemmaListDisagreed->pLemma;
				arrLits = pLemmaBlock->arrLits;
				nLemmaLength = arrLits[0];
				
				nLitIndex = nLitIndexInBlock = 1;
				nLitIndexInBlockLen = nLemmaLength + 1;
				if (nLitIndexInBlockLen > LITS_PER_LEMMA_BLOCK)
				  nLitIndexInBlockLen = LITS_PER_LEMMA_BLOCK;
				
				int nLowest_BtSI = nBacktrackStackIndex+1;
				int nNewWatchedVar = 0;
				int nNewNegativeLit = 0;
				int nSecond_BtSI = nBacktrackStackIndex+1;
				int nSecondWatchedVar = 0;
				int nSecondNegativeLit = 0;
				
				while(1) {
					for(; nLitIndexInBlock < nLitIndexInBlockLen; nLitIndexInBlock++) {
						nVble = arrLits[nLitIndexInBlock];
						negativeLit = 0; 
						if (nVble < 0) {
							nVble = -1*nVble;
							negativeLit = 1;
						}
						nVbleValue = arrSolution[nVble]; 
						if((nVbleValue ^ negativeLit) == 1) {
							//This variable satisfies this lemma
							int nCurrent_BtSI = arrBacktrackStackIndex[nVble];
							if(nCurrent_BtSI < nLowest_BtSI) {
								nSecondWatchedVar = nNewWatchedVar;
								nSecondNegativeLit = nNewNegativeLit;
								nSecond_BtSI = nLowest_BtSI;
								nLowest_BtSI = nCurrent_BtSI;
								nNewWatchedVar = nVble;
								nNewNegativeLit = negativeLit;
							} else if (nCurrent_BtSI < nSecond_BtSI) {
								nSecondWatchedVar = nVble;
								nSecondNegativeLit = negativeLit;
								nSecond_BtSI = nCurrent_BtSI;
							}
						}
					}
					nLitIndexInBlock = 0;
					nLitIndex += LITS_PER_LEMMA_BLOCK;
					nLitIndexInBlockLen = nLemmaLength - nLitIndex + 2;
					if (nLitIndexInBlockLen <  0) break;
					if (nLitIndexInBlockLen > LITS_PER_LEMMA_BLOCK)
					  nLitIndexInBlockLen = LITS_PER_LEMMA_BLOCK;
					pLemmaBlock = pLemmaBlock->pNext;
					arrLits = pLemmaBlock->arrLits;
				}
				nVble = nNewWatchedVar;
				negativeLit = nNewNegativeLit;
				if(nVble != pLemmaListDisagreed->nWatchedVble[1] && nVble!=pLemmaListDisagreed->nWatchedVble[0]) {
					AffectedFuncsStruct *pAFS_1 = arrAFS + pLemmaListDisagreed->nWatchedVble[1];
					if(pLemmaListDisagreed->nWatchedVblePolarity[1])
					  AFSMoveLemmaToBack(pLemmaListDisagreed->pPrevLemma[1], pLemmaListDisagreed, &(pAFS_1->LemmasWherePosTail[1]), negativeLit, nVble, 1);
					else
					  AFSMoveLemmaToBack(pLemmaListDisagreed->pPrevLemma[1], pLemmaListDisagreed, &(pAFS_1->LemmasWhereNegTail[1]), negativeLit, nVble, 1);
				}
				if(nSecondWatchedVar != 0) {
					AFSMoveLemmaToBack(previous, pLemmaListDisagreed, pLemmaListTail, nSecondNegativeLit, nSecondWatchedVar, 0);
					pLemmaListDisagreed = previous;
				} else  {
					previous = pLemmaListDisagreed;
				}
			} else {
				previous = pLemmaListDisagreed;
			}

         continue;
      }

      pLemmaBlock = pLemmaListDisagreed->pLemma;
      arrLits = pLemmaBlock->arrLits;
      nLemmaLength = arrLits[0];

      nLitIndex = nLitIndexInBlock = 1;
      nLitIndexInBlockLen = nLemmaLength + 1;
      if (nLitIndexInBlockLen > LITS_PER_LEMMA_BLOCK)
         nLitIndexInBlockLen = LITS_PER_LEMMA_BLOCK;

      while (1)
      {

         for (; nLitIndexInBlock < nLitIndexInBlockLen;
               nLitIndexInBlock++)
         {
            nVble = arrLits[nLitIndexInBlock];
            negativeLit = 0; 
            if (nVble < 0)
            {
               nVble = -1*nVble;
               negativeLit = 1;
            };
            nVbleValue = arrSolution[nVble]; 
            /*
             * nVbleValue xor negativeLit
             * BOOL_FALSE 0
             * BOOL_TRUE 1
             * BOOL_UNKNOWN 2
             *                1 (negative => BOOL_FALSE)
             *                0 (positive => BOOL_TRUE)
             0 xor 1 = 1; <= satisfied
             1 xor 1 = 0; <= (not sat)
             2 xor 1 = 3; <= unkonwn

             0 xor 0 = 0; <= (not sat)
             1 xor 0 = 1; <= satisfied
             2 xor 0 = 2; <= unkonwn
             */
            if (nVbleValue ^ negativeLit) {
               if (nVbleValue == BOOL_UNKNOWN && nVble == pLemmaListDisagreed->nWatchedVble[1]) 
               { bWatchedVbleUnknown = true; continue; }

					int satisfied = 0;
					if(USE_BACKBURNER2) {
						if ((nVbleValue ^ negativeLit) == 1) { //Lemma is satisfied
							satisfied = 1;
							//Search for the variable in this lemma that satisfies the lemma
							//and has the lowest BacktrackStackIndex (highest in the search tree)
							int nLowest_BtSI = arrBacktrackStackIndex[nVble];
							int nNewWatchedVar = nVble;
							int nNewNegativeLit = negativeLit;
							int nSecond_BtSI = arrBacktrackStackIndex[nVble];
							int nSecondWatchedVar = 0;
							int nSecondNegativeLit = 0;
							nLitIndexInBlock++;
							while(1) {
								for(; nLitIndexInBlock < nLitIndexInBlockLen; nLitIndexInBlock++) {
									nVble = arrLits[nLitIndexInBlock];
									negativeLit = 0; 
									if (nVble < 0) {
										nVble = -1*nVble;
										negativeLit = 1;
									}
									nVbleValue = arrSolution[nVble]; 
									if((nVbleValue ^ negativeLit) == 1) { //If this variable also satisfies this lemma
										int nCurrent_BtSI = arrBacktrackStackIndex[nVble];
										if(nCurrent_BtSI < nLowest_BtSI) {
											nSecondWatchedVar = nNewWatchedVar;
											nSecondNegativeLit = nNewNegativeLit;
											nSecond_BtSI = nLowest_BtSI;
											nLowest_BtSI = nCurrent_BtSI;
											nNewWatchedVar = nVble;
											nNewNegativeLit = negativeLit;
										} else if (nCurrent_BtSI < nSecond_BtSI) {
											nSecondWatchedVar = nVble;
											nSecondNegativeLit = negativeLit;
											nSecond_BtSI = nCurrent_BtSI;
										}
									}
								}
								nLitIndexInBlock = 0;
								nLitIndex += LITS_PER_LEMMA_BLOCK;
								nLitIndexInBlockLen = nLemmaLength - nLitIndex + 2;
								if (nLitIndexInBlockLen <  0) break;
								if (nLitIndexInBlockLen > LITS_PER_LEMMA_BLOCK)
								  nLitIndexInBlockLen = LITS_PER_LEMMA_BLOCK;
								pLemmaBlock = pLemmaBlock->pNext;
								arrLits = pLemmaBlock->arrLits;
							}
							nVble = nNewWatchedVar;
							negativeLit = nNewNegativeLit;
							if(nSecondWatchedVar != 0) {
								AffectedFuncsStruct *pAFS_1 = arrAFS + pLemmaListDisagreed->nWatchedVble[1];
								if(pLemmaListDisagreed->nWatchedVblePolarity[1])
								  AFSMoveLemmaToBack(pLemmaListDisagreed->pPrevLemma[1], pLemmaListDisagreed, &(pAFS_1->LemmasWherePosTail[1]), nSecondNegativeLit, nSecondWatchedVar, 1);
								else
								  AFSMoveLemmaToBack(pLemmaListDisagreed->pPrevLemma[1], pLemmaListDisagreed, &(pAFS_1->LemmasWhereNegTail[1]), nSecondNegativeLit, nSecondWatchedVar, 1);
							}
						}
					} //END USE_BACKBURNER
					
					// Lemma is now satisfied  or unknown
					//
               // Remove lemma from his old list
					if(satisfied)
					  AFSMoveLemmaToBack(previous, pLemmaListDisagreed, pLemmaListTail, negativeLit, nVble, 0);
					else
					  AFSMoveLemma(previous, pLemmaListDisagreed, pLemmaListTail, negativeLit, nVble, 0);
						
               bWatchedVbleUnknown = false;
               bContinue = true;
               break;
            }
         }
         if (bContinue) break;
         else {
            nLitIndexInBlock = 0;
            nLitIndex += LITS_PER_LEMMA_BLOCK;
            nLitIndexInBlockLen = nLemmaLength - nLitIndex + 2;
            if (nLitIndexInBlockLen <  0) break;
            if (nLitIndexInBlockLen > LITS_PER_LEMMA_BLOCK)
               nLitIndexInBlockLen = LITS_PER_LEMMA_BLOCK;
            pLemmaBlock = pLemmaBlock->pNext;
            arrLits = pLemmaBlock->arrLits;
         };
      }

      if(bContinue) 
      { 
         pLemmaListDisagreed = previous;
         bContinue = false;
         continue; //Done with current lemma, Goto next lemma 
      }

      if(bWatchedVbleUnknown)
      {
         //Apply nWatchedVble[1] as an inference

         int nWatchedVble = pLemmaListDisagreed->nWatchedVble[1];
         int nWatchedVblePolarity = pLemmaListDisagreed->nWatchedVblePolarity[1];

         assert(arrSolution[nWatchedVble] == BOOL_UNKNOWN);

         TB_9(
               d9_printf3("Inferring X %d=%d\n", nWatchedVble, nWatchedVblePolarity);
               d9_printf1("Inference came from unit lemma\n");
               DisplayLemmaInfo(pLemmaListDisagreed);
            );

         pLemmaListDisagreed->nNumLemmaInfs++;
         pLemmaListDisagreed->nLemmaLastUsed = ite_counters[NUM_LEMMA_INTO_CACHE];
         if (pLemmaListDisagreed->nLemmaFirstUseful == 0) 
            pLemmaListDisagreed->nLemmaFirstUseful = ite_counters[NUM_LEMMA_INTO_CACHE];
         ite_counters[INF_LEMMA]++;
         InferLiteral(nWatchedVble, nWatchedVblePolarity, false,
               pLemmaListDisagreed->pLemma, pLemmaListDisagreed, 1);

			
         /* keep it in the memory until you backtrack */
         (pLemmaListDisagreed->nBacktrackStackReferences)++; 

         // Move lemma to front of lemma priority queue.
         //MoveToFrontOfLPQ(pLemmaListDisagreed);

         previous = pLemmaListDisagreed;
         bWatchedVbleUnknown = false;
         continue;
      }
      pLemmaListDisagreed->nNumLemmaConflict++;
      pLemmaListDisagreed->nLemmaLastUsed = ite_counters[NUM_LEMMA_INTO_CACHE];
      if (pLemmaListDisagreed->nLemmaFirstUseful == 0) 
         pLemmaListDisagreed->nLemmaFirstUseful = ite_counters[NUM_LEMMA_INTO_CACHE];
      pConflictLemma = pLemmaListDisagreed->pLemma;
		pConflictLemmaInfo = pLemmaListDisagreed;
      // goto_Backtrack;
      return ERR_BT_LEMMA;
   }

   if (nInferredValue == BOOL_TRUE) {
      pLemmaListDisagreed = &(pAFS->LemmasWhereNeg[1]);
		pLemmaListTail = &(pAFS->LemmasWhereNegTail[1]);
	} else {
      pLemmaListDisagreed = &(pAFS->LemmasWherePos[1]);
		pLemmaListTail = &(pAFS->LemmasWherePosTail[1]);
	}

   previous = pLemmaListDisagreed;
   bContinue = false;
   bWatchedVbleUnknown = false;

   for (pLemmaListDisagreed = pLemmaListDisagreed->pNextLemma[1];
         pLemmaListDisagreed;
         pLemmaListDisagreed = pLemmaListDisagreed->pNextLemma[1])
   {
      if(arrSolution[pLemmaListDisagreed->nWatchedVble[0]] == pLemmaListDisagreed->nWatchedVblePolarity[0])
      {
			//Lemma is currently satisfied by watched variable 0.

         if(USE_BACKBURNER1) {
				//Search for the variable in this lemma that satisfies the lemma
				//and has the lowest BacktrackStackIndex (highest in the search tree)
				pLemmaBlock = pLemmaListDisagreed->pLemma;
				arrLits = pLemmaBlock->arrLits;
				nLemmaLength = arrLits[0];
				
				nLitIndex = nLitIndexInBlock = 1;
				nLitIndexInBlockLen = nLemmaLength + 1;
				if (nLitIndexInBlockLen > LITS_PER_LEMMA_BLOCK)
				  nLitIndexInBlockLen = LITS_PER_LEMMA_BLOCK;
				
				int nLowest_BtSI = nBacktrackStackIndex+1;
				int nNewWatchedVar = 0;
				int nNewNegativeLit = 0;
				int nSecond_BtSI = nBacktrackStackIndex+1;
				int nSecondWatchedVar = 0;
				int nSecondNegativeLit = 0;
				
				while(1) {
					for(; nLitIndexInBlock < nLitIndexInBlockLen; nLitIndexInBlock++) {
						nVble = arrLits[nLitIndexInBlock];
						negativeLit = 0; 
						if (nVble < 0) {
							nVble = -1*nVble;
							negativeLit = 1;
						}
						nVbleValue = arrSolution[nVble]; 
						if((nVbleValue ^ negativeLit) == 1) {
							//This variable satisfies this lemma
							int nCurrent_BtSI = arrBacktrackStackIndex[nVble];
							if(nCurrent_BtSI < nLowest_BtSI) {
								nSecondWatchedVar = nNewWatchedVar;
								nSecondNegativeLit = nNewNegativeLit;
								nSecond_BtSI = nLowest_BtSI;
								nLowest_BtSI = nCurrent_BtSI;
								nNewWatchedVar = nVble;
								nNewNegativeLit = negativeLit;
							} else if (nCurrent_BtSI < nSecond_BtSI) {
								nSecondWatchedVar = nVble;
								nSecondNegativeLit = negativeLit;
								nSecond_BtSI = nCurrent_BtSI;
							}
						}
					}
					nLitIndexInBlock = 0;
					nLitIndex += LITS_PER_LEMMA_BLOCK;
					nLitIndexInBlockLen = nLemmaLength - nLitIndex + 2;
					if (nLitIndexInBlockLen <  0) break;
					if (nLitIndexInBlockLen > LITS_PER_LEMMA_BLOCK)
					  nLitIndexInBlockLen = LITS_PER_LEMMA_BLOCK;
					pLemmaBlock = pLemmaBlock->pNext;
					arrLits = pLemmaBlock->arrLits;
				}
				nVble = nNewWatchedVar;
				negativeLit = nNewNegativeLit;
				if(nVble != pLemmaListDisagreed->nWatchedVble[0] && nVble!=pLemmaListDisagreed->nWatchedVble[1]) {
					AffectedFuncsStruct *pAFS_0 = arrAFS + pLemmaListDisagreed->nWatchedVble[0];
					if(pLemmaListDisagreed->nWatchedVblePolarity[0])
					  AFSMoveLemmaToBack(pLemmaListDisagreed->pPrevLemma[0], pLemmaListDisagreed, &(pAFS_0->LemmasWherePosTail[0]), negativeLit, nVble, 0);
					else
					  AFSMoveLemmaToBack(pLemmaListDisagreed->pPrevLemma[0], pLemmaListDisagreed, &(pAFS_0->LemmasWhereNegTail[0]), negativeLit, nVble, 0);
				}
				if(nSecondWatchedVar != 0) {
					AFSMoveLemmaToBack(previous, pLemmaListDisagreed, pLemmaListTail, nSecondNegativeLit, nSecondWatchedVar, 1);
					pLemmaListDisagreed = previous;
				} else  {
					previous = pLemmaListDisagreed;
				}
			} else {
				previous = pLemmaListDisagreed;
			}
         continue;
      }

      pLemmaBlock = pLemmaListDisagreed->pLemma;
      arrLits = pLemmaBlock->arrLits;
      nLemmaLength = arrLits[0];

      nLitIndex = nLitIndexInBlock = 1;
      nLitIndexInBlockLen = nLemmaLength + 1;
      if (nLitIndexInBlockLen > LITS_PER_LEMMA_BLOCK)
         nLitIndexInBlockLen = LITS_PER_LEMMA_BLOCK;

      while (1) {
         for (; nLitIndexInBlock < nLitIndexInBlockLen;
				  nLitIndexInBlock++) {
				nVble = arrLits[nLitIndexInBlock];
				negativeLit = 0; 
				if (nVble < 0)
				  {
					  nVble = -1*nVble;
					  negativeLit = 1;
				  };
            nVbleValue = arrSolution[nVble];
            /*
             negative, value=BOOL_FALSE^1 = satisfied
             0 xor 1 = 1;
             1 xor 1 = 0;
             2 xor 1 = 3;

             positive, value=BOOL_TRUE^0 = satisfied
             0 xor 0 = 0;
             1 xor 0 = 1;
             2 xor 0 = 2;
             */
            if (nVbleValue ^ negativeLit) {
               if (nVbleValue == BOOL_UNKNOWN && nVble == pLemmaListDisagreed->nWatchedVble[0]) 
               { bWatchedVbleUnknown = true; continue; }

					int satisfied = 0;
					if(USE_BACKBURNER2) {
						if((nVbleValue ^ negativeLit) == 1) { //Lemma is satisfied
							satisfied = 1;
							//Search for the variable in this lemma that satisfies the lemma
							//and has the lowest BacktrackStackIndex (highest in the search tree)
							int nLowest_BtSI = arrBacktrackStackIndex[nVble];
							int nNewWatchedVar = nVble;
							int nNewNegativeLit = negativeLit;
							int nSecond_BtSI = arrBacktrackStackIndex[nVble];
							int nSecondWatchedVar = 0;
							int nSecondNegativeLit = 0;
							nLitIndexInBlock++;
							while(1) {
								for(; nLitIndexInBlock < nLitIndexInBlockLen; nLitIndexInBlock++) {
									nVble = arrLits[nLitIndexInBlock];
									negativeLit = 0; 
									if (nVble < 0) {
										nVble = -1*nVble;
										negativeLit = 1;
									}
									nVbleValue = arrSolution[nVble]; 
									if((nVbleValue ^ negativeLit) == 1) { //If this variable also satisfies this lemma
										int nCurrent_BtSI = arrBacktrackStackIndex[nVble];
										if(nCurrent_BtSI < nLowest_BtSI) {
											nSecondWatchedVar = nNewWatchedVar;
											nSecondNegativeLit = nNewNegativeLit;
											nSecond_BtSI = nLowest_BtSI;
											nLowest_BtSI = nCurrent_BtSI;
											nNewWatchedVar = nVble;
											nNewNegativeLit = negativeLit;
										} else if (nCurrent_BtSI < nSecond_BtSI) {
											nSecondWatchedVar = nVble;
											nSecondNegativeLit = negativeLit;
											nSecond_BtSI = nCurrent_BtSI;
										}
									}
								}
								nLitIndexInBlock = 0;
								nLitIndex += LITS_PER_LEMMA_BLOCK;
								nLitIndexInBlockLen = nLemmaLength - nLitIndex + 2;
								if (nLitIndexInBlockLen <  0) break;
								if (nLitIndexInBlockLen > LITS_PER_LEMMA_BLOCK)
								  nLitIndexInBlockLen = LITS_PER_LEMMA_BLOCK;
								pLemmaBlock = pLemmaBlock->pNext;
								arrLits = pLemmaBlock->arrLits;
							}
							nVble = nNewWatchedVar;
							negativeLit = nNewNegativeLit;
							if(nSecondWatchedVar != 0) {
								AffectedFuncsStruct *pAFS_0 = arrAFS + pLemmaListDisagreed->nWatchedVble[0];
								if(pLemmaListDisagreed->nWatchedVblePolarity[0])
								  AFSMoveLemmaToBack(pLemmaListDisagreed->pPrevLemma[0], pLemmaListDisagreed, &(pAFS_0->LemmasWherePosTail[0]), nSecondNegativeLit, nSecondWatchedVar, 0);
								else
								  AFSMoveLemmaToBack(pLemmaListDisagreed->pPrevLemma[0], pLemmaListDisagreed, &(pAFS_0->LemmasWhereNegTail[0]), nSecondNegativeLit, nSecondWatchedVar, 0);
							}
						}
					} //END USE_BACKBURNER
					
					//Lemma is satisfied now or unknown
					if(satisfied)
					  AFSMoveLemmaToBack(previous, pLemmaListDisagreed, pLemmaListTail, negativeLit, nVble, 1);
					else
					  AFSMoveLemma(previous, pLemmaListDisagreed, pLemmaListTail, negativeLit, nVble, 1);
					
               bWatchedVbleUnknown = false;
               bContinue = true;
               break;
            }
         }
         if (bContinue) break;
         else {
            nLitIndexInBlock = 0;
            nLitIndex += LITS_PER_LEMMA_BLOCK;
            nLitIndexInBlockLen = nLemmaLength - nLitIndex + 2;
            if (nLitIndexInBlockLen <  0) break;
            if (nLitIndexInBlockLen > LITS_PER_LEMMA_BLOCK)
               nLitIndexInBlockLen = LITS_PER_LEMMA_BLOCK;
            pLemmaBlock = pLemmaBlock->pNext;
            arrLits = pLemmaBlock->arrLits;
         };
      }

      if(bContinue) 
      { 
         pLemmaListDisagreed = previous;
         bContinue = false;
         continue; 
      }

      if(bWatchedVbleUnknown)
      {
         //Apply nWatchedVble[0] as an inference
         int nWatchedVble = pLemmaListDisagreed->nWatchedVble[0];
         int nWatchedVblePolarity
            = pLemmaListDisagreed->nWatchedVblePolarity[0];

         assert(arrSolution[nWatchedVble] == BOOL_UNKNOWN);

         TB_9(
               d9_printf3("Inferring X %d=%d\n", nWatchedVble, nWatchedVblePolarity);
               d9_printf1("Inference came from unit lemma\n");
               DisplayLemmaInfo(pLemmaListDisagreed);
            );

         pLemmaListDisagreed->nNumLemmaInfs++;
         pLemmaListDisagreed->nLemmaLastUsed = ite_counters[NUM_LEMMA_INTO_CACHE];
         if (pLemmaListDisagreed->nLemmaFirstUseful == 0) 
            pLemmaListDisagreed->nLemmaFirstUseful = ite_counters[NUM_LEMMA_INTO_CACHE];
         ite_counters[INF_LEMMA]++;
         InferLiteral(nWatchedVble, nWatchedVblePolarity, false,
               pLemmaListDisagreed->pLemma, pLemmaListDisagreed, 1);
         /* pBacktrackTop->pLemmaInfo = pLemmaListDisagreed; */

         /* keep it in the memory until you backtrack */
         (pLemmaListDisagreed->nBacktrackStackReferences)++;

         // Move lemma to front of lemma priority queue.
         //MoveToFrontOfLPQ(pLemmaListDisagreed);

         previous = pLemmaListDisagreed;
         bWatchedVbleUnknown = false;
         continue;
      }
      pLemmaListDisagreed->nNumLemmaConflict++;
      pLemmaListDisagreed->nLemmaLastUsed = ite_counters[NUM_LEMMA_INTO_CACHE];
      if (pLemmaListDisagreed->nLemmaFirstUseful == 0) 
         pLemmaListDisagreed->nLemmaFirstUseful = ite_counters[NUM_LEMMA_INTO_CACHE];
      pConflictLemma = pLemmaListDisagreed->pLemma;
		pConflictLemmaInfo = pLemmaListDisagreed;
      // goto_Backtrack;
      return ERR_BT_LEMMA;
   }
   return NO_ERROR;
}
