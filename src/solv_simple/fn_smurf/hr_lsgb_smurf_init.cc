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

ITE_INLINE double LSGBSumNodeWeights(void *pState) {
   SmurfStateEntry *pSmurfState = (SmurfStateEntry *)pState;
	if (pSmurfState == pTrueSimpleSmurfState) return JHEURISTIC_K_TRUE;
	
	int num_variables = 0;
	double fTotalTransitions = 0.0;
	while(pSmurfState!=NULL) {
		if (!pSmurfState->cType == FN_SMURF) return 0;
		num_variables++;
		/* ----- POSITIVE TRANSITION ------ */
		fTotalTransitions += pSmurfState->fHeurWghtofTrueTransition;
		
		/* ----- NEGATIVE TRANSITION ------ */
		fTotalTransitions += pSmurfState->fHeurWghtofFalseTransition;
		
		pSmurfState = (SmurfStateEntry *)pSmurfState->pNextVarInThisState;
	}
	return fTotalTransitions / (((double)num_variables) * 2.0 * JHEURISTIC_K);
}

ITE_INLINE double LSGBGetHeurScoreTransition(SmurfStateEntry *pState, bool bPolarity) {
	int num_inferences = 0;

	void *pNextState = bPolarity?pState->pVarIsTrueTransition:pState->pVarIsFalseTransition;
	while(pNextState != NULL && ((TypeStateEntry *)pNextState)->cType == FN_INFERENCE) {
		num_inferences++;
		pNextState = ((InferenceStateEntry *)pNextState)->pVarTransition;
	}

	if(pNextState == NULL) return JHEURISTIC_K_UNKNOWN; //This can happen if smurfs are built lazily

	double fInferenceWeights = JHEURISTIC_K_INF * (double)num_inferences;	
   return fInferenceWeights + arrGetStateHeuristicScore[(int)((TypeStateEntry *)pNextState)->cType](pNextState);
}

void LSGBSmurfSetHeurScore(void *pState) {
	SmurfStateEntry *pSmurfState = (SmurfStateEntry *)pState;
	if (pSmurfState == pTrueSimpleSmurfState) return;
	
	while(pSmurfState!=NULL) {
		if (!pSmurfState->cType == FN_SMURF) return;
		/* ----- POSITIVE TRANSITION ------ */
		pSmurfState->fHeurWghtofTrueTransition = LSGBGetHeurScoreTransition(pSmurfState, BOOL_TRUE);
		
		/* ----- NEGATIVE TRANSITION ------ */
		pSmurfState->fHeurWghtofFalseTransition = LSGBGetHeurScoreTransition(pSmurfState, BOOL_FALSE);
		
		pSmurfState = (SmurfStateEntry *)pSmurfState->pNextVarInThisState;
	}
}
