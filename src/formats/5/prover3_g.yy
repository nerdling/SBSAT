%name-prefix="prover3_"
%{
#include "ite.h"
#include "bddnode.h"
#include "symtable.h"
#include "functions.h"

   int prover3_lex();
   void prover3_error(const char *);
   BDDNode *ite_op(proc_op2fn fn, int *);
   void     ite_op_id_equ(char *var, BDDNode *bdd);
   void     ite_op_equ(char *var, t_op2fn fn, BDDNode **);
   BDDNode *ite_op_exp(t_op2fn fn, BDDNode **);
   void     ite_op_are_equal(BDDNode **);
   void     ite_new_int_leaf(char *, char *);
   void     ite_flag_vars(symrec **, int);
   BDDNode *tmp_equ_var(BDDNode *p);
   void push_symbols();
   void pop_symbols();
   void set_S_vars_indep(symrec *s);

   /* FIXME: make it more dynamic! */
   extern symrec *varlist[1000];
   extern int varindex;

   /* FIXME: make it more dynamic! */
   extern BDDNode *explist[10][1000];
   extern int expindex[10];
   extern int explevel;

   extern int lines;
   extern int normal_bdds;
   extern int spec_fn_bdds;
   extern int t_sym_max;

   void p3_add(char *dst, int argc, int tag, char *arg1, char *arg2);
   void c3_done();

#define NOT_Tag 1
#define AND_Tag 2
#define OR_Tag 3
#define IMP_Tag 4
#define EQUIV_Tag 5

#ifndef __attribute__
#define __attribute__(x)
#endif

%}

%union {
    int         num;      /* For returning numbers.               */
    char        id[200];  /* For returning ids.                   */
    t_op2fn     op2fn;    /* For returning op2fn                  */
    BDDNode     *bdd;     /* For returning exp                    */
}

%token ID 
%left P_EQUIV P_IMP  /* lowest precedence */
%left '#'
%left '&'
%left '~' /* highest precedence */
%type <id> ID 

%% /* Grammar rules and actions follow */

start: ID top_exp_list
      { if (strcmp($1, "S0")) { fprintf(stderr, "missing S0\n"); exit(1); } c3_done(); }
;

top_exp_list: /* nothing */
   | top_exp_list top_exp
;

top_exp:  
     '&' '(' ID P_EQUIV '~' ID ')' 
      { p3_add($3, 1, NOT_Tag, $6, NULL); }
   | '&' '(' ID P_EQUIV ID '#' ID ')' 
      { p3_add($3, 2, OR_Tag, $5, $7); }
   | '&' '(' ID P_EQUIV ID '&' ID ')' 
      { p3_add($3, 2, AND_Tag, $5, $7); }
   | '&' '(' ID P_EQUIV '(' ID P_EQUIV ID ')' ')' 
      { p3_add($3, 2, EQUIV_Tag, $6, $8); }
   | '&' '(' ID P_EQUIV '(' ID P_IMP ID ')' ')' 
      { p3_add($3, 2, IMP_Tag, $6, $8); }
;


%%

typedef
struct {
   int tag;
   int argc;
   int arg1;
   int arg2;
   /*------*/
   BDDNode *bdd;
   int vars;
   /*------*/
   int ref;
   int top_and;
   int *refs;
   int refs_idx;
   int refs_max;
} t_ctl;

int c3_max = 0;
t_ctl *c3 = NULL;
int s2id(char *s_id);
void c3_dump();
void c3_traverse_ref(int i, int ref);
void c3_top_bdds(int idx);

void
c3_done()
{
   c3[0].top_and = 1;
   fprintf(stderr, "c3_traverse_ref\n");
   c3_traverse_ref(0, -1);
   //fprintf(stderr, "c3_dump\n");
   //c3_dump();
   fprintf(stderr, "c3_top_bdds\n");
   c3_top_bdds(0);
   fprintf(stderr, "all done\n");
}

void
c3_add_ref(int idx, int ref)
{
   if (c3[idx].refs_idx+1 >= c3[idx].refs_max) {
      c3[idx].refs_max += 10;
      c3[idx].refs = (int*)realloc(c3[idx].refs, c3[idx].refs_max*sizeof(int));
   }
   c3[idx].refs[c3[idx].refs_idx++] = ref;
}

void
c3_traverse_ref(int i, int ref)
{
   int idx = -1*i;
   assert(idx >= 0);
   c3[idx].ref++;
   if (ref >= 0) c3_add_ref(idx, ref);
   if (c3[idx].ref > 1) return;
   if (c3[idx].top_and && c3[idx].tag == AND_Tag) {
      if (c3[idx].arg1 < 0 && c3[-1*c3[idx].arg1].tag == AND_Tag) 
         c3[-1*c3[idx].arg1].top_and = 1;
      if (c3[idx].argc == 2 && c3[idx].arg2 < 0 && c3[-1*c3[idx].arg2].tag == AND_Tag) 
         c3[-1*c3[idx].arg2].top_and = 1;
   }
   if (c3[idx].arg1 < 0) c3_traverse_ref(c3[idx].arg1, idx);
   if (c3[idx].argc == 2 && c3[idx].arg2 < 0) c3_traverse_ref(c3[idx].arg2, idx);
}



void
c3_alloc(int max)
{
   c3 = (t_ctl*)realloc((void*)c3, max*sizeof(t_ctl));
   memset((char*)c3+c3_max*sizeof(t_ctl), 0, (max-c3_max)*sizeof(t_ctl));
   c3_max = max;
}

void
p3_add(char *dst, int argc, int tag, char *arg1, char *arg2)
{
   int i_dst = s2id(dst);
   int i_arg1 = 0;
   int i_arg2 = 0;

   if (arg1[0] == 'S') i_arg1 = -1*atoi(arg1+1);
   else i_arg1 = i_getsym(arg1, SYM_VAR); 
   if (argc == 2) {
      if (arg2[0] == 'S') i_arg2 = -1*atoi(arg2+1);
      else i_arg2 = i_getsym(arg2, SYM_VAR); 
   }
   if (c3_max <= i_dst) c3_alloc(i_dst+10);

   c3[i_dst].tag = tag;
   c3[i_dst].argc = argc;
   c3[i_dst].arg1 = i_arg1;
   c3[i_dst].arg2 = i_arg2;
/*
   if (argc == 1) 
      fprintf(stderr, "%s = not %s \n", dst, arg1);
   else
      fprintf(stderr, "%s = %s %s \n", dst, arg1, arg2);
*/
}

int
s2id(char *s_id)
{
   int i;
   if(s_id[0] != 'S') { fprintf(stderr, "temp var without 'S'\n"); exit(1); };
   i = atoi(s_id+1);
   return i;
}

void
c3_dump()
{
   int i;
   for(i=0;i<c3_max;i++) {
      switch (c3[i].argc) {
       case 2: fprintf(stderr, "%cS%d = %d %d (%d)\n", 
                     c3[i].top_and?'*':' ', i, c3[i].arg1, c3[i].arg2, c3[i].ref); break;
       case 1: fprintf(stderr, "%cS%d = %d (%d)\n", 
                     c3[i].top_and?'*':' ', i, c3[i].arg1, c3[i].ref); break;
       default: break;
      }
   }
}

#define VARS_MAX 10

BDDNode *
c3_bdds(int idx, int *vars)
{
   BDDNode *bdd = NULL;
   if (idx < 0) {
      BDDNode *bdd1 = NULL;
      BDDNode *bdd2 = NULL;
      int vars1 = 0;
      int vars2 = 0;
      idx = -1*idx;
      if (c3[idx].bdd != NULL) {
         *vars = c3[idx].vars;
         return c3[idx].bdd;
      }
      bdd1 = c3_bdds(c3[idx].arg1, &vars1);
      if (vars1 > VARS_MAX) {
         bdd1 = tmp_equ_var(bdd1);
         vars1 = 1;
      }
      if (c3[idx].argc == 2) {
         bdd2 = c3_bdds(c3[idx].arg2, &vars2);
         if (vars2 > VARS_MAX) {
            bdd2 = tmp_equ_var(bdd2);
            vars2 = 1;
         }
      }
      switch(c3[idx].tag) {
       case OR_Tag: bdd = ite_or(bdd1, bdd2); break;
       case AND_Tag: bdd = ite_and(bdd1, bdd2); break;
       case IMP_Tag: bdd = ite_imp(bdd1, bdd2); break;
       case EQUIV_Tag: bdd = ite_equ(bdd1, bdd2); break;
       case NOT_Tag: bdd = ite_not(bdd1); break;
       default: assert(0); exit(1); break;
      }
      if (vars1+vars2 > VARS_MAX) {
         
         bdd = tmp_equ_var(bdd);
      }
      *vars = vars1 + vars2;
      c3[idx].bdd = bdd;
      c3[idx].vars = *vars;
   } else {
      /* variable */
      bdd = ite_var(idx);
      *vars = 1;
   }
   return bdd;
}

void
c3_top_bdds(int idx)
{
   int vars;
   if (idx > 0 || c3[-1*idx].top_and == 0) {
      functions_add(c3_bdds(idx, &vars), UNSURE, 0);
   } else {
      idx = -1*idx;
      fprintf(stderr, "%d vars=%d functions=%d\r", idx, vars_max, functions_max);
      c3_top_bdds(c3[idx].arg1);
      if (c3[idx].argc==2) c3_top_bdds(c3[idx].arg2);
   }
}

