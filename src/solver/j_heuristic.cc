/* =========FOR INTERNAL USE ONLY. NO DISTRIBUTION PLEASE ========== */

/*********************************************************************
 Copyright 1999-2003, University of Cincinnati.  All rights reserved.
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

#include "ite.h"
#include "solver.h"

double *arrJWeights;

ITE_INLINE SmurfState * GetSmurfState(int i);
ITE_INLINE void J_SetupHeuristicScores();
ITE_INLINE void J_Setup_arrJWeights();
ITE_INLINE void GetHeurScoresFromSmurf(int i);
ITE_INLINE void J_ResetHeuristicScores();

// The constant c is taken from the documentation on the heuristics
// which was provided in the Summer 2000 progress report.
// The constant is added into the positive and negative scores of
// each variable.  So, for example, the positive heuristic score
// for a variable is c plus the sum of the weights of the positive transitions
// for that variable in all of the constraints which mention the variable.
// The Summer 2000 report set c = 1/4096.  MvF used c = 1 in his prototype.
//static const double c_heur = 1.0 / 4096.0;
static const double c_heur = 1.0;

extern int nNumSpecialFuncs;
extern int nNumRegSmurfs;	// Number of regular Smurfs.
extern int nNumVariables;

extern SpecialFunc *arrSpecialFuncs;

void
DisplayUpdatesToHeuristicTransScores(Transition *pTransition);

void
J_UpdateHeuristicScoresFromTransition(Transition *pTransition);

ITE_INLINE void
J_InitHeuristicScores()
// Initialize the heuristic scores for all of the variables
// the problem.  Called before the search begins.
{
   d2_printf1("Initializing heuristicScores\n");

   pTrueSmurfState->fNodeHeuristicWeight = JHEURISTIC_K_TRUE;

   J_Setup_arrJWeights();
   J_SetupHeuristicScores();

   /* 
    DisplayJHeuristicValues();
    int i = 0;
    while ((pState = GetSmurfState(i)) != NULL)
    {
    fprintf(stddbg, "State ");
    fprintf(stddbg, "%2d: ", i);
    for(int j=0;j<pState->vbles.nNumElts;j++) {
    fprintf(stddbg, "%d ", arrSolver2IteVarMap[pState->vbles.arrElts[j]]);
    }
    fprintf(stddbg, "\n");
    if (pState->pFunc) { printBDDerr(pState->pFunc); fprintf(stddbg, "\n"); }
    fprintf(stddbg, "score: %f\n", pState->fNodeHeuristicWeight);
    for(int j=0;j<pState->nNumHeuristicXors;j++)
    fprintf(stddbg, "[%d]: %f\n", j, pState->arrHeuristicXors[j]);
    i++;
    }
    */
}

ITE_INLINE void
J_FreeHeuristicScores()
{
   ite_free((void*)pTrueSmurfState->arrHeuristicXors);
   ite_free((void*)arrJWeights);
}

ITE_INLINE void
J_ResetHeuristicScores()
{
   for (int i = 0; i < nNumVariables; i++)
   {
      arrHeurScores[i].Pos = arrHeurScores[i].Neg = 0; //c_heur;
   }

   // Loop through the regular Smurfs.
   for (int i = 0; i < nNumRegSmurfs; i++)
   {
      GetHeurScoresFromSmurf(i);
   } 

   // Loop through the special functions.
   for (int i = 0; i < nNumSpecialFuncs; i++)
   {
      switch (arrSpecialFuncs[i].nFunctionType) {
       case AND: GetHeurScoresFromSpecialFunc_AND_C(i); break;
                 //case AND: GetHeurScoresFromSpecialFunc_AND(i); break;
       case XOR: GetHeurScoresFromSpecialFunc_XOR_C(i); break;
                 //case XOR: GetHeurScoresFromSpecialFunc_XOR(i); break;
       default: assert(0); break;
      }
   }
}


#define J_ONE 0

// CLASSIC
#define HEUR_WEIGHT(x,i) (J_ONE+x.Pos) * (J_ONE+x.Neg)
//#define HEUR_WEIGHT(x,i) (x.Pos*x.Neg+arrVarScores[i].neg>arrVarScores[i].pos?arrVarScores[i].neg:arrVarScores[i].pos)
//#define HEUR_WEIGHT(x,i) (arrVarScores[i].neg>arrVarScores[i].pos?arrVarScores[i].neg:arrVarScores[i].pos)

// Var_Score
//#define HEUR_WEIGHT(x,i) (var_score[i] * (J_ONE+x.Pos) * (J_ONE+x.Neg))

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

#define HEUR_EXTRA_IN() D_8(DisplayJHeuristicValues();); d8_printf1("\n");
#define HEUR_EXTRA_OUT() \
   d8_printf6("JHeuristic: %c%d (%.10f,%.10f) because of %f\n",  \
         (*pnBranchValue==BOOL_TRUE?'+':'-'), \
         nBestVble,  \
         arrHeurScores[nBestVble].Pos, \
         arrHeurScores[nBestVble].Neg, \
         fMaxWeight);

#define HEUR_SIGN(nBestVble) \
   (arrHeurScores[nBestVble].Pos >= arrHeurScores[nBestVble].Neg?BOOL_TRUE:BOOL_FALSE)

#include "heur_choice.cc"

#undef HEUR_WEIGHT
#define HEUR_WEIGHT(x,i) (x.Pos*x.Neg*(arrLemmaVbleCountsNeg[i]>arrLemmaVbleCountsPos[i]?arrLemmaVbleCountsNeg[i]:arrLemmaVbleCountsPos[i]))

#undef HEUR_SIGN
#define HEUR_SIGN(nBestVble) \
   (arrHeurScores[nBestVble].Pos >= arrHeurScores[nBestVble].Neg?BOOL_TRUE:BOOL_FALSE)
   //(arrLemmaVbleCountsPos[nBestVble]*arrHeurScores[nBestVble].Pos >= arrLemmaVbleCountsNeg[nBestVble]*arrHeurScores[nBestVble].Neg?BOOL_TRUE:BOOL_FALSE)

#undef HEUR_FUNCTION
#define HEUR_FUNCTION J_OptimizedHeuristic_l
#include "heur_choice.cc"

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

ITE_INLINE
double
GetHeurWeight(SmurfState *pState, int i, int nVble, int nValue)
   // Returns the heuristic weight of a transition.
{
   Transition *pTransition;

   if (pState == pTrueSmurfState) return 0;

   assert(i<pState->vbles.nNumElts && pState->vbles.arrElts[i] == nVble);
/*
   else {
      int j=0;
      if (pState->vbles.nNumElts <= i || pState->vbles.arrElts[i] != nVble) {
         for (j=0; j<pState->vbles.nNumElts; j++)
            if (pState->vbles.arrElts[j] == nVble) break;
         if (j==pState->vbles.nNumElts) return 0;
         i = j;
      }
   }
*/

   assert(pState->arrTransitions != 0);
/*
   if (pState->arrTransitions == 0)
   {
      // 'true' state -- all variables have weight 0
      return 0.0;
   }
*/
   if ((pTransition=FindTransition(pState, i, nVble, nValue))==NULL) return 0;

   return pTransition->fHeuristicWeight;
}

ITE_INLINE void
DisplayJHeuristicValues()
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


// Update scores of RHS variables.
ITE_INLINE void
J_Update_RHS_AND(SpecialFunc * pSpecialFunc, HWEIGHT fPosDelta, HWEIGHT fNegDelta)
{
   int nNumRHSUnknowns = pSpecialFunc->rhsVbles.nNumElts;
   int *arrRHSVbles = pSpecialFunc->rhsVbles.arrElts;
   int *arrRHSPolarities = pSpecialFunc->arrRHSPolarities;
   int nRHSVble;
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
      /*
       else
       {
       arrHeurScores[nRHSVble].Pos = 0;
       arrHeurScores[nRHSVble].Neg = 0;
       }
       */
   } // for each RHS vble
}

// Update scores of RHS variables.
ITE_INLINE void
J_Update_RHS_AND_C(SpecialFunc * pSpecialFunc, 
     double fLastSum, double fLastConstPos, double fLastMultiPos, double fLastConstNeg, double fLastMultiNeg, 
     double fSum, double fConstPos, double fMultiPos, double fConstNeg, double fMultiNeg)
{
   int nNumRHSUnknowns = pSpecialFunc->rhsVbles.nNumElts;
   int *arrRHSVbles = pSpecialFunc->rhsVbles.arrElts;
   int *arrRHSPolarities = pSpecialFunc->arrRHSPolarities;
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
      /*
       else
       {
       arrHeurScores[nRHSVble].Pos = 0;
       arrHeurScores[nRHSVble].Neg = 0;
       }
       */
   } // for each RHS vble
}

ITE_INLINE void
J_SetHeurWeightsTrueState(int nNumXors)
{
         if (pTrueSmurfState->nNumHeuristicXors < nNumXors) {
            pTrueSmurfState->arrHeuristicXors 
               = (double*)realloc(pTrueSmurfState->arrHeuristicXors, nNumXors*sizeof(double));
            pTrueSmurfState->nNumHeuristicXors = nNumXors;
            pTrueSmurfState->arrHeuristicXors[0] = 0;       
            pTrueSmurfState->arrHeuristicXors[1] = 0;      
            pTrueSmurfState->arrHeuristicXors[2] = 1;
            for(int j=3;j<nNumXors;j++) {
               pTrueSmurfState->arrHeuristicXors[j] = arrXorEqWght[j-1];
            }
         };
}

ITE_INLINE double
J_SumInferenceWeights(Transition *pTransition)
{
  //return pTransition->positiveInferences.nNumElts +
  //       pTransition->negativeInferences.nNumElts;

//#define MK_WEIGHTS
#ifdef MK_WEIGHTS
  return pTransition->pState->vbles.nNumElts - (pTransition->pNextState->vbles.nNumElts + 1);
#endif

  int i=0;
  double fSum = 0;

  for(i=0;i<pTransition->positiveInferences.nNumElts;i++)
    fSum += arrJWeights[pTransition->positiveInferences.arrElts[i]];

  for(i=0;i<pTransition->negativeInferences.nNumElts;i++)
    fSum += arrJWeights[pTransition->negativeInferences.arrElts[i]];

//#define MK_WEIGHTS_X
#ifdef MK_WEIGHTS_X
  fSum += (pTransition->pState->vbles.nNumElts - (pTransition->pNextState->vbles.nNumElts + 1 +
      pTransition->positiveInferences.nNumElts + pTransition->negativeInferences.nNumElts));
     ///JHEURISTIC_K;
#endif

  return fSum;
}

ITE_INLINE double
J_SetHeurScoreTransition(SmurfState *pState, int i, Transition *pTransition, int nRegSmurfIndex, int nNumXors, int polarity)
{
   double fTransitionWeight = J_SumInferenceWeights(pTransition) +
                          pTransition->pNextState->fNodeHeuristicWeight;
   pTransition->fHeuristicWeight = fTransitionWeight;

   if (nNumXors) {
        if (pState->vbles.arrElts[i] != arrSmurfEqualityVble[nRegSmurfIndex]) {
           for (int j=3;j<nNumXors;j++) {
                  pState->arrHeuristicXors[j] += pTransition->pNextState->arrHeuristicXors[j];
        }
      } else {
        pState->arrHeuristicXors[polarity] = pTransition->fHeuristicWeight;
      }
   }
   return fTransitionWeight;
}

ITE_INLINE void
J_SetHeurScoresForSmurfs_Counting(int nRegSmurfIndex, SmurfState *pState, int nNumXors)
{
   if (pState == pTrueSmurfState) {
      return;
   }

   // FIND OUT IF THE HEUR ALREADY COMPUTED 
   if (pState->cFlag == 2) return;
   pState->cFlag = 2;

   double fTotalCount = 0;

   for (int i=0;i<pState->vbles.nNumElts;i++)
   {
      /* ----- POSITIVE TRANSITIONS ------ */
      {
         Transition *pTransition = FindTransition(pState, i, pState->vbles.arrElts[i], BOOL_TRUE);
         J_SetHeurScoresForSmurfs_Counting(nRegSmurfIndex, pTransition->pNextState, nNumXors);
         fTotalCount += pTransition->pNextState->fNodeHeuristicWeight;
      }

      /* ----- NEGATIVE TRANSITIONS ------ */
      {
         Transition *pTransition = FindTransition(pState, i, pState->vbles.arrElts[i], BOOL_FALSE);
         J_SetHeurScoresForSmurfs_Counting(nRegSmurfIndex, pTransition->pNextState, nNumXors);
         fTotalCount += pTransition->pNextState->fNodeHeuristicWeight;
      }
   }

   pState->fNodeHeuristicWeight = 1+fTotalCount;

   for (int i=0;i<pState->vbles.nNumElts;i++)
   {
      Transition *pTransition;
      pTransition = FindTransition(pState, i, pState->vbles.arrElts[i], BOOL_TRUE);
      pTransition->fHeuristicWeight = 
         pTransition->pState->fNodeHeuristicWeight - pTransition->pNextState->fNodeHeuristicWeight;

      pTransition = FindTransition(pState, i, pState->vbles.arrElts[i], BOOL_FALSE);
      pTransition->fHeuristicWeight =
         pTransition->pState->fNodeHeuristicWeight - pTransition->pNextState->fNodeHeuristicWeight;
   }
}

ITE_INLINE void
J_SetHeurScoresForSmurfs(int nRegSmurfIndex, SmurfState *pState, int nNumXors)
{
   if (pState == pTrueSmurfState) {
      if (pTrueSmurfState->nNumHeuristicXors < nNumXors)
          J_SetHeurWeightsTrueState(nNumXors);
      return;
   }

   // FIND OUT IF THE HEUR ALREADY COMPUTED 
   if (pState->cFlag == 2 && pState->nNumHeuristicXors >= nNumXors) return;
   pState->cFlag = 2;

   if (pState->nNumHeuristicXors < nNumXors) {
      ite_free((void*)pState->arrHeuristicXors);
      pState->arrHeuristicXors = (double*)ite_calloc(nNumXors, sizeof(double),
            9, "pState->arrHeuristicXors");
      pState->nNumHeuristicXors = nNumXors;
   }

   double fTotalTransitions  = 0;
   for (int i=0;i<pState->vbles.nNumElts;i++)
   {
      /* ----- POSITIVE TRANSITIONS ------ */
      {
         Transition *pTransition = FindTransition(pState, i, pState->vbles.arrElts[i], BOOL_TRUE);
         J_SetHeurScoresForSmurfs(nRegSmurfIndex, pTransition->pNextState, nNumXors);
         fTotalTransitions  += J_SetHeurScoreTransition(pState, i, pTransition, nRegSmurfIndex, nNumXors, 1);
      }

      /* ----- NEGATIVE TRANSITIONS ------ */
      {
         Transition *pTransition = FindTransition(pState, i, pState->vbles.arrElts[i], BOOL_FALSE);
         J_SetHeurScoresForSmurfs(nRegSmurfIndex, pTransition->pNextState, nNumXors);
         fTotalTransitions  += J_SetHeurScoreTransition(pState, i, pTransition, nRegSmurfIndex, nNumXors, 0);
      }
   }

   pState->fNodeHeuristicWeight = fTotalTransitions / (pState->vbles.nNumElts * 2 * JHEURISTIC_K);

   if (nNumXors) {
      /* only for base case */
      pState->arrHeuristicXors[2] = pState->fNodeHeuristicWeight;
      for (int j=3;j<nNumXors;j++) {
          pState->arrHeuristicXors[j] += (j-1)*2* pState->arrHeuristicXors[j-1];
          pState->arrHeuristicXors[j] /= (2.0 * (pState->vbles.nNumElts-1+j-1) * JHEURISTIC_K);
      }
   }

}

ITE_INLINE void
J_SetupHeuristicScores()
{
   // Loop through the regular Smurfs.
   for (int i = 0; i < nNumRegSmurfs; i++)
   {
      int nRHSXorVbles = 0;
      int specfn = arrSmurfChain[i].specfn;
      if (specfn != -1)  nRHSXorVbles = arrSpecialFuncs[specfn].rhsVbles.nNumElts+1;
      if (sHeuristic[1] == 'C') {
         assert(nRHSXorVbles == 0);
         J_SetHeurScoresForSmurfs_Counting(i, arrRegSmurfInitialStates[i], nRHSXorVbles);
      } else {
         J_SetHeurScoresForSmurfs(i, arrRegSmurfInitialStates[i], nRHSXorVbles);
      }

   } 
}

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
      if (sHeuristic[1] == 0) {
         arrJWeights[nVble] = 1;
      } else {
         assert(BREAK_XORS == 0);
         long sum = arrAFS[nVble].nNumRegSmurfsAffected + 
            arrAFS[nVble].nNumSpecialFuncsAffected;
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
   if (sHeuristic[1] == 'r' || sHeuristic[1] == 'R') {
      for (int nVble = 0; nVble <= gnMaxVbleIndex; nVble++)
      {
         arrJWeights[nVble] /= max;
      }
   }
}
