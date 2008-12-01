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

#ifndef SBSAT_SOLVER_H
#define SBSAT_SOLVER_H

/* list of available heuristics */
enum {
   NULL_HEURISTIC,
   JOHNSON_HEURISTIC,
   C_LEMMA_HEURISTIC,
   JOHNSON_LEMMA_HEURISTIC,
   INTERACTIVE_HEURISTIC,
   STATE_HEURISTIC
};

/* list of possible smurf function types */
enum {
	FN_SMURF,
   FN_OR_COUNTER,
   FN_XOR_COUNTER,
	FN_OR,
	FN_XOR,
	FN_XOR_GELIM,
	FN_AND_EQU,
	FN_OR_EQU,
	FN_MINMAX,
	FN_MINMAX_COUNTER,
	FN_NEG_MINMAX,
   FN_WATCHED_LIST,
	FN_INFERENCE
};

int solve();
int walkSolve();
extern t_solution_info *solution_info;
extern t_solution_info *solution_info_head;

struct TypeStateEntry {
	char cType;
	bool visited;
	int (*ApplyInferenceToState)(int nBranchVar, bool bBVPolarity, int nSmurfNumber, void **arrSmurfStates);
};

//Structures and functions for the simpleSolver
//Also used in the smurf_fpga output format.
struct SmurfStateEntry {
	char cType; //FN_SMURF
	bool visited;
	int (*ApplyInferenceToState)(int nBranchVar, bool bBVPolarity, int nSmurfNumber, void **arrSmurfStates);
	int nTransitionVar;
	double fHeurWghtofTrueTransition;
	double fHeurWghtofFalseTransition;

	void *pVarIsTrueTransition;
	void *pVarIsFalseTransition;
	//bool bVarIsSafe; //1 if nTransitionVar is safe for True transisiton, or both.
	                 //-1 if nTransitionVar is safe for False transition.
	                 //0 if nTransitionVar is not safe.
	//This is 1 if nTransitionVar should be inferred True,
	//       -1 if nTransitionVar should be inferred False,
	//        0 if nTransitionVar should not be inferred.
	void *pNextVarInThisStateGT; //There are n SmurfStateEntries linked together,
	void *pNextVarInThisStateLT; //in the structure of a heap,
	                             //where n is the number of variables in this SmurfStateEntry.
	                             //All of these SmurfStateEntries represent the same function,
	                             //but a different variable (nTransitionVar) is
	                             //highlighted for each link in the heap.
	                             //If this is 0, we have reached a leaf node.
   void *pNextVarInThisState;   //Same as above except linked linearly, instead of a heap.
                                //Used for computing the heuristic of a state.
};

struct InferenceStateEntry {
	char cType; //FN_INFERENCE
	bool visited; //Used for displaying the smurfs
	int nTransitionVar;
	bool bPolarity;
	void *pVarTransition;
};

struct WatchedListStateEntry {
	char cType; //FN_WATCHED_LIST
	bool visited; //Used for displaying the smurfs
	int nWatchedListSize;
	int *pnWatchedList;
	void *pTransition;
};

struct ORStateEntry {
	char cType; //FN_OR
	bool visited; //Used for displaying the smurfs
	int (*ApplyInferenceToState)(int nBranchVar, bool bBVPolarity, int nSmurfNumber, void **arrSmurfStates);
	int nSize;
	int *pnTransitionVars;
	bool *bPolarity;
};

struct ORCounterStateEntry {
	char cType; //FN_OR_COUNTER
	bool visited; //Used for displaying the smurfs
	int (*ApplyInferenceToState)(int nBranchVar, bool bBVPolarity, int nSmurfNumber, void **arrSmurfStates);
	int nSize;
	void *pTransition;
	ORStateEntry *pORState;	
};

struct XORStateEntry {
	char cType; //FN_XOR
	bool visited; //Used for displaying the smurfs
	int (*ApplyInferenceToState)(int nBranchVar, bool bBVPolarity, int nSmurfNumber, void **arrSmurfStates);
	int nSize;
	bool bParity;
	int *pnTransitionVars;
};

struct XORCounterStateEntry {
	char cType; //FN_XOR_COUNTER
	bool visited; //Used for displaying the smurfs
	int (*ApplyInferenceToState)(int nBranchVar, bool bBVPolarity, int nSmurfNumber, void **arrSmurfStates);
	int nSize;
	void *pTransition;
	XORStateEntry *pXORState; //For heuristic purposes
};

struct XORGElimStateEntry {
	char cType; //FN_XOR_GELIM
	bool visited; //Used for displaying the smurfs
	int (*ApplyInferenceToState)(int nBranchVar, bool bBVPolarity, int nSmurfNumber, void **arrSmurfStates);
	int nSize;
	void *pVector;
	int *pnTransitionVars;
};

//SEAN!!! Idea: Could make minmax state machine that is hooked together JUST like a minmax BDD.

struct MINMAXStateEntry {
	char cType; //FN_MINMAX
	bool visited; //Used for displaying the smurfs
	int nSize;
	int nMin;
	int nMax;
	int *pnTransitionVars;
};

struct MINMAXCounterStateEntry {
	char cType; //FN_MINMAX_COUNTER
	bool visited; //Used for displaying the smurfs
	int (*ApplyInferenceToState)(int nBranchVar, bool bBVPolarity, int nSmurfNumber, void **arrSmurfStates);
	int nVarsLeft;
	int nNumTrue; //Dynamic
	void *pTransition;
	MINMAXStateEntry *pMINMAXState;
};

struct XORGElimTableStruct {
	unsigned char *frame;
	int32_t *first_bit;
	void *mask;
	int num_vectors;
};

struct SmurfStack {
	int nVarChoiceCurrLevel; //Index to array of size nNumVars
	int nNumFreeVars;
	int nNumSmurfsSatisfied;
	int nHeuristicPlaceholder;
	int nWatchedListStackTop;
	void **arrSmurfStates;   //Pointer to array of size nNumSmurfs
	XORGElimTableStruct *XORGElimTable; //For holding the Gaussian Elimination Table.
};

struct SmurfStatesTableStruct {
	int curr_size;
	int max_size;
	void **arrStatesTable; //Pointer to a table of smurf states.
	SmurfStatesTableStruct *pNext;
};

struct ProblemState {
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
	int ***arrReverseOccurenceList; //Pointer to lists of pointers into arrVariableOccursInSmurf.
	                                //arrROL[x][y] points to the location of arrVOIS[y][?] = x
	
	// Dynamic
	int nCurrSearchTreeLevel;
	double *arrPosVarHeurWghts;       //Pointer to array of size nNumVars
	double *arrNegVarHeurWghts;       //Pointer to array of size nNumVars
	int *arrInferenceQueue;           //Pointer to array of size nNumVars (dynamically indexed by arrSmurfStack[level].nNumFreeVars
	int *arrInferenceDeclaredAtLevel; //Pointer to array of size nNumVars
	int **arrWatchedListStack;         //Pointer to array, max size is same as arrVariableOccursInSmurf
	SmurfStack *arrSmurfStack;        //Pointer to array of size nNumVars
};

extern SmurfStateEntry *TrueSimpleSmurfState;
extern ProblemState *SimpleSmurfProblemState;

void PrintSmurfStateEntry(SmurfStateEntry *ssEntry);
void PrintAllSmurfStateEntries();
int Init_SimpleSmurfSolver();
int simpleSolve();
extern int smurfs_share_paths;

#endif
