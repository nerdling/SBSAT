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

struct ORWeightStruct *arrORWeight = NULL;

int nORMaxRHSSize = 1;

//---------------------------------------------------------------

ITE_INLINE void LSGBORStateSetHeurScore(void *pState) {
	ORStateEntry *pORState = (ORStateEntry *)pState;
	int size = pORState->nSize;

	HWEIGHT K = JHEURISTIC_K;
	
	if(size > nORMaxRHSSize) {
		size+=20;
		if(nORMaxRHSSize == 1) {
			arrORWeight = (ORWeightStruct*)ite_calloc(size+1, sizeof(ORWeightStruct), 9, "arrORWeight");
			arrORWeight[2].fNeg = JHEURISTIC_K_TRUE+JHEURISTIC_K_INF;
			arrORWeight[2].fFmla = (arrORWeight[2].fNeg + JHEURISTIC_K_TRUE) / (2*K);
			nORMaxRHSSize = 2;
		} else
		  arrORWeight = (ORWeightStruct*)ite_recalloc(arrORWeight, nORMaxRHSSize, size+1, sizeof(ORWeightStruct), 9, "arrORWeight");

		for (int i = nORMaxRHSSize+1; i <= size; i++) {
			arrORWeight[i].fNeg = arrORWeight[i - 1].fFmla;
			arrORWeight[i].fFmla = (arrORWeight[i].fNeg + JHEURISTIC_K_TRUE) / (2*K);
		}
		nORMaxRHSSize = size;
	}
}

ITE_INLINE void LSGBORCounterStateSetHeurScore(void *pState) {
	LSGBORStateSetHeurScore(((ORCounterStateEntry *)pState)->pORState);
}

ITE_INLINE double LSGBORStateGetHeurScore(void *pState) {
	return arrORWeight[2].fFmla;
}

ITE_INLINE double LSGBORGetHeurNeg(ORStateEntry *pState) {
	return arrORWeight[2].fNeg;
}

ITE_INLINE double LSGBORCounterStateGetHeurScore(void *pState) {
	return arrORWeight[((ORCounterStateEntry *)pState)->nSize].fFmla;
}

ITE_INLINE double LSGBORCounterGetHeurNeg(ORCounterStateEntry *pState) {
	return arrORWeight[pState->nSize].fNeg;
}

ITE_INLINE void LSGBORFree() {
	if(arrORWeight!=NULL) ite_free((void **)&arrORWeight);
}
