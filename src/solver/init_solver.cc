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

int *arrChangedSpecialFn = NULL;
int *arrChangedSmurfs = NULL;

// Stack of the indicies of the previous
// inferred variables (variables inferred to be true or inferred to be false,
// including branch variables)
BacktrackStackEntry *arrBacktrackStack; 

ITE_INLINE void FreeHeuristicTablesForSpecialFuncs();


ITE_INLINE int
RecordInitialInferences()
{
   int i;
   BDDNode *pFunc;
   bool bValueOfVble;
   int nCurrentAtomValue;
   bool bInitialInferenceFound = false;

   for (i = 0; i < nmbrFunctions; i++)
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

   int *arrFuncType = functionType;
   int nVble;

   int nNumElementsSmurfs = 0;
   int nNumElementsSpecFn = 0;

   // For each variable, count the number of special funcs
   // and regular Smurfs which will mention it when the brancher starts.
   for (int nFuncIndex = 0; nFuncIndex < nmbrFunctions; nFuncIndex++)
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
   for (int nFuncIndex = 0; nFuncIndex < nmbrFunctions; nFuncIndex++)
   {
      if (IsSpecialFunc(arrFuncType[nFuncIndex]))
      {
         IntegerSetIterator isetNext(*(arrFunctions[nFuncIndex]->addons->pReduct->addons->pVbles));
         while (isetNext(nVble))
         {
            assert(nVble != 0);
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

   ite_free((void*)arrRegSmurfIndexForVble);
   ite_free((void*)arrSpecialFuncIndexForVble);

   return arrAFS;
}

ITE_INLINE void
FreeAFS()
{
   ite_free((void*)arrAFSBufferSmurfs);
   ite_free((void*)arrAFSBufferSpecFn);
   ite_free((void*)arrAFS);
}

ITE_INLINE void
Update_arrVarScores()
{
   for (int i = 1; i<gnMaxVbleIndex; i++)
   {
      arrVarScores[i].pos = arrVarScores[i].pos/2 + 
         arrLemmaVbleCountsPos[i] - arrVarScores[i].last_count_pos;
      arrVarScores[i].neg = arrVarScores[i].neg/2 + 
         arrLemmaVbleCountsNeg[i] - arrVarScores[i].last_count_neg;
      arrVarScores[i].last_count_pos = arrLemmaVbleCountsPos[i];
      arrVarScores[i].last_count_neg = arrLemmaVbleCountsNeg[i];
   }
}


ITE_INLINE int
InitBrancher()
{
   d2_printf1("InitBrancher\n");

   arrVarScores = (t_arrVarScores*)ite_calloc(gnMaxVbleIndex, 
         sizeof(t_arrVarScores), 9, "arrVarScores");
   Update_arrVarScores();

   /* Backtrack arrays */
   arrUnsetLemmaFlagVars = (int*)ite_calloc(gnMaxVbleIndex, sizeof(int),
         9, "arrUnsetLemmaFlagVars");
   arrTempLemma = (int*)ite_calloc(gnMaxVbleIndex, sizeof(int),
         9, "arrTempLemma");
   arrLemmaFlag = (bool*)ite_calloc(gnMaxVbleIndex+1, sizeof(bool),
         9, "arrLemmaFlag");
   arrBacktrackStackIndex = (int*)ite_calloc(gnMaxVbleIndex+1, sizeof(int),
         9, "arrBacktrackStackIndex");

   // Initialization of variables and data structures
   assert(pTrueSmurfState);  // Should be initialized by SmurfFactory.
   arrFunctionType = functionType;
   arrFunctions = functions;

   /* init Inference queue */
   arrInferenceQueue
      = (int *)ite_calloc(nNumVariables, sizeof(*arrInferenceQueue), 2, 
            "inference queue");
   arrAFS = CreateAffectedFuncsStructures(gnMaxVbleIndex);

   if(nNumRegSmurfs > 0) {
      arrCurrentStates
         = (SmurfState **)ite_calloc(nNumRegSmurfs, sizeof(SmurfState *), 2,
               "current states");
      arrPrevStates
         = (SmurfState **)ite_calloc(nNumRegSmurfs, sizeof(SmurfState *), 2,
               "previous states");
   } else
      arrCurrentStates = NULL;

   if (nHeuristic == JOHNSON_HEURISTIC) {
      J_InitHeuristicScores();
   }

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


      assert(arrJWeights);
      for (int i = 0; i < nNumSpecialFuncs; i++) {
         arrPrevNumRHSUnknowns[i] =
         arrNumRHSUnknownsNew[i] =
         arrNumRHSUnknowns[i] = arrSpecialFuncs[i].rhsVbles.nNumElts;
         arrPrevNumLHSUnknowns[i] =
         arrNumLHSUnknownsNew[i] =
         arrNumLHSUnknowns[i] = arrSpecialFuncs[i].nLHSVble > 0? 1: 0;
         arrSumRHSUnknowns[i] = 0;

         for (int j=0; j<arrSpecialFuncs[i].rhsVbles.nNumElts; j++)
            arrSumRHSUnknowns[i] += arrJWeights[arrSpecialFuncs[i].rhsVbles.arrElts[j]];

         assert(arrSolution[0]!=BOOL_UNKNOWN);
      }
   }

ITE_INLINE void InitializeSmurfStatesStack();
ITE_INLINE void InitializeSpecialFnStack();
  /* have to have the stack at least for stages so I don't have to check
     the existence of special fn every time */
  InitializeSmurfStatesStack();
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
   arrBacktrackStack
      = (BacktrackStackEntry *)ite_calloc(nNumVariables, sizeof(*arrBacktrackStack), 2,
            "backtrack stack");
    /* init Fn Inference queue */
   arrFnInferenceQueue
      = (t_fn_inf_queue *)ite_calloc(nNumSpecialFuncs+nNumRegSmurfs+1, sizeof(t_fn_inf_queue), 2,
            "function inference queue");

   //  InitHeuristicTablesForSpecialFuncs(nNumVariables);

   int ret = RecordInitialInferences();

   // The variable with index 0 plays a special role (which is in handling
   // the PLAINOR functions).  We force its value to false before the
   // search begins.
   arrSolution[0] = BOOL_FALSE;
   /* for restart */
   for (int i = 1; i<nNumVariables; i++) {
      //assert(arrSolution[i] == BOOL_UNKNOWN);
      arrSolution[i] = BOOL_UNKNOWN;
   }

   switch (nHeuristic) {
    case JOHNSON_HEURISTIC:
       proc_call_heuristic = J_OptimizedHeuristic;
       D_9(
             DisplayJHeuristicValues();
          );
       break;
    case C_LEMMA_HEURISTIC:
       proc_call_heuristic = L_OptimizedHeuristic;
       break;
    case INTERACTIVE_HEURISTIC:
       proc_call_heuristic = I_OptimizedHeuristic;
       break;
    default:
       dE_printf1("Unknown heuristic\n");
       exit(1);
   }

   if (csv_trace_file) {
      fd_csv_trace_file = fopen(csv_trace_file, "w");
   }

   return ret;
}

ITE_INLINE void
FreeBrancher()
{
   d2_printf1("FreeBrancher\n");

   if (fd_csv_trace_file) fclose(fd_csv_trace_file);

   ite_free((void*)arrUnsetLemmaFlagVars);
   ite_free((void*)arrTempLemma);
   ite_free((void*)arrBacktrackStackIndex);
   ite_free((void*)arrLemmaFlag);

   ite_free((void*)arrInferenceQueue);
   ite_free((void*)arrFnInferenceQueue);

   ite_free((void*)arrNumRHSUnknowns);
   ite_free((void*)arrNumRHSUnknownsNew);
   ite_free((void*)arrPrevNumRHSUnknowns);

   ite_free((void*)arrNumLHSUnknowns);
   ite_free((void*)arrNumLHSUnknownsNew);
   ite_free((void*)arrPrevNumLHSUnknowns);

   ite_free((void*)arrSumRHSUnknowns);
   ite_free((void*)arrSumRHSUnknownsNew);
   ite_free((void*)arrPrevSumRHSUnknowns);

   ite_free((void*)arrCurrentStates);
   ite_free((void*)arrPrevStates);
   ite_free((void*)arrChangedSmurfs);
   ite_free((void*)arrChangedSpecialFn);
   ite_free((void*)arrChoicePointStack);
   ite_free((void*)arrBacktrackStack);
   ite_free((void*)arrVarScores);

   FreeAFS();
   if (nHeuristic == JOHNSON_HEURISTIC) {
      J_FreeHeuristicScores();
   }
   FreeSpecialFnStack();
   FreeSmurfStatesStack();
}
