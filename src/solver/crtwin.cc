/* =========FOR INTERNAL USE ONLY. NO DISTRIBUTION PLEASE ========== */

/*********************************************************************
 Copyright 1999-2004, University of Cincinnati.  All rights reserved.
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

ITE_INLINE void DisplayBacktrackInfo(double &fPrevEndTime, double &fStartTime);

void
crtwin_draw() 
{
   double fPrevEndTime=0, fStartTime=0;
   putpad(CL);/* clear the screen */

   move(30, 5);
   putpad(SO);/* standout mode */
   printf("SBSAT %s", VERSION);
   putpad(SE);/* end standout mode */

   move(0, 7);

   {
      FILE *tmpstd = stddbg;
      stddbg = stdout;
      DisplayBacktrackInfo(fPrevEndTime, fStartTime);
      stddbg = tmpstd;
   }
   fflush(stdout);
}

void
DumpAllVarsToFile(char *filename)
{
   FILE *fout = fopen(filename, "w");
   if (!fout) return;
   for (int i=0; i<nNumVariables; i++) {
      fprintf(fout, "%d ", i);
      fprintf(fout, "%d %d %d %d %d %d ",
            var_stat[i].chosen[0],
            var_stat[i].chosen[1],
            var_stat[i].backjumped[0],
            var_stat[i].backjumped[1],
            var_stat[i].infs[0],
            var_stat[i].infs[1]);
      if (arrHeurScores) {
         fprintf(fout, "%f %f ",
               arrHeurScores[i].Neg,
               arrHeurScores[i].Pos
               );
      }
      fprintf(fout, "\n");
   }
   fclose(fout);
}

void
dump_lemmas(char *_filename)
{
   char filename[128];
   get_freefile(_filename, NULL, filename, 128);
   DisplayAllBrancherLemmasToFile(filename, 1);
}

void
dump_vars(char *_filename)
{
   char filename[128];
   get_freefile(_filename, NULL, filename, 128);
   DumpAllVarsToFile(filename);
}

void
crtwin_cmd(char c)
{
   switch(c) {
    case 'L': if (*lemma_out_file) dump_lemmas(lemma_out_file); break;
    case 'V': if (*var_stat_file) dump_vars(var_stat_file); break;
    case '+': BACKTRACKS_PER_STAT_REPORT *= 2; break;
    case '-': if (BACKTRACKS_PER_STAT_REPORT > 1) BACKTRACKS_PER_STAT_REPORT /= 2; break;
    default: break;
   }
}

void
crtwin(void) {
   int draw = 0;
   char c;

#define PERIOD_DUMP_LEMMAS
#ifdef PERIOD_DUMP_LEMMAS
   if (*lemma_out_file) dump_lemmas(lemma_out_file); 
   if (*var_stat_file) dump_vars(var_stat_file); 
#endif

   do {
      c = term_getchar();
      if (c == 0) break;
      crtwin_cmd(c);
   } while(c != 0);

   if (draw == 0) crtwin_draw();
}

void
crtwin_init()
{
   init_terminal_in();
   init_terminal_out();
}


