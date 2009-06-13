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

typedef struct {
   BDDNode *memory;
   int bucket_overlap;
   int sizeBuckets;
   int numBuckets;
   int memorySize;
   int max;
} bdd_pool_type;

bdd_pool_type *bddmemory;
int bddmemory_max=0;
int numBDDPool=0;
int curBDDPos =0;
BDDNode **bddtable_hash_memory = NULL;
int bddtable_hash_memory_size = 0;
int bddtable_hash_memory_mask = 0;
BDDNode *bddtable_free = NULL;
int bddtable_free_count = 0;
int bddtable_free_count_last = 0;
int bddtable_used_count_last = 0;
int current_bddpool_size = 1000; // progressively increasing up to _bdd_pool_size

void bdd_fix_inferences(BDDNode *node);
inline void 
bddtable_alloc_node(BDDNode **node, int v, BDDNode *r, BDDNode *e);
void itetable_removeall();

void bddtable_alloc_pool(int pool)
{
   if (pool >= bddmemory_max)
   {
      bddmemory = (bdd_pool_type*)ite_recalloc(bddmemory, bddmemory_max, pool+100, 
            sizeof(bdd_pool_type), 2, "bddmemory");
      bddmemory_max = pool+100;
   }
   bddmemory[pool].max = current_bddpool_size;
   if (current_bddpool_size < _bdd_pool_size) {
      current_bddpool_size *= 10;
      if (current_bddpool_size > _bdd_pool_size) 
         current_bddpool_size = _bdd_pool_size; 
   }
   bddmemory[pool].memory = (BDDNode*)ite_calloc(bddmemory[pool].max, sizeof(BDDNode), 2, "bdd memory pool");
   d9_printf2("Allocated BDD Memory Pools %d\n", pool+1);
}

void FreeInferencePool();
void FreeLListPool();

void
bddtable_free_pools()
{
   int i;
   for (i=0;i<=numBDDPool;i++) {
      ite_free((void**)&(bddmemory[i].memory));
   }
   FreeInferencePool();
   FreeLListPool();
   ite_free((void**)&bddtable_hash_memory);
   ite_free((void**)&bddmemory);
   bddmemory_max = 0;
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
#ifdef BDD_MIRROR_NODE
      false_ptr->mirrCase = false_ptr;
      true_ptr->mirrCase = true_ptr;
#endif
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

inline void bddtable_alloc_not_node(BDDNode *not_node);
#ifdef BDD_MIRROR_NODE
inline void bddtable_alloc_mirr_node(BDDNode *mirr_node);
#endif

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
      ite_counters[BDD_NODE_NEW]++;
      /* could not find the node => allocate new one */
      bddtable_alloc_node(node, v, r, e);
      BDDNode *ret_node = *node; // next function invalidates node memory ptr location
      bddtable_alloc_not_node(ret_node); 
#ifdef BDD_MIRROR_NODE
      bddtable_alloc_mirr_node(ret_node);
#endif
      return ret_node;
   }
   return (*node);
}

inline void
bddtable_alloc_node(BDDNode **node, int v, BDDNode *r, BDDNode *e)
{
   if (bddtable_free != NULL) {
      (*node) = bddtable_free;
      bddtable_free = bddtable_free->next;
      (*node)->next = NULL;
      bddtable_free_count--;
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
   ite_counters[BDD_NODE_STEPS]++;
   BDDNode *prev = NULL;

   prev = *node;
   bddtable_alloc_node(node, v, r, e);
   not_node->notCase = *node;
   (*node)->notCase = not_node;
   (*node)->next = prev;
}

#ifdef BDD_MIRROR_NODE
inline void
bddtable_alloc_mirr_node(BDDNode *mirr_node)
{
   // create mirr_node->mirrCase (could be mirr_node or mirr_node->mirrCase)
   // create mirr_node->mirrCase->notCase
   // create mirr_node->mirrCase->mirrCase = mirr_node
   // create mirr_node->notCase->mirrCase
   // create mirr_node->notCase->mirrCase->mirrCase = mirr_node->notCase
   BDDNode *node = NULL;
   int v = mirr_node->variable;
   BDDNode *r = mirr_node->elseCase->mirrCase;
   BDDNode *e = mirr_node->thenCase->mirrCase;

   if (mirr_node->thenCase == r && mirr_node->elseCase == e) {
      node = mirr_node;
   } else if (mirr_node->notCase->thenCase == r && mirr_node->notCase->elseCase == e) {
      node = mirr_node->notCase;
   } else {
      int hash_pos = bddtable_hash_fn(v, r, e);

      BDDNode **_node = bddtable_hash_memory+hash_pos;
      ite_counters[BDD_NODE_STEPS]++;
      BDDNode *prev = NULL;

      prev = *_node;
      bddtable_alloc_node(_node, v, r, e);
      (*_node)->next = prev;
      node = *_node;
   }
   mirr_node->mirrCase = node;
   node->mirrCase = mirr_node;

   if (mirr_node->mirrCase->notCase == NULL) {
      bddtable_alloc_not_node(mirr_node->mirrCase); 
   }

   mirr_node->notCase->mirrCase = mirr_node->mirrCase->notCase;
   mirr_node->notCase->mirrCase = mirr_node->mirrCase->notCase;
   mirr_node->mirrCase->notCase->mirrCase = mirr_node->notCase;
   mirr_node->notCase->mirrCase->notCase = mirr_node->mirrCase;

   assert(mirr_node->mirrCase && mirr_node->notCase->mirrCase);
   assert(mirr_node->mirrCase->notCase && mirr_node->notCase->mirrCase->notCase);
   assert(mirr_node->mirrCase->notCase->mirrCase && mirr_node->notCase->mirrCase->notCase->mirrCase);
}
#endif

void
bdd_flag_nodes(BDDNode *node)
{
   if (node->flag == 1001001001) return;
   node->flag = 1001001001;
   //node->inferences = NULL;
   bdd_flag_nodes(node->thenCase);
   bdd_flag_nodes(node->elseCase);
	bdd_flag_nodes(node->notCase); 
#ifdef BDD_MIRROR_NODE
	bdd_flag_nodes(node->mirrCase); 
	bdd_flag_nodes(node->notCase->mirrCase); 
#endif
	//if(node->or_bdd!=NULL) bdd_flag_nodes(node->or_bdd); 
	//node->or_bdd = NULL;
	//if(node->t_and_not_e_bdd!=NULL) bdd_flag_nodes(node->t_and_not_e_bdd);
	//node->t_and_not_e_bdd = NULL;
	//if(node->not_t_and_e_bdd!=NULL) bdd_flag_nodes(node->not_t_and_e_bdd)
	//node->not_t_and_e_bdd = NULL;
}

int bddtable_node_new_last = 0;

// BDD Garbage collection

void clear_all_bdd_flags() {
	for (int i=0;i<=numBDDPool;i++) {
		int max = bddmemory[i].max;
		if (i == numBDDPool) max = curBDDPos;
		for (int j=0;j<max;j++) {
			BDDNode *node = (bddmemory[i].memory+j);
			node->flag = 0;
		}
	}
}

void clear_all_bdd_pState() {
	for (int i=0;i<=numBDDPool;i++) {
		int max = bddmemory[i].max;
		if (i == numBDDPool) max = curBDDPos;
		for (int j=0;j<max;j++) {
			BDDNode *node = (bddmemory[i].memory+j);
			node->pState = NULL;
		}
	}
}

int
bdd_gc_test()
{
   if (enable_gc == 0) return 0;
   if (ite_counters[BDD_NODE_NEW]-bddtable_node_new_last < _bdd_pool_size) return 0;
   if (bddtable_free_count > bddtable_free_count_last) return 0;
   return 1;
}

void
bdd_gc(int force)
{
   int i,j;

   // don't do it if there are free nodes
   if (enable_gc == 0) return;
   if (force == 0) {
      if (ite_counters[BDD_NODE_NEW]-bddtable_node_new_last < _bdd_pool_size) return;
      if (bddtable_free_count > bddtable_free_count_last) return;
   }
   bddtable_node_new_last = ite_counters[BDD_NODE_NEW];

   d4_printf2("\nBDD_GC START (free %d) ", bddtable_free_count);
   struct timeval tv_start;
   gettimeofday(&tv_start, NULL);
   double rt_start = get_runtime();

   // flag all referenced nodes
   true_ptr->flag = 1001001001;
   false_ptr->flag = 1001001001;
   for (i=0;i<nmbrFunctions;i++)
   {
      bdd_flag_nodes(functions[i]);
   }
   for (i=0;i<original_numout;i++)
   {
      bdd_flag_nodes(original_functions[i]);
   }
   
   // remove unreferenced and rehash referenced
   for (i=0;i<=bddtable_hash_memory_mask; i++)
   {
      bddtable_hash_memory[i] = NULL;
   }
   
   bddtable_free = NULL;
   bddtable_free_count = 0;
   bddtable_used_count_last = 0;
   BDDNode **chain_free = &bddtable_free;
   for (i=0;i<=numBDDPool;i++)
   {
      int max = bddmemory[i].max;
      if (i == numBDDPool) max = curBDDPos;
      int pre_pool_nodes_taken=bddtable_used_count_last;
      BDDNode **pre_pool_chain_free = chain_free;
      for (j=0;j<max;j++)
      {
         BDDNode *node = (bddmemory[i].memory+j);
         if (node->flag != 1001001001)
         {
            // deleted 
            DeallocateInferences_var(node->inferences, node->variable);
				memset(node, 0, sizeof(BDDNode));
            *chain_free = node;
            chain_free = &(node->next);
            //node->next = bddtable_free;
            //bddtable_free = node;
            bddtable_free_count++;
         } else
         {
            // rehash
            int hash_pos = bddtable_hash_fn(node->variable, node->thenCase, node->elseCase);
            BDDNode **hash_node = bddtable_hash_memory+hash_pos;
            node->next = *hash_node;
            *hash_node = node;
            node->flag = 0;
            bddtable_used_count_last++;
         }
      }
      
      if (pre_pool_nodes_taken == bddtable_used_count_last &&
            i != numBDDPool) {
         d4_printf1(" Removing pool ");
         // can drop this pool
         ite_free((void**)&bddmemory[i].memory);
         chain_free = pre_pool_chain_free;
         *chain_free = NULL; 
         bddtable_free_count -= max;
         for(j=i;j<numBDDPool;j++) bddmemory[j] = bddmemory[j+1];
         memset((void*)&(bddmemory[j]), 0, sizeof(bdd_pool_type));
         numBDDPool--; i--;
      }
   }
 
   true_ptr->flag = 0;
   false_ptr->flag = 0;
   bddtable_free_count_last = bddtable_free_count;

   itetable_removeall();
   struct timeval tv_stop;
   gettimeofday(&tv_stop, NULL);
   double rt_stop = get_runtime();
   d4_printf6("BDD_GC END(used %d, pools %d, free %d, time=%ldms, cpu=%.0fms)\n", 
         bddtable_used_count_last, numBDDPool+1, bddtable_free_count,
         (tv_stop.tv_sec-tv_start.tv_sec)*1000+(tv_stop.tv_usec-tv_start.tv_usec)/1000,
         (rt_stop-rt_start)*1000);

}

void
bdd_fix_inferences(BDDNode *node)
{
   if (node == true_ptr || node == false_ptr) return;
   if (node->inferences != NULL) return;
   if (node->flag == 1) return;
   node->flag = 1;
   bdd_fix_inferences(node->thenCase); 
   bdd_fix_inferences(node->elseCase); 
   bdd_fix_inferences(node->notCase); 
#ifdef BDD_MIRROR_NODE
   bdd_fix_inferences(node->mirrCase); 
#endif
   GetInferFoAN(node);
   infer *ptr = node->inferences;
   while(ptr != NULL) {
      assert(ptr->nums[0] != 0);
      ptr = ptr->next;
   }
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
