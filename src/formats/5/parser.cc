#include "ite.h"
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

