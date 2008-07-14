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

#include "sbsat.h"
#include "sbsat_solver.h"
#include "solver.h"

ITE_INLINE double LSGBSumNodeWeights(SmurfStateEntry *pState) {
	if (pState == pTrueSimpleSmurfState) return JHEURISTIC_K_TRUE;
	
	int num_variables = 0;
	double fTotalTransitions = 0.0;
	while(pState!=NULL) {
		if (pState->cType != FN_SMURF) return 0;
		num_variables++;
		/* ----- POSITIVE TRANSITION ------ */
		fTotalTransitions += pState->fHeurWghtofTrueTransition;
		
		/* ----- NEGATIVE TRANSITION ------ */
		fTotalTransitions += pState->fHeurWghtofFalseTransition;
		
		pState = (SmurfStateEntry *)pState->pNextVarInThisState;
	}
	return fTotalTransitions / (((double)num_variables) * 2.0 * JHEURISTIC_K);
}

ITE_INLINE double LSGBGetHeurScoreTransition(SmurfStateEntry *pState, bool bPolarity) {
	int num_inferences = 0;

	void *pNextState = bPolarity?pState->pVarIsTrueTransition:pState->pVarIsFalseTransition;
	if(((TypeStateEntry *)pNextState)->cType == FN_WATCHED_LIST) { //Skip the possible FN_WATCHED_LIST State
		pNextState = ((WatchedListStateEntry *)pNextState)->pTransition;
	}
	while(((TypeStateEntry *)pNextState)->cType == FN_INFERENCE) {
		num_inferences++;
		pNextState = ((InferenceStateEntry *)pNextState)->pVarTransition;
	}
	if(((TypeStateEntry *)pNextState)->cType == FN_WATCHED_LIST) { //Skip the possible FN_WATCHED_LIST State
		pNextState = ((WatchedListStateEntry *)pNextState)->pTransition;
	}
	
	double fInferenceWeights = JHEURISTIC_K_INF * num_inferences;	
	if (((TypeStateEntry *)pNextState)->cType == FN_SMURF) {
		return fInferenceWeights + LSGBSumNodeWeights((SmurfStateEntry *)pNextState);
	} else if (((TypeStateEntry *)pNextState)->cType == FN_OR) {
		return fInferenceWeights + LSGBORGetHeurScore((ORStateEntry *)pNextState);
	} else if (((TypeStateEntry *)pNextState)->cType == FN_OR_COUNTER) {
		return fInferenceWeights + LSGBORCounterGetHeurScore((ORCounterStateEntry *)pNextState);
	} else if (((TypeStateEntry *)pNextState)->cType == FN_XOR) {
		return fInferenceWeights + LSGBXORGetHeurScore((XORStateEntry *)pNextState);
	} else if (((TypeStateEntry *)pNextState)->cType == FN_XOR_COUNTER) {
		return fInferenceWeights + LSGBXORCounterGetHeurScore((XORCounterStateEntry *)pNextState);
	} else if (((TypeStateEntry *)pNextState)->cType == FN_XOR_GELIM) {
		return fInferenceWeights + LSGBarrXORWeight(((XORGElimStateEntry *)pNextState)->nSize);
	}
	return 0;
}

ITE_INLINE
void LSGBSmurfSetHeurScores(SmurfStateEntry *pState) {
	if (pState == pTrueSimpleSmurfState) return;
	
	while(pState!=NULL) {
		if (pState->cType != FN_SMURF) return;
		/* ----- POSITIVE TRANSITION ------ */
		pState->fHeurWghtofTrueTransition = LSGBGetHeurScoreTransition(pState, BOOL_TRUE);
		
		/* ----- NEGATIVE TRANSITION ------ */
		pState->fHeurWghtofFalseTransition = LSGBGetHeurScoreTransition(pState, BOOL_FALSE);
		
		pState = (SmurfStateEntry *)pState->pNextVarInThisState;
	}
}
