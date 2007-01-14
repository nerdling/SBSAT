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

ITE_INLINE TransitionAu *
AddStateTransitionAu(SmurfAuState *pSmurfAuState,
                   int i,
                   int nVble,
                   int nValueOfVble,
						 int nAutarkyVble,
                   BDDNodeStruct *pFuncEvaled,
                   SmurfAuState *pSmurfAuStateOfEvaled
                   )
{
	//Make sure the state is not transitioning to itself.
	assert(pSmurfAuState != pSmurfAuStateOfEvaled);
	
	TransitionAu *pTransitionAu = FindOrAddTransitionAu(pSmurfAuState, i, nVble, nValueOfVble, nAutarkyVble);
	
	assert(pTransitionAu->pNextState == NULL);
	//assert(pTransitionAu->pState == NULL);
	
	pTransitionAu->pNextState = pSmurfAuStateOfEvaled;
	//pTransitionAu->pState = pSmurfAuState;
	
	infer *head = pFuncEvaled->inferences;
	int *ptr = NULL;
	if(head != NULL) {
		//There should never be an inference except on the autarky variable.
		//If this function has an inference, it must be the autarky variable.
		ptr = (int*)ite_calloc(1, sizeof(int), 9, "inferences"); //Just 1 inference.
		pTransitionAu->positiveInferences.arrElts = ptr;
		pTransitionAu->negativeInferences.arrElts = ptr;
      pTransitionAu->positiveInferences.nNumElts = 0;
      pTransitionAu->negativeInferences.nNumElts = 0;
		
		while(head != NULL) {
			if (head->nums[1] == 0) {
				if (head->nums[0] > 0) {
					if(arrIte2SolverVarMap[head->nums[0]] == nAutarkyVble) {
						ptr[0] = nAutarkyVble;
						pTransitionAu->positiveInferences.nNumElts = 1;
						pTransitionAu->negativeInferences.nNumElts = 0;
						break;
					}
				} else if (head->nums[0] < 0) {
					if(arrIte2SolverVarMap[-head->nums[0]] == nAutarkyVble) {
						ptr[0] = nAutarkyVble;
						pTransitionAu->positiveInferences.nNumElts = 0;
						pTransitionAu->negativeInferences.nNumElts = 1;
						break;
					}
				}
			}
			head = head->next;
		}
		//If head == NULL that means the autarky BDD was able to make an inference
		//that did not involve the autarky variable. Or an equivalence was made
		//like {1=2}.
	}
   if (pTransitionAu->positiveInferences.nNumElts ||
         pTransitionAu->negativeInferences.nNumElts)
	  pTransitionAu->pNextState = pTrueSmurfAuState;
   return pTransitionAu;
}


ITE_INLINE TransitionAu *
FindTransitionAuDebug (SmurfAuState * pState, int i, int nVble, int nVbleValue, int nAutarkyVble)
{
  assert (pState != pTrueSmurfAuState);
  assert (pState->vbles.nNumElts > i);
  assert (pState->vbles.arrElts[i] == nVble);
  assert (nVbleValue == BOOL_TRUE || nVbleValue == BOOL_FALSE);
  assert (pState->arrTransitionAus);
  return pState->arrTransitionAus + 2 * i + nVbleValue;
}

