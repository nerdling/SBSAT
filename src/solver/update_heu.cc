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

// assert USE_LEMMA_VAR_HEURISTIC is not set... !

#include "ite.h"
#include "solver.h"

#undef UPDATE_HEURISTIC

#ifdef JHEURISTIC
#define UPDATE_HEURISTIC J_UpdateHeuristic
#endif // JHEURISTIC

#ifndef UPDATE_HEURISTIC
#define UPDATE_HEURISTIC UpdateHeuristic
#endif

ITE_INLINE void
UPDATE_HEURISTIC()
{
   t_fn_inf_queue *pFnInfQueue = arrFnInferenceQueue;
   while (pFnInfQueue < pFnInferenceQueueNextElt) {
      int i = pFnInfQueue->fn_id;
      if (pFnInfQueue->fn_type == 0) {
#ifdef JHEURISTIC
         J_UpdateHeuristicSmurf(arrPrevStates[i], arrCurrentStates[i], i);
#endif
         arrPrevStates[i] = arrCurrentStates[i];
         arrChangedSmurfs[i]=0;
      } else {
#ifdef JHEURISTIC
         d9_printf2("JHeuristic update for special function %d\n", i);
         switch(arrSpecialFuncs[i].nFunctionType) {
          //case AND: J_UpdateHeuristic_AND(arrSpecialFuncs+i, arrPrevNumRHSUnknowns[i], arrNumRHSUnknowns[i], 
          //                arrPrevNumLHSUnknowns[i], arrNumLHSUnknowns[i]);
          case AND: 
             J_UpdateHeuristic_AND_C(arrSpecialFuncs+i, 
                          arrPrevNumRHSUnknowns[i], arrNumRHSUnknowns[i], 
                          arrPrevNumLHSUnknowns[i], arrNumLHSUnknowns[i],
                          arrPrevSumRHSUnknowns[i], arrSumRHSUnknowns[i]);
                    break;
          //case XOR: J_UpdateHeuristic_XOR(arrSpecialFuncs+i, 
          //                arrPrevNumRHSUnknowns[i], arrNumRHSUnknowns[i], -1);
          case XOR: J_UpdateHeuristic_XOR_C(arrSpecialFuncs+i, 
                          arrPrevNumRHSUnknowns[i], arrNumRHSUnknowns[i], 
                          arrPrevSumRHSUnknowns[i], arrSumRHSUnknowns[i], -1);
                    break;
          default: assert(0);
                   exit(1);
                   break;
         }
#endif
         arrPrevNumRHSUnknowns[i] = arrNumRHSUnknowns[i];
         arrPrevNumLHSUnknowns[i] = arrNumLHSUnknowns[i];
         arrPrevSumRHSUnknowns[i] = arrSumRHSUnknowns[i];
         arrChangedSpecialFn[i]=0;
      }
      pFnInfQueue++;
   }
   return;
}

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
          case AND: J_UpdateHeuristic_AND(arrSpecialFuncs+i, arrPrevNumRHSUnknowns[i], arrNumRHSUnknowns[i], 
                          arrPrevNumLHSUnknowns[i], arrNumLHSUnknowns[i]);
                    break;
          case XOR: J_UpdateHeuristic_XOR(arrSpecialFuncs+i, arrPrevNumRHSUnknowns[i], arrNumRHSUnknowns[i], -1);
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

   for(int i=0;i<nNumRegSmurfs;i++)
   {
      if (arrCurrentStates[i] != arrPrevStates[i]) {
         J_UpdateHeuristicSmurf(arrPrevStates[i], arrCurrentStates[i], i);
         arrPrevStates[i] = arrCurrentStates[i];
      }
      if (arrChangedSmurfs[i] != 0)
         arrChangedSmurfs[i]=0;
   }
}
*/
