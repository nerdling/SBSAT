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
	FN_INFERENCE,
	FN_OR,
   FN_OR_COUNTER,
	FN_XOR,
   FN_XOR_COUNTER,
   FN_AND_EQU,
	FN_OR_EQU,
	FN_MINMAX,
	FN_NEG_MINMAX
};

int solve();
int walkSolve();
extern t_solution_info *solution_info;
extern t_solution_info *solution_info_head;

struct TypeStateEntry {
	char cType;
};

//Structures and functions for simpleSolve
//Used in the smurf_fpga output format.
struct SmurfStateEntry {
	char cType; //FN_SMURF
	int nTransitionVar;
	void *pVarIsTrueTransition;
	void *pVarIsFalseTransition;
	double fHeurWghtofTrueTransition;
	double fHeurWghtofFalseTransition;
	bool bVarIsSafe; //1 if nTransitionVar is safe for True transisiton, or both.
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
	int nTransitionVar;
	void *pVarTransition;
	bool bPolarity;
};

struct ORStateEntry {
	char cType; //FN_OR
	int *nTransitionVars;
	bool *bPolarity;
	int nSize;
};

struct ORCounterStateEntry {
	char cType; //FN_OR_COUNTER
	void *pTransition;
	int nSize;
	ORStateEntry *pORState;	
};

struct XORStateEntry {
	char cType; //FN_OR
	int *nTransitionVars;
	bool bParity;
	int nSize;
};

struct XORCounterStateEntry {
	char cType; //FN_OR_COUNTER
	void *pTransition;
	int nSize;
	XORStateEntry *pXORState; //For heuristic purposes
};

struct SmurfStack {
	int nNumFreeVars;
	int nHeuristicPlaceholder;
	int nVarChoiceCurrLevel; //Index to array of size nNumVars
	void **arrSmurfStates;   //Pointer to array of size nNumSmurfs
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
	// Dynamic
	int nCurrSearchTreeLevel;
	double *arrPosVarHeurWghts;       //Pointer to array of size nNumVars
	double *arrNegVarHeurWghts;       //Pointer to array of size nNumVars
	int *arrInferenceQueue;           //Pointer to array of size nNumVars (dynamically indexed by arrSmurfStack[level].nNumFreeVars
	int *arrInferenceDeclaredAtLevel; //Pointer to array of size nNumVars
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
