
#ifndef BDDNODE_H
#define BDDNODE_H

#include "ite.h"
#include "symtable.h"

BDDNode * find_or_add_node (int v, BDDNode * r, BDDNode * e);
BDDNode * find_or_add_node_s (int v, void* s, BDDNode * r, BDDNode * e);
BDDNode * ite_s (BDDNode *, BDDNode *, BDDNode *);
void printBDD(BDDNode *);
extern BDDNode * false_ptr, *true_ptr;
extern BDDNode * op_or  (BDDNode *a, BDDNode *b);
extern BDDNode * op_nor (BDDNode *a, BDDNode *b);
extern BDDNode * op_xor (BDDNode *a, BDDNode *b);
extern BDDNode * op_and (BDDNode *a, BDDNode *b);
extern BDDNode * op_nand(BDDNode *a, BDDNode *b);
extern BDDNode * op_imp (BDDNode *a, BDDNode *b);
extern BDDNode * op_nimp(BDDNode *a, BDDNode *b);
extern BDDNode * op_equ (BDDNode *a, BDDNode *b);
extern BDDNode * op_limp(BDDNode *a, BDDNode *b);
extern BDDNode * op_rimp(BDDNode *a, BDDNode *b);


typedef BDDNode *(*proc_op2fn)(BDDNode *, BDDNode *);
typedef struct _t_op2fn {
        proc_op2fn fn;
        int        fn_type;
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

#define ite_nor_s(a, b)  (ite_not_s(ite_or_s(a, b)))
#define ite_nimp_s(a, b) (ite_not_s(ite_imp_s(a, b)))
#define ite_nand_s(a, b) (ite_not_s(ite_and_s(a, b)))


void bdd_init();
int parser_init();
#endif
