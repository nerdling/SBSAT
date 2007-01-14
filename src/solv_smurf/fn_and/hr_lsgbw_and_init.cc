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

ITE_INLINE void LSGBWAndGetHeurScores(int nFnId);
ITE_INLINE void LSGBWAndUpdateFunctionInfEnd(int nFnId);

extern int nMaxRHSSize;

void
HrLSGBWFnAndInit()
{
   for(int j=0; arrFnAndTypes[j] != 0; j++)
   {
      int i=arrFnAndTypes[j];
      procHeurGetScores[i] = LSGBWAndGetHeurScores;
      procHeurUpdateFunctionInfEnd[i] = LSGBWAndUpdateFunctionInfEnd;
   }
}


// Call this one after all functions are created (to have nMaxRHSSize)
ITE_INLINE void
LSGBWAndInitHeuristicTables()
   // Initialize some tables for computing heuristic values
   // for variables that are mentioned in special functions.
{

   HWEIGHT K = JHEURISTIC_K;

   // We need nMaxRHSSize to be at least one to insure that entry 1 exists
   // and we don't overrun the arrays.

   arrAndEqWghtCx = (double*)ite_calloc(nMaxRHSSize + 2, sizeof(double), 9, "arrAndEqWghtCx");
   arrAndEqWghtCe = (double*)ite_calloc(nMaxRHSSize + 2, sizeof(double), 9, "arrAndEqWghtCe");
   arrAndEqWghtCt = (double*)ite_calloc(nMaxRHSSize + 2, sizeof(double), 9, "arrAndEqWghtCt");

   arrAndEqWghtCt[1] = 1;
   arrAndEqWghtCx[1] = 1/(2*K);
   arrAndEqWghtCe[1] = 1/(2*K);

   for (int i = 2; i <= nMaxRHSSize; i++)
   {
      arrAndEqWghtCt[i] =  arrAndEqWghtCt[i-1]*(i-1)/(2*i*K);
      arrAndEqWghtCx[i] = (arrAndEqWghtCx[i-1] + 1) * i / (2*K*(i+1));
      arrAndEqWghtCe[i] = (arrAndEqWghtCe[i-1]*(i-1) + arrAndEqWghtCt[i] + 1) / (2*K*(i+1));
   }

   // move it all one up
   for (int i = nMaxRHSSize+1; i> 0; i--)
   {
      arrAndEqWghtCt[i] = arrAndEqWghtCt[i-1];
      arrAndEqWghtCx[i] = arrAndEqWghtCx[i-1];
      arrAndEqWghtCe[i] = arrAndEqWghtCe[i-1];
   }
   arrAndEqWghtCx[1] = 1;
   arrAndEqWghtCe[1] = 0;
   arrAndEqWghtCt[1] = 0;
}

ITE_INLINE void
LSGBWAndFreeHeuristicTables() 
{
   ite_free((void**)&arrAndEqWghtCt);
   ite_free((void**)&arrAndEqWghtCx);
   ite_free((void**)&arrAndEqWghtCe);
}


ITE_INLINE void
LSGBWAndGetHeurScores(int nFnId)
   // Used in initialization of heuristic scores.
   // For the special function with index nFnId,
   // determine all of the uninstantiated variables.
   // Determine the weight which the special function contributes
   // to the positive and negative scores for each such variable.
   // Add these weights into the corresponding arrays.
{
   int nNumRHSUnknowns = arrSolverFunctions[nFnId].fn_and.nNumRHSUnknowns;
   int nLHSVble = arrSolverFunctions[nFnId].fn_and.nLHSVble;
   int nLHSVbleValue = arrSolution[nLHSVble];
   int nLHSPolarity = arrSolverFunctions[nFnId].fn_and.nLHSPolarity;
   int nLHSLitValue = BOOL_UNKNOWN;
   double fSum = 0;

   if (arrAndEqWght == NULL) LSGBWAndInitHeuristicTables();

   for (int j = 0; j < arrSolverFunctions[nFnId].fn_and.rhsVbles.nNumElts; j++) {
      int vble = arrSolverFunctions[nFnId].fn_and.rhsVbles.arrElts[j];
      if (arrSolution[vble]==BOOL_UNKNOWN) 
      {
         if (arrJWeights) fSum += arrJWeights[vble];
      }
   }
   arrSolverFunctions[nFnId].fn_and.fSumRHSUnknowns = 
      arrSolverFunctions[nFnId].fn_and.fSumRHSUnknownsNew = 
      arrSolverFunctions[nFnId].fn_and.fSumRHSUnknownsPrev = fSum;

   if (nLHSVbleValue != BOOL_UNKNOWN)
   {
      nLHSLitValue = nLHSVbleValue == nLHSPolarity ? 
         BOOL_TRUE : BOOL_FALSE;
   }

   if (nLHSLitValue == BOOL_TRUE)
   {
      // We should have closed the inference set before calling this routine.
      assert (nNumRHSUnknowns <= 0);
      return;
   }
   else if (nLHSLitValue == BOOL_FALSE)
   {
      J_Update_RHS_AND_C(arrSolverFunctions[nFnId].fn_and.rhsVbles.nNumElts, arrSolverFunctions[nFnId].fn_and.rhsVbles.arrElts, arrSolverFunctions[nFnId].fn_and.arrRHSPolarities, 
            0, 0, 0, 0, 0, // fLastSum, fLastConstPos, fLastMultiPos, fLastConstNeg, fLastMultiNeg, 
            fSum, 0, arrAndEqWghtCt[nNumRHSUnknowns/*-1*/], 0, 0); // Ct
   }
   else
   {
      // LHS literal is uninstantiated.

      Save_arrHeurScores(nLHSVble);

      // Handle scores for LHS variable.
      if (nLHSPolarity == BOOL_TRUE)
      {
         arrHeurScores[nLHSVble].Pos += fSum; // the RHS is inferred
         arrHeurScores[nLHSVble].Neg += arrAndEqWghtCt[nNumRHSUnknowns+1]*fSum; // Transition to Ct
      }
      else
      {
         arrHeurScores[nLHSVble].Pos += arrAndEqWghtCt[nNumRHSUnknowns+1]*fSum; // Transition to Ct
         arrHeurScores[nLHSVble].Neg += fSum; // the RHS is inferred
      }

      J_Update_RHS_AND_C(arrSolverFunctions[nFnId].fn_and.rhsVbles.nNumElts, arrSolverFunctions[nFnId].fn_and.rhsVbles.arrElts, arrSolverFunctions[nFnId].fn_and.arrRHSPolarities, 
            0, 0, 0, 0, 0, // fLastSum, fLastConstPos, fLastMultiPos, fLastConstNeg, fLastMultiNeg, 
            fSum, arrAndEqWghtCx[nNumRHSUnknowns/*-1*/]*arrJWeights[nLHSVble], arrAndEqWghtCe[nNumRHSUnknowns/*-1*/], 
            arrJWeights[nLHSVble], 0);
   }
}
