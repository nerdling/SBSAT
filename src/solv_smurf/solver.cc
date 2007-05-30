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

#include "sbsat.h"
#include "sbsat_solver.h"
#include "solver.h"

SolverFunction * arrSolverFunctions;
int nNumFuncs;

FnInit   procFnInit[] = FN_INIT_LIST ;

int nNumFns=0;

int *arrLemmaVbleCountsPos;
int *arrLemmaVbleCountsNeg;
int *arrLastLemmaVbleCountsPos;
int *arrLastLemmaVbleCountsNeg;

/* functions specific hooks */
fnCreateFunction              *procCreateFunction=NULL;

// Functino Stack
Save2Stack                    *procSave2Stack=NULL;
RestoreFromStack              *procRestoreFromStack=NULL;

// Create and Update Affected Functions per variable
CreateAFS                     *procCreateAFS=NULL;
AffectedVarList               *procAffectedVarList=NULL;
UpdateAffectedFunction        *procUpdateAffectedFunction=NULL;
UpdateAffectedFunction_Infer  *procUpdateAffectedFunction_Infer=NULL;
UpdateFunctionInfEnd          *procUpdateFunctionInfEnd=NULL;

// Update Heuristic per function
HeurUpdateFunctionInfEnd *procHeurUpdateFunctionInfEnd = NULL;

// Heuristics
HeurInit               procHeurInit = NULL;
HeurFree               procHeurFree = NULL;
HeurUpdate             procHeurUpdate = NULL;
HeurSelect             procHeurSelect = NULL;
HeurBacktrack          procHeurBacktrack = NULL;
HeurGetScores         *procHeurGetScores = NULL;
HeurUpdateLemma        procHeurAddLemma = NULL;
HeurUpdateLemma        procHeurRemoveLemma = NULL;
HeurUpdateLemmaSpace   procHeurAddLemmaSpace = NULL;

FnInfPriority arrFnInfPriority[MAX_FN_PRIORITY];
int nLastFnInfPriority;

ITE_INLINE int  InitBrancherX();
int HeuristicInit();

int
solve_init()
{
  int ret = SOLV_UNKNOWN;

  d9_printf1("InitSolver\n");

  
  if (BREAK_XORS) {
  // change splitXors to add two new functions into functions array to complement/replace the existing smurf
     int total_vars = splitXors();
     d9_printf3("Split(%ld) returned %d\n", numinp, total_vars);
     numinp = total_vars;
  }

  HeuristicInit();

  InitVarMap();

  InitVariables();

  InitFunctions();

  /* Convert bdds and special functions to smurfs */
  ret = CreateFunctions();
  if (ret != SOLV_UNKNOWN) return ret;

  bdd_gc(1);

  FnStackInit();

  AFSInit();
  
  procHeurInit();

  BtStackInit();

  ret = InitBrancherX();
  if (ret != SOLV_UNKNOWN) return ret;

  //if (K_TOP_VARIABLES) ret = kSolver(K_TOP_VARIABLES);
  //if (ret != SOLV_UNKNOWN) return ret;
  return ret;
}

void
solve_free()
{
  d5_printf1("FreeSolver\n");

  d4_printf5("SMURF States Statistic(total %ld): %ld/%ld (%f hit rate)\n",
        (long)(ite_counters[SMURF_NODE_NEW]),
        (long)(ite_counters[SMURF_NODE_FIND] - ite_counters[SMURF_NODE_NEW]),
        (long)(ite_counters[SMURF_NODE_FIND]),
        ite_counters[SMURF_NODE_FIND]==0?0:1.0 * (ite_counters[SMURF_NODE_FIND] - ite_counters[SMURF_NODE_NEW]) / ite_counters[SMURF_NODE_FIND]);

  BtStackFree();

  procHeurFree();
  
  AFSFree();

  FnStackFree();

  FreeFunctions();

  FreeVariables();

  FreeVarMap();
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
HeuristicInit()
{
   /* decode heuristic type -- johnson, lemma, interactive, state */
   switch (sHeuristic[0]) {
    case 'n': procHeurInit = HrNullInit; break;
    case 'j': 
              switch (sHeuristic[1]) {
               case 'l': procHeurInit = HrLSGBLemmaInit; break;
               case 's': procHeurInit = HrLSGBWInit; break;
               case 'S': procHeurInit = HrLSGBWInit; break;
               case 'q': procHeurInit = HrLSGBWInit; break;
               case 'Q': procHeurInit = HrLSGBWInit; break;
               case 'r': procHeurInit = HrLSGBWInit; break;
               case 'R': procHeurInit = HrLSGBWInit; break;
               case 'd': procHeurInit = HrLSGBWInit; break;
               case 0: procHeurInit = HrLSGBInit; break;
               default: dE_printf2("error: Unknown LSGB type heuristic %s\n", sHeuristic+1);
              }
              break;
    case 'l': procHeurInit = HrLemmaInit; break;
    case 'v': procHeurInit = HrVSIDSInit; break;
    case 'b': procHeurInit = HrBerkMinInit; break;
	 case 'a': procHeurInit = HrAnneInit; break;
	 case 'm': procHeurInit = HrMiniSatInit; break;
    //case 'i': nHeuristic = INTERACTIVE_HEURISTIC; break;
    //case 'm': nHeuristic = STATE_HEURISTIC; break;
    default: 
              dE_printf2("error: Unknown heuristic type %c\n", sHeuristic[0]); 
              exit(1);
              break;
   }
   return NO_ERROR;
}
