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

//#define ORIG_LEMMA_CACHE
#define MAX_NUM_CACHED_LEMMAS_0 (MAX_NUM_CACHED_LEMMAS/5)
#define MAX_NUM_CACHED_LEMMAS_2 (MAX_NUM_CACHED_LEMMAS*4/5)

ITE_INLINE void L0PQEnqueue(LemmaInfoStruct *p);
ITE_INLINE void L1PQEnqueue(LemmaInfoStruct *p);
ITE_INLINE void L2PQEnqueue(LemmaInfoStruct *p);
ITE_INLINE void LPQEnqueue(int c, LemmaInfoStruct *pLemmaInfo);
ITE_INLINE void LPQDequeue(LemmaInfoStruct *pLemmaInfo);
int L0Filter(LemmaInfoStruct *p);
int L1Filter(LemmaInfoStruct *p);
int L2Filter(LemmaInfoStruct *p);

typedef int (*proc_lpqdequeue)(LemmaInfoStruct *);

// The next two variables are entry points into the Lemma Priority Queue.
// This queue is a doubly linked list of the LemmaInfoStructs of
// all of the lemmas that have been entered into the "Lemma Cache".
// Thus, only the brancher enters lemmas into this queue (never
// the Smurf Factory).  Whenever a lemma causes an inference, it
// is moved to the front of the queue.
// Whenever we run out of lemma space, we delete lemmas from the back
// of the queue and recover their LemmaBlocks for the lemma space.
LemmaInfoStruct *pLPQFirst[3] = {NULL, NULL, NULL};
LemmaInfoStruct *pLPQLast[3] = {NULL, NULL, NULL};
proc_lpqdequeue proc_LPQDequeue_filter[3] = {L0Filter, L1Filter, L2Filter};
int nNumCachedLemmas[3] = {0, 0, 0};


// ------------------------ dequeue filters -----------------------------------------
int L0Filter(LemmaInfoStruct *p)
{
   if (0
          || p->pLemma->arrLits[0] < 7
          || p->nLemmaLastUsed 
      ) {
      p->nLemmaLastUsed = 0;
      L1PQEnqueue(p);
      return 1;
   }
   FreeLemma(p);
   return 0; /* did not add it anywhere */
}

int L1Filter(LemmaInfoStruct *p)
{
#ifdef ORIG_LEMMA_CACHE
   FreeLemma(p);
   return 0; /* did not go through */
#endif
   if (0
          || p->pLemma->arrLits[0] < 7 
          || p->nLemmaLastUsed
      ) {
      p->nLemmaLastUsed = 0;
      L2PQEnqueue(p);
      return 1; /* it will always go to the next one */
   }
   FreeLemma(p);
   return 0; /* did not go through */
}

int L2Filter(LemmaInfoStruct *p)
{
   FreeLemma(p);
   return 0; /* did not add it anywhere */
}



// ------------------------ Add/Remove Lemma Into/From Cache -------------------------
// needs FreeLemma && LPQ[En][De]queue

ITE_INLINE
void
AddLemmaIntoCache(LemmaInfoStruct *p)
{
   ite_counters[NUM_LEMMA_INTO_CACHE]++;

   assert (p->bPutInCache);

   p->nNumLemmaMoved=0;
   p->nNumLemmaConflict=0;
   p->nNumLemmaInfs=0;
   p->nLemmaFirstUseful=0;
   p->nLemmaLastUsed = 0;
   p->nLemmaNumber=ite_counters[NUM_LEMMA_INTO_CACHE];

   AddLemmaIntoWatchedLits(p);
#ifdef ORIG_LEMMA_CACHE
   L1PQEnqueue(p);
#else
   L0PQEnqueue(p);
#endif
}

ITE_INLINE void
RemoveLemmasFromCache(int c, int num_lemmas_to_remove)
   // Free enough lemmas in order to free at least n LemmaBlocks.
   // The freed lemmas are taken from the tail of the Lemma Priority Queue,
   // which contains all of the lemmas which have been created
   // by the brancher and moved into the lemma cache.
{
   assert(pLPQLast[c] == pLPQFirst[c] || (pLPQLast[c] != NULL && pLPQFirst[c] != NULL));
   LemmaInfoStruct *p = pLPQLast[c];

   while (num_lemmas_to_remove)
   {
      if (p == NULL) return; /* not enough free-able lemmas */

      assert(p->nBacktrackStackReferences >= 0);
      if (p->nBacktrackStackReferences <= 0)
      {
         LPQDequeue(p);
         proc_LPQDequeue_filter[c](p);
         num_lemmas_to_remove--;
      }
      p = p->pLPQPrev;
   }
}

// --------------------------------- LPQ ------------------------------------
// needs RemoveLemmasFromCache

ITE_INLINE void
L0PQEnqueue(LemmaInfoStruct *p)
{
   if (nNumCachedLemmas[0] > MAX_NUM_CACHED_LEMMAS_0) 
      RemoveLemmasFromCache(0, nNumCachedLemmas[0]-MAX_NUM_CACHED_LEMMAS_0+1);

   LPQEnqueue(0, p);
}

ITE_INLINE void
L1PQEnqueue(LemmaInfoStruct *p)
{
   if (nNumCachedLemmas[1] > MAX_NUM_CACHED_LEMMAS) 
      RemoveLemmasFromCache(1, nNumCachedLemmas[1]-MAX_NUM_CACHED_LEMMAS+1);

   LPQEnqueue(1, p);
}

ITE_INLINE void
L2PQEnqueue(LemmaInfoStruct *p)
{
   if (nNumCachedLemmas[2] > MAX_NUM_CACHED_LEMMAS_2) 
      RemoveLemmasFromCache(2, nNumCachedLemmas[2]-MAX_NUM_CACHED_LEMMAS_2+1);

   LPQEnqueue(2, p);
}


ITE_INLINE void
LPQEnqueue(int c, LemmaInfoStruct *pLemmaInfo)
   // Place *pLemmaInfo at the front of the Lemma Priority Queue.
{
   nNumCachedLemmas[c]++;

   assert(!pLPQFirst[c] || !(pLPQFirst[c]->pLPQPrev));

   if (pLPQFirst[c])
   {
      assert(pLPQLast[c]);
      pLPQFirst[c]->pLPQPrev = pLemmaInfo;
   }
   else
   {
      assert(!pLPQLast[c]);
      pLPQLast[c] = pLemmaInfo;
   }
   pLemmaInfo->pLPQPrev = NULL;
   pLemmaInfo->pLPQNext = pLPQFirst[c];
   pLemmaInfo->cache = c;
   pLPQFirst[c] = pLemmaInfo;
}

ITE_INLINE void
LPQDequeue(LemmaInfoStruct *p)
{
   int c = p->cache;

   nNumCachedLemmas[c]--; 

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

   if (p == pLPQLast[c])
   {
      pLPQLast[c] = p->pLPQPrev;
   }

   if (p == pLPQFirst[c])
   {
      pLPQFirst[c] = p->pLPQNext;
   }
}

ITE_INLINE void
MoveToFrontOfLPQ(LemmaInfoStruct *pLemmaInfo)
   // Precondition:  *pLemmaInfo has already been placed in the Lemma
   // Priority Queue (LPQ).
   // Postcondition:  *pLemmaInfo has moved to the front of the LPQ.
{
   pLemmaInfo->nNumLemmaMoved++;
   pLemmaInfo->nLemmaLastUsed = ite_counters[NUM_LEMMA_INTO_CACHE];
   if (pLemmaInfo->nLemmaFirstUseful == 0) 
      pLemmaInfo->nLemmaFirstUseful = ite_counters[NUM_LEMMA_INTO_CACHE];

   LPQDequeue(pLemmaInfo);
   L1PQEnqueue(pLemmaInfo);
}

// -------------------------------- lemma cache display ---------------------------------
ITE_INLINE
void
DisplayAllBrancherLemmas()
{
   cout << "Displaying ALL lemmas " << endl;

   for (LemmaInfoStruct *pLemmaInfo = pLPQFirst[0];
         pLemmaInfo;
         pLemmaInfo = pLemmaInfo->pLPQNext)
   {
      DisplayLemma(pLemmaInfo->pLemma);
      assert((pLemmaInfo == pLPQLast[0]) == (pLemmaInfo->pLPQNext == NULL));
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
   for (int i=0;i<3;i++)
   {
      for (LemmaInfoStruct *pLemmaInfo = pLPQFirst[i];
            pLemmaInfo;
            pLemmaInfo = pLemmaInfo->pLPQNext)
      {
			//if(pLemmaInfo->nBacktrackStackReferences<=0)
			  {
				  if (flag == 1) 
					 fprintf(pFile, "c %d %d %d %d %d %d %d %d %d\n",
								pLemmaInfo->pLemma->arrLits[0],
								pLemmaInfo->nNumLemmaMoved,
								pLemmaInfo->nNumLemmaConflict,
								(int)(ite_counters[NUM_LEMMA_INTO_CACHE]-pLemmaInfo->nLemmaNumber),
								pLemmaInfo->nNumLemmaInfs,
								pLemmaInfo->nBacktrackStackReferences,
								pLemmaInfo->nLemmaFirstUseful?
								(int)(pLemmaInfo->nLemmaFirstUseful-pLemmaInfo->nLemmaNumber):0,
								pLemmaInfo->nLemmaLastUsed?
								(int)(pLemmaInfo->nLemmaLastUsed-pLemmaInfo->nLemmaNumber):0,
								LemmaIsSAT(pLemmaInfo->pLemma));
				  DisplayLemmaToFile(pFile, pLemmaInfo->pLemma);
				  fprintf(pFile, "\n");
				  assert((pLemmaInfo == pLPQLast[0]) == (pLemmaInfo->pLPQNext == NULL));
			}
		}
   }
}


