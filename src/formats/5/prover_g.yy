%name-prefix="prover_"
%{
#include "sbsat.h"
#include "bddnode.h"
#include "symtable.h"
#include "functions.h"

   int prover_lex();
   void prover_error(const char *);

   void push_symbols();
   void pop_symbols();
   void set_S_vars_indep(symrec *s);

   int level = 0;
   int orlevel = 0;
   int symbols = 0;
   int p_level = 0;
   int p_symbols[50];

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
    { symrec *s=s_getsym($1, SYM_VAR); assert(s); set_S_vars_indep(s); BDDNode *ret = ite_vars(s); functions_add(ret, UNSURE, 0); }
   | '~' exp
     {  functions_add(ite_not($2), UNSURE, 0); assert(p_level==0); }
   | '(' exp ')'
     {  functions_add($2, UNSURE, 0); assert(p_level==0); }
   | top_exp '&' top_exp
;

exp:  ID
    { symrec *s=s_getsym($1, SYM_VAR); assert(s); set_S_vars_indep(s); $$ = ite_vars(s); symbols++; }
   |  '~' exp
     { $$ = ite_not( $2 ); }
   |  '(' { /*push_symbols();*/ } exp ')' { /*if (symbols >= 10) { $3=tmp_equ_var($3); symbols=0;}; pop_symbols();*/ }
     { $$ = $3; }
   | exp '&' exp 
     { $$ = ite_and($1, $3); }
   |  exp '#' { if (orlevel==0 && p_level==0) { /*$1 = tmp_equ_var($1);*/ } orlevel++; } exp
     { orlevel--; if (orlevel==0 && p_level==0) { } $$=ite_or($1,$4); }
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

