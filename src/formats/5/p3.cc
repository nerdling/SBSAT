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
#include "symtable.h"
#include "functions.h"
#include "p3.h"

int p3_max = 0;
t_ctl *p3 = NULL;
int p3_alt_flag = 1;

void p3_clear_sharing(int idx, int flag);

void
p3_done()
{
   p3[0].top_and = 1;
   p3_clear_sharing(0, p3_alt_flag);
   p3_alt_flag = 1-p3_alt_flag; 
   p3_traverse_ref(0, -1);
   //p3_dump();
   p3_top_bdds(0);
}


void
p3_add_ref(int idx, int ref)
{
   if (p3[idx].refs_idx+1 >= p3[idx].refs_max) {
      p3[idx].refs_max += 10;
      p3[idx].refs = (int*)realloc(p3[idx].refs, p3[idx].refs_max*sizeof(int));
   }
   p3[idx].refs[p3[idx].refs_idx++] = ref;
}

void
p3_traverse_ref(int i, int ref)
{
   int idx = -1*i;
   assert(idx >= 0);
   p3[idx].ref++;
   if (ref >= 0) p3_add_ref(idx, ref);
   if (p3[idx].ref > 1) return;
   if (p3[idx].top_and && p3[idx].tag == AND_Tag) {
      if (p3[idx].arg1 < 0 && p3[-1*p3[idx].arg1].tag == AND_Tag) 
         p3[-1*p3[idx].arg1].top_and = 1;
      if (p3[idx].argc == 2 && p3[idx].arg2 < 0 && p3[-1*p3[idx].arg2].tag == AND_Tag) 
         p3[-1*p3[idx].arg2].top_and = 1;
   }
   if (p3[idx].arg1 < 0) p3_traverse_ref(p3[idx].arg1, idx);
   if (p3[idx].argc == 2 && p3[idx].arg2 < 0) p3_traverse_ref(p3[idx].arg2, idx);
}



void
p3_alloc(int max)
{
   p3 = (t_ctl*)realloc((void*)p3, max*sizeof(t_ctl));
   memset((char*)p3+p3_max*sizeof(t_ctl), 0, (max-p3_max)*sizeof(t_ctl));
   p3_max = max;
}

void
prover3_free()
{
   for (int i=0;i<p3_max;i++) {
      ite_free((void**)&p3[i].refs);
   }
   ite_free((void**)&p3); p3_max = 0;
}
      
void
p3_add(char *dst, int argc, int tag, char *arg1, char *arg2)
{
   int i_dst = s2id(dst);
   int i_arg1 = 0;
   int i_arg2 = 0;

   if (arg1[0] == 'S') i_arg1 = -1*atoi(arg1+1);
   else i_arg1 = i_getsym(arg1, SYM_VAR); 
   if (argc == 2) {
      if (arg2[0] == 'S') i_arg2 = -1*atoi(arg2+1);
      else i_arg2 = i_getsym(arg2, SYM_VAR); 
   }
   if (p3_max <= i_dst) p3_alloc(i_dst+10);

   p3[i_dst].tag = tag;
   p3[i_dst].argc = argc;
   p3[i_dst].arg1 = i_arg1;
   p3[i_dst].arg2 = i_arg2;
/*
   if (argc == 1) 
      fprintf(stderr, "%s = not %s \n", dst, arg1);
   else
      fprintf(stderr, "%s = %s %s \n", dst, arg1, arg2);
*/
}

int
s2id(char *s_id)
{
   int i;
   if(s_id[0] != 'S') { fprintf(stderr, "temp var without 'S'\n"); exit(1); };
   i = atoi(s_id+1);
   return i;
}
/*
void
p3_dump()
{
   int i;
   for(i=0;i<p3_max;i++) {
      switch (p3[i].argc) {
       case 2: fprintf(stderr, "%cS%d = %d %d (%d)", 
                     p3[i].top_and?'*':' ', i, p3[i].arg1, p3[i].arg2, p3[i].ref); break;
       case 1: fprintf(stderr, "%cS%d = %d (%d)", 
                     p3[i].top_and?'*':' ', i, p3[i].arg1, p3[i].ref); break;
       default: break;
      }
      fprintf(stderr, " %x %d %d %d %x %d %d\n",
            p3[i].bdd==NULL?0:p3[i].bdd, p3[i].vars, p3[i].top_and, p3[i].ref, p3[i].refs==NULL?0:p3[i].refs,
            p3[i].refs_idx, p3[i].refs_max);
   }
}
*/
BDDNode *
p3_bdds(int idx, int *vars)
{
   BDDNode *bdd = NULL;
   if (idx < 0) {
      BDDNode *bdd1 = NULL;
      BDDNode *bdd2 = NULL;
      int vars1 = 0;
      int vars2 = 0;
      idx = -1*idx;
      if (p3[idx].bdd != NULL) {
         *vars = p3[idx].vars;
         return p3[idx].bdd;
      }
      bdd1 = p3_bdds(p3[idx].arg1, &vars1);
      if (vars1 > prover3_max_vars) {
         bdd1 = tmp_equ_var(bdd1);
         vars1 = 1;
      }
      if (p3[idx].argc == 2) {
         bdd2 = p3_bdds(p3[idx].arg2, &vars2);
         if (vars2 > prover3_max_vars) {
            bdd2 = tmp_equ_var(bdd2);
            vars2 = 1;
         }
      }
      switch(p3[idx].tag) {
       case OR_Tag: bdd = ite_or(bdd1, bdd2); break;
       case AND_Tag: bdd = ite_and(bdd1, bdd2); break;
       case IMP_Tag: bdd = ite_imp(bdd1, bdd2); break;
       case EQUIV_Tag: bdd = ite_equ(bdd1, bdd2); break;
       case NOT_Tag: bdd = ite_not(bdd1); break;
       default: assert(0); exit(1); break;
      }
      
      if (vars1+vars2 > prover3_max_vars) {
         bdd = tmp_equ_var(bdd);
      }
      
      *vars = vars1 + vars2;
      p3[idx].bdd = bdd;
      p3[idx].vars = *vars;
   } else {
      /* variable */
      bdd = ite_var(idx);
      *vars = 1;
   }
   return bdd;
}

void
p3_clear_sharing(int idx, int flag)
{
   if (idx <= 0) {
      idx = -1*idx;
      if (p3[idx].flag == flag) return;
      p3[idx].flag = flag;
      p3[idx].bdd = NULL;
      p3[idx].vars = 0;
      //p3[idx].top_and = 0;
      p3_clear_sharing(p3[idx].arg1, flag);
      if (p3[idx].argc == 2) {
         p3_clear_sharing(p3[idx].arg2, flag);
      }
   } else {
      /* variable */
   }
}

void
p3_clear_flag(int idx)
{
   if (idx <= 0) {
      idx = -1*idx;
      if (p3[idx].flag == 0) return;
      p3[idx].flag = 0;
      p3_clear_flag(p3[idx].arg1);
      if (p3[idx].argc == 2) {
         p3_clear_flag(p3[idx].arg2);
      }
   } else {
      /* variable */
   }
}

void
p3_top_bdds(int idx)
{
   int vars;
   if (idx > 0 || p3[-1*idx].top_and == 0) {
      functions_add(p3_bdds(idx, &vars), UNSURE, 0);
   } else {
      idx = -1*idx;
      d3_printf4("%d vars=%d functions=%d\r", idx, vars_max, functions_max);
      p3_top_bdds(p3[idx].arg1);
      if (p3[idx].argc==2) p3_top_bdds(p3[idx].arg2);
   }
}

