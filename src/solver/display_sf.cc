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
// Display SmurfFactory
// Routines for displaying creating state machines.
// Started 12/26/2000 - J. Ward

#include "ite.h"
#include "solver.h"

//#define PRINT_SMURF_STATS
#define DISPLAY_LEMMAS

// External variables.
extern int nSmurfStatePoolSize;
extern int nSmurfStatePoolIndex;

//#define PRINT_SMURF_STATS
#define DISPLAY_LEMMAS

char * opnames[EQU_BASE] =  {
       "UNSURE",
       "AND",
       "NAND",
       "OR",
       "NOR",
       "XOR",       
       "EQU/XNOR",
       "RIMP",
       "RNIMP",
       "LIMP",
       "LNIMP",
       "ITE",
       "NITE",
       "AND_EQUAL",
       "NEW_INT_LEAF",
       "IMPAND",
       "IMPOR",
       "PLAINOR",
       "PLAINXOR"
};

ITE_INLINE
char *
StringFromFunctionType(int nFuncType)
{
  if (nFuncType < EQU_BASE) return opnames[nFuncType];
  return "UNKNOWN FUNC TYPE";
}



ITE_INLINE
void
dbPrintBDD(BDDNodeStruct *pFunc)
  // Debug print BDD.
  // Print the BDD and print an end of line so that the output gets flushed.
  // Useful for calling directly from gdb.
{
  printBDD(pFunc);
  cout << endl;
}

ITE_INLINE
void
DisplayVbleParam(int arrVParam[])
{
  if (arrVParam)
    {
      cout << "Variable parameterization mapping: ";
      int nLength = arrVParam[0];
      for (int j = 0; j <= nLength; j++)
	{
	  cout << arrVParam[j] << " ";
	}
      cout << endl;
    }
  else
    {
      cout << "Unparameterized." << endl;
    }
}

ITE_INLINE
void
DisplaySpecialFunction(SpecialFunc *pSpecialFunc, int nFunctionType)
{
  if (pSpecialFunc->nLHSPolarity == BOOL_FALSE)
    {
      cout << "not ";
    }

  if (nFunctionType != 1)
    {
      cout << "v" << pSpecialFunc->nLHSVble << " = ";
    }
  char *pszConnective = NULL;
  switch (nFunctionType)
    {
    case (1):
      // or
      pszConnective = " or ";
      break;

    case (3):
      // and=
      pszConnective = " and ";
      break;

    case (4):
      // or=
      pszConnective = " or ";
      break;

    default:
		assert(0);
      break;
    }

  int nNumRHSElts = pSpecialFunc->rhsVbles.nNumElts;
  int *arrRHSVbles = pSpecialFunc->rhsVbles.arrElts;
  int *arrRHSPolarities =  pSpecialFunc->arrRHSPolarities;

  if (nNumRHSElts > 0)
    {
      cout << " v" << arrRHSVbles[0];
    }
  else
    {
      assert(0);
    }

  for (int i = 1; i < nNumRHSElts; i++)
    {
      cout << pszConnective;
      if (arrRHSPolarities[i] == BOOL_FALSE)
	{
	  cout << "not ";
	}
      cout << "v" << arrRHSVbles[i];
    }
  cout << endl;
}

ITE_INLINE
void
DisplayTransition(Transition *pTransition, int nVble, int nValue)
{
/*
	      cout << "X" << nVble << " = "
		   << (nValue == BOOL_FALSE ? "false" : "true")
		   << " Next: SmurfState["
		   << pTransition->pNextState - arrSmurfStatePool
		   << "] ("
		   << pTransition->pNextState << ")" << endl;
*/
	      cout << "Pos Inferences: ";
	      Display_ISAB(pTransition->positiveInferences);
	      cout << endl;
#ifdef DISPLAY_LEMMAS
	      int nNumInferences = pTransition->positiveInferences.nNumElts;
	      LemmaBlock **arrLemmas = pTransition->arrPosInferenceLemmas;
	      cout << "Lemmas:" << endl;
	      for (int i = 0; i < nNumInferences; i++)
		{
		  DisplayLemma(arrLemmas[i]);
		}
#endif
	      cout << "Neg Inferences: ";
	      Display_ISAB(pTransition->negativeInferences);
	      cout << endl;
#ifdef DISPLAY_LEMMAS
	      nNumInferences = pTransition->negativeInferences.nNumElts;
	      arrLemmas = pTransition->arrNegInferenceLemmas;
	      cout << "Lemmas:" << endl;
	      for (int i = 0; i < nNumInferences; i++)
		{
		  DisplayLemma(arrLemmas[i]);
		}
#endif
	      cout << endl;
#ifdef PRINT_HEURISTIC_INFO
	      if (nHeuristic == JOHNSON_HEURISTIC) 
	      	DisplayUpdatesToHeuristicTransScores(Transition *pTransition);
#endif // PRINT_HEURISTIC_INFO
}

#ifdef NOT_DEFINED
ITE_INLINE
void
DisplayFunction(BDDNode *pFunc, int nFunctionType, int nMaxVbleIndex)
{
  cout << pFunc;

  if (IsSpecialFunc(nFunctionType))
    {
      int nIndex = pFunc->addons->u.nSpecialFuncIndex;
      DisplaySpecialFunction(arrSpecialFuncs + nIndex, nFunctionType);
      return;
    }

  cout << " variable: " << pFunc->variable << " thenCase: "
       << pFunc->thenCase << " elseCase: " << pFunc->elseCase
       << " pSmurfState: " << pFunc->addons->pSmurfState << endl;

  if (pFunc == false_ptr || pFunc == true_ptr)
    {
      return;
    }

  cout << " Implied: ";
  pFunc->addons->pImplied->Display();
  cout << endl;
  cout << " pReduct: ";
  cout << pFunc->addons->pReduct;
  cout << endl;

  if (pFunc->addons->pReduct != pFunc)
    {
      return;
    }

  cout << " Variables: ";
  pFunc->addons->pVbles->Display();
  cout << endl;
  cout << "Transitions:" << endl;
  Transition *arrTransitions = pFunc->addons->pSmurfState->arrTransitions;
/*
  for (int i = 0; i <= nMaxVbleIndex; i++)
    {
      for (int j = 0; j < 2; j++)
	{
	  bool bValueOfVble = (j == 0);
	  int nTransIndex = TransitionIndex(i, bValueOfVble);
	  Transition *pTransition = arrTransitions + nTransIndex;
	  if (pTransition->pNextState != pFunc->addons->pSmurfState)
	    {
	      DisplayTransition(pTransition, i, j);
	      //cout << "X" << i << " = " << (bValueOfVble ? "true" : "false");
	      //cout << " Next: " << pTransition->pNextState << endl;
	      //cout << "Pos Inferences: ";
	      //Display_ISAB(pTransition->positiveInferences);
	      //cout << endl << "Neg Inferences: ";
	      //Display_ISAB(pTransition->negativeInferences);
	      //cout << endl;
	    }
	}
    }
*/
  ///*
  // Display children in recursion.
  if (pFunc->thenCase != false_ptr && pFunc->thenCase != true_ptr)
    {
      cout << endl;
      DisplayFunction(pFunc->thenCase, nFunctionType, nMaxVbleIndex);
    }
  if (pFunc->elseCase != false_ptr && pFunc->elseCase != true_ptr)
    {
      cout << endl;
      DisplayFunction(pFunc->elseCase, nFunctionType, nMaxVbleIndex);
    }
  //*/
}
#endif // NOT_DEFINED

ITE_INLINE
void
DisplaySmurfState(int nIndex, SmurfState *pState)
{
  Transition *pTransition;
  int nVble;

  cout << "SmurfState[" << nIndex << "] (" << pState << ")"<< endl;
  cout << "Transitions:" << endl;

  int nNumVbles = pState->vbles.nNumElts;
  int *arrVbles = pState->vbles.arrElts;
  for (int i = 0; i < nNumVbles; i++)
    {
      nVble = arrVbles[i];

      //cout << "X" << nVble << " = True" << endl;
      pTransition = FindTransition(pState, i, nVble, BOOL_TRUE);
      DisplayTransition(pTransition, nVble, BOOL_TRUE);

      //cout << "X" << nVble << " = False" << endl;
      pTransition = FindTransition(pState, i, nVble, BOOL_FALSE);
      DisplayTransition(pTransition, nVble, BOOL_FALSE);      
    }
}

ITE_INLINE
void
DisplaySmurfStates()
{
/*
  int nTrueIndex = pTrueSmurfState - arrSmurfStatePool;
  cout << "True Smurf state is SmurfState[" << nTrueIndex
       << "] (" << pTrueSmurfState << ")."
       << endl << endl;

  cout << "Root states for regular Smurfs:  ";
  for (int i = 0; i < nRegSmurfIndex; i++)
    {
      cout << endl;
      SmurfState *pState = arrRegSmurfInitialStates[i];
      int nIndex = pState - arrSmurfStatePool;
      cout << "Constraint[" << i << "]: "
	   << "SmurfState[" << nIndex << "] ("
	   << pState
	   << ")  ";
    }
  cout << endl << endl;

  cout << "SmurfStates: ";
  int nNumSmurfStates = nSmurfStatePoolIndex;
  for (int i = 0; i < nNumSmurfStates; i++)
    {
      cout << endl;
      DisplaySmurfState(i, arrSmurfStatePool + i);
    }
  cout << endl;
*/
}

#ifdef NOT_DEFINED
ITE_INLINE
void
DisplayBDDsAndSmurfs(int nMaxVbleIndex)
{
  cout << "Circuit problem:" << endl;
  cout << "# of constraints:  " << nmbrFunctions << endl;
  for (int i = 0; i < nmbrFunctions; i++)
    {
      cout << "Constraint " << i << ":" << functions[i] << endl;
    }
  cout << endl;

  int nLengthOfBddHashTable = 1 << (numBuckets+sizeBuckets);
  cout << "nLengthOfBddHashTable = " << nLengthOfBddHashTable << endl;

  /*
    Old way of displaying runs through the entire BDD hash table.
    This doesn't work so well now that we have the special functions.
  */
  cout << "BDD hash table:" << endl;
  for (int i = 0; i < nLengthOfBddHashTable; i++)
    {
      if (memory + i == false_ptr
	  || (memory[i].addons
	      && memory[i].addons->pSmurfState))
	{
	  DisplayFunction(memory + i, 0,
			  nMaxVbleIndex);
	  cout << endl;
	}
    }

  /*
  // New way runs through the constraints in order.
  cout << endl << "False:" << endl;
  DisplayFunction(false_ptr, 0, nMaxVbleIndex);
  cout << endl << "True:" << endl;
  DisplayFunction(true_ptr, 0, nMaxVbleIndex);
  for (int i = 0; i < nmbrFunctions; i++)
    {
      cout << endl << "Constraint " << i << ":" << endl;
      DisplayFunction(functions[i],
		      functionType[i],
		      nMaxVbleIndex);
    }
  */
}
#endif // NOT_DEFINED

