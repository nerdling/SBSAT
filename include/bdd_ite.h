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

#ifndef BDD_ITE_H
#define BDD_ITE_H

#ifdef NO_BDD_MACROS
BDDNode *ite_var (int);
//BDDNode *ite_not(BDDNode *);
#define ite_not(b) (b->notCase)
BDDNode *ite_and(BDDNode *, BDDNode *);
BDDNode *ite_or(BDDNode *, BDDNode *);
BDDNode *ite_or_te(BDDNode *);
BDDNode *ite_equ(BDDNode *, BDDNode *);
BDDNode *ite_xor(BDDNode *, BDDNode *);
BDDNode *ite_imp(BDDNode *, BDDNode *);
BDDNode *ite_itequ(BDDNode *, BDDNode *, BDDNode *, BDDNode *);
BDDNode *ite_nand(BDDNode *, BDDNode *);
BDDNode *ite_nor(BDDNode *, BDDNode *);
BDDNode *ite_nimp(BDDNode *, BDDNode *);
BDDNode *reduce_t(int, BDDNode *);
BDDNode *reduce_f(int, BDDNode *);
int top_variable(BDDNode *, BDDNode *, BDDNode *);
#else
#define ite_var(v)     (v<0?find_or_add_node (-1*v, false_ptr, true_ptr):\
                            find_or_add_node (   v, true_ptr,  false_ptr))
#define ite_not(a)     (ite(a, false_ptr, true_ptr))
#define ite_and(a, b)  (ite(a, b, false_ptr))
#define ite_or(a, b)   (ite(a, true_ptr, b))
#define ite_equ(a, b)  (ite_not(ite_xor(a, b)))
#define ite_xor(a, b)  (ite(a, ite_not(b), b))
#define ite_imp(a, b)  (ite(a, b, true_ptr))
#define ite_itequ(a, b, c, d) (ite_equ(d, ite(a, b, c)))

#define ite_nor(a, b)  (ite_not(ite_or(a, b)))
#define ite_nimp(a, b) (ite_not(ite_imp(a, b)))
#define ite_nand(a, b) (ite_not(ite_and(a, b)))

#define reduce_t(v, x) ((x)->variable == v ? (x)->thenCase : (x))
#define reduce_f(v, x) ((x)->variable == v ? (x)->elseCase : (x))
#define MIN3(x,y,z) (x<y?(x<z?x:z):y<z?y:z)
#define top_variable(x, y, z) MIN3((x)->variable, (y)->variable, (z)->variable)
#endif

//#define ite_x_F_z(x,z) ite_x_y_F(ite_not(x), z)
#define ite_x_F_z(x,z) _ite_x_F_z(x,z)
//#define ite_x_T_z(x,z) ite_not(ite_x_y_F(ite_not(x), ite_not(z)))
#define ite_x_T_z(x,z) _ite_x_T_z(x,z)
//#define ite_x_y_T(x,y) ite_not(ite_x_y_F(x, ite_not(y)))
#define ite_x_y_T(x,y) _ite_x_y_T(x,y)

#endif
