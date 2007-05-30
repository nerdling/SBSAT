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

double *arrXorEqWght = NULL;

ITE_INLINE void LSGBXorInitHeuristicTables();
ITE_INLINE void LSGBXorFreeHeuristicTables();

void
HrLSGBFnXorInit()
{
   for(int j=0; arrFnXorTypes[j] != 0; j++)
   {
      int i=arrFnXorTypes[j];
      procHeurGetScores[i] = LSGBXorGetHeurScores;
      procHeurUpdateFunctionInfEnd[i] = LSGBXorUpdateFunctionInfEnd;
   }
}

ITE_INLINE void
LSGBXorInitHeuristicTables()
{
  HWEIGHT K = JHEURISTIC_K;

  // We need nMaxRHSSize to be at least one to insure that entry 1 exists
  // and we don't overrun the arrays.
  int nMaxRHSSize = 1;

  for(int i=0; i<nNumFuncs; i++) 
     if (arrSolverFunctions[i].nType == PLAINXOR || arrSolverFunctions[i].nType == XOR_PART_BDDXOR)
        if (nMaxRHSSize < arrSolverFunctions[i].fn_xor.rhsVbles.nNumElts)
           nMaxRHSSize = arrSolverFunctions[i].fn_xor.rhsVbles.nNumElts;

  arrXorEqWght = (double*)ite_calloc(nMaxRHSSize+1, sizeof(double), 9, "arrXorEqWght");
  arrXorEqWght[0] = 0.0;
  arrXorEqWght[1] = JHEURISTIC_K_TRUE+JHEURISTIC_K_INF; // 1.0;
  for (int i = 2; i <= nMaxRHSSize; i++)
  {
     arrXorEqWght[i] = arrXorEqWght[i-1]/K;
  }
}

ITE_INLINE void
LSGBXorFreeHeuristicTables()
{
  ite_free((void**)&arrXorEqWght);
}

ITE_INLINE void
LSGBXorGetHeurScores(int nFnId)
{
   int nNumRHSUnknowns = arrSolverFunctions[nFnId].fn_xor.nNumRHSUnknowns;

   if (arrXorEqWght == NULL) LSGBXorInitHeuristicTables();

   HWEIGHT fScore = arrXorEqWght[nNumRHSUnknowns-1];
   J_Update_RHS_AND(arrSolverFunctions[nFnId].fn_xor.rhsVbles.nNumElts, arrSolverFunctions[nFnId].fn_xor.rhsVbles.arrElts, arrSolverFunctions[nFnId].fn_xor.arrRHSPolarities, 
         fScore, fScore);
}


ITE_INLINE void
LSGBXorUpdateFunctionInfEnd(int nFnId)
{
   int nOldNumRHSUnknowns = arrSolverFunctions[nFnId].fn_xor.nNumRHSUnknownsPrev;
   int nNumRHSUnknowns = arrSolverFunctions[nFnId].fn_xor.nNumRHSUnknowns;
   //int counter = arrSolverFunctions[nFnId].fn_xor.counter;

	//SEAN!!!
   HWEIGHT fDelta = (nNumRHSUnknowns==0?0:arrXorEqWght[nNumRHSUnknowns-1]) -
      arrXorEqWght[nOldNumRHSUnknowns-1];
   J_Update_RHS_AND(arrSolverFunctions[nFnId].fn_xor.rhsVbles.nNumElts, arrSolverFunctions[nFnId].fn_xor.rhsVbles.arrElts, arrSolverFunctions[nFnId].fn_xor.arrRHSPolarities,
         fDelta, fDelta);
}

