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
#ifndef PREPROC_H
#define PREPROC_H

#include "equivclass.h"
extern Linear *l;
//extern int *tempint;
extern infer *lastinfer;
extern int notdone;
extern infer *inferlist;
extern int *num_funcs_var_occurs;
extern llistStruct *amount;
extern int *P1_repeat;
extern int *P2_repeat;
extern int *Restct_repeat;
extern int *ReFPS_repeat;
extern int *Steal_repeat;
extern int *St_repeat;
extern int *Dep_repeat;
extern int str_length;
extern long affected;

void Init_Repeats();
void Delete_Repeats();
void SetRepeats(int x);
void UnSetRepeats(int x);

int Rebuild_BDDx (int x);
int Rebuild_BDD (BDDNode *, int *, int *&);
int add_newFunctions(BDDNode **, int);
int Do_Apply_Inferences ();
int Do_Apply_Inferences_backend ();
void printBDDInfs(BDDNode *);
infer *possible_infer_x(BDDNode *f, int x);
int Do_Strength();
int Do_Pruning();
int Do_Pruning_1();
int Do_Pruning_2();
int Do_Pruning_Restrict();
int Do_Steal();
int Do_Cofactor();
int Do_ExQuantify();
int Do_ExQuantifyAnd();
int Do_DepCluster();
int Do_Split();
int Do_Rewind();
void Do_Flow();
void Do_Flow_Grouping();

int Finish_Preprocessing();
int Init_Preprocessing();

int CreateInferences();

#endif
