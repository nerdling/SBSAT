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

t_solution_info *solution_info = NULL;
t_solution_info *solution_info_head = NULL;

ITE_INLINE int
RecordSolution ()
{
   if (verify_solution) {
     VerifySolution ();
   }

   // collect all choice points 
   ChoicePointStruct *pChoicePoint = arrChoicePointStack;

   int  tmp_nNumElts = pChoicePointTop - pChoicePoint;
   int  tmp_nNumElts_allocate;
   if (result_display_type) {
      tmp_nNumElts_allocate = numinp+1;
   } else {
      tmp_nNumElts_allocate = tmp_nNumElts+1;
   }

   // FIXME: add lemma only if lemmas are enabled
   int *tmp_arrElts = (int*)ite_calloc(tmp_nNumElts_allocate, sizeof (int),
         9, "solution node");
   for (int i = 0; i<tmp_nNumElts; i++) {
      // FLIP ALL OF THEM!!! Just for the lemma though 
      tmp_arrElts[i] = pChoicePoint->nBranchVble * 
		  (arrSolution[pChoicePoint->nBranchVble]==BOOL_FALSE?1:-1);
      pChoicePoint++;
   };

   int nNumBlocks;
   LemmaBlock *pFirstBlock;
   LemmaBlock *pLastBlock;
   EnterIntoLemmaSpace(tmp_nNumElts, tmp_arrElts, 
			  false, pFirstBlock, pLastBlock, nNumBlocks);

   if (result_display_type) {
		// create another node in solution chain 
		t_solution_info *tmp_solution_info;
		tmp_solution_info = (t_solution_info*)ite_calloc(1, sizeof(t_solution_info),
 								  9, "solution array");
      tmp_solution_info->nNumElts = tmp_nNumElts_allocate;
      tmp_solution_info->arrElts = tmp_arrElts;
		
		if (solution_info_head == NULL) {
         solution_info = tmp_solution_info;
         solution_info_head = solution_info;
      } else {
			solution_info->next = (struct _t_solution_info*)tmp_solution_info;
			solution_info = (t_solution_info*)(solution_info->next);
      }
      for (int i = 0; i<nNumVariables; i++) {
         if (arrSolver2IteVarMap[i] <= numinp)
            tmp_solution_info->arrElts[arrSolver2IteVarMap[i]] = arrSolution[i];
      }
   } else {
      free(tmp_arrElts);
   }
	
   // is this the last solution we are looking for 
   ite_counters[NUM_SOLUTIONS]++;
   if (ite_counters[NUM_SOLUTIONS] == max_solutions) return 0;

   d5_printf1("Recording the solution and continuing backtracking...\n");
   pConflictLemma = pFirstBlock;
   // goto_Backtrack;
   ite_counters[ERR_BT_SOL_LEMMA]++;
   return 1;
}

