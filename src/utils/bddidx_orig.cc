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

BDDNode *orig_bdd_memory=NULL;

void
orig_bdd_init()
{
  ITE_NEW_CATCH(
      orig_bdd_memory  = new BDDNode[1 << (numBuckets+sizeBuckets)];, "memory");
  if (false_ptr == NULL && true_ptr == NULL)
    {
      BDDNode * next = orig_bdd_memory;
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

    /*
       long nActualMemorySize = (1 << (numBuckets+sizeBuckets));
       for (int i = 0; i < nActualMemorySize; i++) {
       memory[i].variable = 0;
       memory[i].addons = 0;
       }
     */
}

BDDNode * 
orig_find_or_add_node (int v, BDDNode * r, BDDNode * e) {
	long startx = ((v + (int) r + (int) e) & ((1 << numBuckets)-1)) << sizeBuckets;
	BDDNode * curr = &orig_bdd_memory[startx];
	BDDNode * last = &orig_bdd_memory[(1 << (numBuckets+sizeBuckets)) - 1];
	BDDNode * start = curr;
	assert (curr >= orig_bdd_memory);
	assert (curr <= last);
	for (; curr <= last; curr++) {
		if (curr->variable == 0) {
			curr->variable = v;
			curr->thenCase = r;
			curr->elseCase = e;
			GetInferFoAN(curr); //Get Inferences
			//curr->inferences = GetInferFoAN(curr); //Get Inferences
		   return curr;
		}
      if ((curr->variable == v) && (curr->thenCase == r)
			 && (curr->elseCase == e))
        return curr;
	}
	for (curr = &orig_bdd_memory[0]; curr < start; curr++) {
      if (curr->variable == 0) {
			curr->variable = v;
			curr->thenCase = r;
			curr->elseCase = e;
			GetInferFoAN(curr); //Get Inferences
			//curr->inferences = GetInferFoAN(curr); //Get Inferences
		   return curr;
		}
      if ((curr->variable == v) && (curr->thenCase == r)
			 && (curr->elseCase == e))
        return curr;
	}
	fprintf (stderr, "\nPlease allocate more memory for BDD hash table\n");
	exit(1);
   return NULL;
}
