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

extern LemmaBlock *pLemmaSpaceNextAvail;

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

void
InitLemmaSpacePool(int at_least)
{
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

  d2_printf2 ("Allocated %ld bytes for lemma space.\n",
       (long)(LEMMA_SPACE_SIZE * sizeof(LemmaBlock)));

  for (int i = 0; i < LEMMA_SPACE_SIZE - 1; i++)
    {
      arrLemmaSpace[i].pNext = arrLemmaSpace + i + 1;
      //arrLemmaSpace[i].arrLits = (int *)new int[LITS_PER_LEMMA_BLOCK];
    }

  arrLemmaSpace[LEMMA_SPACE_SIZE - 1].pNext = pLemmaSpaceNextAvail;
  nLemmaSpaceBlocksAvail += tmp_lemmaspace_pool->max;
  pLemmaSpaceNextAvail = arrLemmaSpace;
}

ITE_INLINE
void 
AllocateMoreLemmaSpace(int at_least)
{
  InitLemmaSpacePool(at_least);
}

void
FreeLemmaSpacePool()
{
 t_lemmaspace_pool *tmp_lemmaspace_pool = NULL;
 while (lemmaspace_pool_head != NULL)
 {
   ite_free((void*)lemmaspace_pool_head->memory);
   tmp_lemmaspace_pool = (t_lemmaspace_pool*)(lemmaspace_pool_head->next);
   ite_free((void*)lemmaspace_pool_head);
   lemmaspace_pool_head = tmp_lemmaspace_pool;
 }
}
