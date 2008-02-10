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

struct OREqFalseWghtStruct *arrOREqFalseWght = NULL;
struct OREqWghtStruct *arrOREqWght = NULL;
double *arrOREqWghtCx = NULL;
double *arrOREqWghtCe = NULL;
double *arrOREqWghtCt = NULL; // lhs is false

void LSGBORGetHeurScores(int nFnId);

int nORMaxRHSSize = 1;

/*void
HrLSGBFnORInit()
{
   for(int j=0; arrFnORTypes[j] != 0; j++)
   {
      int i=arrFnORTypes[j];
      procHeurGetScores[i] = LSGBORGetHeurScores;
      procHeurUpdateFunctionInfEnd[i] = LSGBORUpdateFunctionInfEnd;
   }
}
*/

// Call this one after all functions are created (to have nORMaxRHSSize)
ITE_INLINE void
LSGBORInitHeuristicTables() {
   // Initialize some tables for computing heuristic values
   // for variables that are mentioned in special functions.

   HWEIGHT K = JHEURISTIC_K;

   // We need nORMaxRHSSize to be at least one to insure that entry 1 exists
   // and we don't overrun the arrays.

   arrOREqFalseWght = (OREqFalseWghtStruct*)ite_calloc(nORMaxRHSSize + 1, sizeof(OREqFalseWghtStruct), 
          9, "arrOREqFalseWght");

   arrOREqWght = (OREqWghtStruct*)ite_calloc(nORMaxRHSSize + 1,  sizeof(OREqWghtStruct),
          9, "arrOREqWght");

   // index is the CURRENT number of unknowns!
   
   // .fNeg = 0; should be !!! JHEURISTIC_K_TRUE !! for all indeces
   // never happens!
   //arrOREqFalseWght[1].fPos = 2*JHEURISTIC_K_TRUE+1*JHEURISTIC_K_INF;
   arrOREqWght[0].fFmla = (1*JHEURISTIC_K_INF+JHEURISTIC_K_TRUE);
   arrOREqFalseWght[1].fPos = 2*JHEURISTIC_K_TRUE+1*JHEURISTIC_K_INF;
   arrOREqFalseWght[1].fFmla = arrOREqFalseWght[1].fPos; ///(2*K);

   for (int i = 2; i <= nORMaxRHSSize; i++) {
      arrOREqFalseWght[i].fPos = JHEURISTIC_K_TRUE+arrOREqFalseWght[i - 1].fFmla;
      arrOREqFalseWght[i].fFmla = arrOREqFalseWght[i].fPos / (2 * K);
   }

   // now fFmla is a weight of the state of PLAIN OR

   for (int i = 1; i <= nORMaxRHSSize; i++) {
      arrOREqWght[i].fLHSPos = i*JHEURISTIC_K_INF+JHEURISTIC_K_TRUE;
      arrOREqWght[i].fLHSNeg = arrOREqFalseWght[i].fFmla;
      arrOREqWght[i].fRHSPos = arrOREqWght[i - 1].fFmla;
      arrOREqWght[i].fRHSNeg = 1*JHEURISTIC_K_INF+JHEURISTIC_K_TRUE;

      // HERE I'm changing fFmla to OR_EQU
      arrOREqWght[i].fFmla
		  = (arrOREqWght[i].fLHSPos
			  + arrOREqWght[i].fLHSNeg
			  + i * arrOREqWght[i].fRHSPos
			  + i * arrOREqWght[i].fRHSNeg) / (2 * (i+1) * K);
   }

   for(int i=0;i<=nORMaxRHSSize;i++) {
      d9_printf5("%d: PLAIN OR fFmla %f, fPos %f fNeg %f\n", i, 
					  arrOREqFalseWght[i].fFmla,
					  arrOREqFalseWght[i].fPos,
					  arrOREqFalseWght[i].fNeg);
      d9_printf5("%d: EQ OR fFmla %f, fLHSPos %f fLHSNeg %f ", i,
					  arrOREqWght[i].fFmla,
					  arrOREqWght[i].fLHSPos,
					  arrOREqWght[i].fLHSNeg);
      d9_printf3("fRHSPos %f fRHSNeg %f\n", 
					  arrOREqWght[i].fRHSPos,
					  arrOREqWght[i].fRHSNeg);
   }
}

ITE_INLINE void LSGBORFreeHeuristicTables() {
   ite_free((void**)&arrOREqFalseWght);
   ite_free((void**)&arrOREqWght);
}

//---------------------------------------------------------------

ITE_INLINE void LSGBORStateSetHeurScores(ORStateEntry *pState) {
	int size = pState->nSize;

	HWEIGHT K = JHEURISTIC_K;
	
	if(size > nORMaxRHSSize) {
		size+=20;
		if(nORMaxRHSSize == 1) {
			arrOREqFalseWght = (OREqFalseWghtStruct*)ite_calloc(size+1, sizeof(OREqFalseWghtStruct), 9, "arrOREqFalseWght");
			arrOREqFalseWght[2].fPos = JHEURISTIC_K_TRUE+JHEURISTIC_K_INF;
			arrOREqFalseWght[2].fFmla = (arrOREqFalseWght[2].fPos + JHEURISTIC_K_TRUE) / (2*K);
			nORMaxRHSSize = 2;
		} else
		  arrOREqFalseWght = (OREqFalseWghtStruct*)ite_recalloc(arrOREqFalseWght, nORMaxRHSSize, size+1, sizeof(OREqFalseWghtStruct), 9, "arrOREqFalseWght");

		for (int i = nORMaxRHSSize+1; i <= size; i++) {
			arrOREqFalseWght[i].fPos = arrOREqFalseWght[i - 1].fFmla;
			arrOREqFalseWght[i].fFmla = (arrOREqFalseWght[i].fPos + JHEURISTIC_K_TRUE) / (2*K);
		}
		nORMaxRHSSize = size;
	}
}

ITE_INLINE void LSGBORCounterStateSetHeurScores(ORCounterStateEntry *pState) {
	LSGBORStateSetHeurScores(pState->pORState);
}

ITE_INLINE double LSGBORGetHeurScore(ORStateEntry *pState) {
	return arrOREqFalseWght[2].fFmla;
}

ITE_INLINE double LSGBORGetHeurPos(ORStateEntry *pState) {
	return arrOREqFalseWght[2].fPos;
}

ITE_INLINE double LSGBORCounterGetHeurScore(ORCounterStateEntry *pState) {
	return arrOREqFalseWght[pState->nSize].fFmla;
}

ITE_INLINE double LSGBORCounterGetHeurPos(ORCounterStateEntry *pState) {
	return arrOREqFalseWght[pState->nSize].fPos;
}
