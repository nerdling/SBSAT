
#include "sbsat.h"
#include "sbsat_solver.h"
#include "solver.h"

ITE_INLINE int
HrLSGBLemmaInit()
{
   HrLSGBInit();
   HrLemmaInit();

   procHeurUpdate = HrLSGBLemmaUpdate;
   procHeurFree = HrLSGBLemmaFree;
   procHeurSelect = J_OptimizedHeuristic_l;

   return NO_ERROR;
}

ITE_INLINE int
HrLSGBLemmaFree()
{
   HrLSGBFree();
   HrLemmaFree();
   return NO_ERROR;
}

ITE_INLINE int
HrLSGBLemmaUpdate()
{
   HrLSGBUpdate();
   HrLemmaUpdate();
   return NO_ERROR;
}

#define HEUR_WEIGHT(x,i) (x.Pos*x.Neg*(arrLemmaVbleCountsNeg[i]>arrLemmaVbleCountsPos[i]?arrLemmaVbleCountsNeg[i]:arrLemmaVbleCountsPos[i]))

// Var_Score
//#define HEUR_WEIGHT(x,i) (var_score[i] * ((J_ONE+x.Pos) * (J_ONE+x.Neg)) * (J_ONE+arrLemmaVbleCountsNeg[i]>arrLemmaVbleCountsPos[i]?arrLemmaVbleCountsNeg[i]:arrLemmaVbleCountsPos[i]))

#define HEUR_SIGN(nBestVble, multPos, multNeg) \
   (arrHeurScores[nBestVble].Pos*multPos >= arrHeurScores[nBestVble].Neg*multNeg?BOOL_TRUE:BOOL_FALSE)
   //(arrLemmaVbleCountsPos[nBestVble]*arrHeurScores[nBestVble].Pos >= arrLemmaVbleCountsNeg[nBestVble]*arrHeurScores[nBestVble].Neg?BOOL_TRUE:BOOL_FALSE)

#define HEUR_FUNCTION J_OptimizedHeuristic_l
#include "hr_choice.cc"

#undef HEUR_WEIGHT
#undef HEUR_SIGN
#undef HEUR_FUNCTION
