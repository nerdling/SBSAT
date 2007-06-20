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

t_arrVarScores *arrBerkMinVarScores = NULL;

ITE_INLINE int
HrBerkMinInit()
{
   InitBerkMinHeurArrays(gnMaxVbleIndex);

   procHeurUpdate = HrBerkMinUpdate;
   procHeurFree = HrBerkMinFree;
   procHeurSelect = BerkMin_OptimizedHeuristic;
   procHeurAddLemma = AddBerkMinHeuristicInfluence;
   procHeurAddLemmaSpace = AddBerkMinSpaceHeuristicInfluence;
   procHeurRemoveLemma = RemoveBerkMinHeuristicInfluence;

   arrBerkMinVarScores = (t_arrVarScores*)ite_calloc(gnMaxVbleIndex, 
         sizeof(t_arrVarScores), 9, "arrBerkMinVarScores");
/*
   for(int i=0;i<nNumFuncs;i++)
   {
      for(int j=0;arrFnAndTypes[j];j++) {
         if (arrSolverFunctions[i].nType == arrFnAndTypes[j]) {
            AddBerkMinBlockHeuristicInfluence(arrSolverFunctions[i].fn_and.pLongLemma);

            for(int k=0;k<arrSolverFunctions[i].fn_and.rhsVbles.nNumElts;k++)
               AddBerkMinBlockHeuristicInfluence(arrSolverFunctions[i].fn_and.arrShortLemmas[k]);

            break;
         }
      }
   }
*/


   for(int i=1;i<gnMaxVbleIndex;i++)
   {
      arrLemmaVbleCountsPos[i] += arrAFS[i].nNumOneAFS;
      arrLemmaVbleCountsNeg[i] += arrAFS[i].nNumOneAFS;
   }

   //return HrBerkMinUpdate();
   return NO_ERROR;
}

ITE_INLINE int
HrBerkMinFree()
{
   ite_free((void**)&arrBerkMinVarScores);
   DeleteBerkMinHeurArrays();
   return NO_ERROR;
}

ITE_INLINE int
HrBerkMinUpdate()
{

	if (ite_counters[NUM_BACKTRACKS] % 1000 == 0)
	{
		//divide counters by 4
		for(int i = 1; i < gnMaxVbleIndex; i++)
		{
			//arrBerkMinVarScores[i].pos = arrBerkMinVarScores[i].pos / 4;
			//arrBerkMinVarScores[i].neg = arrBerkMinVarScores[i].neg / 4;
			arrLemmaVbleCountsPos[i] = arrLemmaVbleCountsPos[i] / 4;
			arrLemmaVbleCountsNeg[i] = arrLemmaVbleCountsNeg[i] / 4;
		}
	}
	if (0) {
		for (int i = 1; i<gnMaxVbleIndex; i++)
		  {
			  d9_printf4("%d: (pos count = %d, neg count = %d)\n", i, arrLemmaVbleCountsPos[i], arrLemmaVbleCountsNeg[i]);
			  
			  //if the variable is positive, increment .pos
			  arrBerkMinVarScores[i].pos = arrLemmaVbleCountsPos[i];
			  //arrBerkMinVarScores[i].last_count_pos = arrLemmaVbleCountsPos[i];
			  arrBerkMinVarScores[i].neg = arrLemmaVbleCountsNeg[i];
			  //arrBerkMinVarScores[i].last_count_neg = arrLemmaVbleCountsPos[i];
		  }
	}
	
   return NO_ERROR;
}


//choose HEUR_WEIGHT from (arrLemmaVbleCountsNeg[i] + arrLemmaVbleCountsPos[i])  -- but if the last clause added was a lemma, then 
// choose only from variables within that lemma!!  (how do i do this??)
#define HEUR_WEIGHT(x,i) (arrLemmaVbleCountsPos[i] + arrLemmaVbleCountsNeg[i]);

//Var Score
//#define HEUR_WEIGHT(x,i) (var_score[i] * (J_ONE+arrLemmaVbleCountsNeg[i]>arrLemmaVbleCountsPos[i]?arrLemmaVbleCountsNeg[i]:arrLemmaVbleCountsPos[i]))

//#define HEUR_EXTRA_OUT()  { fprintf(stderr, "%c%d (pos: %d, neg %d)\n", (*pnBranchValue==BOOL_TRUE?'+':'-'), *pnBranchAtom, arrLemmaVbleCountsPos[*pnBranchAtom], arrLemmaVbleCountsNeg[*pnBranchAtom]);}
#define HEUR_FUNCTION BerkMin_AllVarChoiceHeuristic
#define HEUR_SIGN(nBestVble, multPos, multNeg) \
  (arrLemmaVbleCountsPos[nBestVble] > arrLemmaVbleCountsNeg[nBestVble] ? \
   BOOL_TRUE:BOOL_FALSE)
#include "hr_choice.cc"

ITE_INLINE int
BerkMin_OptimizedHeuristic(int *pnBranchAtom, int *pnBranchValue)
{
   int nBestVble = -1;
   double fMaxWeight = 0.0;
   double fVbleWeight;

   HEUR_EXTRA_IN();

   //search through clauses until we find a lemma that is NOT satisifed.
   for(int nLPQ = 0; nLPQ < 3; nLPQ++){
		for(LemmaInfoStruct *pLIS = pLPQFirst[nLPQ]; pLIS != NULL; pLIS = pLIS->pLPQNext){
			LemmaBlock *pLemma = pLIS->pLemma;
			
			assert(!pLIS->nLemmaCameFromSmurf);
			//if lemma is not satisfied then choose the variable in this lemma w/ highest weight.
			if(!LemmaIsSAT(pLemma)){
				//
				// SEARCH LEMMA VARIABLES
				//
				// Initialize to first uninstantiated variable.
				int nLemmaVar = 0, nLemmaIndex = 1;
				for (; nLemmaVar < pLIS->pLemma->arrLits[0]; nLemmaVar++, nLemmaIndex++){
					if(nLemmaIndex == LITS_PER_LEMMA_BLOCK) {
						nLemmaIndex = 0;
						pLemma = pLemma->pNext;
					}
					
					int l = abs(pLemma->arrLits[nLemmaIndex]);
					if (arrSolution[l] == BOOL_UNKNOWN){
						nBestVble = l;
						fMaxWeight = HEUR_WEIGHT(arrHeurScores[l], l);
						break;
					}
				}
				
				assert (nBestVble >= 0);
				// Search through the remaining uninstantiated lemma variables.
				for (; nLemmaVar<pLIS->pLemma->arrLits[0]; nLemmaVar++, nLemmaIndex++){
					if(nLemmaIndex == LITS_PER_LEMMA_BLOCK) {
						nLemmaIndex = 0;
						pLemma = pLemma->pNext;
					}
					int l = abs(pLemma->arrLits[nLemmaIndex]);
					if (arrSolution[l] == BOOL_UNKNOWN){
						fVbleWeight = HEUR_WEIGHT(arrHeurScores[l], l);
						if (HEUR_COMPARE(fVbleWeight, fMaxWeight)){
							fMaxWeight = fVbleWeight;
							nBestVble = l;
						}
					}
				}
				goto ReturnHeuristicResult;
			}
		}
   }
	
   //if all lemmas are satisfied then call BerkMin_AllVarChoiceHeuristic

   HEUR_EXTRA_OUT();
   
	return BerkMin_AllVarChoiceHeuristic(pnBranchAtom, pnBranchValue);

   //return values.

 ReturnHeuristicResult:
   assert (arrSolution[nBestVble] == BOOL_UNKNOWN);
   *pnBranchAtom = nBestVble;
   if (arrVarTrueInfluences)
     *pnBranchValue = HEUR_SIGN(nBestVble, arrVarTrueInfluences[nBestVble], (1-arrVarTrueInfluences[nBestVble]));
   else
     *pnBranchValue = HEUR_SIGN(nBestVble, 1, 1);

   return NO_ERROR;

}

#undef HEUR_WEIGHT
#undef HEUR_SIGN
#undef HEUR_FUNCTION
