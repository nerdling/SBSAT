/* =========FOR INTERNAL USE ONLY. NO DISTRIBUTION PLEASE ========== */

/*********************************************************************
 Copyright 1999-2004, University of Cincinnati.  All rights reserved.
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

#include "ite.h"
#include "solver.h"

ITE_INLINE void
BDD2Specfn_XOR(BDDNodeStruct *pFunc,
      int nFunctionType,
      int _nEqualityVble,
      SpecialFunc *pSpecialFunc
      )
{
   assert(/*nFunctionType == XOR ||*/ nFunctionType == PLAINXOR);

   // We assume that special func has no implied literals.
   SFADDONS(pFunc->addons)->pReduct = pFunc;

   /* -------------- Left Hand Side -------------- */

   // the result should be ODD for fn to be sat
   // The function is of the form 1 = l_1 + ... + l_n.
   // LHS should be -FALSE ==> TRUE
   // Store LHS variable. and polarity
   pSpecialFunc->nLHSVble = arrIte2SolverVarMap[0]; /* ALWAYS FALSE VARIABLE */
   pSpecialFunc->nLHSPolarity = BOOL_FALSE;
   pSpecialFunc->nFunctionType = SFN_XOR;

   /* -------------- Right Hand Side -------------- */
   /* 0. Create the List of Variables and Polarities */		  

   // Store variable set of RHS.
   // Here we assume that the LHS variable does not also occur on
   // the RHS of the equality.
   //
  
   // FIXME: can do it even better -- if it really is special func
   long tempint_max = 0;
   long y=0;
   unravelBDD(&y, &tempint_max, &pSpecialFunc->rhsVbles.arrElts, pFunc);
   qsort(pSpecialFunc->rhsVbles.arrElts, y, sizeof(int), revcompfunc);
   pSpecialFunc->rhsVbles.nNumElts = y;
   pSpecialFunc->rhsVbles.arrElts = (int*)realloc(pSpecialFunc->rhsVbles.arrElts, pSpecialFunc->rhsVbles.nNumElts*sizeof(int));


   if (pSpecialFunc->rhsVbles.nNumElts <= 0)
   {
      cout << "Special function found with zero variables "
         << "in right hand side." << endl;
      printBDD(pFunc);
      assert(0);
      exit(1);
   }

   // Determine the polarities of the literals on the RHS of the equation:
   pSpecialFunc->arrRHSPolarities
      = (int *)ite_calloc(pSpecialFunc->rhsVbles.nNumElts, sizeof(int),
            9, "special function polarities");
   int *arrRHSPolarities = pSpecialFunc->arrRHSPolarities;

   /* set polarities and map variables from ite to solver ordering */
   for (int i = 0; i < pSpecialFunc->rhsVbles.nNumElts; i++) {
      pSpecialFunc->rhsVbles.arrElts[i] = 
         arrIte2SolverVarMap[pSpecialFunc->rhsVbles.arrElts[i]];
      arrRHSPolarities[i] = BOOL_UNKNOWN;
   }

   /* 1. Create BDD representing RHS. */
   BDDNodeStruct *pRHSFunc = pFunc;

   /* 2. Traverse the BDD, storing the variable index and polarity  for each literal. */
   BDDNodeStruct *pCurrentNode = pRHSFunc;
   int i = 0; // Index into the array of variables mentioned in the func.
   while (pCurrentNode != true_ptr && pCurrentNode != false_ptr)
   {
      assert(pCurrentNode->variable >= 0);
      if (pSpecialFunc->rhsVbles.arrElts[i] != arrIte2SolverVarMap[pCurrentNode->variable]) {
         printBDD(pRHSFunc);
         assert(0);
      }
      assert(pSpecialFunc->rhsVbles.arrElts[i] == arrIte2SolverVarMap[pCurrentNode->variable]);
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

   ConstructLemmasForXOR(pSpecialFunc);
}

