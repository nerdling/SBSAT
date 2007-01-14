/* =========FOR INTERNAL USE ONLY. NO DISTRIBUTION PLEASE ========== */

/*********************************************************************
 Copyright 1999-2007, University of Cincinnati.  All rights reserved.
 By using this software the USER indicates that he or she has read,
 understood and will comply with the following:

 --- University of Cincinnati hereby grants USER nonexclusive permission
 to use, copy and/or modify this software for internal, noncommercial,
 research purposes only. Any distribution, including commercial sale
 or license, of this software, copies of the software, its associated
 documentation and/or modifications of either is strictly prohibited
 without the prior consent of University of Cincinnati.  Title to copyright
 to this software and its associated documentation shall at all times
 remain with University of Cincinnati.  Appropriate copyright notice shall
 be placed on all software copies, and a complete copy of this notice
 shall be included in all copies of the associated documentation.
 No right is  granted to use in advertising, publicity or otherwise
 any trademark, service mark, or the name of University of Cincinnati.


 --- This software and any associated documentation is provided "as is"

 UNIVERSITY OF CINCINNATI MAKES NO REPRESENTATIONS OR WARRANTIES, EXPRESS
 OR IMPLIED, INCLUDING THOSE OF MERCHANTABILITY OR FITNESS FOR A
 PARTICULAR PURPOSE, OR THAT  USE OF THE SOFTWARE, MODIFICATIONS, OR
 ASSOCIATED DOCUMENTATION WILL NOT INFRINGE ANY PATENTS, COPYRIGHTS,
 TRADEMARKS OR OTHER INTELLECTUAL PROPERTY RIGHTS OF A THIRD PARTY.

 University of Cincinnati shall not be liable under any circumstances for
 any direct, indirect, special, incidental, or consequential damages
 with respect to any claim by USER or any third party on account of
 or arising from the use, or inability to use, this software or its
 associated documentation, even if University of Cincinnati has been advised
 of the possibility of those damages.
*********************************************************************/

#include "sbsat.h"
#include "bddnode.h"

int  s_error=0;
int  s_line=1;

int bdd_debug;
int cnf_debug;
int trace_debug;
int blif_debug;
int prover_debug;
int prover3_debug;
int iscas_debug;

extern int bdd__flex_debug;
extern int cnf__flex_debug;
extern int trace__flex_debug;
extern int blif__flex_debug;
extern int prover__flex_debug;
extern int prover3__flex_debug;
extern int iscas__flex_debug;

extern FILE*bdd_in;
extern FILE*cnf_in;
extern FILE*trace_in;
extern FILE*prover_in;
extern FILE*prover3_in;
extern FILE*iscas_in;

typedef struct {
  char desc[128];
  char ext[128];
  char str_detect[128];
  int *yy_debug;
  int *flex_debug;
  FILE **filein;
} input_type;

input_type file_formats[] = {
  { "Mark bdd format",   "ite",  "^p bdd",  &bdd_debug,   &bdd__flex_debug,   &bdd_in },
  { "Dimacs cnf format", "cnf",   "^p cnf",   &cnf_debug,   &cnf__flex_debug,   &cnf_in },
  { "Trace format",      "trace", "^MODULE", &trace_debug, &trace__flex_debug, &trace_in },
  { "", "", "", NULL, NULL, NULL }
};

void
yy_error(const char *s)  /* Called by yyparse on error */
{
  //printf ("%s: %d: %s\n", filename, s_line, s);
  printf ("%d: %s\n", s_line, s);
  s_error=1;
}

void
trace_error(const char *s)  /* Called by yyparse on error */
{
  //printf ("%s: %d: %s\n", filename, s_line, s);
  printf ("%d: %s\n", s_line, s);
  s_error=1;
}

void
blif_error(const char *s)  /* Called by yyparse on error */
{
  //printf ("%s: %d: %s\n", filename, s_line, s);
  printf ("%d: %s\n", s_line, s);
  s_error=1;
}

void
prover_error(const char *s)  /* Called by yyparse on error */
{
  //printf ("%s: %d: %s\n", filename, s_line, s);
  printf ("%d: %s\n", s_line, s);
  s_error=1;
}

void
prover3_error(const char *s)  /* Called by yyparse on error */
{
  //printf ("%s: %d: %s\n", filename, s_line, s);
  printf ("%d: %s\n", s_line, s);
  s_error=1;
}

void
iscas_error(const char *s)  /* Called by yyparse on error */
{
  //printf ("%s: %d: %s\n", filename, s_line, s);
  printf ("%d: %s\n", s_line, s);
  s_error=1;
}

int parser_init()
{
  bdd_debug = 0;
  cnf_debug = 0;
  trace_debug = 0;
  blif_debug = 0;
  prover_debug = 0;
  prover3_debug = 0;
  iscas_debug = 0;

  bdd__flex_debug = 0;
  cnf__flex_debug = 0;
  trace__flex_debug = 0;
  blif__flex_debug = 0;
  prover__flex_debug = 0;
  prover3__flex_debug = 0;
  iscas__flex_debug = 0;

  return 0;
}

