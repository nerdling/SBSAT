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
// smurffactory.h
// Started 1/3/2001 - J. Ward

#ifndef SMURFFACTORY_H
#define SMURFFACTORY_H

#include "heuristic.h"

// debug options
//#define DISPLAY_TRACE
//#define DISPLAY_CHOICEPOINTS


#define FLAG_CHANGED_STRUCTURES
// j heuristic debug option
//#define TRACE_HEURISTIC

#define SHARE_STATES_IF_PATHS_MATCH


#define IsSpecialFunc(nFuncType) ((nFuncType) == OR || (nFuncType) == AND || (nFuncType) == PLAINOR || (nFuncType) == PLAINXOR || (nFuncType) == MINMAX)
#define BOOL_NEG(nBoolValue) ((nBoolValue) == BOOL_TRUE ? BOOL_FALSE : BOOL_TRUE)

struct SmurfState;

#define INIT_STACK_BACKTRACKS_ALLOC 100

#define SMURF_STATES_STACK_ALLOC_MULT 6 /* >= 2 */

typedef union {
    SmurfState *state;
    void *next_pool; 
    int   index_pool;
    int   version;
} tSmurfStateStackValue;

typedef struct { 
	int smurf; 
	int path_idx; 
	tSmurfStateStackValue u;
	int prev;
} tSmurfStatesStack;

extern int nMaxSmurfStatesStackIdx;
extern int nCurSmurfStatesVersion;
extern int nSmurfStatesStackIdx;
extern tSmurfStatesStack *arrSmurfStatesStack;
extern int *arrSmurfStatesFlags;

ITE_INLINE void Add_arrSmurfStatesStack(int vx);

#define Save_arrCurrentStates(vx)  { if (arrSmurfStatesFlags[vx]!=nCurSmurfStatesVersion) Add_arrSmurfStatesStack(vx); }

#define SPECIAL_FN_STACK_ALLOC_MULT 6 /* >= 2 */

typedef union {
    int   value;
    void *next_pool; 
    int   index_pool;
    int   version;
} tSpecialFnStackValue;

typedef struct { 
	int fn; 
	tSpecialFnStackValue u;
	int prev;
   int lhsvalue;
   double rhssum;
   int rhscounter;
} tSpecialFnStack;

typedef struct {
	int idx;
	int *literals;
} t_smurf_path;

typedef struct {
   int specfn;
	int next;
	int prev;
} t_smurf_chain;

extern t_smurf_path *arrSmurfPath;
extern t_smurf_chain *arrSmurfChain;

extern int nMaxSpecialFnStackIdx;
extern int nCurSpecialFnVersion;
extern int nSpecialFnStackIdx;
extern tSpecialFnStack *arrSpecialFnStack;
extern int *arrSpecialFnFlags;

ITE_INLINE void Add_arrNumRHSUnknowns(int vx);

#define Save_arrNumRHSUnknowns(vx)  { if (arrSpecialFnFlags[vx]!=nCurSpecialFnVersion) Add_arrNumRHSUnknowns(vx); }

//#define LITS_PER_LEMMA_BLOCK (MAX_VBLES_PER_SMURF + 2)
#define LITS_PER_LEMMA_BLOCK (8 + 2)

// The following struct represents a single node in a linked
// list used to represent a lemma.
struct LemmaBlock
{
  int arrLits[LITS_PER_LEMMA_BLOCK];
  LemmaBlock *pNext;
};

struct IntegerSet_ArrayBased
{
  int nNumElts;
  int *arrElts;
};

struct Transition
{
  //SmurfState *pState;
  SmurfState *pNextState;
  IntegerSet_ArrayBased positiveInferences;
  IntegerSet_ArrayBased negativeInferences;
  double fHeuristicWeight;
};

struct SmurfState
{
  struct BDDNodeStruct *pFunc;
  Transition *arrTransitions;
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
struct SpecialFunc
{
  int nFunctionType;
  int nLHSVble; // index of the vble on the LHS of the equation (-1 if none)
  int nLHSPolarity; // true => positive literal; false => negative lit.
  int nLHSLitMarkedValue; // Value of the LHS literal.  This member will be
  // updated in the search only after the LHS variable has been dequeued
  // from the inference queue.  It is used to update the heuristic values
  // of variables mentioned in the constraint.  We currently update the
  // heuristic values by adding in delta values.  In order to do this
  // correctly, we need to know whether the heuristic values currently
  // in place for this constraint were based
  // on the assumption that the LHS was instantiated or uninstantiated.
  // It would not be correct to determine this by going directly to the current
  // partial assignment, because the LHS variable might be assigned
  // in the partial assignment but might not have been dequeued from
  // the inference queue.  In this case, the heuristic scores would
  // reflect the assumption that the LHS was UNassigned.
  IntegerSet_ArrayBased rhsVbles;
  int *arrRHSPolarities;
  LemmaBlock *pLongLemma; // Given that the func is written in the form shown
  // above, then the long lemma is lit \/ ~lit_0 \/ ... \/ ~lit_(n-1).
  // This is a prime implicant of the constraint.
  LemmaBlock **arrShortLemmas; // Given the form shown above, the constraint
  // has n "short lemmas" of the form ~lit \/ lit_i, 0 <= i <= n-1.
  // Each of these clauses are also prime implicants of the constraint. 
  int LinkedSmurfs; // function was split into special function and smurfs 
  int min, max; // MinMax Special function bounds
};

struct BDDNodeStruct;

// Data members that are added to the BDDNodeStruct.
typedef struct {
  BDDNodeStruct *pReduct;
  SmurfState *pState; /* for compressing smurfs */
} SmurfFactoryAddons;

#define SFADDONS(x) ((SmurfFactoryAddons*)(x))

ITE_INLINE void FreeSmurfFactoryAddons(SmurfFactoryAddons *f);
ITE_INLINE void InitHeuristicTablesForSpecialFuncs();

ITE_INLINE SmurfState *
AllocateSmurfState();

ITE_INLINE void
FreeSmurfStatePool();

ITE_INLINE Transition *
FindTransition(SmurfState *pState, int nVble, int nVbleValue);
ITE_INLINE Transition *
FindOrAddTransition(SmurfState *pState, int nVble, int nVbleValue);

ITE_INLINE void
AllocateSmurfStatesStack(int newsize);

ITE_INLINE void
AllocateSpecialFnStack(int newsize);

#endif // SMURFFACTORY_H
