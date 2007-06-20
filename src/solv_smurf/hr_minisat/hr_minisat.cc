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

t_arrVarScores *arrMiniSatVarScores = NULL;

ITE_INLINE int
HrMiniSatInit()
{
   InitMiniSatHeurArrays(gnMaxVbleIndex);

   procHeurUpdate = HrMiniSatUpdate;
   procHeurFree = HrMiniSatFree;
   procHeurSelect = MiniSat_OptimizedHeuristic;
   procHeurAddLemma = AddMiniSatHeuristicInfluence;
   procHeurAddLemmaSpace = AddMiniSatSpaceHeuristicInfluence;
   procHeurRemoveLemma = RemoveMiniSatHeuristicInfluence;

   arrMiniSatVarScores = (t_arrVarScores*)ite_calloc(gnMaxVbleIndex, 
         sizeof(t_arrVarScores), 9, "arrMiniSatVarScores");
/*
   for(int i=0;i<nNumFuncs;i++)
   {
      for(int j=0;arrFnAndTypes[j];j++) {
         if (arrSolverFunctions[i].nType == arrFnAndTypes[j]) {
            AddMiniSatBlockHeuristicInfluence(arrSolverFunctions[i].fn_and.pLongLemma);

            for(int k=0;k<arrSolverFunctions[i].fn_and.rhsVbles.nNumElts;k++)
               AddMiniSatBlockHeuristicInfluence(arrSolverFunctions[i].fn_and.arrShortLemmas[k]);

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
   //return HrMiniSatUpdate();
   return NO_ERROR;
}

ITE_INLINE int
HrMiniSatFree()
{
   ite_free((void**)&arrMiniSatVarScores);
   DeleteMiniSatHeurArrays();
   return NO_ERROR;
}

ITE_INLINE int
HrMiniSatUpdate()
{
  //divide counters -- decay 5%
  for(int i = 1; i < gnMaxVbleIndex; i++){
    //arrMiniSatVarScores[i].pos = arrMiniSatVarScores[i].pos / 4;
    //arrMiniSatVarScores[i].neg = arrMiniSatVarScores[i].neg / 4;
    arrLemmaVbleCountsPos[i] = arrLemmaVbleCountsPos[i] - (arrLemmaVbleCountsPos[i] / 20);
    arrLemmaVbleCountsNeg[i] = arrLemmaVbleCountsNeg[i] - (arrLemmaVbleCountsNeg[i] / 20);
  }


  for (int i = 1; 0 && i<gnMaxVbleIndex; i++){
  
    d9_printf4("%d: (pos count = %d, neg count = %d)\n", i, arrLemmaVbleCountsPos[i], arrLemmaVbleCountsNeg[i]);
    
    //if the variable is positive, increment .pos
    arrMiniSatVarScores[i].last_count_pos = arrMiniSatVarScores[i].pos;
    arrMiniSatVarScores[i].pos = arrLemmaVbleCountsPos[i];
    arrMiniSatVarScores[i].last_count_neg = arrMiniSatVarScores[i].neg;
    arrMiniSatVarScores[i].neg = arrLemmaVbleCountsNeg[i];

  }
  
  return NO_ERROR;
}



#define HEUR_WEIGHT(x,i) (arrLemmaVbleCountsPos[i] > arrLemmaVbleCountsNeg[i] ?  arrLemmaVbleCountsPos[i] : arrLemmaVbleCountsNeg[i]);

#define HEUR_FUNCTION MiniSat_OptimizedHeuristic

#define HEUR_SIGN(nBestVble, multPos, multNeg) \
  (arrLemmaVbleCountsPos[nBestVble] > arrLemmaVbleCountsNeg[nBestVble] ? \
   BOOL_TRUE:BOOL_FALSE)
#include "hr_choice.cc"


#undef HEUR_WEIGHT
#undef HEUR_SIGN
#undef HEUR_FUNCTION
