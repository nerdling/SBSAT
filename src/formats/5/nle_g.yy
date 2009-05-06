%name-prefix="nle_"
%{
#define  nle_error yy_error
#include <stdio.h>
#include "bddnode.h"
#include "symtable.h"

#include "cuddObj.hh"

#define YYDEBUG 0
int nle_counter = 0;

Cudd *nle_manager;	
ZDD nle_zdd;
ZDD nle_monomial;
int nle_new_monomial = 1;
int nle_nvars;


int *nle_output_literals;
int nle_array[3];
int nle_index = 0;
char *nle_symbol_name;
char nle_string_buffer[1000];
int nle_string_index = 0;

int nle_lex();
void nle_error(const char*);

#define CACHE_SLOTS 1444444
#define MAX_MEMORY 750000000

//#define YYSTYPE int

//void nle_nothing() { goto yyerrlab1; };

#ifndef __attribute__
#define __attribute__(x)
#endif

%}

%union {
    int  num;      /* For returning numbers.               */
    char id[200];  /* For returning ids.                   */
    BDDNode *bdd;  /*                                      */
}

%token P_nle VAR NEWLINE MULT PLUS UINT CONSTANT
%type <num> UINT 
%type <id> VAR
//%type <id> word WORD IO_IDENTIFIER

%% /* Grammar rules and actions follow */

file:  header formulas
{

}
;

header: /* empty */
	| P_nle UINT NEWLINE
	{ 
	nle_nvars = $2;
	printf("num vars %d\n",nle_nvars);
	nle_manager = new Cudd(0,nle_nvars,CUDD_UNIQUE_SLOTS,CACHE_SLOTS,MAX_MEMORY);
	// doesn't seem to be possible to grow the number of variables with ZDDs.
	nle_zdd = nle_manager->zddZero();
	//nle_monomial = nle_manager->zddOne(0);

}	
;


newlines: NEWLINE | NEWLINE newlines;


formulas:  newlines | /* empty */ | formula_entry formulas;


formula_entry: formula newlines
{

  nle_zdd.print(2,3);
  nle_zdd.PrintMinterm();
}
;

formula: constant | monomial_term | monomial_term plus formula | constant plus formula;


var: VAR
{

  symrec *varid;
  if((varid = getsym($1)) == NULL)
    varid = putsym($1,SYM_VAR);

  printf("var%s %d",$1,varid->id);
  if (nle_new_monomial = 1){
    nle_monomial = nle_manager->zddVar(varid->id - 1);
    nle_new_monomial = 0;
  }
  else nle_monomial *= nle_manager->zddVar(varid->id-1);


}

constant: CONSTANT
{
  nle_zdd += nle_manager->zddOne(0);
  printf("constant\n");
}
;

plus: PLUS
{
  printf(" plus ");
}

monomial: var | monomial MULT var
{


}

monomial_term: monomial
{

  nle_zdd += nle_monomial;
  printf(" monomial ");
  nle_new_monomial = 1;


}

%%

