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

LemmaBlock *pLemmaSpaceNextAvail = NULL;
int nTotalBytesForLemmaInferences  = 0;
	
// The next two variables are entry points into the Lemma Priority Queue.
// This queue is a doubly linked list of the LemmaInfoStructs of
// all of the lemmas that have been entered into the "Lemma Cache".
// Thus, only the brancher enters lemmas into this queue (never
// the Smurf Factory).  Whenever a lemma causes an inference, it
// is moved to the front of the queue.
// Whenever we run out of lemma space, we delete lemmas from the back
// of the queue and recover their LemmaBlocks for the lemma space.
LemmaInfoStruct *pLPQFirst = NULL;
LemmaInfoStruct *pLPQLast = NULL;

LemmaLookupStruct *arrLemmaLookupSpace = NULL;
int nNumPrimeImplicants;
SmurfState **arrPrecomputedStates = NULL;
int *arrVbleMapping = NULL;
unsigned long *arrVbleEncoding = NULL;
IntegerSet_ArrayBased reverseVariableMapping;
int gnNumLemmas = 0;

typedef struct _tarrLemmaInfo {
   LemmaInfoStruct *memory;
   struct _tarrLemmaInfo *next;
} tarrLemmaInfo;
tarrLemmaInfo *head_arrLemmaInfo = NULL;

LemmaInfoStruct *pFreeLemmaInfoStructPool = NULL;
int *arrBacktrackStackIndex;

/*****************************************************/

ITE_INLINE
int lemma_compfunc(const void *x, const void *y) {
  int pp, qq;
  
  pp = *(const int *)x;
  qq = *(const int *)y;
  if (arrBacktrackStackIndex[pp] < arrBacktrackStackIndex[qq]) return -1;
  if (arrBacktrackStackIndex[pp] == arrBacktrackStackIndex[qq])
#ifndef FORCE_STABLE_QSORT
   return 0;
#else
  { if (x < y) return -1;
    else if (x > y) return 1;
    else return 0;
  }
#endif
  return 1;
}

/*****************************************************/



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

ITE_INLINE
bool
LemmaBitEncoding::
SubsumedBy(LemmaBitEncoding lbe)
   // Return true iff the literals mentioned in lbe are a subset
   // of the literals mentioned in *self.
{
   return ((~unEncoding & lbe.unEncoding) == 0);
}

ITE_INLINE
void
EnterIntoLemmaSpace(int nNumElts,
      int arrLemmaLiterals[],
      bool bRecycleLemmasAsNeeded,
      LemmaBlock *&pFirstBlock,
      LemmaBlock *&pLastBlock,
      int &nNumBlocks)
// nNumElts is the number of literals in a lemma.
// arrLemmLiterals contains the literals.
// The routine allocates from the lemma space enough lemma blocks
// to hold the lemmas and fills them with nNumElts+2 integers.
// The first integer will be nNumElts, the next nNumElts integers
// will be the entries in arrLemmaLiterals.
// The last integer entered will be 0.
// If there is not enough blocks available in the Lemma Space
// then bRecycleLemmasAsNeeded is tested.
// If it is true, then an attempt is made to free enough old
// lemmas to make the request.
// If there are not enough blocks available, and bRecycleLemmasAsNeeded
// is false, then the program aborts.
// Aborts if there is not enough Lemma Space available
// to hold the lemma.
// Assuming that there are enough free blocks, then the allocated
// blocks get linked together.
// A postcondition is that pFirstBlock points to the first block
// of the newly allocated lemma, and that pLastBlock points to its
// last block.
{
   extern int gnNumCachedLemmas;
   int nNumEntries = nNumElts + 2; /* add the size and the ending 0 */
   int nNumBlocksRequired = nNumEntries / LITS_PER_LEMMA_BLOCK +
      (nNumEntries % LITS_PER_LEMMA_BLOCK>0 ? 1: 0);

   if (nHeuristic == C_LEMMA_HEURISTIC
#ifdef JOHNSON_HEURISTIC_LEMMA
          || nHeuristic == JOHNSON_HEURISTIC
#endif
          ) 
      UpdateHeuristicWithLemma(nNumElts, arrLemmaLiterals);

   if (MAX_NUM_CACHED_LEMMAS && gnNumCachedLemmas >= MAX_NUM_CACHED_LEMMAS)
   {
      FreeLemmas(gnNumCachedLemmas - MAX_NUM_CACHED_LEMMAS + 1);
   }

   if (nNumBlocksRequired > nLemmaSpaceBlocksAvail)
   {
      AllocateMoreLemmaSpace(nNumBlocksRequired-nLemmaSpaceBlocksAvail);
   }

   LemmaBlock *pCurrentBlock = pFirstBlock = pLemmaSpaceNextAvail;
   int *arrLits = pCurrentBlock->arrLits;
   int i = 0;  // Index into the arrLits[] of the current block.
   int j = 0; // Index into arrLemmaLiterals[].

   arrLits[i++] = nNumElts;

   if (nNumBlocksRequired > 1) {
      for (int nBlockNum = 1; nBlockNum < nNumBlocksRequired; nBlockNum++)
      {
         for (; i < LITS_PER_LEMMA_BLOCK; i++)
         {
            arrLits[i] = arrLemmaLiterals[j++];
         }
         i = 0; /* first block was shorter by 1 for the size */
         pCurrentBlock = pCurrentBlock->pNext;
         arrLits = pCurrentBlock->arrLits;
      }
   }

   while (j < nNumElts)
   {
      arrLits[i++] = arrLemmaLiterals[j++];
   }
   arrLits[i] = 0; 

   // Last block.
   pLastBlock = pCurrentBlock;
   nNumBlocks = nNumBlocksRequired;
   nLemmaSpaceBlocksAvail -= nNumBlocksRequired;
   pLemmaSpaceNextAvail = pLastBlock->pNext;
   pLastBlock->pNext = 0;
}

ITE_INLINE
void
FillLemmaWithReversedPolarities(LemmaBlock *pLemma)
{
   LemmaBlock *pCurrentBlock = pLemma;
   int *arrLits = pCurrentBlock->arrLits;
   int i = 0;  // Index into the arrLits[] of the current block.
   int j = 0; // Index into arrLemmaLiterals[].

   int nNumElts = arrLits[i++];
   int nNumEntries = nNumElts + 2; /* add the size and the ending 0 */
   int nNumBlocksRequired = nNumEntries / LITS_PER_LEMMA_BLOCK +
      (nNumEntries % LITS_PER_LEMMA_BLOCK>0 ? 1: 0);
   int litsPerBlock = LITS_PER_LEMMA_BLOCK;

   if (nNumBlocksRequired > 1) {
      for (int nBlockNum = 1; nBlockNum < nNumBlocksRequired; nBlockNum++)
      {
         for (; i < litsPerBlock; i++)
         {
            assert(arrSolution[abs(arrLits[i])] != BOOL_UNKNOWN);
            arrLits[i] = (arrSolution[abs(arrLits[i])]==BOOL_FALSE?1:-1)*
               abs(arrLits[i]);
            j++;
         }
         i = 0; /* first block was shorter by 1 for the size */
         pCurrentBlock = pCurrentBlock->pNext;
         arrLits = pCurrentBlock->arrLits;
      }
   }

   while (j < nNumElts)
   {
      assert(arrSolution[abs(arrLits[i])] != BOOL_UNKNOWN);
      arrLits[i] = (arrSolution[abs(arrLits[i])]==BOOL_FALSE?1:-1)*
         abs(arrLits[i]);
      i++;
      j++;
   }
   assert(arrLits[i] == 0); 
}

ITE_INLINE
LemmaBlock *
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

ITE_INLINE
void
DisplayAllBrancherLemmas()
{
   cout << "Displaying ALL lemmas " << endl;

   for (LemmaInfoStruct *pLemmaInfo = pLPQFirst;
         pLemmaInfo;
         pLemmaInfo = pLemmaInfo->pLPQNext)
   {
      DisplayLemma(pLemmaInfo->pLemma);
      assert((pLemmaInfo == pLPQLast) == (pLemmaInfo->pLPQNext == NULL));
   }
}

ITE_INLINE void
DisplayAllBrancherLemmasToFile(char *filename, int flag)
   // Displays all of the brancher lemmas to a file named "output".
{
   FILE *pFile;

   if ((pFile = fopen(filename==NULL?"output":filename, "wb+")) == NULL)
   {
      cout << "Can't open 'output' for writting" << endl;
      exit (1);
   };
   for (LemmaInfoStruct *pLemmaInfo = pLPQFirst;
         pLemmaInfo;
         pLemmaInfo = pLemmaInfo->pLPQNext)
   {
      if (flag == 1)
         fprintf(pFile, "c %d %d %d %d %d %d %d %d\n", 
               pLemmaInfo->pLemma->arrLits[0],
               pLemmaInfo->nNumLemmaMoved,
               pLemmaInfo->nNumLemmaConflict,
               (int)(ite_counters[NUM_LPQ_ENQUEUE]-pLemmaInfo->nLemmaNumber),
               pLemmaInfo->nNumLemmaInfs,
               pLemmaInfo->nBacktrackStackReferences,
               pLemmaInfo->nLemmaFirstUseful?
               (int)(pLemmaInfo->nLemmaFirstUseful-pLemmaInfo->nLemmaNumber):0,
               pLemmaInfo->nLemmaLastUsed?
               (int)(pLemmaInfo->nLemmaLastUsed-pLemmaInfo->nLemmaNumber):0);
      DisplayLemmaToFile(pFile, pLemmaInfo->pLemma);
      fprintf(pFile, "\n");
      assert((pLemmaInfo == pLPQLast) == (pLemmaInfo->pLPQNext == NULL));
   }
}

ITE_INLINE
LemmaBlock *
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

ITE_INLINE
void
DisplayLemma(int *pnLemma)
{
   int nLength = *pnLemma++;

   for (int i = 0; i < nLength; i++)
   {
      fprintf(stddbg, "%d ",  pnLemma[i]);
   }
   fprintf(stddbg, "\n");
}

ITE_INLINE
void
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

ITE_INLINE
void
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

ITE_INLINE
void
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

ITE_INLINE
void
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

ITE_INLINE
void
DisplayLemmaInfo(LemmaInfoStruct *pLemmaInfo)
{
   fprintf(stddbg, "nWatchedVble[0]: %d nWatchedVble[1]: %d\n",
         pLemmaInfo->nWatchedVble[0], pLemmaInfo->nWatchedVble[1]);
   fprintf(stddbg, "nWatchedVblePolarity[0]: %d nWatchedVblePolarity[1]: %d\n",
         pLemmaInfo->nWatchedVblePolarity[0], pLemmaInfo->nWatchedVblePolarity[1]);
   DisplayLemmaStatus(pLemmaInfo->pLemma);
}

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

ITE_INLINE
void
InitLemmaInfoArray()
{
   LemmaInfoStruct *garrLemmaInfo = NULL;

   assert(MAX_NUM_LEMMAS);
   //assert(garrLemmaInfo==NULL);
   assert(pFreeLemmaInfoStructPool==NULL);
   garrLemmaInfo = (LemmaInfoStruct*)ite_calloc(MAX_NUM_LEMMAS, sizeof(LemmaInfoStruct),
         2, "lemma info array");

   for (int i = 0; i < MAX_NUM_LEMMAS - 1; i++)
   {
      garrLemmaInfo[i].pNextLemma[0] = garrLemmaInfo + i + 1;
   }
   //garrLemmaInfo[MAX_NUM_LEMMAS - 1].pNextLemma[0] = NULL; -- it is NULL already
   pFreeLemmaInfoStructPool = garrLemmaInfo;

   tarrLemmaInfo *t = (tarrLemmaInfo*)ite_calloc(1, sizeof(tarrLemmaInfo), 9, "lemma info array ptr");
   t->memory = garrLemmaInfo;
   if (head_arrLemmaInfo == NULL) { head_arrLemmaInfo = t; t->next = NULL; }
   else t->next = head_arrLemmaInfo;
}

ITE_INLINE
void
FreeLemmaInfoArray()
{
   while (head_arrLemmaInfo) {
      tarrLemmaInfo *t = head_arrLemmaInfo->next;
      ite_free((void**)&head_arrLemmaInfo->memory);
      ite_free((void**)&head_arrLemmaInfo);
      head_arrLemmaInfo = t;
   }
   pFreeLemmaInfoStructPool = NULL;
}

ITE_INLINE
bool
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

ITE_INLINE
bool
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

ITE_INLINE
void
FreeLemmaBlocks(LemmaInfoStruct *pLemmaInfo)
   // LemmaBlocks are returned to the pool pointed to by
   // pLemmaSpaceNextAvail.
{
   pLemmaInfo->pLemmaLastBlock->pNext = pLemmaSpaceNextAvail;
   pLemmaSpaceNextAvail = pLemmaInfo->pLemma;
   nLemmaSpaceBlocksAvail += pLemmaInfo->nNumBlocks;
}

ITE_INLINE
void
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

ITE_INLINE
void
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

ITE_INLINE
void
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

ITE_INLINE
void
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

ITE_INLINE
void
FreeLemmaInfoStruct(LemmaInfoStruct *pLemmaInfo, bool bIsCached)
   // The LemmaInfoStruct is returned to the pool pointed to by
   // pFreeLemmaInfoStructPool.
{
   extern int gnNumCachedLemmas;
   assert(pLemmaInfo->nBacktrackStackReferences == 0);

   if (bIsCached)
   {
      // Remove it from the lists which it is in based on
      // its watched variables.

      // Watched variable 1 list --
      assert(pLemmaInfo->pPrevLemma[0]);
      pLemmaInfo->pPrevLemma[0]->pNextLemma[0]
         = pLemmaInfo->pNextLemma[0];
      if (pLemmaInfo->pNextLemma[0])
      {
         pLemmaInfo->pNextLemma[0]->pPrevLemma[0]
            = pLemmaInfo->pPrevLemma[0];
      }

      // Watched variable 2 list --
      assert(pLemmaInfo->pPrevLemma[1]);
      pLemmaInfo->pPrevLemma[1]->pNextLemma[1]
         = pLemmaInfo->pNextLemma[1];
      if (pLemmaInfo->pNextLemma[1])
      {
         pLemmaInfo->pNextLemma[1]->pPrevLemma[1]
            = pLemmaInfo->pPrevLemma[1];
      }

      gnNumCachedLemmas--; 
   }

   // Put it in the global pool.
   pLemmaInfo->pNextLemma[0] = pFreeLemmaInfoStructPool;
   pLemmaInfo->pNextLemma[1] = 0;
   pLemmaInfo->pPrevLemma[0] = 0;
   pLemmaInfo->pPrevLemma[1] = 0;
   pFreeLemmaInfoStructPool = pLemmaInfo;
}

ITE_INLINE
void
FreeLemma(LemmaInfoStruct *pLemmaInfo, bool bIsCached)
   // Frees the LemmaInfoStruct and LemmaBlocks for a lemma so that
   // they can be recycled for use in another lemma.
   // bIsCached should be true iff the lemma was already placed in the lemma
   // cache.
{
   //#ifdef LHEURISTIC
   if (nHeuristic == C_LEMMA_HEURISTIC
#ifdef JOHNSON_HEURISTIC_LEMMA
         || nHeuristic == JOHNSON_HEURISTIC
#endif
         )
      RemoveLemmasHeuristicInfluence(pLemmaInfo);
   //#endif // LHEURISTIC
   FreeLemmaBlocks(pLemmaInfo);
   FreeLemmaInfoStruct(pLemmaInfo, bIsCached);
   gnNumLemmas--;
}

ITE_INLINE
void
FreeLemmas(int n)
   // Free enough lemmas in order to free at least n LemmaBlocks.
   // The freed lemmas are taken from the tail of the Lemma Priority Queue,
   // which contains all of the lemmas which have been created
   // by the brancher and moved into the lemma cache.
{
   assert(pLPQLast == pLPQFirst || (pLPQLast != NULL && pLPQFirst != NULL));
   LemmaInfoStruct *p = pLPQLast;
   int nNumBlocksFreed = 0;

   while (nNumBlocksFreed < n)
   {
      if (p == NULL)
      {
         /* need more LemmaStructs */
         dE_printf3("Lemma space exhausted (%d<%d)", nNumBlocksFreed, n);
         assert(0);
         exit(1);
      }

      assert(p->nBacktrackStackReferences >= 0);
      if (p->nBacktrackStackReferences <= 0)
      {
         // Recycle *p.

         // Remove from Lemma Priority Queue.
         if (p->pLPQPrev)
         {
            p->pLPQPrev->pLPQNext = p->pLPQNext;
         }
         if (p->pLPQNext)
         {
            p->pLPQNext->pLPQPrev = p->pLPQPrev;
         }
         // Not going to take the time to update
         // p->pLPQPrev and p->pLPQNext.

         if (p == pLPQLast)
         {
            pLPQLast = p->pLPQPrev;
         }

         if (p == pLPQFirst)
         {
            pLPQFirst = p->pLPQNext;
         }

         nNumBlocksFreed += p->nNumBlocks;
         FreeLemma(p, true);
      }
      p = p->pLPQPrev;
   }
}

ITE_INLINE
LemmaInfoStruct *
AllocateLemmaInfoStruct()
{
   if (!pFreeLemmaInfoStructPool)
   {
      // No available LemmaInfoStructs.
      // So free a lemma.
      // Better -- allocate more LemmaInfoStructs
      //FreeLemmas(1);
      d9_printf1("no pFreeLemmaInfoStructPool available\n");
      InitLemmaInfoArray();
   }

   assert(pFreeLemmaInfoStructPool);

   LemmaInfoStruct *pReturn = pFreeLemmaInfoStructPool;
   pFreeLemmaInfoStructPool = pFreeLemmaInfoStructPool->pNextLemma[0];
   return pReturn;
}

ITE_INLINE
void
LPQEnqueue(LemmaInfoStruct *pLemmaInfo)
   // Place *pLemmaInfo at the front of the Lemma Priority Queue.
{
   assert(!pLPQFirst || !(pLPQFirst->pLPQPrev));
   ite_counters[NUM_LPQ_ENQUEUE]++;
   pLemmaInfo->nNumLemmaMoved=0;
   pLemmaInfo->nNumLemmaConflict=0;
   pLemmaInfo->nNumLemmaInfs=0;
   pLemmaInfo->nLemmaFirstUseful=0;
   pLemmaInfo->nLemmaLastUsed = 0;
   pLemmaInfo->nLemmaNumber=ite_counters[NUM_LPQ_ENQUEUE];

   if (pLPQFirst)
   {
      pLPQFirst->pLPQPrev = pLemmaInfo;
   }
   else
   {
      assert(!pLPQLast);
      pLPQLast = pLemmaInfo;
   }
   pLemmaInfo->pLPQNext = pLPQFirst;
   pLPQFirst = pLemmaInfo;
   pLPQFirst->pLPQPrev = NULL;
}

ITE_INLINE
void
MoveToFrontOfLPQ(LemmaInfoStruct *pLemmaInfo)
   // Precondition:  *pLemmaInfo has already been placed in the Lemma
   // Priority Queue (LPQ).
   // Postcondition:  *pLemmaInfo has moved to the front of the LPQ.
{
   pLemmaInfo->nNumLemmaMoved++;
   pLemmaInfo->nLemmaLastUsed = ite_counters[NUM_LPQ_ENQUEUE];
   if (pLemmaInfo->nLemmaFirstUseful == 0) 
      pLemmaInfo->nLemmaFirstUseful = ite_counters[NUM_LPQ_ENQUEUE];
   LemmaInfoStruct *pPrev = pLemmaInfo->pLPQPrev;
   if (pPrev)
   {
      // Struct is not at the head of the LPQ.  So move it there.
      LemmaInfoStruct *pNext = pLemmaInfo->pLPQNext;
      pPrev->pLPQNext = pNext;
      if (pNext)
      {
         pNext->pLPQPrev = pPrev;
      }
      if (pLemmaInfo == pLPQLast)
      {
         assert(pNext == NULL);
         pLPQLast = pPrev;
      }
      assert(!(pLPQFirst->pLPQPrev));
      pLPQFirst->pLPQPrev = pLemmaInfo;
      pLemmaInfo->pLPQNext = pLPQFirst;
      pLemmaInfo->pLPQPrev = NULL;
      pLPQFirst = pLemmaInfo;
   }
}

ITE_INLINE
LemmaInfoStruct *
AddLemma(int nNumLiterals, int arrLiterals[], bool bPutInCache, 
      LemmaInfoStruct *pUnitLemmaList, LemmaInfoStruct **pUnitLemmaListTail
      )
{
   nCallsToAddLemma++;

   // Initialize lemma info struct.
   LemmaInfoStruct *pLemmaInfo = AllocateLemmaInfoStruct();
   //LPQEnqueue(pLemmaInfo); -- it might not be the cached lemma yet

   //Sort the lemma in the order of the branch process
   //  if(bPutInCache) qsort(arrLiterals, nNumLiterals, sizeof(int), lemma_compfunc);

   // Allocate the lemma blocks and store the literals there.
   EnterIntoLemmaSpace(nNumLiterals, arrLiterals, true,
         pLemmaInfo->pLemma, pLemmaInfo->pLemmaLastBlock,
         pLemmaInfo->nNumBlocks);

   pLemmaInfo->bPutInCache = bPutInCache;
   pLemmaInfo->nBacktrackStackReferences = 0;

   if (bPutInCache==false) {
      pLemmaInfo->nWatchedVble[0] = 0;
      pLemmaInfo->nWatchedVble[1] = 0;
      pLemmaInfo->nWatchedVblePolarity[0] = BOOL_UNKNOWN;
      pLemmaInfo->nWatchedVblePolarity[1] = BOOL_UNKNOWN;
      pLemmaInfo->pNextLemma[0] = NULL;
   } else {
      // Update information concerning which variables affect
      // which lemmas.
      int nLiteral;
      int nWatchedLiteral1 = 0;
      int nWatchedLiteral2 = 0;
      int nWatchedLiteralBSI1 = -1; // "Backtrack Stack Index" of watched lit 1.
      // Specifically, the index on the backtrack stack at which the literal's
      // variable is mentioned.
      int nWatchedLiteralBSI2 = -1;  
      int nCurrentLiteralBSI;

      //Update brancher information for this variable in this lemma.
      //m two highest BSI are Watched Lit 1 and 2
      for (int i = 0; i < nNumLiterals; i++)
      {
         nLiteral = arrLiterals[i];

         nCurrentLiteralBSI = arrBacktrackStackIndex[abs(nLiteral)];
         assert(nCurrentLiteralBSI >= 0);

         if(nCurrentLiteralBSI > nWatchedLiteralBSI1)
         {
            nWatchedLiteral2 = nWatchedLiteral1;
            nWatchedLiteralBSI2 = nWatchedLiteralBSI1;
            nWatchedLiteral1 = nLiteral;
            nWatchedLiteralBSI1 = nCurrentLiteralBSI;
         }
         else if(nCurrentLiteralBSI > nWatchedLiteralBSI2)
         {
            nWatchedLiteral2 = nLiteral;
            nWatchedLiteralBSI2 = nCurrentLiteralBSI;	  
         }

         //assert(arrSolution[abs(nLiteral)] != BOOL_UNKNOWN);
         //assert((nLiteral >= 0) == (arrSolution[abs(nLiteral)] == BOOL_FALSE));
      }

      assert(nWatchedLiteral1 != 0 || nNumLiterals == 0);
      assert(nWatchedLiteral2 != 0 || nNumLiterals <= 1);

      pLemmaInfo->nWatchedVble[0] = abs(nWatchedLiteral1);
      pLemmaInfo->nWatchedVble[1] = abs(nWatchedLiteral2);  
      pLemmaInfo->nWatchedVblePolarity[0] = ((nWatchedLiteral1 >= 0) ? BOOL_TRUE : BOOL_FALSE);
      pLemmaInfo->nWatchedVblePolarity[1] = ((nWatchedLiteral2 >= 0) ? BOOL_TRUE : BOOL_FALSE);
   }

   if (pUnitLemmaList) {
      /* new lemma is put in the beginning of pUnitLemmaList */
      pLemmaInfo->pNextLemma[0] = pUnitLemmaList->pNextLemma[0];
      pUnitLemmaList->pNextLemma[0] = pLemmaInfo;

      if(*pUnitLemmaListTail == NULL)
         *pUnitLemmaListTail = pLemmaInfo;

      assert(IsInLemmaList(*pUnitLemmaListTail, pUnitLemmaList));
   }

   gnNumLemmas++; 

   //Done updating brancher information for this variable in this lemma.

#ifdef DISPLAY_LEMMA_TRACE
   //if (gnNumLemmas == 39)
   if (0)
   {
      cout << "From AddLemma():" << endl;
      DisplayLemmaInfo(pLemmaInfo);
   }
#endif

   TB_9(
      d9_printf1("Adding lemma: ");
      DisplayLemma(pLemmaInfo->pLemma);
      DisplayLemmaStatus(pLemmaInfo->pLemma);
      d9_printf1("\n");
   )

   return pLemmaInfo;
}

ITE_INLINE
bool
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

/* FIXME: if needed
ITE_INLINE
bool
CheckLemmas(LemmaInfoStruct *pUnitLemmaList)
   // Return true if there is a problem with any of the lemmas.
{
   for (int i = 0; i < gnNumLemmas; i++)
   {
      if (CheckLemma(garrLemmaInfo[i], arrSolution, pUnitLemmaList))
      {
         cout << "Error at garrLemmaInfo[" << i << "] "<< garrLemmaInfo + i
            << endl;
         return true;
      }
   }
   return false;
}
*/
