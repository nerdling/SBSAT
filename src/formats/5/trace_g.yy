%name-prefix="trace_"
%{
#include "sbsat.h"
#include "bddnode.h"
#include "symtable.h"
#include "functions.h"

   int trace_lex();
   void trace_error(const char *);
   BDDNode *ite_op(proc_op2fn fn, int *);
   void     ite_op_id_equ(char *var, BDDNode *bdd);
   void     ite_op_equ(char *var, t_op2fn fn, BDDNode **);
   BDDNode *ite_op_exp(t_op2fn fn, BDDNode **);
   void     ite_op_are_equal(BDDNode **);
   void     ite_new_int_leaf(char *, char *);
   void     ite_flag_vars(symrec **, int);
   void trace_reallocate_varlist();
   void trace_reallocate_exp();
   void trace_free();

   symrec **trace_varlist = NULL;
   int trace_varmax = 0;
   int trace_varindex;

   BDDNode ***explist = NULL;
   int *expindex = NULL;
   int *expmax = NULL;
   int explevel_max;
   int explevel;


   int lines=0;

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

%token INTNUMBER MODULE ENDMODULE ID INPUT OUTPUT ARE_EQUAL OP OP_ITE U_OP U_OP_NOT
%token STRUCTURE OP_NEW_INT_LEAF TPRINT STRING C_OP
%type <id> ID STRING U_OP
%type <num> INTNUMBER
%type <op2fn> OP
%type <bdd> exp exp_start


%% /* Grammar rules and actions follow */

module_input:    /* empty */
        | module_input module input output structure endmodule
;

module:  MODULE ID
;

endmodule:  ENDMODULE
            { /* print_symtable(); */ trace_free(); }
;

input: /* empty */
	      | INPUT varlist ';'
            { trace_varlist[trace_varindex] = NULL; trace_reallocate_varlist(); ite_flag_vars(trace_varlist, /*independent - 1*/1);  }
	      | INPUT ';'
;

output: /* empty */
	      | OUTPUT varlist ';'
            { trace_varlist[trace_varindex] = NULL; trace_reallocate_varlist(); ite_flag_vars(trace_varlist, /*dependent - 0*/0);  }
	      | OUTPUT ';'
;

structure: STRUCTURE lines
;

lines:  /* empty */
	      | lines line ';'
            { if (((++lines) % 100) == 0) d2_printf2("\r%d", lines); }
;

line:     ID '=' OP '(' exp_list ')'
	         { trace_reallocate_exp(); explist[explevel][expindex[explevel]] = NULL; ite_op_equ($1, $3, explist[explevel]); explevel--; }
        | ID '=' exp_start
	         { ite_op_id_equ($1,$3); }
	     | ID '=' OP_NEW_INT_LEAF '(' ID ')'
            { ite_new_int_leaf($1, $5); }
	     | ARE_EQUAL '(' exp_list ')'
            { trace_reallocate_exp(); explist[explevel][expindex[explevel]] = NULL; ite_op_are_equal(explist[explevel]); explevel--; }
	     | C_OP '(' INTNUMBER ')'
            { fprintf(stderr, "nonhandled are_c_op\n"); assert(0); }
	     | TPRINT '(' STRING ')'
	         { printf("%s\n", $3); }
;

varlist:   ID
	         { trace_varindex=0; trace_reallocate_varlist(); trace_varlist[trace_varindex++] = s_getsym($1, SYM_VAR); assert(trace_varlist[trace_varindex-1]); }
	      | varlist ',' ID
	         { trace_reallocate_varlist(); trace_varlist[trace_varindex++] = s_getsym($3, SYM_VAR);  assert(trace_varlist[trace_varindex-1]);}
;

exp_start: exp
	         { $$ = $1; }
;

exp_list:  exp 
	          { explevel++; trace_reallocate_exp(); expindex[explevel]=0; explist[explevel][expindex[explevel]++] = $1; }
         | exp_list ',' exp
	          { trace_reallocate_exp(); explist[explevel][expindex[explevel]++] = $3; }
;

exp:      ID
	         { symrec *s=s_getsym($1, SYM_VAR); assert(s); $$ = ite_vars(s); }
	     | OP_ITE '(' exp_start ',' exp_start ',' exp_start ')'
	         { $$ = ite_s( $3, $5, $7); }
	     | OP '(' exp_list ')'
	         { trace_reallocate_exp(); explist[explevel][expindex[explevel]] = NULL; $$ = ite_op_exp($1,explist[explevel]); explevel--; }
	     | U_OP_NOT '(' ID ')'
	         { symrec *s=s_getsym($3, SYM_VAR); assert(s); $$ = ite_not_s(ite_vars(s)); }
	     | U_OP '(' ID ')'
	         { fprintf(stderr, "nonhandled u_op\n"); $$ = NULL; }
;

%%

void
trace_reallocate_exp()
{
   if (explevel >= explevel_max)
   {
      explist = (BDDNode ***)ite_recalloc((void*)explist, explevel_max, explevel_max+10, sizeof(BDDNode **), 9, "explist");
      expindex = (int *)ite_recalloc((void*)expindex, explevel_max, explevel_max+10, sizeof(int), 9, "expindex");
      expmax = (int *)ite_recalloc((void*)expmax, explevel_max, explevel_max+10, sizeof(int), 9, "expmax");
      explevel_max += 10;
   }
   if (expindex[explevel] >= expmax[explevel])
   {
      explist[explevel] = (BDDNode **)ite_recalloc((void*)explist[explevel], expmax[explevel], expmax[explevel]+100, sizeof(BDDNode *), 9, "explist array");
      expmax[explevel] += 100;
   }
}

void
trace_reallocate_varlist()
{
   if (trace_varindex >= trace_varmax)
   {
      trace_varlist = (symrec **)ite_recalloc((void*)trace_varlist, trace_varmax, trace_varmax+100, sizeof(symrec *), 9, "trace_varlist");
      trace_varmax += 100;
   }
}

void
trace_free()
{
   ite_free((void**)&trace_varlist);
   trace_varmax = 0;

   for(int i=0;i<explevel_max;i++)
      ite_free((void**)&explist[i]);
   ite_free((void**)&explist);
   ite_free((void**)&expindex);
   ite_free((void**)&expmax);
   explevel_max = 0;
}

void
ite_op_id_equ(char *var, BDDNode *bdd)
{
   symrec  *s_ptr = s_getsym(var, SYM_VAR);
   assert(s_ptr);
   s_set_indep(s_ptr, 0);
   //independantVars[s_ptr->id] = 0;
   BDDNode *ptr = ite_equ(ite_vars(s_ptr), bdd);
   functions_add(ptr, UNSURE, s_ptr->id);
}

int ite_last_fn=0;

void
ite_op_equ(char *var, t_op2fn fn, BDDNode **_explist)
{
   fn.fn_type = MAKE_EQU(fn.fn_type);
   symrec  *s_ptr = s_getsym(var, SYM_VAR);
   assert(s_ptr);
   s_set_indep(s_ptr, 0);
   //independantVars[s_ptr->id] = 0;
   BDDNode *ptr = ite_op_exp(fn, _explist);
   ptr = ite_equ(ite_vars(s_ptr), ptr);
   functions_add(ptr, ite_last_fn, s_ptr->id);
}

int ite_op_flag=0;

/* recursive counter of variables and union of variables */
void
ite_op_check(int *new_vars, int *total_vars, BDDNode *bdd)
{
   if (bdd == true_ptr || bdd == false_ptr) return;
   if (((symrec*)(bdd->var_ptr))->flag!=ite_op_flag) {
      ((symrec*)(bdd->var_ptr))->flag=ite_op_flag;
      (*new_vars)++;
   }
   (*total_vars)++;
   ite_op_check(new_vars, total_vars, bdd->thenCase);
   ite_op_check(new_vars, total_vars, bdd->elseCase);
}

int
explist_sort(const void *x, const void *y)
{
   int x_var = (*(*(BDDNode ***)&x))->variable;
   int y_var = (*(*(BDDNode ***)&y))->variable;
   //return (x_var - y_var);
   return (y_var - x_var);
}

BDDNode *ite_op_exp(t_op2fn fn, BDDNode **_explist)
{
   int i=0, num_members=0;
   BDDNode *ptr=NULL;
   assert(_explist[0] != NULL);
   /* first check the bdds:
    1. if they all have just one var => special fn
    2. if they combined have more than X distict var => split it
    */
   ite_op_flag++; 
   if (ite_op_flag > (1<<30)) { sym_clear_all_flag(); ite_op_flag = 1; }
   int spec_fn=0;
   int total_vars = 0;
   int new_vars = 0;

   for (num_members=0;_explist[num_members]!=NULL;num_members++) {
      ite_op_check(&new_vars, &total_vars, _explist[num_members]);
      if (total_vars <= 1) total_vars = 0;
   }

   /* check if each bdd in the function have just one variable */
   if (total_vars == 0 && num_members>=2 /*limit[fn.fn_type]*/) {
      /* we have a special function */ 
      spec_fn=1;
   } else {
      /* this does not happen in the current setup */
      /* if you decide to keep some special functions in a bigger BDD */
      /* you can use this to break bigger BDDs appart */
      /* check if they have more than a limit to get broken up */
      if (new_vars >= 5 /*s_limit*/ || num_members>=2 /*limit[fn.fn_type]*/) {
         printf("x");
         /* we have a special function 
          but we have to break it apart
          */
         spec_fn=1;
         for (i=0;_explist[i]!=NULL;i++) {
            /* take _explist[i] and ite_equ to a new temp var */
            ite_op_flag++; /* make sure we don't overflow... */
            int _new_vars=0;
            int _total_vars=0;
            ite_op_check(&_new_vars, &_total_vars, _explist[i]);
            if (total_vars > 1) {
               symrec *s_ptr = tputsym(SYM_VAR);
               /* save _explist[i] into another function */
               BDDNode *new_ptr = ite_equ(ite_vars(s_ptr), _explist[i]);
               functions_add(new_ptr, UNSURE, s_ptr->id);

               /* replace _explist[i] with ite_var of the temp var */
               _explist[i] = ite_vars(s_ptr);
            }
         }
      }
   }

   if (fn.as_type == AS_FULL) 
      qsort (_explist, num_members, sizeof (BDDNode*), explist_sort);

   /* apply the function */
   if (fn.as_type != AS_LEFT || fn.as_type == AS_RIGHT)
   {
      for (i=0;_explist[i]!=NULL;i++) {
         assert(_explist[i]->var_ptr);
         if (ptr==NULL) ptr = _explist[i];
         else ptr = fn.fn(ptr, _explist[i]);
      }
   } else {
      for(i=0;_explist[i]!=NULL;i++) { }
      while(i>0) {
         i--;
         assert(_explist[i]->var_ptr);
         if (ptr==NULL) ptr = _explist[i];
         else ptr = fn.fn(_explist[i], ptr);
      }
   }
   if (fn.neg_all) ptr = ite_not_s(ptr);
   assert(ptr != NULL);
   assert(ptr->var_ptr);
   /* if it is a special function separate it */
   /* into a different function               */
   /* _EQU functions are handled using the grammar */
   if (spec_fn && !IS_EQU(fn.fn_type)) {
      symrec *s_ptr = tputsym(SYM_VAR);
      ptr = ite_equ(ite_vars(s_ptr),ptr);
      functions_add(ptr, MAKE_EQU(fn.fn_type), s_ptr->id);

      ptr = ite_vars(s_ptr);
   };
   ite_last_fn = fn.fn_type;
   return ptr;
}

void
ite_op_are_equal(BDDNode **_explist)
{
   t_op2fn fn;

   /* LOOK: make sure ite_op_exp does not change the _explist much */
   /* ite_and positive */
   fn.fn=op_equ;
   fn.fn_type=EQU_EQU;
   fn.as_type=AS_FULL;
   fn.neg_all=0;
   BDDNode *ptr = ite_op_exp(fn, _explist);

   functions_add(ptr, EQU, 0); /* the only plain or? */
}

void
ite_new_int_leaf(char *id, char *zero_one)
{
   symrec *s_ptr = s_getsym(id, SYM_VAR);
   assert(s_ptr);
   s_set_indep(s_ptr, 0);
   //independantVars[s_ptr->id] = 0;
   BDDNode *ptr = ite_vars(s_ptr);
   if (*zero_one == '0') ptr = ite_not_s(ptr);
   else if (*zero_one != '1') {
      fprintf(stderr, "new_int_leaf with non 0/1 argument (%s) -- assuming 1\n", zero_one);
   }
   functions_add(ptr, UNSURE, s_ptr->id);
}

void
ite_flag_vars(symrec **varlist, int indep)
{
  for (int i=0; varlist[i]!=NULL; i++) {
     s_set_indep(varlist[i], indep);
     //independantVars[varlist[i]->id] = indep;
  }
}

void
printAllFunctions()
{
   for(int i=0;i<nmbrFunctions;i++) 
   { 
      printf("eq: %d\n", equalityVble[i]); 
      printBDD(functions[i]); 
      printf("\n"); 
   } 
}
