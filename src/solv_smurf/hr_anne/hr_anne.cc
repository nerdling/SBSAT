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

t_arrVarScores *arrAnneVarScores = NULL;

ITE_INLINE int
HrAnneInit()
{
   InitAnneHeurArrays(gnMaxVbleIndex);

   procHeurUpdate = HrAnneUpdate;
   procHeurFree = HrAnneFree;
   procHeurSelect = Anne_OptimizedHeuristic;
   procHeurAddLemma = AddAnneHeuristicInfluence;
   procHeurAddLemmaSpace = AddAnneSpaceHeuristicInfluence;
   procHeurRemoveLemma = RemoveAnneHeuristicInfluence;

   arrAnneVarScores = (t_arrVarScores*)ite_calloc(gnMaxVbleIndex, 
         sizeof(t_arrVarScores), 9, "arrAnneVarScores");

/*
   for(int i=0;i<nNumFuncs;i++)
   {
      for(int j=0;arrFnAndTypes[j];j++) {
         if (arrSolverFunctions[i].nType == arrFnAndTypes[j]) {
            AddAnneBlockHeuristicInfluence(arrSolverFunctions[i].fn_and.pLongLemma);

            for(int k=0;k<arrSolverFunctions[i].fn_and.rhsVbles.nNumElts;k++)
               AddAnneBlockHeuristicInfluence(arrSolverFunctions[i].fn_and.arrShortLemmas[k]);

            break;
         }
      }
   }
*/
 
   for(int i=1;i<gnMaxVbleIndex;i++)
   {
      arrLemmaVbleCountsPos[i] += arrAFS[i].nNumOneAFS;
      arrLemmaVbleCountsNeg[i] += arrAFS[i].nNumOneAFS;
   }

   //return HrAnneUpdate();
   return NO_ERROR;
}

ITE_INLINE int
HrAnneFree()
{
   ite_free((void**)&arrAnneVarScores);
   DeleteAnneHeurArrays();
   return NO_ERROR;
}

ITE_INLINE int
HrAnneUpdate()
{
   if (ite_counters[NUM_BACKTRACKS] % 255 == 0)
   {
      for (int i = 1; i<gnMaxVbleIndex; i++)
      {
         d9_printf4("%d: (pos count = %d, neg count = %d)\n", i, arrLemmaVbleCountsPos[i], arrLemmaVbleCountsNeg[i]);
         arrAnneVarScores[i].pos = arrAnneVarScores[i].pos/2 + 
            arrLemmaVbleCountsPos[i] - arrAnneVarScores[i].last_count_pos;
         arrAnneVarScores[i].neg = arrAnneVarScores[i].neg/2 + 
            arrLemmaVbleCountsNeg[i] - arrAnneVarScores[i].last_count_neg;
         arrAnneVarScores[i].last_count_pos = arrLemmaVbleCountsPos[i];
         arrAnneVarScores[i].last_count_neg = arrLemmaVbleCountsNeg[i];
      }
   }
   return NO_ERROR;
}


#define J_ONE 1
#define HEUR_WEIGHT(x,i) (arrLemmaVbleCountsPos[i] > arrLemmaVbleCountsNeg[i] ?  arrLemmaVbleCountsPos[i] : arrLemmaVbleCountsNeg[i]);

//Var Score
//#define HEUR_WEIGHT(x,i) (var_score[i] * (J_ONE+arrLemmaVbleCountsNeg[i]>arrLemmaVbleCountsPos[i]?arrLemmaVbleCountsNeg[i]:arrLemmaVbleCountsPos[i]))

//#define HEUR_EXTRA_OUT()  { fprintf(stderr, "%c%d (pos: %d, neg %d)\n", (*pnBranchValue==BOOL_TRUE?'+':'-'), *pnBranchAtom, arrLemmaVbleCountsPos[*pnBranchAtom], arrLemmaVbleCountsNeg[*pnBranchAtom]);}
#define HEUR_FUNCTION Anne_OptimizedHeuristic
#define HEUR_SIGN(nBestVble, multPos, multNeg) \
  (arrLemmaVbleCountsPos[nBestVble]*multPos > arrLemmaVbleCountsNeg[nBestVble]*multNeg?\
   BOOL_TRUE:BOOL_FALSE)
#include "hr_choice.cc"

#undef HEUR_WEIGHT
#undef HEUR_SIGN
#undef HEUR_FUNCTION
