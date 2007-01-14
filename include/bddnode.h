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


#ifndef BDDNODE_H
#define BDDNODE_H

#include "sbsat.h"
#include "symtable.h"

BDDNode * find_or_add_node (int v, BDDNode * r, BDDNode * e);
BDDNode * find_or_add_node_s (int v, void* s, BDDNode * r, BDDNode * e);
BDDNode * ite_s (BDDNode *, BDDNode *, BDDNode *);
void printBDD(BDDNode *);
extern BDDNode * false_ptr, *true_ptr;
extern BDDNode * op_or  (BDDNode *a, BDDNode *b);
extern BDDNode * op_xor (BDDNode *a, BDDNode *b);
extern BDDNode * op_and (BDDNode *a, BDDNode *b);
extern BDDNode * op_imp (BDDNode *a, BDDNode *b);
extern BDDNode * op_equ (BDDNode *a, BDDNode *b);

#define AS_FULL 0
#define AS_LEFT 1
#define AS_RIGHT 2

typedef BDDNode *(*proc_op2fn)(BDDNode *, BDDNode *);
typedef struct _t_op2fn {
        proc_op2fn fn;
        int        fn_type;
        int        as_type;
        int        neg_all;
} t_op2fn;

#define MAX3ID(x,y,z) (x->id>y->id?(x->id>z->id?x:z):y->id>z->id?y:z)

#define top_variable_s(x, y, z) \
        ((x)->variable>(y)->variable?((x)->variable>(z)->variable?(x)->var_ptr:(z)->var_ptr):(y)->variable>(z)->variable?(y)->var_ptr:(z)->var_ptr)
#define ite_vars(v_ptr) find_or_add_node_s(v_ptr->id, (void*)v_ptr, true_ptr,  false_ptr)

#define ite_not_s(a)     (ite_s(a, false_ptr, true_ptr))
#define ite_and_s(a, b)  (ite_s(a, b, false_ptr))
#define ite_or_s(a, b)   (ite_s(a, true_ptr, b))
#define ite_equ_s(a, b)  (ite_not_s(ite_xor_s(a, b)))
#define ite_xor_s(a, b)  (ite_s(a, ite_not_s(b), b))
#define ite_imp_s(a, b)  (ite_s(a, b, true_ptr))
#define ite_itequ_s(a, b, c, d) (ite_equ_s(d, ite_s(a, b, c)))

   //FIXME:!!!!!!!! not?!!! 
#define ite_nor_s(a, b)  (ite_not_s(ite_or_s(a, b)))
#define ite_nimp_s(a, b) (ite_not_s(ite_imp_s(a, b)))
#define ite_nand_s(a, b) (ite_not_s(ite_and_s(a, b)))


void bdd_init();
int parser_init();
#endif
