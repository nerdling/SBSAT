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

typedef struct {
   BDDNode *memory;
   int bucket_overlap;
   int sizeBuckets;
   int numBuckets;
   int memorySize;
   int max;
} bdd_pool_type;

/* FIXME: number of pools hardcoded to 100 */
bdd_pool_type bddmemory[100];
int bddmemory_index=0;
int numBDDPool=0;
int curBDDPos =0;
BDDNode **bddtable_hash_memory = NULL;
int bddtable_hash_memory_size = 0;
int bddtable_hash_memory_mask = 0;
BDDNode *bddtable_free = NULL;

void bdd_gc();
void bdd_fix_inferences(BDDNode *node);
void bddtable_alloc_node(BDDNode **node, int v, BDDNode *r, BDDNode *e);
void itetable_removeall();

void bddtable_alloc_pool(int pool)
{
   if (pool >= 100)
   {
      dE_printf1("Increase the number of BDD Pools\n");
      exit(1);
   }
   bddmemory[pool].max = _bdd_pool_size; 
   bddmemory[pool].memory = (BDDNode*)ite_calloc(bddmemory[pool].max, sizeof(BDDNode), 2, "bdd memory pool");
   d9_printf2("Allocated BDD Memory Pools %d\n", pool+1);
}

void FreeInferencePool();

void
bddtable_free_pools()
{
   int i;
   for (i=0;i<=numBDDPool;i++) {
      free(bddmemory[i].memory);
   }
   FreeInferencePool();
   free(bddtable_hash_memory);
}

void
bdd_init()
{ 
   /* allocate the first pool */
   bddtable_alloc_pool(0);
   numBDDPool = 0;
   curBDDPos  = 2;

   bddtable_hash_memory_size = (1 << (numBuckets+sizeBuckets));
   bddtable_hash_memory_mask = (1 << (numBuckets+sizeBuckets))-1;
   bddtable_hash_memory = (BDDNode**)ite_calloc(bddtable_hash_memory_size, sizeof(BDDNode*),
         2, "hash_memory");

   if (false_ptr == NULL && true_ptr == NULL)
   {
      BDDNode * next = bddmemory[0].memory;
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

      false_ptr->notCase = true_ptr;
      true_ptr->notCase = false_ptr;
   }
}

// for optimized code it does not matter whether this is inlined or macro
// in the debug mode macro is better
inline int
bddtable_hash_fn(int v, BDDNode *r, BDDNode *e)
{
   return 
//#define hash_fn(v,r,e) 
      ((v + (*(int*)&r) + (*(int*)&e)) & bddtable_hash_memory_mask)
   ;
}

void bddtable_alloc_not_node(BDDNode *not_node);

BDDNode * 
bddtable_find_or_add_node (int v, BDDNode * r, BDDNode * e)
{
   ite_counters[BDD_NODE_FIND]++;

   assert(v != 0);
   assert(v >= r->variable && v >= e->variable);
   /*
   if (DEBUG_LVL&32) {
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
   */

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

   //int hash_pos = (v + (*(int*)&r)/sizeof(BDDNode) + (*(int*)&e)/sizeof(BDDNode)) & hash_memory_mask;
   //int hash_pos = (v^(*(int*)&r)^(*(int*)&e)) & hash_memory_mask; // slow!
   //int hash_pos = (v + (*(int*)&r)>>2 + (*(int*)&e)>>2) & hash_memory_mask;
   //int hash_pos = (v + (*(int*)&r) + (*(int*)&e)) & hash_memory_mask;
   int hash_pos = bddtable_hash_fn(v, r, e);

   BDDNode **node = bddtable_hash_memory+hash_pos;
   BDDNode **prev = NULL; 
   ite_counters[BDD_NODE_STEPS]++;

   while (!(*node == NULL || ((*node)->variable == v && (*node)->thenCase == r && (*node)->elseCase == e)))
   {
      prev = node;
      node = &((*node)->next);
      ite_counters[BDD_NODE_STEPS]++;
   }
	
   if (*node != NULL) {
      /* if this node has one in front of it in the chain
       * swap it so the next time this node needs less steps */
		
		if (prev && prev != node) {
			
         BDDNode *tmp = *prev;
         *prev = *node;
         *node = tmp;
			
         BDDNode *tmp_next = (*prev)->next;
			
         (*prev)->next = *node;
         (*node)->next = tmp_next;
			
         node = prev;
      }
		
	} else {
      /* could not find the node => allocate new one */
      bddtable_alloc_node(node, v, r, e);
      bddtable_alloc_not_node((*node));//, v, r->notCase, e->notCase);
   }
   return (*node);
}

inline void
bddtable_alloc_node(BDDNode **node, int v, BDDNode *r, BDDNode *e)
{
   ite_counters[BDD_NODE_NEW]++;
   if (bddtable_free != NULL) {
      (*node) = bddtable_free;
      bddtable_free = bddtable_free->next;
      (*node)->next = NULL;
   } else {
      if (bddmemory[numBDDPool].max == curBDDPos) {
         ++numBDDPool;
         bddtable_alloc_pool(numBDDPool);
         curBDDPos = 0;
      }
      (*node) = bddmemory[numBDDPool].memory+curBDDPos++;
   }
   (*node)->variable = v;
   (*node)->thenCase = r;
   (*node)->elseCase = e;
   GetInferFoAN(*node); //Get Inferences
}

inline void
bddtable_alloc_not_node(BDDNode *not_node)
{
   int v = not_node->variable;
   BDDNode *r = not_node->thenCase->notCase;
   BDDNode *e = not_node->elseCase->notCase;
   int hash_pos = bddtable_hash_fn(v, r, e);

   BDDNode **node = bddtable_hash_memory+hash_pos;
   BDDNode **prev = NULL; 
   ite_counters[BDD_NODE_STEPS]++;

   while (!(*node == NULL/* || ((*node)->variable == v && (*node)->thenCase == r && (*node)->elseCase == e)*/))
   {
      prev = node;
      node = &((*node)->next);
      ite_counters[BDD_NODE_STEPS]++;
   }
   assert(*node == NULL);
   bddtable_alloc_node(node, v, r, e);
   not_node->notCase = *node;
   (*node)->notCase = not_node;
}

void
bdd_flag_nodes(BDDNode *node)
{
   if (node->flag == 1) return;
   node->flag = 1;
   //node->inferences = NULL;
   bdd_flag_nodes(node->thenCase);
   bdd_flag_nodes(node->elseCase);
	bdd_flag_nodes(node->notCase); //SEAN
	if(node->or_bdd!=NULL) bdd_flag_nodes(node->or_bdd); //mk
}


// BDD Garbage collection
void
bdd_gc()
{
   int i,j;

   // don't do it if there are free nodes
   if (bddtable_free != NULL) return;

   d4_printf1("BDD_GC START\n");
#ifndef NDEBUG
   int totalin=0, totalout=0;
   // count free nodes -- statistics -- can be removed
   BDDNode *p = bddtable_free;
   while (p!=NULL) {
      totalin++;
      p = p->next;
   }
#endif


   // clean all flags
   for (i=0;i<=numBDDPool;i++)
   {
      int max = bddmemory[i].max;
      if (i == numBDDPool) max = curBDDPos;
      for (j=0;j<max;j++) 
         (bddmemory[i].memory+j)->flag = 0;
   }

   // flag all referenced nodes
   true_ptr->flag = 1;
   false_ptr->flag = 1;
   for (i=0;i<nmbrFunctions;i++)
   {
     // printBDDInfs(functions[i]);
     // fprintf(stddbg, "\n");
      bdd_flag_nodes(functions[i]);
   }
   for (i=0;i<original_numout;i++)
   {
      bdd_flag_nodes(original_functions[i]);
   }

   
   // deallocate infereces
   //FreeInferencePool(); 
   

   // clean the hash table
   for (i=0;i<=bddtable_hash_memory_mask; i++)
   {
      bddtable_hash_memory[i] = NULL;
   }

   // remove unreferenced and rehash referenced
   bddtable_free = NULL;
   for (i=0;i<=numBDDPool;i++)
   {
      int max = bddmemory[i].max;
      if (i == numBDDPool) max = curBDDPos;
      for (j=0;j<max;j++)
      {
         BDDNode *node = (bddmemory[i].memory+j);
         if (node->flag == 0)
         {
            // deleted 
            DeallocateInferences_var(node->inferences, node->variable);

            node->or_bdd = NULL; //mk

				memset(node, 0, sizeof(BDDNode));
            node->next = bddtable_free;
            bddtable_free = node;
         } else
         {
            // rehash
            int hash_pos = bddtable_hash_fn(node->variable, node->thenCase, node->elseCase);
            BDDNode **hash_node = bddtable_hash_memory+hash_pos;
            node->next = *hash_node;;
            *hash_node = node;
         }
      }
   }
/*
   for (i=0;i<nmbrFunctions;i++)
   {
      bdd_fix_inferences(functions[i]);
   }
   for (i=0;i<original_numout;i++)
   {
      bdd_fix_inferences(original_functions[i]);
   }
   */
/*
   fprintf(stddbg, "out:\n");
   for (i=0;i<nmbrFunctions;i++)
   {
      printBDDInfs(functions[i]);
      fprintf(stddbg, "\n");
   }
*/
#ifndef NDEBUG
   // count free nodes -- statistics -- can be removed
   p = bddtable_free;
   while (p!=NULL) {
      totalout++;
      p = p->next;
   }
   d4_printf4("BDD_GC %d -> %d (%.02f%%)\n", 
         totalin, totalout, totalin==0?0:1.0*(totalout-totalin)/totalin);
#endif
   
   itetable_removeall();
}

void
bdd_fix_inferences(BDDNode *node)
{
   if (node == true_ptr || node == false_ptr) return;
   if (node->inferences != NULL) return;
   bdd_fix_inferences(node->thenCase); 
   bdd_fix_inferences(node->elseCase); 
   GetInferFoAN(node);
}

FILE *
ite_fopen(char *filename, const char *fileflags)
{
   FILE *f = fopen(filename, fileflags);
   if (!f) {
      cerr << "Can't open " << filename << endl;
   }
   return f;
}

long
ite_filesize(char *filename)
{
   struct stat buf;
   if (stat(filename, &buf) != 0) {
      /* error in errno */
      return 0;
   };
   return buf.st_size;
}


void 
bddtable_load(void *_bddtable, int _bddtable_len, void *_bddtable_start, int *_shift)
{
   int i;
   curBDDPos = _bddtable_len;
   assert(bddmemory[0].max > curBDDPos);
   memmove(bddmemory[0].memory, _bddtable, sizeof(BDDNode)*curBDDPos);
   int shift = (char*)(bddmemory[0].memory) - (char*)(_bddtable_start);
   *_shift = shift;
   bddtable_free = NULL;

   /* fix bdd table */
   d2_printf1("Fixing bdd table .. \n");
   for (i=0;i<curBDDPos;i++) {
      if (bddmemory[0].memory[i].thenCase) {
         bddmemory[0].memory[i].thenCase = (BDDNode*)
            ((char*)bddmemory[0].memory[i].thenCase + shift);
         bddmemory[0].memory[i].elseCase = (BDDNode*)
            ((char*)bddmemory[0].memory[i].elseCase + shift);
			bddmemory[0].memory[i].notCase = (BDDNode*)
			  ((char*)bddmemory[0].memory[i].notCase + shift);
         bddmemory[0].memory[i].next = NULL;
      } else {
         bddmemory[0].memory[i].next = bddtable_free;
         bddtable_free = bddmemory[0].memory[i].next;
      }

      /* fix bdd hash table */
      int v = bddmemory[0].memory[i].variable;
      BDDNode * r = bddmemory[0].memory[i].thenCase;
      BDDNode * e = bddmemory[0].memory[i].elseCase;
      int hash_pos = bddtable_hash_fn(v, r, e);
      BDDNode **node = bddtable_hash_memory+hash_pos;
      bddmemory[0].memory[i].next = *node;
      bddmemory[0].memory[i].inferences = NULL;
      *node = bddmemory[0].memory + i;
   }
   false_ptr = bddmemory[0].memory;
   true_ptr = bddmemory[0].memory+1;

   d2_printf1("Fixing bdd table inferences .. \n");
   GetInferFoAN(bddmemory[0].memory+0);
   GetInferFoAN(bddmemory[0].memory+1);
   for (i=2;i<curBDDPos;i++) {
      if (bddmemory[0].memory[i].inferences == NULL && // no inferences yet
            bddmemory[0].memory[i].thenCase != NULL) // not free
         bdd_fix_inferences(bddmemory[0].memory+i);
   }
}

void 
bddtable_get(void **_bddtable, int *_bddtable_len, int *_bddtable_msize)
{
   if (numBDDPool) {
      cerr << "Can't dump split BDD pool -- please increase bdd-pool-size" << endl;
      exit(1);
      return;
   }
   *_bddtable = bddmemory[0].memory;
   *_bddtable_len = curBDDPos;
   *_bddtable_msize = sizeof(BDDNode);
}
