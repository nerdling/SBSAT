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
 *  common.h (S. Weaver, J. Franco, J. Ward)
 *  Structures used by various modules
 *********************************************************************/

// In input BDD's, variables are represented by integers
// 2..nmbrVars+1.  To save a bit of space, in the Smurf villages they
// will be stored as 0..nmbrVars-1.  Thus, any output procedure is
// going to have to add 2 to each variable number again.

// variable: the variable number for the decision, or
// INT_MAX (for true and false)
// thenCase, elseCase: should point to themselves if variable == INT_MAX
// this allows for total canonicity of the BDD's

#ifndef COMMON_H
#define COMMON_H

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
                                                                                                                                                 
typedef struct xorrecord {
	Recd *clauses;
	char *vars;
	char *save;
	int   save_size;
	struct xorrecord *next;
} XORd;

typedef struct func {
	int  no_vars;
	char *truth_table;
	int  *var_list;
	Recd *reduced0;
	Recd *reduced1;
	XORd *xor0;
	XORd *xor1;
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

struct pathStruct {
	int numpaths;
	intlist *paths;
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
};

typedef struct BDDNodeStruct {
   int flag;
   int variable;
   void *var_ptr;
   struct BDDNodeStruct *thenCase, *elseCase, *notCase;
   infer *inferences;
   BDDNodeStruct *next;
   void *addons;
   
   // bdd2cnf.cc addons
   int t_var;
   // bdd2cnf.cc end

   // BDDWalksat addons
	int hamming;
	float density;
	float tbr_weight;
   // BDDWalksat end
} BDDNode;

extern   int nmbrFunctions;
extern   int original_numout;
extern   struct BDDNodeStruct **functions;
extern   struct BDDNodeStruct **original_functions;
extern   struct BDDNodeStruct **xorFunctions;
extern   int *functionType;
extern   int *equalityVble; // Variable on the LHS of an ite=, and=, or or= BDD.
extern   int *independantVars;
extern   char **labels;
extern   varinfo *variablelist;
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

