%name-prefix="blif_"
%{
#include "ite.h"
#include "bddnode.h"
#include "symtable.h"
#include "functions.h"

   int blif_lex();
   void blif_error(const char *);
   BDDNode *ite_op(proc_op2fn fn, int *);
   void     ite_op_id_equ(char *var, BDDNode *bdd);
   void     ite_op_equ(char *var, t_op2fn fn, BDDNode **);
   BDDNode *ite_op_exp(t_op2fn fn, BDDNode **);
   void     ite_op_are_equal(BDDNode **);
   void     ite_new_int_leaf(char *, char *);
   void     ite_flag_vars(symrec **, int);

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

%token MODEL INPUTS OUTPUTS LATCH NAMES END TT TTONE ID ZEROONE
%type <id> ID TT TTONE // STRING U_OP
// %type <num> INTNUMBER
// %type <op2fn> OP
//%type <bdd> exp exp_start


%% /* Grammar rules and actions follow */

module_input:    /* empty */
        | module_input module inputlist outputlist commandslist end
;

module:  /* empty */
        | MODEL ID
;

end:  /* empty */
    | END
      { /* print_symtable(); */ }
;

inputlist: /* empty */
           | inputlist input
;

input:  INPUTS varlist 
            { varlist[varindex] = NULL; ite_flag_vars(varlist, /*independent - 1*/1);  }
	   | INPUTS 
;

outputlist: /* empty */
           | outputlist output
;

output:  OUTPUTS varlist 
            { varlist[varindex] = NULL; ite_flag_vars(varlist, /*dependent - 0*/0);  }
	    | OUTPUTS 
;

commandslist: /* empty */
         | commandslist commands

commands:  latch
         | names

;

latch:  LATCH varlist ZEROONE
;

names:  namesheader namesbody
;

namesheader: NAMES varlist 
;

namesbody: truthlist
;

varlist:   ID
	         { varindex=0; varlist[varindex++] = 0; /*s_getsym($1);*/ }
	      | varlist ID
	         { varlist[varindex++] = 0; /*s_getsym($2);*/ }
;

truthlist: truthline
        | truthlist truthline
;

truthline: TT
         | TTONE
;

%%

