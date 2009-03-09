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

int nXORMaxRHSSize = 1;

struct XORWeightStruct *arrXORWeight = NULL;

//---------------------------------------------------------------

ITE_INLINE void LSGBXORStateSetHeurScore(void *pState) {
	XORStateEntry *pXORState = (XORStateEntry *)pState;
	int size = pXORState->nSize;

	HWEIGHT K = JHEURISTIC_K;
	
	if(size > nXORMaxRHSSize) {
		size+=20;
		if(nXORMaxRHSSize == 1) {
			arrXORWeight = (XORWeightStruct *)ite_calloc(size+1, sizeof(XORWeightStruct), 9, "arrXORWeight");
			arrXORWeight[0].fFmla = 0.0;
			arrXORWeight[1].fFmla = JHEURISTIC_K_TRUE+JHEURISTIC_K_INF;
			nXORMaxRHSSize = 1;
		} else
		  arrXORWeight = (XORWeightStruct *)ite_recalloc(arrXORWeight, nXORMaxRHSSize, size+1, sizeof(XORWeightStruct), 9, "arrXORWeight");

		for (int i = nXORMaxRHSSize+1; i <= size; i++) {
			arrXORWeight[i].fFmla = arrXORWeight[i - 1].fFmla/K;
		}
		nXORMaxRHSSize = size;
	}
}

ITE_INLINE void LSGBXORCounterStateSetHeurScore(void *pState) {
	LSGBXORStateSetHeurScore(((XORCounterStateEntry *)pState)->pXORState);
}

ITE_INLINE double LSGBXORStateGetHeurScore(void *pState) {
	return arrXORWeight[2].fFmla;
}

ITE_INLINE double LSGBXORGetHeurScoreTrans(XORStateEntry *pState) {
	return arrXORWeight[1].fFmla;
}

ITE_INLINE double LSGBXORCounterStateGetHeurScore(void *pState) {
	return arrXORWeight[((XORCounterStateEntry *)pState)->nSize].fFmla;
}

ITE_INLINE double LSGBXORCounterGetHeurScoreTrans(XORCounterStateEntry *pState) {
	return arrXORWeight[pState->nSize-1].fFmla;
}

ITE_INLINE double LSGBarrXORWeight(void *pState) {
	return arrXORWeight[((XORGElimStateEntry *)pState)->nSize].fFmla;
}

ITE_INLINE double LSGBarrXORWeightTrans(int nSize) {
	return arrXORWeight[nSize-1].fFmla;
}

ITE_INLINE void LSGBXORFree() {
	if(arrXORWeight!=NULL) ite_free((void **)&arrXORWeight);
}

