%name-prefix="prover_"
%{
#include "ite.h"
#include "bddnode.h"
#include "symtable.h"
#include "functions.h"

   int prover_lex();
   void prover_error(const char *);
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
   int level = 0;
   int orlevel = 0;
   int symbols = 0;
   int p_level = 0;
   int p_symbols[20];

   void prover_nothing() { /*unput (0);*/ }

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
%type <bdd> exp 
// %type <num> INTNUMBER
// %type <op2fn> OP
// %type <bdd> exp exp_start


%% /* Grammar rules and actions follow */

top_exp: ID
    { symrec *s=s_getsym($1, SYM_VAR); assert(s); set_S_vars_indep(s); BDDNode *ret = ite_vars(s); functions_add(ret, UNSURE, 0); /*printf("top_id\n");*/ }
   | '~' exp
     {  functions_add($2, UNSURE, 0); /*printf("top_not\n");*/ assert(p_level==0); }
   | '(' exp ')'
     {  functions_add($2, UNSURE, 0); /*printf("top_par\n");*/ assert(p_level==0); }
   | top_exp '&' top_exp
;

exp:  ID
    { symrec *s=s_getsym($1, SYM_VAR); assert(s); set_S_vars_indep(s); $$ = ite_vars(s); symbols++; }
   |  '~' exp
     { $$ = ite_not( $2 ); }
   |  '(' { push_symbols(); } exp ')' { if (symbols >= 10) { $3=tmp_equ_var($3); symbols=0;}; pop_symbols(); }
     { $$ = $3; }
   | exp '&' exp 
     { $$ = ite_and($1, $3); }
   |  exp '#' { if (orlevel==0 && p_level==0) { $1 = tmp_equ_var($1); /*printf("ortop\n");*/ } orlevel++; } exp
     { orlevel--; if (orlevel==0 && p_level==0) { /*printf("orret\n");*/} $$=ite_or($1,$4); }
   |  exp P_EQUIV exp
     { $$ = ite_equ($1, $3); }
   |  exp P_IMP exp
     { $$ = ite_imp($1, $3); }
;


%%

void set_S_vars_indep(symrec *s)
{
   //if (s->name[0] == 'S') s_set_indep(s, 0);
}

void push_symbols()
{
   p_symbols[p_level++] = symbols; 
   symbols=0;
}

void pop_symbols()
{
   p_level--;
   symbols += p_symbols[p_level];
}

BDDNode *tmp_equ_var(BDDNode *p) 
{
    symrec *s_ptr = tputsym(SYM_VAR); 
    BDDNode *ret=ite_vars(s_ptr); 
    BDDNode *e=ite_equ(ret, p); 
    functions_add(e, UNSURE, 0/*s_ptr->id*/); /*printf("ex\n"); */
    return ret;
}
