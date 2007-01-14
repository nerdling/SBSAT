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

typedef struct _ITENode {
   int variable;
   BDDNode *r;
   BDDNode *e;
   BDDNode *cached_ite;
   struct _ITENode *next;
} ITENode;


typedef struct {
   ITENode *memory;
   int memory_size;
   int max;
} itetable_pool_type;

itetable_pool_type *itememory;
int itememory_index=0;
int itememory_max=0;
int itetable_num_pools=0;
int itetable_current_pos=0;
ITENode *itetable_free = NULL;
ITENode **itetable_hash_memory = NULL;
int itetable_hash_memory_size = 0;
int itetable_hash_memory_mask = 0;

inline void 
itetable_alloc_node(ITENode **node, int v, BDDNode *r, BDDNode *e, BDDNode *cached_ite);

void itetable_alloc_pool(int pool)
{
   if (pool >= itememory_max)
   {
      itememory = (itetable_pool_type*)ite_recalloc(itememory, itememory_max, pool+100, 
            sizeof(itetable_pool_type), 2, "itememory");
      itememory_max = pool+100;
   }
   itememory[pool].max = _bdd_pool_size; 
   itememory[pool].memory = (ITENode*)ite_calloc(itememory[pool].max, sizeof(ITENode), 2, "ite memory pool");
   d9_printf2("Allocated ITE Memory Pools %d\n", pool+1);
}

void
itetable_free_pools()
{
   int i;
   for (i=0;i<=itetable_num_pools;i++) {
      ite_free((void**)&(itememory[i].memory));
   }
   ite_free((void**)&itetable_hash_memory);
   ite_free((void**)&itememory);
   itememory_max = 0;
   itememory_index = 0;
}

void
itetable_init()
{ 
   /* allocate the first pool */
   itetable_alloc_pool(0);
   itetable_num_pools = 0;
   itetable_current_pos = 0;

   itetable_hash_memory_size = (1 << (numBuckets+sizeBuckets));
   itetable_hash_memory_mask = (1 << (numBuckets+sizeBuckets))-1;
   itetable_hash_memory = (ITENode**)ite_calloc(itetable_hash_memory_size, sizeof(ITENode*),
         2, "ite hash_memory");
}

// for optimized code it does not matter whether this is inlined or macro
// in the debug mode macro is better
inline int
itetable_hash_fn(int v, BDDNode *r, BDDNode *e)
{
   return 
//#define hash_fn(v,r,e) 
      ((v + (*(int*)&r) + (*(int*)&e)) & itetable_hash_memory_mask)
   ;
}

BDDNode * 
itetable_add_node(int v, BDDNode * r, BDDNode * e, BDDNode *cached_ite)
{
   if (cached_ite == NULL) return NULL;

   int hash_pos = itetable_hash_fn(v, r, e);
   ITENode **node = itetable_hash_memory+hash_pos;
   ITENode *new_node;
   itetable_alloc_node(&new_node, v, r, e, cached_ite);
   new_node->next = *node;
   *node = new_node;
   return (*node)->cached_ite;
}

BDDNode * 
itetable_find_or_add_node(int v, BDDNode * r, BDDNode * e, BDDNode *cached_ite)
{
   int hash_pos = itetable_hash_fn(v, r, e);

   ITENode **node = itetable_hash_memory+hash_pos;
   ITENode **prev = NULL; 

   while (!(*node == NULL || ((*node)->variable == v && (*node)->r == r && (*node)->e == e)))
   {
      prev = node;
      node = &((*node)->next);
      ite_counters[BDD_NODE_STEPS]++;
   }
	
   if (*node != NULL) {
      /* if this node has one in front of it in the chain
       * swap it so the next time this node needs less steps */
		
		if (prev && prev != node) {
			
         ITENode *tmp = *prev;
         *prev = *node;
         *node = tmp;
			
         ITENode *tmp_next = (*prev)->next;
			
         (*prev)->next = *node;
         (*node)->next = tmp_next;
			
         node = prev;
      }
      if (cached_ite != NULL) {
         assert((*node)->cached_ite == cached_ite);
      }
		
	} else {
      /* could not find the node => allocate new one */
      if (cached_ite == NULL) return NULL;
      itetable_alloc_node(node, v, r, e, cached_ite);
   } 
   //return cached_ite;
   return (*node)->cached_ite;
}

inline void
itetable_alloc_node(ITENode **node, int v, BDDNode *r, BDDNode *e, BDDNode *cached_ite)
{
   if (itetable_free != NULL) {
      (*node) = itetable_free;
      itetable_free = itetable_free->next;
      (*node)->next = NULL;
   } else {
      if (itememory[itetable_num_pools].max == itetable_current_pos) {
         ++itetable_num_pools;
         itetable_alloc_pool(itetable_num_pools);
         itetable_current_pos= 0;
      }
      (*node) = itememory[itetable_num_pools].memory+itetable_current_pos++;
   }
   (*node)->variable = v;
   (*node)->r = r;
   (*node)->e = e;
   (*node)->cached_ite = cached_ite;
}

void
itetable_removeall()
{
   int i,j;

   // clean the hash table
   for (i=0;i<=itetable_hash_memory_mask; i++)
   {
      itetable_hash_memory[i] = NULL;
   }

   itetable_free = NULL;
   for (i=0;i<=itetable_num_pools;i++)
   {
      int max = itememory[i].max;
      if (i == itetable_num_pools) max = itetable_current_pos;
      for (j=0;j<max;j++)
      {
         ITENode *node = (itememory[i].memory+j);
         node->next = itetable_free;
         itetable_free = node;
      }
   }
}
