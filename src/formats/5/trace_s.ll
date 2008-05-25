%{
#include "sbsat.h"
#include "bddnode.h"
#include "libt5_a-trace_g.h"
extern int s_line;
/* remove warning about unput not used */
#define YY_NO_UNPUT
/* remove warning statement has no effect */
#define ECHO
#define yyterminate() { trace__delete_buffer(YY_CURRENT_BUFFER); return YY_NULL; }

#define FILL_OP(xfn, xfntype, xfnas, xfnneg) { trace_lval.op2fn.fn=xfn; trace_lval.op2fn.fn_type=xfntype; trace_lval.op2fn.as_type=xfnas; trace_lval.op2fn.neg_all=xfnneg; }
%}

%option noyywrap
%option debug
/*%option outfile="trace_s.c" */
%option outfile="lex.yy.c"
%option prefix="trace_" 

INTNUMBER    [+-][0-9]+
/*INTNUMBER    [+-]?[0-9]+*/
ID           [-a-zA-Z0-9_.\[\]]+
/* ID           [a-zA-Z_][\[\].-a-zA-Z0-9_]* */

/* fix me -- nand is not nand(a, nand(b, nand(...))) but not(and(a,and(b,and(...))))*/
/* move this to the next section
nand		{ FILL_OP(op_nand, NAND);  return OP; }
nor		{ FILL_OP(op_nor, NOR);    return OP; }
nimp		{ FILL_OP(op_nimp, LIMP);  return OP; }
*/
%%

MODULE 		return MODULE;
ENDMODULE 	return ENDMODULE;
INPUT 		return INPUT;
OUTPUT 		return OUTPUT;
STRUCTURE 	return STRUCTURE;
ite		return OP_ITE;
new_int_leaf    return OP_NEW_INT_LEAF;
are_equal	return ARE_EQUAL;

trace_verbose_print              return TPRINT;
check_point_for_force_reordering return C_OP;

and		{ FILL_OP(op_and, AND, AS_FULL, 0);    return OP; }
or		{ FILL_OP(op_or, OR, AS_FULL, 0);      return OP; }
xor		{ FILL_OP(op_xor, XOR, AS_FULL, 0);    return OP; }
limp		{ FILL_OP(op_imp, LIMP, AS_LEFT, 0);  return OP; }
rimp		{ FILL_OP(op_imp, RIMP, AS_RIGHT, 0); return OP; }
imp		{ FILL_OP(op_imp, LIMP, AS_LEFT, 0);   return OP; }
equ		{ FILL_OP(op_equ, EQU, AS_RIGHT, 0);    return OP; }

lnimp		{ FILL_OP(op_imp, LNIMP, AS_LEFT, 1);  return OP; }
rnimp		{ FILL_OP(op_imp, RNIMP, AS_RIGHT, 1);  return OP; }
nand		{ FILL_OP(op_and, NAND, AS_LEFT, 1);  return OP; }
nor		{ FILL_OP(op_or, NOR, AS_LEFT, 1);  return OP; }

exists	        { FILL_OP(NULL, 0, 0, 0);        return OP; }
rel_prod	{ FILL_OP(NULL, 0, 0, 0);        return OP; }
restrict	{ FILL_OP(NULL, 0, 0, 0);        return OP; }
not             {                          return U_OP_NOT; }

vars_next_to_curr|vars_curr_to_next|support_vars  { strcpy(trace_lval.id, yytext); return U_OP; }
	   
{INTNUMBER} { trace_lval.num=atoi(yytext);  return INTNUMBER; }
{ID}        { strcpy(trace_lval.id,yytext); return ID; }

";"|"("|")"|","|"="   return *yytext;
"\""[^"\n]*"\""	      { strcpy(trace_lval.id,yytext); return STRING; }

"%"[^\n]*         /* eat up one-line comments */
"#"[^\n]*         /* eat up one-line comments */

[ \t\r]+            /* eat up whitespace */
"\n"	          /* eat up new-lines */ s_line++;

.                 printf( "Unrecognized character: %s\n", yytext ); 

%%

