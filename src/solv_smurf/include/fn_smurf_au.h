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

#ifndef FN_SMURF_AU_H
#define FN_SMURF_AU_H

//#include "heuristic.h"

// debug options
//#define DISPLAY_TRACE
//#define DISPLAY_CHOICEPOINTS


#define FLAG_CHANGED_STRUCTURES
// j heuristic debug option
//#define TRACE_HEURISTIC

#define SHARE_STATES_IF_PATHS_MATCH


#define IsSpecialFunc(nFuncType) ((nFuncType) == OR || (nFuncType) == AND || (nFuncType) == PLAINOR || (nFuncType) == PLAINXOR || (nFuncType) == MINMAX)
#define BOOL_NEG(nBoolValue) ((nBoolValue) == BOOL_TRUE ? BOOL_FALSE : BOOL_TRUE)

struct SmurfAuState;

#define INIT_STACK_BACKTRACKS_ALLOC 100


/*
typedef union {
    void *next_pool; 
    int   index_pool;
} tSmurfAuStateStackValue;

typedef struct { 
	int smurf_au; 
   union {
      SmurfAuState *state;
      int version;
   };
	int path_idx; 
	int prev;
} tSmurfAuStatesStack;


typedef struct { 
	int fn; 
   union {
     int   value;
     int   version;
   };
	int prev;
   int lhsvalue;
   double rhssum;
   int rhscounter;
} tSpecialFnStack;


typedef struct {
   int nType; // 0-SmurfAu, .., 1000-change pool, 1001-change fn id
   union {
      Stack_NextPool next_pool;
      Stack_FnTypeChange fn_type_change;
      SmurfAuStack ;
      SpecFnStack_PLAINOR;
      SpecFnStack_AND;
      SpecFnStack_XOR;
      SpecFnStack_MINMAX;
   }
} FunctionStack;
*/

extern int nMaxSmurfAuStatesStackIdx;
extern int nCurSmurfAuStatesVersion;
extern int nSmurfAuStatesStackIdx;
extern int *arrSmurfAuStatesFlags;

// The following struct represents a single node in a linked
// list used to represent a lemma.
typedef struct {
	int idx;
	int *literals;
} t_smurf_au_path;

typedef struct {
   int specfn;
	int next;
	int prev;
} t_smurf_au_chain;

extern t_smurf_au_chain *arrSmurfAuChain;

struct TransitionAu
{
  //SmurfAuState *pState;
  SmurfAuState *pNextState;
  IntegerSet_ArrayBased positiveInferences;
  IntegerSet_ArrayBased negativeInferences;
  double fHeuristicWeight;
};

struct SmurfAuState
{
  struct BDDNodeStruct *pFunc;
  TransitionAu *arrTransitionAus;
  IntegerSet_ArrayBased vbles;
  double fNodeHeuristicWeight;
  char cFlag;
  char cVisited;
  double *arrHeuristicXors;
  int nNumHeuristicXors;
};

// Structure for representing Boolean functions of the form:
// [lit = ] Op(lit_0, lit_1, ..., lit_(n-1)), where Op is an associative,
// commutative operator such as /\ or \/.
/*
struct SpecialFunc
{
  int nFunctionType;
  int nLHSVble; // index of the vble on the LHS of the equation (-1 if none)
  int nLHSPolarity; // true => positive literal; false => negative lit.
  IntegerSet_ArrayBased rhsVbles;
  int *arrRHSPolarities;
  LemmaBlock *pLongLemma; // Given that the func is written in the form shown
  // above, then the long lemma is lit \/ ~lit_0 \/ ... \/ ~lit_(n-1).
  // This is a prime implicant of the constraint.
  LemmaBlock **arrShortLemmas; // Given the form shown above, the constraint
  // has n "short lemmas" of the form ~lit \/ lit_i, 0 <= i <= n-1.
  // Each of these clauses are also prime implicants of the constraint. 
  int LinkedSmurfAus; // function was split into special function and smurf_aus 
  int min, max; // MinMax Special function bounds
};

struct {
   FunctionState Current;
   SmurfAuState Initial;
} SmurfAu;
*/
//struct BDDNodeStruct;

ITE_INLINE void InitHeuristicTablesForSpecialFuncs();

ITE_INLINE SmurfAuState *
AllocateSmurfAuState();

ITE_INLINE void
FreeSmurfAuStatePool();

ITE_INLINE TransitionAu *
FindTransitionAu(SmurfAuState *pState, int nVble, int nVbleValue, int nAutarkyVble);
ITE_INLINE TransitionAu *
FindOrAddTransitionAu(SmurfAuState *pState, int nVble, int nVbleValue, int nAutarkyVble);

ITE_INLINE void AllocateSmurfAuStatesStack(int newsize);
ITE_INLINE void AllocateSpecialFnStack(int newsize);

ITE_INLINE TransitionAu *
CreateTransitionAu(SmurfAuState *pState, int i, int nSolverVble, int nAutarkyVble, int value);
ITE_INLINE TransitionAu *
FindTransitionAuDebug (SmurfAuState * pState, int i, int nVble, int nVbleValue, int nAutarkyVble);
#define FindTransitionAu FindTransitionAuDebug
//#define FindTransitionAu(pState, i, nVble, nVbleValue) (pState->arrTransitionAus + 2 * i + nVbleValue)
#define FindOrAddTransitionAu FindTransitionAu

extern SmurfAuState *pTrueSmurfAuState;
ITE_INLINE SmurfAuState * BDD2SmurfAu(BDDNodeStruct *pFunc, int nAutarkyVble);

ITE_INLINE TransitionAu* AddStateTransitionAu(
         SmurfAuState *pSmurfAuState, int i, int nVble,
         int nValueOfVble, int nAutarkyVble, BDDNodeStruct *pFuncEvaled,
         SmurfAuState *pSmurfAuStateOfEvaled);

ITE_INLINE void InitializeSmurfAuStatePool (int nNumSmurfAuStatesInPool);
ITE_INLINE void SmurfAuStatesDisplayInfo();

ITE_INLINE int SmurfAuCreateFunction(void *fn, int id, BDDNode *bdd, int type, int eqVble);
ITE_INLINE int FnSmurfAuInit();
ITE_INLINE void FnSmurfAuFree();

int SmurfAuCreateFunction(int nFnId, BDDNode *bdd, int type, int eqVble);
void SmurfAuAffectedVarList(int nFnId, int **arr1, int *num1, int **arr2, int *num2);
void SmurfAuCreateAFS(int nFnId, int nVarId, int nAFSIndex);

// Function structure
typedef struct {
   SmurfAuState *pCurrentState;
   SmurfAuState *pInitialState;
   SmurfAuState *pPrevState;
   t_smurf_au_path arrSmurfAuPath;
   int nSmurfAuEqualityVble;
} Function_SmurfAu;

// AFS structure
typedef struct {
} AFS_SmurfAu;

// Stack structure
typedef struct {
   int path_idx;
   SmurfAuState *state;
} Stack_SmurfAu;

int SmurfAuUpdateAffectedFunction(int nFnId);
int SmurfAuUpdateAffectedFunction_Infer(void *oneafs, int x);

int SmurfAuSave2Stack(int nFnId, void *one_stack);
int SmurfAuRestoreFromStack(void *one_stack);

void SmurfAuUpdateFunctionInfEnd(int nFnId);

void LSGBSmurfAuUpdateFunctionInfEnd(int nFnId);

void LSGBSmurfAuSetHeurScores(int nRegSmurfAuIndex, SmurfAuState *pState);
void LSGBSmurfAuGetHeurScores(int nFnId);

void LSGBWSmurfAuSetHeurScores(int nRegSmurfAuIndex, SmurfAuState *pState);
void LSGBWSmurfAuGetHeurScores(int nFnId);

void HrLSGBFnSmurfAuInit();
void HrLSGBWFnSmurfAuInit();

#endif // SMURFFACTORY_H
