/* =========FOR INTERNAL USE ONLY. NO DISTRIBUTION PLEASE ========== */

/*********************************************************************
 Copyright 1999-2004, University of Cincinnati.  All rights reserved.
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

int solve();
int walkSolve();
extern t_solution_info *solution_info;
extern t_solution_info *solution_info_head;

//Structures and functions for simpleSolve
//Used in the smurf_fpga output format.
struct SmurfStateEntry {
	// Static
	int nTransitionVar;
	int nVarIsTrueTransition;
	int nVarIsFalseTransition;
	double nHeurWghtofTrueTransition;
	double nHeurWghtofFalseTransition;
	int nVarIsSafe; //1 if nTransitionVar is safe for True transisiton, or both.
	                //-1 if nTransitionVar is safe for False transition.
	                //0 if nTransitionVar is not safe.
	int nVarIsAnInference;
	//This is 1 if nTransitionVar should be inferred True,
	//       -1 if nTransitionVar should be inferred False,
	//        0 if nTransitionVar should not be inferred.
	int nNextVarInThisState; //There are n SmurfStateEntries linked together,
	                         //where n is the number of variables in this SmurfStateEntry.
	                         //All of these SmurfStateEntries represent the same function,
	                         //but a different variable (nTransitionVar) is
	                         //highlighted for each link in the list.
	                         //If this is 0, we have reached the end of the list.
};

struct SmurfStack {
	int nNumFreeVars;
	int *arrSmurfStates;             //Pointer to array of size nNumSmurfs
};

struct ProblemState {
	// Static
	int nNumSmurfs;
	int nNumVars;
	int nNumSmurfStateEntries;
	SmurfStateEntry *arrSmurfStatesTable; //Pointer to the table of all smurf states.
	                                      //Will be of size nNumSmurfStateEntries
	int **arrVariableOccursInSmurf; //Pointer to lists of Smurfs, indexed by variable number, that contain that variable.
	                                //Max size would be nNumSmurfs * nNumVars, but this would only happen if every
	                                //Smurf contained every variable. Each list is terminated by a -1 element.
	// Dynamic
	int nCurrSearchTreeLevel;
	double *arrPosVarHeurWghts; //Pointer to array of size nNumVars
	double *arrNegVarHeurWghts; //Pointer to array of size nNumVars
	int *arrInferenceQueue;  //Pointer to array of size nNumVars (dynamically indexed by arrSmurfStack[level].nNumFreeVars
	int *arrInferenceDeclaredAtLevel; //Pointer to array of size nNumVars
	SmurfStack *arrSmurfStack; //Pointer to array of size nNumVars
};

extern SmurfStateEntry *TrueSimpleSmurfState;
extern ProblemState *SimpleSmurfProblemState;

void PrintSmurfStateEntry(SmurfStateEntry *ssEntry);
void PrintAllSmurfStateEntries();
int Init_SimpleSmurfSolver();
int simpleSolve();
extern int smurfs_share_paths;

#endif
