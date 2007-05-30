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

#ifndef SOLVER_H
#define SOLVER_H

struct IntegerSet_ArrayBased
{
  int nNumElts;
  int *arrElts;
};


#include "solver_features.h"
#include "fn_smurf.h"
#include "fn_smurf_au.h"
#include "fn_smurf_xor.h"
#include "fn_lemma.h"
#include "fn_and.h"
#include "fn_minmax.h"
#include "fn_xor.h"
#include "fn_xor_smurf.h"
#include "brancher.h"
#include "heuristic.h"
#include "hr_null.h"
#include "hr_lsgbw.h"
#include "hr_lsgb_lemma.h"
#include "hr_lsgb.h"
#include "hr_lemma.h"
#include "hr_vsids.h"
#include "hr_berkmin.h"
#include "hr_anne.h"
#include "hr_minisat.h"

#define FN_INIT_LIST { \
      FnSmurfInit, \
      FnLemmaInit, \
      FnAndInit, \
      FnMinMaxInit, \
      FnXorInit, \
      FnSmurfAuInit, \
      FnXorSmurfInit, \
      FnSmurfXorInit, \
      NULL }
/*
enum {
   TYPE_UNKNOWN  = 0,
   TYPE_SMURF    = 1,
   TYPE_SMURF_AU = 2,
   TYPE_SMURF_EX = 3,
   TYPE_AND      = 4,
   TYPE_PLAINOR  = 5,
   TYPE_XOR      = 6,
   TYPE_XOR_EX   = 7,
   TYPE_MINMAX   = 8 
};
*/
typedef struct {
   int nFnId;
   int nType;
} t_fn_inf_queue;


typedef struct {
   int fn;
} Stack_FnTypeChange;

typedef struct _FnStack {
   int nFnId;
   int nType; // 0-Smurf, .., 1000-change pool, 1001-change fn id
   int prev_version;
   union {
      struct {
         _FnStack * next_pool;
         int index_pool;
      }/*Stack_NextPool*/  fn_next_pool;
      Stack_FnTypeChange  fn_fn_type_change;
      Stack_Smurf         fn_smurf;
      Stack_SmurfAu       fn_smurf_au;
      //Stack_PlainOR       fn_plainor;
      Stack_AND           fn_and;
      Stack_XOR           fn_xor;
      Stack_MinMax        fn_minmax;
   };
} FnStack;

typedef struct {
   FnStack * next_pool;
   int index_pool;
} Stack_NextPool;


typedef struct {
   int nFnId;
   int nType;
   union {
      AFS_Smurf    fn_smurf;
      AFS_SmurfAu  fn_smurf_au;
      //AFS_SmurfXOR fn_smurf_xor;
      //AFS_PLAINOR  fn_plainor;
      AFS_AND      fn_and;
      AFS_XOR      fn_xor;
      AFS_MINMAX   fn_minmax;
      //LemmaAFS     fn_lemma;
      //AFS_XORSmurf fn_xor_smurf;
   };
} OneAFS;

typedef struct 
// Structure which gives a list of the functions (constraints) mentioning
// a particular variable, and the role of the variable in each of those
// functions.
{
   int nNumOneAFS;
   OneAFS *arrOneAFS;

   // The following members are used to implement the Chaff-style lemmas.
   // For instance, pLemmasWherePos1 points to the list of lemmas
   // where the current variable is mentioned positively and where
   // the variable is the first of the two literals in the lemma
   // which are being watched by the brancher.
   LemmaInfoStruct LemmasWherePos[2];
   LemmaInfoStruct LemmasWherePosTail[2];
   // pLemmasWherePos2 is the same thing, except that in each of these lemmas
   // the variable is the _second_ of the two literals which are being watched
   // by the brancher.
   LemmaInfoStruct LemmasWhereNeg[2];
   LemmaInfoStruct LemmasWhereNegTail[2];
} AffectedFuncsStruct;

typedef struct _SolverFunction {
   int nFnId;
   int nType;
   int nFnPriority;
   _SolverFunction *pFnInfNext;
   union {
      Function_AND      fn_and;
      Function_Smurf    fn_smurf;
      Function_SmurfAu  fn_smurf_au;
      //Function_SmurfXOR fn_smurf_xor;
      //Function_PlainOR fn_plainor;
      Function_MinMax   fn_minmax;
      Function_XOR      fn_xor;
      //Function_XORSmurf fn_xor_smurf;
   };
} SolverFunction;

extern SolverFunction * arrSolverFunctions;

#define MAX_FN_PRIORITY 3
typedef struct {
   SolverFunction *First;
   SolverFunction *Last;
} FnInfPriority;
extern FnInfPriority arrFnInfPriority[MAX_FN_PRIORITY];
extern int nLastFnInfPriority;

/*
InitFunction procInitFunction[] = { };
AfterInitFunction procAfterInitFunction[] = { };
PreFreeFunction procFreeFunction[] = { }
FreeFunction procFreeFunction[] = { }
DisplayFunction procDisplayFunction[] = { }
*/

typedef int  (*fnCreateFunction)(int nFnId, BDDNode *, int type, int eqVble);
typedef int  (*Save2Stack)(int id, void *one_stack);
typedef int  (*RestoreFromStack)(void *one_stack);
typedef int  (*UpdateAffectedFunction)(int nFnId);
typedef int  (*UpdateAffectedFunction_Infer)(void *oneafs, int x);
typedef int  (*HeurInit)();
typedef int  (*HeurFree)();
typedef int  (*HeurUpdate)();
typedef int  (*HeurBacktrack)(int n);
typedef int  (*HeurSelect)(int *atom, int *value);
typedef void (*AffectedVarList)(int nFnId, int **arr1, int *num1, int **arr2, int *num2);
typedef void (*CreateAFS)(int nFnId, int nVarId, int nAFSIndex);
typedef void (*UpdateFunctionInfEnd)(int nFnId);
typedef void (*HeurUpdateFunctionInfEnd)(int nFnId);
typedef int  (*FnInit)();
typedef int  (*HrInit)();
typedef void (*HeurGetScores)(int nFnId);
typedef void (*HeurTypeInit)();
typedef void (*HeurUpdateLemma)(LemmaInfoStruct *pLemmaInfo);
typedef void (*HeurUpdateLemmaSpace)(int *arr, int num);

     
extern int nNumFns;

extern FnInit                  procFnInit[];
extern fnCreateFunction       *procCreateFunction;
extern Save2Stack             *procSave2Stack;
extern RestoreFromStack       *procRestoreFromStack;
extern UpdateAffectedFunction *procUpdateAffectedFunction;
extern UpdateAffectedFunction_Infer *procUpdateAffectedFunction_Infer;
extern UpdateFunctionInfEnd   *procUpdateFunctionInfEnd;
extern AffectedVarList        *procAffectedVarList;
extern CreateAFS              *procCreateAFS;

extern HeurInit                procHeurInit;
extern HeurFree                procHeurFree;
extern HeurUpdate               procHeurUpdate;
extern HeurSelect               procHeurSelect;
extern HeurUpdateFunctionInfEnd *procHeurUpdateFunctionInfEnd;
extern HeurBacktrack            procHeurBacktrack;
extern HeurGetScores          *procHeurGetScores;
extern HeurTypeInit           *procHeurTypeInit;
extern HeurUpdateLemma         procHeurAddLemma;
extern HeurUpdateLemma         procHeurRemoveLemma;
extern HeurUpdateLemmaSpace    procHeurAddLemmaSpace;

extern int nCurFnVersion;

extern int **arrPattern;
extern int *arrTempSlideLemma;
extern int *arrTempSlideSmurf;

extern int nIndepVars;
extern int nDepVars;
extern int nNumVariables;

extern int gnMaxVbleIndex;
extern int nBacktrackStackIndex;
extern int *arrBacktrackStackIndex;
extern int nNumUnresolvedFunctions;
extern char *arrSolution;

extern int *pInferenceQueueNextElt;
extern int *pInferenceQueueNextEmpty;
extern int *arrInferenceQueue;

extern t_fn_inf_queue *pFnInferenceQueueNextElt;
extern t_fn_inf_queue *pFnInferenceQueueNextEmpty;
extern t_fn_inf_queue *arrFnInferenceQueue;
extern t_fn_inf_queue *pFnInfQueueUpdate;

extern LemmaInfoStruct * pConflictLemmaInfo;
extern LemmaBlock * pConflictLemma;
extern int *arrSmurfEqualityVble;

// flag changed functions and smurfs 
extern int *arrChangedFn;


// preallocated structures for BackTrack 
extern int  *arrUnsetLemmaFlagVars;
extern bool *arrLemmaFlag;
extern int  *arrTempLemma;
extern int  *arrPrevTempLemma;
extern bool *arrSmurfsRefFlag;
extern int  *arrTempSmurfsRef;

//extern SpecialFunc *arrSpecialFuncs;
extern int nNumFuncs;

extern int *arrFunctionType;
extern BDDNodeStruct **arrFunctions;

extern int *arrInferenceQueue;
extern int nNumVariables;
extern AffectedFuncsStruct *arrAFS; // "Affected Funcs Struct":
extern int nInferredAtom;
extern int nInferredValue;

extern int gnNumLemmas;

// Backtrack arrays 
extern int nNumUnresolvedFunctions;
/*
typedef struct {
   int nSolution;
   int nBtStackIndex;
   int nIndep;
   int nLemmaFlag;

   union {
      LemmaHeur;
      JohnsonHeur;
      CombinedHeur;
   }
} Variable;
*/

typedef struct {
   int chosen[2];
   int backjumped[2];
   int infs[2];
} t_var_stat;
extern t_var_stat *var_stat;

extern int compress_smurfs;

extern int *arrIte2SolverVarMap;
extern int *arrSolver2IteVarMap;

extern int total_vars;

extern int *arrLemmaVbleCountsPos;
extern int *arrLemmaVbleCountsNeg;

extern t_solution_info *solution_info;
extern t_solution_info *solution_info_head;
extern BacktrackStackEntry *pStartBacktrackStack;

ITE_INLINE int  BrancherInit();
ITE_INLINE void BrancherFree();

ITE_INLINE void FnStackInit();
ITE_INLINE void FnStackFree();
ITE_INLINE void FnStackPushFunctionLevel();
ITE_INLINE void FnStackMark(int nMark);
ITE_INLINE void FnStackPush(int nFnId, int nType);
#define FN_STACK_PUSH(vx, type)  { if (arrFnFlags[vx]!=nCurFnVersion) FnStackPush(vx, type); else { d9_printf3("Not pushing %d %d\n", arrFnFlags[vx], nCurFnVersion); }}
extern int *arrFnFlags;

ITE_INLINE void AFSFree();
ITE_INLINE int  AFSInit();

ITE_INLINE int ConstructTempLemma();
ITE_INLINE int ConstructTempSmurfsRef();
ITE_INLINE void AddLemmaIntoCache(LemmaInfoStruct *p);
ITE_INLINE void InferLiteral(int nInferredAtom, int nInferredValue,
             bool bWasChoicePoint,
             LemmaBlock *pLemma, LemmaInfoStruct *pCachedLemma, int infer);
ITE_INLINE void
InferNLits(int nNumElts, int *arrElts, int *arrPolarities, LemmaBlock **arrShortLemmas, int n);
ITE_INLINE void pop_state_information(int n);
ITE_INLINE int pop_mark_state_information();

ITE_INLINE int MaxVbleIndex();
ITE_INLINE int CreateFunctions();
ITE_INLINE int VerifySolution();

int solve_init();
void solve_free();
void InitFunctions();
void FreeFunctions();

//void DisplaySpecialFunc(SpecialFunc *p);
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
int UpdateEachAffectedFunction(AffectedFuncsStruct *pAFS, int max_fn_priority);
void Functions_Update_Inf_End_np();
void Functions_Update_Inf_End();

void InitVarMap();
void FreeVarMap();

void InitVariables();
void FreeVariables();

void BtStackInit();
void BtStackClear();
void BtStackFree();

/* choice point hints */
int GetChoicePointHint();
void AddChoicePointHint(int x);
void FreeChoicePointHint();
void InitChoicePointHint();
void EmptyChoicePointHint();

/* crtwin */
void crtwin();
void crtwin_init();
#endif
