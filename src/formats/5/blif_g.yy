%name-prefix="blif_"
%{
#include "sbsat.h"
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

   symrec **blif_varlist = NULL;
   int blif_varmax = 0;
   int blif_varindex;

   extern int lines;
   extern int t_sym_max;

   void blif_nothing() { /*unput (0);*/ }
   void blif_reallocate_varlist();

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

%token MODEL INPUTS OUTPUTS LATCH NAMES END ID ZEROONE TT TTONE
//%type <id> ID TT TTONE // STRING U_OP
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
            { blif_varlist[blif_varindex] = NULL; ite_flag_vars(blif_varlist, /*independent - 1*/1);  }
	   | INPUTS 
;

outputlist: /* empty */
           | outputlist output
;

output:  OUTPUTS varlist 
            { blif_varlist[blif_varindex] = NULL; ite_flag_vars(blif_varlist, /*dependent - 0*/0);  }
	    | OUTPUTS 
;

commandslist: /* empty */
         | commandslist commands
;

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
	         { blif_varindex=0; blif_reallocate_varlist(); blif_varlist[blif_varindex++] = 0; /*s_getsym($1);*/ }
	      | varlist ID
	         { blif_reallocate_varlist(); blif_varlist[blif_varindex++] = 0; /*s_getsym($2);*/ }
;

truthlist: truthline
        | truthlist truthline
;

truthline: TT
         | TTONE
;

%%

void
blif_reallocate_varlist()
{
   if (blif_varindex >= blif_varmax)
   {
      blif_varlist = (symrec **)ite_recalloc((void*)blif_varlist, blif_varmax, blif_varmax+100, sizeof(int), 9, "blif_varlist");
      blif_varmax += 100;
   }
}

