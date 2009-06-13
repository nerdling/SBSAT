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

#ifndef PROTOTYPES_H
#define PROTOTYPES_H


double get_runtime();
long get_memusage();
void unravelBDD (int *, int *, int **, BDDNode *);
int compfunc (const void *, const void *);
int revcompfunc (const void *, const void *);
int abscompfunc (const void *, const void *);
int absrevcompfunc (const void *, const void *);


BDDNode *and_dot(BDDNode *, BDDNode *);
BDDNode *constant_and(BDDNode *, BDDNode *);
BDDNode *possible_BDD(BDDNode *, int);

BDDNode *bdd2xdd(BDDNode *);
BDDNode *xddnot(BDDNode *);
BDDNode *xddxor(BDDNode *, BDDNode *);
BDDNode *xddand(BDDNode *, BDDNode *);

int splitXors();
void countSingleXors(BDDNode *, int *, int *);
int are_oppos(BDDNode *, BDDNode *);

void SAT_to_CNF();
void Smurfs_to_BDD();
void Do_Lemmas();
void writestring(char *s);
void myputc(char c);
char mygetc(void);
void myungetc(char c);
void readfile();
int look_up(char *s);

BDDNode *gcf(BDDNode *, BDDNode *);
int nmbrVarsInCommon(int, int, int);
BDDNode *strengthen(int, int);
BDDNode *strengthen_fun(BDDNode *, BDDNode *);
BDDNode *xquantify(BDDNode *, int);
BDDNode *uquantify(BDDNode *, int);
BDDNode *safe_assign(BDDNode *, int);
BDDNode *safe_assign_eq(BDDNode *, int);
BDDNode *safe_assign_func(BDDNode *, int);
int countnodes(BDDNode *);
int count_true_paths(BDDNode *, int *, int);
void marknodes(BDDNode *);

#define print_bdd(f) print_bdd1(f, 0)
void print_bdd1(BDDNode *, int);
int verifyBDD (BDDNode *, int);
void verifyCircuit(int);
void printBDDerr(BDDNode *);
void printBDD(BDDNode *);
void printBDDfile(BDDNode *, FILE *);
void printBDDfile_sym(BDDNode *, FILE *);
void printBDDTree(BDDNode *, int *);
void printITEBDD(BDDNode *);
void printBDDdot_file(BDDNode **, int);
void printBDDdot_stdout(BDDNode **, int);
void PrintSmurfs(BDDNode **bdds, int size); //Used to display the SMURFS via the dot format.
BDDNode *ReadSmurf(int *, char *, int, int *, int);
BDDNode *mitosis(BDDNode *, int *, int *);
BDDNode *f_mitosis(BDDNode *, BDDNode **, int *);
BDDNode *shared_structure(BDDNode *, int, int);
BDDNode *f_apply(BDDNode *, BDDNode**);
BDDNode *MinMaxBDD(int *, int, int, int, int);
void unmark(BDDNode *);
float mark_trues(BDDNode *);
void Fill_Density();
double calculateNeed(BDDNode *f, int v, int pos); //For calculating sigma of the message passing routines.
void DNF_to_CNF();
int CNF_to_BDD();
//CircuitStruct *split(int, int);
void cheat_replace(BDDNode *, int, int);
void num_replace_all(llist *, int , int);
BDDNode *num_replace(BDDNode *, int, int);
BDDNode *remove_fpsx (int, int);
BDDNode *remove_fps(BDDNode *, BDDNode *);
BDDNode *restrictx (int, int);
BDDNode *restrict (BDDNode *, BDDNode *);
BDDNode *pruning(BDDNode *, BDDNode *);
BDDNode *pruning_p1(BDDNode *, BDDNode *);
BDDNode *pruning_p2(BDDNode *, BDDNode *);
BDDNode *steal(BDDNode *, BDDNode *);
BDDNode *Build_BDD_From_Inferences(BDDNode *);
void findPathsToX (BDDNode *, int *, int **, int, intlist *, int *, BDDNode *);
void findPathsToFalse(BDDNode *, int *, int **, intlist *, int *);
void findPathsToTrue(BDDNode *, int *, int **, intlist *, int *);
void readCircuit ();
void BDD_to_Smurfs();

void printCircuit();
void printCircuitTree();
void printSchlipfCircuit();
void printBDDToCNF();
void printBDDToCNFQM();
void printBDDToCNF3SAT();
void printBDDToAAG();

void GetInferFoAN(BDDNode *);
infer *GetInfer(int *, int *, int **, BDDNode *);
BDDNode *set_variable_noflag(BDDNode *, int, int);
void set_variable_all(llist *, int, int);
BDDNode *set_variable(BDDNode *, int, int);
BDDNode *set_variable_all_infs(BDDNode *);
BDDNode *set_variable_noflag(BDDNode *, int, int);

//XDD set variable
BDDNode *set_variable_xdd_noflag(BDDNode *, int, int);
BDDNode *set_variable_xdd(BDDNode *, int, int);

extern int vars_max;
int vars_alloc(int);
extern int functions_max;
int functions_alloc(int);
void bcount(char *, int);
int getEquiv(int, int *, int);
int *getANDLiterals(int, int *, int);
int getTruth (int *, char *, int , BDDNode *);
void bddloop();
int Preprocessor();
int getNuminp();
int countX (BDDNode *, BDDNode *);
int countFalses(BDDNode *);
int countTrues(BDDNode *);
int makeAllResolutions(intlist *&, int, int);
Recd *resolve (int *, int, int, int);
void getRidOfRecdList (Recd *);
void writeCircuit();
void readCircuit_wvf();

ITE_INLINE void DisplaySolution(int, int []);
void DisplayTraceInferenceQueue();

void xorloop ();

void memcpy_ite(void*, void*, int);
void get_freefile(char *basename, char *file_dir, char *filename, int filename_max);
void ShowResultLine(FILE *fout, char *var, int var_idx, int negative, int value);

void
reload_bdd_circuit(int _numinp, int _numout,
                   void *_bddtable, int _bddtable_len,
                   void *_bddtable_start,
                   void *_equalityvble, 
                   void *_functions, 
                   void *_functiontype, 
                   void *_length, 
                   void *_variablelist,
                   void *_independantVars);

void
get_bdd_circuit(int *_numinp, int *_numout,
                   void **_bddtable, int *_bddtable_len, int *_bddtable_msize,
                   void **_equalityvble, 
                   void **_functions,  int *_functions_msize,
                   void **_functiontype, 
                   void **_length, 
                   void **_variablelist, int *_variablelist_msize,
                   void **_independantVars);
void
read_bdd_circuit();

// termcap 
int init_terminal_out();
int init_terminal_in();
void free_terminal_in();
void free_terminal_out();
void move(int col, int row);
void putpad(char *str);
int term_getchar();
extern char *CM, *SO, *SE, *CL;
extern int term_width, term_height;

void print_roller();
void print_nonroller();
void print_roller_init();

void DeallocateLLists(llist *next);
void DeallocateOneLList(llist *next);
llist *AllocateLList(int x, llist *next);

void DeallocateInferences(infer *next);
void DeallocateInferences_var(infer *next, int var);
void DeallocateOneInference(infer *next);
infer *AllocateInference(int num0, int num1, infer *next);

FILE * ite_fopen(char *filename, const char *fileflags);
long ite_filesize(char *filename);

BDDNode *tmp_equ_var(BDDNode *p);

int bdd_gc_test();
void bdd_gc(int force=0);

#endif
