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

double *arrJWeights;

ITE_INLINE SmurfState * GetSmurfState(int i);
ITE_INLINE void J_SetupHeuristicScores();
ITE_INLINE void J_Setup_arrJWeights();
ITE_INLINE void GetHeurScoresFromSmurf(int i);
ITE_INLINE void J_ResetHeuristicScores();

ITE_INLINE int
HrLSGBWInit()
{
   InitHeurScoresStack();
   J_Setup_arrJWeights();

   procHeurBacktrack = HrLSGBBacktrack;
   procHeurUpdate = HrLSGBUpdate;
   procHeurFree = HrLSGBWFree;

   HrLSGBWFnSmurfInit();
   HrLSGBWFnAndInit();

   for (int i = 0; i < nNumVariables; i++)
   {
      arrHeurScores[i].Pos = arrHeurScores[i].Neg = 0; //c_heur;
   }
/*
   for (int nType = 0; nType< nNumTypes; nType++)
   {
      if (procHeurTypeInit[nType]) procHeurTypeInit[nType]();
   }
*/
   for(int nFnId=0;nFnId<nNumFuncs;nFnId++)
   {
      d9_printf2("Set Heur Scores for %d\n", nFnId);
      if (procHeurGetScores[arrSolverFunctions[nFnId].nType]) 
         procHeurGetScores[arrSolverFunctions[nFnId].nType](nFnId);
   }
   /*
      if (nheuristic == johnson_heuristic) {
         assert(arrjweights);
         for (int i = 0; i < nnumspecialfuncs; i++) {
            for (int j=0; j<arrspecialfuncs[i].rhsvbles.nnumelts; j++)
               arrsumrhsunknowns[i] += arrjweights[arrspecialfuncs[i].rhsvbles.arrelts[j]];
         }
      }
   
   // Set up the Special Func Stack.
      for (int i = 0; i < nNumSpecialFuncs; i++) {
         arrPrevNumRHSUnknowns[i] =
         arrNumRHSUnknownsNew[i] =
         arrNumRHSUnknowns[i] = arrSpecialFuncs[i].rhsVbles.nNumElts;
         arrPrevNumLHSUnknowns[i] =
         arrNumLHSUnknownsNew[i] =
         arrNumLHSUnknowns[i] = arrSpecialFuncs[i].nLHSVble > 0? 1: 0;
         arrSumRHSUnknowns[i] = 0;
         arrPrevRHSCounter[i] =
         arrRHSCounterNew[i] =
         arrRHSCounter[i] = 0;
         assert(arrSolution[0]!=BOOL_UNKNOWN);
      }
   

*/
   procHeurSelect = J_OptimizedHeuristic;

   D_9(
         DisplayHeuristicValues();
      );
   return 0;
}

ITE_INLINE int
HrLSGBWFree()
{
   FreeHeurScoresStack();
   return 0;
}

// Update scores of RHS variables.
ITE_INLINE void
J_Update_RHS_AND_C(int nNumRHSUnknowns, int *arrRHSVbles, int *arrRHSPolarities, 
     double fLastSum, double fLastConstPos, double fLastMultiPos, double fLastConstNeg, double fLastMultiNeg, 
     double fSum, double fConstPos, double fMultiPos, double fConstNeg, double fMultiNeg)
{
   int nRHSVble;

   d9_printf4("J_Update_RHS_AND_C(pSpecialFunc, \n fLastSum=%f, fLastConstPos=%f, fLastMultiPos=%f, ", 
         fLastSum, fLastConstPos, fLastMultiPos);
   d9_printf3("fLastConstNeg=%f, fLastMultiNeg=%f,\n ", fLastConstNeg, fLastMultiNeg);
   d9_printf4("fSum=%f, fConstPos=%f, fMultiPos=%f, ", fSum, fConstPos, fMultiPos);
   d9_printf3("fConstNeg=%f, fMultiNeg=%f)\n", fConstNeg, fMultiNeg);

   for (int i = 0; i < nNumRHSUnknowns; i++)
   {
      nRHSVble = arrRHSVbles[i];
      assert(nRHSVble>0);
      if (arrSolution[nRHSVble] == BOOL_UNKNOWN)
      { 
         Save_arrHeurScores(nRHSVble);
         HWEIGHT fPosDelta = fConstPos - fLastConstPos + 
            (fSum-arrJWeights[nRHSVble])*fMultiPos - 
            (fLastSum-arrJWeights[nRHSVble])*fLastMultiPos;
         HWEIGHT fNegDelta = fConstNeg - fLastConstNeg + 
            (fSum-arrJWeights[nRHSVble])*fMultiNeg - 
            (fLastSum-arrJWeights[nRHSVble])*fLastMultiNeg;
         if (arrRHSPolarities[i] == BOOL_TRUE)
         {
            // Variable polarity is positive.
            arrHeurScores[nRHSVble].Pos += fPosDelta;
            arrHeurScores[nRHSVble].Neg += fNegDelta;
         }
         else
         {
            // Variable polarity is negative.
            arrHeurScores[nRHSVble].Pos += fNegDelta;
            arrHeurScores[nRHSVble].Neg += fPosDelta;
         }
      }
       //else
       //{
       //arrHeurScores[nRHSVble].Pos = 0;
       //arrHeurScores[nRHSVble].Neg = 0;
       //}
   } // for each RHS vble
}

