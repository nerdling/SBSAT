%{
#include "ite.h"
#include "bddnode.h"
#include "libt5_a-blif_g.h"
extern int s_line;
%}
%s names ttable
%option noyywrap
%option debug
/*%option outfile="blif_s.c" */
%option outfile="lex.yy.c"
%option prefix="blif_" 

INTNUMBER    [+-][0-9]+
/*INTNUMBER    [+-]?[0-9]+*/
ID           [-a-zA-Z0-9_.\[\]]+
/* ID           [a-zA-Z_][\[\].-a-zA-Z0-9_]* */
TRUTHTABLEONE  [-01]+
TRUTHTABLE  [-01]+" "[01]?

%%

.model   { BEGIN(INITIAL); return MODEL; }
.inputs  { BEGIN(INITIAL); return INPUTS; }
.outputs { BEGIN(INITIAL); return OUTPUTS; }
.latch   { BEGIN(INITIAL); return LATCH; }
.names   { BEGIN(names); return NAMES; }
.end     { BEGIN(INITIAL); return END; }

<ttable>{TRUTHTABLE}     { strcpy(blif_lval.id,yytext); return TT; }
<ttable>{TRUTHTABLEONE}  { strcpy(blif_lval.id,yytext); return TTONE; }
<INITIAL>[01][ ]*"\n"               return ZEROONE;
{ID}          { strcpy(blif_lval.id,yytext); return ID; }

"%"[^\n]*         /* eat up one-line comments */
"#"[^\n]*         /* eat up one-line comments */

[ \t\r]+            /* eat up whitespace */
"\\""\n"	    /* eat up new-lines */ s_line++;
<names>"\n"       { BEGIN(ttable); }
"\n"	          /* eat up new-lines */ s_line++;

.                 printf( "Unrecognized character: %s\n", yytext ); 

%%

