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
#include "solver.h"

#define ADDONS_POOL_SIZE 10000

typedef struct _t_addons_pool {
  SmurfFactoryAddons *memory;
  int   max;
  struct _t_addons_pool *next;
} t_addons_pool;

t_addons_pool *addons_pool = NULL;
t_addons_pool *addons_pool_head = NULL;
int addons_pool_index=0;

ITE_INLINE void
InitializeAddonsPool()
{
  t_addons_pool *tmp_addons_pool; 
  tmp_addons_pool = (t_addons_pool*)calloc(1, sizeof(t_addons_pool));
  tmp_addons_pool->max = ADDONS_POOL_SIZE;
  tmp_addons_pool->memory = (SmurfFactoryAddons *)calloc(tmp_addons_pool->max, sizeof(SmurfFactoryAddons));

  if (addons_pool_head == NULL) {
     addons_pool = tmp_addons_pool;
     addons_pool_head = addons_pool;
  } else {
     addons_pool->next = (struct _t_addons_pool*)tmp_addons_pool;
     addons_pool = (t_addons_pool*)(addons_pool->next);
  }
  addons_pool_index = 0;
}

ITE_INLINE void
FreeAddonsPool()
{
 t_addons_pool *tmp_addons_pool = NULL;
 while (addons_pool_head != NULL)
 {
   for (int i=0; i<addons_pool_head->max; i++)
      FreeSmurfFactoryAddons(addons_pool_head->memory+i);
   free(addons_pool_head->memory);
   tmp_addons_pool = (t_addons_pool*)(addons_pool_head->next);
   free(addons_pool_head);
   addons_pool_head = tmp_addons_pool;
 }
}

ITE_INLINE SmurfFactoryAddons * 
AllocateSmurfFactoryAddons() {

  SmurfFactoryAddons *addons;
  if (addons_pool == NULL || addons_pool_index == addons_pool->max) {
	  InitializeAddonsPool();
  }
  addons = addons_pool->memory+addons_pool_index;
  addons_pool_index++;
  return addons;
}

ITE_INLINE void
FreeSmurfFactoryAddons(SmurfFactoryAddons *f)
{

  if (f->pImplied != NULL)
   {
     delete f->pImplied;
     f->pImplied = NULL;
   }
}

