%{
#include "sbsat.h"
#include "bddnode.h"
#include "libt5_a-aig_g.h"
extern int s_line;
/* remove warning about unput not used */
#define YY_NO_UNPUT
/* remove warning statement has no effect */
#define ECHO

#define yyterminate() { aig__delete_buffer(yy_current_buffer); return YY_NULL; }

%}
%option noyywrap
%option debug
/*%option outfile="aig_s.c"*/
%option outfile="lex.yy.c"
%option prefix="aig_"


UNS_INT    [0-9]+
COMMENT_HEADER "c"
STRING [^\n]
IO_IDENTIFIER [ilo]UNS_INT

%%

"p aig"		return P_AIG;
{UNS_INT}	{return atoi(yytext);}


"%"[^\n]*         /* eat up one-line comments */
"#"[^\n]*         /* eat up one-line comments */
"c"[^\n]*         /* eat up one-line comments */
";"[^\n]*         /* eat up one-line comments */

[ \t\r]+            /* eat up whitespace */
"\n"	          /* eat up new-lines */ s_line++;

.                 printf( "Unrecognized character: %s\n", yytext ); 

%%

