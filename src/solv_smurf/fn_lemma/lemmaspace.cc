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

LemmaBlock *pLemmaSpaceNextAvail = NULL;

LemmaBlock *arrLemmaSpace = 0;
int nLemmaSpaceBlocksAvail = 0;

typedef struct _t_lemmaspace_pool {
  LemmaBlock *memory;
  int   max;
  struct _t_lemmaspace_pool *next;
} t_lemmaspace_pool;

t_lemmaspace_pool *lemmaspace_pool = NULL;
t_lemmaspace_pool *lemmaspace_pool_head = NULL;
int lemmaspace_pool_index=0;

ITE_INLINE void
InitLemmaSpacePool(int at_least) {
  t_lemmaspace_pool *tmp_lemmaspace_pool;
  tmp_lemmaspace_pool = (t_lemmaspace_pool*)ite_calloc(1, sizeof(t_lemmaspace_pool),
        9, "tmp_lemmaspace_pool");
  tmp_lemmaspace_pool->max = at_least>LEMMA_SPACE_SIZE?at_least:LEMMA_SPACE_SIZE;
  tmp_lemmaspace_pool->memory = (LemmaBlock *)ite_calloc(tmp_lemmaspace_pool->max, sizeof(LemmaBlock),
        9, "tmp_lemmaspace_pool->memory");

  if (lemmaspace_pool_head == NULL) {
     lemmaspace_pool = tmp_lemmaspace_pool;
     lemmaspace_pool_head = lemmaspace_pool;
  } else {
     lemmaspace_pool->next = (struct _t_lemmaspace_pool*)tmp_lemmaspace_pool;
     lemmaspace_pool = (t_lemmaspace_pool*)(lemmaspace_pool->next);
  }
  lemmaspace_pool_index = 0;

  arrLemmaSpace = tmp_lemmaspace_pool->memory;
  if (!arrLemmaSpace) {
      dE_printf1("Unable to allocate lemma space");
      exit(1);
  }

  dm2_printf2 ("Allocated %ld bytes for lemma space.\n",
       (long)(tmp_lemmaspace_pool->max * sizeof(LemmaBlock)));

  for (int i = 0; i < tmp_lemmaspace_pool->max - 1; i++)
    {
      arrLemmaSpace[i].pNext = arrLemmaSpace + i + 1;
    }

  arrLemmaSpace[tmp_lemmaspace_pool->max - 1].pNext = pLemmaSpaceNextAvail;
  nLemmaSpaceBlocksAvail += tmp_lemmaspace_pool->max;
  pLemmaSpaceNextAvail = arrLemmaSpace;
}

ITE_INLINE void 
AllocateMoreLemmaSpace(int at_least)
{
  InitLemmaSpacePool(at_least);
}

ITE_INLINE void
FreeLemmaSpacePool()
{
 t_lemmaspace_pool *tmp_lemmaspace_pool = NULL;
 while (lemmaspace_pool_head != NULL)
 {
   ite_free((void**)&lemmaspace_pool_head->memory);
   tmp_lemmaspace_pool = (t_lemmaspace_pool*)(lemmaspace_pool_head->next);
   ite_free((void**)&lemmaspace_pool_head);
   lemmaspace_pool_head = tmp_lemmaspace_pool;
 }
}

ITE_INLINE void
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
   int nNumEntries = nNumElts + 2; /* add the size and the ending 0 */
   int nNumBlocksRequired = nNumEntries / LITS_PER_LEMMA_BLOCK +
      (nNumEntries % LITS_PER_LEMMA_BLOCK>0 ? 1: 0);

   if (procHeurAddLemmaSpace) procHeurAddLemmaSpace(arrLemmaLiterals, nNumElts);

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

ITE_INLINE void
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

ITE_INLINE void
FreeLemmaBlocks(LemmaInfoStruct *pLemmaInfo)
   // LemmaBlocks are returned to the pool pointed to by
   // pLemmaSpaceNextAvail.
{
	pLemmaInfo->pLemmaLastBlock->pNext = pLemmaSpaceNextAvail;
   pLemmaSpaceNextAvail = pLemmaInfo->pLemma;
   nLemmaSpaceBlocksAvail += pLemmaInfo->nNumBlocks;
	//pLemmaInfo->pLemma = NULL;
	
	if(pLemmaInfo->pSmurfsReferenced) {
		pLemmaInfo->pSmurfsReferencedLastBlock->pNext = pLemmaSpaceNextAvail;
		pLemmaSpaceNextAvail = pLemmaInfo->pSmurfsReferenced;
		nLemmaSpaceBlocksAvail += pLemmaInfo->nNumSRBlocks;
		//pLemmaInfo->pSmurfsReferenced = NULL;
	}

	assert(pLemmaSpaceNextAvail!=NULL);
}
