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
   int max;
} bdd_pool_type;

/* FIXME: number of pools hardcoded to 100 */
bdd_pool_type bddmemory_vsb[100];
int bddmemory_vsb_index=0;
int numBDDPool=0;
int curBDDPos =0;
BDDNode **hash_memory = NULL;
int hash_memory_size = 0;
int hash_memory_mask = 0;

void bdd_bdd_alloc_pool(int pool)
{
   if (pool >= 100)
   {
      dE_printf1("Increase the number of BDD Pools\n");
      exit(1);
   }
   //bddmemory_vsb[pool].bucket_overlap = BUCKET_OVERLAP;
   //bddmemory_vsb[pool].numBuckets = numBuckets;
   //bddmemory_vsb[pool].sizeBuckets = sizeBuckets;
   //bddmemory_vsb[pool].memorySize = numBuckets + sizeBuckets;
   bddmemory_vsb[pool].max = 1000000; //1 << bddmemory_vsb[pool].memorySize;
   bddmemory_vsb[pool].memory = (BDDNode*)calloc(bddmemory_vsb[pool].max, sizeof(BDDNode));
   d2_printf2 ("Allocated %ld bytes for bdd pool\n", (long)((bddmemory_vsb[pool].max)*sizeof(BDDNode)));
   if (!bddmemory_vsb[pool].memory) { fprintf(stderr, "out of memory"); exit(1); }
}

void FreeInferencePool();

void
bdd_bdd_free_pools()
{
   int i;
   for (i=0;i<=numBDDPool;i++) {
      free(bddmemory_vsb[i].memory);
   }
   FreeInferencePool();
   free(hash_memory);
}

void
bddvsb_init()
{ 
   /* allocate the first pool */
   bdd_bdd_alloc_pool(0);
   numBDDPool = 0;
   curBDDPos  = 2;

   hash_memory_size = (1 << (numBuckets+sizeBuckets));
   hash_memory_mask = (1 << (numBuckets+sizeBuckets))-1;
   hash_memory = (BDDNode**)calloc(hash_memory_size, sizeof(BDDNode*));
   if (hash_memory == NULL) { dE_printf2("Can't allocate %ld bytes for hash table\n", (long)(hash_memory_size*sizeof(BDDNode*))); exit(1); }
   d2_printf2 ("Allocated %ld bytes for bdd hash table\n", (long)(hash_memory_size*sizeof(BDDNode*)));

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
#ifdef MK_NULL
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
#endif

   //int hash_pos = ((v + (*(int*) &r)/sizeof(BDDNode) + (*(int*)&e)/sizeof(BDDNode)) & hash_memory_mask;
   //int hash_pos = (v^(*(int*)&r)^(*(int*)&e)) & hash_memory_mask; // slow!
   //int hash_pos = (v + (*(int*)&r)>>2 + (*(int*)&e)>>2) & hash_memory_mask;
   int hash_pos = (v + (*(int*)&r) + (*(int*)&e)) & hash_memory_mask;

   BDDNode **node = hash_memory+hash_pos;
   BDDNode **prev = NULL; 
   //BDDNode **prev = hash_memory+hash_pos;
   ite_counters[BDD_NODE_STEPS]++;

   while (!(*node == NULL || ((*node)->variable == v && (*node)->thenCase == r && (*node)->elseCase == e)))
   {
      prev = node;
      node = &((*node)->next);
      ite_counters[BDD_NODE_STEPS]++;
   }

   if (*node != NULL) {
      if (prev && prev != node) {// != node && 0) {
         //assert(&(*prev)->next == node);

         BDDNode *tmp = *prev;
         *prev = *node;
         *node = tmp;

         BDDNode *tmp_next = (*prev)->next;

         //if ((*node)->next == *prev) {
            (*prev)->next = *node;
         //} else {
         //   (*prev)->next = (*node)->next;
         //}
         (*node)->next = tmp_next;

         node = prev;
      }
   } else {
      ite_counters[BDD_NODE_NEW]++;
      if (bddmemory_vsb[numBDDPool].max == curBDDPos) {
	  ++numBDDPool;
	  bdd_bdd_alloc_pool(numBDDPool);
	  curBDDPos = 0;
      }
      (*node) = bddmemory_vsb[numBDDPool].memory+curBDDPos++;
      (*node)->variable = v;
      (*node)->thenCase = r;
      (*node)->elseCase = e;
      GetInferFoAN(*node); //Get Inferences
   }
   return *node;
}
