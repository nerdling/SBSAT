%name-prefix="bdd_"
%{
#define  bdd_error yy_error
#include <stdio.h>
//#define YYSTYPE int
extern int i;
int bdd_lex();
void bdd_error(const char *);

#ifndef __attribute__
#define __attribute__(x)
#endif

%}

%union {
    int  num;      /* For returning numbers.               */
    char id[200];  /* For returning ids.                   */
}

%token INTNUMBER ID S_OP OP OP_ITE U_OP BDDID P_BDD ADD_STATE
%token STRING C_OP 
%type <id> ID OP STRING
%type <num> INTNUMBER


%% /* Grammar rules and actions follow */

input:  header clauses
;

header: /* empty */
        | P_BDD INTNUMBER INTNUMBER 
;

clauses: /* empty */
        | clauses clause
;

clause: star cmd
        { printf("=>functions[%d]\n",i++); }
;

star: /* empty */
	| '*'
;

cmd:  ADD_STATE '(' BDDID ',' INTNUMBER ')'
	| exp
;

bddlist: exp
	{ }
	| bddlist ',' exp
	{ }
;

exp:      INTNUMBER
	{ }
	| OP_ITE '(' exp ',' exp ',' exp ')'
	{ }
	| OP '(' bddlist ')'
	{ }
	| U_OP '(' BDDID ')'
	{ }
;

%%

