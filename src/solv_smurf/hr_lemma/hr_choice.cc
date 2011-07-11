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

// sanity checks
#ifndef HEUR_FUNCTION
#error "define HEUR_FUNCTION before including this file"
#endif

#ifndef HEUR_WEIGHT
#error "define HEUR_WEIGHT before including this file"
#endif

#ifndef HEUR_COMPARE
// CLASSIC - MAX = THE BEST
#define HEUR_COMPARE(v, max) (v > max)
#endif

#ifndef HEUR_EXTRA_IN
#define HEUR_EXTRA_IN()
#endif

#ifndef HEUR_EXTRA_OUT
#define HEUR_EXTRA_OUT()
#endif

// REVERSED - MIN = THE BEST
//#define HEUR_COMPARE(v, max) (v < max)


ITE_INLINE int
HEUR_FUNCTION(int *pnBranchAtom, int *pnBranchValue)
{
   int nBestVble = -1;
   int i,j;
   double fMaxWeight = 0.0;
   double fVbleWeight;

   HEUR_EXTRA_IN();

   //
   // SEARCH PREDEFINED SETS FIRST
   // 
   if (arrVarChoiceLevels) {
      int level=0;
      for(level=0;level<nVarChoiceLevelsNum;level++)
      {
         j=0;
         while(arrVarChoiceLevels[level][j] != 0)
         {
            i=arrVarChoiceLevels[level][j];
            if (arrSolution[i] == BOOL_UNKNOWN)
            {
               //if (arrHeurScores[i].Pos == 0 && arrHeurScores[i].Neg == 0) continue;
               nBestVble = i;
               fMaxWeight = HEUR_WEIGHT(arrHeurScores[i], i);
               break;
            }
            j++;
         }
         if (nBestVble >= 0) 
         {
            while(arrVarChoiceLevels[level][j] != 0)
            {
               i=arrVarChoiceLevels[level][j];
               if (arrSolution[i] == BOOL_UNKNOWN)
               {
                  //if (arrHeurScores[i].Pos == 0 && arrHeurScores[i].Neg == 0) continue;
                  fVbleWeight = HEUR_WEIGHT(arrHeurScores[i], i);
                  if (HEUR_COMPARE(fVbleWeight, fMaxWeight))
                  {
                     fMaxWeight = fVbleWeight;
                     nBestVble = i;
                  }
               }
               j++;
            }
            goto ReturnHeuristicResult;
         }
      }
   }

   //
   // SEARCH INDEPENDENT VARIABLES
   //
   // Determine the variable with the highest weight:
   // 
   // Initialize to the lowest indexed variable whose value is uninstantiated.
   for (i = 1; i < nIndepVars+1; i++)
   {
      if (arrSolution[i] == BOOL_UNKNOWN)
      {
        //if (arrHeurScores[i].Pos == 0 && arrHeurScores[i].Neg == 0) continue;
         nBestVble = i;
         fMaxWeight = HEUR_WEIGHT(arrHeurScores[i], i);
         break;
      }
   }

   if (nBestVble >= 0)
   {
      // Search through the remaining uninstantiated independent variables.
      for (i = nBestVble + 1; i < nIndepVars+1; i++)
      {
         if (arrSolution[i] == BOOL_UNKNOWN)
         {
            //if (arrHeurScores[i].Pos == 0 && arrHeurScores[i].Neg == 0) continue;
            fVbleWeight = HEUR_WEIGHT(arrHeurScores[i], i);
            if (HEUR_COMPARE(fVbleWeight, fMaxWeight))
            {
               fMaxWeight = fVbleWeight;
               nBestVble = i;
            }
         }
      }

# ifdef TRACE_HEURISTIC
      cout << endl << "X" << nBestVble << "*: then "
         << arrHeurScores[nBestVble].Pos
         << " else " << arrHeurScores[nBestVble].Neg
         << " -> " << fMaxWeight << "  ";
# endif

      goto ReturnHeuristicResult;
   }

   //
   // SEARCH DEPENDENT VARIABLES
   //
   // Initialize to first uninstantiated variable.
   for (i = nIndepVars+1; i < nIndepVars+1+nDepVars; i++)
   {
      if (arrSolution[i] == BOOL_UNKNOWN)
      {
         //if (arrHeurScores[i].Pos == 0 && arrHeurScores[i].Neg == 0) continue;
         nBestVble = i;
         fMaxWeight = HEUR_WEIGHT(arrHeurScores[i], i);
         break;
      }
   }

   if (nBestVble >= 0)
   {
      // Search through the remaining uninstantiated independent variables.
      for (i = nBestVble + 1; i < nIndepVars+1+nDepVars; i++)
      {
         if (arrSolution[i] == BOOL_UNKNOWN)
         {
            //if (arrHeurScores[i].Pos == 0 && arrHeurScores[i].Neg == 0) continue;
            fVbleWeight = HEUR_WEIGHT(arrHeurScores[i], i);
            if (HEUR_COMPARE(fVbleWeight, fMaxWeight))
            {
               fMaxWeight = fVbleWeight;
               nBestVble = i;
            }
         }
      }

#ifdef TRACE_HEURISTIC
      cout << endl << "X" << nBestVble << ": then "
         << arrHeurScores[nBestVble].Pos
         << " else " << arrHeurScores[nBestVble].Neg
         << " -> " << fMaxWeight << "  ";
#endif
      goto ReturnHeuristicResult;
   }

   //
   // SEARCH TEMP VARIABLES
   //
   // Initialize to first uninstantiated variable.
   for (i = nIndepVars+1+nDepVars; i < nNumVariables; i++)
   {
      if (arrSolution[i] == BOOL_UNKNOWN)
      {
         nBestVble = i;
         fMaxWeight = HEUR_WEIGHT(arrHeurScores[i], i);
         break;
      }
   }

   if (nBestVble >= 0)
   {
      // Search through the remaining uninstantiated independent variables.
      for (i = nBestVble + 1; i < nNumVariables; i++)
      {
         if (arrSolution[i] == BOOL_UNKNOWN)
         {
            fVbleWeight = HEUR_WEIGHT(arrHeurScores[i], i);
				if (HEUR_COMPARE(fVbleWeight, fMaxWeight))
            {
               fMaxWeight = fVbleWeight;
               nBestVble = i;
            }
         }
      }

      goto ReturnHeuristicResult;
   }
   else
   {
      dE_printf1 ("Error in heuristic routine:  No uninstantiated variable found\n");
      exit (1);
   }

ReturnHeuristicResult:
   assert (arrSolution[nBestVble] == BOOL_UNKNOWN);
   *pnBranchAtom = nBestVble;
   if (arrVarTrueInfluences)
      *pnBranchValue = HEUR_SIGN(nBestVble, arrVarTrueInfluences[nBestVble], (1-arrVarTrueInfluences[nBestVble]));
   else
      *pnBranchValue = HEUR_SIGN(nBestVble, 1, 1);

   HEUR_EXTRA_OUT();

   return NO_ERROR;
}

