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

#ifndef SOLVER_H
#define SOLVER_H

#define JOHNSON_HEURISTIC_LEMMA

void FreeAddonsPool();

extern int nIndepVars;
extern int nDepVars;
extern int nNumVariables;
extern SmurfState *pTrueSmurfState;
extern int gnMaxVbleIndex;
extern int gnNumCachedLemmas;
extern int nBacktrackStackIndex;
extern BacktrackStackEntry *pBacktrackTop;     
extern int *arrBacktrackStackIndex;
extern ChoicePointStruct *arrChoicePointStack;
extern ChoicePointStruct *pChoicePointTop;
extern int nNumUnresolvedFunctions;
extern char *arrSolution;

extern int *pInferenceQueueNextElt;
extern int *pInferenceQueueNextEmpty;
extern int *arrInferenceQueue;

typedef struct {
   int fn_type; /* fn type - smurf=0/specfn=1 */
   int fn_id;
} t_fn_inf_queue;

extern t_fn_inf_queue *pFnInferenceQueueNextElt;
extern t_fn_inf_queue *pFnInferenceQueueNextEmpty;
extern t_fn_inf_queue *arrFnInferenceQueue;

extern LemmaBlock * pConflictLemma;
extern LemmaInfoStruct * pConflictLemmaInfo;
extern SmurfState **arrCurrentStates;
extern SmurfState **arrPrevStates;
extern int *arrSmurfEqualityVble;

/* flag changed functions and smurfs */
extern int *arrChangedSpecialFn;
extern int *arrChangedSmurfs;


/* preallocated structures for BackTrack */
extern int *arrUnsetLemmaFlagVars;
extern bool *arrLemmaFlag;
extern int  *arrTempLemma;

extern SpecialFunc *arrSpecialFuncs;
extern int nNumSpecialFuncs;
extern int nNumRegSmurfs; // Number of regular Smurfs.
//representing the Boolean function 'true'.
extern int *arrFunctionType;
extern BDDNodeStruct **arrFunctions;
extern int *arrInferenceQueue;
extern int nNumVariables;
extern AffectedFuncsStruct *arrAFS; // "Affected Funcs Struct":
extern int nInferredAtom;
extern int nInferredValue;
extern SmurfState **arrCurrentStates; // Current states of all of the Smurfs, i.e.,

extern ChoicePointStruct *arrChoicePointStack; // Info on previous branch variables.
extern ChoicePointStruct *pChoicePointTop; // Next free position in above stack.

extern int *pInferenceQueueNextElt; // ptr to next available elt in inference queue
//  For a given variable index,
// gives the functions may be affected by updating that variable.

extern int gnNumLemmas;
extern int *arrBacktrackStackIndex;
extern int nBacktrackStackIndex;
extern BacktrackStackEntry *pBacktrackTop; // Next free position in backtrack stack.
// The next five identifiers are used for identifying and processing
// a lemma which witnesses a forced assignment which causes a conflict
// during the search.
extern int *arrTempLemma;

/* Backtrack arrays */
extern int *arrUnsetLemmaFlagVars;
extern bool *arrLemmaFlag;

extern int *pInferenceQueueNextEmpty; // ptr to next empty slot in inference queue
extern int nNumUnresolvedFunctions;
extern int nVble;

extern int *arrNumRHSUnknowns;
extern int *arrNumRHSUnknownsNew;
extern int *arrPrevNumRHSUnknowns;

extern int *arrNumLHSUnknowns;
extern int *arrNumLHSUnknownsNew;
extern int *arrPrevNumLHSUnknowns;

extern double *arrSumRHSUnknowns;
extern double *arrSumRHSUnknownsNew;
extern double *arrPrevSumRHSUnknowns;


extern int compress_smurfs;

extern BDDNode *false_ptr;
extern BDDNode *true_ptr;

extern int *arrIte2SolverVarMap;
extern int *arrSolver2IteVarMap;

extern int total_vars;

extern int *arrLemmaVbleCountsPos;
extern int *arrLemmaVbleCountsNeg;

ITE_INLINE int InitBrancher();
ITE_INLINE void FreeSpecialFnStack();
ITE_INLINE void FreeSmurfStatesStack();
ITE_INLINE void FreeBrancher();
ITE_INLINE void FreeSmurfFactory();

ITE_INLINE int ConstructTempLemma();
ITE_INLINE void AddLemmaIntoCache(LemmaInfoStruct *p);
ITE_INLINE void InferLiteral(int nInferredAtom, int nInferredValue,
             bool bWasChoicePoint,
             LemmaBlock *pLemma, LemmaInfoStruct *pCachedLemma, int infer);
ITE_INLINE void pop_state_information(int n);
ITE_INLINE void Mark_arrSmurfStatesStack(int);
ITE_INLINE void Mark_arrNumRHSUnknowns(int);

ITE_INLINE int MaxVbleIndex();
ITE_INLINE int SmurfFactory();
ITE_INLINE int VerifySolution();

ITE_INLINE void InitializeAddons(BDDNodeStruct *pFunc);
ITE_INLINE void AssertNullAddons(BDDNodeStruct *pFunc);
ITE_INLINE BDDNodeStruct *EvalBdd(BDDNodeStruct *pFunc, int nVble, bool bValueOfVble);
ITE_INLINE void ComputeVbleSet(BDDNode *pFunc);

ITE_INLINE SmurfState * BDD2Smurf(BDDNodeStruct *pFunc);
ITE_INLINE void SpecFn2Smurf (BDDNodeStruct *pFunc, int nFunctionType,
               int nEqualityVble,  SpecialFunc *pSpecialFunc);
ITE_INLINE void ComputeVbleSet(BDDNode *pFunc);
ITE_INLINE BDDNodeStruct *EvalBdd(BDDNodeStruct *pFunc,
               int nVble, bool bValueOfVble);
ITE_INLINE Transition* AddStateTransition(
         SmurfState *pSmurfState, int i, int nVble,
         int nValueOfVble, BDDNodeStruct *pFuncEvaled,
         SmurfState *pSmurfStateOfEvaled);
ITE_INLINE void InitializeSmurfStatePool (int nNumSmurfStatesInPool);
ITE_INLINE void SmurfStatesDisplayInfo();
ITE_INLINE void UpdateHeuristic();
ITE_INLINE void J_UpdateHeuristic();
ITE_INLINE void J_UpdateHeuristicSmurf(SmurfState *pOldState, SmurfState *pState, int);

ITE_INLINE void Update_arrVarScores();

int solve_init();
void solve_free();

ITE_INLINE Transition *
FindTransitionDebug (SmurfState * pState, int i, int nVble, int nVbleValue);
#define FindTransition FindTransitionDebug
//#define FindTransition(pState, i, nVble, nVbleValue) (pState->arrTransitions + 2 * i + nVbleValue)
#define FindOrAddTransition FindTransition

void
Heuristic(int *pnBranchAtom, int *pnBranchValue);

void DisplaySpecialFunc(SpecialFunc *p);
void DisplayStatus(int nNumSmurfs,
              SmurfState **arrCurrentStates,
              int nNumUnresolvedFunctions,
              int nNumChoicePts,
              int nNumBacktracks,
              int arrNumRHSUnknowns[]);
void DisplayPartialSolution(int nNumVariables);
#ifdef DISPLAY_TRACE
enum InferenceType {REG_SMURF, LEMMA, SPECIAL_FUNC};
void DisplayRelevantVbleAssignments(BDDNode *pBDD);
void DisplayInference(InferenceType eInfType,
                 void *pConstraintInfo,
                 int nNewInferredAtom,
                 int nNewInferredValue);
#endif
void DisplaySolution(int nMaxVbleIndex);

/* crtwin */
void crtwin();
void crtwin_init();
#endif
