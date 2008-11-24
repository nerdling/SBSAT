%{
#include "sbsat.h"
#include "bddnode.h"
#include "libt5_a-iscas_g.h"

/* remove warning about unput not used */
#define YY_NO_UNPUT
/* remove warning statement has no effect */
#define ECHO

#define yyterminate() { iscas__delete_buffer(YY_CURRENT_BUFFER); return YY_NULL; }

extern int s_line;
void iscas_ll_nothing() { }
%}
%s names ttable
%option noyywrap
%option debug
/*%option outfile="blif_s.c" */
%option outfile="lex.yy.c"
%option prefix="iscas_" 

ID           [-a-zA-Z0-9_.\[\]]+

%%

INPUT   { return P_INPUT; }
OUTPUT  { return P_OUTPUT; }
=  { return '='; }
,  { return ','; }
OR   { return P_OR; } /* or */
AND    { return P_AND; } /* and */
\(    { return '('; }
\)    { return ')'; }
NOT    { return P_NOT; }

{ID}          { strcpy(iscas_lval.id,yytext); return ID; }

"#"[^\n]*         /* eat up one-line comments */

[ \t\r]+            /* eat up whitespace */
"\\""\n"	    /* eat up new-lines */ { s_line++; if (s_line%100==0) d2_printf3("\rLine: %d, Functions: %d", s_line, nmbrFunctions); }
"\n"	          /* eat up new-lines */ { s_line++;  if (s_line%100==0) d2_printf3("\rLine: %d, Functions: %d", s_line, nmbrFunctions); }

.                 printf( "Unrecognized character: %s\n", yytext ); 

%%
