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
BacktrackStackEntry *arrBacktrackStack=NULL; 
BacktrackStackEntry *pStartBacktrackStack=NULL; 
int * arrBacktrackStackIndex = NULL;

ITE_INLINE void FreeHeuristicTablesForSpecialFuncs();


ITE_INLINE int
RecordInitialInferences()
{
   int i;
   BDDNode *pFunc;
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
      infer *head = pFunc->inferences;
      while(head != NULL)
      {
         if (head->nums[1] != 0) { head=head->next; continue; }
         int nVble = head->nums[0];
         head=head->next;
         int nValueOfVble = BOOL_TRUE;;
         if (nVble < 0) { 
            nVble = -nVble; 
            nValueOfVble = BOOL_FALSE; 
         }

         if (!bInitialInferenceFound)
         {
            printf("\nWarning.  Inference found in constraint %d", i);
            printf("\n prior to beginning backtracking search:  ");
            printf("\nBrancher variable %d(%d) = ", arrIte2SolverVarMap[nVble], nVble);
            assert(nValueOfVble == BOOL_TRUE || nValueOfVble == BOOL_FALSE);
            printf((nValueOfVble == BOOL_TRUE) ? "true." : "false.");
            printf("\nCurrent value = ");
            printf((arrSolution[arrIte2SolverVarMap[nVble]] == BOOL_TRUE) ? "true." :
                  (arrSolution[arrIte2SolverVarMap[nVble]] == BOOL_FALSE) ?  "false." :
               "unknown");
            printf("\n numinp = %ld numout = %ld", (long)numinp, (long)numout);
            D_3(printBDD(arrFunctions[i]); )
            printf("\nThis message will appear only once for the first inference found\n");
//            bInitialInferenceFound = true;
         }
         nVble = arrIte2SolverVarMap[nVble];
         assert(nVble != 0);

         nCurrentAtomValue = arrSolution[nVble];
         if (nValueOfVble == BOOL_TRUE)
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

   int nSpecialFuncIndex = 0;
   int nRegSmurfIndex = 0;

   // For each variable, count the number of special funcs
   // and regular Smurfs which will mention it when the brancher starts.
   for (int nFuncIndex = 0; nFuncIndex < nmbrFunctions; nFuncIndex++)
   {
      //t_smurf_chain *arrSmurfChain;

      SpecialFunc *pSpecialFunc = NULL;
      if (IsSpecialFunc(arrFuncType[nFuncIndex]) || arrSmurfChain[nRegSmurfIndex].specfn != -1) 
         pSpecialFunc = arrSpecialFuncs + nSpecialFuncIndex;

      if (pSpecialFunc)
      {
         if (pSpecialFunc->nLHSVble != 0)
         {
            nVble = pSpecialFunc->nLHSVble;
            arrAFS[nVble].nNumSpecialFuncsAffected++;
            nNumElementsSpecFn++;
         }
         for(int i=0;i<pSpecialFunc->rhsVbles.nNumElts;i++)
         {
            nVble = pSpecialFunc->rhsVbles.arrElts[i];
            arrAFS[nVble].nNumSpecialFuncsAffected++;
         }
         nNumElementsSpecFn+=pSpecialFunc->rhsVbles.nNumElts;
         nSpecialFuncIndex++;
      }

      if (IsSpecialFunc(arrFuncType[nFuncIndex])) {}
      else
      {
         for(int i=0;i<arrRegSmurfInitialStates[nRegSmurfIndex]->vbles.nNumElts;i++)
         {
            nVble = arrRegSmurfInitialStates[nRegSmurfIndex]->vbles.arrElts[i];
            arrAFS[nVble].nNumRegSmurfsAffected++;
         }
         nNumElementsSmurfs+=arrRegSmurfInitialStates[nRegSmurfIndex]->vbles.nNumElts;
         nRegSmurfIndex++;
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
  
   nSpecialFuncIndex = 0;
   nRegSmurfIndex = 0;
   for (int nFuncIndex = 0; nFuncIndex < nmbrFunctions; nFuncIndex++)
   {
      int tmp_nSpecialFuncIndex = -1;
      if (IsSpecialFunc(arrFuncType[nFuncIndex]) || arrSmurfChain[nRegSmurfIndex].specfn != -1) 
         tmp_nSpecialFuncIndex = nSpecialFuncIndex;

      if (tmp_nSpecialFuncIndex != -1)
      {
         SpecialFunc *pSpecialFunc = arrSpecialFuncs + tmp_nSpecialFuncIndex;
         if (pSpecialFunc->nLHSVble != 0)
         {
            int nVble = pSpecialFunc->nLHSVble;
            int nIndex = arrSpecialFuncIndexForVble[nVble];
            arrAFS[nVble].arrSpecFuncsAffected[nIndex]
               .nSpecFuncIndex = tmp_nSpecialFuncIndex;
            if (pSpecialFunc->nLHSPolarity == BOOL_TRUE)
            {
               arrAFS[nVble].arrSpecFuncsAffected[nIndex]
                  .nPolarity = BOOL_TRUE;
            }
            else
            {
               arrAFS[nVble].arrSpecFuncsAffected[nIndex]
                  .nPolarity = BOOL_FALSE;
            }
            arrAFS[nVble].arrSpecFuncsAffected[nIndex].nRHSIndex = -1;
            arrSpecialFuncIndexForVble[nVble]++;
         }
         for(int i=0;i<pSpecialFunc->rhsVbles.nNumElts;i++)
         {
            int nVble = pSpecialFunc->rhsVbles.arrElts[i];
            int nIndex = arrSpecialFuncIndexForVble[nVble];
            arrAFS[nVble].arrSpecFuncsAffected[nIndex]
               .nSpecFuncIndex = tmp_nSpecialFuncIndex;
            if (pSpecialFunc->arrRHSPolarities[i] == BOOL_TRUE)
            {
               arrAFS[nVble].arrSpecFuncsAffected[nIndex]
                  .nPolarity = BOOL_TRUE;
            }
            else
            {
               arrAFS[nVble].arrSpecFuncsAffected[nIndex]
                  .nPolarity = BOOL_FALSE;
            }
            arrAFS[nVble].arrSpecFuncsAffected[nIndex].nRHSIndex = i;
            arrSpecialFuncIndexForVble[nVble]++;
         }
         nSpecialFuncIndex++;
      }

      if (IsSpecialFunc(arrFuncType[nFuncIndex])) {}
      else
      {
         for(int i=0;i<arrRegSmurfInitialStates[nRegSmurfIndex]->vbles.nNumElts;i++)
         {
            int nVble = arrRegSmurfInitialStates[nRegSmurfIndex]->vbles.arrElts[i];
            int nIndex = arrRegSmurfIndexForVble[nVble];
            arrAFS[nVble].arrRegSmurfsAffected[nIndex] = nRegSmurfIndex;
            arrRegSmurfIndexForVble[nVble]++;
         }
         nRegSmurfIndex++;
      }
   }

   ite_free((void**)&arrRegSmurfIndexForVble);
   ite_free((void**)&arrSpecialFuncIndexForVble);

   return arrAFS;
}

ITE_INLINE void
FreeAFS()
{
   ite_free((void**)&arrAFSBufferSmurfs);
   ite_free((void**)&arrAFSBufferSpecFn);
   ite_free((void**)&arrAFS);
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
   d5_printf1("InitBrancher\n");

   switch (nHeuristic) {
    case JOHNSON_HEURISTIC:
       proc_update_heuristic = J_UpdateHeuristic;
       if (sHeuristic[1] == 'l') {
         int i = strlen(sHeuristic);
         if (i>8) i=8;
         memmove(sHeuristic+1, sHeuristic+2, i-1); 
         proc_call_heuristic = J_OptimizedHeuristic_l;
      } else {
         proc_call_heuristic = J_OptimizedHeuristic;
      }
      D_9(
             DisplayJHeuristicValues();
          );
       break;
    case C_LEMMA_HEURISTIC:
       proc_call_heuristic = L_OptimizedHeuristic;
       proc_update_heuristic = UpdateHeuristic;
       break;
    case INTERACTIVE_HEURISTIC:
       proc_call_heuristic = I_OptimizedHeuristic;
       proc_update_heuristic = UpdateHeuristic;
       break;
    default:
       dE_printf1("Unknown heuristic\n");
       exit(1);
   }


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

      arrRHSCounter = (int *)ite_calloc(nNumSpecialFuncs, sizeof(int),
            9, "arrRHSCounter");
      arrRHSCounterNew = (int *)ite_calloc(nNumSpecialFuncs, sizeof(int),
            9, "arrRHSCounterNew");
      arrPrevRHSCounter = (int *)ite_calloc(nNumSpecialFuncs, sizeof(int),
            9, "arrPrevRHSCounter");

      if (nHeuristic == JOHNSON_HEURISTIC) {
         assert(arrJWeights);
         for (int i = 0; i < nNumSpecialFuncs; i++) {
            for (int j=0; j<arrSpecialFuncs[i].rhsVbles.nNumElts; j++)
               arrSumRHSUnknowns[i] += arrJWeights[arrSpecialFuncs[i].rhsVbles.arrElts[j]];
         }
      }

      for (int i = 0; i < nNumSpecialFuncs; i++) {
         arrPrevNumRHSUnknowns[i] =
         arrNumRHSUnknownsNew[i] =
         arrNumRHSUnknowns[i] = arrSpecialFuncs[i].rhsVbles.nNumElts;
         arrPrevNumLHSUnknowns[i] =
         arrNumLHSUnknownsNew[i] =
         arrNumLHSUnknowns[i] = arrSpecialFuncs[i].nLHSVble > 0? 1: 0;
         arrSumRHSUnknowns[i] = 0;
         arrPrevRHSCounter[i] =
         arrRHSCounterNew[i] =
         arrRHSCounter[i] = 0;

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

   // The variable with index 0 plays a special role (which is in handling
   // the PLAINOR functions).  We force its value to false before the
   // search begins.
   arrSolution[0] = BOOL_FALSE;
   /* for restart */
   for (int i = 1; i<nNumVariables; i++) {
      //assert(arrSolution[i] == BOOL_UNKNOWN);
      arrSolution[i] = BOOL_UNKNOWN;
   }

   if (*csv_trace_file) {
      fd_csv_trace_file = fopen(csv_trace_file, "w");
   }

   return SOLV_UNKNOWN;
}

ITE_INLINE void
FreeBrancher()
{
   d5_printf1("FreeBrancher\n");

   if (fd_csv_trace_file) fclose(fd_csv_trace_file);

   ite_free((void**)&arrUnsetLemmaFlagVars);
   ite_free((void**)&arrTempLemma);
   ite_free((void**)&arrBacktrackStackIndex);
   ite_free((void**)&arrLemmaFlag);

   ite_free((void**)&arrInferenceQueue);
   ite_free((void**)&arrFnInferenceQueue);

   ite_free((void**)&arrNumRHSUnknowns);
   ite_free((void**)&arrNumRHSUnknownsNew);
   ite_free((void**)&arrPrevNumRHSUnknowns);

   ite_free((void**)&arrNumLHSUnknowns);
   ite_free((void**)&arrNumLHSUnknownsNew);
   ite_free((void**)&arrPrevNumLHSUnknowns);

   ite_free((void**)&arrSumRHSUnknowns);
   ite_free((void**)&arrSumRHSUnknownsNew);
   ite_free((void**)&arrPrevSumRHSUnknowns);

   ite_free((void**)&arrRHSCounter);
   ite_free((void**)&arrRHSCounterNew);
   ite_free((void**)&arrPrevRHSCounter);

   ite_free((void**)&arrCurrentStates);
   ite_free((void**)&arrPrevStates);
   ite_free((void**)&arrChangedSmurfs);
   ite_free((void**)&arrChangedSpecialFn);
   ite_free((void**)&arrChoicePointStack);
   ite_free((void**)&arrBacktrackStack);
   ite_free((void**)&arrVarScores);

   FreeAFS();
   if (nHeuristic == JOHNSON_HEURISTIC) {
      J_FreeHeuristicScores();
   }
   FreeSpecialFnStack();
   FreeSmurfStatesStack();
}
