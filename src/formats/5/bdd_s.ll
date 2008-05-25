%{
#include "sbsat.h"
#include "bddnode.h"
#include "libt5_a-bdd_g.h"
extern int s_line;

/* remove warning about unput not used */
#define YY_NO_UNPUT
/* remove warning statement has no effect */
#define ECHO

#define yyterminate() { bdd__delete_buffer(YY_CURRENT_BUFFER); return YY_NULL; }

%}

%option noyywrap
%option debug
/*%option outfile="bdd_s.c"*/
%option outfile="lex.yy.c"
%option prefix="bdd_"

INTNUMBER    [+-]?[0-9]+
ID           [a-zA-Z_][-a-zA-Z0-9_]*
BDDID        $[0-9]+

%%

"p bdd"		return P_BDD;
add_state       return ADD_STATE;
ite		return OP_ITE;

equ  { strcpy(bdd_lval.id,yytext); return OP; }
and  { strcpy(bdd_lval.id,yytext); return OP; }
nand { strcpy(bdd_lval.id,yytext); return OP; }
or   { strcpy(bdd_lval.id,yytext); return OP; }
nor  { strcpy(bdd_lval.id,yytext); return OP; }
xor3 { strcpy(bdd_lval.id,yytext); return OP; }

not	 	return U_OP;
	   
{INTNUMBER} { bdd_lval.num=atoi(yytext);  return INTNUMBER; }
{ID}        { strcpy(bdd_lval.id,yytext); return ID; }
{BDDID}     { bdd_lval.num=atoi(yytext+1); return BDDID; }

"*"|"("|")"|","|"="   return *yytext;
"\""[^"\n]*"\""	      { strcpy(bdd_lval.id,yytext); return STRING; }

"%"[^\n]*         /* eat up one-line comments */
"#"[^\n]*         /* eat up one-line comments */
";"[^\n]*         /* eat up one-line comments */

[ \t\r]+            /* eat up whitespace */
"\n"	          /* eat up new-lines */ s_line++;

.                 printf( "Unrecognized character: %s\n", yytext ); 

%%

