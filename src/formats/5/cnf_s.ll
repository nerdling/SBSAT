%{
#include "ite.h"
#include "bddnode.h"
#include "libt5_la-cnf_g.h"
extern int s_line;
/* remove warning about unput not used */
#define YY_NO_UNPUT
/* remove warning statement has no effect */
#define ECHO
%}
%option noyywrap
%option debug
/*%option outfile="cnf_s.c"*/
%option outfile="lex.yy.c"
%option prefix="cnf_"


INTNUMBER    [+-]?[0-9]+

%%

"p cnf"		return P_CNF;
{INTNUMBER}     { cnf_lval.num=atoi(yytext); if (cnf_lval.num==0) return ZERO; else return NON_ZERO; }


"%"[^\n]*         /* eat up one-line comments */
"#"[^\n]*         /* eat up one-line comments */
"c"[^\n]*         /* eat up one-line comments */
";"[^\n]*         /* eat up one-line comments */

[ \t\r]+            /* eat up whitespace */
"\n"	          /* eat up new-lines */ s_line++;

.                 printf( "Unrecognized character: %s\n", yytext ); 

%%

