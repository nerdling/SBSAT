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
XUpdateEachAffectedFunction(AffectedFuncsStruct *pAFS, int x)
{
   int ret = NO_ERROR;

   if (pAFS == NULL) return NO_ERROR;

   // Determine the potentially affected special functions.
   OneAFS *pOneAFS = pAFS->arrOneAFS;

   for (int i = 0; i < pAFS->nNumOneAFS; i++, pOneAFS++)
   {
      if ((arrChangedFn[pOneAFS->nFnId]&1)==0) continue;

      assert((arrChangedFn[pOneAFS->nFnId]&3)==3);
      arrChangedFn[pOneAFS->nFnId]=2;

      d9_printf2("Big update function %d ", pOneAFS->nFnId);

      ret = procUpdateAffectedFunction[pOneAFS->nType](pOneAFS->nFnId);
      if (ret != NO_ERROR) {
         d9_printf2("Conflict %d ", pOneAFS->nFnId);
         break;
      }
   }
   return ret;
}


ITE_INLINE int
UpdateEachAffectedFunction(AffectedFuncsStruct *pAFS, int max_fn_priority)
{
   int ret = NO_ERROR;

   nLastFnInfPriority = 0;
   while(nLastFnInfPriority < max_fn_priority)
   {
      while(arrFnInfPriority[nLastFnInfPriority].First != NULL) {
         int nCurFnPriority = nLastFnInfPriority;
         int nFnId = arrFnInfPriority[nLastFnInfPriority].First->nFnId;
         int nType = arrFnInfPriority[nLastFnInfPriority].First->nType;

         assert((arrChangedFn[nFnId]&3)==3);
         d9_printf2("Big update function %d ", nFnId);

         ret = procUpdateAffectedFunction[nType](nFnId);
			if (ret != NO_ERROR) {
            d9_printf2("Conflict %d ", nFnId);
            goto update_conflict;
            break;
         }

         arrChangedFn[nFnId]=2;
         arrFnInfPriority[nCurFnPriority].First = 
            arrFnInfPriority[nCurFnPriority].First->pFnInfNext;
         if (arrFnInfPriority[nCurFnPriority].First == NULL)
            arrFnInfPriority[nCurFnPriority].Last = NULL; 

			if(pInferenceQueueNextElt < pInferenceQueueNextEmpty && nCurFnPriority == MAX_FN_PRIORITY-1) {
				nLastFnInfPriority = 0;
				return ret;
			}
		}
      nLastFnInfPriority++;
   }
update_conflict:
   nLastFnInfPriority = 0;
   return ret;
}

ITE_INLINE void
Functions_Update_Inf_End()
{
   t_fn_inf_queue *pFnInfQueue = arrFnInferenceQueue;
   while (pFnInfQueue < pFnInferenceQueueNextElt) {
      if (procHeurUpdateFunctionInfEnd[pFnInfQueue->nType]) 
         procHeurUpdateFunctionInfEnd[pFnInfQueue->nType](pFnInfQueue->nFnId);
      procUpdateFunctionInfEnd[pFnInfQueue->nType](pFnInfQueue->nFnId);
      arrChangedFn[pFnInfQueue->nFnId]=0;
      pFnInfQueue++;
   }
   return;
}

ITE_INLINE void
Functions_Update_Inf_End_np()
{
   t_fn_inf_queue *pFnInfQueue = arrFnInferenceQueue;
   while (pFnInfQueue < pFnInferenceQueueNextElt) {
      procUpdateFunctionInfEnd[pFnInfQueue->nType](pFnInfQueue->nFnId);
      arrChangedFn[pFnInfQueue->nFnId]=0;
      pFnInfQueue++;
   }
   return;
}

/*
      d9_printf2("JHeuristic update for special function %d\n", i);
      switch(arrSpecialFuncs[i].nFunctionType) {
          //case SFN_AND: J_UpdateHeuristic_AND(arrSpecialFuncs+i, arrPrevNumRHSUnknowns[i], arrNumRHSUnknowns[i], 
          //                arrPrevNumLHSUnknowns[i], arrNumLHSUnknowns[i]);
       case SFN_AND: 
          J_UpdateHeuristic_AND_C(arrSpecialFuncs+i, 
                arrPrevNumRHSUnknowns[i], arrNumRHSUnknowns[i], 
                arrPrevNumLHSUnknowns[i], arrNumLHSUnknowns[i],
                arrPrevSumRHSUnknowns[i], arrSumRHSUnknowns[i]);
          break;
          //case SFN_XOR: J_UpdateHeuristic_XOR(arrSpecialFuncs+i, 
          //                arrPrevNumRHSUnknowns[i], arrNumRHSUnknowns[i], -1);
       case SFN_XOR: J_UpdateHeuristic_XOR_C(arrSpecialFuncs+i, 
                           arrPrevNumRHSUnknowns[i], arrNumRHSUnknowns[i], 
                           arrPrevSumRHSUnknowns[i], arrSumRHSUnknowns[i], -1);
                     break;
       case SFN_MINMAX: J_UpdateHeuristic_MINMAX(arrSpecialFuncs+i, 
                              arrPrevNumRHSUnknowns[i], arrNumRHSUnknowns[i], 
                              arrPrevRHSCounter[i], arrRHSCounter[i]);
                        break;
       default: assert(0);
                exit(1);
                break;
      }
-----------------
      arrPrevNumRHSUnknowns[i] = arrNumRHSUnknowns[i];
      arrPrevNumLHSUnknowns[i] = arrNumLHSUnknowns[i];
      arrPrevSumRHSUnknowns[i] = arrSumRHSUnknowns[i];
      arrPrevRHSCounter[i] = arrRHSCounter[i];
      arrChangedSpecialFn[i]=0;
      */
/*
ITE_INLINE void
UPDATE_HEURISTIC()
{
   for(int i=0;i<nNumSpecialFuncs;i++)
   {
      if (arrNumRHSUnknowns[i] != arrPrevNumRHSUnknowns[i] ||
            arrNumLHSUnknowns[i] != arrPrevNumLHSUnknowns[i])
      {
         switch(arrSpecialFuncs[i].nFunctionType) {
          case SFN_AND: J_UpdateHeuristic_AND(arrSpecialFuncs+i, arrPrevNumRHSUnknowns[i], arrNumRHSUnknowns[i], 
                          arrPrevNumLHSUnknowns[i], arrNumLHSUnknowns[i]);
                    break;
          case SFN_XOR: J_UpdateHeuristic_XOR(arrSpecialFuncs+i, arrPrevNumRHSUnknowns[i], arrNumRHSUnknowns[i], -1);
                    break;
          default: assert(0);
                   exit(1);
                   break;
         }
         arrPrevNumRHSUnknowns[i] = arrNumRHSUnknowns[i];
         arrPrevNumLHSUnknowns[i] = arrNumLHSUnknowns[i];
      }
      if (arrChangedSpecialFn[i] != 0)
         arrChangedSpecialFn[i]=0;
   }

}
*/
