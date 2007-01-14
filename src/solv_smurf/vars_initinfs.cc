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

ITE_INLINE int
RecordInitialInferences()
{
   int i;
   BDDNode *pFunc;
   int nCurrentAtomValue;
   bool bInitialInferenceFound = false;

   for (i = 0; i < nmbrFunctions; i++)
   {
      if (IsSpecialFunc(functionType[i]))
      {
         // We assume that special funcs have no literal implicants.
         continue;
      }
      pFunc = functions[i];
      infer *head = pFunc->inferences;
      while(head != NULL)
      {
         if (head->nums[1] != 0) { head=head->next; continue; }
         int nVble = head->nums[0];
         head=head->next;
         int nValueOfVble = BOOL_TRUE;;
         if (nVble < 0) { 
            nVble = -nVble; 
            nValueOfVble = BOOL_FALSE; 
         }

         if (!bInitialInferenceFound)
         {
            printf("\nWarning.  Inference found in constraint %d", i);
            printf("\n prior to beginning backtracking search:  ");
            printf("\nBrancher variable %d(%d) = ", arrIte2SolverVarMap[nVble], nVble);
            assert(nValueOfVble == BOOL_TRUE || nValueOfVble == BOOL_FALSE);
            printf((nValueOfVble == BOOL_TRUE) ? "true." : "false.");
            printf("\nCurrent value = ");
            printf((arrSolution[arrIte2SolverVarMap[nVble]] == BOOL_TRUE) ? "true." :
                  (arrSolution[arrIte2SolverVarMap[nVble]] == BOOL_FALSE) ?  "false." :
               "unknown");
            printf("\n numinp = %ld numout = %ld", (long)numinp, (long)numout);
            D_3(printBDD(arrFunctions[i]); )
            printf("\nThis message will appear only once for the first inference found\n");
//            bInitialInferenceFound = true;
         }
         nVble = arrIte2SolverVarMap[nVble];
         assert(nVble != 0);

         nCurrentAtomValue = arrSolution[nVble];
         if (nValueOfVble == BOOL_TRUE)
         {
            // Positive inference
            if (nCurrentAtomValue == BOOL_FALSE)
            {
               // Conflict -- problem is unsatisfiable.
               dE_printf1("Conflict -- problem is unsatisfiable.\n");
               //goto_NoSolution;
               return SOLV_UNSAT;
            }
            else if (nCurrentAtomValue == BOOL_UNKNOWN)
            {
               // Set value of atom.
               arrSolution[nVble] = BOOL_TRUE;

               // Enqueue inference.
               *(pInferenceQueueNextEmpty++) = nVble;
            }
         }
         else
         {
            // Negative inference
            if (nCurrentAtomValue == BOOL_TRUE)
            {
               // Conflict -- problem is unsatisfiable.
               dE_printf1("Conflict -- problem is unsatisfiable.\n");
               //goto_NoSolution;
               return 1;
            }
            else if (nCurrentAtomValue == BOOL_UNKNOWN)
            {
               // Set value of atom.
               arrSolution[nVble] = BOOL_FALSE;

               // Enqueue inference.
               *(pInferenceQueueNextEmpty++) = nVble;
            }
         }
      }
   }

   return SOLV_UNKNOWN;
}
