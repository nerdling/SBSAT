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

ITE_INLINE void FillLemmaWithReversedPolarities(LemmaBlock *pLemma);

ITE_INLINE void
InferNLits_MINMAX(int nFnId, int nNumRHSUnknowns, int value)
{
   int nNumElts = arrSolverFunctions[nFnId].fn_minmax.rhsVbles.nNumElts;
   int *arrElts = arrSolverFunctions[nFnId].fn_minmax.rhsVbles.arrElts;
   int arrNewPolar[BOOL_MAX];
   if (value == BOOL_TRUE) {
      arrNewPolar[BOOL_TRUE] = BOOL_TRUE;
      arrNewPolar[BOOL_FALSE] = BOOL_FALSE;
   } else {
      arrNewPolar[BOOL_TRUE] = BOOL_FALSE;
      arrNewPolar[BOOL_FALSE] = BOOL_TRUE;
   }
   if (NO_LEMMAS == 1) 
   {
      for (int j = 0; j < nNumElts; j++)
      {
         int vble = arrElts[j];
         if (arrSolution[vble] == BOOL_UNKNOWN)
         {
            InferLiteral(vble, arrNewPolar[arrSolverFunctions[nFnId].fn_minmax.arrRHSPolarities[j]],
                  false, NULL, NULL, 1);
         }
      }
   } else {
      int *arrLits = (int*)ite_calloc(nNumElts-nNumRHSUnknowns+1, sizeof(int), 
            9, "arrlits for minmax infer lemma");
      int nLitsPos = 0;
      for(int j=0; j<nNumElts; j++)
      {
         int vble = arrElts[j];
         if (arrSolution[vble]!=BOOL_UNKNOWN)
         {
            arrLits[nLitsPos++] = vble * (arrSolution[vble]==arrSolverFunctions[nFnId].fn_minmax.arrRHSPolarities[j]?-1:1);
            assert(arrSolution[vble] == (arrLits[nLitsPos-1] > 0 ? BOOL_FALSE : BOOL_TRUE));
         }
      }
      assert(nLitsPos == nNumElts-nNumRHSUnknowns);
      LemmaBlock *pLemma = NULL;
      LemmaInfoStruct *pLemmaInfo = NULL;
      for (int j = 0; j < nNumElts; j++)
      {
         int vble = arrElts[j];
         if (arrSolution[vble] == BOOL_UNKNOWN)
         {
            arrLits[nLitsPos] = vble * (arrNewPolar[arrSolverFunctions[nFnId].fn_minmax.arrRHSPolarities[j]]==BOOL_TRUE?1:-1);
            pLemmaInfo=AddLemma(nNumElts-nNumRHSUnknowns+1, arrLits, false, NULL, NULL);
            pLemma = pLemmaInfo->pLemma;
				pLemmaInfo->nLemmaCameFromSmurf = 1;
            InferLiteral(vble, arrNewPolar[arrSolverFunctions[nFnId].fn_minmax.arrRHSPolarities[j]],
                  false, pLemma, pLemmaInfo, 1);
         }
      }
      free(arrLits);
   }
}


ITE_INLINE int
UpdateSpecialFunction_MINMAX(int nFnId)
{
   // need to retrieve the counter too to be effective
   int counter=0;
   int nNumRHSUnknowns = 0;
   int nNumElts = arrSolverFunctions[nFnId].fn_minmax.rhsVbles.nNumElts;
   int *arrElts = arrSolverFunctions[nFnId].fn_minmax.rhsVbles.arrElts;
   for (int j = 0; j < nNumElts; j++) 
   {
      int vble = arrElts[j];
    //  fprintf(stderr, "\nvble(%d) value(%d) pola(%d)", vble, arrSolution[vble], arrSolverFunctions[nFnId].fn_minmax.arrRHSPolarities[j]);
      if (arrSolution[vble]==BOOL_UNKNOWN)
      {
         nNumRHSUnknowns++;
      }
      else if (arrSolution[vble] == arrSolverFunctions[nFnId].fn_minmax.arrRHSPolarities[j])
      {
         counter++;
      }
   }
   //fprintf(stderr, "\nUnknowns(%d), counter(%d)\n", nNumRHSUnknowns, counter);

   if (nNumRHSUnknowns == 0 || counter > arrSolverFunctions[nFnId].fn_minmax.max || 
         (counter >= arrSolverFunctions[nFnId].fn_minmax.min && (counter+nNumRHSUnknowns)<=arrSolverFunctions[nFnId].fn_minmax.max) ||
         ((counter+nNumRHSUnknowns)< arrSolverFunctions[nFnId].fn_minmax.min))
   {
      // check for conflict? 
      if (counter < arrSolverFunctions[nFnId].fn_minmax.min || counter > arrSolverFunctions[nFnId].fn_minmax.max)
      {
         // Conflict -- backtrack.
         // create lemma 
         if (NO_LEMMAS == 0) {
            //assert(arrSolverFunctions[nFnId].fn_minmax.pLongLemma);
            //FillLemmaWithReversedPolarities(arrSolverFunctions[nFnId].fn_minmax.pLongLemma);
            //pConflictLemma = arrSolverFunctions[nFnId].fn_minmax.pLongLemma;
            
            /* assign it to pConflictLemmaInfo so we can free it */
            int *arrLits = (int*)ite_calloc(nNumElts-nNumRHSUnknowns, sizeof(int), 
                  9, "arrlits for minmax conflict lemma");
            int nLitsPos=0;
            for(int j=0; j<nNumElts; j++)
            {
               int vble = arrElts[j];
               if (arrSolution[vble]!=BOOL_UNKNOWN)
               {
                  // reverse all polarities
                  arrLits[nLitsPos++] = vble * (arrSolution[vble]==arrSolverFunctions[nFnId].fn_minmax.arrRHSPolarities[j]?-1:1);
                  assert(arrSolution[vble] == (arrLits[nLitsPos-1] > 0 ? BOOL_FALSE : BOOL_TRUE));
               }
            }
            pConflictLemmaInfo = AddLemma(nLitsPos, arrLits, false, NULL, NULL);
				pConflictLemmaInfo->nLemmaCameFromSmurf = 1;
            pConflictLemma = pConflictLemmaInfo->pLemma;
				
				free(arrLits);
         }
         // goto_Backtrack;
         return ERR_BT_SPEC_FN_MINMAX;
      }

      // The function is now satisfied.
      nNumRHSUnknowns = 0;
      nNumUnresolvedFunctions--;

   } // nNumRHSUnknowns == 0 
   else
      if ((counter+nNumRHSUnknowns) == arrSolverFunctions[nFnId].fn_minmax.min) {
         // infer all remaining to TRUE
         ite_counters[INF_SPEC_FN_MINMAX] += nNumRHSUnknowns;
         InferNLits_MINMAX(nFnId, nNumRHSUnknowns, BOOL_TRUE); // uses arrShortLemmas
         nNumRHSUnknowns = 0;
         nNumUnresolvedFunctions--;
      }
      else if (counter == arrSolverFunctions[nFnId].fn_minmax.max)
      {
         // infer all remaining to FALSE
         ite_counters[INF_SPEC_FN_MINMAX] += nNumRHSUnknowns;
         InferNLits_MINMAX(nFnId, nNumRHSUnknowns, BOOL_FALSE); // uses arrShortLemmas
         nNumRHSUnknowns = 0;
         nNumUnresolvedFunctions--;
      }

   arrSolverFunctions[nFnId].fn_minmax.nNumRHSUnknowns = nNumRHSUnknowns;
   arrSolverFunctions[nFnId].fn_minmax.nRHSCounter = counter;
   /*
   if (nHeuristic != JOHNSON_HEURISTIC) 
           {
              arrPrevNumRHSUnknowns[nFnId] = arrNumRHSUnknowns[nFnId];
              arrPrevRHSCounter[nFnId]     = arrRHSCounter[nFnId];
           }
           */

   return NO_ERROR;
}

int MinMaxUpdateAffectedFunction(int nFnId)
{
   // big update
   if (arrSolverFunctions[nFnId].fn_minmax.nNumRHSUnknowns <= 0) return NO_ERROR ;
   return UpdateSpecialFunction_MINMAX(nFnId); 
}

int MinMaxUpdateAffectedFunction_Infer(void *oneafs, int x)
{
   // small update
   arrSolverFunctions[((OneAFS*)oneafs)->nFnId].fn_minmax.nNumRHSUnknownsNew--;
   return NO_ERROR;
}

int MinMaxSave2Stack(int nFnId, void *one_stack)
{
   ((FnStack*)one_stack)->fn_minmax.rhs_unknowns =
      arrSolverFunctions[nFnId].fn_minmax.nNumRHSUnknowns;
   ((FnStack*)one_stack)->fn_minmax.rhs_counter =
      arrSolverFunctions[nFnId].fn_minmax.nRHSCounter;
   return NO_ERROR;
}

int MinMaxRestoreFromStack(void *one_stack)
{
   int nFnId = ((FnStack*)one_stack)->nFnId;
   arrSolverFunctions[nFnId].fn_minmax.nNumRHSUnknowns =
      arrSolverFunctions[nFnId].fn_minmax.nNumRHSUnknownsNew =
      arrSolverFunctions[nFnId].fn_minmax.nNumRHSUnknownsPrev =
      ((FnStack*)one_stack)->fn_minmax.rhs_unknowns;
   arrSolverFunctions[nFnId].fn_minmax.nRHSCounter =
      arrSolverFunctions[nFnId].fn_minmax.nRHSCounterNew =
      arrSolverFunctions[nFnId].fn_minmax.nRHSCounterPrev =
      ((FnStack*)one_stack)->fn_minmax.rhs_counter;

   return NO_ERROR;
}

void MinMaxUpdateFunctionInfEnd(int nFnId)
{
   arrSolverFunctions[nFnId].fn_minmax.nNumRHSUnknownsNew =
      arrSolverFunctions[nFnId].fn_minmax.nNumRHSUnknownsPrev =
      arrSolverFunctions[nFnId].fn_minmax.nNumRHSUnknowns;
}
 

