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

#ifndef SBSAT_PREPROC_H
#define SBSAT_PREPROC_H

#include "equivclass.h"

extern Equiv *l;
//extern int *tempint;
extern infer *lastinfer;
extern int notdone;
extern infer *inferlist;
extern int *num_funcs_var_occurs;
extern llistStruct *amount;
extern char *P1_repeat;
extern char *P2_repeat;
extern char *Restct_repeat;
extern char *ReFPS_repeat;
extern char *Steal_repeat;
extern char *St_repeat;
extern char *Dep_repeat;
extern char *Ea_repeat;
extern char *Es_repeat;
extern char *Sa_repeat;
extern int str_length;
extern long affected;

extern int num_inferences;
extern int Pos_replace;
extern int Neg_replace;
extern int Setting_Pos;
extern int Setting_Neg;

extern int length_size;
extern int variables_size;

extern int *original_functionType;
extern int *original_equalityVble;

extern int bdd_tempint_max;
extern int *bdd_tempint;

extern int STRENGTH;
extern double start_prep;

int countBDDs();
void Init_Repeats();
void Delete_Repeats();
void SetRepeats(int x);
void UnSetRepeats(int x);

int Rebuild_BDDx (int x);
int Rebuild_BDD (BDDNode *, int *, int *&);

BDDNode *strip_x_BDD(BDDNode *, int);
BDDNode *strip_x(int, int);
BDDNode *collect_x (BDDNode *, int);
int add_newFunctions(BDDNode **, int, int **);
int Do_Apply_Inferences ();
int Do_Apply_Inferences_backend ();
void printBDDInfs(BDDNode *);
BDDNode *safe_assign(BDDNode *f, int v);
BDDNode *safe_assign_eq(BDDNode *f, int v);
BDDNode *safe_assign_func(BDDNode *f, int v);
BDDNode *safe_assign_all(BDDNode **bdds, llistStruct *amount, int v);
BDDNode *safe_assign_eq_all(BDDNode **bdds, llistStruct *amount, int v);
void set_variable_all_bdds(llist *k, int num, int torf, BDDNode **bdds);
infer *possible_infer_x(BDDNode *f, int x);
BDDNode *possible_BDD_x(BDDNode *f, int x);
int check_bdd_for_safe(int x, int ret);
int check_bdd_for_safe_eq(int x, int ret);
int Do_Strength();
int Do_SimpleAnd();
int Do_Pruning();
int Do_Pruning_1();
int Do_Pruning_2();
int Do_Pruning_Restrict();
int Do_Steal();
int Do_Cofactor();
int Do_ExQuantify();
int Do_ExQuantifyAnd();
int Do_ExSafeCluster();
int Do_PossibleAnding();
int Do_DepCluster();
int Do_Split();
int Do_Rewind();
int Do_Clear_FunctionType();
int Do_Find_FunctionType();
int Do_Prover3();
int Do_Identify_Same_Structure();
int Do_SafeAssign();
int Do_SafeSearch();
int Do_ExtendRes();
int Do_Diameter();
void Do_Flow();
void Do_Flow_Grouping();
int Do_Message_Passing();

int Finish_Preprocessing();
int Init_Preprocessing();

int CreateInferences();

#endif
