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
#include "integerset.h"

#define INTNODE_POOL_SIZE 10000

typedef struct _t_intnode_pool {
  IntNode *memory;
  int   max;
  struct _intnode_pool *next;
} t_intnode_pool;

t_intnode_pool *intnode_pool = NULL;
t_intnode_pool *intnode_pool_head = NULL;
int intnode_pool_index=0;

void
InitializeIntNodePool()
{
  t_intnode_pool *tmp_intnode_pool; 
  tmp_intnode_pool = (t_intnode_pool*)calloc(1, sizeof(t_intnode_pool));
  tmp_intnode_pool->max = INTNODE_POOL_SIZE;
  tmp_intnode_pool->memory = (IntNode *)calloc(tmp_intnode_pool->max, sizeof(IntNode));

  if (intnode_pool_head == NULL) {
     intnode_pool = tmp_intnode_pool;
     intnode_pool_head = intnode_pool;
  } else {
     intnode_pool->next = (struct _intnode_pool*)tmp_intnode_pool;
     intnode_pool = (t_intnode_pool*)(intnode_pool->next);
  }
  intnode_pool_index = 0;
}

void
FreeIntNodePool()
{
 t_intnode_pool *tmp_intnode_pool = NULL;
 while (intnode_pool_head != NULL)
 {
   free(intnode_pool_head->memory);
   tmp_intnode_pool = (t_intnode_pool*)(intnode_pool_head->next);
   free(intnode_pool_head);
   intnode_pool_head = tmp_intnode_pool;
 }
}

IntNode * 
AllocateIntNode(int n, IntNode *next) {
IntNode *intnode;

  //intnode = (IntNode*)calloc(1, sizeof(IntNode));
  if (intnode_pool == NULL || intnode_pool_index == intnode_pool->max) {
	  InitializeIntNodePool();
  }
  intnode = intnode_pool->memory+intnode_pool_index;
  intnode_pool_index++;
  intnode->nData = n;
  intnode->pNext = next;
  return intnode;
}

