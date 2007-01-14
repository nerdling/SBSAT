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

extern int zecc_limit;
extern int *zecc_arr;

extern long long *arrCPsDepthBreadth;

ITE_INLINE void
SelectNewBranchPoint()
{
   int nInferredAtom = 0;
   int nInferredValue = BOOL_UNKNOWN;

   // We need to select a new branch point.
   d9_printf2("Calling heuristic to choose choice point #%ld\n", (long)ite_counters[NUM_CHOICE_POINTS]);

   //Functions_Update_Inf_End_np();
   Functions_Update_Inf_End();
   if (procHeurUpdate) procHeurUpdate();


//#define SEAN_ZECCHINA
#ifdef SEAN_ZECCHINA
   //override heuristic for first so many variables...
   //Special Zecchina code...Sean play!
   for(int zecc = 0; zecc < zecc_limit; zecc++) {
      if(zecc_arr[zecc]>0) {
         if(arrSolution[zecc_arr[zecc]] == BOOL_UNKNOWN) {
            nInferredAtom = zecc_arr[zecc];
            nInferredValue = BOOL_TRUE;
            break;
         }
      } else {
         if(arrSolution[-zecc_arr[zecc]] == BOOL_UNKNOWN) {
            nInferredAtom = -zecc_arr[zecc];
            nInferredValue = BOOL_FALSE;
            break;
         }
      }
   }
#endif

   nInferredAtom = GetChoicePointHint();
   if (nInferredAtom < 0) {
      nInferredAtom = -nInferredAtom;
      nInferredValue = BOOL_FALSE;
   } else {
      nInferredValue = BOOL_TRUE;
   }

   // Call heuristic.
   if (nInferredAtom == 0)
      procHeurSelect(&nInferredAtom, &nInferredValue);

   var_stat[nInferredAtom].chosen[nInferredValue]++;
   assert(nInferredValue == BOOL_TRUE || nInferredValue == BOOL_FALSE);
   assert(arrSolution[nInferredAtom] == BOOL_UNKNOWN);

   if (nInferredAtom > nIndepVars && nInferredAtom <= nDepVars)
      ite_counters[HEU_DEP_VAR]++;

   ite_counters[NUM_CHOICE_POINTS]++;

   FnStackPushFunctionLevel();

   // Push branch atom onto stack.
   pChoicePointTop->nBranchVble = nInferredAtom;

   // Push num unresolved functions
   pChoicePointTop->nNumUnresolved = nNumUnresolvedFunctions;

   pChoicePointTop++;
   pFnInferenceQueueNextElt = pFnInferenceQueueNextEmpty = arrFnInferenceQueue;
   pFnInfQueueUpdate = arrFnInferenceQueue;

   assert(nLastFnInfPriority == 0);

   //fprintf(stderr, "%d\n", pChoicePointTop-arrChoicePointStack);
   arrCPsDepthBreadth[pChoicePointTop-arrChoicePointStack]++;

   InferLiteral(nInferredAtom, nInferredValue, false, NULL, NULL, 0);
   return;
}

