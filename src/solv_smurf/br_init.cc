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

#include "sbsat.h"
#include "sbsat_solver.h"
#include "solver.h"

int *arrChangedFn = NULL;

// Stack of the indicies of the previous
// inferred variables (variables inferred to be true or inferred to be false,
// including branch variables)
BacktrackStackEntry *arrBacktrackStack=NULL; 
BacktrackStackEntry *pStartBacktrackStack=NULL; 
int * arrBacktrackStackIndex = NULL;

ITE_INLINE int
InitBrancherX()
{
   assert(arrSolution[0]!=BOOL_UNKNOWN);
   if (nNumFuncs && arrChangedFn) memset(arrChangedFn, 0, sizeof(int)*nNumFuncs);
   nNumUnresolvedFunctions = nNumFuncs; 
   BtStackClear();
   return SOLV_UNKNOWN;
}

/*
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
   for (int i = 0; i < nNumFuncs; i++) {
      arrSmurfPath[i].idx = 0;
   }
*/

/*
  if (nHeuristic == JOHNSON_HEURISTIC) {
      for (int i = 0; i < nNumSpecialFuncs; i++) {
         arrSumRHSUnknowns[i] = 0;

         for (int j=0; j<arrSpecialFuncs[i].rhsVbles.nNumElts; j++)
            arrSumRHSUnknowns[i] += arrJWeights[arrSpecialFuncs[i].rhsVbles.arrElts[j]];
      }

     J_ResetHeuristicScores();
  }
*/
//  return RecordInitialInferences();
   //  InitHeuristicTablesForSpecialFuncs(nNumVariables);

/*
   if (*csv_trace_file) {
      fd_csv_trace_file = fopen(csv_trace_file, "w");
   }
   if (fd_csv_trace_file) fclose(fd_csv_trace_file);
*/


ITE_INLINE void
BtStackInit()
{
   // init Inference queue 
   arrInferenceQueue
      = (int *)ite_calloc(nNumVariables, sizeof(*arrInferenceQueue), 2, 
            "inference queue");
   arrBacktrackStackIndex = (int*)ite_calloc(gnMaxVbleIndex+1, sizeof(int),
         9, "arrBacktrackStackIndex");
   arrChoicePointStack 
      = (ChoicePointStruct *)ite_calloc(gnMaxVbleIndex, sizeof(*arrChoicePointStack), 2,
            "choice point stack");
   arrBacktrackStack
      = (BacktrackStackEntry *)ite_calloc(nNumVariables, sizeof(*arrBacktrackStack), 2,
            "backtrack stack");

    // init Fn Inference queue 
   arrFnInferenceQueue
      = (t_fn_inf_queue *)ite_calloc(nNumFuncs+1, sizeof(t_fn_inf_queue), 2,
            "function inference queue");
   arrChangedFn = (int*)ite_calloc(nNumFuncs, sizeof(int),
         9, "arrChangedFn");
}
ITE_INLINE void
BtStackClear()
{
   for(int x = 1; x <= gnMaxVbleIndex; x++) 
   {
//      arrLemmaFlag[x] = false;
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
}

ITE_INLINE void
BtStackFree()
{
   ite_free((void**)&arrInferenceQueue);
   ite_free((void**)&arrFnInferenceQueue);

   ite_free((void**)&arrChangedFn);
   ite_free((void**)&arrChoicePointStack);
   ite_free((void**)&arrBacktrackStack);
}
