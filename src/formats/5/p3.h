#ifndef P3_H
#define P3_H

#include "sbsat.h"
#include "bddnode.h"
#include "symtable.h"
#include "functions.h"

#define NOT_Tag 1
#define AND_Tag 2
#define OR_Tag 3
#define IMP_Tag 4
#define EQUIV_Tag 5

typedef
struct {
   int tag;
   int argc;
   int arg1;
   int arg2;
   /*------*/
   BDDNode *bdd;
   int vars;
   /*------*/
   int ref;
   int top_and;
   int *refs;
   int refs_idx;
   int refs_max;
   /*------*/
   int flag;
} t_ctl;

void p3_add(char *dst, int argc, int tag, char *arg1, char *arg2);
void p3_done();
void p3_dump();
void p3_traverse_ref(int i, int ref);
void p3_top_bdds(int idx);
int s2id(char *s_id);

extern int p3_max;
extern t_ctl *p3;

#endif
