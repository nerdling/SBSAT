%{
#include "sbsat.h"
#include "bddnode.h"
#include "libt5_a-prover_g.h"

/* remove warning about unput not used */
#define YY_NO_UNPUT
/* remove warning statement has no effect */
#define ECHO

#define yyterminate() { prover__delete_buffer(YY_CURRENT_BUFFER); return YY_NULL; }

extern int s_line;
void prover_ll_nothing() { }
%}
%s names ttable
%option noyywrap
%option debug
/*%option outfile="blif_s.c" */
%option outfile="lex.yy.c"
%option prefix="prover_" 

ID           [-a-zA-Z0-9_.\[\]]+

%%

\<-\>  { return P_EQUIV; }
-\>   { return P_IMP; }
#    { return '#'; } /* or */
&    { return '&'; } /* and */
\(    { return '('; }
\)    { return ')'; }
~    { return '~'; }

{ID}          { strcpy(prover_lval.id,yytext); return ID; }

"%"[^\n]*         /* eat up one-line comments */

[ \t\r]+            /* eat up whitespace */
"\\""\n"	    /* eat up new-lines */ { s_line++; if (s_line%100==0) d2_printf3("\rLine: %d, Functions: %d", s_line, nmbrFunctions); }
"\n"	          /* eat up new-lines */ { s_line++; if (s_line%100==0) d2_printf3("\rLine: %d, Functions: %d", s_line, nmbrFunctions); }

.                 printf( "Unrecognized character: %s\n", yytext ); 

%%
