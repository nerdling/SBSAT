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

t_smurf_chain *arrSmurfChain;

void ExtendFns(int new_num)
{
   if (new_num <= nNumFns) return;
   procCreateFunction = (fnCreateFunction*)ite_recalloc(procCreateFunction, nNumFns, new_num, sizeof(void*), 2, "");
   procSave2Stack = (Save2Stack*)ite_recalloc(procSave2Stack, nNumFns, new_num, sizeof(void*), 2, "");
   procRestoreFromStack = (RestoreFromStack*)ite_recalloc(procRestoreFromStack, nNumFns, new_num, sizeof(void*), 2, "");
   procCreateAFS = (CreateAFS*)ite_recalloc(procCreateAFS, nNumFns, new_num, sizeof(void*), 2, "");
   procAffectedVarList = (AffectedVarList*)ite_recalloc(procAffectedVarList, nNumFns, new_num, sizeof(void*), 2, "");
   procUpdateAffectedFunction = (UpdateAffectedFunction*)ite_recalloc(procUpdateAffectedFunction, nNumFns, new_num, sizeof(void*), 2, "");
   procUpdateAffectedFunction_Infer = (UpdateAffectedFunction_Infer*)ite_recalloc(procUpdateAffectedFunction_Infer, nNumFns, new_num, sizeof(void*), 2, "");
   procUpdateFunctionInfEnd = (UpdateFunctionInfEnd*)ite_recalloc(procUpdateFunctionInfEnd, nNumFns, new_num, sizeof(void*), 2, "");
   procHeurUpdateFunctionInfEnd = (HeurUpdateFunctionInfEnd*)ite_recalloc(procHeurUpdateFunctionInfEnd, nNumFns, new_num, sizeof(void*), 2, "");
   procHeurGetScores = (HeurGetScores*)ite_recalloc(procHeurGetScores, nNumFns, new_num, sizeof(void*), 2, "");
   nNumFns = new_num;
}  
   


SmurfState *pTrueSmurfState = NULL;  // Pointer to the Smurf state
//representing the Boolean function 'true'.

ITE_INLINE int
FnSmurfInit() 
{
   ExtendFns(MAX_FUNC);

   for(int i=0;i<MAX_FUNC;i++)
   {
      procCreateFunction[i] = SmurfCreateFunction;

      // Function Stack
      procSave2Stack[i] = SmurfSave2Stack;
      procRestoreFromStack[i] = SmurfRestoreFromStack;

      // Create and Update Affected Functions per variable
      procCreateAFS[i] = SmurfCreateAFS;
      procAffectedVarList[i] = SmurfAffectedVarList;
      procUpdateAffectedFunction[i] = SmurfUpdateAffectedFunction;
      procUpdateAffectedFunction_Infer[i] = SmurfUpdateAffectedFunction_Infer;
      procUpdateFunctionInfEnd[i] = SmurfUpdateFunctionInfEnd;
      //procFunctionFree[0] = FnSmurfFree;

   }

   /* allocate an array for smurf initial states */
   //d3_printf2("Number of regular Smurfs: %d\n", nNumRegSmurfs);

   // Initialize info regarding the 'true' function.
   pTrueSmurfState = AllocateSmurfState();
	pTrueSmurfState->pFunc = true_ptr;
	pTrueSmurfState->fNodeHeuristicWeight = JHEURISTIC_K_TRUE;
/*
   if (nNumRegSmurfs > 0)
   {

      arrSmurfChain = (t_smurf_chain*)ite_calloc(nNumRegSmurfs, sizeof(t_smurf_chain),
            9, "arrSmurfChain");

      for (int i = 0; i < nNumRegSmurfs; i++)
      {
         arrSmurfChain[i].next = -1;
         arrSmurfChain[i].prev = -1;
         arrSmurfChain[i].specfn = -1;
      }

   }
   */
   return NO_ERROR;
}

ITE_INLINE void
Smurf_Free() {
   d9_printf1("FreeSmurfFactory\n");
/*
   long states_one=0, states_two=0, states_three=0;
   for (int i = 0; i < nNumRegSmurfs; i++) {
      CountSmurfsUsage(arrRegSmurfInitialStates[i], &states_one, &states_two, &states_three);
   }

   d3_printf4("SmurfState State all: %lld, visited: %ld, heuristic: %ld\n",
         ite_counters[SMURF_NODE_NEW], states_one+states_three, states_two);
*/
   /* from smurf factory */
   /*
   for (int i = 0; i < nNumRegSmurfs; i++)
      ite_free((void**)&arrSmurfPath[i].literals);

   ite_free((void**)&arrSmurfChain);
   */
   FreeSmurfStatePool();
}

/*
ITE_INLINE void
CountSmurfsUsage(SmurfState *pState, long *one, long *two, long *three)
{
   if (pState->cVisited == 0) return; else
   if (pState->cVisited == 1) (*one)++; else
   if (pState->cVisited == 2) (*two)++; else
   if (pState->cVisited == 3) (*three)++; else assert(0);
   pState->cVisited = 0;

   for(int i=0; i<pState->vbles.nNumElts; i++) {
      Transition *pTransition;
      pTransition = FindTransition(pState, i, pState->vbles.arrElts[i], BOOL_TRUE);
      if (pTransition->pNextState) CountSmurfsUsage(pTransition->pNextState, one, two, three);
      pTransition = FindTransition(pState, i, pState->vbles.arrElts[i], BOOL_FALSE);
      if (pTransition->pNextState) CountSmurfsUsage(pTransition->pNextState, one, two, three);
   }
}
*/

