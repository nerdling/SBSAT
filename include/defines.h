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
/*********************************************************************
 *  defines.h (S. Weaver, J. Franco, J. Ward)
 *  Defines used by various modules
 *********************************************************************/

#ifndef DEFINES_H
#define DEFINES_H

#define NO_RESULT       0 // NO_ERROR

/* preprocessor result codes */
#define TRIV_UNSAT      1
#define TRIV_SAT        2
#define PREP_CHANGED    3
#define PREP_NO_CHANGE  4
#define PREP_ERROR      5
#define PREP_MAX        6

/* solver result codes */
#define SOLV_SAT	   7
#define SOLV_UNSAT	8
#define SOLV_UNKNOWN	9
#define SOLV_ERROR  10	

/* conversion result codes */
#define CONV_OUTPUT    11	


#ifndef ITE_INLINE
#define ITE_INLINE
#endif

enum {
  UNSURE=0,  /* 0 */
  AND,       /* 1 */
  NAND,      /* 2 */
  OR,        /* 3 */
  NOR,       /* 4 */
  XOR,       /* 5 */
  EQU,       /* 6  = XNOR */
  RIMP,      /* 7 */
  RNIMP,     /* 8 */
  LIMP,      /* 9 */
  LNIMP,     /* 10 */
  ITE,       /* 11 */
  NITE,      /* 12 */
  AND_EQUAL, /* 13 */
  NEW_INT_LEAF,  /* 14 */
  IMPAND,    /* 15 *///x-> a & b & c
  IMPOR,     /* 16 *///x-> a v b v c
  PLAINOR,   /* 17 *///a v b v c
  PLAINXOR,  /* 18 *///a + b + c
  MINMAX,    /* 19 */
  EQU_BASE=20,  /* equ_base + 0 */
  AND_EQU,       /* equ_base + 1 */
  NAND_EQU,      /* equ_base + 2 */
  OR_EQU,        /* equ_base + 3 */
  NOR_EQU,       /* equ_base + 4 */
  XOR_EQU,       /* equ_base + 5 */
  EQU_EQU,       /* equ_base + 6  = XNOR */
  RIMP_EQU,      /* equ_base + 7 */
  RNIMP_EQU,     /* equ_base + 8 */
  LIMP_EQU,      /* equ_base + 9 */
  LNIMP_EQU,     /* equ_base + 10 */
  ITE_EQU,       /* equ_base + 11 */
  NITE_EQU,      /* equ_base + 12 */
  AND_EQUAL_EQU, /* equ_base + 13 */
  NEW_INT_LEAF_EQU,  /* equ_base + 14 */
  IMPAND_EQU,    /* equ_base + 15 *///x-> a & b & c
  IMPOR_EQU,     /* equ_base + 16 *///x-> a v b v c
  PLAINOR_EQU,   /* equ_base + 17 *///a v b v c
  PLAINXOR_EQU,  /* equ_base + 18 *///a + b + c
  MINMAX_EQU,    /* equ_base - 19 */
  MAX_FUNC
};

#define IS_EQU(fn) (fn>EQU_BASE)
#define MAKE_EQU(fn) (fn+EQU_BASE)

extern const char * opnames[EQU_BASE];
#define XNOR 6  //XNOR and EQU are the same function

#define BOOL_FALSE 0
#define BOOL_TRUE 1
#define BOOL_UNKNOWN 2
#define BOOL_MAX 3

/* various stacks */
#define LEVEL_START -1
#define POOL_START -2
#define POOL_END -3
#define LEVEL_MARK -4

//#define IS_TRUE_FALSE(f) (f->variable == INT_MAX
#define IS_TRUE_FALSE(f) (f==true_ptr || f==false_ptr)

/* list of available heuristics */
enum {
   JOHNSON_HEURISTIC,
   C_LEMMA_HEURISTIC,
   INTERACTIVE_HEURISTIC,
   STATE_HEURISTIC
};

#endif
