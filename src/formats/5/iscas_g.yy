%name-prefix="iscas_"
%{
#include "ite.h"
#include "bddnode.h"
#include "symtable.h"
#include "functions.h"

   int iscas_lex();
   void iscas_error(const char *);
   BDDNode *ite_op(proc_op2fn fn, int *);
   void     ite_op_id_equ(char *var, BDDNode *bdd);
   void     ite_op_equ(char *var, t_op2fn fn, BDDNode **);
   BDDNode *ite_op_exp(t_op2fn fn, BDDNode **);
   void     ite_op_are_equal(BDDNode **);
   void     ite_new_int_leaf(char *, char *);
   void     ite_flag_vars(symrec **, int);
   BDDNode *tmp_equ_var(BDDNode *p);
   void set_S_vars_indep(symrec *s);

   /* FIXME: make it more dynamic! */
   BDDNode *iscas_explist[30480];
   int iscas_expmax=30480;
   int iscas_expindex=0;

   extern int lines;
   extern int normal_bdds;
   extern int spec_fn_bdds;

void iscas_not(char *v1, char *v2);
void iscas_and_equ(char *var, BDDNode **explist);
void iscas_or_equ(char *var, BDDNode **explist);

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

%token ID P_AND P_OR P_NOT P_INPUT P_OUTPUT
%type <id> ID 

%% /* Grammar rules and actions follow */

top_exp_list: /* nothing */
   | top_exp_list top_exp
;

top_exp:  
     P_INPUT '(' ID ')' 
      { symrec  *s_ptr = s_getsym($3, SYM_VAR); s_set_indep(s_ptr, 1); }
   | P_OUTPUT '(' ID ')'
      { symrec  *s_ptr = s_getsym($3, SYM_VAR); s_set_indep(s_ptr, 0); 
         functions_add(ite_equ(ite_vars(s_ptr), true_ptr), UNSURE, s_ptr->id); }
   | ID '=' P_AND '(' varlist ')'
      { iscas_explist[iscas_expindex] = NULL; iscas_and_equ($1, iscas_explist); }
   | ID '=' P_OR '(' varlist ')'
      { iscas_explist[iscas_expindex] = NULL; iscas_or_equ($1, iscas_explist); }
   | ID '=' P_NOT '(' ID ')'
      { iscas_not($1, $5); }
;

varlist:   ID
     { iscas_expindex=0; iscas_explist[iscas_expindex++] = ite_vars(s_getsym($1, SYM_VAR)); }
   | varlist ',' ID
     { if (iscas_expindex == iscas_expmax) perror("expindex over"); iscas_explist[iscas_expindex++] = ite_vars(s_getsym($3, SYM_VAR)); }
;

%%

void
iscas_not(char *v1, char *v2)
{
   symrec *s=s_getsym(v1, SYM_VAR); 
   symrec *r=s_getsym(v2, SYM_VAR);
   assert(s && r); 
   BDDNode *bdd=ite_equ(ite_vars(s), ite_not(ite_vars(r)));
   functions_add(bdd, UNSURE, s->id);
}

void
iscas_and_equ(char *var, BDDNode **explist)
{
   symrec  *s_ptr=s_getsym(var, SYM_VAR);
   assert(s_ptr);
   s_set_indep(s_ptr, 0);
   t_op2fn fn;
   fn.fn=op_and; 
   fn.fn_type=AND_EQU;
   BDDNode *ptr = ite_op_exp(fn, explist);
   ptr = ite_equ(ite_vars(s_ptr), ptr);
   functions_add(ptr, AND_EQU, s_ptr->id);
}

void
iscas_or_equ(char *var, BDDNode **explist)
{
   symrec  *s_ptr=s_getsym(var, SYM_VAR);
   assert(s_ptr);
   s_set_indep(s_ptr, 0);
   t_op2fn fn;
   fn.fn=op_or; 
   fn.fn_type=OR_EQU;
   BDDNode *ptr = ite_op_exp(fn, explist);
   ptr = ite_equ(ite_vars(s_ptr), ptr);
   functions_add(ptr, OR_EQU, s_ptr->id);
}


