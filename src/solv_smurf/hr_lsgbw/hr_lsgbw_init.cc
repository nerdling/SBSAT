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
