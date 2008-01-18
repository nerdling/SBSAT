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

#include "sbsat.h"

#define SORT_ITE_OPS


#ifdef NO_BDD_MACROS
int top_variable (BDDNode * x, BDDNode * y, BDDNode * z)
{
   if (x->variable > y->variable)
   {
      if (x->variable > z->variable)
         return x->variable;
      return z->variable;
   }
   if (y->variable > z->variable)
      return y->variable;
   return z->variable;
}

BDDNode * reduce_t (int v, BDDNode * x) {
   if (x->variable == v)
	  return x->thenCase;
   return x;
}

BDDNode * reduce_f (int v, BDDNode * x) {
   if (x->variable == v)
	  return x->elseCase;
   return x;
}
#endif

inline BDDNode *_ite_x_y_F(BDDNode *x, BDDNode *y);

inline
BDDNode *ite_x_y_F(BDDNode *x, BDDNode *y)
{
   if (y == true_ptr) return x;
   if (y == false_ptr) return false_ptr;
   return _ite_x_y_F(x, y);
}

inline
BDDNode *_ite_x_y_F(BDDNode *x, BDDNode *y)
{
   if (x == true_ptr) return y;
   if (x == false_ptr) return false_ptr;

   int v;
   BDDNode * r;
   BDDNode * e;

   if (x->variable > y->variable) {
      BDDNode *cached = itetable_find_or_add_node(1, x, y, NULL);
      if (cached) return cached;
      v = x->variable;
      r = _ite_x_y_F(x->thenCase, y);
      e = _ite_x_y_F(x->elseCase, y);
		// this happens but not often enough to bring a speedup
		if (r == x->thenCase && e == x->elseCase) return x;
   } else if (x->variable == y->variable) {
      if (x == y) return x;
      else if (x->notCase == y) return false_ptr;
      else {
         BDDNode *cached = itetable_find_or_add_node(1, x, y, NULL);
         if (cached) return cached;
         v = x->variable;
         if (y->thenCase == true_ptr) r=x->thenCase;
         else if (y->thenCase == false_ptr) r=false_ptr;
         else r = _ite_x_y_F(x->thenCase, y->thenCase);
         if (y->elseCase == true_ptr) e=x->elseCase;
         else if (y->elseCase == false_ptr) e=false_ptr;
         else e = _ite_x_y_F(x->elseCase, y->elseCase);
			//This happens but maybe not often enough to bring a speedup
			if (r == x->thenCase && e == x->elseCase) return x;
			if (r == y->thenCase && e == y->elseCase) return y;
      }
   } else {
		//Swap x and y
      BDDNode *tmp = x;	x = y; y = tmp;
		//This swap makes itetable work correctly because now
		//x and y need to be swapped for the itetable call below.
		BDDNode *cached = itetable_find_or_add_node(1, x, y, NULL);
      if (cached) return cached;
      v = x->variable;
      r = _ite_x_y_F(x->thenCase, y);
      e = _ite_x_y_F(x->elseCase, y);
		//This happens but maybe not often enough to bring a speedup.
		if (r == x->thenCase && e == x->elseCase) return x;
   } 

   //if (r == e) return (r);
	if (r == e) return
	  itetable_add_node(1, x, y, r);

	return itetable_add_node(1, x, y, find_or_add_node(v, r, e));
   return find_or_add_node(v, r, e);
}

inline
BDDNode *_ite_x_y_T(BDDNode *x, BDDNode *y)
{
   if (x == true_ptr) return y;
   if (x == false_ptr) return true_ptr;

   if (y == true_ptr) return true_ptr;
   if (y == false_ptr) return ite_not(x);
  
   BDDNode *cached = itetable_find_or_add_node(2, x, y, NULL);
   if (cached) return cached;

   int v;
   BDDNode * r;
   BDDNode * e;

   if (x->variable > y->variable) {
      v = x->variable;
      r = _ite_x_y_T(x->thenCase, y);
      e = _ite_x_y_T(x->elseCase, y);
   } else if (x->variable == y->variable) {
      if (x == y) return true_ptr;
      else if (x->notCase == y) return y;
      else {
         v = x->variable;
         r = _ite_x_y_T(x->thenCase, y->thenCase);
         e = _ite_x_y_T(x->elseCase, y->elseCase);
      }
   } else {
      v = y->variable;
      r = _ite_x_y_T(x, y->thenCase);
      e = _ite_x_y_T(x, y->elseCase);
   } 

   //if (r == e) return (r);
	if (r == e) return 
	  itetable_add_node(2, x, y, r);

   return itetable_add_node(2, x, y, find_or_add_node(v, r, e));
   return find_or_add_node(v, r, e);
}

inline
BDDNode *_ite_x_T_z(BDDNode *x, BDDNode *z)
{
   if (x == true_ptr) return true_ptr;
   if (x == false_ptr) return z;

   if (z == true_ptr) return true_ptr;
   if (z == false_ptr) return x;
  
   BDDNode *cached = itetable_find_or_add_node(3, x, z, NULL);
   if (cached) return cached;

   int v;
   BDDNode * r;
   BDDNode * e;

   if (x->variable > z->variable) {
      v = x->variable;
      r = _ite_x_T_z(x->thenCase, z);
      e = _ite_x_T_z(x->elseCase, z);
   } else if (x->variable == z->variable) {
      if (x == z) return x;
      else if (x->notCase == z) return true_ptr;
      else {
         v = x->variable;
         r = _ite_x_T_z(x->thenCase, z->thenCase);
         e = _ite_x_T_z(x->elseCase, z->elseCase);
      }
   } else {
      v = z->variable;
      r = _ite_x_T_z(x, z->thenCase);
      e = _ite_x_T_z(x, z->elseCase);
   } 

   //if (r == e) return (r);
	if (r == e) return 
	  itetable_add_node(3, x, z, r);
	
   return itetable_add_node(3, x, z, find_or_add_node(v, r, e));
   return find_or_add_node(v, r, e);
}

inline
BDDNode *_ite_x_F_z(BDDNode *x, BDDNode *z)
{
   if (x == true_ptr) return false_ptr;
   if (x == false_ptr) return z;

   if (z == true_ptr) return ite_not(x);
   if (z == false_ptr) return false_ptr;

	BDDNode *cached = itetable_find_or_add_node(4, x, z, NULL);
   if (cached) return cached;
	
   int v;
   BDDNode * r;
   BDDNode * e;

   if (x->variable > z->variable) {
      v = x->variable;
      r = _ite_x_F_z(x->thenCase, z);
      e = _ite_x_F_z(x->elseCase, z);
   } else if (x->variable == z->variable) {
      if (x == z) return false_ptr;
      else if (x->notCase == z) return ite_not(x);
      else {
         v = x->variable;
         r = _ite_x_F_z(x->thenCase, z->thenCase);
         e = _ite_x_F_z(x->elseCase, z->elseCase);
      }
   } else {
      v = z->variable;
      r = _ite_x_F_z(x, z->thenCase);
      e = _ite_x_F_z(x, z->elseCase);
   } 

   //if (r == e) return (r);
	if (r == e) return
	  itetable_add_node(4, x, z, r);
	
   return itetable_add_node(4, x, z, find_or_add_node(v, r, e));
   return find_or_add_node(v, r, e);
}

inline
BDDNode *ite_x_y_ny(BDDNode *x, BDDNode *y)
{
   if (x == true_ptr) return y;
   if (x == false_ptr) return ite_not(y);

   if (y == true_ptr) return x;
   if (y == false_ptr) return ite_not(x);
  
   BDDNode *cached = itetable_find_or_add_node(5, x, y, NULL);
   if (cached) return cached;
   
   int v;
   BDDNode * r;
   BDDNode * e;

   if (x->variable > y->variable) {
      v = x->variable;
      r = ite_x_y_ny(x->thenCase, y);
      e = ite_x_y_ny(x->elseCase, y);
   } else if (x->variable == y->variable) {
      if (x == y) return true_ptr;
      else if (x->notCase == y) return false_ptr;
      else {
         v = x->variable;
         r = ite_x_y_ny(x->thenCase, y->thenCase);
         e = ite_x_y_ny(x->elseCase, y->elseCase);
      }
   } else {
      v = y->variable;
      r = ite_x_y_ny(x, y->thenCase);
      e = ite_x_y_ny(x, y->elseCase);
   } 

   //if (r == e) return (r);
	if (r == e) return
	  itetable_add_node(5, x, y, r);
	
   return itetable_add_node(5, x, y, find_or_add_node(v, r, e));
   return find_or_add_node(v, r, e);
}

BDDNode *ite_xvar_y_z(BDDNode * x, BDDNode * y, BDDNode * z) {
   if (x == true_ptr) return y;
   if (x == false_ptr) return z;

   if(y == true_ptr) {
      if(z == false_ptr) return x;
      if(z == true_ptr) return true_ptr;
      return ite_x_T_z(x, z);
   } else if(y == false_ptr) {
      if(z == true_ptr) return ite_not(x);
      if(z == false_ptr) return false_ptr;
      return ite_x_F_z(x, z);
	}

   if (z == true_ptr) {
      return ite_x_y_T(x, y);
   } else if (z == false_ptr) {
      return ite_x_y_F(x, y);
   }
	//int v = top_variable(x, y, z);
   int v;
   BDDNode * r;
   BDDNode * e;

	BDDNode *cached = itetable_find_or_add_node(x->variable+30, y, z, NULL);
	if (cached) return cached;
	
	if (x->variable > y->variable) {
      if (x->variable > z->variable) {
			if(y == z) return y;
         v = x->variable;
         if (y->notCase == z) {
            r = ite_x_y_ny(x->thenCase, y);
            e = ite_x_y_ny(x->elseCase, y);
         } else {
            r = ite_xvar_y_z(x->thenCase, y, z); // <----
            e = ite_xvar_y_z(x->elseCase, y, z); // <----
         }
      } else if (x->variable == z->variable) {
         v = x->variable;
			if(x == z) {
				r = ite_x_y_F(x->thenCase, y);
				e = ite_x_y_F(x->elseCase, y);
         } else if (x->notCase == z) {
				r = ite_x_y_T(x->thenCase, y);
				e = ite_x_y_T(x->elseCase, y);
			} else {			
				r = ite_xvar_y_z(x->thenCase, y, z->thenCase); // <----
				e = ite_xvar_y_z(x->elseCase, y, z->elseCase); // <----
			}
      } else {
         v = z->variable;
			if(x == y) {
            r = ite_x_T_z(z->thenCase, x);
				e = ite_x_T_z(z->elseCase, x);
         } else if (x->notCase == y) {
            r = ite_x_F_z(x, z->thenCase);
				e = ite_x_F_z(x, z->elseCase);
			} else {
				r = ite_xvar_y_z(x, y, z->thenCase); // <----
				e = ite_xvar_y_z(x, y, z->elseCase); // <----
			}
      }
   } else if (y->variable < z->variable) {
		v = z->variable;
		if(x == y) {
			r = ite_x_T_z(z->thenCase, x);
			e = ite_x_T_z(z->elseCase, x);
      } else if (x->notCase == y) {
         r = ite_x_F_z(x, z->thenCase);
         e = ite_x_F_z(x, z->elseCase);
		} else {		
			r = ite_xvar_y_z(x, y, z->thenCase); // <----
			e = ite_xvar_y_z(x, y, z->elseCase); // <----
		}
   } else if (x->variable == z->variable) {
      if (y->variable == x->variable) {
			if(y == z) return y;
			v = x->variable;
         if (y->notCase == z) {
            r = ite_x_y_ny(x->thenCase, y->thenCase);
            e = ite_x_y_ny(x->elseCase, y->elseCase);
         } else if(x == y) {
				r = ite_x_T_z(x->thenCase, z->thenCase);
				e = ite_x_T_z(x->elseCase, z->elseCase);
         } else if (x->notCase == y) {
            r = ite_x_F_z(x->thenCase, z->thenCase);
            e = ite_x_F_z(x->elseCase, z->elseCase);
			} else if(x == z) {
				r = ite_x_y_F(x->thenCase, y->thenCase);
				e = ite_x_y_F(x->elseCase, y->elseCase);
         } else if (x->notCase == z) {
            r = ite_x_y_T(x->thenCase, y->thenCase);
            e = ite_x_y_T(x->elseCase, y->elseCase);
			} else {
				r = ite_xvar_y_z(x->thenCase, y->thenCase, z->thenCase); // <----
				e = ite_xvar_y_z(x->elseCase, y->elseCase, z->elseCase); // <----
			}
      } else {
			v = y->variable;
			if(x == z) {
				r = ite_x_y_F(y->thenCase, x);
				e = ite_x_y_F(y->elseCase, x);
         } else if (x->notCase == z) {
            r = ite_x_y_T(x, y->thenCase);
            e = ite_x_y_T(x, y->elseCase);
			} else {
				r = ite_xvar_y_z(x, y->thenCase, z); // <----
				e = ite_xvar_y_z(x, y->elseCase, z); // <----
			}
      }
   } else {
      if (y->variable == x->variable) {
			v = x->variable;
			if(x == y) {
				r = ite_x_T_z(x->thenCase, z);
				e = ite_x_T_z(x->elseCase, z);
         } else if (x->notCase == y) {
            r = ite_x_F_z(x->thenCase, z);
            e = ite_x_F_z(x->elseCase, z);
			} else {			
				r = ite_xvar_y_z(x->thenCase, y->thenCase, z); // <----
				e = ite_xvar_y_z(x->elseCase, y->elseCase, z); // <----
			}
      } else if (y->variable == z->variable) {
			if(y == z) return y;
         v = y->variable;		
         if (y->notCase == z) {
            r = ite_x_y_ny(x, y->thenCase);
            e = ite_x_y_ny(x, y->elseCase);
         } else {
            r = ite_xvar_y_z(x, y->thenCase, z->thenCase); // <----
            e = ite_xvar_y_z(x, y->elseCase, z->elseCase); // <----
         }
      } else {
			v = y->variable;
			if(x == z) {
				r = ite_x_y_F(y->thenCase, x);
				e = ite_x_y_F(y->elseCase, x);
         } else if (x->notCase == z) {
            r = ite_x_y_T(x, y->thenCase);
            e = ite_x_y_T(x, y->elseCase);
			} else {
				r = ite_xvar_y_z(x, y->thenCase, z); // <----
				e = ite_xvar_y_z(x, y->elseCase, z); // <----
			}
      }
   }

   //if (r == e) return (r);
	if (r == e) return
	  itetable_add_node(x->variable+30, y, z, r);

	return itetable_add_node(x->variable+30, y, z, find_or_add_node(v, r, e));
	return find_or_add_node(v, r, e);
}

BDDNode *ite(BDDNode * x, BDDNode * y, BDDNode * z) {
   if (x == true_ptr) return y;
   if (x == false_ptr) return z;

   if(y == true_ptr) {
      if(z == false_ptr) return x;
      if(z == true_ptr) return true_ptr;
      return ite_x_T_z(x, z);
   } else if(y == false_ptr) {
      if(z == true_ptr) return ite_not(x);
      if(z == false_ptr) return false_ptr;
      return ite_x_F_z(x, z);
	}

   if (z == true_ptr) {
      return ite_x_y_T(x, y);
   } else if (z == false_ptr) {
      return ite_x_y_F(x, y);
   }

//	if (IS_TRUE_FALSE(x->thenCase) && IS_TRUE_FALSE(x->elseCase))
//	  return ite_xvar_y_z(x, y, z);
	
	//int v = top_variable(x, y, z);
   int v;
   BDDNode * r;
   BDDNode * e;

	if (x->variable > y->variable) {
      if (x->variable > z->variable) {
			if(y == z) return y;
         v = x->variable;
         if (y->notCase == z) {
            r = ite_x_y_ny(x->thenCase, y);
            e = ite_x_y_ny(x->elseCase, y);
         } else {
            r = ite(x->thenCase, y, z); // <----
            e = ite(x->elseCase, y, z); // <----
         }
      } else if (x->variable == z->variable) {
         v = x->variable;
			if(x == z) {
				r = ite_x_y_F(x->thenCase, y);
				e = ite_x_y_F(x->elseCase, y);
         } else if (x->notCase == z) {
				r = ite_x_y_T(x->thenCase, y);
				e = ite_x_y_T(x->elseCase, y);
			} else {			
				r = ite(x->thenCase, y, z->thenCase); // <----
				e = ite(x->elseCase, y, z->elseCase); // <----
			}
      } else {
         v = z->variable;
			if(x == y) {
            r = ite_x_T_z(z->thenCase, x);
				e = ite_x_T_z(z->elseCase, x);
         } else if (x->notCase == y) {
            r = ite_x_F_z(x, z->thenCase);
				e = ite_x_F_z(x, z->elseCase);
			} else {
				r = ite(x, y, z->thenCase); // <----
				e = ite(x, y, z->elseCase); // <----
			}
      }
   } else if (y->variable < z->variable) {
		v = z->variable;
		if(x == y) {
			r = ite_x_T_z(z->thenCase, x);
			e = ite_x_T_z(z->elseCase, x);
      } else if (x->notCase == y) {
         r = ite_x_F_z(x, z->thenCase);
         e = ite_x_F_z(x, z->elseCase);
		} else {		
			r = ite(x, y, z->thenCase); // <----
			e = ite(x, y, z->elseCase); // <----
		}
   } else if (x->variable == z->variable) {
      if (y->variable == x->variable) {
			if(y == z) return y;
			v = x->variable;
         if (y->notCase == z) {
            r = ite_x_y_ny(x->thenCase, y->thenCase);
            e = ite_x_y_ny(x->elseCase, y->elseCase);
         } else if(x == y) {
				r = ite_x_T_z(x->thenCase, z->thenCase);
				e = ite_x_T_z(x->elseCase, z->elseCase);
         } else if (x->notCase == y) {
            r = ite_x_F_z(x->thenCase, z->thenCase);
            e = ite_x_F_z(x->elseCase, z->elseCase);
			} else if(x == z) {
				r = ite_x_y_F(x->thenCase, y->thenCase);
				e = ite_x_y_F(x->elseCase, y->elseCase);
         } else if (x->notCase == z) {
            r = ite_x_y_T(x->thenCase, y->thenCase);
            e = ite_x_y_T(x->elseCase, y->elseCase);
			} else {
				r = ite(x->thenCase, y->thenCase, z->thenCase); // <----
				e = ite(x->elseCase, y->elseCase, z->elseCase); // <----
			}
      } else {
			v = y->variable;
			if(x == z) {
				r = ite_x_y_F(y->thenCase, x);
				e = ite_x_y_F(y->elseCase, x);
         } else if (x->notCase == z) {
            r = ite_x_y_T(x, y->thenCase);
            e = ite_x_y_T(x, y->elseCase);
			} else {
				r = ite(x, y->thenCase, z); // <----
				e = ite(x, y->elseCase, z); // <----
			}
      }
   } else {
      if (y->variable == x->variable) {
			v = x->variable;
			if(x == y) {
				r = ite_x_T_z(x->thenCase, z);
				e = ite_x_T_z(x->elseCase, z);
         } else if (x->notCase == y) {
            r = ite_x_F_z(x->thenCase, z);
            e = ite_x_F_z(x->elseCase, z);
			} else {			
				r = ite(x->thenCase, y->thenCase, z); // <----
				e = ite(x->elseCase, y->elseCase, z); // <----
			}
      } else if (y->variable == z->variable) {
			if(y == z) return y;
         v = y->variable;		
         if (y->notCase == z) {
            r = ite_x_y_ny(x, y->thenCase);
            e = ite_x_y_ny(x, y->elseCase);
         } else {
            r = ite(x, y->thenCase, z->thenCase); // <----
            e = ite(x, y->elseCase, z->elseCase); // <----
         }
      } else {
			v = y->variable;
			if(x == z) {
				r = ite_x_y_F(y->thenCase, x);
				e = ite_x_y_F(y->elseCase, x);
         } else if (x->notCase == z) {
            r = ite_x_y_T(x, y->thenCase);
            e = ite_x_y_T(x, y->elseCase);
			} else {
				r = ite(x, y->thenCase, z); // <----
				e = ite(x, y->elseCase, z); // <----
			}
      }
   }

   if (r == e) return (r);
   return find_or_add_node(v, r, e);
}

#ifdef NO_BDD_MACROS
/*
BDDNode * ite_not(BDDNode * a)
{
   //return ite (a, false_ptr, true_ptr);
   return ite_x_F_T(a);
}
*/
BDDNode * ite_or_te(BDDNode * a)
{
   //if (a->or_bdd != NULL) return a->or_bdd;
   return (/*a->or_bdd =*/ ite_or(a->thenCase, a->elseCase));
}

BDDNode * ite_or (BDDNode * a, BDDNode * b)
{
#ifdef SORT_ITE_OPS
   if (a->variable < b->variable)
      return ite_x_T_z(b, /*true_ptr,*/ a);
   else
#endif
      return ite_x_T_z(a, /*true_ptr,*/ b);
}

BDDNode * ite_nor(BDDNode * a, BDDNode * b)
{
   return ite_not(ite_or (a, b));
}

BDDNode * ite_imp (BDDNode * a, BDDNode * b)
{
   return ite_x_y_T(a, b);//, true_ptr);
}

BDDNode * ite_nimp(BDDNode * a, BDDNode * b)
{
   return ite_not(ite_imp(a, b));
}

BDDNode * ite_xor(BDDNode * a, BDDNode * b)
{
#ifdef SORT_ITE_OPS
   if (a->variable < b->variable)
      return ite_x_y_ny(b, ite_not (a));//, a);
   else
#endif
   return ite_x_y_ny(a, ite_not (b));//, b);
}

BDDNode * ite_equ(BDDNode * a, BDDNode * b)
{
   return ite_not(ite_xor (a, b));
}

BDDNode * ite_and(BDDNode * a, BDDNode * b)
{
#ifdef SORT_ITE_OPS
   if (a->variable < b->variable)
      return ite_x_y_F(b, a); //, false_ptr);
   else
#endif
   return ite_x_y_F(a, b); //, false_ptr);
}

BDDNode * ite_nand (BDDNode * a, BDDNode * b)
{
#ifdef SORT_ITE_OPS
   if (a->variable < b->variable)
      return ite_not (ite_and (b, a));
   else
#endif
   return ite_not (ite_and (a, b));
}

BDDNode * ite_itequ (BDDNode * a, BDDNode * b, BDDNode * c, BDDNode * d) 
{
   return ite_equ (d, ite (a, b, c));
}


/***********************************************************************/ 
BDDNode * ite_var (int v)
{
   if (v < 0)
      return find_or_add_node (-v, false_ptr, true_ptr);
	else if (v > 0)
	  return find_or_add_node (v, true_ptr, false_ptr);
	else return false_ptr;
}

#endif // NO_BDD_MACROS

