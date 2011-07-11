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

// External variables.
long gnTotalBytesForTransitionAus = 0;

ITE_INLINE TransitionAu *
CreateTransitionAu(SmurfAuState *pSmurfAuState, int i, int nSolverVble, int value, int nAutarkyVble)
{
   int nVble = arrSolver2IteVarMap[nSolverVble];

   // Compute transition that occurs when vble is set to true.
	BDDNodeStruct *pFuncEvaled;
	//If we are transitioning on the autarky variable, we send the smurf to the true state.
	//fprintf(stderr, "creating transition on %d(%d)=%d with autarkyvble %d\n", nVble, nSolverVble, value, nAutarkyVble);
	if(nSolverVble == nAutarkyVble) pFuncEvaled = true_ptr;
	else pFuncEvaled = set_variable(pSmurfAuState->pFunc, nVble, value==BOOL_TRUE?1:0);
	if(pFuncEvaled == false_ptr) 	//set the autarky variable to true
	  pFuncEvaled = ite_var(-arrSolver2IteVarMap[nAutarkyVble]);
   SmurfAuState *pSmurfAuStateOfEvaled = BDD2SmurfAu(pFuncEvaled, nAutarkyVble);
   assert(pSmurfAuStateOfEvaled != NULL);
   AddStateTransitionAu(pSmurfAuState, i, nSolverVble, value, nAutarkyVble,
								pFuncEvaled, pSmurfAuStateOfEvaled);
	
	//This is just a validity check of the newly created transition.
   TransitionAu *transition = FindTransitionAu(pSmurfAuState, i, nSolverVble, value, nAutarkyVble);
   assert(transition->pNextState != NULL);
   return transition;
}

ITE_INLINE SmurfAuState *
ComputeSmurfAuOfNormalized(BDDNodeStruct *pFunc, int nAutarkyVble)
   // Precondition:  *pFunc is 'normalized', i.e., no literal
   // is logically implied by *pFunc.
   // Creates SmurfAu states to represent the function and its children.
   // Autarky smurfs currently give no heurstic value.
	// This may change in the future.
{
   assert(pFunc != false_ptr);

   ite_counters[SMURF_AU_NODE_FIND]++;

   // Collapse 'true' SmurfAu states.
   if (pFunc == true_ptr) return pTrueSmurfAuState;

   //if (pFunc->pState_Au) return (SmurfAuState*)(pFunc->pState_Au);

   SmurfAuState *pSmurfAuState = AllocateSmurfAuState();

   ite_counters[SMURF_AU_NODE_NEW]++;

   pSmurfAuState->pFunc = pFunc;
   //pFunc->pState_Au = (void*)pSmurfAuState;

   // get all the variables
   int tempint_max = 0;
   int y=0;
   unravelBDD(&y, &tempint_max, &pSmurfAuState->vbles.arrElts, pFunc);
   pSmurfAuState->vbles.nNumElts = y;
   pSmurfAuState->vbles.arrElts = (int*)realloc(pSmurfAuState->vbles.arrElts, pSmurfAuState->vbles.nNumElts*sizeof(int));

   /* mapping ite->solver variables */
	int nAuVarIndex = -1;
	for (int i=0;i<pSmurfAuState->vbles.nNumElts;i++) {
      if (pSmurfAuState->vbles.arrElts[i]==0 || 
			 arrIte2SolverVarMap[pSmurfAuState->vbles.arrElts[i]]==0) {
         dE_printf1("\nassigned variable in an Autarky BDD in the solver");
         dE_printf3("\nvariable id: %d, true_false=%d\n", 
						  pSmurfAuState->vbles.arrElts[i],
						  variablelist[pSmurfAuState->vbles.arrElts[i]].true_false);
         //exit(1);
      }
		if(arrIte2SolverVarMap[pSmurfAuState->vbles.arrElts[i]] == nAutarkyVble)
		  nAuVarIndex = i;

		pSmurfAuState->vbles.arrElts[i] = arrIte2SolverVarMap[pSmurfAuState->vbles.arrElts[i]];
   }
	
	assert(nAuVarIndex!=-1);
	
	//Put the autarky variable at the front of the list, so it will be transitioned on first
	//when looking for inferences
	if(nAuVarIndex!=0) {
		int nSwap_Index = 0;
		int nSwapLit = pSmurfAuState->vbles.arrElts[nSwap_Index];
		pSmurfAuState->vbles.arrElts[nSwap_Index] = pSmurfAuState->vbles.arrElts[nAuVarIndex];
		pSmurfAuState->vbles.arrElts[nAuVarIndex] = nSwapLit;
	}
	
	assert(pSmurfAuState->vbles.arrElts[0] = nAutarkyVble);
	
	//Sort the variables.
   //qsort(pSmurfAuState->vbles.arrElts, pSmurfAuState->vbles.nNumElts, sizeof(int), revcompfunc);

   /* allocate transitions */
   int nBytesForTransitionAus = pSmurfAuState->vbles.nNumElts * 2 * sizeof (TransitionAu);
   pSmurfAuState->arrTransitionAus = (TransitionAu *)ite_calloc(pSmurfAuState->vbles.nNumElts * 2, sizeof (TransitionAu),
         9, "pSmurfAuState->arrTransitionAus");

   gnTotalBytesForTransitionAus += nBytesForTransitionAus;

	/*
	for(int i=0;i<pSmurfAuState->vbles.nNumElts;i++)
	  {
		  int nSolverVble = pSmurfAuState->vbles.arrElts[i];
		  
		  // Compute transition that occurs when vble is set to true.
		  CreateTransitionAu(pSmurfAuState, i, nSolverVble, BOOL_TRUE, nAutarkyVble);
		  // Compute transition that occurs when vble is set to false.
		  CreateTransitionAu(pSmurfAuState, i, nSolverVble, BOOL_FALSE, nAutarkyVble);
	  }
	 */
	
   return pSmurfAuState;
}

ITE_INLINE
SmurfAuState *
BDD2SmurfAu(BDDNodeStruct *pFunc, int nAutarkyVble)
   // Constructs a SmurfAu representation for the constraint *pFunc.
   // Returns 0 if constraint pointed to by pFunc is unsatisfiable.
   // Otherwise, returns a pointer to the initial SmurfAu state.
{
   // special autarky function -- represent with autarky state machine.
   //BDDNodeStruct *pReduct = set_variable_all_infs(pFunc);

	//There should never be an inference except on the autarky variable.
	//If this function has an inference, it must be the autarky variable.
	//A transition on the autarky variable sends the smurf to the true state.
	//if(pReduct != pFunc) pReduct = true_ptr;
	
   //return ComputeSmurfAuOfNormalized(pReduct, nAutarkyVble);
	return ComputeSmurfAuOfNormalized(pFunc, nAutarkyVble);
}

int SmurfAuCreateFunction(int nFnId, BDDNode *bdd, int nFnType, int eqVble)
{
   arrSolverFunctions[nFnId].nFnId = nFnId;
   arrSolverFunctions[nFnId].nType = nFnType;
   arrSolverFunctions[nFnId].nFnPriority = MAX_FN_PRIORITY-1;
   arrSolverFunctions[nFnId].fn_smurf_au.nSmurfAuEqualityVble = arrIte2SolverVarMap[abs(eqVble)];
   arrSolverFunctions[nFnId].fn_smurf_au.pInitialState = BDD2SmurfAu(bdd, arrIte2SolverVarMap[abs(eqVble)]);
	assert(arrSolverFunctions[nFnId].fn_smurf_au.pInitialState != pTrueSmurfAuState);
   if (arrSolverFunctions[nFnId].fn_smurf_au.pInitialState == pTrueSmurfAuState) {
      nNumUnresolvedFunctions--;
      d9_printf3("Decremented nNumUnresolvedFunctions to %d due to autarky smurf # %d\n",
            nNumUnresolvedFunctions, nFnId);
   }
   arrSolverFunctions[nFnId].fn_smurf_au.pPrevState = arrSolverFunctions[nFnId].fn_smurf_au.pCurrentState = arrSolverFunctions[nFnId].fn_smurf_au.pInitialState;
   arrSolverFunctions[nFnId].fn_smurf_au.arrSmurfAuPath.literals = (int*)ite_calloc(arrSolverFunctions[nFnId].fn_smurf_au.pInitialState->vbles.nNumElts+1, sizeof(int),
               9, "arrSmurfAuPath[].literals");
   return 0;
}

void SmurfAuAffectedVarList(int nFnId, int **arr1, int *num1, int **arr2, int *num2)
{
   *num1 = arrSolverFunctions[nFnId].fn_smurf_au.pInitialState->vbles.nNumElts;
   *arr1 = arrSolverFunctions[nFnId].fn_smurf_au.pInitialState->vbles.arrElts;
   *num2 = 0;
}

void SmurfAuCreateAFS(int nFnId, int nVarId, int nAFSIndex)
{
   arrAFS[nVarId].arrOneAFS[nAFSIndex].nFnId = nFnId;
   arrAFS[nVarId].arrOneAFS[nAFSIndex].nType = AUTARKY_FUNC;
}

