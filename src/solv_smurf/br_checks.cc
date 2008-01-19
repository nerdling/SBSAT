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

ITE_INLINE int BackTrack();
ITE_INLINE int BackTrack_NL();
ITE_INLINE int BackTrack_SBJ();
double fStartTime;
double fEndTime;
double fPrevEndTime;
int (* proc_backtrack)() = NULL;

ITE_INLINE void DisplayBacktrackInfo(double &fPrevEndTime, double &fStartTime);

ITE_INLINE void
dump_lemmas(char *_filename)
{
	char filename[256];
	get_freefile(_filename, NULL, filename, 256);
	DisplayAllBrancherLemmasToFile(filename, 1);
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

ITE_INLINE bool
CheckLimits(double fStartTime)
{
   double fEndTime;
   fEndTime = get_runtime();

   if (nCtrlC) {
      return 1;
   }

   if (nTimeLimit && (fEndTime - fStartTime) > nTimeLimit) {
      d2_printf2("Bailling out because the Time limit of %lds ", nTimeLimit);
      d2_printf2("is smaller than elapsed time %.0fs\n", (fEndTime - fStartTime));
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
CheckBtHooks()
{
   int ret = 0;
   ite_counters[NUM_BACKTRACKS]++;
   d9_printf2("\nStarting backtrack # %ld\n", (long)ite_counters[NUM_BACKTRACKS]);
   if (ite_counters[NUM_BACKTRACKS] % BACKTRACKS_PER_STAT_REPORT == 0) {

      if (reports == 0) {
         DisplayBacktrackInfo(fPrevEndTime, fStartTime);
			if (*lemma_out_file) dump_lemmas(lemma_out_file);
		}
      //else 
      //   crtwin();
      if (CheckLimits(fStartTime)) { 
         d2_printf1("Interrupting brancher. Solution Unknown.\n");
         ite_counters[ERR_LIMITS] = 1;
         return 1; /* LOOK: ERR_limits */
      }
      if (fd_csv_trace_file) {
         dump_counters(fd_csv_trace_file);
      }
   }

   /* setup bt function as a hook with frequency 1 ? */
   ret = proc_backtrack();

   if (ret==0 && proc_hook && ite_counters[NUM_BACKTRACKS] % 1000 == 0) {
      proc_hook();
   }

//#define MK_SPLIT_TEST
#ifdef MK_SPLIT_TEST
   if (ite_counters[NUM_BACKTRACKS] % 10000 == 0) {
      int *path, path_size;
      if (ITE_Split(&path, &path_size) == 0) {
         // got it!
         d2_printf1("------------- Split! -------------\n");
         for(int i=0;i<path_size;i++)
           d2_printf3("%c%d ", (path[i]&1?'+':'-'), path[i]>>1);
         d2_printf1("\n");
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
   //else if (sbj) proc_backtrack = BackTrack_SBJ;
   else proc_backtrack = BackTrack;

   //if (reports != 0) crtwin_init();

   //if (*solverr_presets) ret = BrancherPresetString(solver_presets);
   
   // Main inferencing loop.
   fPrevEndTime = fStartTime = get_runtime();

   return ret;
}

ITE_INLINE int
CheckFinalHooks()
{
   if (ite_counters_f[BRANCHER_TIME] < 0.0) ite_counters_f[BRANCHER_TIME] = 0.0; 
   ite_counters_f[BRANCHER_TIME] = get_runtime() - fStartTime;
   
   D_2(
         DisplayBacktrackInfo(fPrevEndTime, fStartTime);
         DisplayStatistics(ite_counters[NUM_CHOICE_POINTS], ite_counters[NUM_BACKTRACKS], ite_counters[NUM_TOTAL_BACKJUMPS]);
      );
	if (*lemma_out_file) dump_lemmas(lemma_out_file);
   
   return 0;
}

