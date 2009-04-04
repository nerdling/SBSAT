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

#ifndef COMMON_H
#define COMMON_H

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
#define SOLV_LIMIT  11

/* conversion result codes */
#define CONV_OUTPUT    12


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
  NEW_INT_LEAF, /* 14 */
  IMPAND,    /* 15 *///x-> a & b & c
  IMPOR,     /* 16 *///x-> a v b v c
  PLAINOR,   /* 17 *///a v b v c
  PLAINAND,  /* 18 *///a & b & c
  PLAINXOR,  /* 19 *///a + b + c
  MINMAX,    /* 20 */
  NEG_MINMAX,/* 21 */
  XDD,       /* 22 */

  EQU_BASE=23,  /* equ_base + 0 */
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
  MINMAX_EQU,    /* equ_base + 19 */
  AUTARKY_FUNC,  
  BDDXOR_BROKEN,
  BDD_PART_BDDXOR,
  XOR_PART_BDDXOR,
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

struct llist{
   int num;
   struct llist *next;
};

struct llistStruct{
   struct llist *head;
   struct llist *tail;
};

typedef struct hashrecord {
	bool used;
	char *data;
	struct hashrecord *next;
} Recd;

typedef struct /*func*/ {
	int  no_vars;
	char *truth_table;
	int  *var_list;
	Recd *reduced0;
	Recd *reduced1;
} func_object;

struct infer{
   int nums[2];
   struct infer *next;
};

struct floatlist {
	int length;
	int *num;
	float *count;
};

struct dualintlist {
	int length;
	int *num;
	int *count;
};

struct intlist{
  int length;
  int *num;
};

struct int_p{
  int var;
  int *loc;
};

struct pathStruct {
	int numpaths;
	intlist *paths;
};

struct BDDState { //Used to store the state of BDDs in bddwalk.cc
	int visited; //Records if this BDD has been visited this flip or not
	int IsSAT;   //Records the value of this BDD (0 or 1) based on the current assignment
};

struct varinfo{
   int equalvars;  //for variablelist is 3 = 4
   int replace;    //dag for variablelist is the replaced numbers
   int true_false; //andor for variablelist is True/False
};

struct minmax {
   int *num;
   int length;//length of num
   int min;
   int max;
};

struct store{
   int *num;
   int num_alloc;
   int min;
   int max;
   int length;//length for variablelist is 3 = 4
   long dag;  //dag for variablelist is the replaced numbers
   int andor; //andor for variablelist is True/False
	int isXor;
};

typedef struct {
   union {
      struct { 
         int fn; 
         int xor_part;
      } bdd_part_bddxor;
      struct { 
         int fn; 
         int bdd_part;
      } xor_part_bddxor;
      struct { 
         int bdd_part; 
         int xor_part; 
      } bddxor_broken;
   };
} FNProps;

extern   long numinp; // highest variable id occuring in any BDD
extern   long numout;
extern   int nmbrFunctions;
extern   int original_numout;
extern   struct BDDNodeStruct **functions;
extern   struct BDDNodeStruct **original_functions;
//extern   struct BDDNodeStruct **xorFunctions;
extern   int *functionType;
extern   int *equalityVble; // Variable on the LHS of an ite=, and=, or or= BDD.
extern   int *independantVars;
extern   FNProps *functionProps;
extern   char **labels;
extern   varinfo *variablelist;
extern   llistStruct *amount;
extern   int *num_funcs_var_occurs;
extern   float *var_score;
extern   store *variables;
extern   int *length;
extern   int nCtrlC;

typedef struct _t_solution_info {
  int *arrElts;
  int nNumElts;
  struct _t_solution_info *next;
} t_solution_info;

#endif

