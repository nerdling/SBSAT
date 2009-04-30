%{
#include "sbsat.h"
#include "bddnode.h"
#include "libt5_a-nle_g.h"
#include <string.h>

extern int s_line;
/* remove warning about unput not used */
#define YY_NO_UNPUT
/* remove warning statement has no effect */
#define ECHO

#define yyterminate() { nle__delete_buffer(YY_CURRENT_BUFFER); return YY_NULL; }

%}
%option noyywrap
/*%option debug*/
/*%option outfile="nle_s.c"*/
%option outfile="lex.yy.c"
%option prefix="nle_"

VAR [^ *+,\n]+
MULT   "*"
PLUS	"+"
COMMA	","[ \t\r\n]

%%


"nle"			return P_nle;

[ \t\r\n]+		/* eat up whitespace */

{VAR}		  	{printf("var "); strncpy(nle_lval.id,yytext,200); return VAR;}
{COMMA}			{printf("comma\n"); return COMMA;}
{MULT}                  {printf("mult "); return MULT;}
{PLUS}			{printf("plus "); return PLUS;}

%%

