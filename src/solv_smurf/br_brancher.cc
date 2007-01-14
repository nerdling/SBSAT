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

ITE_INLINE int UpdateEachAffectedLemma(AffectedFuncsStruct *pAFS, int nInferredValue);
ITE_INLINE void SelectNewBranchPoint();
ITE_INLINE void DisplayStatistics(int nNumChoicePts, int nNumBacktracks, int nNumBackjumps);
ITE_INLINE int RecordSolution(); 
ITE_INLINE void J_ResetHeuristicScores();
ITE_INLINE int RecordInitialInferences();


int gnNumCachedLemmas=0;
t_proc_hook proc_hook = NULL;

int *arrInferenceQueue; // Indicies of atoms that already have values
// infered, but which still need to be processed in order to update the
// Smurf states and in order to infer the resulting consequences.
// (Note:  we may want to use pointers to atoms here rather than array
// indicies?)  How it works:  When we decide that an atom is to take a
// particular value we (1) record the value in the arrSolution array, and
// (2) place a reference to the atom in the inference queue,
// so that the consequences of the atom taking that value will be
// processed later.
int *pInferenceQueueNextEmpty; // ptr to next empty slot in inference queue
int *pInferenceQueueNextElt; // ptr to next available elt in inference queue
//  For a given variable index,
// gives the functions may be affected by updating that variable.

t_fn_inf_queue *arrFnInferenceQueue=NULL;
t_fn_inf_queue *pFnInferenceQueueNextEmpty=NULL;
t_fn_inf_queue *pFnInferenceQueueNextElt=NULL;
t_fn_inf_queue *pFnInfQueueUpdate=NULL;

int nNumVariables=0;

AffectedFuncsStruct *arrAFS; // "Affected Funcs Struct":
SmurfState **arrCurrentStates=0; // Current states of all of the Smurfs, i.e.,
// arrCurrentStates[i] is a pointer to the current state of the i-th Smurf.
// arrCurrentStates is basically a "top-of-stack" pointer into arrStateInfoStack.
//SmurfState **arrPrevStates=0; // Current states of all of the Smurfs, i.e.,
char *arrSolution;
int nVble;

ChoicePointStruct *arrChoicePointStack; // Info on previous branch variables.
ChoicePointStruct *pStartChoicePointStack; 
ChoicePointStruct *pChoicePointTop; // Next free position in above stack.

int nBacktrackStackIndex = 1;
BacktrackStackEntry *pBacktrackTop; // Next free position in backtrack stack.
// The next five identifiers are used for identifying and processing
// a lemma which witnesses a forced assignment which causes a conflict
// during the search.
int *arrConflictLemmaLits;

/* Backtrack arrays */
int *arrUnsetLemmaFlagVars;

int nNumUnresolvedFunctions;

ITE_INLINE int CheckInitHooks();
ITE_INLINE int CheckFinalHooks();
ITE_INLINE int CheckBtHooks();

ITE_INLINE int
ITE_Deduce()
{
   int err=0;

   do {
      // While inference queue is nonempty
      while (pInferenceQueueNextElt < pInferenceQueueNextEmpty)
      {
         // Dequeue atom and value
         int nInferredAtom = *(pInferenceQueueNextElt++);
         int nInferredValue = arrSolution[nInferredAtom];

         var_stat[nInferredAtom].infs[nInferredValue]++;

         // Determine the potentially affected constraints.
         AffectedFuncsStruct *pAFS = arrAFS + nInferredAtom;

         TB_9(
               d9_printf3("\nInferred Atom %c%d\n", 
                  (nInferredValue==BOOL_TRUE?'+':'-'),
                  nInferredAtom);
             );

         if (NO_LEMMAS == 0) {
            err = UpdateEachAffectedLemma(pAFS, nInferredValue);
            if (err != NO_ERROR) goto deduce_error;
         }
         err = UpdateEachAffectedFunction(pAFS, MAX_FN_PRIORITY-1);
         if (err != NO_ERROR) goto deduce_error;

      } // while inference queue is non-empty

      err = UpdateEachAffectedFunction(NULL, MAX_FN_PRIORITY);
      if (err != NO_ERROR) goto deduce_error;
   } while (pInferenceQueueNextElt < pInferenceQueueNextEmpty);
deduce_error:

   ite_counters[err]++;
   return err;
}

// ITE_INLINE
int
Brancher()
   // The main brancher routine.
   // Returns true iff the Village has a solution,
   // in which case it allocates an array of integers to hold the
   // solution, and points *parrSolution to the array.
   // The array size will be _ircuit->variables.universeSize + 2.
   // The array values will all be BOOL_FALSE, BOOL_TRUE, or BOOL_UNKNOWN.
   // BOOL_UNKNOWN indicates a "do not care" situation for the
   // corresponding variable.
{  
   int ret = SOLV_UNKNOWN;

   ret = CheckInitHooks(); 

   if (ret == SOLV_UNKNOWN)
   while (1)
   {
      if (ITE_Deduce()==0)
      {
         assert(nNumUnresolvedFunctions >= 0);
         if (nNumUnresolvedFunctions != 0) {
            SelectNewBranchPoint();
            continue;
         }

         /* if this is the last solution we need -- break! */
         if (RecordSolution() == 0) break;

         /* backtrack and look for more solutions */
      };

      /* if nowhere to backtrack -- leave */
      if (CheckBtHooks() != 0) break;
   }

   CheckFinalHooks();

   d2_printf2("Time in brancher:  %4.3f secs.\n", ite_counters_f[BRANCHER_TIME]);
   double fBacktracksPerSec;
   if (ite_counters_f[BRANCHER_TIME] == 0.0) fBacktracksPerSec = 0;
   else fBacktracksPerSec = ite_counters[NUM_BACKTRACKS] / ite_counters_f[BRANCHER_TIME];
   d2_printf2("%.3f backtracks per sec.\n", fBacktracksPerSec);      

   if (ite_counters[NUM_SOLUTIONS])
      ret = SOLV_SAT;
   else		            
   if (ite_counters[ERR_LIMITS])
      ret = SOLV_UNKNOWN;
   else
      ret = SOLV_UNSAT;

   //FreeLemmas(MAX_NUM_CACHED_LEMMAS);
   return ret;
}

/*
ITE_INLINE int
BrancherPresetString(char *ptr)
{
   int lit, sign;
   while (*ptr != 0) {

      if (*ptr == '=' || *ptr == '!' || *ptr == '#') {
         if (*(ptr+1) != ' ' && *(ptr+1) != 0) { 
            dE_printf2("Error in preset string %s\n", ptr);
            return SOLV_ERROR;
         }
         ITE_GetNextDecision(&lit, &sign);
      } else {
         if (*ptr == '-' || *ptr == '+') lit = atoi(ptr+1);
         else lit = atoi(ptr);
      }
      switch (*ptr) {
       case '-': sign = 0; break;
       case '+': sign = 1; break;
       case '#':
       case '!': sign = 1-sign; break;
       case '=': break;
       default:
                 dE_printf2("Error in preset string %s\n", ptr);
                 return SOLV_ERROR;
      }
      while (*ptr != ' ' && *ptr != 0) ptr++;
      if (*ptr == ' ') ptr++; // skip space
      ITE_MakeDecision(lit, sign);
      if (ITE_Deduce() != 0) return SOLV_UNSAT;
   }
   return SOLV_UNKNOWN;
}

ITE_INLINE int
BrancherPresetInt(int *ptr)
{
   int lit, sign;
   while (*ptr != 0) {

      if (*ptr < 0) {
         lit = -*ptr;
         sign = 0;
      } else {
         lit = *ptr;
         sign = 1;
      }
      ptr++;
      if (arrSolution[lit] == BOOL_UNKNOWN) ITE_MakeDecision(lit, sign);
      else {
         if (arrSolution[lit] != sign) return SOLV_UNSAT;
      }
      if (ITE_Deduce() != 0) return SOLV_UNSAT;
   }
   return SOLV_UNKNOWN;
}
*/

ITE_INLINE int
ITE_Split(int **path, int *path_size)
{
   if (pStartChoicePointStack >= pChoicePointTop) return 1;
   int vble = pStartChoicePointStack->nBranchVble;
   pStartChoicePointStack++;

   int i=1;
   for (BacktrackStackEntry *ptr = arrBacktrackStack; 
         ptr->nBranchVble != vble; ptr++) i++;
   *path_size = i;
   i=0;
   *path = (int*)ite_calloc(*path_size, sizeof(int), 9, "path");
   BacktrackStackEntry *ptr = arrBacktrackStack; 
   for (; ptr->nBranchVble != vble; ptr++) {
      (*path)[i] = (ptr->nBranchVble << 1) + arrSolution[ptr->nBranchVble];
      i++;
   }
   (*path)[i] = (ptr->nBranchVble << 1) + 1-arrSolution[ptr->nBranchVble];
   return 0;
}

