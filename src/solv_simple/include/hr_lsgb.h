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

#ifndef HR_SIMP_LSGB_H
#define HR_SIMP_LSGB_H

typedef struct {
	double Pos;
	double Neg;
	/* add more heuristic values here */
} HeurScores;

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

// The following structure is used for the entries in the arrORWeight
// array.
// An OR constraint is one of the form
// lit_1 \/ lit_2 \/ ... \/ lit_i.

// arrORWeight[i].fNeg is the heuristic weight of
// a transisition on an OR constraint (who is of length i)
// which sets one of the literals to false.

// arrORWeight[i].fFmla is the heuristic weight of the constraint as a
// whole.
struct ORWeightStruct
{
  HWEIGHT fNeg;
  HWEIGHT fFmla;
};

//---------------------------------------------------------------------

// The following structure is used for the entries in the arrXORWeight
// array.
// An XOR constraint is one of the form
// lit_1 + lit_2 + ... + lit_i.

// arrXORWeight[i] is the heuristic weight of the constraint as a
// whole.

struct XORWeightStruct {
	HWEIGHT fFmla;
};

//---------------------------------------------------------------------

extern int (*Simple_Solver_Heuristic)();
int Simple_DC_Heuristic();
int Simple_LSGB_Heuristic();
int Simple_PMVSIDS_Heuristic();

typedef void (*CalculateStateHeuristic)(void *pState, int nCurrInfLevel);
extern CalculateStateHeuristic *arrCalculateStateHeuristic;

typedef void (*SetStateHeuristicScore)(void *pState);
extern SetStateHeuristicScore *arrSetStateHeuristicScore;

typedef double (*GetStateHeuristicScore)(void *pState);
extern GetStateHeuristicScore *arrGetStateHeuristicScore;

extern int use_poor_mans_vsids;
extern HeurScores *arrHeurScores;
extern struct ORWeightStruct *arrORWeight;
extern struct XORWeightStruct *arrXORWeight;
extern int *arrPMVSIDS;

#endif // HR_SIMP_LSGB_H
