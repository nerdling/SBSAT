/* =========FOR INTERNAL USE ONLY. NO DISTRIBUTION PLEASE ========== */

/*********************************************************************
 Copyright 1999-2003, University of Cincinnati.  All rights reserved.
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
#include "ite.h"
#include "solver.h"

extern SmurfState *pTrueSmurfState;
extern SpecialFunc *arrSpecialFuncs;
extern int nNumSpecialFuncs;
extern int *arrFcnIndexRegSmurf;
extern BDDNodeStruct **arrFunctions;
extern int nNumRegSmurfs;


void
DisplaySpecialFunc(SpecialFunc *p);

void
DisplayStatus(int nNumSmurfs,
	      SmurfState **arrCurrentStates,
	      int nNumUnresolvedFunctions,
	      int nNumChoicePts,
	      int nNumBacktracks,
	      int arrNumRHSUnknowns[])
{
  cout << "nNumUnresolvedFunctions: " << nNumUnresolvedFunctions
       << " nNumChoicePts: " << nNumChoicePts
       << " nNumBacktracks: " << nNumBacktracks
       << endl;

  return;

  cout << "Unresolved Smurfs: " << endl;
  for (int i = 0; i < nNumSmurfs; i++)
    {
      /*
      for (int j = 0; j < 2 && i < nNumSmurfs; j++)
	{
	  cout << i << ": " << arrCurrentStates[i];
	  i++;
	  cout << "   ";
	}
       */
      if (arrCurrentStates[i] != pTrueSmurfState)
	{
	  cout << i << ": " << arrCurrentStates[i] << endl;
	}
    }
  
  cout << "Unresolved special functions:" << endl;
  if (nNumSpecialFuncs > 0)
    {
      for (int i = 0; i < nNumSpecialFuncs; i++)
	{
	  if (arrNumRHSUnknowns[i] > 0)
	    {
	      cout << "Special function " << i << ":" << endl;
	      DisplaySpecialFunc(arrSpecialFuncs + i);
	      cout << "#RHS Unknowns: "
		   << arrNumRHSUnknowns[i] << endl;
	    }
	}
    }
}

void
DisplayPartialSolution(int nNumVariables)
{
  for (int i = 0; i < nNumVariables; i++)
    {
      cout << i << ": " << arrSolution[i] << "  ";
    }
  cout << endl;
}

void
DisplayStatistics(int nNumChoicePts, int nNumBacktracks, int nNumBackjumps)
{
  fprintf(stddbg, "Choice Points: %d ", nNumChoicePts);
  fprintf(stddbg, ", Backtracks: %d, Backjumps: %d \n", 
		  nNumBacktracks, nNumBackjumps);
}

void
DisplaySpecialFunc(SpecialFunc *p)
     // Display the current status of one of our special case constraints,
     // e.g., a long and=.  (Used for tracing or debugging.)
{
  //cout << "Special Function:" << endl;
  cout << "nLHSVble: " << p->nLHSVble << " arrSolution[nLHSVble]: "
       << arrSolution[p->nLHSVble]
       << " nLHSPolarity: " << p->nLHSPolarity << endl;
  for (int i = 0; i < p->rhsVbles.nNumElts; i++)
    {
      int nVble = p->rhsVbles.arrElts[i];
      cout << nVble;
      if (p->arrRHSPolarities[i] == BOOL_FALSE)
	{
	  cout << "(neg)";
	}
      cout << ":" << arrSolution[nVble] << " ";
    }
  cout << endl;
}

#ifdef DISPLAY_TRACE
//enum InferenceType {REG_SMURF, LEMMA, SPECIAL_FUNC};

void
DisplayRelevantVbleAssignments(BDDNode *pBDD)
{
  int nVble;
  IntegerSetIterator isetNext(*(pBDD->addons->pVbles));
  while (isetNext(nVble))
    {
      cout << "X" << nVble << ": "
           << arrSolution[nVble] << "  ";
    }
  cout << endl;
}

void
DisplayInference(InferenceType eInfType,
		 void *pConstraintInfo,
		 int nNewInferredAtom,
		 int nNewInferredValue)
{
  switch (eInfType)
    {
    case REG_SMURF:
      {
	int nSmurfIndex = *((int *)pConstraintInfo);
	int nConstraintIndex = arrFcnIndexRegSmurf[nSmurfIndex];
	cout << "Inferring X" << nNewInferredAtom
	     << " = " << nNewInferredValue
	     << " from reg Smurf " << nConstraintIndex
	     << ":" << endl;

	BDDNode *pBdd = arrFunctions[nConstraintIndex];
	printBDD(pBdd);
	cout << endl;
	DisplayRelevantVbleAssignments(pBdd, arrSolution);
      }
      break;

    case LEMMA:
      {
	LemmaInfoStruct *pLemmaInfo = (LemmaInfoStruct *)pConstraintInfo;
	cout << "Inferring X" << nNewInferredAtom
	     << " = " << nNewInferredValue
	     << " from lemma:" << endl;
	DisplayLemmaInfo(pLemmaInfo);
      }
      break;

    case SPECIAL_FUNC:
      {
	SpecialFunc *pSpecialFunc = (SpecialFunc *)pConstraintInfo;
	cout << "Inferring X" << nNewInferredAtom
	     << " = " << nNewInferredValue
	     << " from Special Func:" << endl;

	DisplaySpecialFunc(pSpecialFunc);
      }
      break;

    default:
      assert(0);
    }
}

#endif

void
DisplayLemmaList1(LemmaInfoStruct *pList)
{
  for (LemmaInfoStruct *p = pList; p;
       p = p->pNextLemma[0])
    {
      cout << p << endl;
    }
}

void
DisplayLemmaList2(LemmaInfoStruct *pList)
{
  for (LemmaInfoStruct *p = pList; p;
       p = p->pNextLemma[1])
    {
      cout << p << endl;
    }
}


void DisplaySolution(int nMaxVbleIndex)
{
  int i;
  for (i = 0; i <= nMaxVbleIndex; i++)
    {
      switch (arrSolution[i])
	{
	case BOOL_TRUE:
	  printf("X%d = true ", i);
	  break;

	case BOOL_FALSE:
	  printf("X%d = false ", i);
	  break;

	default:
	  printf("X%d = don't care ", i);
	  break;
	}
    }
  printf("\n");
}

extern int nNumBacktracks;
extern int gnNumCachedLemmas;
extern int gnNumLemmas;
extern int nCallsToAddLemma;
extern int nNumBackjumps;
//extern long autarky_count;

ITE_INLINE
void
CalculateProgress(int *_whereAmI, int *_total)
{
  int whereAmI=0;
  int total=0;
  int soft_count=14;
  int hard_count=28;
  int count=0;

  extern BacktrackStackEntry *arrBacktrackStack;
  extern BacktrackStackEntry *pBacktrackTop;
  extern ChoicePointStruct *arrChoicePointStack;
  BacktrackStackEntry *pBacktrack = arrBacktrackStack;
  ChoicePointStruct *pChoicePoint = arrChoicePointStack;
  while (pBacktrack < pBacktrackTop && 
         (count<soft_count || (count < hard_count && whereAmI==0)) ) 
  {

    if (pBacktrack->bWasChoicePoint == true) {
      whereAmI *= 2;
      whereAmI += 1;
      total *= 2;
      total += 1;
      count++;
    }else
    if (pBacktrack->nAtom == pChoicePoint->nBranchVble) {
      whereAmI *= 2;
      total *= 2;
      total += 1;
      pChoicePoint++;
      count++;
    };

    pBacktrack++;
  }
  *_whereAmI = whereAmI;
  *_total    = total;
}

ITE_INLINE
void DisplayBacktrackInfo(double &fPrevEndTime, double &fStartTime)
{
      double fEndTime = get_runtime();
      double fTotalDurationInSecs = fEndTime - fStartTime;
      double fDurationInSecs = fEndTime - fPrevEndTime;
      double fBacktracksPerSec = BACKTRACKS_PER_STAT_REPORT / (fDurationInSecs>0?fDurationInSecs:0.001);
      fPrevEndTime = fEndTime;

      d2_printf2("Backtracks: %d ", nNumBacktracks);
      d2_printf2("Time: %4.3f secs.    ", fTotalDurationInSecs);
      d2_printf2("%4.3f backtracks per sec.\n", fBacktracksPerSec);

      int whereAmI = 0;
      int total = 0;
      double progress = 0.0;
      CalculateProgress(&whereAmI, &total);
      if (total == 0) total=1;
      progress = (float)whereAmI*100/total;
      d2_printf3("\n Progress: %x/%x        ", whereAmI, total);
      char number[10];
      char back[10] = "\b\b\b\b\b\b\b\b\b";
      sprintf(number, "% 3.2f%%", progress);
      back[strlen(number)]=0;
      if (DEBUG_LVL == 1) {
	      fprintf(stderr, "%s%s", number, back);
      } else {
	      d0_printf3("%s%s", number, back);
	      fflush(stddbg);
      }

#ifdef HAVE_IMAGE_JPEG
      D_2(
      graph_record(progress);  /* other variables are taken out of ite_counters */
      )
#endif

      d2_printf3("\n Choices (total, dependent): (%lld, %lld)",
      		  ite_counters[NO_ERROR],
		  ite_counters[HEU_DEP_VAR]
		  );
      if (NO_LEMMAS == 0)
      d2_printf4("\n Lemmas (cached, non-cached, added): (%d, %d, %d)",
      		  gnNumCachedLemmas,
		  gnNumLemmas - gnNumCachedLemmas,
		  nCallsToAddLemma
		  );
      /*d2_printf5("\n Inferences by (smurfs, ANDs, XORs, lemmas): (%lld, %lld, %lld, %lld)",
      		  ite_counters[INF_SMURF],
      		  ite_counters[INF_SPEC_FN_AND],
      		  ite_counters[INF_SPEC_FN_XOR],
      		  ite_counters[INF_LEMMA]
		  );*/
      d2_printf1("\n");
      d2_printf1(" Inferences by ");
      if (nNumRegSmurfs) d2_printf2("smurfs: %lld; ", ite_counters[INF_SMURF]);
      if (nNumSpecialFuncs) {
         d2_printf2("ANDs: %lld; ", ite_counters[INF_SPEC_FN_AND]);
         d2_printf2("XORs: %lld; ", ite_counters[INF_SPEC_FN_XOR]);
      }
      if (NO_LEMMAS == 0) d2_printf2("lemmas: %lld; ", ite_counters[INF_LEMMA]);
      d2_printf1("\n");

      d2_printf1(" Backtracks by ");
      if (nNumRegSmurfs) d2_printf2("smurfs: %lld; ", ite_counters[ERR_BT_SMURF]);
      if (nNumSpecialFuncs) {
         d2_printf2("ANDs: %lld; ", ite_counters[ERR_BT_SPEC_FN_AND]);
         d2_printf2("XORs: %lld; ", ite_counters[ERR_BT_SPEC_FN_XOR]);
      }
      if (NO_LEMMAS == 0) d2_printf2("lemmas: %lld; ", ite_counters[ERR_BT_LEMMA]);
      d2_printf1("\n");
      if (backjumping) d2_printf2(" Backjumps: %d\n", nNumBackjumps);
      if (autarky) d2_printf2(" Autarkies: %d\n", nNumAutarkies);
      //cout << "\n Autark levels: " << autarky_count;
      d2_printf1("\n");
}

