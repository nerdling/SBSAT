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

void LoadLemmas(char *filename);

int *arrChangedSpecialFn = NULL;
int *arrChangedSmurfs = NULL;

extern int nNumBytesInStateArray; // # of bytes in a current state array.
extern int nNumBytesInSpecialFuncStackEntry;
extern SmurfState **arrRegSmurfInitialStates;

// Stack of the indicies of the previous
// inferred variables (variables inferred to be true or inferred to be false,
// including branch variables)
BacktrackStackEntry *arrBacktrackStack; 

ITE_INLINE void FreeHeuristicTablesForSpecialFuncs();
ITE_INLINE void InitializeSmurfStatesStack();
ITE_INLINE void InitializeSpecialFnStack();


ITE_INLINE
int
RecordInitialInferences()
{
   int i;
   BDDNode *pFunc;
   int nNumFuncs = nmbrFunctions;
   bool bValueOfVble;
   int nCurrentAtomValue;
   bool bInitialInferenceFound = false;

   for (i = 0; i < nNumFuncs; i++)
   {
      if (IsSpecialFunc(arrFunctionType[i]))
      {
         // We assume that special funcs have no literal implicants.
         continue;
      }
      pFunc = arrFunctions[i];
      if (pFunc->addons->pImplied == NULL) {
         // There is no literal implicant for this smurf ??
         dE_printf1("CHECK ME -- no implicant for the smurf\n");
         continue;
      }
      LiteralSetIterator litsetNext(*(pFunc->addons->pImplied));
      while (litsetNext(nVble, bValueOfVble))
      {
         nVble = arrIte2SolverVarMap[nVble];
         if (!bInitialInferenceFound)
         {
            printf("Warning.  Inference found in constraint %d", i);
            printf(" prior to beginning backtracking search:  ");
            printf("Brancher variable %d = ", nVble);
            assert(bValueOfVble == BOOL_TRUE || bValueOfVble == BOOL_FALSE);
            printf((bValueOfVble == BOOL_TRUE) ? "true.\n" : "false.\n");
            //bInitialInferenceFound = true;
         }

         nCurrentAtomValue = arrSolution[nVble];
         if (bValueOfVble)
         {
            // Positive inference
            if (nCurrentAtomValue == BOOL_FALSE)
            {
               // Conflict -- problem is unsatisfiable.
               dE_printf1("Conflict -- problem is unsatisfiable.\n");
               //goto_NoSolution;
               return SOLV_UNSAT;
            }
            else if (nCurrentAtomValue == BOOL_UNKNOWN)
            {
               // Set value of atom.
               arrSolution[nVble] = BOOL_TRUE;

               // Enqueue inference.
               *(pInferenceQueueNextEmpty++) = nVble;
            }
         }
         else
         {
            // Negative inference
            if (nCurrentAtomValue == BOOL_TRUE)
            {
               // Conflict -- problem is unsatisfiable.
               dE_printf1("Conflict -- problem is unsatisfiable.\n");
               //goto_NoSolution;
               return 1;
            }
            else if (nCurrentAtomValue == BOOL_UNKNOWN)
            {
               // Set value of atom.
               arrSolution[nVble] = BOOL_FALSE;

               // Enqueue inference.
               *(pInferenceQueueNextEmpty++) = nVble;
            }
         }
      }
   }

   return SOLV_UNKNOWN;
}

int *arrAFSBufferSmurfs = NULL;
IndexRoleStruct *arrAFSBufferSpecFn = NULL;

ITE_INLINE
AffectedFuncsStruct *
CreateAffectedFuncsStructures(int nMaxVbleIndex)
   // Create an array of AffectedFuncsStructs.
   // The i-th structure in the array will indicate the functions
   // (constraints) which mention the i-th variable.
   // In the case where the function is a special func
   // (i.e. not a regular Smurf) the role of the variable in
   // the function will be indicated.
{
   AffectedFuncsStruct *arrAFS;
   arrAFS = (AffectedFuncsStruct*)ite_calloc(nMaxVbleIndex+1, sizeof(AffectedFuncsStruct),
         9, "arrAFS");
   garrAFS = arrAFS;

   int nNumFuncs = nmbrFunctions;
   int *arrFuncType = functionType;
   int nVble;

/* zeroing done using calloc, 
   structures now part of the AFS

   // Initialize counts to zero and lists to empty.
   for (int i = 0; i < nMaxVbleIndex + 1; i++)
   {
      arrAFS[i].nNumRegSmurfsAffected = 0;
      arrAFS[i].nNumSpecialFuncsAffected = 0;
      // Our lists of LemmaInfoStructs all start with a single
      // empty head node.
      arrAFS[i].pLemmasWherePos[0] = (LemmaInfoStruct*)ite_calloc(1, sizeof(LemmaInfoStruct),
            9, "arrAFS[].pLemmasWherePos[0]");
      arrAFS[i].pLemmasWherePos[1] = (LemmaInfoStruct*)ite_calloc(1, sizeof(LemmaInfoStruct),
            9, "arrAFS[].pLemmasWherePos[0]");
      arrAFS[i].pLemmasWhereNeg[0] = (LemmaInfoStruct*)ite_calloc(1, sizeof(LemmaInfoStruct),
            9, "arrAFS[].pLemmasWherePos[0]");
      arrAFS[i].pLemmasWhereNeg[1] = (LemmaInfoStruct*)ite_calloc(1, sizeof(LemmaInfoStruct),
            9, "arrAFS[].pLemmasWherePos[0]");
   }
*/

   int nNumElementsSmurfs = 0;
   int nNumElementsSpecFn = 0;

   // For each variable, count the number of special funcs
   // and regular Smurfs which will mention it when the brancher starts.
   for (int nFuncIndex = 0; nFuncIndex < nNumFuncs; nFuncIndex++)
   {
      IntegerSetIterator isetNext(*(arrFunctions[nFuncIndex]->addons->pReduct->addons->pVbles));

      if (IsSpecialFunc(arrFuncType[nFuncIndex]))
      {
         while (isetNext(nVble))
         {
            arrAFS[arrIte2SolverVarMap[nVble]].nNumSpecialFuncsAffected++;
            nNumElementsSpecFn++;
         }
      }
      else
      {
         while (isetNext(nVble))
         {
            arrAFS[arrIte2SolverVarMap[nVble]].nNumRegSmurfsAffected++;
            nNumElementsSmurfs++;
         }

         if (xorFunctions[nFuncIndex])
         {
            IntegerSetIterator isetNext(*(xorFunctions[nFuncIndex]->addons->pReduct->addons->pVbles));
            while (isetNext(nVble))
            {
               arrAFS[arrIte2SolverVarMap[nVble]].nNumSpecialFuncsAffected++;
               nNumElementsSpecFn++;
            }
         }
      }
   }

   arrAFSBufferSmurfs = (int*)ite_calloc(nNumElementsSmurfs, sizeof(int), 2, "AFS Smurfs");
   arrAFSBufferSpecFn = (IndexRoleStruct*)ite_calloc(nNumElementsSpecFn, sizeof(IndexRoleStruct),
                         2, "AFS Spec Fn");
   int nAFSBufferSmurfsIdx = 0;
   int nAFSBufferSpecFnIdx = 0;

   // Allocate arrays for each variable.
   for (int nVble = 0; nVble <= nMaxVbleIndex; nVble++)
   {
      if (arrAFS[nVble].nNumRegSmurfsAffected > 0) {
         arrAFS[nVble].arrRegSmurfsAffected = arrAFSBufferSmurfs + nAFSBufferSmurfsIdx;
         nAFSBufferSmurfsIdx += arrAFS[nVble].nNumRegSmurfsAffected;
      }
      if (arrAFS[nVble].nNumSpecialFuncsAffected > 0) {
         arrAFS[nVble].arrSpecFuncsAffected = arrAFSBufferSpecFn + nAFSBufferSpecFnIdx;
         nAFSBufferSpecFnIdx += arrAFS[nVble].nNumSpecialFuncsAffected;
      }

/*
      // Allocate arrRegSmurfsAffected array.
      if (arrAFS[nVble].nNumRegSmurfsAffected > 0)
      {
         ITE_NEW_CATCH(
               arrAFS[nVble].arrRegSmurfsAffected
               = new int[arrAFS[nVble].nNumRegSmurfsAffected];,
               "arrAFS[].arrRegSmurfsAffected");	  
      }

      // Allocate arrSpecFuncsAffected array.
      if (arrAFS[nVble].nNumSpecialFuncsAffected > 0)
      {
         ITE_NEW_CATCH(
               arrAFS[nVble].arrSpecFuncsAffected
               = new IndexRoleStruct[arrAFS[nVble].nNumSpecialFuncsAffected];,
               "arrAFS[].arrSpecFuncsAffected");
      }
*/
   }

   // Setup two arrays of indicies used to keep track of
   // where to put the next piece of information into
   // arrAFS[nVble].arrRegSmurfsAffected
   // or arrAFS[nVble].arrSpecFuncsAffected.
   int *arrRegSmurfIndexForVble = (int*)ite_calloc(nMaxVbleIndex+1, sizeof(int),
         9, "arrRegSmurfIndexForVble");
   int *arrSpecialFuncIndexForVble = (int*)ite_calloc(nMaxVbleIndex+1, sizeof(int),
         9, "arrSpecialFuncIndexForVble");

   // Now fill in the information concerning which variables are mentioned
   // in which functions.
   int nSpecialFuncIndex = 0;
   int nRegSmurfIndex = 0;
   for (int nFuncIndex = 0; nFuncIndex < nNumFuncs; nFuncIndex++)
   {
      if (IsSpecialFunc(arrFuncType[nFuncIndex]))
      {
         IntegerSetIterator isetNext(*(arrFunctions[nFuncIndex]->addons->pReduct->addons->pVbles));
         while (isetNext(nVble))
         {
            nVble = arrIte2SolverVarMap[nVble];
            int nIndex = arrSpecialFuncIndexForVble[nVble];
            arrAFS[nVble].arrSpecFuncsAffected[nIndex]
               .nSpecFuncIndex = nSpecialFuncIndex;

            // Determine the role of the variable in the special func. 
            SpecialFunc *pSpecFunc = arrSpecialFuncs + nSpecialFuncIndex;
            if (nVble == pSpecFunc->nLHSVble)
            {
               if (pSpecFunc->nLHSPolarity == BOOL_TRUE)
               {
                  arrAFS[nVble].arrSpecFuncsAffected[nIndex]
                     .nPolarity = BOOL_TRUE;
               }
               else
               {
                  arrAFS[nVble].arrSpecFuncsAffected[nIndex]
                     .nPolarity = BOOL_FALSE;
               }
               arrAFS[nVble].arrSpecFuncsAffected[nIndex].nRHSIndex
                  = -1;
            }
            else
            {
               // It is a RHS variable.  Determine its polarity.
               int *arrRHSVbles = pSpecFunc->rhsVbles.arrElts;
               int nNumRHSVbles = pSpecFunc->rhsVbles.nNumElts;
               int i;

               for (i = 0; i < nNumRHSVbles; i++)
               {
                  if (nVble == arrRHSVbles[i])
                  {
                     if (pSpecFunc->arrRHSPolarities[i] == BOOL_TRUE)
                     {
                        arrAFS[nVble].arrSpecFuncsAffected[nIndex]
                           .nPolarity = BOOL_TRUE;
                     }
                     else
                     {
                        arrAFS[nVble].arrSpecFuncsAffected[nIndex]
                           .nPolarity = BOOL_FALSE;
                     }
                     arrAFS[nVble].arrSpecFuncsAffected[nIndex].nRHSIndex
                        = i;
                     break;
                  }
               }
               assert(i != nNumRHSVbles); // bItemFound
            }

            arrSpecialFuncIndexForVble[nVble]++;
         }
         nSpecialFuncIndex++;
      }
      else
      {
         IntegerSetIterator isetNext(*(arrFunctions[nFuncIndex]->addons->pReduct->addons->pVbles));

         // Current constraint is represented as a regular Smurf.
         while (isetNext(nVble))
         {
            nVble = arrIte2SolverVarMap[nVble];
            int nIndex = arrRegSmurfIndexForVble[nVble];
            arrAFS[nVble].arrRegSmurfsAffected[nIndex]
               = nRegSmurfIndex;
            arrRegSmurfIndexForVble[nVble]++;
         }
         nRegSmurfIndex++;
         if (xorFunctions[nFuncIndex]) {
            IntegerSetIterator isetNext(*(xorFunctions[nFuncIndex]->addons->pReduct->addons->pVbles));
            /* FIXME: copy from the above -- create function for this: */
         while (isetNext(nVble))
         {
            nVble = arrIte2SolverVarMap[nVble];
            int nIndex = arrSpecialFuncIndexForVble[nVble];
            arrAFS[nVble].arrSpecFuncsAffected[nIndex]
               .nSpecFuncIndex = nSpecialFuncIndex;

            // Determine the role of the variable in the special func. 
            SpecialFunc *pSpecFunc = arrSpecialFuncs + nSpecialFuncIndex;
            if (nVble == pSpecFunc->nLHSVble)
            {
               if (pSpecFunc->nLHSPolarity == BOOL_TRUE)
               {
                  arrAFS[nVble].arrSpecFuncsAffected[nIndex]
                     .nPolarity = BOOL_TRUE;
               }
               else
               {
                  arrAFS[nVble].arrSpecFuncsAffected[nIndex]
                     .nPolarity = BOOL_FALSE;
               }
               arrAFS[nVble].arrSpecFuncsAffected[nIndex].nRHSIndex
                  = -1;
            }
            else
            {
               // It is a RHS variable.  Determine its polarity.
               int *arrRHSVbles = pSpecFunc->rhsVbles.arrElts;
               int nNumRHSVbles = pSpecFunc->rhsVbles.nNumElts;
               int i;

               for (i = 0; i < nNumRHSVbles; i++)
               {
                  if (nVble == arrRHSVbles[i])
                  {
                     if (pSpecFunc->arrRHSPolarities[i] == BOOL_TRUE)
                     {
                        arrAFS[nVble].arrSpecFuncsAffected[nIndex]
                           .nPolarity = BOOL_TRUE;
                     }
                     else
                     {
                        arrAFS[nVble].arrSpecFuncsAffected[nIndex]
                           .nPolarity = BOOL_FALSE;
                     }
                     arrAFS[nVble].arrSpecFuncsAffected[nIndex].nRHSIndex
                        = i;
                     break;
                  }
               }
               assert(i != nNumRHSVbles); // bItemFound
            }

            arrSpecialFuncIndexForVble[nVble]++;
         }
         nSpecialFuncIndex++;
         }
      }
   }

   free(arrRegSmurfIndexForVble);
   free(arrSpecialFuncIndexForVble);

   return arrAFS;
}

ITE_INLINE void
FreeAFS()
{
   free(arrAFSBufferSmurfs);
   free(arrAFSBufferSpecFn);
   free(garrAFS);
}

ITE_INLINE int
InitSolveVillage()
{
   d2_printf1("InitSolveVillage\n");

   int nNumFuncs = nmbrFunctions;

   pUnitLemmaList = (LemmaInfoStruct*)ite_calloc(1, sizeof(LemmaInfoStruct),
         9, "pUnitLemmaList"); /*m local to backtrack function only */

   /* Backtrack arrays */
   ITE_NEW_CATCH(
         arrUnsetLemmaFlagVars = new int[gnMaxVbleIndex];
         arrTempLemma = new int[gnMaxVbleIndex];
         arrBacktrackStackIndex = new int[gnMaxVbleIndex+1];
         arrLemmaFlag = new bool[gnMaxVbleIndex+1];,
         "arrUnsetLemmaFlagVars");

   nNumUnresolvedFunctions = nNumRegSmurfs + nNumSpecialFuncs; //nNumFuncs;

   for(int x = 1; x <= gnMaxVbleIndex; x++) 
   {
      arrLemmaFlag[x] = false;
      arrBacktrackStackIndex[x] = gnMaxVbleIndex+1;
   }
   arrBacktrackStackIndex[0] = 0;

   // Initialization of variables and data structures
   assert(pTrueSmurfState);  // Should be initialized by SmurfFactory.
   arrFunctionType = functionType;
   arrFunctions = functions;

   /* init Inference queue */
   arrInferenceQueue
      = (int *)ite_calloc(nNumVariables, sizeof(*arrInferenceQueue), 2, 
            "inference queue");
   pInferenceQueueNextElt = pInferenceQueueNextEmpty = arrInferenceQueue;


   arrAFS = CreateAffectedFuncsStructures(gnMaxVbleIndex);
   nNumBytesInStateArray = sizeof(SmurfState *) * nNumRegSmurfs;

   if(nNumRegSmurfs > 0) {
      arrCurrentStates
         = (SmurfState **)ite_calloc(nNumRegSmurfs, sizeof(SmurfState *), 2,
               "current states");
      arrPrevStates
         = (SmurfState **)ite_calloc(nNumRegSmurfs, sizeof(SmurfState *), 2,
               "previous states");
   } else
      arrCurrentStates = NULL;


   int nRegSmurfIndex = 0;
   for (int i = 0; i < nNumFuncs; i++)
   {
      if (!IsSpecialFunc(arrFunctionType[i]))
      {
         arrPrevStates[nRegSmurfIndex] =
         arrCurrentStates[nRegSmurfIndex] =
            arrRegSmurfInitialStates[nRegSmurfIndex];
         //= arrFunctions[i]->addons->pSmurfState;

         if (arrCurrentStates[nRegSmurfIndex] == pTrueSmurfState)
         {
            nNumUnresolvedFunctions--;
         }
         nRegSmurfIndex++;
      }
   }
   InitializeSmurfStatesStack();


   // Set up the Special Func Stack.

   if (nNumSpecialFuncs > 0) { 
      arrNumRHSUnknowns = (int *)ite_calloc(nNumSpecialFuncs, sizeof(int),
            9, "arrNumRHSUnknowns");
      arrNumRHSUnknownsNew = (int *)ite_calloc(nNumSpecialFuncs, sizeof(int),
            9, "arrNumRHSUnknownsNew");
      arrPrevNumRHSUnknowns = (int *)ite_calloc(nNumSpecialFuncs, sizeof(int),
            9, "arrPrevNumRHSUnknowns");

      arrNumLHSUnknowns = (int *)ite_calloc(nNumSpecialFuncs, sizeof(int),
            9, "arrNumLHSUnknowns");
      arrNumLHSUnknownsNew = (int *)ite_calloc(nNumSpecialFuncs, sizeof(int),
            9, "arrNumLHSUnknownsNew");
      arrPrevNumLHSUnknowns = (int *)ite_calloc(nNumSpecialFuncs, sizeof(int),
            9, "arrPrevNumLHSUnknowns");

      arrSumRHSUnknowns = (double *)ite_calloc(nNumSpecialFuncs, sizeof(double),
            9, "arrSumRHSUnknowns");
      arrSumRHSUnknownsNew = (double *)ite_calloc(nNumSpecialFuncs, sizeof(double),
            9, "arrSumRHSUnknownsNew");
      arrPrevSumRHSUnknowns = (double *)ite_calloc(nNumSpecialFuncs, sizeof(double),
            9, "arrPrevSumRHSUnknowns");

      nNumBytesInSpecialFuncStackEntry = nNumSpecialFuncs * sizeof(int);
      for (int i = 0; i < nNumSpecialFuncs; i++) {
         arrPrevNumRHSUnknowns[i] =
         arrNumRHSUnknownsNew[i] =
         arrNumRHSUnknowns[i] = arrSpecialFuncs[i].rhsVbles.nNumElts;
         arrPrevNumLHSUnknowns[i] =
         arrNumLHSUnknownsNew[i] =
         arrNumLHSUnknowns[i] = arrSpecialFuncs[i].nLHSVble > 0? 1: 0;

         assert(arrSolution[0]!=BOOL_UNKNOWN);
      }
   }

   /* have to have the stack at least for stages so I don't have to check
    the existence of special fn every time */
   InitializeSpecialFnStack();


   if (nNumRegSmurfs) {
      arrChangedSmurfs = (int*)ite_calloc(nNumRegSmurfs, sizeof(int),
            9, "arrChangedSmurfs");
      //for (int i=0;i<nNumRegSmurfs;i++) arrChangedSmurfs[i]=1;
   }

   if (nNumSpecialFuncs) {
      arrChangedSpecialFn = (int*)ite_calloc(nNumSpecialFuncs, sizeof(int),
            9, "arrChangedSpecialFn");
      //for (int i=0;i<nNumSpecialFuncs;i++) arrChangedSpecialFn[i]=1;
   }

   arrChoicePointStack 
      = (ChoicePointStruct *)ite_calloc(gnMaxVbleIndex, sizeof(*arrChoicePointStack), 2,
            "choice point stack");
   pChoicePointTop = arrChoicePointStack;

   arrBacktrackStack
      = (BacktrackStackEntry *)ite_calloc(nNumVariables, sizeof(*arrBacktrackStack), 2,
            "backtrack stack");
   pBacktrackTop = arrBacktrackStack;

   /* init Fn Inference queue */
   arrFnInferenceQueue
      = (t_fn_inf_queue *)ite_calloc(nNumSpecialFuncs+nNumRegSmurfs+1, sizeof(t_fn_inf_queue), 2,
            "function inference queue");
   pFnInferenceQueueNextElt = pFnInferenceQueueNextEmpty = arrFnInferenceQueue;

   gnNumLemmas = 0;
   InitLemmaInfoArray();
   //  InitHeuristicTablesForSpecialFuncs(nNumVariables);

   if (*lemma_in_file) LoadLemmas(lemma_in_file);

   if (nHeuristic == JOHNSON_HEURISTIC) J_InitHeuristic();

   int ret = RecordInitialInferences();

   // The variable with index 0 plays a special role (which is in handling
   // the PLAINOR functions).  We force its value to false before the
   // search begins.
   arrSolution[0] = BOOL_FALSE;
   *(pInferenceQueueNextEmpty++) = 0;

   return ret;
}

ITE_INLINE void
FreeSolveVillage()
{
   d2_printf1("FreeSolveVillage\n");
   delete [] arrUnsetLemmaFlagVars;
   delete [] arrTempLemma;
   delete [] arrBacktrackStackIndex;
   delete [] arrLemmaFlag;
   if (arrInferenceQueue) free(arrInferenceQueue);
   if (arrFnInferenceQueue) free(arrFnInferenceQueue);

   if (arrNumRHSUnknowns)   free(arrNumRHSUnknowns);
   if (arrNumRHSUnknownsNew) free(arrNumRHSUnknownsNew);
   if (arrPrevNumRHSUnknowns) free(arrPrevNumRHSUnknowns);

   if (arrNumLHSUnknowns)   free(arrNumLHSUnknowns);
   if (arrNumLHSUnknownsNew) free(arrNumLHSUnknownsNew);
   if (arrPrevNumLHSUnknowns) free(arrPrevNumLHSUnknowns);

   if (arrSumRHSUnknowns)   free(arrSumRHSUnknowns);
   if (arrSumRHSUnknownsNew) free(arrSumRHSUnknownsNew);
   if (arrPrevSumRHSUnknowns) free(arrPrevSumRHSUnknowns);

   if (arrCurrentStates)    free(arrCurrentStates);
   if (arrPrevStates)       free(arrPrevStates);
   if (arrChangedSmurfs)    free(arrChangedSmurfs);
   if (arrChangedSpecialFn) free(arrChangedSpecialFn);
   free(arrChoicePointStack);
   free(arrBacktrackStack);
   free(pUnitLemmaList);
   FreeIntNodePool();
   FreeLemmaInfoArray();
   if (nHeuristic == JOHNSON_HEURISTIC) {
      J_FreeHeurScoresStack();
   }

   FreeAFS();
}
