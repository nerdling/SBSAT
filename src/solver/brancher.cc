/* =========FOR INTERNAL USE ONLY. NO DISTRIBUTION PLEASE ========== */

/*********************************************************************
 Copyright 1999-2003, University of Cincinnati.  All rights reserved.
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

ITE_INLINE int BackTrack();
ITE_INLINE int BackTrack_NL();
ITE_INLINE int BackTrack_SBJ();
ITE_INLINE void SelectNewBranchPoint();
ITE_INLINE int UpdateEachAffectedLemma(AffectedFuncsStruct *pAFS, int nInferredValue);
ITE_INLINE int UpdateEachAffectedSpecialFunction(AffectedFuncsStruct *pAFS);
ITE_INLINE int UpdateEachAffectedRegularSmurf(AffectedFuncsStruct *pAFS);
ITE_INLINE void DisplayBacktrackInfo(double &fPrevEndTime, double &fStartTime);
ITE_INLINE void DisplayStatistics(int nNumChoicePts, int nNumBacktracks, int nNumBackjumps);
ITE_INLINE int RecordSolution(); 
ITE_INLINE void graph_free();
ITE_INLINE void J_ResetHeuristicScores();

extern int nNumRegSmurfs; // Number of regular Smurfs.
SmurfState *pTrueSmurfState = 0;  // Pointer to the Smurf state
//representing the Boolean function 'true'.
extern int NO_LEMMAS;

int *arrFunctionType;
BDDNodeStruct **arrFunctions;
int nAssertionFailedVble = 0;
int gnNumCachedLemmas=0;
extern BacktrackStackEntry *arrBacktrackStack; 
extern SmurfState **arrRegSmurfInitialStates;


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

int nNumVariables;

AffectedFuncsStruct *arrAFS; // "Affected Funcs Struct":
SmurfState **arrCurrentStates=0; // Current states of all of the Smurfs, i.e.,
// arrCurrentStates[i] is a pointer to the current state of the i-th Smurf.
// arrCurrentStates is basically a "top-of-stack" pointer into arrStateInfoStack.
SmurfState **arrPrevStates=0; // Current states of all of the Smurfs, i.e.,
char *arrSolution;
int nVble;

ChoicePointStruct *arrChoicePointStack; // Info on previous branch variables.
ChoicePointStruct *pStartChoicePointStack; 
ChoicePointStruct *pChoicePointTop; // Next free position in above stack.

int nCallsToAddLemma =  0;
int nBacktrackStackIndex = 1;
BacktrackStackEntry *pBacktrackTop; // Next free position in backtrack stack.
// The next five identifiers are used for identifying and processing
// a lemma which witnesses a forced assignment which causes a conflict
// during the search.
int *arrConflictLemmaLits;

/* Backtrack arrays */
int *arrUnsetLemmaFlagVars;

int nNumUnresolvedFunctions;

int *arrNumRHSUnknowns = 0;
int *arrNumRHSUnknownsNew = 0;
int *arrPrevNumRHSUnknowns = 0;

int *arrNumLHSUnknowns = 0;
int *arrNumLHSUnknownsNew = 0;
int *arrPrevNumLHSUnknowns = 0;

double *arrSumRHSUnknowns = 0;
double *arrSumRHSUnknownsNew = 0;
double *arrPrevSumRHSUnknowns = 0;

extern long nTimeLimit;
extern long nNumChoicePointLimit;
extern int nCtrlC;

ITE_INLINE
bool
CheckLimits(double fStartTime)
{
   double fEndTime, fDurationInSecs;
   fEndTime = get_runtime();
   fDurationInSecs = fEndTime - fStartTime;

   if (nCtrlC) {
      return 1;
   }

   if (nTimeLimit && (fEndTime - fStartTime) > nTimeLimit) {
      d2_printf2("Bailling out because the Time limit %ld ", nTimeLimit);
      d2_printf2("is smaller than elapsed time %.0f\n", (fEndTime - fStartTime));
      return 1;
   }

   if (nNumChoicePointLimit && ite_counters[NUM_CHOICE_POINTS]>nNumChoicePointLimit) {
      d2_printf2("Bailling out because the limit on the number of choicepoints %ld ",
            nNumChoicePointLimit);
      d2_printf2("is smaller than the number of choicepoints %ld\n", (long)ite_counters[NUM_CHOICE_POINTS]);
      return 1;
   }

   return 0;
}

ITE_INLINE int
ITE_Deduce()
{
   int err=0;

   // While inference queue is nonempty
   while (pInferenceQueueNextElt < pInferenceQueueNextEmpty)
   {
      // Dequeue atom and value
      int nInferredAtom = *(pInferenceQueueNextElt++);
      int nInferredValue = arrSolution[nInferredAtom];

      // Determine the potentially affected constraints.
      AffectedFuncsStruct *pAFS = arrAFS + nInferredAtom;

      TB_9(
            d9_printf3("\nInferred Atom %c%d\n", 
               (nInferredValue==BOOL_TRUE?'+':'-'),
               nInferredAtom);
            );

      if ((err = UpdateEachAffectedRegularSmurf(pAFS))) break;
      if ((err = UpdateEachAffectedSpecialFunction(pAFS))) break;

      if (NO_LEMMAS == 0)
         if ((err = UpdateEachAffectedLemma(pAFS, nInferredValue))) break;

   } // while inference queue is non-empty
#ifdef MK_NULL 
   if (err == 0) err = ITE_Fn_Deduce();
#endif
   ite_counters[err]++;
   return err;
}

#ifdef MK_NULL
ITE_INLINE void
ITE_Fn_Deduce()
{
   // While fn inference queue is nonempty
   while (pFnInferenceQueueNextElt < pFnInferenceQueueNextEmpty)
   {
      // Dequeue func and value
      AffectedFuncsStruct *pAFS = arrAFS + nInferredAtom;
      int nInferredValue = pFnInferenceQueueNextElt->
         int nInferredValue = arrSolution[nInferredAtom];

      // Determine the potentially affected constraints.

      TB_9(
            d9_printf3("\nInferred Atom %c%d\n", 
               (nInferredValue==BOOL_TRUE?'+':'-'),
               nInferredAtom);
         )

         if ((err = UpdateEachAffectedRegularSmurf(pAFS))) break;
      if ((err = UpdateEachAffectedSpecialFunction(pAFS))) break;

      if (NO_LEMMAS == 0)
         if ((err = UpdateEachAffectedLemma(pAFS, nInferredValue))) break;
   } // while fn inference queue is non-empty
   return err;
}
#endif

double fStartTime;
double fEndTime;
double fPrevEndTime;
int (* proc_backtrack)() = NULL;

ITE_INLINE int
BrancherPreset()
{
   // brancher_presets
   char *ptr = brancher_presets;
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
                 break;
      }
      while (*ptr != ' ' && *ptr != 0) ptr++;
      if (*ptr == ' ') ptr++; // skip space
      ITE_MakeDecision(lit, sign);
      if (ITE_Deduce() != 0) return SOLV_UNSAT;
   }
   return SOLV_UNKNOWN;
}


ITE_INLINE void
dump_counters(FILE *fd)
{
   for(int i=0;i<MAX_COUNTER;i++)
      fprintf(fd, "%lld, ", ite_counters[i]);
   for(int i=0;i<MAX_COUNTER_F;i++)
      fprintf(fd, "%f, ", ite_counters_f[i]);
   fprintf(fd, "\n");
}

ITE_INLINE int
ITE_Split(int *lit)
{
   if (pStartChoicePointStack >= pChoicePointTop) return 1;

   pStartChoicePointStack++;
   return 0;
}

ITE_INLINE int
ITE_GetPath(int **path, int *path_size)
{
   return 0;
}

ITE_INLINE int
CheckBtHooks()
{
   int ret = 0;
   ite_counters[NUM_BACKTRACKS]++;
   d9_printf2("\nStarting backtrack # %ld\n", (long)ite_counters[NUM_BACKTRACKS]);
   if (ite_counters[NUM_BACKTRACKS] % BACKTRACKS_PER_STAT_REPORT == 0) {

      if (reports == 0) 
         DisplayBacktrackInfo(fPrevEndTime, fStartTime);
      else 
         crtwin();
      if (CheckLimits(fStartTime)) { 
         d2_printf1("Interrupting brancher. Solution Unknown.\n");
         ite_counters[ERR_LIMITS] = 1;
         return 1; /* FIXME: ERR_limits!! */
      }
      if (fd_csv_trace_file) {
         dump_counters(fd_csv_trace_file);
      }
   }
   if (ite_counters[NUM_BACKTRACKS] % 255 == 0)
      Update_arrVarScores();

   /* setup bt function as a hook with frequency 1 ? */
   ret = proc_backtrack();

//#define MK_SPLIT_TEST
#ifdef MK_SPLIT_TEST
   if (ite_counters[NUM_BACKTRACKS] == 10000) {
      int lit;
      int *path, path_size;
      if (ITE_Split(&lit) == 0) {
         ITE_GetPath(&path, &path_size);
      }
   } 
#endif

   return ret;
}

ITE_INLINE int
CheckInitHooks()
{
   int ret = SOLV_UNKNOWN;
   if (NO_LEMMAS == 1) proc_backtrack = BackTrack_NL;
   else if (sbj) proc_backtrack = BackTrack_SBJ;
   else proc_backtrack = BackTrack;

   if (reports != 0) crtwin_init();

   if (*brancher_presets) ret = BrancherPreset();

   return ret;
}

ITE_INLINE int
CheckFinalHooks()
{
   D_2(
         DisplayBacktrackInfo(fPrevEndTime, fStartTime);
         DisplayStatistics(ite_counters[NUM_CHOICE_POINTS], ite_counters[NUM_BACKTRACKS], ite_counters[NUM_TOTAL_BACKJUMPS]);
      );

#ifdef HAVE_IMAGE_JPEG
   D_2(
         if (jpg_filename[0]) {
         graph_generate(jpg_filename);
         }
      )
      graph_free();
#endif 
   return 0;
}

ITE_INLINE void
InitBrancherX()
{
   if (nNumRegSmurfs) {
      memset(arrChangedSmurfs, 0, sizeof(int)*nNumRegSmurfs);
   }

   if (nNumSpecialFuncs) {
      memset(arrChangedSpecialFn, 0, sizeof(int)*nNumSpecialFuncs);
   }

   for (int i = 0; i < nNumSpecialFuncs; i++) {
      arrPrevNumRHSUnknowns[i] =
         arrNumRHSUnknownsNew[i] =
         arrNumRHSUnknowns[i] = arrSpecialFuncs[i].rhsVbles.nNumElts;
      arrPrevNumLHSUnknowns[i] =
         arrNumLHSUnknownsNew[i] =
         arrNumLHSUnknowns[i] = arrSpecialFuncs[i].nLHSVble > 0? 1: 0;
      arrSumRHSUnknowns[i] = 0;

      for (int j=0; j<arrSpecialFuncs[i].rhsVbles.nNumElts; j++)
         arrSumRHSUnknowns[i] += arrJWeights[arrSpecialFuncs[i].rhsVbles.arrElts[j]];

      assert(arrSolution[0]!=BOOL_UNKNOWN);
   }

   /* for restart */
   for (int i = 1; i<nNumVariables; i++) {
      //assert(arrSolution[i] == BOOL_UNKNOWN);
      arrSolution[i] = BOOL_UNKNOWN;
   }
   
   nNumUnresolvedFunctions = nNumRegSmurfs + nNumSpecialFuncs; 

   int nRegSmurfIndex = 0;
   for (int i = 0; i < nmbrFunctions; i++)
   {
      if (!IsSpecialFunc(arrFunctionType[i]))
      {
         arrPrevStates[nRegSmurfIndex] =
         arrCurrentStates[nRegSmurfIndex] =
            arrRegSmurfInitialStates[nRegSmurfIndex];

         if (arrCurrentStates[nRegSmurfIndex] == pTrueSmurfState)
         {
            nNumUnresolvedFunctions--;
         }
         nRegSmurfIndex++;
      }
   }

   for (int i = 0; i < nNumRegSmurfs; i++) {
      arrSmurfPath[i].idx = 0;
   }

   for(int x = 1; x <= gnMaxVbleIndex; x++) 
   {
      arrLemmaFlag[x] = false;
      arrBacktrackStackIndex[x] = gnMaxVbleIndex+1;
   }
   arrBacktrackStackIndex[0] = 0;
   pInferenceQueueNextElt = pInferenceQueueNextEmpty = arrInferenceQueue;
   pFnInferenceQueueNextElt = pFnInferenceQueueNextEmpty = arrFnInferenceQueue;

  pStartChoicePointStack =
  pChoicePointTop = arrChoicePointStack;
  pConflictLemma = NULL;

  pStartBacktrackStack =
  pBacktrackTop = arrBacktrackStack;
  nBacktrackStackIndex = 1;

  if (nHeuristic == JOHNSON_HEURISTIC)
     J_ResetHeuristicScores();

 //*(pInferenceQueueNextEmpty++) = 0;
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

   // Main inferencing loop.
   fPrevEndTime = fStartTime = get_runtime();

   InitBrancherX();

   ret = CheckInitHooks(); /* FIXME: check for result */

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

   if (ite_counters_f[BRANCHER_TIME] < 0.0) ite_counters_f[BRANCHER_TIME] = 0.0; 
   ite_counters_f[BRANCHER_TIME] = get_runtime() - fStartTime;
   d2_printf2("Time in brancher:  %4.3f secs.\n", ite_counters_f[BRANCHER_TIME]);
   double fBacktracksPerSec;
   if (ite_counters_f[BRANCHER_TIME] == 0.0) fBacktracksPerSec = 0;
   else fBacktracksPerSec = ite_counters[NUM_BACKTRACKS] / ite_counters_f[BRANCHER_TIME];
   d2_printf2("%.3f backtracks per sec.\n", fBacktracksPerSec);      

   CheckFinalHooks();

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
