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

#define LLIST_POOL_SIZE 10000

typedef struct _t_llist_pool {
  llist *memory;
  int   max;
  struct _t_llist_pool *next;
} t_llist_pool;

t_llist_pool *llist_pool = NULL;
t_llist_pool *llist_pool_head = NULL;
int llist_pool_index=0;
llist *llist_free = NULL;

void
InitializeLListPool()
{
   //d4_printf1("Init llistence pool\n");
   t_llist_pool *tmp_llist_pool; 
   tmp_llist_pool = (t_llist_pool*)calloc(1, sizeof(t_llist_pool));
   tmp_llist_pool->max = LLIST_POOL_SIZE;
   tmp_llist_pool->memory = (llist *)calloc(tmp_llist_pool->max, sizeof(llist));

   if (llist_pool_head == NULL) {
      llist_pool = tmp_llist_pool;
      llist_pool_head = llist_pool;
   } else {
      llist_pool->next = (struct _t_llist_pool*)tmp_llist_pool;
      llist_pool = (t_llist_pool*)(llist_pool->next);
   }
   llist_pool_index = 0;
}

void
FreeLListPool()
{
   t_llist_pool *tmp_llist_pool = NULL;
   while (llist_pool_head != NULL)
   {
      free(llist_pool_head->memory);
      tmp_llist_pool = (t_llist_pool*)(llist_pool_head->next);
      free(llist_pool_head);
      llist_pool_head = tmp_llist_pool;
   }
   llist_pool = NULL;
}

llist * 
AllocateLList(int x, llist *next) {

   llist *list;
   //llist * infs = (llist *)calloc(1, sizeof(llist));
   if (llist_free != NULL) {
      list = llist_free;
      llist_free = llist_free->next;
  } else {
     if (llist_pool == NULL || llist_pool_index == llist_pool->max) {
        InitializeLListPool();
     }
     list = llist_pool->memory+llist_pool_index;
     llist_pool_index++;
  }
  list->num = x;
  list->next = next;
  return list;
}

void
DeallocateLLists(llist *next)
{
   if (next == NULL) return;
   llist *last = next;
   while (last->next != NULL) last = last->next;
   last->next = llist_free;
   llist_free = next;
}

void
DeallocateOneLList(llist *next)
{
   if (next == NULL) return;
   next->next = llist_free;
   llist_free = next;
}

void
fprint_llist(FILE *fout, llist *next)
{
   while(next != NULL) {
      fprintf(fout, "(%d) ", next->num);
      next = next->next;
   }
   fprintf(fout, "\n");
}

