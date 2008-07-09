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
ConstructLemmasForXOR(int nFnId);

ITE_INLINE void
BDD2Specfn_XOR(int nFnId, BDDNode *pFunc, int nFunctionType, int _nEqualityVble)
{
   assert(/*nFunctionType == XOR ||*/ nFunctionType == PLAINXOR || nFunctionType == XOR_PART_BDDXOR);

   /* -------------- Left Hand Side -------------- */

   // the result should be ODD for fn to be sat
   // The function is of the form 1 = l_1 + ... + l_n.
   // LHS should be -FALSE ==> TRUE
   // Store LHS variable. and polarity
   //arrSolverFunctions[nFnId].fn_xor.nLHSVble = arrIte2SolverVarMap[0]; /* ALWAYS FALSE VARIABLE */
   //arrSolverFunctions[nFnId].fn_xor.nLHSPolarity = BOOL_FALSE;
   //arrSolverFunctions[nFnId].fn_xor.nFunctionType = SFN_XOR;

   /* -------------- Right Hand Side -------------- */
   /* 0. Create the List of Variables and Polarities */		  

   // Store variable set of RHS.
   // Here we assume that the LHS variable does not also occur on
   // the RHS of the equality.
   //
   //
   int tempint_max = 0;
   int nNumElts=0;
   unravelBDD(&nNumElts, &tempint_max, &(arrSolverFunctions[nFnId].fn_xor.rhsVbles.arrElts), pFunc);

   /* set polarities and map variables from ite to solver ordering */
   int *arrRHSPolarities =
      arrSolverFunctions[nFnId].fn_xor.arrRHSPolarities
      = (int *)ite_calloc(nNumElts, sizeof(int),
            9, "special function polarities");

   qsort(arrSolverFunctions[nFnId].fn_xor.rhsVbles.arrElts, nNumElts, sizeof(int), revcompfunc);
   for(int i=0;i<nNumElts;i++) {
      arrSolverFunctions[nFnId].fn_xor.rhsVbles.arrElts[i] = arrIte2SolverVarMap[arrSolverFunctions[nFnId].fn_xor.rhsVbles.arrElts[i]];
      arrRHSPolarities[i] = BOOL_UNKNOWN;
   }
   arrSolverFunctions[nFnId].fn_xor.rhsVbles.nNumElts = nNumElts;


   /* 1. Create BDD representing RHS. */
   BDDNodeStruct *pRHSFunc = pFunc;

   /* 2. Traverse the BDD, storing the variable index and polarity  for each literal. */
   BDDNodeStruct *pCurrentNode = pRHSFunc;
   int i = 0; // Index into the array of variables mentioned in the func.
   while (pCurrentNode != true_ptr && pCurrentNode != false_ptr)
   {
      if (arrSolverFunctions[nFnId].fn_xor.rhsVbles.arrElts[i] != arrIte2SolverVarMap[pCurrentNode->variable]) {
         fprintf(stderr, "Error in XOR special function construction. Incorrect mapping.\n");
         printBDD(pRHSFunc);
         assert(0);
         exit(1);
      }
      assert(arrSolverFunctions[nFnId].fn_xor.rhsVbles.arrElts[i] == arrIte2SolverVarMap[pCurrentNode->variable]);
      arrRHSPolarities[i] = BOOL_TRUE;
      pCurrentNode = pCurrentNode->thenCase;
      i++;
   }
   /* the function result should be ODD to be sat */
   /* all true infers true while even number of nodes */
   /* all false infers true while odd number of nodes */
   if ((pCurrentNode == true_ptr  && (i % 2)==1) ||
         (pCurrentNode == false_ptr && (i % 2)==0) )
      arrRHSPolarities[0] = BOOL_TRUE; 
   else
      arrRHSPolarities[0] = BOOL_FALSE;

   ConstructLemmasForXOR(nFnId);
}

int XorCreateFunction(int nFnId, BDDNode *bdd, int nFnType, int eqVble)
{
   arrSolverFunctions[nFnId].nFnId = nFnId;
   arrSolverFunctions[nFnId].nType = nFnType;
   BDD2Specfn_XOR(nFnId, bdd, nFnType, eqVble);
   arrSolverFunctions[nFnId].fn_xor.nNumRHSUnknowns =
      arrSolverFunctions[nFnId].fn_xor.nNumRHSUnknownsNew =
      arrSolverFunctions[nFnId].fn_xor.nNumRHSUnknownsPrev =
      arrSolverFunctions[nFnId].fn_xor.rhsVbles.nNumElts;
   return 0;
}

void XorAffectedVarList(int nFnId, int **arr1, int *num1, int **arr2, int *num2)
{
   *num1 = arrSolverFunctions[nFnId].fn_xor.rhsVbles.nNumElts;
   *arr1 = arrSolverFunctions[nFnId].fn_xor.rhsVbles.arrElts;
   *num2 = 0;
   *arr2 = NULL;
}

void XorCreateAFS(int nFnId, int nVarId, int nAFSIndex)
{
   arrAFS[nVarId].arrOneAFS[nAFSIndex].nFnId = nFnId;
   arrAFS[nVarId].arrOneAFS[nAFSIndex].nType = arrSolverFunctions[nFnId].nType;
}


