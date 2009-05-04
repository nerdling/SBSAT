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

#ifndef BDDTABLE_H
#define BDDTABLE_H

#define find_or_add_node bddtable_find_or_add_node

typedef struct BDDNodeStruct {
   int variable;
   BDDNodeStruct *next;
   struct BDDNodeStruct *thenCase, *elseCase, *notCase;

   /* memoised values */
   int flag;
   infer *inferences, *tmp_infer;
   
	union {
		struct BDDNodeStruct *tmp_bdd;
		int tmp_int;
	};
	  
	//struct BDDNodeStruct *or_bdd;
	//struct BDDNodeStruct *t_and_not_e_bdd, *not_t_and_e_bdd;
#ifdef BDD_MIRROR_NODE
   struct BDDNodeStruct *mirrCase;
#endif

	/* autarky smurf state */
	//void *pState_Au;
	
   /* for tracer5 */
   void *var_ptr;

	/* smurf state */
	void *pState;
	
	// BDDWalksat addons
	int hamming;
	float density;
	float tbr_weight;
	// BDDWalksat end
	
} BDDNode;

extern   BDDNode *false_ptr, *true_ptr;

void bdd_init();
void itetable_init();
void bdd_flag_nodes(BDDNode *node);
void clear_all_bdd_flags();
void clear_all_bdd_pState();
BDDNode *find_or_add_node(int, BDDNode *, BDDNode *);
BDDNode *itetable_find_or_add_node (int v, BDDNode * r, BDDNode * e, BDDNode *cached_ite);
BDDNode *itetable_add_node(int v, BDDNode * r, BDDNode * e, BDDNode *cached_ite);
BDDNode *ite(BDDNode * x, BDDNode * y, BDDNode * z);
BDDNode *ite_xvar_y_z(BDDNode * x, BDDNode * y, BDDNode * z);
void bddtable_load(void *_bddtable, int _bddtable_len, void *_bddtable_start, int *_shift);
void bddtable_get(void **_bddtable, int *_bddtable_len, int *_bddtable_msize);


#endif

