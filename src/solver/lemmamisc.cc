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
// lemmainfo.cc
// Started 8/16/01 - J. Ward

#include "ite.h"
#include "solver.h"

extern int nCallsToAddLemma;
extern int nLemmaSpaceBlocksAvail;

int nTotalBytesForLemmaInferences  = 0;
	
LemmaLookupStruct *arrLemmaLookupSpace = NULL;
int nNumPrimeImplicants;
SmurfState **arrPrecomputedStates = NULL;
int *arrVbleMapping = NULL;
unsigned long *arrVbleEncoding = NULL;
IntegerSet_ArrayBased reverseVariableMapping;

int *arrBacktrackStackIndex;

// ------------------------------------------------------------------------
// TOC:
//
// ------------------------------------------------------------------------
// ----------------------- Smurf ------------------------------------------
// ----------------------- variable mapping and encoding ------------------
// ----------------------- precomputed states array -----------------------
// ----------------------- lemmas for smurf -------------------------------
// ----------------------- special functions lemma construction -----------
// ----------------------- lemma display ----------------------------------
// ----------------------- checking and verification ----------------------
// ------------------------------------------------------------------------

// ----------------------- Smurf ------------------------------------------
ITE_INLINE
PartialAssignmentEncoding::
PartialAssignmentEncoding()
{
  unEncoding = 0;
}

ITE_INLINE
PartialAssignmentEncoding::
PartialAssignmentEncoding(unsigned long n)
{
  unEncoding = n;
}

ITE_INLINE
PartialAssignmentEncoding
PartialAssignmentEncoding::
AddLiteral(int nVble, bool bPolarity)
  // Add a literal mentioning nVble to the current assignment encoding.
  // The way we do this is:
  // Let n be the mapping of the variable for the current constraint.
  // (See ComputeVariableMapping() below.)
  // Add 3^n to the current encoding if bPolarity is true.
  // O.w. add 2 * 3^n.
{
   unsigned long unEncodingOfLiteral;
   if (SMURFS_SHARE_PATHS) {
      // Get relative position of vble index in the current constraint.
      int nVbleMapping = arrVbleMapping[nVble];
      if (!(nVbleMapping >= 0 && nVbleMapping < MAX_VBLES_PER_SMURF)) 
      {
         fprintf(stderr, "nVbleMapping(%d) < MAX_VBLES_PER_SMURF failed\n", nVbleMapping);
         fprintf(stderr, "Please increase MAX_VBLES_PER_SMURF ( -S <n> )\n");
         exit(1);
      }

      // Get the power of 3 encoding of the variable.
      unsigned long unVbleEncoding = arrVbleEncoding[nVbleMapping];

      unEncodingOfLiteral
         = unEncoding + (bPolarity ? unVbleEncoding : (2 * unVbleEncoding));
   } else {
      unEncodingOfLiteral = 0;
   }

  return PartialAssignmentEncoding(unEncodingOfLiteral);
}

ITE_INLINE bool
PartialAssignmentEncoding::
FindOrAddState(SmurfState **ppSmurfState)
{
   if (SMURFS_SHARE_PATHS) {
      // Check whether a SmurfState has been allocated for this path
      // by checking the value of arrPrecomputedStates[unEncoding].
      if (!(/*unEncoding >= 0 &&*/ unEncoding < (unsigned long)MAX_NUM_PATH_EQUIV_SETS)) 
      {
         fprintf(stderr, "unEncoding(%ld) < MAX_NUM_PATH_EQUIV_SETS failed\n", unEncoding);
         fprintf(stderr, "Please increase MAX_VBLES_PER_SMURF ( -S <n> )\n");
         exit(1);
      };
      SmurfState **ppState = arrPrecomputedStates + unEncoding;

      /*cout << "unEncoding: " << unEncoding  
       << " arrPrecomputedStates: " << arrPrecomputedStates
       << " arrPrecomputedStates[unEncoding]: " 
       << arrPrecomputedStates[unEncoding]; // << endl;
       */ 

      if (*ppState)
      {
         // The answer is yes.  Return a pointer to the state.
         *ppSmurfState = *ppState;
         return true;
      }
      // The answer is no.  Allocate a state.
      *ppState = AllocateSmurfState(); 
      *ppSmurfState = *ppState;
      return false;
   }
   *ppSmurfState = AllocateSmurfState(); 
   return false;
}

// -------------------------------- variable mapping and encoding --------------------

ITE_INLINE
void
InitVbleEncodingArray()
{
   assert(!arrVbleEncoding);

   ITE_NEW_CATCH(
         arrVbleEncoding = new unsigned long [MAX_VBLES_PER_SMURF],
         "arrVbleEnconding")
      unsigned long unPower = 1;
   for (int i = 0; i < MAX_VBLES_PER_SMURF; i++)
   {
      arrVbleEncoding[i] = unPower;
      unPower *= 3;
   }
}

ITE_INLINE
void
FreeVbleEncodingArray()
{
   assert(arrVbleEncoding);
   delete[] arrVbleEncoding;
   arrVbleEncoding = 0;
}


ITE_INLINE
void
InitVbleMappingArray(int nArrayLength)
{
   assert(!arrVbleMapping);
   ITE_NEW_CATCH(
         arrVbleMapping = new int[nArrayLength],
         "Unable to allocate variable mapping array.");
}

ITE_INLINE
void
FreeVbleMappingArray()
{
   assert(arrVbleMapping);
   delete[] arrVbleMapping;
   arrVbleMapping = 0;
}

ITE_INLINE
void
ComputeVariableMapping(IntegerSet &isetVbles)
   // E.g., suppose a variable set is {4, 6, 11, 14}.
   // The mapping we want takes variable indicies and maps them
   // to their relative positions in the variable set.
   // In this example, 4 |-> 0,  6 |-> 1,  11 |-> 2,  14 |-> 3.
   // Stores the mapping in the external static array arrVbleMapping.
{
   // IntegerSetIterator is assumed to return its elements in increasing order.
   int nVble;
   int nLocalIndex = 0;
#ifndef NDEBUG
   int nOldValue = 0;
#endif

   assert(arrVbleMapping);
   IntegerSetIterator isetNext(isetVbles);
   while (isetNext(nVble))
   {
      // Some assertions to check the increasing order assumption.
      assert((nLocalIndex == 0) || nOldValue > nVble);
#ifndef NDEBUG
      nOldValue = nVble;
#endif

      arrVbleMapping[nVble] = nLocalIndex++;
   }
}

ITE_INLINE
void
ComputeVariableMapping(IntegerSet_ArrayBased &isetabVbles)
   // Same basic functionality as the preceding version of
   // ComputeVariableMapping.
{
   int nNumVbles = isetabVbles.nNumElts;
   int *arrVbles = isetabVbles.arrElts;

   assert(arrVbleMapping);
   for (int i = 0; i < nNumVbles; i++)
   {
      // Verifying that values in arrVbles are in increasing order.
      assert((i == 0) || arrVbles[i-1] > arrVbles[i]);

      arrVbleMapping[arrVbles[i]] = i;
   }
}

// ----------------------------- precomputed states array --------------------------------

ITE_INLINE
void
InitPrecomputedStatesArray()
{  
   assert(!arrPrecomputedStates);
   arrPrecomputedStates = (SmurfState**)ite_calloc(MAX_NUM_PATH_EQUIV_SETS,sizeof(SmurfState*),
         2, "precomputed states");
   ResetPrecomputedStatesArray();
}

ITE_INLINE
void
ResetPrecomputedStatesArray()
{
   assert(arrPrecomputedStates);
   memset(arrPrecomputedStates, 0, MAX_NUM_PATH_EQUIV_SETS*sizeof(SmurfState*));
}

ITE_INLINE
void
FreePrecomputedStatesArray()
{
   assert(arrPrecomputedStates);
   ite_free((void**)&arrPrecomputedStates);
   arrPrecomputedStates = 0;
}

// ----------------------------------- lemmas for smurf -----------------------------

ITE_INLINE
void
ComputeLemmasForSmurf(SmurfState *pState)
   // pState points to the root state of a Smurf.
   // Traverse the Smurf (breadth-first) generating
   // one lemma for each inference on each Smurf transition.
   // The lemma generated in each case should be one of the shortest
   // prime implicants of the Smurf's constraints such that the
   // prime implicant implies the inference.
   // Each literal in the lemma, other that the literal which is inferred
   // by the transistion, should be negated by the path which
   // led to this state.
   // This lemma is an array of signed integers which represents a
   // disjunction of literals.
{
   LemmatizeQueue ltq;
   LemmaBitEncoding lbe;  // Initialized to encoding of empty path.
   ComputeVariableMapping(pState->vbles);
   reverseVariableMapping = pState->vbles;

   ResetLemmaLookupSpace();
   pState->cFlag = 1;
   ltq.Enqueue(pState, lbe);
   while (!ltq.IsEmpty())
   {
      ltq.Dequeue(pState, lbe);
      int nNumVbles = pState->vbles.nNumElts;
      int *arrVbles = pState->vbles.arrElts;
      for (int i = 0; i < nNumVbles; i++)
      {
         int nVble = arrVbles[i];

         Transition *pTransitionT = FindTransition(pState, i, nVble, BOOL_TRUE);
         Transition *pTransitionF = FindTransition(pState, i, nVble, BOOL_FALSE);
         int nTotalInferences = 
                    pTransitionT->positiveInferences.nNumElts +
                    pTransitionT->negativeInferences.nNumElts +
                    pTransitionF->positiveInferences.nNumElts +
                    pTransitionF->negativeInferences.nNumElts;
         LemmaBlock **arrInferenceBuffer = 
                (LemmaBlock **)ite_calloc(nTotalInferences, sizeof(LemmaBlock*),
                     9, "lemma information transition");
         nTotalBytesForLemmaInferences  += nTotalInferences*sizeof(LemmaBlock*);

         /////////////////////////////
         // Handle true transition. //
         /////////////////////////////
         Transition *pTransition = pTransitionT;

         assert(pTransition);
         LemmaBitEncoding lbe2 = lbe.AddLiteral(nVble, false);

         // Place a lemma on each positive inference.
         int nNumInferences = pTransition->positiveInferences.nNumElts;
         int *arrInferences = pTransition->positiveInferences.arrElts;
         assert(pTransition->arrPosInferenceLemmas == 0);
         if (nNumInferences > 0) {
            pTransition->arrPosInferenceLemmas = arrInferenceBuffer;
            arrInferenceBuffer += nNumInferences;
            for (int j = 0; j < nNumInferences; j++)
            {
               LemmaBitEncoding lbe3 = lbe2.AddLiteral(arrInferences[j], true);
               pTransition->arrPosInferenceLemmas[j] = FindOrAddLemma(lbe3);
            }
         }
         // Place a lemma on each negative inference.
         nNumInferences = pTransition->negativeInferences.nNumElts;
         arrInferences = pTransition->negativeInferences.arrElts;
         assert(pTransition->arrNegInferenceLemmas == 0);
         if (nNumInferences > 0) {
            pTransition->arrNegInferenceLemmas = arrInferenceBuffer;
            arrInferenceBuffer += nNumInferences;
            for (int j = 0; j < nNumInferences; j++)
            {
               LemmaBitEncoding lbe3 = lbe2.AddLiteral(arrInferences[j], false);
               pTransition->arrNegInferenceLemmas[j] = FindOrAddLemma(lbe3);
            }
         } 
         // Place the resulting state in the lemmatize queue
         // so that we can continue our breadthfirst search.
         SmurfState *pNextState = pTransition->pNextState;
         if (pNextState->cFlag != 1)
         {
            pNextState->cFlag = 1;
            ltq.Enqueue(pNextState, lbe2);
         }


         //////////////////////////////
         // Handle false transition. //
         //////////////////////////////
         pTransition = pTransitionF;

         assert(pTransition);
         lbe2 = lbe.AddLiteral(nVble, true);

         // Place a lemma on each positive inference.
         nNumInferences = pTransition->positiveInferences.nNumElts;
         arrInferences = pTransition->positiveInferences.arrElts;
         assert(pTransition->arrPosInferenceLemmas == 0);
         if (nNumInferences > 0) {
            pTransition->arrPosInferenceLemmas = arrInferenceBuffer;
            arrInferenceBuffer += nNumInferences;
            for (int j = 0; j < nNumInferences; j++)
            {
               LemmaBitEncoding lbe3 = lbe2.AddLiteral(arrInferences[j], true);
               pTransition->arrPosInferenceLemmas[j] = FindOrAddLemma(lbe3);
            }
         }

         // Place a lemma on each negative inference.
         nNumInferences = pTransition->negativeInferences.nNumElts;
         arrInferences = pTransition->negativeInferences.arrElts;
         assert(pTransition->arrNegInferenceLemmas == 0);
         if (nNumInferences) {
            pTransition->arrNegInferenceLemmas = arrInferenceBuffer;
            arrInferenceBuffer += nNumInferences;
            for (int j = 0; j < nNumInferences; j++)
            {
               LemmaBitEncoding lbe3 = lbe2.AddLiteral(arrInferences[j], false);
               pTransition->arrNegInferenceLemmas[j] = FindOrAddLemma(lbe3);
            }
         }

         // Place the resulting state in the lemmatize queue
         // so that we can continue our breadthfirst search.
         pNextState = pTransition->pNextState;
         if (pNextState->cFlag != 1)
         {
            pNextState->cFlag = 1;
            ltq.Enqueue(pNextState, lbe2);
         }
      }
   }

}


ITE_INLINE
LemmaBitEncoding::
LemmaBitEncoding()
{
   unEncoding = 0;
}

ITE_INLINE
LemmaBitEncoding
LemmaBitEncoding::
AddLiteral(int nVble, bool bPolarity)
   // Add a literal mentioning nVble to the current assignment encoding.
   // The way we do this is:
   // Let n be the mapping of the variable for the current constraint.
   // (See ComputeVariableMapping() below.)
   // If bPolarity is false then add (1 << (2*n)) to the current encoding.
   // O.w., add (1 << (2*n + 1)).
   // The advantage of this encoding is that it will be quick to
   // check whether one path/set-of-literals is a subset of another
   // (subsumption).
{
   // Get relative position of vble index in the current constraint.
   int nVbleMapping = arrVbleMapping[nVble];
   assert(nVbleMapping >= 0 && nVbleMapping < MAX_VBLES_PER_SMURF);

   // Determine which bit to set.
   unsigned long unBitIndex = 2 * nVbleMapping;
   if (bPolarity)
   {
      unBitIndex++;
   }

   return LemmaBitEncoding(unEncoding | (1 << unBitIndex));
}

ITE_INLINE bool
LemmaBitEncoding::
SubsumedBy(LemmaBitEncoding lbe)
   // Return true iff the literals mentioned in lbe are a subset
   // of the literals mentioned in *self.
{
   return ((~unEncoding & lbe.unEncoding) == 0);
}

ITE_INLINE LemmaBlock *
LemmaBitEncoding::
EnterNewLemma()
   // The lemma represented by the LemmaBitEncoding is entered into
   // the Lemma Space, and a pointer to its first LemmaBlock is returned.
   // The lemma is also entered into the lemma lookup space so that it
   // will not need to be created more than once for the current state machine.
{
   unsigned long unBitPattern = 1;
   int nNumElts = 0;
   int arrLemmaLiterals[2*(MAX_MAX_VBLES_PER_SMURF+1)];

   for (int i = 0; i < 2 * MAX_VBLES_PER_SMURF; i++)
   {
      if (unBitPattern & unEncoding)
      {
         nNumElts++;
      }
      unBitPattern = unBitPattern << 1;
   }

   unBitPattern = 1;
   int nBitIndex = 0;
   int nLiteral;
   int nArrayIndex = 0;  // Index into arrLemmaLiterals[].
   for (int i = 0; i < nNumElts; i++)
   {
      while ((unBitPattern & unEncoding) == 0)
      {
         unBitPattern = unBitPattern << 1;
         nBitIndex++;
         assert(nBitIndex < 2 * MAX_VBLES_PER_SMURF);
      }

      // Get the index of the variable mentioned by the literal.
      nLiteral = reverseVariableMapping.arrElts[nBitIndex / 2];

      // Negative literals are represented in lemmas by negative numbers.
      if ((nBitIndex % 2) == 0)
      {
         nLiteral = -nLiteral;
      }

      arrLemmaLiterals[nArrayIndex++] = nLiteral;
      unBitPattern = unBitPattern << 1;
      nBitIndex++;
   }

   int nNumBlocks;
   LemmaBlock *pReturn;
   LemmaBlock *pLastBlock;
   EnterIntoLemmaSpace(nNumElts, arrLemmaLiterals, 
         false, pReturn, pLastBlock,
         nNumBlocks);

   // Enter the new lemma into the lemma lookup space.
   arrLemmaLookupSpace[nNumPrimeImplicants].lbe = *this;
   arrLemmaLookupSpace[nNumPrimeImplicants].pLemma = pReturn;
   nNumPrimeImplicants++;

   return pReturn;
}

ITE_INLINE
LemmaBitEncoding::
LemmaBitEncoding(unsigned long unNewEncoding)
{
   unEncoding = unNewEncoding;
}

ITE_INLINE
void
InitLemmaLookupSpace()
{
   assert(!arrLemmaLookupSpace);
   ITE_NEW_CATCH(
         arrLemmaLookupSpace = new LemmaLookupStruct[MAX_NUM_PRIME_IMPLICANTS],
         "Unable to allocate lemma lookup space.");

   nNumPrimeImplicants = 0;
}

ITE_INLINE
void
ResetLemmaLookupSpace()
{
   nNumPrimeImplicants = 0;
}

ITE_INLINE
void
FreeLemmaLookupSpace()
{
   assert(arrLemmaLookupSpace);
   delete[] arrLemmaLookupSpace;
   arrLemmaLookupSpace = 0;
}

ITE_INLINE
LemmatizeQueue::
LemmatizeQueue()
{
   pFirst = pLast = 0;
}

ITE_INLINE
void
LemmatizeQueue::
Enqueue(SmurfState *pState, LemmaBitEncoding lbe)
{
   if (!pFirst)
   {
      assert(!pLast);
      ITE_NEW_CATCH(
            pFirst = pLast = new LemmatizeQueueCell,
            "Not enough memory to compute lemmas.");

      pFirst->pState = pState;
      pFirst->lbe = lbe;
      pFirst->pNext = 0;
      return;
   }

   LemmatizeQueueCell *pFormerLast = pLast;
   assert(!pFormerLast->pNext);
   ITE_NEW_CATCH(
         pLast = pFormerLast->pNext = new LemmatizeQueueCell,
         "Not enough memory to compute lemmas.");

   pLast->pState = pState;
   pLast->lbe = lbe;
   pLast->pNext = 0;
   return;
}

ITE_INLINE
void
LemmatizeQueue::
Dequeue(SmurfState *&pState, LemmaBitEncoding &lbe)
{
   assert(pFirst);
   pState = pFirst->pState;
   lbe = pFirst->lbe;

   LemmatizeQueueCell *pToBeDeleted = pFirst;
   pFirst = pFirst->pNext;

   if (!pFirst)
   {
      pLast = 0;
   }

   delete pToBeDeleted;
}

ITE_INLINE
bool
LemmatizeQueue::
IsEmpty()
{
   return (pFirst == 0);
}

ITE_INLINE LemmaBlock *
FindOrAddLemma(LemmaBitEncoding lbe)
{
   for (int i = 0; i < nNumPrimeImplicants; i++)
   {
      if (lbe.SubsumedBy(arrLemmaLookupSpace[i].lbe))
      {
         return arrLemmaLookupSpace[i].pLemma;
      }
   }

   return lbe.EnterNewLemma();
}

// ----------------------- special functions lemma construction -------------------------
ITE_INLINE
void
ConstructLemmasForLongAndEquals(SpecialFunc *pSpecialFunc)
{
   int nNumRHSVbles = pSpecialFunc->rhsVbles.nNumElts;
   int *arrRHSVbles = pSpecialFunc->rhsVbles.arrElts;
   int *arrRHSPolarities = pSpecialFunc->arrRHSPolarities;
   int nLHSVble = pSpecialFunc->nLHSVble;
   LemmaBlock *pFirstBlock;
   LemmaBlock *pLastBlock;
   int nNumBlocks;
   int nNumElts;  // Number of literals in the lemma.

   ///////////////////////////
   // Construct long lemma. //
   ///////////////////////////

   // nLHSVble would be 0 in a PLAINOR constraint.
   // We do not want to put vble 0 in our lemma.
   bool bSkipLHSLit = (nLHSVble == 0 ? true : false);

   // Store lemma length.
   nNumElts
      = (bSkipLHSLit ? nNumRHSVbles : nNumRHSVbles + 1);

   int *arrLits;
   ITE_NEW_CATCH(
         arrLits = new int[nNumElts], "arrLits")
      int nLitIndex = 0;

   // Store literals.
   if (!bSkipLHSLit)
   {
      arrLits[nLitIndex++]
         = ((pSpecialFunc->nLHSPolarity == BOOL_TRUE) ? nLHSVble : -nLHSVble);
   }

   for (int i = 0; i < nNumRHSVbles; i++)
   {
      arrLits[nLitIndex++]
         = ((arrRHSPolarities[i] == BOOL_TRUE)
               ? -arrRHSVbles[i] : arrRHSVbles[i]);
   }

   EnterIntoLemmaSpace(nNumElts, arrLits,
         false, pFirstBlock, pLastBlock, nNumBlocks);
   pSpecialFunc->pLongLemma = pFirstBlock;

   ////////////////////////////////////
   // Construct set of short lemmas. //
   ////////////////////////////////////
   pSpecialFunc->arrShortLemmas 
      = (LemmaBlock **)ite_calloc(nNumRHSVbles, sizeof(LemmaBlock*),
            9, "pSpecialFunc->arrShortLemmas");
   nTotalBytesForLemmaInferences  += nNumRHSVbles*sizeof(LemmaBlock*);
   LemmaBlock **arrShortLemmas = pSpecialFunc->arrShortLemmas;
   for (int i = 0; i < nNumRHSVbles; i++)
   {
      // Store literals.  Each short lemma has exactly two literals.
      arrLits[0]
         = ((pSpecialFunc->nLHSPolarity == BOOL_TRUE) ? -nLHSVble : nLHSVble);
      arrLits[1]
         = ((arrRHSPolarities[i] == BOOL_TRUE)
               ? arrRHSVbles[i] : -arrRHSVbles[i]);

      EnterIntoLemmaSpace(2, arrLits, false,
            pFirstBlock, pLastBlock, nNumBlocks);
      arrShortLemmas[i] = pFirstBlock;
   }

   delete [] arrLits;
}

ITE_INLINE
void
ConstructLemmasForLongXorEquals(SpecialFunc *pSpecialFunc)
{
   int nNumRHSVbles = pSpecialFunc->rhsVbles.nNumElts;
   int *arrRHSVbles = pSpecialFunc->rhsVbles.arrElts;
   LemmaBlock *pFirstBlock;
   LemmaBlock *pLastBlock;
   int nNumBlocks;

   ///////////////////////////
   // Construct long lemma. //
   ///////////////////////////
   // THIS IS USELESS -- WE NEED TO ALLOCATE THE SPACE ONLY
   // but I left it in here anyway - m
   ///////////////////////////

   int *arrLits;
   ITE_NEW_CATCH(
         arrLits = new int[nNumRHSVbles], "arrLits")
      int nLitIndex = 0;

   for (int i = 0; i < nNumRHSVbles; i++)
   {
      arrLits[nLitIndex++] = arrRHSVbles[i];
   }

   EnterIntoLemmaSpace(nNumRHSVbles, arrLits,
         false, pFirstBlock, pLastBlock, nNumBlocks);
   pSpecialFunc->pLongLemma = pFirstBlock;
   delete [] arrLits;
}

// ------------------------------ lemma display -----------------------------------
ITE_INLINE void
DisplayLemma(int *pnLemma)
{
   int nLength = *pnLemma++;

   for (int i = 0; i < nLength; i++)
   {
      fprintf(stddbg, "%d ",  pnLemma[i]);
   }
   fprintf(stddbg, "\n");
}

ITE_INLINE void
DisplayLemma(LemmaBlock *pLemma)
{
   LemmaBlock *pLemmaBlock = pLemma;
   int *arrLits = pLemmaBlock->arrLits;
   int nLemmaLength = arrLits[0];
   fprintf(stddbg, "(len: %d) ", nLemmaLength);
   int nLitIndex;
   int nLitIndexInBlock;
   for (nLitIndex = 1, nLitIndexInBlock = 1;
         nLitIndex <= nLemmaLength;
         nLitIndex++, nLitIndexInBlock++)
   {
      if (nLitIndexInBlock == LITS_PER_LEMMA_BLOCK)
      {
         nLitIndexInBlock = 0;
         pLemmaBlock = pLemmaBlock->pNext;
         arrLits = pLemmaBlock->arrLits;
      }
      fprintf(stddbg, "%d ", arrLits[nLitIndexInBlock]);
   }
   fprintf(stddbg, "\n");
}

ITE_INLINE void
DisplayLemmaToFile(FILE *pFile, LemmaBlock *pLemma)
{
   LemmaBlock *pLemmaBlock = pLemma;
   int *arrLits = pLemmaBlock->arrLits;
   int nLemmaLength = arrLits[0];
   int nLitIndex;
   int nLitIndexInBlock;
   for (nLitIndex = 1, nLitIndexInBlock = 1;
         nLitIndex <= nLemmaLength;
         nLitIndex++, nLitIndexInBlock++)
   {
      if (nLitIndexInBlock == LITS_PER_LEMMA_BLOCK)
      {
         nLitIndexInBlock = 0;
         pLemmaBlock = pLemmaBlock->pNext;
         arrLits = pLemmaBlock->arrLits;
      }
      //cout << arrLits[nLitIndexInBlock] << " ";
      fprintf(pFile, "%d ", arrLits[nLitIndexInBlock]);
   }
   fprintf(pFile, "0");
}

ITE_INLINE void
DisplayLemmaStatus(LemmaBlock *pLemma)
{
   int nNumSat = 0;
   int nNumNotContradicted = 0;
   LemmaBlock *pLemmaBlock = pLemma;
   int *arrLits = pLemmaBlock->arrLits;
   int nLemmaLength = arrLits[0];
   int nLitIndex;
   int nLitIndexInBlock;
   for (nLitIndex = 1, nLitIndexInBlock = 1;
         nLitIndex <= nLemmaLength;
         nLitIndex++, nLitIndexInBlock++)
   {
      if (nLitIndexInBlock == LITS_PER_LEMMA_BLOCK)
      {
         nLitIndexInBlock = 0;
         pLemmaBlock = pLemmaBlock->pNext;
         arrLits = pLemmaBlock->arrLits;
      }
      int nLit = arrLits[nLitIndexInBlock];
      assert(nLit != 0);
      int nVble = abs(nLit);
      if (arrSolution[nVble] == BOOL_UNKNOWN)
      {
         nNumNotContradicted++;
         fprintf(stddbg, " *");
      }
      else if (arrSolution[nVble] == BOOL_TRUE)
      {
         if (nLit > 0)
         {
            nNumSat++;
            nNumNotContradicted++;
            fprintf(stddbg, " sat");
         }
         else
         {
            fprintf(stddbg, " unsat");
         }
      }
      else
      {
         assert(arrSolution[nVble] == BOOL_FALSE);
         if (nLit < 0)
         {
            nNumSat++;
            nNumNotContradicted++;
            fprintf(stddbg, " sat");
         }
         else
         {
            fprintf(stddbg, " unsat");
         }
      }
      fprintf(stddbg, "(%d)", nLit);
   }
   fprintf(stddbg, "\n");
   /* 
    cout << "# Sat: " << nNumSat 
    << "  # not contradicted: " << nNumNotContradicted
    << endl;
    */
}

ITE_INLINE void
DisplayLemmaLevels(LemmaBlock *pLemma, int arrLevel[])
{
   cout << "Levels:" << endl;

   LemmaBlock *pLemmaBlock = pLemma;
   int *arrLits = pLemmaBlock->arrLits;
   int nLemmaLength = arrLits[0];
   int nLitIndex; // Index relative to the whole lemma.
   int nLitIndexInBlock; // Index relative to the individual
   // lemma block.  I.e., this is
   // an index into arrLits.
   int nLiteral;
   int nVble;

   for (nLitIndex = 1, nLitIndexInBlock = 1;
         nLitIndex <= nLemmaLength;
         nLitIndex++, nLitIndexInBlock++)
   {
      if (nLitIndexInBlock == LITS_PER_LEMMA_BLOCK)
      {
         nLitIndexInBlock = 0;
         pLemmaBlock = pLemmaBlock->pNext;
         arrLits = pLemmaBlock->arrLits;
      }
      nLiteral = arrLits[nLitIndexInBlock];
      nVble = abs(nLiteral);
      cout << nLiteral << ":" << arrLevel[nVble] << " ";
   }
   cout << endl;
}

ITE_INLINE void
DisplayLemmaInfo(LemmaInfoStruct *pLemmaInfo)
{
   fprintf(stddbg, "nWatchedVble[0]: %d nWatchedVble[1]: %d\n",
         pLemmaInfo->nWatchedVble[0], pLemmaInfo->nWatchedVble[1]);
   fprintf(stddbg, "nWatchedVblePolarity[0]: %d nWatchedVblePolarity[1]: %d\n",
         pLemmaInfo->nWatchedVblePolarity[0], pLemmaInfo->nWatchedVblePolarity[1]);
   DisplayLemmaStatus(pLemmaInfo->pLemma);
}

ITE_INLINE void
DisplayLemmaList(int nVble, int nPos, int nWhichWatch)
{
   AffectedFuncsStruct *pAFS = arrAFS + nVble;
   LemmaInfoStruct *pLemmaInfo;
   cout << "DisplayLemmaList() " << endl;
   if (nPos == 1)
   {
      if (nWhichWatch == 1)
      {
         pLemmaInfo = &(pAFS->LemmasWherePos[0]);
      }
      else
      {
         assert(nWhichWatch == 2);
         pLemmaInfo = &(pAFS->LemmasWherePos[1]);
      }
   }
   else
   {
      assert(nPos == 0);
      if (nWhichWatch == 1)
      {
         pLemmaInfo = &(pAFS->LemmasWhereNeg[0]);
      }
      else
      {
         assert(nWhichWatch == 2);
         pLemmaInfo = &(pAFS->LemmasWhereNeg[1]);
      }
   }

   int nCount = 0;

   do
   {
      pLemmaInfo = (nWhichWatch == 1 ? pLemmaInfo->pNextLemma[0]
            : pLemmaInfo->pNextLemma[1]);
      cout << pLemmaInfo << " ";
      nCount++;
   }
   while (pLemmaInfo && nCount < 100);
   cout << endl;
}

// ------------------------------- checking and verification -------------------------------


ITE_INLINE bool
IsInLemmaList(LemmaInfoStruct *pLemmaInfo, LemmaInfoStruct *pList)
{
   if (!pLemmaInfo)
   {
      return true;
   }

   for (LemmaInfoStruct *p = pList; p; p = p->pNextLemma[0])
   {
      if (p == pLemmaInfo)
      {
         return true;
      }
   }

   return false;
}

ITE_INLINE bool
LemmasEqual(int nLength1, int arrLits1[],
      int nLength2, int arrLits2[])
{
   if (nLength1 != nLength2)
   {
      return false;
   }

   for (int i = 0; i < nLength1; i++)
   {
      if (arrLits1[i] != arrLits2[i])
      {
         return false;
      }
   }

   return true;
}

ITE_INLINE void
CheckLengthOfLemmaList(int nVble, int nPos, int nWhichWatch,
      int nNumBacktracks)
{
   AffectedFuncsStruct *pAFS = arrAFS + nVble;
   LemmaInfoStruct *pLemmaInfo;
   if (nPos == 1)
   {
      if (nWhichWatch == 1)
      {
         pLemmaInfo = &(pAFS->LemmasWherePos[0]);
      }
      else
      {
         assert(nWhichWatch == 2);
         pLemmaInfo = &(pAFS->LemmasWherePos[1]);
      }
   }
   else
   {
      assert(nPos == 0);
      if (nWhichWatch == 1)
      {
         pLemmaInfo = &(pAFS->LemmasWhereNeg[0]);
      }
      else
      {
         assert(nWhichWatch == 2);
         pLemmaInfo = &(pAFS->LemmasWhereNeg[1]);
      }
   }

   int nCount = 0;

   do
   {
      pLemmaInfo = (nWhichWatch == 1 ? pLemmaInfo->pNextLemma[0]
            : pLemmaInfo->pNextLemma[1]);
      //cout << pLemmaInfo << " ";
      nCount++;
   }
   while (pLemmaInfo && nCount < 1000);
   
   if (nCount >= 1000)
   {
      cout << "CheckLengthOfLemmaList -- nNumBacktracks = "
         << ite_counters[NUM_BACKTRACKS] << endl;
      assert(0);
   }
}

ITE_INLINE void
VerifyLemmaList(int nVble, int nPos, int nWhichWatch)
{
   AffectedFuncsStruct *pAFS = arrAFS + nVble;
   LemmaInfoStruct *pLemmaInfo;
   LemmaInfoStruct *pPrev;
   if (nPos == 1)
   {
      if (nWhichWatch == 1)
      {
         pPrev = &(pAFS->LemmasWherePos[0]);
      }
      else
      {
         assert(nWhichWatch == 2);
         pPrev = &(pAFS->LemmasWherePos[1]);
      }
   }
   else
   {
      assert(nPos == 0);
      if (nWhichWatch == 1)
      {
         pPrev = &(pAFS->LemmasWhereNeg[0]);
      }
      else
      {
         assert(nWhichWatch == 2);
         pPrev = &(pAFS->LemmasWhereNeg[1]);
      }
   }

   int nCount = 0;

   do
   {
      pLemmaInfo = (nWhichWatch == 1 ? pPrev->pNextLemma[0]
            : pPrev->pNextLemma[1]);
      if (!pLemmaInfo)
      {
         break;
      }
#ifndef NDEBUG
      int nWatchedVble = (nWhichWatch == 1 ? pLemmaInfo->nWatchedVble[0]
            : pLemmaInfo->nWatchedVble[1]);
      assert(nWatchedVble == nVble);
      int nWatchedVblePolarity = (nWhichWatch == 1
            ? pLemmaInfo->nWatchedVblePolarity[0]
            : pLemmaInfo->nWatchedVblePolarity[1]);
      assert(nWatchedVblePolarity == nPos);
      LemmaInfoStruct *pStoredPrev = (nWhichWatch == 1
            ? pLemmaInfo->pPrevLemma[0]
            : pLemmaInfo->pPrevLemma[1]);
      assert(pPrev == pStoredPrev);
#endif
      nCount++;
      pPrev = pLemmaInfo;
   }
   while (true);
   //while (nCount < 1000);
   //assert(nCount < 1000);
}

ITE_INLINE void
VerifyLemmaLists()
{
   extern int gnMaxVbleIndex;
   for (int nVble = 1; nVble <= gnMaxVbleIndex; nVble++)
   {
      VerifyLemmaList(nVble, 1, 1);
      VerifyLemmaList(nVble, 1, 2);
      VerifyLemmaList(nVble, 0, 1);
      VerifyLemmaList(nVble, 0, 2);
   }
}

ITE_INLINE bool
CheckLemma(LemmaInfoStruct &lemma, LemmaInfoStruct *pUnitLemmaList)
// Returns true if an error is found.
// Detects three types of errors:  
// (1) a lemma wrongly reporting being satisfied,
// (2) an unfired unit lemma, and
// (3) a contradicted lemma.
{
   //int *pnLemma = lemma.pnLemma;
   LemmaBlock *pLemmaBlock = lemma.pLemma;
   int *arrLits = pLemmaBlock->arrLits;
   int nLemmaLength = arrLits[0];
   //int *pnLiteral = pnLemma + 1;
   int nLitIndex;
   int nLitIndexInBlock;
   int nNumUnknown = 0;
   int nLiteral;
   int nVble;
   int nVbleValue;

   for (nLitIndex = 1, nLitIndexInBlock = 1;
         nLitIndex <= nLemmaLength;
         nLitIndex++, nLitIndexInBlock++)
   {
      if (nLitIndexInBlock == LITS_PER_LEMMA_BLOCK)
      {
         nLitIndexInBlock = 0;
         pLemmaBlock = pLemmaBlock->pNext;
         arrLits = pLemmaBlock->arrLits;
      }
      nLiteral = arrLits[nLitIndexInBlock];
      nVble = abs(nLiteral);
      nVbleValue = arrSolution[nVble];
      if (nVbleValue == BOOL_UNKNOWN)
      {
         nNumUnknown++;
      }
      else
      {
         if ((nLiteral > 0) == (nVbleValue == BOOL_TRUE))
         {
            // Lemma is satisfied.
            return false;
         }
      }
   }

   if (nNumUnknown == 0)
   {
      cout << endl << "ERROR -- Contradicted lemma:" << endl;
      DisplayLemmaInfo(&lemma);
      cout << endl;
      return true;
   }
   else if (nNumUnknown == 1)
   {
      cout << endl << "ERROR -- Unfired unit lemma:" << endl;
      DisplayLemmaInfo(&lemma);
      cout << endl;

      bool bFound = false;
      LemmaInfoStruct *p = pUnitLemmaList->pNextLemma[0];
      while (!bFound && p)
      {
         if (p == &lemma)
         {
            bFound = true;
         }
         else
         {
            p = p->pNextLemma[0];
         }
      }
      if (bFound)
      {
         cout << "Lemma is in unit lemma list" << endl;
      }
      else
      {
         cout << "Lemma is not in unit lemma list" << endl;
      }

      return true;
   }
   return false;
}

