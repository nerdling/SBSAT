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
// SmurfFactory.cc
// Routines for creating state machines.
// Started 12/26/2000 - J. Ward

#include "ite.h"
#include "solver.h"

// External variables.
extern SmurfState *pTrueSmurfState;
SpecialFunc *arrSpecialFuncs =  0;
SmurfState **arrRegSmurfInitialStates = 0;
t_smurf_path *arrSmurfPath;
t_smurf_chain *arrSmurfChain;
int *arrSmurfEqualityVble;
int nNumSpecialFuncs;
int nNumRegSmurfs; // Number of regular Smurfs.
int nSpecialFuncIndex;
int nRegSmurfIndex;
int gnMaxVbleIndex;
extern int compress_smurfs;

extern BDDNode *false_ptr;
extern BDDNode *true_ptr;

ITE_INLINE void DisplaySmurfStates();
ITE_INLINE char * StringFromFunctionType(int nFuncType);
ITE_INLINE void InitSmurfFactory();

ITE_INLINE int
SmurfFactory()
{
   double fStartTime = get_runtime();

   InitSmurfFactory();

   d9_printf1("SmurfFactory\n");

   InitVbleMappingArray(total_vars + 1);
   InitVbleEncodingArray();
   if (SMURFS_SHARE_PATHS)
      InitPrecomputedStatesArray();
   InitLemmaLookupSpace();

   int *arrIte2SolverSpecFn = (int*)ite_calloc(nmbrFunctions, sizeof(int), 
         9, "arrIte2SolverSpecFn");
   int *arrIte2SolverSmurf = (int*)ite_calloc(nmbrFunctions, sizeof(int),
         9, "arrIte2SolverSmurf");
   for (int i = 0; i<nmbrFunctions; i++) {
      arrIte2SolverSpecFn[i] = -1;
      arrIte2SolverSmurf[i] = -1;
   }

   // Construct the Smurfs.
   for (int i = 0; i < nmbrFunctions; i++)
   {
      if (i%100 == 0)
         d2_printf3("\rCreating Smurfs ... %d/%d", i, nmbrFunctions);

      int nFunctionType = functionType[i];
      BDDNodeStruct *pFunc = functions[i];
      if (pFunc == false_ptr)  return SOLV_UNSAT;
      BDDNode *pSpecFunc = xorFunctions[i];

      if (pSpecFunc != NULL) 
      {
         d4_printf1("&");
         /* spec fn is XOR! */
      } else {
         if (IsSpecialFunc(nFunctionType))
         {
            pSpecFunc = functions[i];
         }
      }

      if (!IsSpecialFunc(nFunctionType))
      {
         d3_printf3("Constructing Smurf for %d/%d\r", i, nmbrFunctions);
         arrIte2SolverSmurf[i] = nRegSmurfIndex;
         arrSmurfEqualityVble[nRegSmurfIndex] = arrIte2SolverVarMap[equalityVble[i]];
         SmurfState *pSmurfState = BDD2Smurf(pFunc);

         if (pSmurfState == NULL)
         {
            fprintf(stderr, "Could not create the Smurf state.");
            return SOLV_ERROR;
         }
         arrRegSmurfInitialStates[nRegSmurfIndex] = pSmurfState;
         arrSmurfPath[nRegSmurfIndex].literals = 
            (int*)ite_calloc(arrRegSmurfInitialStates[nRegSmurfIndex]->vbles.nNumElts+1, sizeof(int),
               9, "arrSmurfPath[].literals");
         nRegSmurfIndex++;
   
         if (SMURFS_SHARE_PATHS)
            ResetPrecomputedStatesArray();
      } 

      if (pSpecFunc != NULL)
      {
         if (!IsSpecialFunc(nFunctionType)) {
            nFunctionType = PLAINXOR;
            arrSpecialFuncs[nSpecialFuncIndex].LinkedSmurfs = nRegSmurfIndex-1;
         } else {
            arrSpecialFuncs[nSpecialFuncIndex].LinkedSmurfs = -1;
         }
         d3_printf3("Constructing Special Function for %d/%d\r", i, nmbrFunctions);
         arrIte2SolverSpecFn[i] = nSpecialFuncIndex;
         SpecFn2Smurf(pSpecFunc, nFunctionType,
               equalityVble[i], &(arrSpecialFuncs[nSpecialFuncIndex]));
         nSpecialFuncIndex++;

      }

#ifdef PRINT_SMURF_STATS
      //printf("Constructed Smurf: %d  Total Smurf states: %d Func Type: %d",
      //i, nSmurfStatePoolIndex, functionType[i]);
      printf(" Top Vble: %d", functions[i]->variable);
      if (functionType[i] > 0)
      {
         printf(" LHS Vble: %d", equalityVble[i]);
      }
      printf("\n");
#endif
   }

   assert(nRegSmurfIndex == nNumRegSmurfs);
   if (nNumRegSmurfs) {
   }

   if (nSpecialFuncIndex) {
      for(int i=0; i < nSpecialFuncIndex; i++)
      {
         if (arrSpecialFuncs[i].nFunctionType == XOR &&
               arrSpecialFuncs[i].LinkedSmurfs != -1) {
            int smurf = arrSpecialFuncs[i].LinkedSmurfs;
            if (smurf != -1) {
               arrSmurfChain[smurf].specfn = i;
            }
         }
      }
   }

   free(arrIte2SolverSpecFn);
   free(arrIte2SolverSmurf);

   if (compress_smurfs == 0) {
      // Fill in lemma information
      for (int i = 0; i < nNumRegSmurfs; i++)
      {
         ComputeLemmasForSmurf(arrRegSmurfInitialStates[i]);
      }
      extern int  nTotalBytesForLemmaInferences;
      dm2_printf2("Allocated %d bytes for Lemma Inferences\n",
            nTotalBytesForLemmaInferences);
   }

   FreeVbleEncodingArray();
   if (SMURFS_SHARE_PATHS)
      FreePrecomputedStatesArray();
   FreeVbleMappingArray();
   FreeLemmaLookupSpace();

   if (nHeuristic == JOHNSON_HEURISTIC)  {
      InitHeuristicTablesForSpecialFuncs(gnMaxVbleIndex);
   }

   // Display statistics.
   double fEndTime = get_runtime();
   ite_counters_f[BUILD_SMURFS] = fEndTime - fStartTime;
   SmurfStatesDisplayInfo();
   d3_printf2 ("Time to build Smurf states:  %4.3f secs.\n", ite_counters_f[BUILD_SMURFS]);

   d4_printf4("SMURF States Statistic: %ld/%ld (%f hit rate)\n",
         (long)(ite_counters[SMURF_NODE_FIND] - ite_counters[SMURF_NODE_NEW]),
         (long)(ite_counters[SMURF_NODE_FIND]),
         ite_counters[SMURF_NODE_FIND]==0?0:1.0 * (ite_counters[SMURF_NODE_FIND] - ite_counters[SMURF_NODE_NEW]) / ite_counters[SMURF_NODE_FIND]);
   d2_printf1("\rCreating Smurfs ... Done                     \n");

   return SOLV_UNKNOWN;
}


ITE_INLINE void
InitSmurfFactory()
{
   d9_printf1("InitSmurfFactory\n");
   d2_printf1("Creating Smurfs ... ");
#ifdef DISPLAY_TRACE
   for (int i = 0; i < nmbrFunctions; i++)
   {
      cout << "Constraint " << i << ":" << endl;
      dbPrintBDD(functions[i]);
   }
#endif


   // Make sure that the addons pointer of each BDD is NULL.
   // there are two cycles since the parts of BDD might be shared
   //for (int i = 0; i < nmbrFunctions; i++)
   //    AssertNullAddons(functions[i]);

   for (int i = 0; i < nmbrFunctions; i++) {
      InitializeAddons(functions[i]);
      if (xorFunctions[i])
         InitializeAddons(xorFunctions[i]);
   }

   InitializeAddons(true_ptr);

   // Initialize info regarding the 'true' function.
   pTrueSmurfState = AllocateSmurfState();
   ITE_NEW_CATCH(
         true_ptr->addons->pImplied = new LiteralSet();,
         "true_ptr->addons->pImplied");
   assert(true_ptr->addons->pImplied->IsEmpty());
   true_ptr->addons->pReduct = true_ptr;
   ITE_NEW_CATCH(
         true_ptr->addons->pVbles = new IntegerSet();,
         "true_ptr->addons->pVbles");
   assert(true_ptr->addons->pVbles->IsEmpty());

   // Count number of special functions
   // See "struct SpecialFunc" in SmurfFactory.h for a description
   // of what a "special function" is.
   nNumSpecialFuncs = 0;
   nNumRegSmurfs = 0;
   for (int i = 0; i < nmbrFunctions; i++)
   {
      int nFunctionType = functionType[i];
      if (nFunctionType != UNSURE && nFunctionType != ITE
            && !IsSpecialFunc(nFunctionType))
      {
         dE_printf3("SmurfFactory -- Unrecognized function type: %d for %d assuming UNSURE\n", 
               nFunctionType, i);
         nFunctionType = UNSURE;
      }

      BDDNode *pFunc = functions[i];
      ComputeVbleSet(pFunc);

      if (xorFunctions[i]) {
         nNumSpecialFuncs++;
         nNumRegSmurfs++;
         ComputeVbleSet(xorFunctions[i]);
      } else
      if (IsSpecialFunc(functionType[i])) 
         nNumSpecialFuncs++;
      else
         nNumRegSmurfs++;

   }

   /* allocate an array for special functions */
   d3_printf2("Number of special functions: %d\n", nNumSpecialFuncs);
   if (nNumSpecialFuncs > 0)
   {
      arrSpecialFuncs
         = (SpecialFunc *)ite_calloc(nNumSpecialFuncs, sizeof(SpecialFunc),
               2, "special function array");
   }
   nSpecialFuncIndex = 0;

   /* allocate an array for smurf initial states */
   d3_printf2("Number of regular Smurfs: %d\n", nNumRegSmurfs);
   if (nNumRegSmurfs > 0)
   {
      arrRegSmurfInitialStates
         = (SmurfState **)ite_calloc(nNumRegSmurfs, sizeof(SmurfState *),
               2, "initial smurf state pointers");
      arrSmurfEqualityVble = (int*)ite_calloc(nNumRegSmurfs, sizeof(int),
            9, "arrSmurfEqualityVble");
      arrSmurfPath = (t_smurf_path*)ite_calloc(nNumRegSmurfs, sizeof(t_smurf_path),
            9, "arrSmurfPath");

      arrSmurfChain = (t_smurf_chain*)ite_calloc(nNumRegSmurfs, sizeof(t_smurf_chain),
            9, "arrSmurfChain");

      for (int i = 0; i < nNumRegSmurfs; i++)
      {
         arrSmurfChain[i].next = -1;
         arrSmurfChain[i].prev = -1;
         arrSmurfChain[i].specfn = -1;
      }

   }
   nRegSmurfIndex = 0;
}

ITE_INLINE void
FreeSmurfFactory()
{
   d9_printf1("FreeSmurfFactory\n");
   if (nHeuristic == JOHNSON_HEURISTIC) {
      FreeHeuristicTablesForSpecialFuncs();
   }

   /* from smurf factory */
   for (int i = 0; i < nNumRegSmurfs; i++)
      ite_free((void*)arrSmurfPath[i].literals);

   ite_free((void*)arrSmurfPath);
   ite_free((void*)arrSmurfChain);
   ite_free((void*)arrSmurfEqualityVble);
   ite_free((void*)arrRegSmurfInitialStates);
   nRegSmurfIndex = 0;

   for(int i=0; i < nNumSpecialFuncs; i++) {
      ite_free((void*)arrSpecialFuncs[i].rhsVbles.arrElts);
      ite_free((void*)arrSpecialFuncs[i].arrRHSPolarities);
      ite_free((void*)arrSpecialFuncs[i].arrShortLemmas);
   }
   ite_free((void*)arrSpecialFuncs);
   nSpecialFuncIndex = 0;

   FreeAddonsPool();
   FreeSmurfStatePool();
}

