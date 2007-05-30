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

/* 
 * input: pConflictLemma (used in ConstructTempLemma )
 * output:
 */

LemmaBlock * pConflictLemma=NULL;
LemmaInfoStruct * pConflictLemmaInfo=NULL;
bool *arrLemmaFlag=NULL;
int  *arrTempLemma=NULL;
int  *arrPrevTempLemma=NULL;
extern long long *arrInfsDepthBreadth;

bool *arrSmurfsRefFlag=NULL;
int *arrTempSmurfsRef=NULL;

ITE_INLINE
void
InferLiteral(int nInferredAtom,
      int nInferredValue,
      bool bWasChoicePoint,
      LemmaBlock *pLemma,
      LemmaInfoStruct *pCachedLemma,
      int infer)
{
   assert(nInferredValue == BOOL_TRUE || nInferredValue == BOOL_FALSE);

   //if(pLemma!=NULL) assert(pCachedLemma!=NULL); SEAN!!! Someday all functions should infer correct LemmaInfoStructs
	
   D_9(
         d9_printf3("Inferring %c%d ", (nInferredValue==BOOL_TRUE?'+':'-'),
            nInferredAtom);
        
         if (pLemma) {
        // DisplayLemmaStatus(pLemma);
         } else {
         if (infer == 0)
         d9_printf1("(choice point)\n");
         }
         
      )

   // Determine the potentially affected special functions.
   OneAFS *pOneAFS = arrAFS[nInferredAtom].arrOneAFS;
   int ret = NO_ERROR;

   // Update each affected special function.
   for (int i = 0; i < arrAFS[nInferredAtom].nNumOneAFS; i++, pOneAFS++)
   {
      d9_printf2("Small update function %d ", pOneAFS->nFnId);

      /* bit 0 - marked for big update
       * bit 1 - visited on this level -- information for the final update
       */

      if ((arrChangedFn[pOneAFS->nFnId]&1)==0) {
         if (arrChangedFn[pOneAFS->nFnId]==0) {
            FN_STACK_PUSH(pOneAFS->nFnId, pOneAFS->nType);
            /* add on the heuristic update stack */
            assert(pFnInferenceQueueNextElt - arrFnInferenceQueue < nNumFuncs);
            pFnInferenceQueueNextElt->nFnId = pOneAFS->nFnId;
            pFnInferenceQueueNextElt->nType = arrSolverFunctions[pOneAFS->nFnId].nType;
            pFnInferenceQueueNextElt++;
         } else {
            d9_printf1("Already in arrFnInferenceQueue\n");
         }

         int nFnPriority = arrSolverFunctions[pOneAFS->nFnId].nFnPriority;
         if (arrFnInfPriority[nFnPriority].Last != NULL)
         {
            arrFnInfPriority[nFnPriority].Last->pFnInfNext = 
               &arrSolverFunctions[pOneAFS->nFnId];
            arrFnInfPriority[nFnPriority].Last = 
               arrFnInfPriority[nFnPriority].Last->pFnInfNext;
         }
         else
         {
            arrFnInfPriority[nFnPriority].First = 
               arrFnInfPriority[nFnPriority].Last = 
               &arrSolverFunctions[pOneAFS->nFnId];
         }
         arrFnInfPriority[nFnPriority].Last->pFnInfNext = NULL;
         if (nLastFnInfPriority > nFnPriority) 
            nLastFnInfPriority = nFnPriority;

         arrChangedFn[pOneAFS->nFnId]=3;
      } else {
         d9_printf1("Already marked and not updated\n");
			int nFnPriority = arrSolverFunctions[pOneAFS->nFnId].nFnPriority;
         if (nLastFnInfPriority > nFnPriority) 
            nLastFnInfPriority = nFnPriority;
      }

      ret = procUpdateAffectedFunction_Infer[pOneAFS->nType](pOneAFS, 10);
      if (ret != NO_ERROR) 
      {
         d9_printf1("Small Update Error\n");
         break;
      }
   }


   assert (arrSolution[nInferredAtom] == BOOL_UNKNOWN);
   arrSolution[nInferredAtom] = nInferredValue;

   // Enqueue inference.
   *(pInferenceQueueNextEmpty++) = nInferredAtom;

   arrBacktrackStackIndex[nInferredAtom] = nBacktrackStackIndex++;

	//if(pBacktrackTop->nBranchVble == 1610 && ite_counters[NUM_BACKTRACKS]>60000) assert(0);
	
   pBacktrackTop->nBranchVble = nInferredAtom;
   pBacktrackTop->bWasChoicePoint = bWasChoicePoint;
   pBacktrackTop->pLemma = pLemma;
   pBacktrackTop->pLemmaInfo = pCachedLemma;
   pBacktrackTop++;

   arrInfsDepthBreadth[pBacktrackTop-arrBacktrackStack]++;
}


ITE_INLINE void
InferNLits(int nNumElts, int *arrElts, int *arrPolarities, LemmaBlock **arrShortLemmas, int n)
{
   for (int j = 0; j < nNumElts && n; j++)
   {
      if (arrSolution[arrElts[j]] == BOOL_UNKNOWN)
      {
         n--;
         InferLiteral(arrElts[j], arrPolarities[j],
               false,
               arrShortLemmas[j],
               NULL, 1);
      }
   }
   assert(n==0);
}

