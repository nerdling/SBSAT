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

extern int nTotalBytesForLemmaInferences;
	
LemmaLookupStruct *arrLemmaLookupSpace = NULL;
int nNumPrimeImplicants;
SmurfState **arrPrecomputedStates = NULL;
int *arrVbleMapping = NULL;
unsigned long *arrVbleEncoding = NULL;
IntegerSet_ArrayBased reverseVariableMapping;

// ------------------------------------------------------------------------
// TOC:
//
// ------------------------------------------------------------------------
// ----------------------- Smurf ------------------------------------------
// ----------------------- variable mapping and encoding ------------------
// ----------------------- precomputed states array -----------------------
// ----------------------- lemmas for smurf -------------------------------
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

