#include <stdio.h>
#include "bddnode.h"

BDDNode * op_or  (BDDNode *a, BDDNode *b) { return ite_or_s(a,b);   }
BDDNode * op_nor (BDDNode *a, BDDNode *b) { return ite_nor_s(a,b);  }
BDDNode * op_xor (BDDNode *a, BDDNode *b) { return ite_xor_s(a,b);  }
BDDNode * op_and (BDDNode *a, BDDNode *b) { return ite_and_s(a,b);  }
BDDNode * op_nand(BDDNode *a, BDDNode *b) { return ite_nand_s(a,b); }
BDDNode * op_imp (BDDNode *a, BDDNode *b) { return ite_imp_s(a,b);  }
BDDNode * op_nimp(BDDNode *a, BDDNode *b) { return ite_nimp_s(a,b); }
BDDNode * op_equ (BDDNode *a, BDDNode *b) { return ite_equ_s(a,b);  }
BDDNode * op_limp(BDDNode *a, BDDNode *b) { return ite_imp_s(a,b); }
BDDNode * op_rimp(BDDNode *a, BDDNode *b) { return ite_imp_s(a,b); }

BDDNode * find_or_add_node_s (int v, void* s, BDDNode * r, BDDNode * e)
{
  assert(s);
  assert(((symrec*)s)->id == v);
  BDDNode *tmp_bdd = find_or_add_node(v, r, e);
  tmp_bdd->var_ptr = s;
  assert(tmp_bdd->variable == v);
  return tmp_bdd;
}

BDDNode * ite_s (BDDNode * x, BDDNode * y, BDDNode * z)
{
  int v;
  symrec* s;
  BDDNode *r, *e;
  if (x == true_ptr)  return y;
  if (x == false_ptr) return z;
  s = (symrec*)top_variable_s(x, y, z);
  v = s->id;
  r = ite_s (reduce_t(v, x), reduce_t(v, y), reduce_t(v, z));
  e = ite_s (reduce_f(v, x), reduce_f(v, y), reduce_f(v, z));
  return r == e ? r : find_or_add_node_s (v, s, r, e);
}
