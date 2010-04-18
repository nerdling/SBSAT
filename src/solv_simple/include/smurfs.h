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

//This file must be C compliant for interface with PicoSAT

#ifndef SIMPLE_SMURFS_H
#define SIMPLE_SMURFS_H

/* list of possible smurf function types */
enum {
   FN_FREE_STATE, //0
   FN_SMURF,
   FN_OR_COUNTER,
   FN_XOR_COUNTER,
	FN_MINMAX_COUNTER,
   FN_NEGMINMAX_COUNTER,
   FN_OR,
	FN_XOR,
	FN_MINMAX,
	FN_NEGMINMAX,
	FN_XOR_GELIM,
   FN_INFERENCE,
   FN_TYPE_STATE,
   FN_WATCHED_SMURF,
   FN_AND_EQU,
	FN_OR_EQU,
   NUM_SMURF_TYPES
};

typedef struct TypeStateEntry {
	unsigned cType:5;
	unsigned visited:1;
	unsigned pStateOwner:26;
	void *pPreviousState; //Used during lemma creation.
 	int32_t nLemmaLiteral; //Used during lemma creation.
} TypeStateEntry;

//Structures and functions for the simpleSolver
//Also used in the smurf_fpga output format.
typedef struct SmurfStateEntry {
	unsigned cType:5; //FN_SMURF || FN_WATCHED_SMURF
	unsigned visited:1;
	unsigned pStateOwner:26;
	void *pPreviousState; //Used during lemma creation.
	int32_t nLemmaLiteral; //Used during lemma creation.

	int32_t nTransitionVar;
	double fHeurWghtofTrueTransition;
	double fHeurWghtofFalseTransition;

	void *pVarIsTrueTransition;
	void *pVarIsFalseTransition;
	//bool bVarIsSafe; //1 if nTransitionVar is safe for True transisiton, or both.
	                   //-1 if nTransitionVar is safe for False transition.
	                   //0 if nTransitionVar is not safe.

	void *pNextVarInThisStateGT; //There are n SmurfStateEntries linked together,
	void *pNextVarInThisStateLT; //in the structure of a heap,
	                             //where n is the number of variables in this SmurfStateEntry.
	                             //All of these SmurfStateEntries represent the same function,
	                             //but a different variable (nTransitionVar) is
	                             //highlighted for each link in the heap.
	                             //If this is 0, we have reached a leaf node.
   void *pNextVarInThisState;   //Same as above except linked linearly, instead of a heap.
	BDDNode *pSmurfBDD; //Used when building smurfs on the fly.
} SmurfStateEntry;

typedef struct InferenceStateEntry {
	unsigned cType:5; //FN_INFERENCE
	unsigned visited:1; //Used for displaying the smurfs.

	unsigned pStateOwner:26;
	void *pPreviousState; //Used during lemma creation.
	int32_t nLemmaLiteral; //Used during lemma creation.

	unsigned nTransitionVar:26;
	unsigned bPolarity:1;
	void *pVarTransition;
	BDDNode *pInferenceBDD; //Used when building smurfs on the fly.
} InferenceStateEntry;

struct ORCounterStateEntry;

typedef struct ORStateEntry {
	unsigned cType:5; //FN_OR
	unsigned visited:1; //Used for displaying the smurfs.
	unsigned pStateOwner:26;
	void *pPreviousState; //Used during lemma creation.
	int32_t nLemmaLiteral; //Used during lemma creation.

	int16_t nSize;
	int *pnTransitionVars;
	bool *bPolarity;
	ORCounterStateEntry *pORCounterState;
   BDDNode *pORStateBDD;
} ORStateEntry;

typedef struct ORCounterStateEntry {
	unsigned cType:5; //FN_OR_COUNTER
	unsigned visited:1; //Used for displaying the smurfs.
	unsigned pStateOwner:26;
	void *pPreviousState; //Used during lemma creation.
	int32_t nLemmaLiteral; //Used during lemma creation.

	int16_t nSize;
	void *pTransition;
	ORStateEntry *pORState;	
} ORCounterStateEntry;

typedef struct XORStateEntry {
	unsigned cType:5; //FN_XOR
	unsigned visited:1; //Used for displaying the smurfs.
	unsigned pStateOwner:26;
	void *pPreviousState; //Used during lemma creation.
	int32_t nLemmaLiteral; //Used during lemma creation.

	unsigned nSize:26;
	unsigned bParity:1;
	int *pnTransitionVars;
   BDDNode *pXORStateBDD;
} XORStateEntry;

typedef struct XORCounterStateEntry {
	unsigned cType:5; //FN_XOR_COUNTER
	unsigned visited:1; //Used for displaying the smurfs.
	unsigned pStateOwner:26;
	void *pPreviousState; //Used during lemma creation.
	int32_t nLemmaLiteral; //Used during lemma creation.
	int32_t nSize;
	void *pTransition;
	XORStateEntry *pXORState; //For heuristic purposes
} XORCounterStateEntry;

typedef struct XORGElimStateEntry {
	unsigned cType:5; //FN_XOR_GELIM
	unsigned visited:1; //Used for displaying the smurfs.
	unsigned pStateOwner:26;
	void *pPreviousState; //Used during lemma creation.
	int32_t nLemmaLiteral; //Used during lemma creation.

	int32_t nSize;
	void *pVector;
	int *pnTransitionVars;
	BDDNode *pXORGElimStateBDD;
} XORGelimStateEntry;

//SEAN!!! Idea: Could make minmax state machine that is hooked together JUST like a minmax BDD.

typedef struct MINMAXStateEntry {
	unsigned cType:5; //FN_MINMAX
	unsigned visited:1; //Used for displaying the smurfs.
	int16_t nSize;
	int16_t nMin;
	int16_t nMax;
	int *pnTransitionVars;
   BDDNode *pMINMAXStateBDD;
} MINMAXStateEntry;

typedef struct MINMAXCounterStateEntry {
	unsigned cType:5; //FN_MINMAX_COUNTER
	unsigned visited:1; //Used for displaying the smurfs.
	unsigned pStateOwner:26;
	void *pPreviousState; //Used during lemma creation.
	int32_t nLemmaLiteral; //Used during lemma creation.

	int16_t nVarsLeft;
	int16_t nNumTrue; //Dynamic
	void *pTransition;
	MINMAXStateEntry *pMINMAXState;
} MINMAXCounterStateEntry;

typedef struct NEGMINMAXStateEntry {
	unsigned cType:5; //FN_NEGMINMAX
	unsigned visited:1; //Used for displaying the smurfs.
	int16_t nSize;
	int16_t nMin;
	int16_t nMax;
	int *pnTransitionVars;
   BDDNode *pNEGMINMAXStateBDD;
} NEGMINMAXStateEntry;

typedef struct NEGMINMAXCounterStateEntry {
	unsigned cType:5; //FN_NEGMINMAX_COUNTER
	unsigned visited:1; //Used for displaying the smurfs.
	unsigned pStateOwner:26;
	void *pPreviousState; //Used during lemma creation.
	int32_t nLemmaLiteral; //Used during lemma creation.

	int16_t nVarsLeft;
	int16_t nNumTrue; //Dynamic
	void *pTransition;
	NEGMINMAXStateEntry *pNEGMINMAXState;
} NEGMINMAXCounterStateEntry;

typedef struct XORGElimTableStruct {
	unsigned char *frame;
	int32_t *first_bit;
	void *mask;
	int32_t num_vectors;
	int32_t no_funcs;
} XORGElimTableStruct;

typedef struct SmurfStack {
	int32_t nVarChoiceCurrLevel; //Index to array of size nNumVars
	int32_t nNumFreeVars;
	int32_t nNumSmurfsSatisfied;
	int32_t nHeuristicPlaceholder;
	int32_t nIdentifier; //Used for uniqueness. Helpful for printing the search tree.
	void **arrSmurfStates;   //Pointer to array of size nNumSmurfs
	XORGElimTableStruct *XORGElimTable; //For holding the Gaussian Elimination Table.
} SmurfStack;

typedef struct SmurfStatesTableStruct SmurfStatesTableStruct;
struct SmurfStatesTableStruct {
	int32_t curr_size;
	int32_t max_size;
	void **arrStatesTable; //Pointer to a table of smurf states.
	SmurfStatesTableStruct *pNext;
} ;

extern SmurfStateEntry *TrueSimpleSmurfState;

void PrintSmurfStateEntry(SmurfStateEntry *ssEntry);

#endif
