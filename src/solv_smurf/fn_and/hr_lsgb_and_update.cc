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

extern struct AndEqFalseWghtStruct *arrAndEqFalseWght;
extern struct AndEqWghtStruct *arrAndEqWght;
extern double *arrAndEqWghtCx;
extern double *arrAndEqWghtCe;
extern double *arrAndEqWghtCt;

ITE_INLINE void
LSGBAndUpdateFunctionInfEnd(int nFnId)
{
   int nOldNumRHSUnknowns = arrSolverFunctions[nFnId].fn_and.nNumRHSUnknownsPrev;
   int nNumRHSUnknowns = arrSolverFunctions[nFnId].fn_and.nNumRHSUnknowns;
   int nOldNumLHSUnknowns = arrSolverFunctions[nFnId].fn_and.nNumLHSUnknownsPrev;
   int nNumLHSUnknowns = arrSolverFunctions[nFnId].fn_and.nNumLHSUnknowns;

   HWEIGHT fPosDelta;
   HWEIGHT fNegDelta;

   /* remove heuristic value contribution of this constrains */

   /* figure out if LHS was inferred before this lit
    * in order to subtract its Heu value 
    */
   if (nOldNumLHSUnknowns == 0)
   {
      fPosDelta = arrAndEqFalseWght[nOldNumRHSUnknowns].fPos * -1;
      fNegDelta = arrAndEqFalseWght[nOldNumRHSUnknowns].fNeg * -1;
      fPosDelta += arrAndEqFalseWght[nNumRHSUnknowns].fPos;
      fNegDelta += arrAndEqFalseWght[nNumRHSUnknowns].fNeg;
      J_Update_RHS_AND(arrSolverFunctions[nFnId].fn_and.rhsVbles.nNumElts, arrSolverFunctions[nFnId].fn_and.rhsVbles.arrElts, arrSolverFunctions[nFnId].fn_and.arrRHSPolarities,
            fPosDelta, fNegDelta);
      return;
   }

   // Handle scores for RHS variables.
   fPosDelta = arrAndEqWght[nOldNumRHSUnknowns].fRHSPos * -1;
   fNegDelta = arrAndEqWght[nOldNumRHSUnknowns].fRHSNeg * -1;

   if (nNumLHSUnknowns == 0)
   {
      fPosDelta += arrAndEqFalseWght[nNumRHSUnknowns].fPos;
      fNegDelta += arrAndEqFalseWght[nNumRHSUnknowns].fNeg;
      J_Update_RHS_AND(arrSolverFunctions[nFnId].fn_and.rhsVbles.nNumElts, arrSolverFunctions[nFnId].fn_and.rhsVbles.arrElts, arrSolverFunctions[nFnId].fn_and.arrRHSPolarities,
            fPosDelta, fNegDelta);
      return;
   }

   // prev LHS literal was uninstantiated.
   int nLHSVble = arrSolverFunctions[nFnId].fn_and.nLHSVble;
   assert(nLHSVble > 0);

   Save_arrHeurScores(nLHSVble);

   // Handle scores for LHS variable.
   if (arrSolverFunctions[nFnId].fn_and.nLHSPolarity == BOOL_TRUE)
   {
      arrHeurScores[nLHSVble].Pos += arrAndEqWght[nNumRHSUnknowns].fLHSPos -
                                    arrAndEqWght[nOldNumRHSUnknowns].fLHSPos;
      arrHeurScores[nLHSVble].Neg += arrAndEqWght[nNumRHSUnknowns].fLHSNeg -
                                    arrAndEqWght[nOldNumRHSUnknowns].fLHSNeg;
   }
   else
   {
      arrHeurScores[nLHSVble].Pos += arrAndEqWght[nNumRHSUnknowns].fLHSNeg -
                                    arrAndEqWght[nOldNumRHSUnknowns].fLHSNeg;
      arrHeurScores[nLHSVble].Neg += arrAndEqWght[nNumRHSUnknowns].fLHSPos -
                                    arrAndEqWght[nOldNumRHSUnknowns].fLHSPos;
   }

   fPosDelta += arrAndEqWght[nNumRHSUnknowns].fRHSPos;
   fNegDelta += arrAndEqWght[nNumRHSUnknowns].fRHSNeg;

   J_Update_RHS_AND(arrSolverFunctions[nFnId].fn_and.rhsVbles.nNumElts, arrSolverFunctions[nFnId].fn_and.rhsVbles.arrElts, arrSolverFunctions[nFnId].fn_and.arrRHSPolarities,
         fPosDelta, fNegDelta);
}


