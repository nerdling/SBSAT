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
/* dynamic allocation -- great for small problems
 *                       and low physical memory */

#include "ite.h"

#define BDDMEMORY_ALLOC 500000
#define SRANGE_ALLOC 30
#define BRANGE_ALLOC 20

int bddidx_mem_alloc=0;

typedef union {
        void *vptr;
        int   ivalue;
        } uiptr;

typedef struct {
          uiptr value;
          uiptr ptr;
        } nodetype;
/*
typedef struct {
	  void* value;
	  void* ptr;
	} nodetype;
*/

/* BDD Storage */
BDDNode  *bddmemory=NULL;
BDDNode  *bddmemory_last=NULL;
BDDNode  *bddmemory_next=NULL;

/* BDD Index */
nodetype *bddindex = NULL;

nodetype *g_range=NULL;

nodetype *
bddidx_alloc_range(int size)
{
  nodetype *tmp_range = NULL;
  bddidx_mem_alloc += sizeof(nodetype)*(size+1);
  tmp_range = (nodetype*)calloc(size+1, sizeof(nodetype));
  if (!tmp_range) { printf("Out of mem"); exit(1); }

  tmp_range->value.ivalue = 0;
  tmp_range->ptr.ivalue   = size;

  return tmp_range;
}

nodetype *
bddidx_realloc(nodetype *ptr, int add)
{
  nodetype *tmp_range = (nodetype *)realloc(ptr, 
				    sizeof(nodetype)*(ptr->ptr.ivalue+1)+
			            sizeof(nodetype)*add);
  bddidx_mem_alloc += sizeof(nodetype)*add;
  //printf("realloc ");
  if (!tmp_range) { printf("out of mem - realloc"); exit(1); }
  tmp_range->ptr.ivalue += add;
  return tmp_range;
}

void
bddidx_init()
{
  bddidx_mem_alloc += sizeof(BDDNode)*BDDMEMORY_ALLOC;
  bddmemory = (BDDNode*)calloc(BDDMEMORY_ALLOC, sizeof(BDDNode));
  bddmemory_next = bddmemory + 2;
  bddmemory_last = bddmemory_next + (BDDMEMORY_ALLOC-1);

  bddindex = bddidx_alloc_range(BRANGE_ALLOC);
  bddindex->value.ivalue = 1; /* brange must start with 1 srange */

  bddindex[1].value.ivalue = 0;
  bddindex[1].ptr.vptr     = (void*)bddidx_alloc_range(SRANGE_ALLOC);
  g_range = (nodetype *)bddindex[1].ptr.vptr;

  if (false_ptr == NULL && true_ptr == NULL)
    {
      BDDNode * next = bddmemory;
      false_ptr = next;
      false_ptr->variable = 0;
      false_ptr->thenCase = false_ptr;
      false_ptr->elseCase = false_ptr;
      false_ptr->inferences = NULL;
      next++;
      true_ptr = next;
      true_ptr->variable = 0;
      true_ptr->thenCase = true_ptr;
      true_ptr->elseCase = true_ptr;
      true_ptr->inferences = NULL;
    }
}


nodetype *
bddidx_bfind_range(void *v, nodetype *ptr)
{
  int min, max, cur;
  min = 1;
  max = ptr->value.ivalue+1;
  cur = (min+max)/2;
  while (min != (max-1)) {
    if (ptr[cur].value.vptr > v) max=cur; else min=cur;
    cur = (min+max)/2;
  };
  return (nodetype *)(ptr[cur].ptr.vptr);
}

nodetype **
bddidx_sfind_exact(void *v, nodetype *ptr)
{
  int j;
  for (j=1;j<=ptr->value.ivalue;j++) 
       if (ptr[j].value.vptr==v) return (nodetype**)&(ptr[j].ptr.vptr);

  //printf("no srange found ");
  return NULL;
}

BDDNode *
bdd_add(int v, BDDNode *a, BDDNode *b)
{
  //printf("adding node ");
  if (bddmemory_next == bddmemory_last) {
    bddidx_mem_alloc += sizeof(BDDNode)*BDDMEMORY_ALLOC;
    bddmemory_next = 
    bddmemory = (BDDNode*)calloc(BDDMEMORY_ALLOC, sizeof(BDDNode));
    bddmemory_last = bddmemory_next + (BDDMEMORY_ALLOC-1);
  }
  bddmemory_next->variable = v;
  bddmemory_next->thenCase = a;
  bddmemory_next->elseCase = b;
  GetInferFoAN(bddmemory_next); //Get Inferences
  //bddmemory_next->inferences = GetInferFoAN(bddmemory_next); //Get Inferences
  bddmemory_next++;
  return bddmemory_next-1;
};

void
bddidx_add_s(nodetype *sptr, void *v, void *ptr)
{
  sptr[sptr->value.ivalue + 1].value.vptr=v;
  sptr[sptr->value.ivalue + 1].ptr.vptr=ptr;
  sptr->value.ivalue++;
}

int 
bddidx_nodetype_sort(const void*a, const void *b)
{
  return ((const nodetype*)a)->value.vptr>((const nodetype*)b)->value.vptr?1:-1;
}

void
bddidx_add_b(nodetype **bptr, void *v, void *ptr)
{
  if ((*bptr)->value.ivalue >= (*bptr)->ptr.ivalue)
	*bptr = bddidx_realloc(*bptr, BRANGE_ALLOC);
  (*bptr)[(*bptr)->value.ivalue + 1].value.vptr=v;
  (*bptr)[(*bptr)->value.ivalue + 1].ptr.vptr=ptr;
  (*bptr)->value.ivalue++;
  qsort(((*bptr)+1), (*bptr)->value.ivalue, sizeof(nodetype), bddidx_nodetype_sort);
}

void *
bddidx_find_middle(nodetype *srange)
{
  /* this is not correct but the chances are that
     I might be close enought and fast */
  return srange[srange->value.ivalue/2+1].value.vptr>srange[1].value.vptr?srange[srange->value.ivalue/2+1].value.vptr:srange[1].value.vptr;
}


nodetype *
bddidx_split(nodetype **brange, nodetype *srange, void *v, void *middle)
{ 
  int i,j=1;
  nodetype *new_srange=bddidx_alloc_range(SRANGE_ALLOC);
  new_srange->value.ivalue=0;

  //printf("splitting ");

  for (i=1;i<=srange->value.ivalue;i++)
	if (srange[i].value.vptr >= middle) {
		bddidx_add_s(new_srange, srange[i].value.vptr, srange[i].ptr.vptr);
		srange[i].value.vptr=NULL;
	} else {
	  if (j!=i) srange[j]=srange[i]; 
	  j++;
	}
  srange->value.ivalue-=new_srange->value.ivalue;

  bddidx_add_b(brange, middle, new_srange);

  //printf("done ");

  if (v<middle) return srange; else return new_srange;
}

BDDNode *
bddidx_find_or_add_node(int v, BDDNode *a, BDDNode *b)
{
 nodetype *sptr;
 nodetype **bptr2;
 nodetype **bptr3;
 nodetype **pbddptr;
 nodetype *t_bptr2;
 nodetype *t_bptr3;
 BDDNode  *bddptr;
 //printf("find or add node ");

/*
v: bddindex (bptr1) -> range -> sptr(v) -> bptr2
a: bddindex (bptr2) -> range -> sptr(a) -> bptr3
b: bddindex (bptr3) -> range -> sptr(b) -> bdd
*/

 /* level 1 -- variable */
 sptr  = bddidx_bfind_range((void*)v, bddindex);
 bptr2 = bddidx_sfind_exact((void*)v, sptr);
 if (!bptr2) {
	bddptr = bdd_add(v, a, b);
	if (sptr->value.ivalue >= sptr->ptr.ivalue)
		sptr = bddidx_split(&bddindex, sptr, (void*)v, bddidx_find_middle(sptr));

        t_bptr2 = bddidx_alloc_range(BRANGE_ALLOC);
	bddidx_add_s(sptr, (void*)v, t_bptr2);
        sptr = bddidx_alloc_range(SRANGE_ALLOC);
	bddidx_add_b(&t_bptr2, (void*)0, sptr); /* note: #111 arg1 is not pointing to 
				          the real location */
					/* does not matter <= array is empty */

        t_bptr3 = bddidx_alloc_range(BRANGE_ALLOC);
	bddidx_add_s(sptr, (void*)a, t_bptr3);
        sptr = bddidx_alloc_range(SRANGE_ALLOC);
	bddidx_add_b(&t_bptr3, (void*)0, sptr); /* see note: #111 */

	bddidx_add_s(sptr, (void*)b, bddptr);

	//printf("added node done %08x\n", bddptr);
	return bddptr;
	}

 /* level 2 -- bdd1 */
 sptr  = bddidx_bfind_range((void*)a, *bptr2);
 bptr3 = bddidx_sfind_exact((void*)a, sptr);
 if (!bptr3) {
	bddptr = bdd_add(v, a, b);
	if (sptr->value.ivalue >= sptr->ptr.ivalue)
		sptr = bddidx_split(bptr2, sptr, (void*)a, bddidx_find_middle(sptr));
		/* problem: #112 */
		/* problem -- arg1 is not pointing to the real location */
		/* if realloc -- the original location will not get changed */
		/* and core will be dumped! :) */

        t_bptr3 = bddidx_alloc_range(BRANGE_ALLOC);
	bddidx_add_s(sptr, (void*)a, t_bptr3);
        sptr = bddidx_alloc_range(SRANGE_ALLOC);
	bddidx_add_b(&t_bptr3, (void*)0, sptr); /* see note: #111 */

	bddidx_add_s(sptr, (void*)b, bddptr);

	//printf("added node done %08x\n", bddptr);
	return bddptr;
	}

 /* level 3 -- bdd2 */
 sptr    = bddidx_bfind_range((void*)b, *bptr3);
 pbddptr = bddidx_sfind_exact((void*)b, sptr);
 if (!pbddptr) {
	bddptr = bdd_add(v, a, b);
	if (sptr->value.ivalue >= sptr->ptr.ivalue)
		sptr = bddidx_split(bptr3, sptr, (void*)b, bddidx_find_middle(sptr));
		/* see problem: #112 */

	bddidx_add_s(sptr, (void*)b, bddptr);

	//printf("added node done %08x\n", bddptr);
	return bddptr;
	}

 bddptr = (BDDNode*)(*pbddptr);
 //printf("node found done %08x\n", bddptr);

 return bddptr;
}


#ifdef MK_NULL
BDDNode *true_ptr=(BDDNode*)1;
BDDNode *false_ptr=0;
int
main()
{
  int i;

  bddidx_init();
     find_or_add_node(99, true_ptr, false_ptr);
     find_or_add_node(99, true_ptr, false_ptr);

  for (i=1;i<40;i++)
     find_or_add_node(i, true_ptr, false_ptr);

  printf("---------- again\n");
  for (i=1;i<40;i++)
     find_or_add_node(i, true_ptr, false_ptr);

  return 1;
}
#endif
