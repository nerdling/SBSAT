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


// ----------------------- watched lits -----------------------------

ITE_INLINE void
AddLemmaIntoWatchedLits(LemmaInfoStruct *p)
{
   //Add to watched literal 1s lemma list
   AffectedFuncsStruct *pAFS = arrAFS + p->nWatchedVble[0];
   if(p->nWatchedVblePolarity[0])
   {
      p->pNextLemma[0] = pAFS->LemmasWherePos[0].pNextLemma[0];
      p->pPrevLemma[0] = &(pAFS->LemmasWherePos[0]);
      pAFS->LemmasWherePos[0].pNextLemma[0] = p;
      if (p->pNextLemma[0]) p->pNextLemma[0]->pPrevLemma[0] = p;
		else pAFS->LemmasWherePosTail[0].pNextLemma[0] = p; //Maintain the Tail of the list.
	}
   else
   {
      p->pNextLemma[0] = pAFS->LemmasWhereNeg[0].pNextLemma[0];
      p->pPrevLemma[0] = &(pAFS->LemmasWhereNeg[0]);
      pAFS->LemmasWhereNeg[0].pNextLemma[0] = p;
      if (p->pNextLemma[0]) p->pNextLemma[0]->pPrevLemma[0] = p;
		else pAFS->LemmasWhereNegTail[0].pNextLemma[0] = p; //Maintain the Tail of the list.
   }

   //Add to watched literal 2s lemma list
   pAFS = arrAFS + p->nWatchedVble[1];
   if(p->nWatchedVblePolarity[1])
   {
      p->pNextLemma[1] = pAFS->LemmasWherePos[1].pNextLemma[1];
      p->pPrevLemma[1] = &(pAFS->LemmasWherePos[1]);
      pAFS->LemmasWherePos[1].pNextLemma[1] = p;
      if (p->pNextLemma[1]) p->pNextLemma[1]->pPrevLemma[1] = p;
		else pAFS->LemmasWherePosTail[1].pNextLemma[1] = p; //Maintain the Tail of the list.
   }
   else
   {
      p->pNextLemma[1] = pAFS->LemmasWhereNeg[1].pNextLemma[1];
      p->pPrevLemma[1] = &(pAFS->LemmasWhereNeg[1]);
      pAFS->LemmasWhereNeg[1].pNextLemma[1] = p;
      if (p->pNextLemma[1]) p->pNextLemma[1]->pPrevLemma[1] = p;
		else pAFS->LemmasWhereNegTail[1].pNextLemma[1] = p; //Maintain the Tail of the list.
   }
}

ITE_INLINE void
RemoveLemmaFromWatchedLits(LemmaInfoStruct *pLemmaInfo)
{
	// Remove it from the lists which it is in based on
   // its watched variables.

	AffectedFuncsStruct *pAFS = arrAFS + pLemmaInfo->nWatchedVble[0];
	
   // Watched variable 1 list --
   if (pLemmaInfo->pPrevLemma[0])
   {
		pLemmaInfo->pPrevLemma[0]->pNextLemma[0]
		  = pLemmaInfo->pNextLemma[0];
		if (pLemmaInfo->pNextLemma[0]) {
			pLemmaInfo->pNextLemma[0]->pPrevLemma[0]
			  = pLemmaInfo->pPrevLemma[0];
		} else { //This lemma is the Tail of one of the lemma lists
			if (pAFS->LemmasWherePosTail[0].pNextLemma[0] == pLemmaInfo)
			  pAFS->LemmasWherePosTail[0].pNextLemma[0] = pLemmaInfo->pPrevLemma[0];
			else if (pAFS->LemmasWhereNegTail[0].pNextLemma[0] == pLemmaInfo)
			  pAFS->LemmasWhereNegTail[0].pNextLemma[0] = pLemmaInfo->pPrevLemma[0];
			else {
				fprintf(stderr, "LemmaInfoStruct Tail pointer is invalid\n");
				assert(0);
			}   
		}
	} else {
		if (pLemmaInfo->pNextLemma[0]) {
			pLemmaInfo->pNextLemma[0]->pPrevLemma[0]
			  = pLemmaInfo->pPrevLemma[0];
		}
	}

	pAFS = arrAFS + pLemmaInfo->nWatchedVble[1];
	
   // Watched variable 2 list --
   if (pLemmaInfo->pPrevLemma[1])
   {
      pLemmaInfo->pPrevLemma[1]->pNextLemma[1]
		  = pLemmaInfo->pNextLemma[1];
		if (pLemmaInfo->pNextLemma[1]) {
			pLemmaInfo->pNextLemma[1]->pPrevLemma[1]
			  = pLemmaInfo->pPrevLemma[1];
		} else { //This lemma is the Tail of one of the lemma lists
			if (pAFS->LemmasWherePosTail[1].pNextLemma[1] == pLemmaInfo)
			  pAFS->LemmasWherePosTail[1].pNextLemma[1] = pLemmaInfo->pPrevLemma[1];
			else if (pAFS->LemmasWhereNegTail[1].pNextLemma[1] == pLemmaInfo)
			  pAFS->LemmasWhereNegTail[1].pNextLemma[1] = pLemmaInfo->pPrevLemma[1];
			else {
				fprintf(stderr, "LemmaInfoStruct Tail pointer is invalid\n");
				assert(0);
			}
		}
	} else {
		if (pLemmaInfo->pNextLemma[1]) {
			pLemmaInfo->pNextLemma[1]->pPrevLemma[1]
			  = pLemmaInfo->pPrevLemma[1];
		}
   }
}

ITE_INLINE void
LemmaSetWatchedLits(LemmaInfoStruct *pLemmaInfo, int *arrLiterals, int nNumLiterals)
{
   if (pLemmaInfo->bPutInCache==false) {
      pLemmaInfo->nWatchedVble[0] = 0;
      pLemmaInfo->nWatchedVble[1] = 0;
      pLemmaInfo->nWatchedVblePolarity[0] = BOOL_UNKNOWN;
      pLemmaInfo->nWatchedVblePolarity[1] = BOOL_UNKNOWN;
      pLemmaInfo->pNextLemma[0] = NULL;
   } else {
      // Update information concerning which variables affect
      // which lemmas.
      int nLiteral;
      int nWatchedLiteral1 = 0;
      int nWatchedLiteral2 = 0;
      int nWatchedLiteralBSI1 = -1; // "Backtrack Stack Index" of watched lit 1.
      // Specifically, the index on the backtrack stack at which the literal's
      // variable is mentioned.
      int nWatchedLiteralBSI2 = -1;  
      int nCurrentLiteralBSI;

      //Update brancher information for this variable in this lemma.
      //m two highest BSI are Watched Lit 1 and 2
      for (int i = 0; i < nNumLiterals; i++)
      {
         nLiteral = arrLiterals[i];

         nCurrentLiteralBSI = arrBacktrackStackIndex[abs(nLiteral)];
         assert(nCurrentLiteralBSI >= 0);

         if(nCurrentLiteralBSI > nWatchedLiteralBSI1)
         {
            nWatchedLiteral2 = nWatchedLiteral1;
            nWatchedLiteralBSI2 = nWatchedLiteralBSI1;
            nWatchedLiteral1 = nLiteral;
            nWatchedLiteralBSI1 = nCurrentLiteralBSI;
         }
         else if(nCurrentLiteralBSI > nWatchedLiteralBSI2)
         {
            nWatchedLiteral2 = nLiteral;
            nWatchedLiteralBSI2 = nCurrentLiteralBSI;	  
         }

         //assert(arrSolution[abs(nLiteral)] != BOOL_UNKNOWN);
         //assert((nLiteral >= 0) == (arrSolution[abs(nLiteral)] == BOOL_FALSE));
      }

      assert(nWatchedLiteral1 != 0 || nNumLiterals == 0);
      assert(nWatchedLiteral2 != 0 || nNumLiterals <= 1);

      pLemmaInfo->nWatchedVble[0] = abs(nWatchedLiteral1);
      pLemmaInfo->nWatchedVble[1] = abs(nWatchedLiteral2);  
      pLemmaInfo->nWatchedVblePolarity[0] = ((nWatchedLiteral1 >= 0) ? BOOL_TRUE : BOOL_FALSE);
      pLemmaInfo->nWatchedVblePolarity[1] = ((nWatchedLiteral2 >= 0) ? BOOL_TRUE : BOOL_FALSE);
   }
}

