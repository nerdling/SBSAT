%name-prefix="aig_"
%{
#define  aig_error yy_error
#include <stdio.h>
#include "bddnode.h"
int j = 0;
int aig_lex();
void aig_error(const char*);
//#define YYSTYPE int

//void aig_nothing() { goto yyerrlab1; };

#ifndef __attribute__
#define __attribute__(x)
#endif

%}

%union {
    int  num;      /* For returning numbers.               */
    char id[200];  /* For returning ids.                   */
    BDDNode *bdd;  /*                                      */
}

%token ZERO NON_ZERO P_AIG
%type <num> NON_ZERO ZERO
%type <bdd> non_zeros


%% /* Grammar rules and actions follow */

input:  header clauses
;

header: /* empty */
	| P_AIG NON_ZERO NON_ZERO
;

clauses: /* empty */
	| clauses clause
;

clause: non_zeros ZERO
	{ printf("=>functions[%d]\n",j++); }
;

non_zeros: NON_ZERO
	{ $$=ite_var($1); printf("%d ", $1); }
	| non_zeros NON_ZERO
	{ /*$$=ite_or($2, ite_var($1) );*/ printf("%d ", $2); }
;
	
%%

