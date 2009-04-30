%name-prefix="nle_"
%{
#define  nle_error yy_error
#include <stdio.h>
#include "bddnode.h"
#include "symtable.h"
#define YYDEBUG 0
int nle_counter = 0;
int nle_inputs;
int nle_latches;
int nle_outputs;
int nle_ands;
int nle_symbols;

int *nle_input_vars;
int *nle_output_literals;
int nle_array[3];
int nle_index = 0;
char *nle_symbol_name;
char nle_string_buffer[1000];
int nle_string_index = 0;

int nle_lex();
void nle_error(const char*);
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

%token UINT P_nle COMMENT_HEADER WORD IO_IDENTIFIER NEW_LINE
%type <num> UINT 
%type <id> word WORD IO_IDENTIFIER

%% /* Grammar rules and actions follow */

file:  header clauses
;

header: /* empty */
	| P_nle UINT UINT UINT UINT UINT NEW_LINE
	{ nle_inputs = $3;
	  nle_latches = $4;
	  nle_outputs = $5;
	  nle_ands = $6;
	  d2_printf6("header! %d %d %d %d %d\n",$2,$3,$4,$5,$6);
	  if(nle_latches > 0){
	    fprintf(stderr, "\nInput contains latches...exiting.\n");
	    //exit(1);
	  }
	  if(nle_outputs > 1){
	    fprintf(stderr, "\nInput contains more than one output.\n");
	    //exit(1);
	  }
	  vars_alloc($2+1);
	  functions_alloc(nle_ands);
	  nle_input_vars = (int *) malloc(nle_inputs*sizeof(int));
	  nle_output_literals = (int *) malloc(nle_outputs*sizeof(int));
}	
;

clauses: /* empty */
	| lines symbols comments
;

lines: /* empty */
	| lines line
;

line: uints NEW_LINE
	{ 
	  nle_index = 0;
	  if(nle_counter < nle_inputs){
	    nle_input_vars[nle_counter] = nle_array[0]/2;
		 d8_printf3("%d input = %d \n",nle_counter, nle_array[0]);
	  }else if(nle_counter < nle_latches + nle_inputs){
	    d8_printf4("%d latch = %d %d\n",nle_counter, nle_array[0],nle_array[1]);
	  }else if(nle_counter < nle_outputs + nle_latches + nle_inputs){
	    nle_output_literals[nle_counter-nle_latches-nle_inputs] = nle_array[0];
	    d8_printf3("%d output = %d\n",nle_counter,nle_array[0]);
	    if(nle_array[0] == 0){
	      //functions_add(false_ptr, UNSURE, 0);
	    }else if(nle_array[0]%2){
	      //functions_add(ite_var(-(i_getsym_int(nle_array[0]-1, SYM_VAR))), UNSURE, 0);
	    }else functions_add(ite_var(i_getsym_int(nle_array[0], SYM_VAR)), UNSURE, 0);
	    //printBDD(ite_var(i_getsym_int(nle_array[0]%2?nle_array[0]-1:nle_array[0], SYM_VAR)));
	  }else if(nle_counter < nle_ands + nle_outputs + nle_latches + nle_inputs){
	    BDDNode *bdd_nle_array[3];
	    for(int i=0;i<3;i++){
	      if(nle_array[i] == 0){
					bdd_nle_array[i] = false_ptr;
	      }else if(nle_array[i] == 1){
					bdd_nle_array[i] = true_ptr;
	      }else if(nle_array[i]%2){
					bdd_nle_array[i] = ite_var(-(i_getsym_int(nle_array[i]-1, SYM_VAR)));
	      }else bdd_nle_array[i] = ite_var(i_getsym_int(nle_array[i], SYM_VAR));
	    }
	    functions_add(ite_equ(bdd_nle_array[2], ite_and(bdd_nle_array[1], bdd_nle_array[0])), UNSURE, 0);
			  D_8(printBDD(ite_equ(bdd_nle_array[2], ite_and(bdd_nle_array[1], bdd_nle_array[0])));)

	    d8_printf5("%d and = %d %d %d\n",nle_counter, nle_array[2], nle_array[1], nle_array[0]);

	  }else{
	    //shouldn't reach this case
	  }
	  nle_counter++; 
	}
;

uints: /* empty */
	| UINT uints
{ 
  nle_array[nle_index++] = $1;
  /*d8_printf2("\t %d\n",$1);*/
}
;


symbols: /* empty */
	| symbols symbol
;

symbol: IO_IDENTIFIER words NEW_LINE
{ 
  int index = atoi($1+1);
  d8_printf4("symbol %c %d = %s\n",*$1,index,nle_string_buffer);
  nle_string_index = 0;
  nle_string_buffer[0] = '\0';
  if(*$1 == 'i'){
//    putsym_with_id($2, SYM_VAR, nle_input_vars[index]);
    d8_printf2("\t%d\n",nle_input_vars[index]);
  }else if(*$1 == 'o'){
//    putsym_with_id($2, SYM_VAR, nle_output_literals[index]/2);
    d8_printf2("\t%d\n",nle_output_literals[index]/2);
  }

}

;


comments: /* empty */
	| comment_header comment_lines
;

comment_lines: /* empty */
	| comment_lines comment_line
{
  d8_printf2("comment: %s \n",nle_string_buffer);
  nle_string_index = 0;
  nle_string_buffer[0] = '\0';
}
;


comment_header: COMMENT_HEADER NEW_LINE
;

comment_line: words NEW_LINE
;

words: /* empty */
	| words word
{  
  strncat(nle_string_buffer, $2, nle_string_index<1000?1000-nle_string_index:0);
  nle_string_index += strlen($2);
  strncat(nle_string_buffer, " ", nle_string_index<1000?1000-nle_string_index:0);
  nle_string_index++;
}
	| words UINT
{  
  char int_str[10];
  sprintf(int_str,"%d",$2);
  strncat(nle_string_buffer, int_str, nle_string_index<1000?1000-nle_string_index:0);
  nle_string_index += strlen(int_str);
  strncat(nle_string_buffer, " ", nle_string_index<1000?1000-nle_string_index:0);
  nle_string_index++;
}
;


word: WORD
{  
  /*
  strncat(nle_string_buffer, $1, nle_string_index<1000?1000-nle_string_index:0);
  nle_string_index += strlen($1);
  strncat(nle_string_buffer, " ", nle_string_index<1000?1000-nle_string_index:0);
  nle_string_index++;
  */
}
;


%%

