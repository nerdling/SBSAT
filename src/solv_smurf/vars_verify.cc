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
ConsistentBDDSolution(BDDNodeStruct *pFunc)
{
  int nVble;
  int nValue;

  while (pFunc != true_ptr && pFunc != false_ptr)
    {
      nVble = arrIte2SolverVarMap[pFunc->variable];
      assert(nVble >= 0 && nVble <= gnMaxVbleIndex);
      nValue = arrSolution[nVble];
      if (nValue == BOOL_TRUE)
	{
	  pFunc = pFunc->thenCase;
	}
      else if (nValue == BOOL_FALSE)
	{
	  pFunc = pFunc->elseCase;
	}
      else
	{
	  assert(nValue == BOOL_UNKNOWN);
	  if (ConsistentBDDSolution(pFunc->thenCase)
	      || ConsistentBDDSolution(pFunc->elseCase))
	    {
	      return 1;
	    }
	  else
	    {
	      return 0;
	    }
	}
    }
  return (pFunc == true_ptr ? 1 : 0);
}

ITE_INLINE int
ConsistentPartialSolution()
     // Return 1 if arrSolution[] is a consitent partial solution to
     // the constraints given.  Otherwise return 0.
{
  //int nmbrFunctions = nmbrFunctions;
  //struct BDDNodeStruct **functions = functions;
  int i;
  int nConsistent = 1;

  for (i = 0; i < nmbrFunctions /*&& nConsistent*/; i++)
    {
      if (!ConsistentBDDSolution(functions[i]))
      {
	void printBDDTree(BDDNode *bdd, int *which_zoom);
	cout << "Constraint " << i << " violated:" << endl;
	int which_zoom = 0;
	printBDDTree(functions[i], &which_zoom);
	cout << endl;
	nConsistent = 0;

	/*if (IsSpecialFunc(functionType[i]))
	{
	  DisplaySpecialFunc(arrSpecialFuncs + arrSpecialFuncIndex[i],
			     arrSolution);
	}
	*/
      }
    }

  return nConsistent;
}

ITE_INLINE int
VerifyBDDSolution(BDDNodeStruct *pFunc)
     // Return 1 if arrSolution[] causes *pFunc to evaluate to true.
     // Otherwise return 0.
{
   int nVble;
   int nValue;

   while (pFunc != true_ptr && pFunc != false_ptr)
   {
      nVble = arrIte2SolverVarMap[pFunc->variable];
      assert(nVble >= 0 && nVble <= gnMaxVbleIndex);
      nValue = arrSolution[nVble];
      if (nValue == BOOL_TRUE)
      {
         pFunc = pFunc->thenCase;
      }
      else if (nValue == BOOL_FALSE)
      {
         pFunc = pFunc->elseCase;
      }
      else
      {
         assert(nValue == BOOL_UNKNOWN);
         if (VerifyBDDSolution(pFunc->thenCase)
               && VerifyBDDSolution(pFunc->elseCase))
         {
            return 1;
         }
         else
         {
            return 0;
         }
      }
   }
   return (pFunc == true_ptr ? 1 : 0);
}

ITE_INLINE int
VerifySolution()
     // Return 1 if arrSolution[] is actually a satisfying solution to
     // the constraints given.  Otherwise return 0.
{
  int i;
  int nSatisfied = 1;

  for (i = 0; i < nmbrFunctions /*&& nConsistent*/; i++)
    {
		 if (!VerifyBDDSolution(functions[i])) {
			 void printBDDTree(BDDNode *bdd, int *which_zoom);
			 cout << "Constraint " << i << " violated:" << endl;
			 int which_zoom = 0;
			 printBDDTree(functions[i], &which_zoom);
			 cout << endl;
			 nSatisfied = 0;
			 
	/*
	if (IsSpecialFunc(functionType[i]))
	{
	  DisplaySpecialFunc(arrSpecialFuncs + arrSpecialFuncIndex[i],
			     arrSolution);
	}
	*/
      }
    }

  if (!nSatisfied)
    {
      dE_printf1("ERROR -- Solution verification failed!\n");
    }
  else
    {
      d5_printf1("Solution verified.\n");
    }

  return nSatisfied;
}

