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
#ifndef PROTOTYPES_H
#define PROTOTYPES_H

#include <stdio.h>
#include "tracer.h"
#include "equivclass.h"

/*
 * great for small problems
 * bigger problems == take more memory or is slower depending on params
 */
//#define BDDIDX_STORAGE

/*  BDD hash table with variable size/overlapping buckets */
#define BDD_VSB_STORAGE

#ifdef BDD_VSB_STORAGE
#define find_or_add_node bddvsb_find_or_add_node
#define bdd_init         bddvsb_init
#endif

#ifdef BDDIDX_STORAGE
#define find_or_add_node bddidx_find_or_add_node
#define bdd_init         bddidx_init
#endif

#ifndef BDD_VSB_STORAGE
#ifndef BDDIDX_STORAGE
#define find_or_add_node orig_find_or_add_node
#define bdd_init         bddorig_init
#endif
#endif

int walkSolve();
int solve(Tracer * &tracer);
void write_output (char formatout, Tracer * &tracer);
int read_input(char formatin, char formatout, Tracer * &tracer);

double get_runtime();
long get_memusage();
void unravelBDD (long *, int *, BDDNode *);
int compfunc (const void *, const void *);
int revcompfunc (const void *x, const void *y);
int abscompfunc (const void *, const void *);
int absrevcompfunc (const void *, const void *);

void bdd_init();
BDDNode *find_or_add_node(int, BDDNode *, BDDNode *);
BDDNode *ite(BDDNode *, BDDNode *, BDDNode *);

#ifdef NO_BDD_MACROS
BDDNode *ite_var (int);
BDDNode *ite_not(BDDNode *);
BDDNode *ite_and(BDDNode *, BDDNode *);
BDDNode *ite_or(BDDNode *, BDDNode *);
BDDNode *ite_equ(BDDNode *, BDDNode *);
BDDNode *ite_xor(BDDNode *, BDDNode *);
BDDNode *ite_imp(BDDNode *, BDDNode *);
BDDNode *ite_itequ(BDDNode *, BDDNode *, BDDNode *, BDDNode *);
BDDNode *ite_nand(BDDNode *, BDDNode *);
BDDNode *ite_nor(BDDNode *, BDDNode *);
BDDNode *ite_nimp(BDDNode *, BDDNode *);
BDDNode *bdd2xdd(BDDNode *);
int splitXors();
void countSingleXors(BDDNode *, int *, int *);
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

void SAT_to_CNF();
void Smurfs_to_BDD();
void Do_Lemmas();
void writestring(char *s);
void myputc(char c);
char mygetc(void);
void myungetc(char c);
void readfile();
int look_up(char *s);

BDDNode *ite(BDDNode *, BDDNode *, BDDNode *);
BDDNode *ite_xor3(BDDNode *, BDDNode *, BDDNode *);
BDDNode *ite_xor4(BDDNode *, BDDNode *, BDDNode *, BDDNode *);
BDDNode *ite_xor5(BDDNode *, BDDNode *, BDDNode *, BDDNode *, BDDNode *);
BDDNode *ite_xor6(BDDNode *, BDDNode *, BDDNode *, BDDNode *, BDDNode *, BDDNode *);
BDDNode *ite_xor7(BDDNode *, BDDNode *, BDDNode *, BDDNode *, BDDNode *, BDDNode *, BDDNode *);
BDDNode *ite_xor8(BDDNode *, BDDNode *, BDDNode *, BDDNode *, BDDNode *, BDDNode *, BDDNode *, BDDNode *);
BDDNode *ite_xor9(BDDNode *, BDDNode *, BDDNode *, BDDNode *, BDDNode *, BDDNode *, BDDNode *, BDDNode *, BDDNode *);
BDDNode *ite_xor10(BDDNode *, BDDNode *, BDDNode *, BDDNode *, BDDNode *, BDDNode *, BDDNode *, BDDNode *, BDDNode *, BDDNode *);
BDDNode *ite_and3(BDDNode *, BDDNode *, BDDNode *);
BDDNode *ite_and4(BDDNode *, BDDNode *, BDDNode *, BDDNode *);
BDDNode *ite_and5(BDDNode *, BDDNode *, BDDNode *, BDDNode *, BDDNode *);
BDDNode *ite_and6(BDDNode *, BDDNode *, BDDNode *, BDDNode *, BDDNode *, BDDNode *);
BDDNode *ite_and7(BDDNode *, BDDNode *, BDDNode *, BDDNode *, BDDNode *, BDDNode *, BDDNode *);
BDDNode *ite_and8(BDDNode *, BDDNode *, BDDNode *, BDDNode *, BDDNode *, BDDNode *, BDDNode *, BDDNode *);
BDDNode *ite_and9(BDDNode *, BDDNode *, BDDNode *, BDDNode *, BDDNode *, BDDNode *, BDDNode *, BDDNode *, BDDNode *);
BDDNode *ite_and10(BDDNode *, BDDNode *, BDDNode *, BDDNode *, BDDNode *, BDDNode *, BDDNode *, BDDNode *, BDDNode *, BDDNode *);
BDDNode *ite_or3(BDDNode *, BDDNode *, BDDNode *);
BDDNode *ite_or4(BDDNode *, BDDNode *, BDDNode *, BDDNode *);
BDDNode *ite_or5(BDDNode *, BDDNode *, BDDNode *, BDDNode *, BDDNode *);
BDDNode *ite_or6(BDDNode *, BDDNode *, BDDNode *, BDDNode *, BDDNode *, BDDNode *);
BDDNode *ite_or7(BDDNode *, BDDNode *, BDDNode *, BDDNode *, BDDNode *, BDDNode *, BDDNode *);
BDDNode *ite_or8(BDDNode *, BDDNode *, BDDNode *, BDDNode *, BDDNode *, BDDNode *, BDDNode *, BDDNode *);
BDDNode *ite_or9(BDDNode *, BDDNode *, BDDNode *, BDDNode *, BDDNode *, BDDNode *, BDDNode *, BDDNode *, BDDNode *);
BDDNode *ite_or10(BDDNode *, BDDNode *, BDDNode *, BDDNode *, BDDNode *, BDDNode *, BDDNode *, BDDNode *, BDDNode *, BDDNode *);
BDDNode *gcf(BDDNode *, BDDNode *);
int nmbrVarsInCommon(int, int, int *&, store *&, int);
BDDNode* strengthen(int, int, int *&, store *&);
BDDNode *xquantify(BDDNode *, int);
BDDNode *uquantify(BDDNode *, int);
int countnodes(BDDNode *);

#define print_bdd(f) print_bdd1(f, 0)
void print_bdd1(BDDNode *, int);
void verifyCircuit(int);
void printBDDerr(BDDNode *);
void printBDD(BDDNode *);
void printBDDfile(BDDNode *, FILE *);
void printBDDTree(BDDNode *, int *);
BDDNode *ReadSmurf(int *, char *, int, int *, int);
BDDNode *mitosis(BDDNode *, int *, int *);
BDDNode *f_apply(BDDNode *, BDDNode**);
BDDNode *MinMaxBDD(int *, int, int, int, int);
void unmark(BDDNode *bdd);
char getformat (FILE *);
void DNF_to_CNF();
void CNF_to_BDD(int);
//CircuitStruct *split(int, int);
void cheat_replace(BDDNode *, int, int);
BDDNode *num_replace(BDDNode *, int, int);
BDDNode *remove_fpsx (int, int, int *&, store *&);
BDDNode *remove_fps(BDDNode *, BDDNode *);
BDDNode *restrictx (int, int, int *&, store *&);
BDDNode *restrict (BDDNode *, BDDNode *);
BDDNode *p2(BDDNode *, BDDNode *);
BDDNode *pruning(BDDNode *, BDDNode *);
void findPathsToFalse(BDDNode *, int *, int, intlist *, int *);
void readCircuit ();
void BDD_to_Smurfs();

void printCircuit();
void printCircuitTree();
void printSchlipfCircuit();
void printBDDToCNF();
void printBDDToCNFQM();
void printBDDToCNF3SAT();

void GetInferFoAN(BDDNode *);
infer *GetInfer(long *, int *, BDDNode *);
BDDNode *set_variable(BDDNode *, int, int);
int bdd_circuit_init(int n_vars, int n_fns);
void bcount(char *, int);
int getEquiv(int, int *, int);
int *getANDLiterals(int, int *, int);
int getTruth (int *, char *, int , BDDNode *);
void bddloop();
int Preprocessor(varinfo *);
int getNuminp();
int countFalses(BDDNode *);
int makeAllResolutions(intlist *&, int, int);
void SolveItSTDIN(Tracer *);
void finalCheck(Tracer *, int *);
void Backend_Trace(int, int, int *, Tracer *);
void Backend_Trace_NoSolver(int, int *, Tracer *);
void Backend_CNF(int, int, int *);
void Backend_CNF_NoSolver(int, int *);
void writeCircuit();
void readCircuit_wvf();

ITE_INLINE void DisplaySolution(int, int []);
void DisplayTraceInferenceQueue();

int check_gzip (char *filename);
FILE * zread(char *filename);

void xorloop ();

void memcpy_ite(void*, void*, int);
void get_tempfile(char *basename, char *filename, int filename_max);
void ShowResultLine(FILE *fout, char *var, int var_idx, int negative, int value);

ITE_INLINE void
EnterIntoLemmaSpace(int nNumElts, int arrLemmaLiterals[],
		    bool bRecycleLemmasAsNeeded, LemmaBlock *&pFirstBlock,
		    LemmaBlock *&pLastBlock, int &nNumBlocks);


//#define MEMCPY memcpy_ite
#define MEMCPY memcpy


#endif
