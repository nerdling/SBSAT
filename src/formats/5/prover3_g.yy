%name-prefix="prover3_"
%{
#include "sbsat.h"
#include "bddnode.h"
#include "symtable.h"
#include "functions.h"
#include "p3.h"

   int prover3_lex();
   void prover3_error(const char *);

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
      { if (strcmp($1, "S0")) { fprintf(stderr, "missing S0\n"); exit(1); } p3_done(); }
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


