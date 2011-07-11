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
 any trademark,  service mark, or the name of University of Cincinnati.


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

#include "ite.h"
#include "solver.h"

int solver_vars=0;
int *arrSolver2IteVarMap=NULL;
int *arrIte2SolverVarMap=NULL;
int nIndepVars=0;
int nDepVars=0;
t_var_stat *var_stat=NULL;
ITE_INLINE int RecordInitialInferences();


ITE_INLINE int InitSolver();
ITE_INLINE void FreeSolver();
void LoadLemmas(char *filename);
ITE_INLINE void FreeAFS();
ITE_INLINE int InitBrancherX();
ITE_INLINE int kSolver(int k_top_vars);

int
solve_init()
{
  int ret = SOLV_UNKNOWN;

      /*
       * reversing dependency if requested
       */
      if (reverse_independant_dependant)
      {
         for (int x = 0; x < numinp + 1; x++)
         {
            if (independantVars[x] == 1)
               independantVars[x] = 0;
            else if (independantVars[x] == 0)
               independantVars[x] = 1;
         }
      }

      if (clear_dependance)
      {
         for (int x = 0; x < numinp + 1; x++)
            if (independantVars[x] == 0)
               independantVars[x] = 1;
      }

  ret = InitSolver(); // breakxors, variables, lemma space init, lemma heu init
  if (ret != SOLV_UNKNOWN) return ret;
 
  /* Convert bdds and special functions to smurfs */
  ret = SmurfFactory();
  if (ret != SOLV_UNKNOWN) return ret;

  bdd_gc(1);

  ret = InitBrancher();
  if (ret != SOLV_UNKNOWN) return ret;

  ret = InitBrancherX();
  if (ret != SOLV_UNKNOWN) return ret;

  if (K_TOP_VARIABLES) ret = kSolver(K_TOP_VARIABLES);
  if (ret != SOLV_UNKNOWN) return ret;
  return ret;
}

void
solve_free()
{
   d4_printf5("SMURF States Statistic(total %ld): %ld/%ld (%f hit rate)\n",
         (long)(ite_counters[SMURF_NODE_NEW]),
         (long)(ite_counters[SMURF_NODE_FIND] - ite_counters[SMURF_NODE_NEW]),
         (long)(ite_counters[SMURF_NODE_FIND]),
         ite_counters[SMURF_NODE_FIND]==0?0:1.0 * (ite_counters[SMURF_NODE_FIND] - ite_counters[SMURF_NODE_NEW]) / ite_counters[SMURF_NODE_FIND]);
   FreeBrancher();
   FreeSmurfFactory();
   FreeSolver();
}

int
solve()
{
   int ret = solve_init();
   if (ret == SOLV_UNKNOWN) ret = Brancher();
   solve_free();
   return ret;
}

int
_solve()
{
  int ret = SOLV_UNKNOWN;

  ret = InitSolver(); // breakxors, variables, lemma space init, lemma heu init

  if (ret == SOLV_UNKNOWN) {
 
    /* Convert bdds and special functions to smurfs */
    ret = SmurfFactory();

    if (ret == SOLV_UNKNOWN) {

      ret = InitBrancher();

      if (ret == SOLV_UNKNOWN) {

         ret = Brancher();

      };

      /* restart?? */
      //if (ret == SOLV_UNKNOWN || ret == SOLV_UNSAT) ret = Brancher();

      FreeBrancher();
    }
    FreeSmurfFactory();
  }

  FreeSolver();

  return ret;
}

int total_vars=0;

ITE_INLINE int
InitSolver()
{
  d9_printf1("InitSolver\n");

  if (BREAK_XORS) {
     total_vars = splitXors();
     d9_printf3("Split(%ld) returned %d\n", numinp, total_vars);
  } else {
     total_vars = numinp;
  }

  nNumVariables = 1; /* 0 => true, false */

  for (int x = 0; x <= numinp; x++) {
     if (variablelist[x].true_false == -1) {
        nNumVariables++;
        if (independantVars[x] == 1) nIndepVars++;
        else if (independantVars[x] == 0) nDepVars++;
     }
  }
  /* temporary variables (xor) */
  nNumVariables += (total_vars - numinp);

  d3_printf4("Solver vars: %d/%d (not used: %d)\n", nNumVariables-1, total_vars, total_vars-nNumVariables+1);
  d3_printf4("Indep/Dep vars: %d/%d (other vars: %d)\n", nIndepVars, nDepVars, nNumVariables-1-nIndepVars-nDepVars);

  /* ??!!?? */
  if (nNumVariables == 1) return SOLV_SAT;

  /* init mapping arrays */
  arrSolver2IteVarMap = (int *)ite_calloc(nNumVariables, sizeof(int), 2, "solver mapping(s2i)");
  arrIte2SolverVarMap = (int *)ite_calloc(total_vars+1, sizeof(int), 2, "solver mapping(i2s)");

  /* init solution array */
  arrSolution = (char *)ite_calloc(nNumVariables, sizeof(char), 2, "solution vector");

  int nSolutionIndep = 1;
  int nSolutionDep = nIndepVars+1;
  int nSolutionOther = nIndepVars+nDepVars+1;
  int index;

  for (int x=0;x<total_vars+1;x++) {
     index = -1;
     if (x > numinp) index = nSolutionOther++;
     else
        if (variablelist[x].true_false == -1) {
           if (independantVars[x] == 1) index = nSolutionIndep++;
           else if (independantVars[x] == 0) index = nSolutionDep++;
           else index = nSolutionOther++;
        }

     if (index != -1) {
        //arrSolution[index] = BOOL_UNKNOWN;
        arrSolver2IteVarMap[index] = x;
        arrIte2SolverVarMap[x] = index;
        if (x <= numinp && var_score) 
           var_score[index] = var_score[x];
     }
  }
  assert (nmbrFunctions > 0);
  assert (nSolutionIndep == nIndepVars+1);
  assert (nSolutionDep-nIndepVars-1 == nDepVars);

  gnMaxVbleIndex = nNumVariables; 

  var_stat = (t_var_stat *)ite_calloc(nNumVariables, sizeof(t_var_stat), 9, "var_stat");

  d3_printf1 ("Initializing Smurf Factory data structs ...\n");
  InitLemmaSpacePool(0);

  if (nHeuristic == C_LEMMA_HEURISTIC
#ifdef JOHNSON_HEURISTIC_LEMMA
        || nHeuristic == JOHNSON_HEURISTIC
#endif
        ) 
	       InitLemmaHeurArrays(gnMaxVbleIndex);

   if (nHeuristic == JOHNSON_HEURISTIC) {
      InitHeurScoresStack();
   }

   gnNumLemmas = 0;
   InitLemmaInfoArray();
   if (*lemma_in_file) LoadLemmas(lemma_in_file);

  return SOLV_UNKNOWN;
}

ITE_INLINE void
FreeSolver()
{
  d5_printf1("FreeSolver\n");

  if (nHeuristic == C_LEMMA_HEURISTIC 
#ifdef JOHNSON_HEURISTIC_LEMMA
        || nHeuristic == JOHNSON_HEURISTIC
#endif
        ) 
     DeleteLemmaHeurArrays();

  if (nHeuristic == JOHNSON_HEURISTIC) {
     FreeHeurScoresStack();
  }

  //ite_free((void**)&arrSolution);
  ite_free((void**)&arrSolution);
  ite_free((void**)&arrSolver2IteVarMap);
  ite_free((void**)&arrIte2SolverVarMap);
  ite_free((void**)&var_stat);

  FreeLemmaInfoArray();
  FreeLemmaSpacePool();
}
