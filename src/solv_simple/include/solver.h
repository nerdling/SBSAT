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

#include "smurfs.h"
#include "hr_lsgb.h"
#include "fn_smurf.h"
#include "fn_inference.h"
#include "fn_or.h"
#include "fn_xor.h"
#include "fn_minmax.h"
#include "fn_negminmax.h"
#include "display.h"
#include "bb_gelim.h"
#include "bb_lemmas.h"
#include "restarts.h"

#define HEUR_MULT 10000
#define SMURF_TABLE_SIZE 1000000

#define SMURF_STATES_INCREASE_SIZE 9

typedef struct ProblemState {
	//Static
	int nNumSmurfs;
	int nNumVars;
	int nNumSmurfStateEntries;
	SmurfStatesTableStruct *arrSmurfStatesTableHead; //Pointer to the table of all smurf states
	SmurfStatesTableStruct *arrCurrSmurfStates;      //Pointer to the current table of smurf states
	void *pSmurfStatesTableTail;                     //Pointer to the next open block of the arrSmurfStatesTable
	int **arrVariableOccursInSmurf; //Pointer to lists of Smurfs, indexed by variable number, that contain that variable
	                                //Max size would be nNumSmurfs * nNumVars, but this would only happen if every
	                                //Smurf contained every variable. Each list is terminated by a -1 element
	int_p **arrReverseOccurenceList;//Pointer to lists of pointers into arrVariableOccursInSmurf.
	                                //arrROL[x][z].loc points to the location of arrVOIS[y][?] = x
	                                //arrROL[x][z].var = y
	
	// Dynamic
	int nCurrSearchTreeLevel;
	double *arrPosVarHeurWghts;       //Pointer to array of size nNumVars
	double *arrNegVarHeurWghts;       //Pointer to array of size nNumVars
	int *arrInferenceQueue;           //Pointer to array of size nNumVars (dynamically indexed by arrSmurfStack[level].nNumFreeVars
	int *arrInferenceDeclaredAtLevel; //Pointer to array of size nNumVars
	SmurfStack *arrSmurfStack;        //Pointer to array of size nNumVars
	SimpleLemma *arrInferenceLemmas;  //Pointer to array of size nNumVars
	SimpleLemma pConflictClause;
} ProblemState;

extern ProblemState *SimpleSmurfProblemState;
extern SmurfStateEntry *pTrueSimpleSmurfState;

extern int *arrStatesTypeSize;
typedef int (*ApplyInferenceToState)(int nBranchVar, bool bBVPolarity, int nSmurfNumber, void **arrSmurfStates);
extern ApplyInferenceToState *arrApplyInferenceToState;

typedef void (*FreeStateEntry)(void *pState);
extern FreeStateEntry *arrFreeStateEntry;

typedef void (*SetVisitedState)(void *pState, int value);
extern SetVisitedState *arrSetVisitedState;

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

ITE_INLINE void create_clause_from_SmurfState(int nInfVar, TypeStateEntry *pSmurfState, int nNumVarsInSmurf,
															 Cls **clause, int *lits_max_size);
extern "C" { int EnqueueInference_lemmas_hook(int nInfVar, bool bInfPolarity); }

int EnqueueInference(int nInfVar, bool bInfPolarity, int inf_function_type);
void check_SmurfStatesTableSize(int size);
void GarbageCollectSmurfStatesTable(int force);

//Hooks
ITE_INLINE void SmurfStates_Push_Hooks(int current, int destination);
ITE_INLINE void SmurfStates_Pop_Hooks();
ITE_INLINE int ApplyInference_Hooks(int nBranchVar, bool bBVPolarity);
ITE_INLINE int ApplyInferenceToSmurf_Hooks(int nBranchVar, bool bBVPolarity, int nSmurfNumber, void **arrSmurfStates);
ITE_INLINE int Backtrack_Hooks();
ITE_INLINE void Calculate_Heuristic_Values_Hooks();
ITE_INLINE void Alloc_SmurfStack_Hooks(int destination);
ITE_INLINE void Free_SmurfStack_Hooks(int destination);
ITE_INLINE void Init_Solver_PreSmurfs_Hooks();
ITE_INLINE int Init_Solver_MidSmurfs_Hooks(int nSmurfIndex, void **arrSmurfStates);
ITE_INLINE int Init_Solver_PostSmurfs_Hooks(void **arrSmurfStates);
ITE_INLINE void Final_Solver_Hooks();

#endif
