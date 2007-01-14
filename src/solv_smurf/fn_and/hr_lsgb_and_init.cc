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

struct AndEqFalseWghtStruct *arrAndEqFalseWght = NULL;
struct AndEqWghtStruct *arrAndEqWght = NULL;
double *arrAndEqWghtCx = NULL;
double *arrAndEqWghtCe = NULL;
double *arrAndEqWghtCt = NULL; // lhs is false

void LSGBAndGetHeurScores(int nFnId);

extern int nMaxRHSSize;

void
HrLSGBFnAndInit()
{
   for(int j=0; arrFnAndTypes[j] != 0; j++)
   {
      int i=arrFnAndTypes[j];
      procHeurGetScores[i] = LSGBAndGetHeurScores;
      procHeurUpdateFunctionInfEnd[i] = LSGBAndUpdateFunctionInfEnd;
   }
}


// Call this one after all functions are created (to have nMaxRHSSize)
ITE_INLINE void
LSGBAndInitHeuristicTables()
   // Initialize some tables for computing heuristic values
   // for variables that are mentioned in special functions.
{

   HWEIGHT K = JHEURISTIC_K;

   // We need nMaxRHSSize to be at least one to insure that entry 1 exists
   // and we don't overrun the arrays.

   arrAndEqFalseWght = (AndEqFalseWghtStruct*)ite_calloc(nMaxRHSSize + 1, sizeof(AndEqFalseWghtStruct), 
          9, "arrAndEqFalseWght");

   arrAndEqWght = (AndEqWghtStruct*)ite_calloc(nMaxRHSSize + 1,  sizeof(AndEqWghtStruct),
          9, "arrAndEqWght");

   // index is the CURRENT number of unknowns!
   
   // .fNeg = 0; should be !!! JHEURISTIC_K_TRUE !! for all indeces
   // never happens!
   //arrAndEqFalseWght[1].fPos = 2*JHEURISTIC_K_TRUE+1*JHEURISTIC_K_INF;
   arrAndEqWght[0].fFmla = (1*JHEURISTIC_K_INF+JHEURISTIC_K_TRUE);
   arrAndEqFalseWght[1].fPos = 2*JHEURISTIC_K_TRUE+1*JHEURISTIC_K_INF;
   arrAndEqFalseWght[1].fFmla = arrAndEqFalseWght[1].fPos; ///(2*K);

   for (int i = 2; i <= nMaxRHSSize; i++)
   {
      arrAndEqFalseWght[i].fPos = JHEURISTIC_K_TRUE+arrAndEqFalseWght[i - 1].fFmla;
      arrAndEqFalseWght[i].fFmla = arrAndEqFalseWght[i].fPos / (2 * K);
   }

   // now fFmla is a weight of the state of PLAIN AND

   for (int i = 1; i <= nMaxRHSSize; i++)
   {
      arrAndEqWght[i].fLHSPos = i*JHEURISTIC_K_INF+JHEURISTIC_K_TRUE;
      arrAndEqWght[i].fLHSNeg = arrAndEqFalseWght[i].fFmla;
      arrAndEqWght[i].fRHSPos = arrAndEqWght[i - 1].fFmla;
      arrAndEqWght[i].fRHSNeg = 1*JHEURISTIC_K_INF+JHEURISTIC_K_TRUE;

      // HERE I'm changing fFmla to AND_EQU
      arrAndEqWght[i].fFmla
         = (arrAndEqWght[i].fLHSPos
               + arrAndEqWght[i].fLHSNeg
               + i * arrAndEqWght[i].fRHSPos
               + i * arrAndEqWght[i].fRHSNeg) / (2 * (i+1) * K);
   }
   for(int i=0;i<=nMaxRHSSize;i++)
   {
      d9_printf5("%d: PLAIN AND fFmla %f, fPos %f fNeg %f\n", i, 
            arrAndEqFalseWght[i].fFmla,
            arrAndEqFalseWght[i].fPos,
            arrAndEqFalseWght[i].fNeg);
      d9_printf5("%d: EQ AND fFmla %f, fLHSPos %f fLHSNeg %f ", i, 
            arrAndEqWght[i].fFmla,
            arrAndEqWght[i].fLHSPos,
            arrAndEqWght[i].fLHSNeg);
      d9_printf3("fRHSPos %f fRHSNeg %f\n", 
            arrAndEqWght[i].fRHSPos,
            arrAndEqWght[i].fRHSNeg);
   }
}

ITE_INLINE void
LSGBAndFreeHeuristicTables() 
{
   ite_free((void**)&arrAndEqFalseWght);
   ite_free((void**)&arrAndEqWght);
}


void 
LSGBAndGetHeurScores(int nFnId)
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

   if (arrAndEqWght == NULL) LSGBAndInitHeuristicTables();

   if (nLHSVbleValue != BOOL_UNKNOWN)
   {
      nLHSLitValue = nLHSVbleValue == nLHSPolarity ? 
         BOOL_TRUE : BOOL_FALSE;
   }

   if (nLHSLitValue == BOOL_TRUE)
   {
      // We should have closed the inference set before calling this routine.
      fprintf(stderr, "LHS (%c%d) is true (%c%d)\n", 
            (nLHSPolarity==BOOL_TRUE?'+':'-'), nLHSVble,
            (nLHSVbleValue==BOOL_TRUE?'+':'-'), nLHSVble);
      assert (nNumRHSUnknowns <= 0);
      return;
   }
   else if (nLHSLitValue == BOOL_FALSE)
   {
      HWEIGHT fPosDelta = arrAndEqFalseWght[nNumRHSUnknowns].fPos;
      HWEIGHT fNegDelta = arrAndEqFalseWght[nNumRHSUnknowns].fNeg; // 0

      J_Update_RHS_AND(
            arrSolverFunctions[nFnId].fn_and.rhsVbles.nNumElts, 
            arrSolverFunctions[nFnId].fn_and.rhsVbles.arrElts, 
            arrSolverFunctions[nFnId].fn_and.arrRHSPolarities,
            fPosDelta, fNegDelta);
   }
   else
   {
      // LHS literal is uninstantiated.

      Save_arrHeurScores(nLHSVble);

      // Handle scores for LHS variable.
      if (nLHSPolarity == BOOL_TRUE)
      {
         arrHeurScores[nLHSVble].Pos += arrAndEqWght[nNumRHSUnknowns].fLHSPos;
         arrHeurScores[nLHSVble].Neg += arrAndEqWght[nNumRHSUnknowns].fLHSNeg;
      }
      else
      {
         arrHeurScores[nLHSVble].Pos += arrAndEqWght[nNumRHSUnknowns].fLHSNeg;
         arrHeurScores[nLHSVble].Neg += arrAndEqWght[nNumRHSUnknowns].fLHSPos;
      }

      // Handle scores for RHS variables.
      HWEIGHT fPosDelta = arrAndEqWght[nNumRHSUnknowns].fRHSPos;
      HWEIGHT fNegDelta = arrAndEqWght[nNumRHSUnknowns].fRHSNeg;

      J_Update_RHS_AND(
            arrSolverFunctions[nFnId].fn_and.rhsVbles.nNumElts, 
            arrSolverFunctions[nFnId].fn_and.rhsVbles.arrElts, 
            arrSolverFunctions[nFnId].fn_and.arrRHSPolarities,
            fPosDelta, fNegDelta);
   }
}

