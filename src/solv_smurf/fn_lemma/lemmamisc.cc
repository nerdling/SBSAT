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

int **arrPattern;
int *arrTempSlideLemma;
int *arrTempSlideSmurf;

ITE_INLINE void
DisplayLemma(int *pnLemma)
{
   int nLength = *pnLemma++;

   for (int i = 0; i < nLength; i++)
   {
      fprintf(stddbg, "%d ",  pnLemma[i]);
   }
   fprintf(stddbg, "\n");
}

ITE_INLINE void
DisplayLemma(LemmaBlock *pLemma)
{
   LemmaBlock *pLemmaBlock = pLemma;
   int *arrLits = pLemmaBlock->arrLits;
   int nLemmaLength = arrLits[0];
   fprintf(stddbg, "(len: %d) ", nLemmaLength);
   int nLitIndex;
   int nLitIndexInBlock;
   for (nLitIndex = 1, nLitIndexInBlock = 1;
         nLitIndex <= nLemmaLength;
         nLitIndex++, nLitIndexInBlock++)
   {
      if (nLitIndexInBlock == LITS_PER_LEMMA_BLOCK)
      {
         nLitIndexInBlock = 0;
         pLemmaBlock = pLemmaBlock->pNext;
         arrLits = pLemmaBlock->arrLits;
      }
      fprintf(stddbg, "%d ", arrLits[nLitIndexInBlock]);
   }
   fprintf(stddbg, "\n");
}

ITE_INLINE void
DisplayLemmaToFile(FILE *pFile, LemmaBlock *pLemma)
{
   LemmaBlock *pLemmaBlock = pLemma;
   int *arrLits = pLemmaBlock->arrLits;
   int nLemmaLength = arrLits[0];
   int nLitIndex;
   int nLitIndexInBlock;
   for (nLitIndex = 1, nLitIndexInBlock = 1;
         nLitIndex <= nLemmaLength;
         nLitIndex++, nLitIndexInBlock++)
   {
      if (nLitIndexInBlock == LITS_PER_LEMMA_BLOCK)
      {
         nLitIndexInBlock = 0;
         pLemmaBlock = pLemmaBlock->pNext;
         arrLits = pLemmaBlock->arrLits;
      }
      //cout << arrLits[nLitIndexInBlock] << " ";
      fprintf(pFile, "%d ", arrLits[nLitIndexInBlock]);
   }
   fprintf(pFile, "0");
}

ITE_INLINE void
DisplayLemmaArrayToFile(FILE *pFile, int *arrLemma, int nLemmaIndex) {
	for(int x = 0; x < nLemmaIndex; x++) {
		if(arrLemma[x] < 0)
		  fprintf(pFile, "%d", abs(arrLemma[x]));
		fprintf(pFile, "%d", abs(arrLemma[x]));
	}
}

ITE_INLINE void
DisplayLemmaIdentifierToFile(FILE *pFile, LemmaBlock *pLemma)
{
   LemmaBlock *pLemmaBlock = pLemma;
   int *arrLits = pLemmaBlock->arrLits;
   int nLemmaLength = arrLits[0];
   int nLitIndex;
   int nLitIndexInBlock;
   for (nLitIndex = 1, nLitIndexInBlock = 1;
         nLitIndex <= nLemmaLength;
         nLitIndex++, nLitIndexInBlock++)
   {
      if (nLitIndexInBlock == LITS_PER_LEMMA_BLOCK)
      {
         nLitIndexInBlock = 0;
         pLemmaBlock = pLemmaBlock->pNext;
         arrLits = pLemmaBlock->arrLits;
      }
      //cout << arrLits[nLitIndexInBlock] << " ";
		if(arrLits[nLitIndexInBlock] < 0)
		  fprintf(pFile, "%d", abs(arrLits[nLitIndexInBlock]));
		fprintf(pFile, "%d", abs(arrLits[nLitIndexInBlock]));
   }
}

ITE_INLINE int
SlideLemma(LemmaInfoStruct *pLemmaInfo, LemmaInfoStruct *pUnitLemmaList, LemmaInfoStruct **pUnitLemmaListTail)
{
	bool bComplete = false; //indicates whether all function sets have been checked
	int nOffset = 0; //offset is initially 0 so it can be set appropriately later
	bool bIncrement = true; //start off by incrementing the offset, then decrement later when false

	int nLowestFunc = nmbrFunctions;
	int nHighestFunc = 0;

	LemmaBlock *pSmurfs = pLemmaInfo->pSmurfsReferenced;
	for(int nSmurfVar = 0, nSmurfIndex = 1;
		 nSmurfVar < pLemmaInfo->pSmurfsReferenced->arrLits[0];
		 nSmurfVar++, nSmurfIndex++) {
		if(nSmurfIndex == LITS_PER_LEMMA_BLOCK) {
			nSmurfIndex = 0;
			pSmurfs = pSmurfs->pNext;
		}
		
		int nCurSmurf = pSmurfs->arrLits[nSmurfIndex];
		
		if(nCurSmurf > nHighestFunc)
		  nHighestFunc = nCurSmurf;
		if(nCurSmurf < nLowestFunc)
		  nLowestFunc = nCurSmurf;
	}
	
	d9_printf1("Sliding Lemma: [ ");
	
	//first create the lemma pattern for each variable in the lemma
	LemmaBlock *pLemma = pLemmaInfo->pLemma;
	for(int nLemmaVar = 0, nLemmaIndex = 1;
		 nLemmaVar < pLemmaInfo->pLemma->arrLits[0];
		 nLemmaVar++, nLemmaIndex++)
	{
		if(nLemmaIndex == LITS_PER_LEMMA_BLOCK) {
			nLemmaIndex = 0;
			pLemma = pLemma->pNext;			
		}

		int nCurVar = abs(pLemma->arrLits[nLemmaIndex]);
		assert(nCurVar > 0);
		
		d9_printf2("%d ", pLemma->arrLits[nLemmaIndex]);
		//d9_printf2("(%d)", arrSolver2IteVarMap[nCurVar]);

		int foundit = 0;
		
		pSmurfs = pLemmaInfo->pSmurfsReferenced;

		for(int nSmurfVar = 0, nSmurfIndex = 1;
			 nSmurfVar < pLemmaInfo->pSmurfsReferenced->arrLits[0];
			 nSmurfVar++, nSmurfIndex++) {
			if(nSmurfIndex == LITS_PER_LEMMA_BLOCK) {
				nSmurfIndex = 0;
				pSmurfs = pSmurfs->pNext;
			}
			
			int nCurSmurf = pSmurfs->arrLits[nSmurfIndex];

			for(int x = 1; x <= arrSolverVarsInFunction[nCurSmurf][0]; x++) {
				if(arrSolverVarsInFunction[nCurSmurf][x] > nCurVar) break; //Only works because arrSolverVars are sorted
				if(arrSolverVarsInFunction[nCurSmurf][x] == nCurVar) {
					foundit = 1;
					arrPattern[nLemmaVar][0] = nCurSmurf;
					arrPattern[nLemmaVar][1] = x;
					arrPattern[nLemmaVar][2] = arrFunctionStructure[nCurSmurf];
					arrPattern[nLemmaVar][3] = pLemma->arrLits[nLemmaIndex]>0?1:-1;

					break;
				}
			}
			if(foundit == 1) break;
		}
		if(foundit==0) {
			fprintf(stderr, "Major problem...exiting\n");
			exit(0);			
		}
	}
	d9_printf1("]\n");
	
	//while not all sets of functions have been checked
	while(!bComplete)
	{
		if(bIncrement) //if trying to increment
		{
			//first increment to the highest point possible
			if(nHighestFunc < nNumFuncs-1) //if the highest numbered function is not the last function in the slider
			{
				nOffset++;
				nHighestFunc++; 
			}
			else
			{
				nOffset = 0; //reset nOffset
				bIncrement = false; //stop incrementing and start decrementing
				continue; //don't loop thru and compare the lemma to its own functions; start decrementing instead
			}
		}
		else
		{
			//next decrement down to the first function possible (which would be function #1)
			if(nLowestFunc > 0)
			{
				nOffset--;
				nLowestFunc--;
			}
			else
			  return 1; //all functions have been checked so stop looping
		}
		
		//check that the function types match
		bool bTypeMatches = true;
		LemmaBlock *pSmurfs = pLemmaInfo->pSmurfsReferenced;
		for(int nSmurfVar = 0, nSmurfIndex = 1;
			 nSmurfVar < pLemmaInfo->pSmurfsReferenced->arrLits[0];
			 nSmurfVar++, nSmurfIndex++) {
			if(nSmurfIndex == LITS_PER_LEMMA_BLOCK) {
				nSmurfIndex = 0;
				pSmurfs = pSmurfs->pNext;
			}
			
			int nFunctionNumber = pSmurfs->arrLits[nSmurfIndex];
			arrTempSlideSmurf[nSmurfVar] = nFunctionNumber + nOffset;
			
			//account for smurfs occupying multiple blocks
			if(arrFunctionStructure[nFunctionNumber] != arrFunctionStructure[(nFunctionNumber + nOffset)]) {
				bTypeMatches = false;
				break;
			}
		}
		arrTempSlideSmurf[pLemmaInfo->pSmurfsReferenced->arrLits[0]] = 0;
		if(bTypeMatches) //if the types match, get variables
		{
			//get the variables for the new lemma
			d9_printf1("  [ ");
			for(int nLemmaVar = 0; nLemmaVar < pLemmaInfo->pLemma->arrLits[0]; nLemmaVar++) {
				arrTempSlideLemma[nLemmaVar] = arrSolverVarsInFunction[(arrPattern[nLemmaVar][0] + nOffset)][arrPattern[nLemmaVar][1]]*arrPattern[nLemmaVar][3];
				//If var not already in arrTempSlideSmurf,
				d9_printf2("%d ", arrTempSlideLemma[nLemmaVar]);
				//d9_printf2("(%d)", arrSolver2IteVarMap[abs(arrTempSlideLemma[nLemmaVar])]);
			}
			d9_printf1("]\n");
			arrTempSlideLemma[pLemmaInfo->pLemma->arrLits[0]] = 0;
			
			LemmaInfoStruct *slid_lemma = AddLemma_SmurfsReferenced(pLemmaInfo->pLemma->arrLits[0], arrTempSlideLemma,
											  pLemmaInfo->pSmurfsReferenced->arrLits[0], arrTempSlideSmurf,
											  true, pUnitLemmaList,
											  pUnitLemmaListTail
											  );
			assert(slid_lemma!=NULL);
			slid_lemma->nLemmaIsASlide = 1;
		}
	}//end while
  return 1;
}

ITE_INLINE int
LemmaIsSAT(LemmaBlock *pLemma)
{
   LemmaBlock *pLemmaBlock = pLemma;
   int *arrLits = pLemmaBlock->arrLits;
   int nLemmaLength = arrLits[0];
   int nLitIndex;
   int nLitIndexInBlock;
   for (nLitIndex = 1, nLitIndexInBlock = 1;
         nLitIndex <= nLemmaLength;
         nLitIndex++, nLitIndexInBlock++)
   {
      if (nLitIndexInBlock == LITS_PER_LEMMA_BLOCK)
      {
         nLitIndexInBlock = 0;
         pLemmaBlock = pLemmaBlock->pNext;
         arrLits = pLemmaBlock->arrLits;
      }
      //cout << arrLits[nLitIndexInBlock] << " ";
      if(arrSolution[abs(arrLits[nLitIndexInBlock])] == BOOL_TRUE && arrLits[nLitIndexInBlock] > 0)
		  return 1;
      if(arrSolution[abs(arrLits[nLitIndexInBlock])] == BOOL_FALSE && arrLits[nLitIndexInBlock] < 0)
		  return 1;
   }
	return 0;
}

ITE_INLINE int
LemmaLitsUnset(LemmaBlock *pLemma)
{
   LemmaBlock *pLemmaBlock = pLemma;
   int *arrLits = pLemmaBlock->arrLits;
   int nLemmaLength = arrLits[0];
   int nLitIndex;
   int nLitIndexInBlock;
   int nLitsUnset = 0;
   for (nLitIndex = 1, nLitIndexInBlock = 1;
         nLitIndex <= nLemmaLength;
         nLitIndex++, nLitIndexInBlock++)
   {
      if (nLitIndexInBlock == LITS_PER_LEMMA_BLOCK)
      {
         nLitIndexInBlock = 0;
         pLemmaBlock = pLemmaBlock->pNext;
         arrLits = pLemmaBlock->arrLits;
      }
      if(arrSolution[abs(arrLits[nLitIndexInBlock])] == BOOL_UNKNOWN)
         nLitsUnset++;
   }
	return nLitsUnset;
}

ITE_INLINE void
DisplayLemmaStatus(LemmaBlock *pLemma)
{
   int nNumSat = 0;
   int nNumNotContradicted = 0;
   LemmaBlock *pLemmaBlock = pLemma;
   int *arrLits = pLemmaBlock->arrLits;
   int nLemmaLength = arrLits[0];
   int nLitIndex;
   int nLitIndexInBlock;
   for (nLitIndex = 1, nLitIndexInBlock = 1;
         nLitIndex <= nLemmaLength;
         nLitIndex++, nLitIndexInBlock++)
   {
      if (nLitIndexInBlock == LITS_PER_LEMMA_BLOCK)
      {
         nLitIndexInBlock = 0;
         pLemmaBlock = pLemmaBlock->pNext;
         arrLits = pLemmaBlock->arrLits;
      }
      int nLit = arrLits[nLitIndexInBlock];
      assert(nLit != 0);
      int nVble = abs(nLit);
      if (arrSolution[nVble] == BOOL_UNKNOWN)
      {
         nNumNotContradicted++;
         fprintf(stddbg, " *");
      }
      else if (arrSolution[nVble] == BOOL_TRUE)
      {
         if (nLit > 0)
         {
            nNumSat++;
            nNumNotContradicted++;
            fprintf(stddbg, " sat");
         }
         else
         {
            fprintf(stddbg, " unsat");
         }
      }
      else
      {
         assert(arrSolution[nVble] == BOOL_FALSE);
         if (nLit < 0)
         {
            nNumSat++;
            nNumNotContradicted++;
            fprintf(stddbg, " sat");
         }
         else
         {
            fprintf(stddbg, " unsat");
         }
      }
      fprintf(stddbg, "(%d)", nLit);
   }
   fprintf(stddbg, "\n");
   /* 
    cout << "# Sat: " << nNumSat 
    << "  # not contradicted: " << nNumNotContradicted
    << endl;
    */
}

ITE_INLINE void
DisplayLemmaLevels(LemmaBlock *pLemma, int arrLevel[])
{
   cout << "Levels:" << endl;

   LemmaBlock *pLemmaBlock = pLemma;
   int *arrLits = pLemmaBlock->arrLits;
   int nLemmaLength = arrLits[0];
   int nLitIndex; // Index relative to the whole lemma.
   int nLitIndexInBlock; // Index relative to the individual
   // lemma block.  I.e., this is
   // an index into arrLits.
   int nLiteral;
   int nVble;

   for (nLitIndex = 1, nLitIndexInBlock = 1;
         nLitIndex <= nLemmaLength;
         nLitIndex++, nLitIndexInBlock++)
   {
      if (nLitIndexInBlock == LITS_PER_LEMMA_BLOCK)
      {
         nLitIndexInBlock = 0;
         pLemmaBlock = pLemmaBlock->pNext;
         arrLits = pLemmaBlock->arrLits;
      }
      nLiteral = arrLits[nLitIndexInBlock];
      nVble = abs(nLiteral);
      cout << nLiteral << ":" << arrLevel[nVble] << " ";
   }
   cout << endl;
}

ITE_INLINE void
DisplayLemmaInfo(LemmaInfoStruct *pLemmaInfo)
{
   fprintf(stddbg, "nWatchedVble[0]: %d nWatchedVble[1]: %d\n",
         pLemmaInfo->nWatchedVble[0], pLemmaInfo->nWatchedVble[1]);
   fprintf(stddbg, "nWatchedVblePolarity[0]: %d nWatchedVblePolarity[1]: %d\n",
         pLemmaInfo->nWatchedVblePolarity[0], pLemmaInfo->nWatchedVblePolarity[1]);
   DisplayLemmaStatus(pLemmaInfo->pLemma);
}

ITE_INLINE void
DisplayLemmaList(int nVble, int nPos, int nWhichWatch)
{
   AffectedFuncsStruct *pAFS = arrAFS + nVble;
   LemmaInfoStruct *pLemmaInfo;
   cout << "DisplayLemmaList() " << endl;
   if (nPos == 1)
   {
      if (nWhichWatch == 1)
      {
         pLemmaInfo = &(pAFS->LemmasWherePos[0]);
      }
      else
      {
         assert(nWhichWatch == 2);
         pLemmaInfo = &(pAFS->LemmasWherePos[1]);
      }
   }
   else
   {
      assert(nPos == 0);
      if (nWhichWatch == 1)
      {
         pLemmaInfo = &(pAFS->LemmasWhereNeg[0]);
      }
      else
      {
         assert(nWhichWatch == 2);
         pLemmaInfo = &(pAFS->LemmasWhereNeg[1]);
      }
   }

   int nCount = 0;

   do
   {
      pLemmaInfo = (nWhichWatch == 1 ? pLemmaInfo->pNextLemma[0]
            : pLemmaInfo->pNextLemma[1]);
      cout << pLemmaInfo << " ";
      nCount++;
   }
   while (pLemmaInfo && nCount < 100);
   cout << endl;
}

// ------------------------------- checking and verification -------------------------------


ITE_INLINE bool
IsInLemmaList(LemmaInfoStruct *pLemmaInfo, LemmaInfoStruct *pList)
{
   if (!pLemmaInfo)
   {
      return true;
   }

   for (LemmaInfoStruct *p = pList; p; p = p->pNextLemma[0])
   {
      if (p == pLemmaInfo)
      {
         return true;
      }
   }

   return false;
}

ITE_INLINE bool
LemmasEqual(int nLength1, int arrLits1[],
      int nLength2, int arrLits2[])
{
   if (nLength1 != nLength2)
   {
      return false;
   }

   for (int i = 0; i < nLength1; i++)
   {
      if (arrLits1[i] != arrLits2[i])
      {
         return false;
      }
   }

   return true;
}

ITE_INLINE void
CheckLengthOfLemmaList(int nVble, int nPos, int nWhichWatch,
      int nNumBacktracks)
{
   AffectedFuncsStruct *pAFS = arrAFS + nVble;
   LemmaInfoStruct *pLemmaInfo;
   if (nPos == 1)
   {
      if (nWhichWatch == 1)
      {
         pLemmaInfo = &(pAFS->LemmasWherePos[0]);
      }
      else
      {
         assert(nWhichWatch == 2);
         pLemmaInfo = &(pAFS->LemmasWherePos[1]);
      }
   }
   else
   {
      assert(nPos == 0);
      if (nWhichWatch == 1)
      {
         pLemmaInfo = &(pAFS->LemmasWhereNeg[0]);
      }
      else
      {
         assert(nWhichWatch == 2);
         pLemmaInfo = &(pAFS->LemmasWhereNeg[1]);
      }
   }

   int nCount = 0;

   do
   {
      pLemmaInfo = (nWhichWatch == 1 ? pLemmaInfo->pNextLemma[0]
            : pLemmaInfo->pNextLemma[1]);
      //cout << pLemmaInfo << " ";
      nCount++;
   }
   while (pLemmaInfo && nCount < 1000);
   
   if (nCount >= 1000)
   {
      cout << "CheckLengthOfLemmaList -- nNumBacktracks = "
         << ite_counters[NUM_BACKTRACKS] << endl;
      assert(0);
   }
}

ITE_INLINE void
VerifyLemmaList(int nVble, int nPos, int nWhichWatch)
{
   AffectedFuncsStruct *pAFS = arrAFS + nVble;
   LemmaInfoStruct *pLemmaInfo;
   LemmaInfoStruct *pPrev;
   if (nPos == 1)
   {
      if (nWhichWatch == 1)
      {
         pPrev = &(pAFS->LemmasWherePos[0]);
      }
      else
      {
         assert(nWhichWatch == 2);
         pPrev = &(pAFS->LemmasWherePos[1]);
      }
   }
   else
   {
      assert(nPos == 0);
      if (nWhichWatch == 1)
      {
         pPrev = &(pAFS->LemmasWhereNeg[0]);
      }
      else
      {
         assert(nWhichWatch == 2);
         pPrev = &(pAFS->LemmasWhereNeg[1]);
      }
   }

   int nCount = 0;

   do
   {
      pLemmaInfo = (nWhichWatch == 1 ? pPrev->pNextLemma[0]
            : pPrev->pNextLemma[1]);
      if (!pLemmaInfo)
      {
         break;
      }
#ifndef NDEBUG
      int nWatchedVble = (nWhichWatch == 1 ? pLemmaInfo->nWatchedVble[0]
            : pLemmaInfo->nWatchedVble[1]);
      assert(nWatchedVble == nVble);
      int nWatchedVblePolarity = (nWhichWatch == 1
            ? pLemmaInfo->nWatchedVblePolarity[0]
            : pLemmaInfo->nWatchedVblePolarity[1]);
      assert(nWatchedVblePolarity == nPos);
      LemmaInfoStruct *pStoredPrev = (nWhichWatch == 1
            ? pLemmaInfo->pPrevLemma[0]
            : pLemmaInfo->pPrevLemma[1]);
      assert(pPrev == pStoredPrev);
#endif
      nCount++;
      pPrev = pLemmaInfo;
   }
   while (true);
   //while (nCount < 1000);
   //assert(nCount < 1000);
}

ITE_INLINE void
VerifyLemmaLists()
{
   for (int nVble = 1; nVble <= gnMaxVbleIndex; nVble++)
   {
      VerifyLemmaList(nVble, 1, 1);
      VerifyLemmaList(nVble, 1, 2);
      VerifyLemmaList(nVble, 0, 1);
      VerifyLemmaList(nVble, 0, 2);
   }
}

ITE_INLINE bool
CheckLemma(LemmaInfoStruct &lemma, LemmaInfoStruct *pUnitLemmaList)
// Returns true if an error is found.
// Detects three types of errors:  
// (1) a lemma wrongly reporting being satisfied,
// (2) an unfired unit lemma, and
// (3) a contradicted lemma.
{
   //int *pnLemma = lemma.pnLemma;
   LemmaBlock *pLemmaBlock = lemma.pLemma;
   int *arrLits = pLemmaBlock->arrLits;
   int nLemmaLength = arrLits[0];
   //int *pnLiteral = pnLemma + 1;
   int nLitIndex;
   int nLitIndexInBlock;
   int nNumUnknown = 0;
   int nLiteral;
   int nVble;
   int nVbleValue;

   for (nLitIndex = 1, nLitIndexInBlock = 1;
         nLitIndex <= nLemmaLength;
         nLitIndex++, nLitIndexInBlock++)
   {
      if (nLitIndexInBlock == LITS_PER_LEMMA_BLOCK)
      {
         nLitIndexInBlock = 0;
         pLemmaBlock = pLemmaBlock->pNext;
         arrLits = pLemmaBlock->arrLits;
      }
      nLiteral = arrLits[nLitIndexInBlock];
      nVble = abs(nLiteral);
      nVbleValue = arrSolution[nVble];
      if (nVbleValue == BOOL_UNKNOWN)
      {
         nNumUnknown++;
      }
      else
      {
         if ((nLiteral > 0) == (nVbleValue == BOOL_TRUE))
         {
            // Lemma is satisfied.
            return false;
         }
      }
   }

   if (nNumUnknown == 0)
   {
      cout << endl << "ERROR -- Contradicted lemma:" << endl;
      DisplayLemmaInfo(&lemma);
      cout << endl;
      return true;
   }
   else if (nNumUnknown == 1)
   {
      cout << endl << "ERROR -- Unfired unit lemma:" << endl;
      DisplayLemmaInfo(&lemma);
      cout << endl;

      bool bFound = false;
      LemmaInfoStruct *p = pUnitLemmaList->pNextLemma[0];
      while (!bFound && p)
      {
         if (p == &lemma)
         {
            bFound = true;
         }
         else
         {
            p = p->pNextLemma[0];
         }
      }
      if (bFound)
      {
         cout << "Lemma is in unit lemma list" << endl;
      }
      else
      {
         cout << "Lemma is not in unit lemma list" << endl;
      }

      return true;
   }
   return false;
}

