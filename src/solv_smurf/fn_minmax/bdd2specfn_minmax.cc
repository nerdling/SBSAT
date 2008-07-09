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

ITE_INLINE void
BDD2Specfn_MINMAX(int nFnId, BDDNode *pFunc, int nFunctionType, int nEqualityVble)
{
   assert(nFunctionType == MINMAX); // PLAINMINMAX?

   /* -------------- Right Hand Side -------------- */
   /* 0. Create the List of Variables and Polarities */		  

   // Store variable set of RHS.
   // Here we assume that the LHS variable does not also occur on
   // the RHS of the equality.
   int tempint_max = 0;
   int nNumElts=0;
   unravelBDD(&nNumElts, &tempint_max, &(arrSolverFunctions[nFnId].fn_minmax.rhsVbles.arrElts), pFunc);

   /* set polarities and map variables from ite to solver ordering */
   int *arrRHSPolarities = 
      arrSolverFunctions[nFnId].fn_minmax.arrRHSPolarities
      = (int *)ite_calloc(nNumElts, sizeof(int),
            9, "special function polarities");

   for(int i=0;i<nNumElts;i++) {
      arrRHSPolarities[i] = BOOL_UNKNOWN;
   }
   qsort(arrSolverFunctions[nFnId].fn_minmax.rhsVbles.arrElts, nNumElts, sizeof(int), revcompfunc);

   arrSolverFunctions[nFnId].fn_minmax.rhsVbles.nNumElts = nNumElts;

   /* 1. Create BDD representing RHS. */
   BDDNodeStruct *pRHSFunc = pFunc;

   /* 2. Traverse the BDD, storing the variable index and polarity  for each literal. */
   BDDNodeStruct *pCurrentNode = pRHSFunc;
   int i = 0; // Index into the array of variables mentioned in the func.
   while (pCurrentNode != true_ptr && pCurrentNode != false_ptr)
   {
      if (arrSolverFunctions[nFnId].fn_minmax.rhsVbles.arrElts[i] != pCurrentNode->variable) {
         printBDD(pRHSFunc);
         assert(0);
      }
      assert(arrSolverFunctions[nFnId].fn_minmax.rhsVbles.arrElts[i] == pCurrentNode->variable);
      arrRHSPolarities[i] = BOOL_TRUE;
      if (pCurrentNode->thenCase != true_ptr && pCurrentNode->thenCase != false_ptr)
         pCurrentNode = pCurrentNode->thenCase;
      else
         pCurrentNode = pCurrentNode->elseCase;
      i++;
   }

   // convert variable ids from global to solver
   for(int i=0;i<nNumElts;i++) {
      arrSolverFunctions[nFnId].fn_minmax.rhsVbles.arrElts[i] = arrIte2SolverVarMap[arrSolverFunctions[nFnId].fn_minmax.rhsVbles.arrElts[i]];
   }

   // set the bounds - max
   arrSolverFunctions[nFnId].fn_minmax.max=0;
   pCurrentNode = pRHSFunc;
   while (pCurrentNode != true_ptr && pCurrentNode != false_ptr)
   {
      pCurrentNode = pCurrentNode->thenCase;
      arrSolverFunctions[nFnId].fn_minmax.max++;
   }
   if (pCurrentNode == true_ptr) arrSolverFunctions[nFnId].fn_minmax.max = arrSolverFunctions[nFnId].fn_minmax.rhsVbles.nNumElts;
   else arrSolverFunctions[nFnId].fn_minmax.max--;

   // set the bounds - min
   arrSolverFunctions[nFnId].fn_minmax.min=arrSolverFunctions[nFnId].fn_minmax.rhsVbles.nNumElts;
   pCurrentNode = pRHSFunc;
   while (pCurrentNode != true_ptr && pCurrentNode != false_ptr)
   {
      pCurrentNode = pCurrentNode->elseCase;
      arrSolverFunctions[nFnId].fn_minmax.min--;
   }
   if (pCurrentNode == true_ptr) arrSolverFunctions[nFnId].fn_minmax.min = 0;
   else arrSolverFunctions[nFnId].fn_minmax.min++;

   assert(arrSolverFunctions[nFnId].fn_minmax.min <= arrSolverFunctions[nFnId].fn_minmax.max);

}

int MinMaxCreateFunction(int nFnId, BDDNode *bdd, int nFnType, int eqVble)
{
   arrSolverFunctions[nFnId].nFnId = nFnId;
   arrSolverFunctions[nFnId].nType = nFnType;
   BDD2Specfn_MINMAX(nFnId, bdd, nFnType, eqVble);
   arrSolverFunctions[nFnId].fn_minmax.nNumRHSUnknowns =
      arrSolverFunctions[nFnId].fn_minmax.nNumRHSUnknownsNew =
      arrSolverFunctions[nFnId].fn_minmax.nNumRHSUnknownsPrev =
      arrSolverFunctions[nFnId].fn_minmax.rhsVbles.nNumElts;
   return 0;
}

void MinMaxAffectedVarList(int nFnId, int **arr1, int *num1, int **arr2, int *num2)
{
   *num1 = arrSolverFunctions[nFnId].fn_minmax.rhsVbles.nNumElts;
   *arr1 = arrSolverFunctions[nFnId].fn_minmax.rhsVbles.arrElts;
   *num2 = 0;
   *arr2 = NULL;
}

void MinMaxCreateAFS(int nFnId, int nVarId, int nAFSIndex)
{
   arrAFS[nVarId].arrOneAFS[nAFSIndex].nFnId = nFnId;
   arrAFS[nVarId].arrOneAFS[nAFSIndex].nType = arrSolverFunctions[nFnId].nType;
}

