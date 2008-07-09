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

int nMaxRHSSize = 1;

ITE_INLINE void
BDD2Specfn_AND(int nFnId, BDDNode *pFunc, int nFunctionType, int nEqualityVble)
{
   assert(nFunctionType == OR || nFunctionType == AND || nFunctionType == PLAINOR);
   int nPolarityOfLHSVble = 0;
   //int nNewFunctionType = 0;

   /* -------------- Left Hand Side -------------- */

   nPolarityOfLHSVble = BOOL_TRUE;
   switch (nFunctionType) {
    case PLAINOR: 
       // Convert PLAINORs to ANDs
       //nNewFunctionType = SFN_AND;
       // The function is of the form l_1 \/ ... \/ l_n.
       // This is equivalent to X0 = not(l_1 \/ ... \/ l_n)
       // if X0 is set to false.  This is how we will represent
       // the constraint.  Note that this reduces to
       //  X0 = not l_1 /\ ... /\ not l_n, so it can be handled
       // as an AND= constraint.  We will force X0 = false in the brancher.
       //	  cout << "PLAINOR" << endl;
       //	  printBDD(pFunc);
       nEqualityVble = 0; /* ALWAYS FALSE VARIABLE */
       break;
    case OR: 
       // Convert ORs to ANDs
       //nNewFunctionType = SFN_AND;
       // The function is of the form l_0 = l_1 \/ ... \/ l_n
       // (this is what it means for the function to be an 'OR'.)
       // This is equivalent to the relation
       // not l_0 = not l_1 /\ ... /\ not l_n.
       // We will use this identity to store the function as an 'AND'.
       // This is accomplished simply by reversing the polarity of
       // the LHS literal and then scanning the BDD just like
       // it was defined to be an 'AND'.
       nPolarityOfLHSVble = BOOL_NEG(nPolarityOfLHSVble);
       /* break is missing intentionally */
    case AND:  
       assert(nEqualityVble != 0);
       //nNewFunctionType = SFN_AND;
       // Check polarity of LHS variable.
       if (nEqualityVble < 0)
       {
          nPolarityOfLHSVble = BOOL_NEG(nPolarityOfLHSVble);
          nEqualityVble = -nEqualityVble;
       }
       break;
    default: assert(0); exit(1); break;
   }


   // Store LHS variable. and polarity
   arrSolverFunctions[nFnId].fn_and.nLHSVble = arrIte2SolverVarMap[nEqualityVble];
   arrSolverFunctions[nFnId].fn_and.nLHSPolarity = nPolarityOfLHSVble;
   //arrSolverFunctions[nFnId].fn_and.nFunctionType = nNewFunctionType;
   //
   assert(nEqualityVble == 0 || arrSolverFunctions[nFnId].fn_and.nLHSVble != 0);



   /* -------------- Right Hand Side -------------- */


   /* 0. Create the List of Variables and Polarities */		  

   // Store variable set of RHS.
   // Here we assume that the LHS variable does not also occur on
   // the RHS of the equality.
  
   int tempint_max = 0;
   int nNumElts=0;
   unravelBDD(&nNumElts, &tempint_max, &(arrSolverFunctions[nFnId].fn_and.rhsVbles.arrElts), pFunc);
   if (nNumElts>1) {
      for(int i=0;i<nNumElts;i++) {
         if (arrSolverFunctions[nFnId].fn_and.rhsVbles.arrElts[i] == nEqualityVble) {
            arrSolverFunctions[nFnId].fn_and.rhsVbles.arrElts[i] = arrSolverFunctions[nFnId].fn_and.rhsVbles.arrElts[nNumElts-1];
            nNumElts--;
            break;
         }
      }
      qsort(arrSolverFunctions[nFnId].fn_and.rhsVbles.arrElts, nNumElts, sizeof(int), revcompfunc);
   }
   arrSolverFunctions[nFnId].fn_and.rhsVbles.nNumElts = nNumElts;
   arrSolverFunctions[nFnId].fn_and.rhsVbles.arrElts = (int*)realloc(arrSolverFunctions[nFnId].fn_and.rhsVbles.arrElts, arrSolverFunctions[nFnId].fn_and.rhsVbles.nNumElts*sizeof(int));
  
   
   for (int i=0;i<arrSolverFunctions[nFnId].fn_and.rhsVbles.nNumElts;i++) {
      assert(arrSolverFunctions[nFnId].fn_and.rhsVbles.arrElts[i] && arrIte2SolverVarMap[arrSolverFunctions[nFnId].fn_and.rhsVbles.arrElts[i]]);
      arrSolverFunctions[nFnId].fn_and.rhsVbles.arrElts[i] = arrIte2SolverVarMap[arrSolverFunctions[nFnId].fn_and.rhsVbles.arrElts[i]];
   }

   // Determine the polarities of the literals on the RHS of the equation:
   arrSolverFunctions[nFnId].fn_and.arrRHSPolarities
      = (int *)ite_calloc(arrSolverFunctions[nFnId].fn_and.rhsVbles.nNumElts, sizeof(int),
            9, "special function polarities");
   int *arrRHSPolarities = arrSolverFunctions[nFnId].fn_and.arrRHSPolarities;

   for (int i = 0; i < arrSolverFunctions[nFnId].fn_and.rhsVbles.nNumElts; i++)
   {
      arrRHSPolarities[i] = BOOL_UNKNOWN;
   }

   /* 1. Create BDD representing RHS. */
   BDDNodeStruct *pRHSFunc=NULL;
   switch (nFunctionType) {
    case PLAINOR:
       pRHSFunc = ite_not(pFunc);
       break;
    case OR:
    case AND:
       {
          pRHSFunc
             = set_variable(pFunc, nEqualityVble,
                   nPolarityOfLHSVble == BOOL_TRUE ? 1 : 0);
       }
       break;
    default: assert(0); exit(1); break;
   }


   /* 2. Traverse the BDD, storing the variable index and polarity  for each literal. */


   switch (nFunctionType) {
    case AND:
    case OR:
    case PLAINOR:
       {
          BDDNodeStruct *pCurrentNode = pRHSFunc;
          int i = 0; // Index into the array of variables mentioned in the func.
          while (pCurrentNode != true_ptr)
          {
             assert(pCurrentNode != false_ptr);
             assert(pCurrentNode->variable >= 0);
             assert(arrSolverFunctions[nFnId].fn_and.rhsVbles.arrElts[i] == arrIte2SolverVarMap[pCurrentNode->variable]);
             if (pCurrentNode->elseCase == false_ptr)
             {
                // Positive literal.
                arrRHSPolarities[i] = BOOL_TRUE;
                pCurrentNode = pCurrentNode->thenCase;
             }
             else
             {
                // Negative literal.

                if (pCurrentNode->thenCase != false_ptr)
                {
                   cout << endl << "Function: " << nFunctionType << endl;
                   printBDD(pFunc);
                   cout << endl << "EvalRHSFunc: " << nEqualityVble << endl;
                   printBDD(pRHSFunc);
                   assert(0);
                }

                assert(pCurrentNode->thenCase == false_ptr);  
                arrRHSPolarities[i] = BOOL_FALSE;
                pCurrentNode = pCurrentNode->elseCase;
             }
             i++;
          }
       }
       ConstructLemmasForAND(nFnId);
       break;
    default: assert(0); exit(1); break;
   }
   if (nMaxRHSSize < arrSolverFunctions[nFnId].fn_and.rhsVbles.nNumElts)
      nMaxRHSSize = arrSolverFunctions[nFnId].fn_and.rhsVbles.nNumElts;
}

int AndCreateFunction(int nFnId, BDDNode *bdd, int nFnType, int eqVble)
{
   arrSolverFunctions[nFnId].nFnId = nFnId;
   arrSolverFunctions[nFnId].nType = nFnType;
   //arrSolverFunctions[nFnId].nFnPriority = 1;
   BDD2Specfn_AND(nFnId, bdd, nFnType, eqVble);
   arrSolverFunctions[nFnId].fn_and.nNumRHSUnknowns = 
      arrSolverFunctions[nFnId].fn_and.nNumRHSUnknownsNew = 
      arrSolverFunctions[nFnId].fn_and.nNumRHSUnknownsPrev = 
      arrSolverFunctions[nFnId].fn_and.rhsVbles.nNumElts;
   arrSolverFunctions[nFnId].fn_and.nNumLHSUnknowns = 
      arrSolverFunctions[nFnId].fn_and.nNumLHSUnknownsNew = 
      arrSolverFunctions[nFnId].fn_and.nNumLHSUnknownsPrev = 
      (arrSolverFunctions[nFnId].fn_and.nLHSVble == 0?0:1);

   return 0;
}

void AndAffectedVarList(int nFnId, int **arr1, int *num1, int **arr2, int *num2)
{
   *num1 = arrSolverFunctions[nFnId].fn_and.rhsVbles.nNumElts;
   *arr1 = arrSolverFunctions[nFnId].fn_and.rhsVbles.arrElts;
   if (arrSolverFunctions[nFnId].fn_and.nLHSVble != 0)
   {
      *num2 = 1;
      *arr2 = &(arrSolverFunctions[nFnId].fn_and.nLHSVble);
   } else {
      *num2 = 0;
      *arr2 = NULL;
   }

}

void AndCreateAFS(int nFnId, int nVarId, int nAFSIndex)
{
   arrAFS[nVarId].arrOneAFS[nAFSIndex].nFnId = nFnId;
   arrAFS[nVarId].arrOneAFS[nAFSIndex].nType = arrSolverFunctions[nFnId].nType;
   if (arrSolverFunctions[nFnId].fn_and.nLHSVble == nVarId) {
      arrAFS[nVarId].arrOneAFS[nAFSIndex].fn_and.nRHSPos = -1; 
   } else {
      for(int i=0; i<arrSolverFunctions[nFnId].fn_and.rhsVbles.nNumElts; i++) {
         if (arrSolverFunctions[nFnId].fn_and.rhsVbles.arrElts[i] == nVarId) {
            arrAFS[nVarId].arrOneAFS[nAFSIndex].fn_and.nRHSPos = i; 
            break;
         }
      }
   }
}


