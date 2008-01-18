/* =========FOR INTERNAL USE ONLY. NO DISTRIBUTION PLEASE ========== */

/*********************************************************************
 Copyright 1999-2006, University of Cincinnati.  All rights reserved.
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
   //d4_printf1("Init inference pool\n");
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
 * but for tmp_infer it might work
 */
void
DeallocateInferences(infer *next)
{
   if (next == NULL) return;
   infer *last = next;
   while (last->next != NULL) last = last->next;
   last->next = infer_free;
   infer_free = next;
}

void
DeallocateOneInference(infer *next)
{
   if (next == NULL) return;
   next->next = infer_free;
   infer_free = next;
}

void
DeallocateInferences_var(infer *next, int var)
{
   if (next == NULL) return;
   infer *last = next;
   if (abs(next->nums[0]) == var && next->nums[1] == 0) {
      // this node is an inference -- clip the first member only
   } else {
      // inferences created for this node only
      while (last->next != NULL) last = last->next;
   }
   last->next = infer_free;
   infer_free = next;
   return;
}

void
fprint_infer(FILE *fout, infer *next)
{
   while(next != NULL) {
      fprintf(fout, "(%d %d) ", next->nums[0], next->nums[1]);
      next = next->next;
   }
   fprintf(fout, "\n");
}

void 
GetInferFoAN(BDDNode *func) {
	infer *r = func->thenCase->inferences;
	infer *e = func->elseCase->inferences;
	//If the elseCase is false then add -(func->variable) to the front
	//  of the list r and return it.
	if (func->elseCase == false_ptr)
	  {
//		  fprintf(stderr, "e is false, so we pull up inferences from r\n");
//		  printBDDerr(func);
//		  fprintf(stderr, "\n");
		  func->inferences = AllocateInference(func->variable, 0, r);
		  return;
	  }
	//If the thenCase is false then add (func->variable) to the front
	//  of the list e and return it.
	if (func->thenCase == false_ptr)
	  {
//		  fprintf(stderr, "r false, so we pull up inferences from e\n");
//		  printBDDerr(func);
//		  fprintf(stderr, "\n");
		  func->inferences = AllocateInference(-(func->variable), 0, e);
		  return;
	  }
	
	//If either branch(thenCase or elseCase) carries true (is NULL)
	//then return NULL and we lose all our nice inferences 
	//could be if((func->thenCase == true_ptr) || (func->elseCase == true_ptr))
	if ((r == NULL) || (e == NULL))
	  {
//		  fprintf(stderr, "Lost all inferences\n");
		  return;
	  }
	//If none of the above cases then we have two lists(r and e) which we
	//  combine into inferarray and return.
	
	assert(func->inferences == NULL);
   infer **infs = &(func->inferences);
   infer *rhead = r;
	infer *ehead = e;
	//Pass 1...Search for simple complements
	//       fprintf(stderr, "Doing simple complement search\n");
   //SEAN!!! it may not be necessary to have two loops (passes) here.
	
	if(DO_EQUIVALENCES) {
		while ((r != NULL) && (e != NULL)) {
			if ((r->nums[0] == -(e->nums[0])) && (r->nums[1] == 0)
				 && (e->nums[1] == 0)) {
//				 fprintf(stderr, "Found a simple complement - %d = %d\n", func->variable, r->nums[0]);
//				 printBDDerr(func);
//				 fprintf(stderr, "\n");

				*infs = AllocateInference(func->variable, r->nums[0], NULL);
				infs = &((*infs) -> next);
				r = r->next;
				e = e->next;
				continue;
			}
			
			//If first nums are different, increment one of them
			if (abs (r->nums[0]) < abs (e->nums[0])) {
				e = e->next;
				continue;
			}
			if (abs (e->nums[0]) < abs (r->nums[0])) {
				r = r->next;
				continue;
			}
			
			//Else if second nums are different, increment one of them
			if (abs (r->nums[1]) < abs (e->nums[1])) {
				e = e->next;
				continue;
			}
			
			//None of the above.
			r = r->next;
		}
		r = rhead;
		e = ehead;
	}
	
	//Pass 2...Search for equals on single and double variable inferences.
	//  ex1. 3 and 3 ... ex2. 4=7 and 4=7
	//       fprintf(stderr, "doing pass 2, searching for single and double variables\n");
	while ((r != NULL) && (e != NULL)) {
		//If first nums of r and e are different, increment one of them
		if (abs (r->nums[0]) < abs (e->nums[0])) {
			e = e->next;
			continue;
		}
		if (abs (e->nums[0]) < abs (r->nums[0])) {
			r = r->next;
			continue;
		}
		
		//If nums 0 of both r and e are the same. Check for single equivalence
		if ((r->nums[0] == e->nums[0]) && (r->nums[1] == 0)
			 && (e->nums[1] == 0)) {
			//				 fprintf(stderr, "Found a single equivalence - %d\n", r->nums[0]);
			//				 printBDDerr(func);
			//				 fprintf(stderr, "\n");
			
			*infs = AllocateInference(r->nums[0], 0, NULL);
			infs = &((*infs) -> next);
			r = r->next;
			e = e->next;
			continue;
		}
		
		//if second nums of r and e are different, increment one of them
		if (abs (r->nums[1]) < abs (e->nums[1])) {
			e = e->next;
			continue;
		}
		if (abs (e->nums[1]) < abs (r->nums[1])) {
			r = r->next;
			continue;
		}
		
		//First nums and second nums of r and e are the same. 
		//Check for double equivalence
		if(DO_EQUIVALENCES) {
			if ((r->nums[1] == e->nums[1]) && (r->nums[1] != 0)
				 && (e->nums[1] != 0)) {
//				 fprintf(stderr, "Found a double equivalence  %d = %d\n", r->nums[0], r->nums[1]);
//				 printBDDerr(func);
//				 fprintf(stderr, "\n");
				 
				*infs = AllocateInference(r->nums[0], r->nums[1], NULL);
				infs = &((*infs) -> next);
				r = r->next;
				e = e->next;
				continue;
			}
		}
		r = r->next;
	}
	
	r = rhead;
	e = ehead;

	//Pass 3...Search for equals on double variable inferences.
	//  ex1. 3, 4 and 3=4 ... ex2. -4,7 and 4=-7
	//       fprintf(stderr, "doing pass 2, searching for single and double variables\n");
	if(DO_EQUIVALENCES) {
		while ((r != NULL) && (e != NULL)) {
			//fprintf(stderr, "\nr:%d=%d e:%d=%d", r->nums[0], r->nums[1], e->nums[0], e->nums[1]);
			//If first nums of r and e are different, increment one of them
			if (abs (r->nums[0]) < abs (e->nums[0])) {
				e = e->next;
				continue;
			}
			
			if (abs (e->nums[0]) < abs (r->nums[0])) {
				r = r->next;
				continue;
			}
			
			if (abs (e->nums[0]) == abs (r->nums[0])) {
				if ((r->nums[1] != 0) && (e->nums[1] == 0)) {
					infer *e_iter = e->next;
					//r0 r1  : e0  e1
					//r0 -r1 : -e0 e1
					//r0 r1  : -e0 -e1 ***
					//r0 -r1 : e0  -e1 ***
					int pos = 1;
					//r->nums[0] is always positive in an equivalence
					assert(r->nums[0] > 0);
					if(r->nums[1] > 0 && e->nums[0] < 0) pos = -1;
					if(r->nums[1] < 0 && e->nums[0] > 0) pos = -1;
					while(e_iter!=NULL) {
						if(abs(e_iter->nums[0]) < abs(r->nums[1])) break;
						if((pos * e_iter->nums[0]) == abs(r->nums[1]) && e_iter->nums[1] == 0) {
							*infs = AllocateInference(r->nums[0], r->nums[1], NULL);
							infs = &((*infs) -> next);
							//fprintf(stderr, "\nadding %d=%d because of:", r->nums[0], r->nums[1]);
							//printBDDerr(func);
							//fprintf(stderr, "\n");
							break;
						}
						e_iter = e_iter->next;
					}
					e = e->next;
					continue;
				}
				
				if ((e->nums[1] != 0) && (r->nums[1] == 0)) {
					infer *r_iter = r->next;
					//e0 e1  : r0  r1
					//e0 -e1 : -r0 r1
					//e0 e1  : -r0 -r1 ***
					//e0 -e1 : r0  -r1 ***
					int pos = 1;
					assert(e->nums[0] > 0);
					//e->nums[0] is always positive in an equivalence
					if(e->nums[1] > 0 && r->nums[0] < 0) pos = -1;
					if(e->nums[1] < 0 && r->nums[0] > 0) pos = -1;
					while(r_iter!=NULL) {
						if(abs(r_iter->nums[0]) < abs(e->nums[1])) break;
						if((pos * r_iter->nums[0]) == abs(e->nums[1]) && r_iter->nums[1] == 0) {
							*infs = AllocateInference(e->nums[0], e->nums[1], NULL);
							infs = &((*infs) -> next);
							//fprintf(stderr, "\nadding %d=%d because of:", e->nums[0], e->nums[1]);
							//printBDDerr(func);
							//fprintf(stderr, "\n");
							break;
						}
						r_iter = r_iter->next;
					}
					r = r->next;
					continue;
				}
				
				//if second nums of r and e are different, increment one of them
				if (abs (r->nums[1]) < abs (e->nums[1])) {
					e = e->next;
					continue;
				}
				if (abs (e->nums[1]) < abs (r->nums[1])) {
					r = r->next;
					continue;
				}
				
				if (abs (e->nums[1]) == abs (r->nums[1])) {
					e=e->next;
					r=r->next;
					continue;
				}
			}
		}
	}
		
	return;
}

