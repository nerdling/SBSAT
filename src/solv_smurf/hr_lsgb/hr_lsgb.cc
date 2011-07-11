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

ITE_INLINE SmurfState * GetSmurfState(int i);
ITE_INLINE void J_SetupHeuristicScores();
ITE_INLINE void GetHeurScoresFromSmurf(int i);

ITE_INLINE int
HrLSGBUpdate()
{
   // Push heuristic scores.
   PushHeuristicScores();
   return 0;
}

ITE_INLINE int
HrLSGBBacktrack(int n)
{
   PopHeuristicScores(n);
   return 0;
}

ITE_INLINE int
HrLSGBInit()
{
   InitHeurScoresStack();

   procHeurBacktrack = HrLSGBBacktrack;
   procHeurUpdate = HrLSGBUpdate;
   procHeurFree = HrLSGBFree;

   HrLSGBFnSmurfInit();
   HrLSGBFnSmurfXorInit();
   HrLSGBFnAndInit();
   HrLSGBFnXorInit();
   HrLSGBFnXorSmurfInit();
   HrLSGBFnMinMaxInit();

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
   //proc_update_heuristic = J_UpdateHeuristic;
   if (sHeuristic[1] == 'l') {
      /*
      int i = strlen(sHeuristic);
      if (i>8) i=8;
      memmove(sHeuristic+1, sHeuristic+2, i-1); 
      procHeurSelect = J_OptimizedHeuristic_l;
      */
      procHeurSelect = J_OptimizedHeuristic;
   } else {
      procHeurSelect = J_OptimizedHeuristic;
   }
   D_9(
         DisplayHeuristicValues();
      );
   return 0;
}

ITE_INLINE int
HrLSGBFree()
{
   FreeHeurScoresStack();
   return 0;
}


#define J_ONE 1

// CLASSIC
#define HEUR_WEIGHT(x,i) (J_ONE+x.Pos) * (J_ONE+x.Neg)
//#define HEUR_WEIGHT(x,i) (x.Pos*x.Neg+arrVarScores[i].neg>arrVarScores[i].pos?arrVarScores[i].neg:arrVarScores[i].pos)
//#define HEUR_WEIGHT(x,i) (arrVarScores[i].neg>arrVarScores[i].pos?arrVarScores[i].neg:arrVarScores[i].pos)

// Var_Score
//#define HEUR_WEIGHT(x,i) (var_score[i] * ((J_ONE+x.Pos) * (J_ONE+x.Neg)))

// ABSOLUTE MAXIMUM
//#define HEUR_WEIGHT(x,i) (x.Neg > x.Pos ? x.Neg : x.Pos)

// BERM
//#define HEUR_WEIGHT(x,i) (x.Neg>x.Pos?((x.Pos*2) + x.Neg):((x.Neg*2) + x.Pos)) 

// ADDITION
//#define HEUR_WEIGHT(x,i) (J_ONE+x.Pos) + (J_ONE+x.Neg)

// NEW??
//#define HEUR_WEIGHT(x,i) (((x.nPosInfs+1)*x.nPos)* (x.nNegInfs+1)*x.nNeg)
//#define HEUR_WEIGHT(x,i) (J_ONE+x.Pos) * (J_ONE+x.Neg) * (arrLemmaVbleCountsPos[i] + arrLemmaVbleCountsNeg[i])

// slider_80_unsat -- the order from the best to worst is (all J_ONE = 0)
// CLASSIC
// BERM
// ADDITION
// ABSOLUTE MAXIMUM

// CLASSIC - MAX = THE BEST
#define HEUR_COMPARE(v, max) (v > max)

// REVERSED - MIN = THE BEST
//#define HEUR_COMPARE(v, max) (v < max)

#define HEUR_FUNCTION J_OptimizedHeuristic

//#define MK_J_HEURISTIC_LEMMA
#ifdef MK_J_HEURISTIC_LEMMA
   /* this needs a variable with the UnitLemmaList from the brancher */
   /* used to be global pUnitLemmaList */
// #define HEUR_EXTRA_IN
   ITE_INLINE void 
   J_Heuristic_Lemma(LemmaInfoStruct **p, int *nInferredAtom, int *nInferredValue);
   if (pUnitLemmaList->pNextLemma[0] && pUnitLemmaList->pNextLemma[0]->pNextLemma[0])  {
      //fprintf(stderr, "x");
      J_Heuristic_Lemma(&(pUnitLemmaList->pNextLemma[0]), pnBranchAtom, pnBranchValue);
      if (*pnBranchAtom) { fprintf(stderr, "."); return; }
   }
#endif

#ifdef HEUR_EXTRA_IN
#undef HEUR_EXTRA_IN
#endif

#ifdef HEUR_EXTRA_OUT
#undef HEUR_EXTRA_OUT
#endif

#define HEUR_EXTRA_IN() D_8(DisplayHeuristicValues();); d8_printf1("\n");
#define HEUR_EXTRA_OUT() \
   d8_printf6("JHeuristic: %c%d (%.10f,%.10f) because of %f\n",  \
         (*pnBranchValue==BOOL_TRUE?'+':'-'), \
         nBestVble,  \
         arrHeurScores[nBestVble].Pos, \
         arrHeurScores[nBestVble].Neg, \
         fMaxWeight);

#define HEUR_SIGN(nBestVble, multPos, multNeg) \
   (arrHeurScores[nBestVble].Pos*multPos >= arrHeurScores[nBestVble].Neg*multNeg?BOOL_TRUE:BOOL_FALSE)
//#define HEUR_SIGN(nBestVble) (BOOL_TRUE)

#include "hr_choice.cc"

#undef HEUR_WEIGHT
#undef HEUR_COMPARE
#undef HEUR_SIGN
#undef HEUR_FUNCTION
#undef HEUR_EXTRA_IN
#undef HEUR_EXTRA_OUT

/*
#define HEUR_WEIGHT(x,i) (x.Pos*x.Neg*(arrLemmaVbleCountsNeg[i]>arrLemmaVbleCountsPos[i]?arrLemmaVbleCountsNeg[i]:arrLemmaVbleCountsPos[i]))

// Var_Score
//#define HEUR_WEIGHT(x,i) (var_score[i] * ((J_ONE+x.Pos) * (J_ONE+x.Neg)) * (J_ONE+arrLemmaVbleCountsNeg[i]>arrLemmaVbleCountsPos[i]?arrLemmaVbleCountsNeg[i]:arrLemmaVbleCountsPos[i]))

#define HEUR_SIGN(nBestVble, multPos, multNeg) \
   (arrHeurScores[nBestVble].Pos*multPos >= arrHeurScores[nBestVble].Neg*multNeg?BOOL_TRUE:BOOL_FALSE)
   //(arrLemmaVbleCountsPos[nBestVble]*arrHeurScores[nBestVble].Pos >= arrLemmaVbleCountsNeg[nBestVble]*arrHeurScores[nBestVble].Neg?BOOL_TRUE:BOOL_FALSE)

#define HEUR_FUNCTION J_OptimizedHeuristic_l
#include "hr_choice.cc"

//#define MK_TEST_ORDER_HEU
#ifdef MK_TEST_ORDER_HEU
int order[] = {89,90,88,91,87,92,86,93,85,94,84,95,83,96,82,97,81,98,80,99,79,100,78,101,77,102,76,103,75,104,74,105,73,106,72,107,71,108,70,109,69,110,68,111,67,112,66,113,65,114,64,115,63,116,62,117,61,118,60,119,59,120,58,121,57,122,56,123,55,124,54,125,53,126,52,127,51,128,50,129,49,130,48,131,47,132,46,133,45,134,44,135,43,136,42,137,41,138,40,139,39,140,38,141,37,142,36,143,35,144,34,145,33,146,32,147,31,148,30,149,29,150,28,151,27,152,26,153,25,154,24,155,23,156,22,157,21,158,20,159,19,160,18,161,17,162,16,163,15,164,14,165,13,166,12,167,11,168,10,169,9,170,8,171,7,172,6,173,5,174,4,175,3,176,2,177,1,178};

void
J_OptimizedHeuristic(int *pnBranchAtom, int *pnBranchValue)
{
   
   int i=1;
   for(i=1;i<177;i++) {
      int var = order[i];
      if (var < nNumVariables && arrSolution[var] == BOOL_UNKNOWN) {
         *pnBranchAtom = var;
         *pnBranchValue = BOOL_TRUE;
      }
   }
   
   return J_OptimizedHeuristic_(pnBranchAtom, pnBranchValue);
}
#endif
  
ITE_INLINE void 
J_Heuristic_Lemma(LemmaInfoStruct **ppUnitLemmaList, int *nInferredAtom, int *nInferredValue)
{
   double fMaxWeight = 0;
   double fVbleWeight = 0;
   int nBestVble = 0;
   for (LemmaInfoStruct *p = (*ppUnitLemmaList)->pNextLemma[0]; p; p = p->pNextLemma[0])
   {
      //fprintf(stderr, "#");
      LemmaBlock *pLemmaBlock = p->pLemma;
      int *arrLits = pLemmaBlock->arrLits;
      int nLemmaLength = arrLits[0];

      for (int nLitIndex = 1, nLitIndexInBlock = 1;
            nLitIndex <= nLemmaLength;
            nLitIndex++, nLitIndexInBlock++)
      {
         if (nLitIndexInBlock == LITS_PER_LEMMA_BLOCK)
         {
            nLitIndexInBlock = 0;
            pLemmaBlock = pLemmaBlock->pNext;
            arrLits = pLemmaBlock->arrLits;
         }
         int i = abs(arrLits[nLitIndexInBlock]);
         if (arrSolution[i] == BOOL_UNKNOWN)
         {
            fprintf(stderr, "?");
            fVbleWeight = HEUR_WEIGHT(arrHeurScores[i],i);
            if (HEUR_COMPARE(fVbleWeight, fMaxWeight))
            {
               fMaxWeight = fVbleWeight;
               nBestVble = i;
            }
         }
      }
   }
   if (nBestVble != 0) 
   {
      *nInferredAtom = nBestVble;
      *nInferredValue = HEUR_SIGN(nBestVble);
   }
   *ppUnitLemmaList = NULL;
}
#undef HEUR_WEIGHT
#undef HEUR_COMPARE
#undef HEUR_SIGN
#undef HEUR_FUNCTION
*/

// Update scores of RHS variables.
ITE_INLINE void
J_Update_RHS_AND(int nNumRHSUnknowns, int *arrRHSVbles, int *arrRHSPolarities, HWEIGHT fPosDelta, HWEIGHT fNegDelta)
{
   int nRHSVble;

   d9_printf4("J_Update_RHS_AND(nNumRHSUnknowns %d, arrRHSVbles, arrPolarities, fPos=%f, fNeg=%f\n", 
         nNumRHSUnknowns, fPosDelta, fNegDelta);

   for (int i = 0; i < nNumRHSUnknowns; i++)
   {
      nRHSVble = arrRHSVbles[i];
      assert(nRHSVble>0);
      if (arrSolution[nRHSVble] == BOOL_UNKNOWN)
      {
         Save_arrHeurScores(nRHSVble);
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

ITE_INLINE void
DisplayHeuristicValues()
{
   fprintf(stddbg, "JHeuristic values: \n");
   for(int i=0;i<gnMaxVbleIndex;i++)
   {
      if (arrSolution[i]==BOOL_UNKNOWN) {
         fprintf(stddbg, "+%d(%d): %f%c\n", i, arrSolver2IteVarMap[i], arrHeurScores[i].Pos, arrSolution[i]!=BOOL_UNKNOWN?'*':' ');
         fprintf(stddbg, "-%d(%d): %f%c\n", i, arrSolver2IteVarMap[i], arrHeurScores[i].Neg, arrSolution[i]!=BOOL_UNKNOWN?'*':' ');
      }
   }
}

