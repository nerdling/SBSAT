%name-prefix="iscas_"
%{
#include "sbsat.h"
#include "bddnode.h"
#include "symtable.h"
#include "functions.h"

   int iscas_lex();
   void iscas_error(const char *);

   BDDNode *ite_op_exp(t_op2fn fn, BDDNode **);

   BDDNode **iscas_explist=NULL;
   int iscas_expmax=0;
   int iscas_expindex=0;

   void iscas_not(char *v1, char *v2);
   void iscas_and_equ(char *var, BDDNode **explist);
   void iscas_or_equ(char *var, BDDNode **explist);
   void iscas_reallocate_explist();

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
     { iscas_expindex=0; iscas_reallocate_explist(); iscas_explist[iscas_expindex++] = ite_vars(s_getsym($1, SYM_VAR)); }
   | varlist ',' ID
     { iscas_reallocate_explist(); iscas_explist[iscas_expindex++] = ite_vars(s_getsym($3, SYM_VAR)); }
;

%%

void
iscas_reallocate_explist()
{
   if (iscas_expindex >= iscas_expmax)
   {
      iscas_explist = (BDDNode **)ite_recalloc((void*)iscas_explist, iscas_expmax, iscas_expmax+100, sizeof(int), 9, "iscas_explist");
      iscas_expmax += 100;
   }
}


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


