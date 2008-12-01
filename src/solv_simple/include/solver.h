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

#ifndef SIMPLE_SOLVER_H
#define SIMPLE_SOLVER_H

#include "hr_lsgb.h"
#include "fn_smurf.h"
#include "fn_or.h"
#include "fn_xor.h"
#include "fn_minmax.h"
#include "display.h"
#include "bb_gelim.h"

#define HEUR_MULT 10000
#define SMURF_TABLE_SIZE 1000000

#define SMURF_STATES_INCREASE_SIZE 9

extern ProblemState *SimpleSmurfProblemState;
extern SmurfStateEntry *pTrueSimpleSmurfState;

extern double fSimpleSolverStartTime;
extern double fSimpleSolverEndTime;
extern double fSimpleSolverPrevEndTime;

extern int smurfs_share_paths;

extern int simple_solver_reset_level;

extern int nInfQueueStart;

extern int add_one_display;

extern int *tempint;
extern int tempint_max;

extern int *arrIte2SimpleSolverVarMap;
extern int *arrSimpleSolver2IteVarMap;

int SimpleBrancher();
void Final_SimpleSmurfSolver();

ITE_INLINE int InitSimpleVarMap();
ITE_INLINE void FreeSimpleVarMap();

void Alloc_SmurfStack(int destination);

int EnqueueInference(int nInfVar, bool bInfPolarity);
void check_SmurfStatesTableSize(int size);

//Hooks
ITE_INLINE void SmurfStates_Push_Hooks(int current, int destination);
ITE_INLINE void SmurfStates_Pop_Hooks();
ITE_INLINE int ApplyInference_Hooks(int nBranchVar, bool bBVPolarity);
ITE_INLINE int ApplyInferenceToSmurf_Hooks(int nBranchVar, bool bBVPolarity, int nSmurfNumber, void **arrSmurfStates);
ITE_INLINE int Backtrack_Hooks();
ITE_INLINE void Calculate_Heuristic_Values_Hooks();
ITE_INLINE void Alloc_SmurfStack_Hooks(int destination);
ITE_INLINE void Init_Solver_PreSmurfs_Hooks();
ITE_INLINE int Init_Solver_MidSmurfs_Hooks(int nSmurfIndex, void **arrSmurfStates);
ITE_INLINE int Init_Solver_PostSmurfs_Hooks(void **arrSmurfStates);
ITE_INLINE void Final_Solver_Hooks();

/*
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
*/

#endif
