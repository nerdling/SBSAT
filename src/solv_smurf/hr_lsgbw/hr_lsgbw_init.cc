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


/*
 * -H j -- standard Johnson (inference = 1)
 * -H js -- inference weight is the sum of # of smurfs, specfn, lemmas
 * -H jq -- sqared sum
 * -H jr -- squared and scaled
 *
 */

ITE_INLINE void
J_Setup_arrJWeights()
{
   double max=0;
   arrJWeights = (double*)ite_calloc(gnMaxVbleIndex+1, sizeof(double), 2, "arrJWeights");

   for (int nVble = 0; nVble <= gnMaxVbleIndex; nVble++)
   {
      assert(BREAK_XORS == 0);
      long sum = arrAFS[nVble].nNumOneAFS;
      if (arrLemmaVbleCountsPos && arrLemmaVbleCountsNeg) {
         sum += arrLemmaVbleCountsPos[nVble];
         sum += arrLemmaVbleCountsNeg[nVble];
      }
      if (sHeuristic[1] == 's' || sHeuristic[1] == 'S') {
         arrJWeights[nVble] += sum;
      }
      if (sHeuristic[1] == 'S') {
         arrJWeights[nVble] += 1;
      }
      if (sHeuristic[1] == 'q' || sHeuristic[1] == 'Q') {
         arrJWeights[nVble] += sqrt((double)sum);
      }
      if (sHeuristic[1] == 'Q') {
         arrJWeights[nVble] += 1;
      }
      if (sHeuristic[1] == 'r' || sHeuristic[1] == 'R') {
         arrJWeights[nVble] += sqrt((double)sum);
         if (max < arrJWeights[nVble]) max = arrJWeights[nVble];
      }
      if (sHeuristic[1] == 'R') {
         arrJWeights[nVble] += 1;
      }
      if (sHeuristic[2] == 'd') {
         if (nVble <= nIndepVars) arrJWeights[nVble] *= 2;
      }
   }
}
