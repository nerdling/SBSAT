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

#include "ite.h"
#include "solver.h"

ITE_INLINE void ConstructLemmasForLongAndEquals(SpecialFunc *pSpecialFunc);


ITE_INLINE void
BDD2Specfn_AND(BDDNodeStruct *pFunc,
      int nFunctionType,
      int nEqualityVble,
      SpecialFunc *pSpecialFunc
      )
{
   assert(nFunctionType == OR || nFunctionType == AND || nFunctionType == PLAINOR);
   int nPolarityOfLHSVble = 0;
   int nNewFunctionType = 0;

   // We assume that special func has no implied literals.
   SFADDONS(pFunc->addons)->pImplied = 0;
   SFADDONS(pFunc->addons)->pReduct = pFunc;

   /* -------------- Left Hand Side -------------- */

   nPolarityOfLHSVble = BOOL_TRUE;
   switch (nFunctionType) {
    case PLAINOR: 
       // Convert PLAINORs to ANDs
       nNewFunctionType = SFN_AND;
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
       nNewFunctionType = SFN_AND;
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
       nNewFunctionType = SFN_AND;
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
   pSpecialFunc->nLHSVble = arrIte2SolverVarMap[nEqualityVble];
   pSpecialFunc->nLHSPolarity = nPolarityOfLHSVble;
   pSpecialFunc->nFunctionType = nNewFunctionType;



   /* -------------- Right Hand Side -------------- */


   /* 0. Create the List of Variables and Polarities */		  

   // Store variable set of RHS.
   // Here we assume that the LHS variable does not also occur on
   // the RHS of the equality.
  
   // FIXME: can do it even better -- if it really is special func
   long tempint_max = 0;
   long y=0;
   unravelBDD(&y, &tempint_max, &pSpecialFunc->rhsVbles.arrElts, pFunc);
   if (y>1) {
      for(int i=0;i<y;i++) {
         if (pSpecialFunc->rhsVbles.arrElts[i] == nEqualityVble) {
            pSpecialFunc->rhsVbles.arrElts[i] = pSpecialFunc->rhsVbles.arrElts[y-1];
            y--;
            break;
         }
      }
      qsort(pSpecialFunc->rhsVbles.arrElts, y, sizeof(int), revcompfunc);
   }
   pSpecialFunc->rhsVbles.nNumElts = y;
   pSpecialFunc->rhsVbles.arrElts = (int*)realloc(pSpecialFunc->rhsVbles.arrElts, pSpecialFunc->rhsVbles.nNumElts*sizeof(int));
  
   
   for (int i=0;i<pSpecialFunc->rhsVbles.nNumElts;i++) {
      pSpecialFunc->rhsVbles.arrElts[i] = arrIte2SolverVarMap[pSpecialFunc->rhsVbles.arrElts[i]];
   }

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

   for (int i = 0; i < pSpecialFunc->rhsVbles.nNumElts; i++)
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
             = EvalBdd(pFunc, nEqualityVble,
                   nPolarityOfLHSVble == BOOL_TRUE ? true : false);
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
             assert(pSpecialFunc->rhsVbles.arrElts[i] == arrIte2SolverVarMap[pCurrentNode->variable]);
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
       ConstructLemmasForLongAndEquals(pSpecialFunc);
       break;
    default: assert(0); exit(1); break;
   }

}

