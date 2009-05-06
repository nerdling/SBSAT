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

UINT    [0-9]+
VAR [^ *+,\n]+
MULT   "*"
PLUS	"+"
NEWLINE   "\n"
CONSTANT     "1"

%%


"nle"			return P_nle;

[ \t\r]+		/* eat up whitespace */

{CONSTANT}		{return CONSTANT;}
{UINT}			{nle_lval.num=atoi(yytext); return UINT;}
{VAR}		  	{strncpy(nle_lval.id,yytext,200); return VAR;}
{NEWLINE}		{return NEWLINE;}
{MULT}                  {return MULT;}
{PLUS}			{return PLUS;}

%%

