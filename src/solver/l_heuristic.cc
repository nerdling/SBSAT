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
// assert USE_LEMMA_VAR_HEURISTIC is set

#include "ite.h"
#include "solver.h"

/* optimized l heur with sorting -- faster */
//#define MK_L_HEU_W_SORT

/* shortened original heur -- no much of a speed improvement in there */
//#define MK_SHORTER_L_HEU

/* original heur */
#define L_HEUR_ORIG

/* 

   other files and l_heuristic.cc
   ------------------------------

   select_bp.cc -- calls this heuristic

   init_solver.cc -- allocates 

 */

extern int *arrLemmaVbleCountsPos;
extern int *arrLemmaVbleCountsNeg;

extern int nIndepVariables;
extern int nDepVariables;
extern int *arrIndepVariables;
extern int *arrDepVariables;
extern int *arrBacktrackStackIndex;


int
l_heuristic_sort(const void *x, const void *y)
{
  /* if the result is negative => right order */

  int i=*(const int*)x;
  int j=*(const int*)y;

  /* ascending UNKNOWN before ZERO or ONE -- ZERO=0,ONE=1,UNKNOWN=2*/
  if (arrSolution[i] != arrSolution[j]) return arrSolution[j]-arrSolution[i];

  /* if they are set -- put the one set earlier to the end */
  if (arrSolution[i] != BOOL_UNKNOWN) return arrBacktrackStackIndex[j]-arrBacktrackStackIndex[i];

  int nVbleWeight1 = arrLemmaVbleCountsPos[i] > arrLemmaVbleCountsNeg[i] ?
	             arrLemmaVbleCountsPos[i] : arrLemmaVbleCountsNeg[i];
  int nVbleWeight2 = arrLemmaVbleCountsPos[j] > arrLemmaVbleCountsNeg[j] ?
	             arrLemmaVbleCountsPos[j] : arrLemmaVbleCountsNeg[j];
  /* descending weight 10 before weight 1 */
  if (nVbleWeight1 != nVbleWeight2) return nVbleWeight2-nVbleWeight1;
  #ifdef FORCE_STABLE_SORT
  return (int)((const int *)x-(const int *)y);
  #endif
  return 0;
}


// ITE_INLINE
void
L_SortVariableArray(int *arr, int max)
{
  qsort(arr, max, sizeof(int), l_heuristic_sort);
}


ITE_INLINE void
L_OptimizedHeuristic(int *pnBranchAtom, int *pnBranchValue)
{
  int nBestVble;
  int i;
  int nMaxWeight = 0;
  int nVbleWeight;
  //static int nTimesCalled = 0;

#ifdef HEURISTIC_USES_LEMMA_COUNTS
   /* FIXME: if needed */
  //extern LemmaInfoStruct *garrLemmaInfo;
  extern int gnNumLemmas;
  if (ite_counters[NUM_CHOICE_POINTS] % LEMMA_COUNT_FREQUENCY == 0)
    {
      // Update the "lemma counts":
      // Scan through the literals of each lemma and determine
      // whether it is satisfied and, if not, how many literals in
      // the lemma are uninstantiated.
      LemmaInfoStruct *pLemmaInfo = garrLemmaInfo;
      for (int i = 0; i < gnNumLemmas; i++)
	{
	  if (pLemmaInfo->bIsInCache)
	    {
	      int nNumUnknown_LemmaCount = 0;
	      for (int *pnLiteral = pLemmaInfo->pnLemma + 1;
		   *pnLiteral; pnLiteral++)
		{
		  int nLiteral = *pnLiteral;
		  int nVble = abs (nLiteral);
		  int nVbleValue = arrSolution[nVble];
		  if (nVbleValue == BOOL_UNKNOWN)
		    {
		      nNumUnknown_LemmaCount++;
		    }
		  else
		    {
		      if ((nLiteral > 0) == (nVbleValue == BOOL_TRUE))
			{
			  // Lemma is satisfied.
			  // We go ahead and break at this point:
			  // Getting a correct count on nNumUnknown
			  // is not important.
			  break;
			}
		    }
		}
	      pLemmaInfo->nNumUnknown_LemmaCount = nNumUnknown_LemmaCount;
	    }
	  pLemmaInfo++;
	}
    }
#endif // HEURISTIC_USES_LEMMA_COUNTS

  // Determine the variable with the highest weight:
  // 
  // Initialize to the lowest indexed variable whose value is uninstantiated.
  nBestVble = -1;
  nMaxWeight = -1;

#ifdef  MK_L_HEU_W_SORT
   if (ite_counters[NUM_CHOICE_POINTS] % 100 == 0) 
	L_SortVariableArray(arrIndepVariables, nIndepVariables);
   


  int j;
  for (j = 0; j < nIndepVariables; j++)
    {
      i = arrIndepVariables[j];
      if (arrSolution[i] == BOOL_UNKNOWN)
      {
	   nVbleWeight
	         = arrLemmaVbleCountsPos[i] > arrLemmaVbleCountsNeg[i] ?
	         arrLemmaVbleCountsPos[i] : arrLemmaVbleCountsNeg[i];
	   if (nMaxWeight < nVbleWeight){ nMaxWeight=nVbleWeight; nBestVble=i; 
		/* printf("hit: %d ", j);*/ goto ReturnHeuristicResult; }
      }
    }
    if (nBestVble != -1) goto ReturnHeuristicResult;
  for (j = 0; j < nDepVariables; j++)
    {
      i = arrDepVariables[j];
      if (arrSolution[i] == BOOL_UNKNOWN)
      {
	   nVbleWeight
	         = arrLemmaVbleCountsPos[i] > arrLemmaVbleCountsNeg[i] ?
	         arrLemmaVbleCountsPos[i] : arrLemmaVbleCountsNeg[i];
	   if (nMaxWeight < nVbleWeight){ nMaxWeight=nVbleWeight; nBestVble=i;}
      }
    }

  // -------------------------------------------------------
#endif

#ifdef L_HEUR_ORIG
  for (i = 1; i < nIndepVars+1; i++)
  {
     if (arrSolution[i] == BOOL_UNKNOWN)
     {
        nBestVble = i;
        nMaxWeight
           = arrLemmaVbleCountsPos[i] > arrLemmaVbleCountsNeg[i] ?
           arrLemmaVbleCountsPos[i] : arrLemmaVbleCountsNeg[i];

        break;
     }
  }

  if (nBestVble >= 0)
  {
     // Search through the remaining uninstantiated independent variables.
     for (i = nBestVble+1; i < nIndepVars+1; i++)
     {
        if (arrSolution[i] == BOOL_UNKNOWN)
        {
           nVbleWeight
              = arrLemmaVbleCountsPos[i] > arrLemmaVbleCountsNeg[i] ?
              arrLemmaVbleCountsPos[i] : arrLemmaVbleCountsNeg[i];

           if (nVbleWeight > nMaxWeight)
           {
              nMaxWeight = nVbleWeight;
              nBestVble = i;
           }
        }
     }

      goto ReturnHeuristicResult;
    }

  // Find highest weight variable in Dependent variables
  // 
  // Initialize to first uninstantiated variable.
  for (i = nIndepVars+1; i < nIndepVars+1+nDepVars; i++)
  {
     if (arrSolution[i] == BOOL_UNKNOWN)
     {
        nBestVble = i;
        nMaxWeight
           = arrLemmaVbleCountsPos[i] > arrLemmaVbleCountsNeg[i] ?
           arrLemmaVbleCountsPos[i] : arrLemmaVbleCountsNeg[i];
        break;
     }
  }

  if (nBestVble >= 0)
  {
     // Search through the remaining uninstantiated dependent variables.
     for (i = nBestVble+1; i < nIndepVars+1+nDepVars; i++)

     {
        if (arrSolution[i] == BOOL_UNKNOWN)
        {
           nVbleWeight
              = arrLemmaVbleCountsPos[i] > arrLemmaVbleCountsNeg[i] ?
              arrLemmaVbleCountsPos[i] : arrLemmaVbleCountsNeg[i];

           if (nVbleWeight > nMaxWeight)
           {
              nMaxWeight = nVbleWeight;
              nBestVble = i;
           }
        }
     }

     goto ReturnHeuristicResult;
  }
  else
  {
     cout << "Error in heuristic routine:  No uninstantiated variable found"
        << endl;
     exit (1);
  }
#endif

ReturnHeuristicResult:
  assert (arrSolution[nBestVble] == BOOL_UNKNOWN);
  *pnBranchAtom = nBestVble;
  if (arrLemmaVbleCountsPos[nBestVble] > arrLemmaVbleCountsNeg[nBestVble])
    {
      *pnBranchValue = BOOL_TRUE;
    }
  else
    {
      *pnBranchValue = BOOL_FALSE;
    }
}

