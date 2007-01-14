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

#ifndef HR_LSGB_H
#define HR_LSGB_H

#include "fn_smurf.h"
//#include "lemmainfo.h"

#define MK_HEUR_SCORES

#ifdef MK_HEUR_SCORES

#define J_HEUR_SCORES_STACK_ALLOC 1000000

typedef struct {
   double Pos;
   double Neg;
   /* add more heuristic values here */
} HeurScores;

typedef union {
   void *next_pool;
   int   index_pool;
} tHeurScoresStackValue;

typedef struct { 
    int v;
    tHeurScoresStackValue u;
    HeurScores h;
    int prev;
} tHeurScoresStack;

extern HeurScores *arrHeurScores;
extern int nCurHeurScoresVersion;
extern int nHeurScoresStackIdx;
extern tHeurScoresStack *arrHeurScoresStack;
extern int *arrHeurScoresFlags;
extern double *arrJWeights;

ITE_INLINE void Add_arrHeurScoresStack(int vx);

#define Save_arrHeurScores(vx)  { if (arrHeurScoresFlags && arrHeurScoresFlags[vx]!=nCurHeurScoresVersion) Add_arrHeurScoresStack(vx); }

#else
#define Save_arrHeurScores(v)
#endif

#define HWEIGHT double // Type of number used for search heuristic weights.

// Definition of "Johnson" heuristic:
// The weight of the 'true' state is zero.
// The weight of the 'false' state is undefined (not needed).
// The weight of a transition is the number of inferences which result
// from the transition plus the weight of the resulting state.
// The weight of a state not identical to 'true' or 'false' is
// the average of the weights of the outgoing transitions
// divided by JHEURISTIC_K (currently #defined in smurffactory.h).

// The following four structures are used to declare some tables
// which are used to compute and update the heuristic scores
// of the variables which appear in 'special functions'.

//---------------------------------------------------------------------

// The following structure is used for the entries in the arrAndEqFalseWght
// array.
// An AndEqFalse constraint is one of the form
// false = lit_1 /\ lit_2 /\ ... /\ lit_i.
// This is equivalent to the constraint
// -lit_1 \/ -lit_2 \/ ... \/ -lit_i
// but the solver uses the AndEqFalse form for PLAINOR special functions.
// Also, when an AndEq constraint (gets its LHS set to false) it
// becomes an AndEqFalse.
// (Lemmas, on the other hand, are stored as disjunctions of literals.)

// arrAndEqFalseWght[i].fPos is the heuristic weight of
// a transisition on an AndEqFalse constraint (whose RHS is of length i)
// which sets one of the RHS literals to true.

// arrAndEqFalseWght[i].fNeg is the analogous weight for setting one of the
// RHS literals to false.

// arrAndEqFalseWght[i].fFmla is the heuristic weight of the constraint as a
// whole.
struct AndEqFalseWghtStruct
{
  HWEIGHT fPos;
  HWEIGHT fNeg;
  HWEIGHT fFmla;
};

//---------------------------------------------------------------------

// The following structure is used for the entries in the arrAndEqWght
// array.
// An AndEq constraint is one of the form
// lit_0 = lit_1 /\ lit_2 /\ ... /\ lit_i.

// arrAndEqWght[i].fLHSPos is the heuristic weight of
// a transisition on an AndEq constraint whose RHS is of length i
// which sets the LHS literal to true.

// arrAndEqWght[i].fLHSNeg is analogous weight for setting the LHS to false.

// arrAndEqWght[i].fRHSPos is the weight for setting one of the RHS literals
// to true.

// arrAndEqWght[i].fRHSNeg is the weight for setting one of the RHS literals
// to false.

// arrAndEqWght[i].fRHSNeg is the weight of the constraint as a whole.
struct AndEqWghtStruct
{
  HWEIGHT fLHSPos;
  HWEIGHT fLHSNeg;
  HWEIGHT fRHSPos;
  HWEIGHT fRHSNeg;
  HWEIGHT fFmla;
};

//---------------------------------------------------------------------

extern HeurScores *arrHeurScores;
extern struct AndEqFalseWghtStruct *arrAndEqFalseWght;
extern struct AndEqWghtStruct *arrAndEqWght;
extern double *arrXorEqWght;
extern double *arrXorEqWghtD;
extern double *arrAndEqWghtCx;
extern double *arrAndEqWghtCe;
extern double *arrAndEqWghtCt;


ITE_INLINE void
UpdateHeuristicWithLemma(int nNumLiterals,
			 int arrLiterals[]);

ITE_INLINE void
UpdateHeuristicScoresFromTransition(Transition *pTransition);

ITE_INLINE void RemoveLemmasHeuristicInfluence(LemmaInfoStruct *pLemmaInfo);
/*
ITE_INLINE void GetHeurScoresFromSpecialFunc_AND(int nFnId);
ITE_INLINE void GetHeurScoresFromSpecialFunc_AND_C(int nFnId);
ITE_INLINE void GetHeurScoresFromSpecialFunc_XOR(int nFnId);
ITE_INLINE void GetHeurScoresFromSpecialFunc_XOR_C(int nFnId);
ITE_INLINE void GetHeurScoresFromSpecialFunc_MINMAX(int nFnId);
ITE_INLINE void GetHeurScoresFromSpecialFunc_MINMAX_C(int nFnId);
*/
ITE_INLINE int TransitionIndex_FromInt(int n, int nValueOfVble);

ITE_INLINE void J_InitHeuristicScores();
ITE_INLINE void J_FreeHeuristicScores();

ITE_INLINE void J_UpdateHeuristicScoresFromTransition(Transition *);



ITE_INLINE int J_OptimizedHeuristic (int *pnBranchAtom, int *pnBranchValue);
ITE_INLINE int J_OptimizedHeuristic_l (int *pnBranchAtom, int *pnBranchValue);
ITE_INLINE int J_OptimizedHeuristic_Berm (int *pnBranchAtom, int *pnBranchValue);

ITE_INLINE void DisplayHeuristicValues();
ITE_INLINE void FreeHeuristicTablesForSpecialFuncs();

ITE_INLINE int  HrLSGBInit();
ITE_INLINE int  HrLSGBFree();
ITE_INLINE int  HrLSGBUpdate();
ITE_INLINE int  HrLSGBBacktrack(int n);
ITE_INLINE void J_Setup_arrJWeights();

/*
ITE_INLINE void
J_UpdateHeuristic_XOR(SpecialFunc *pSpecialFunc, int nOldNumRHSUnknowns, int nNumRHSUnknowns, int counter);
ITE_INLINE void
J_UpdateHeuristic_MINMAX(SpecialFunc *pSpecialFunc, 
      int nOldNumRHSUnknowns, int nNumRHSUnknowns,
      int nOldRHSCounter, int nRHSCounter);
ITE_INLINE void
J_UpdateHeuristic_XOR_C(SpecialFunc *pSpecialFunc, 
      int nOldNumRHSUnknowns, int nNumRHSUnknowns, 
      double fOldSumRHSUnknowns, double fSumRHSUnknowns, 
      int counter);
ITE_INLINE void
J_UpdateHeuristic_AND_C(SpecialFunc *pSpecialFunc, 
      int nOldNumRHSUnknowns, int nNumRHSUnknowns, 
      int nOldNumLHSUnknowns, int nNumLHSUnknowns,
      double nOldSumRHSUnknowns, double nSumRHSUnknowns);
ITE_INLINE void
J_UpdateHeuristic_MINMAX_C(SpecialFunc *pSpecialFunc, 
      int nOldNumRHSUnknowns, int nNumRHSUnknowns, 
      double fOldSumRHSUnknowns, double fSumRHSUnknowns, 
      int counter);
*/

ITE_INLINE void
J_Update_RHS_AND(int nNumRHSUnknowns, int *arrRHSVbles, int *arrRHSPolarities, HWEIGHT fPosDelta, HWEIGHT fNegDelta);
ITE_INLINE void
J_Update_RHS_AND_C(int nNumRHSUnknowns, int *arrRHSVbles, int *arrRHSPolarities, 
     double fLastSum, double fLastConstPos, double fLastMultiPos, double fLastConstNeg, double fLastMultiNeg, 
     double fSum, double fConstPos, double fMultiPos, double fConstNeg, double fMultiNeg);
#endif // J_HEURISTIC_H
