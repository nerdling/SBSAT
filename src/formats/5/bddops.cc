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
 any trademark, service mark, or the name of University of Cincinnati.


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

#include <stdio.h>
#include "bddnode.h"

BDDNode * op_or  (BDDNode *a, BDDNode *b) { return ite_or_s(a,b);   }
BDDNode * op_xor (BDDNode *a, BDDNode *b) { return ite_xor_s(a,b);  }
BDDNode * op_and (BDDNode *a, BDDNode *b) { return ite_and_s(a,b);  }
BDDNode * op_imp (BDDNode *a, BDDNode *b) { return ite_imp_s(a,b);  }
BDDNode * op_equ (BDDNode *a, BDDNode *b) { return ite_equ_s(a,b);  }

BDDNode * find_or_add_node_s (int v, void* s, BDDNode * r, BDDNode * e)
{
  assert(s);
  assert(((symrec*)s)->id == v);
  BDDNode *tmp_bdd = find_or_add_node(v, r, e);
  tmp_bdd->var_ptr = s;
  tmp_bdd->notCase->var_ptr = s;
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
