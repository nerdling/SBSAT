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
/* variable size (growing) overlapping buckets */
/* parallel buckets */

/* buckets  |--- 1 ----|--- 2 ---|--- 3 ---|--- .... ---|--- n ---| 
 overlaps |////////////////////|
 |///////////////////| 

 growing
 new array|--- 1a ---|--- 2a---|--- 3a---|--- .... ---|--- na---|

 this square is growing
 |////////////////////|
 |///////////////////|

 adding more and more parallel buckets
 and adding the size of overlap to fill relatively empty buckets

 the search for hash must be preserved i.e.
 1 -> 2
 add parallel
 1 -> 2 -> 1a -> 2a
 grow overlap
 1 -> 2 -> 1a -> 2a -> 1b -> 1b -> 1b -> 1c -> 1d

 the reason is that if I find 0 -- end the search and use it
 by searching recently added bucket first I might find an empty
 space and end the search without taking into account already filled
 buckets.
 In some situations I might prefer increasing overlap without growing
 buckets because the hash function performs badly.
 Create function to analyze the spread.

 */



#include "ite.h"

#define BUCKET_OVERLAP        6  /*  6 initial bucket overlap  2 */
#define NEW_BUCKET_THREASHOLD 70 /* 70 percent                15 */
#define BUCKET_OVERLAP_INC    16 /* 16 buckets                 2 */

typedef struct {
   BDDNode *memory;
   int bucket_overlap;
   int sizeBuckets;
   int numBuckets;
   int memorySize;
} bdd_pool_type;

//#warning "FIX ME!!!! -- number of pools hardcoded to 100"
bdd_pool_type bddmemory_vsb[100];
int bddmemory_vsb_index=0;
int numBDDPool=0;

void bdd_bdd_alloc_pool(int pool)
{
   if (pool >= 100)
   {
      dE_printf1("Increase the number of BDD Pools\n");
      exit(1);
   }
   bddmemory_vsb[pool].bucket_overlap = BUCKET_OVERLAP;
   bddmemory_vsb[pool].numBuckets = numBuckets;
   bddmemory_vsb[pool].sizeBuckets = sizeBuckets;
   bddmemory_vsb[pool].memorySize = numBuckets + sizeBuckets;
   bddmemory_vsb[pool].memory = (BDDNode*)calloc((1 << bddmemory_vsb[pool].memorySize), sizeof(BDDNode));
   d2_printf2 ("Allocated %d bytes for bdd pool\n", (1 << bddmemory_vsb[pool].memorySize)*sizeof(BDDNode));
   if (!bddmemory_vsb[pool].memory) { fprintf(stderr, "out of memory"); exit(1); }
}

void FreeInferencePool();

void
bdd_bdd_free_pools()
{
   int i;
   for (i=0;i<numBDDPool;i++) {
      free(bddmemory_vsb[i].memory);
   }
   FreeInferencePool();
}

void
bddvsb_init()
{ 
   /* allocate the first pool */
   bdd_bdd_alloc_pool(0);
   numBDDPool=1;

   if (false_ptr == NULL && true_ptr == NULL)
   {
      BDDNode * next = bddmemory_vsb[0].memory;
      false_ptr = next;
      false_ptr->variable = -1;
      false_ptr->thenCase = false_ptr;
      false_ptr->elseCase = false_ptr;
      false_ptr->inferences = NULL;
      next++;
      true_ptr = next;
      true_ptr->variable = -1;
      true_ptr->thenCase = true_ptr;
      true_ptr->elseCase = true_ptr;
      true_ptr->inferences = NULL;
   }
}

BDDNode * 
bddvsb_find_or_add_node (int v, BDDNode * r, BDDNode * e)
{
   long overx;
   long startx=0;
   long endx;
   int curBDDPool=0;

   ite_counters[BDD_NODE_FIND]++;

   assert(v >= r->variable && v >= e->variable);
   if (DEBUG_LVL==10) {
      printf("%d ", v);
      if (r==true_ptr) printf(" true "); 
      else
         if (r==false_ptr) printf(" false "); else
            printf(" x ");
      if (e==true_ptr) printf(" true "); 
      else
         if (e==false_ptr) printf(" false "); else
            printf(" x ");
      printf("\n");
   }

   /* test */
   int t_r, t_e;
   if (r == false_ptr) t_r = 0;
   else if (r == true_ptr) t_r = 1;
   else t_r = 2;

   if (e == false_ptr) t_e = 0;
   else if (e == true_ptr) t_e = 1;
   else t_e = 2; 

   symrec *ptr=getsym_i(v);
   if (ptr && t_e != 2 && t_r != 2) {
      assert (t_e != t_r);
      /* return predefined bddnode 
       * t f
       * f t */
      BDDNode *curr;
      if (t_r == 1) curr = &(ptr->true_false);
      else curr = &(ptr->false_true);
      GetInferFoAN(curr); //Get Inferences
      return curr;
   }

   /* test */

   do {

      for (; curBDDPool<numBDDPool; curBDDPool++) {

         startx = ((v + *(int*) &r + *(int*) &e) & 
               ((1 << bddmemory_vsb[curBDDPool].numBuckets) - 1)) 
            << bddmemory_vsb[curBDDPool].sizeBuckets;
         endx = startx + bddmemory_vsb[curBDDPool].bucket_overlap*
            (1 << bddmemory_vsb[curBDDPool].sizeBuckets);
         long memorySize = 1 << bddmemory_vsb[curBDDPool].memorySize;

         if (endx >= memorySize) { 
            overx = endx - memorySize;
            endx = memorySize-1;
         } else overx = 0;

         BDDNode * start = &(bddmemory_vsb[curBDDPool].memory[startx]);
         BDDNode * end   = &(bddmemory_vsb[curBDDPool].memory[endx]);
         BDDNode * curr  = start;

         do {
            for (; curr <= end; curr++)
            {
               if (curr->variable == 0)
               {
                  curr->variable = v;
                  curr->thenCase = r;
                  curr->elseCase = e;
                  GetInferFoAN(curr); //Get Inferences
                  ite_counters[BDD_NODE_NEW]++;
                  return curr;
               }
               if ((curr->variable == v) && 
                     (curr->thenCase == r) && 
                     (curr->elseCase == e))
                  return curr;
            }
            if (overx == 0) break;
            end  = &(bddmemory_vsb[curBDDPool].memory[overx]);
            curr = &(bddmemory_vsb[curBDDPool].memory[0]);
            overx = 0;
         } while (1);
      } /* over all pools */

      //printf("Starting Analyses\n");

      int nottaken=0; 
      int taken=0; 
      int full=0; 
      int full_blocks=0; 
      int max_full_block=0; 
      int full_block_counter=0; 
      /* count the number of full ones -- there are at least bucket_overlap */
      int bucket_size=1<<bddmemory_vsb[curBDDPool-1].sizeBuckets;
      int total_size=1<<(bddmemory_vsb[curBDDPool-1].memorySize);
      for (int i=0;i<(1<<bddmemory_vsb[curBDDPool-1].numBuckets);i++) {
         nottaken=0;
         for (int j=0;j<bucket_size;j++) {
            if (bddmemory_vsb[curBDDPool-1].memory[i*bucket_size+j].variable == 0) 
               nottaken++;
         }
         taken += (bucket_size - nottaken);
         if (nottaken == 0) {
            full++;
            full_block_counter++;
         } else {
            if (full_block_counter) {
               if (max_full_block<full_block_counter) 
                  max_full_block=full_block_counter;
               full_blocks++;
               full_block_counter=0;
            }
         }
      }
      if (full_block_counter) {
         if (max_full_block<full_block_counter) 
            max_full_block=full_block_counter;
         full_blocks++;
         full_block_counter=0;
      }

      d2_printf4("Taken: %d of %d slots Eff: %2.5f%% ", 
            taken, total_size, (float)taken/total_size*100);
      d2_printf4("Blocks full: %d in chunks: %d Max blocks per chunk: %d\n", 
            full, full_blocks, max_full_block);

      /*  current bucket_overlap is not good enough   */
      /*  how to tie bucket_overlap with taken ?  */
      if ((taken*100/total_size > NEW_BUCKET_THREASHOLD) || 
            (max_full_block > 
             bddmemory_vsb[curBDDPool-1].bucket_overlap+BUCKET_OVERLAP_INC) ||
            ((1<<bddmemory_vsb[curBDDPool-1].numBuckets) < 
             bddmemory_vsb[curBDDPool-1].bucket_overlap+BUCKET_OVERLAP_INC))
      {
         bdd_bdd_alloc_pool(curBDDPool);
         numBDDPool++;
      } else {
         /* let's try this pool again */
         curBDDPool--;
         bddmemory_vsb[curBDDPool].bucket_overlap += BUCKET_OVERLAP_INC;
         d2_printf2("Increasing bucket_overlap to %d\n", bddmemory_vsb[curBDDPool].bucket_overlap);
      }
      /* let's try it again */
   } while (1);
}
