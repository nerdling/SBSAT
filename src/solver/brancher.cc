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

extern int nNumRegSmurfs; // Number of regular Smurfs.
SmurfState *pTrueSmurfState = 0;  // Pointer to the Smurf state
//representing the Boolean function 'true'.
extern int NO_LEMMAS;
extern t_solution_info *solution_info_head;

int *arrFunctionType;
BDDNodeStruct **arrFunctions;
int nAssertionFailedVble = 0;
AffectedFuncsStruct *garrAFS;
LemmaInfoStruct *pUnitLemmaList=NULL;
int gnNumCachedLemmas=0;


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

int nNumBacktracks = 0;
AffectedFuncsStruct *arrAFS; // "Affected Funcs Struct":
SmurfState **arrCurrentStates=0; // Current states of all of the Smurfs, i.e.,
// arrCurrentStates[i] is a pointer to the current state of the i-th Smurf.
// arrCurrentStates is basically a "top-of-stack" pointer into arrStateInfoStack.
SmurfState **arrPrevStates=0; // Current states of all of the Smurfs, i.e.,
char *arrSolution;
int nVble;

ChoicePointStruct *arrChoicePointStack; // Info on previous branch variables.
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

int nNumBackjumps = 0;
int nNumChoicePts = 0;
int nNumBytesInStateArray; // # of bytes in a current state array.
int nNumBytesInSpecialFuncStackEntry;

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

   if (nNumChoicePointLimit && nNumChoicePts>nNumChoicePointLimit) {
      d2_printf2("Bailling out because the limit on the number of choicepoints %ld ",
            nNumChoicePointLimit);
      d2_printf2("is smaller than the number of choicepoints %d\n", nNumChoicePts);
      return 1;
   }

   return 0;
}


// ITE_INLINE
int
SolveVillage()
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
   double fStartTime;
   double fEndTime;
   double fPrevEndTime;
   int err=0;

   if (reports != 0) crtwin_init();

   switch (nHeuristic) {
    case JOHNSON_HEURISTIC:
       proc_call_heuristic = J_OptimizedHeuristic;
       //= J_OptimizedHeuristic_Berm;
       J_InitHeuristicScores();
       D_9(
             DisplayJHeuristicValues();
          );
       break;
    case C_LEMMA_HEURISTIC:
       proc_call_heuristic = L_OptimizedHeuristic;
       break;
    case INTERACTIVE_HEURISTIC:
       proc_call_heuristic = I_OptimizedHeuristic;
       break;
    default:
       dE_printf1("Unknown heuristic\n");
       exit(1);
   }

   // Main inferencing loop.
   fPrevEndTime = fStartTime = get_runtime();

   if (!err)
      while (1)
      {
         // While inference queue is nonempty
         while (pInferenceQueueNextElt < pInferenceQueueNextEmpty)
         {
            // Dequeue atom and value
            int nInferredAtom = *(pInferenceQueueNextElt++);
            int nInferredValue = arrSolution[nInferredAtom];

            // Determine the potentially affected constraints.
            AffectedFuncsStruct *pAFS = arrAFS + nInferredAtom;

            D_9(
                  //if (nNumBacktracks > TRACE_START) 
                  fprintf(stddbg, "\nInferred Atom %c%d\n", 
                     (nInferredValue==BOOL_TRUE?'+':'-'),
                     nInferredAtom);
               )

               if ((err = UpdateEachAffectedRegularSmurf(pAFS))) break;
            if ((err = UpdateEachAffectedSpecialFunction(pAFS))) break;

            if (NO_LEMMAS == 0)
               if ((err = UpdateEachAffectedLemma(pAFS, nInferredValue))) break;

         } // while inference queue is non-empty

#ifdef MK_NULL 
         // While fn inference queue is nonempty
         while (pFnInferenceQueueNextElt < pFnInferenceQueueNextEmpty)
         {
            // Dequeue func and value
            AffectedFuncsStruct *pAFS = arrAFS + nInferredAtom;
            int nInferredValue = pFnInferenceQueueNextElt->
               int nInferredValue = arrSolution[nInferredAtom];

            // Determine the potentially affected constraints.

            D_9(
                  //if (nNumBacktracks > TRACE_START) 
                  fprintf(stddbg, "\nInferred Atom %c%d\n", 
                     (nInferredValue==BOOL_TRUE?'+':'-'),
                     nInferredAtom);
               )

               if ((err = UpdateEachAffectedRegularSmurf(pAFS))) break;
            if ((err = UpdateEachAffectedSpecialFunction(pAFS))) break;

            if (NO_LEMMAS == 0)
               if ((err = UpdateEachAffectedLemma(pAFS, nInferredValue))) break;
         } // while inference queue is non-empty
#endif

         ite_counters[err]++;

         if (err == 0)
         {
            assert(nNumUnresolvedFunctions >= 0);
            if (nNumUnresolvedFunctions != 0) {
               SelectNewBranchPoint();
               continue;
            }

            ret = RecordSolution();
            if (ret == SOLV_SAT) 
               /* this is the last solution we need -- so break! */
               break;

            ite_counters[err]++;
            /* look for more solutions */
         };

         err = 0;

         nNumBacktracks++;
         d9_printf2("\nStarting backtrack # %d\n", nNumBacktracks);

         if (nNumBacktracks % BACKTRACKS_PER_STAT_REPORT == 0) {

            if (reports == 0) 
               DisplayBacktrackInfo(fPrevEndTime, fStartTime);
            else 
               crtwin();
            if (CheckLimits(fStartTime)) { 
               d2_printf1("Interrupting brancher. Solution Unknown.\n");
               break; 
            }
         }

         if (NO_LEMMAS == 0) {
            /* YES -- we have lemmas */

            int ret_bt;

            if (sbj) ret_bt = BackTrack_SBJ();
            else ret_bt = BackTrack();

            if (ret_bt==0) 
            {
               if (ite_counters[NUM_SOLUTIONS])
                  ret = SOLV_SAT;
               else		            
                  ret = SOLV_UNSAT;
               break;
            }
            else continue;
         }

         if (BackTrack_NL()==0) {
            if (ite_counters[NUM_SOLUTIONS])
               ret = SOLV_SAT;
            else		          
               ret = SOLV_UNSAT;
            break;
         }
         else continue;

         /* point that can not be reached */
      }

   switch (nHeuristic) {
    case JOHNSON_HEURISTIC:
       J_FreeHeuristicScores();
       break;
    default: break;
   }

   D_2(
         DisplayBacktrackInfo(fPrevEndTime, fStartTime);
         DisplayStatistics(nNumChoicePts, nNumBacktracks, nNumBackjumps);
      );

   fEndTime = get_runtime();
   ite_counters_f[BRANCHER_TIME] = fEndTime - fStartTime;
   d2_printf2("Time in brancher:  %4.3f secs.\n", ite_counters_f[BRANCHER_TIME]);
   if (ite_counters_f[BRANCHER_TIME] == 0.0) ite_counters_f[BRANCHER_TIME] = 0.00001; /* something real small */
   double fBacktracksPerSec = (nNumBacktracks) / ite_counters_f[BRANCHER_TIME];
   d2_printf2("%.3f backtracks per sec.\n", fBacktracksPerSec);      

#ifdef HAVE_IMAGE_JPEG
   D_2(
         if (jpg_filename[0]) {
         graph_generate(jpg_filename);
         }
      )
      graph_free();
#endif 

   return ret;
}

