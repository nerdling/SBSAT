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

extern double *arrAndEqWghtCx;
extern double *arrAndEqWghtCe;
extern double *arrAndEqWghtCt;

ITE_INLINE void
LSGBWAndUpdateFunctionInfEnd(int nFnId)
{
   int nOldNumRHSUnknowns = arrSolverFunctions[nFnId].fn_and.nNumRHSUnknownsPrev;
   int nNumRHSUnknowns = arrSolverFunctions[nFnId].fn_and.nNumRHSUnknowns;
   int nOldNumLHSUnknowns = arrSolverFunctions[nFnId].fn_and.nNumLHSUnknownsPrev;
   int nNumLHSUnknowns = arrSolverFunctions[nFnId].fn_and.nNumLHSUnknowns;
   double fOldSumRHSUnknowns = arrSolverFunctions[nFnId].fn_and.fSumRHSUnknownsPrev;
   double fSumRHSUnknowns = arrSolverFunctions[nFnId].fn_and.fSumRHSUnknowns;
   int nLHSVble = arrSolverFunctions[nFnId].fn_and.nLHSVble;

   /* remove heuristic value contribution of this constrains */

   /* figure out if LHS was inferred before this lit
    * in order to subtract its Heu value 
    */
   if (nOldNumLHSUnknowns == 0)
   {
      J_Update_RHS_AND_C(arrSolverFunctions[nFnId].fn_and.rhsVbles.nNumElts, arrSolverFunctions[nFnId].fn_and.rhsVbles.arrElts, arrSolverFunctions[nFnId].fn_and.arrRHSPolarities, 
            // fLastSum, fLastConstPos, fLastMultiPos, fLastConstNeg, fLastMultiNeg, 
            fOldSumRHSUnknowns, 0, arrAndEqWghtCt[nOldNumRHSUnknowns/*-1*/], 0, 0,
            fSumRHSUnknowns, 0, arrAndEqWghtCt[nNumRHSUnknowns/*-1*/], 0, 0);
      return;
   }

   // Handle scores for RHS variables.

   if (nNumLHSUnknowns == 0)
   {
      J_Update_RHS_AND_C(arrSolverFunctions[nFnId].fn_and.rhsVbles.nNumElts, arrSolverFunctions[nFnId].fn_and.rhsVbles.arrElts, arrSolverFunctions[nFnId].fn_and.arrRHSPolarities, 
            // fLastSum, fLastConstPos, fLastMultiPos, fLastConstNeg, fLastMultiNeg, 
            fOldSumRHSUnknowns, arrAndEqWghtCx[nOldNumRHSUnknowns/*-1*/]*arrJWeights[nLHSVble], 
            arrAndEqWghtCe[nOldNumRHSUnknowns/*-1*/], arrJWeights[nLHSVble], 0,
            fSumRHSUnknowns, 0, arrAndEqWghtCt[nNumRHSUnknowns/*-1*/], 0, 0);

      return;
   }

   // prev LHS literal was uninstantiated.
   assert(nLHSVble > 0);

   Save_arrHeurScores(nLHSVble);

   // Handle scores for LHS variable.
   if (arrSolverFunctions[nFnId].fn_and.nLHSPolarity == BOOL_TRUE)
   {
      arrHeurScores[nLHSVble].Pos += fSumRHSUnknowns - fOldSumRHSUnknowns; // the RHS is inferred
      arrHeurScores[nLHSVble].Neg += arrAndEqWghtCt[nNumRHSUnknowns+1]*fSumRHSUnknowns -
                                    arrAndEqWghtCt[nOldNumRHSUnknowns+1]*fOldSumRHSUnknowns; // Transition to Ct
   }
   else
   {
      arrHeurScores[nLHSVble].Pos += arrAndEqWghtCt[nNumRHSUnknowns+1]*fSumRHSUnknowns -
                                    arrAndEqWghtCt[nOldNumRHSUnknowns+1]*fOldSumRHSUnknowns; // Transition to Ct
      arrHeurScores[nLHSVble].Neg += fSumRHSUnknowns - fOldSumRHSUnknowns; // the RHS is inferred
   }
   J_Update_RHS_AND_C(arrSolverFunctions[nFnId].fn_and.rhsVbles.nNumElts, arrSolverFunctions[nFnId].fn_and.rhsVbles.arrElts, arrSolverFunctions[nFnId].fn_and.arrRHSPolarities, 
         // fLastSum, fLastConstPos, fLastMultiPos, fLastConstNeg, fLastMultiNeg, 
            fOldSumRHSUnknowns, arrAndEqWghtCx[nOldNumRHSUnknowns/*-1*/]*arrJWeights[nLHSVble], 
            arrAndEqWghtCe[nOldNumRHSUnknowns/*-1*/], arrJWeights[nLHSVble], 0,
            fSumRHSUnknowns, arrAndEqWghtCx[nNumRHSUnknowns/*-1*/]*arrJWeights[nLHSVble], 
            arrAndEqWghtCe[nNumRHSUnknowns/*-1*/], arrJWeights[nLHSVble], 0);

}

