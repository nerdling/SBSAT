%{
#include "sbsat.h"
#include "bddnode.h"
#include "libt5_a-aig_g.h"
#include <string.h>

extern int s_line;
/* remove warning about unput not used */
#define YY_NO_UNPUT
/* remove warning statement has no effect */
#define ECHO

#define yyterminate() { aig__delete_buffer(YY_CURRENT_BUFFER); return YY_NULL; }

%}
%option noyywrap
/*%option debug*/
/*%option outfile="aig_s.c"*/
%option outfile="lex.yy.c"
%option prefix="aig_"

UINT    [0-9]+
COMMENT_HEADER ^c
WORD [^ \n]+
STRING [^\n]
IO_IDENTIFIER (i|l|o)[0-9]+
NEW_LINE "\n"

%%


"aag"			return P_AIG;
{UINT}			{aig_lval.num=atoi(yytext); return UINT;}
{IO_IDENTIFIER}		{strncpy(aig_lval.id,yytext,200); return IO_IDENTIFIER;}

"%"[^\n]*		/* eat up one-line comments */
"#"[^\n]*		/* eat up one-line comments */
";"[^\n]*		/* eat up one-line comments */

[ \t\r]+		/* eat up whitespace */

{COMMENT_HEADER} 	{return COMMENT_HEADER;}

{WORD}		  	{strncpy(aig_lval.id,yytext,200); return WORD;}

{NEW_LINE}	        {s_line++; return NEW_LINE;}

.                	{d2_printf2("Unknown char: %s",yytext);}

%%

