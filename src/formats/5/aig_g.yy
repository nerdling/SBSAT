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

%token UNS_INT P_AIG COMMENT_HEADER STRING IO_IDENTIFIER
%type <num> UNS_INT

%% /* Grammar rules and actions follow */

input:  header clauses
;

header: /* empty */
	| P_AIG UNS_INT UNS_INT UNS_INT UNS_INT UNS_INT
	{printf("header!\n");}
;

clauses: /* empty */
	| inputs latches outputs ands symbols comments
;

inputs: /* empty */
	| inputs input
;

latches: /* empty */
	| latches latch
;

outputs: /* empty */
	| outputs output
;

ands: /* empty */
	| ands and
;

symbols: /* empty */
	| symbols symbol
;

comments: /* empty */
	| comment_header comment_lines
;

comment_lines: /* empty */
	| comment_lines comment_line
;

input: UNS_INT
	{printf("input = %d\n",$1);}
;

output: UNS_INT
	{printf("output = %d\n",$1);}
;

latch: UNS_INT UNS_INT
	{printf("latch = %d %d\n",$1,$2);}
;

and: UNS_INT UNS_INT UNS_INT
	{printf("and = %d %d %d\n",$1,$2,$3);}
;

symbol: IO_IDENTIFIER STRING
;

comment_header: COMMENT_HEADER
;

comment_line: STRING
;

%%

