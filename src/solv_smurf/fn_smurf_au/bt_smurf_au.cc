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
CheckSmurfAuInferences(int nSmurfAuIndex, int *arrInferences, int nNumInferences, int value)
{
   for (int j = 0; j < nNumInferences; j++)
   {
      int nNewInferredAtom = arrInferences[j];
      int nCurrentAtomValue = arrSolution[nNewInferredAtom];

      if (nCurrentAtomValue == value) continue;

      //LemmaBlock *pLemma = NULL;
      //LemmaInfoStruct *pLemmaInfo = NULL;
		if (NO_AU_LEMMAS == 0) {
			//Au smurfs don't give autarky lemmas just yet. To be added later.
		}
     /* 
		if (NO_LEMMAS == 0) { 
			//Au smurfs give 1 variable lemmas.
			//Create lemma 
         int *arrLits = (int*)ite_calloc(1, sizeof(int), 9, "arrLits"); //Lemma of size 1
			arrLits[0] = nNewInferredAtom * (value==BOOL_FALSE?-1:1);
			d9_printf3("\nAutarky Smurf #%d making inference %d\n", nSmurfAuIndex, arrLits[0]);
         
         pLemmaInfo=AddLemma(1, arrLits, false, NULL, NULL);
         pLemma = pLemmaInfo->pLemma;
         free(arrLits);
			
         //DisplayLemmaStatus(pLemma, arrSolution);
      } else {
         //pLemma = NULL;
         //pLemmaInfo = NULL;
      }
*/
      if (nCurrentAtomValue == BOOL_UNKNOWN)
      {
         ite_counters[INF_SMURF_AU]++;
         if (NO_LEMMAS == 1) {
            InferLiteral(nNewInferredAtom, value, false, NULL, NULL, 1);
         } else {
            AddChoicePointHint(nNewInferredAtom * (value==BOOL_FALSE?-1:1));
         }
      }
      else // if (nCurrentAtomValue != value)
      {
         // Conflict -- error
         dE_printf1("Conflict:  Autarky Smurf Can't cause a conflict\n");
         assert(0);

         return ERR_BT_SMURF_AU;
      }
   }
   return 0;
}

ITE_INLINE int 
UpdateRegularSmurfAu(int nSmurfAuIndex)
{
   SmurfAuState *pState;

   d9_printf2("Visiting Autarky Smurf #%d\n", nSmurfAuIndex);

   pState = arrSolverFunctions[nSmurfAuIndex].fn_smurf_au.pCurrentState;
   assert(pState != pTrueSmurfAuState);

   while(1) 
   {
      int vble = 0, k;
      int nNumElts = pState->vbles.nNumElts;
      int *arrElts = pState->vbles.arrElts;
      for (k = 0; k < nNumElts; k++) 
      {
         vble = arrElts[k];
         if (arrSolution[vble]!=BOOL_UNKNOWN)  break;
      }

      assert(nNumElts > 0);

      /* if no more instantiated variables */
      if (k == nNumElts) break;
		
      if (NO_AU_LEMMAS == 0) {
         // keep track of the path for lemmas
         // the atoms in the path is reversed for lemmas
			
			// Right now there are no autarky lemmas, so this is commented out
			// This code will probably have to change as well though.
			//arrSolverFunctions[nSmurfAuIndex].fn_smurf_au.arrSmurfAuPath.literals[arrSolverFunctions[nSmurfAuIndex].fn_smurf_au.arrSmurfAuPath.idx++] 
         //   = arrElts[k] * (arrSolution[vble]==BOOL_FALSE?1:-1);
      }

      // Get the transition.
		int nAutarkyVble = arrSolverFunctions[nSmurfAuIndex].fn_smurf_au.nSmurfAuEqualityVble;
      TransitionAu *pTransitionAu = FindTransitionAu(pState, k, vble, arrSolution[vble], nAutarkyVble);
      if (pTransitionAu->pNextState == NULL) pTransitionAu = CreateTransitionAu(pState, k, vble, arrSolution[vble], nAutarkyVble);
      assert(pTransitionAu->pNextState != NULL);

      if (pTransitionAu->positiveInferences.nNumElts &&
            CheckSmurfAuInferences(
               nSmurfAuIndex,
               pTransitionAu->positiveInferences.arrElts,
                  pTransitionAu->positiveInferences.nNumElts,
                  BOOL_TRUE)) {
         assert(0);
         dE_printf1("Autarky Smurf contradiction\n");
         exit(1);
         break;
      }

      if (pTransitionAu->negativeInferences.nNumElts &&
            CheckSmurfAuInferences(
               nSmurfAuIndex,
               pTransitionAu->negativeInferences.arrElts,
               pTransitionAu->negativeInferences.nNumElts,
               BOOL_FALSE)) {
         assert(0);
         dE_printf1("Autarky Smurf contradiction\n");
         exit(1);
         break;
      }

      pState->cVisited |= 1;
      arrSolverFunctions[nSmurfAuIndex].fn_smurf_au.pCurrentState =
         pState = pTransitionAu->pNextState;
      assert(pState != NULL);
      if (pState == pTrueSmurfAuState)
      {
         nNumUnresolvedFunctions--;
         TB_9(
               d9_printf3("Decremented nNumUnresolvedFunctions to %d due to autarky smurf # %d\n",
                  nNumUnresolvedFunctions, nSmurfAuIndex);
             )
            break; // break the for all vbles in smurf_au loop
      }

   } // while (1) there is a instantiated variable

   return NO_ERROR;
}

int SmurfAuUpdateAffectedFunction(int nFnId)
{
   if (arrSolverFunctions[nFnId].fn_smurf_au.pCurrentState == pTrueSmurfAuState) 
      return NO_ERROR;
   return UpdateRegularSmurfAu(nFnId);
}

int SmurfAuUpdateAffectedFunction_Infer(void *oneafs, int x)
{
   return NO_ERROR;
}

int SmurfAuSave2Stack(int nFnId, void *one_stack)
{
   ((FnStack*)one_stack)->fn_smurf_au.state = arrSolverFunctions[nFnId].fn_smurf_au.pCurrentState;
   if (NO_AU_LEMMAS == 0) 
      ((FnStack*)one_stack)->fn_smurf_au.path_idx = arrSolverFunctions[nFnId].fn_smurf_au.arrSmurfAuPath.idx;
   return NO_ERROR;
}

int SmurfAuRestoreFromStack(void *one_stack)
{
   int nFnId = ((FnStack*)one_stack)->nFnId;
   arrSolverFunctions[nFnId].fn_smurf_au.pPrevState =
   arrSolverFunctions[nFnId].fn_smurf_au.pCurrentState = ((FnStack*)one_stack)->fn_smurf_au.state;
   if (NO_AU_LEMMAS == 0)
      arrSolverFunctions[nFnId].fn_smurf_au.arrSmurfAuPath.idx = ((FnStack*)one_stack)->fn_smurf_au.path_idx;
   return NO_ERROR;
}

void SmurfAuUpdateFunctionInfEnd(int nFnId)
{
   arrSolverFunctions[nFnId].fn_smurf_au.pPrevState = arrSolverFunctions[nFnId].fn_smurf_au.pCurrentState;
}
