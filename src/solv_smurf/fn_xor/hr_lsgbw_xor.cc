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

double *arrXorEqWghtD = NULL;

ITE_INLINE void LSGBWXorInitHeuristicTables();
ITE_INLINE void LSGBWXorFreeHeuristicTables();

void
HrLSGBWFnXorInit()
{
   for(int j=0; arrFnXorTypes[j] != 0; j++)
   {
      int i=arrFnXorTypes[j];
      procHeurGetScores[i] = LSGBWXorGetHeurScores;
      procHeurUpdateFunctionInfEnd[i] = LSGBWXorUpdateFunctionInfEnd;
   }
}

ITE_INLINE void
LSGBWXorInitHeuristicTables()
{
  HWEIGHT K = JHEURISTIC_K;

  // We need nMaxRHSSize to be at least one to insure that entry 1 exists
  // and we don't overrun the arrays.
  int nMaxRHSSize = 1;

  for(int i=0; i<nNumFuncs; i++) 
     if (arrSolverFunctions[i].nType == PLAINXOR)
        if (nMaxRHSSize < arrSolverFunctions[i].fn_xor.rhsVbles.nNumElts)
           nMaxRHSSize = arrSolverFunctions[i].fn_xor.rhsVbles.nNumElts;

  arrXorEqWght = (double*)ite_calloc(nMaxRHSSize+1, sizeof(double), 9, "arrXorEqWght");
  arrXorEqWghtD = (double*)ite_calloc(nMaxRHSSize+1, sizeof(double), 9, "arrXorEqWghtD");

  arrXorEqWghtD[0] = 0.0; // FIXME: ???
  arrXorEqWghtD[1] = 0.0; // FIXME: ???
  if (sHeuristic[1] == 'm') 
  {
     arrXorEqWght[0] = 0.0;
     for (int i = 1; i <= nMaxRHSSize; i++)
     {
        arrXorEqWght[1] = 1.0 / i;
     }
  } else {
     arrXorEqWght[0] = 0.0;
     arrXorEqWght[1] = JHEURISTIC_K_TRUE+JHEURISTIC_K_INF; // 1.0;
     for (int i = 2; i <= nMaxRHSSize; i++)
     {
        arrXorEqWght[i] = arrXorEqWght[i-1]/K;
        arrXorEqWghtD[i] = arrXorEqWghtD[i-1]*(i-1)/i*K;
     }
  }
}

ITE_INLINE void
LSGBWXorFreeHeuristicTables()
{
  ite_free((void**)&arrXorEqWght);
  ite_free((void**)&arrXorEqWghtD);
}

ITE_INLINE void
LSGBWXorGetHeurScores(int nFnId)
{
   fprintf(stderr, "Xor Error: LSGBW is not ready yet\n");
   exit(1);

//   if (arrXorEqWghtD == NULL) LSGBXorInitHeuristicTables();

//   int nNumRHSUnknowns = arrSolverFunctions[nFnId].fn_xor.nNumRHSUnknowns;
//   double fSum = arrSolverFunctions[nFnId].fn_xor.fSumRHSUnknowns;

//      J_Update_RHS_AND_C(arrSolverFunctions[nFnId].fn_xor.rhsVbles.nNumElts, arrSolverFunctions[nFnId].fn_xor.rhsVbles.arrElts, arrSolverFunctions[nFnId].fn_xor.arrRHSPolarities, 
//            0, 0, 0, 0, 0, // fLastSum, fLastConstPos, fLastMultiPos, fLastConstNeg, fLastMultiNeg, 
//            fSum, 0, arrXorEqWghtD[nNumRHSUnknowns-1], 0, arrXorEqWghtD[nNumRHSUnknowns-1]);
}

ITE_INLINE void
LSGBWXorUpdateFunctionInfEnd(int nFnId)
{
   fprintf(stderr, "Xor Error: LSGBW is not ready yet\n");
   exit(1);

   //int nOldNumRHSUnknowns = arrSolverFunctions[nFnId].fn_xor.nNumRHSUnknownsPrev;
   //int nNumRHSUnknowns = arrSolverFunctions[nFnId].fn_xor.nNumRHSUnknowns;
   //int fOldSumRHSUnknowns = arrSolverFunctions[nFnId].fn_xor.fSumRHSUnknownsPrev;
   //int fSumRHSUnknowns = arrSolverFunctions[nFnId].fn_xor.fSumRHSUnknowns;
   //int counter = arrSolverFunctions[nFnId].fn_xor.counter;
   
//   J_Update_RHS_AND_C(arrSolverFunctions[nFnId].fn_xor.rhsVbles.nNumElts, arrSolverFunctions[nFnId].fn_xor.rhsVbles.arrElts, arrSolverFunctions[nFnId].fn_xor.arrRHSPolarities, 
            // fLastSum, fLastConstPos, fLastMultiPos, fLastConstNeg, fLastMultiNeg, 
//            fOldSumRHSUnknowns, 0, arrXorEqWghtD[nOldNumRHSUnknowns-1], 0, arrXorEqWghtD[nOldNumRHSUnknowns-1],
//            fSumRHSUnknowns, 0, arrXorEqWghtD[nNumRHSUnknowns-1], 0, arrXorEqWghtD[nNumRHSUnknowns-1]);
}
