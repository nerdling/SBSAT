/* =========FOR INTERNAL USE ONLY. NO DISTRIBUTION PLEASE ========== */

/*********************************************************************
 Copyright 1999-2004, University of Cincinnati.  All rights reserved.
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

#define INFER_POOL_SIZE 10000

typedef struct _t_infer_pool {
  infer *memory;
  int   max;
  struct _t_infer_pool *next;
} t_infer_pool;

t_infer_pool *infer_pool = NULL;
t_infer_pool *infer_pool_head = NULL;
int infer_pool_index=0;
infer *infer_free = NULL;

void
InitializeInferencePool()
{
   d4_printf1("Init inference pool\n");
   t_infer_pool *tmp_infer_pool; 
   tmp_infer_pool = (t_infer_pool*)calloc(1, sizeof(t_infer_pool));
   tmp_infer_pool->max = INFER_POOL_SIZE;
   tmp_infer_pool->memory = (infer *)calloc(tmp_infer_pool->max, sizeof(infer));

   if (infer_pool_head == NULL) {
      infer_pool = tmp_infer_pool;
      infer_pool_head = infer_pool;
   } else {
      infer_pool->next = (struct _t_infer_pool*)tmp_infer_pool;
      infer_pool = (t_infer_pool*)(infer_pool->next);
   }
   infer_pool_index = 0;
}

void
FreeInferencePool()
{
   t_infer_pool *tmp_infer_pool = NULL;
   while (infer_pool_head != NULL)
   {
      free(infer_pool_head->memory);
      tmp_infer_pool = (t_infer_pool*)(infer_pool_head->next);
      free(infer_pool_head);
      infer_pool_head = tmp_infer_pool;
   }
   infer_pool = NULL;
}

infer * 
AllocateInference(int num0, int num1, infer *next) {

   infer *infs;
   //infer * infs = (infer *)calloc(1, sizeof(infer));
   if (infer_free != NULL) {
      infs = infer_free;
      infer_free = infer_free->next;
  } else {
     if (infer_pool == NULL || infer_pool_index == infer_pool->max) {
        InitializeInferencePool();
     }
     infs = infer_pool->memory+infer_pool_index;
     infer_pool_index++;
  }
  infs->nums[0] = num0;
  infs->nums[1] = num1;
  infs->next = next;
  return infs;
}

/* -- can't do this -- some of the inferences are chained together
void
DeallocateInferences(infer *next)
{
   infer *last = next;
   if (next == NULL) return;
   while (last->next != NULL) last = last->next;
   last->next = infer_free;
   infer_free = next;
}
*/

inline int
infer_has_var(infer *next, int var) {
   return (abs(next->nums[0]) == var || abs(next->nums[1] == var))?1:0;
}

void
DeallocateInferences_var(infer *next, int var)
{
   infer *last = next;
   if (next == NULL || infer_has_var(next, var) == 0) return;
   while (last->next != NULL && infer_has_var(last->next, var) == 1) last = last->next;
   last->next = infer_free;
   infer_free = next;
}

