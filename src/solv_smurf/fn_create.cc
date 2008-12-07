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

int gnMaxVbleIndex;

int smurfs_share_paths = 1;

ITE_INLINE char * StringFromFunctionType(int nFuncType);

ITE_INLINE void
FnCreateSkippedFunction(int nFnId, int nFnType)
{
   arrSolverFunctions[nFnId].nFnId = nFnId;
   arrSolverFunctions[nFnId].nType = nFnType;
}

ITE_INLINE int
CreateFunctions()
{
   double fStartTime = get_runtime();

   d9_printf1("CreateFunctions\n");

   nNumFuncs = nmbrFunctions;
   nNumUnresolvedFunctions = nNumFuncs; 
   arrSolverFunctions = (SolverFunction*)ite_calloc(nmbrFunctions, sizeof(SolverFunction), 2, "SolverFunctions");

	if(slide_lemmas) {
		if(arrSolverVarsInFunction == NULL)
		  arrSolverVarsInFunction = (int **)ite_calloc(nmbrFunctions, sizeof(int *), 9, "arrSolverVarsInFunction *");
		arrPattern = (int **)ite_calloc(nNumVariables, sizeof(int *), 9, "arrPattern");
		for(int x = 0; x < nNumVariables; x++)
		  arrPattern[x] = (int *)ite_calloc(4, sizeof(int), 9, "arrPattern[x]");
		arrTempSlideLemma = (int *)ite_calloc(nNumVariables, sizeof(int), 9, "arrTempSlideLemma");
		arrTempSlideSmurf = (int *)ite_calloc(nmbrFunctions, sizeof(int), 9, "arrTempSlideSmurf");		
	}
	
   for (int i = 0; i < nmbrFunctions; i++)
   {
		if(!smurfs_share_paths) { clear_all_bdd_pState(); true_ptr->pState = pTrueSmurfState;}
      int nFunctionType = functionType[i];
      BDDNodeStruct *pFunc = functions[i];
      if (pFunc == false_ptr)  return SOLV_UNSAT;

      if (procCreateFunction[nFunctionType]) {
         procCreateFunction[nFunctionType](i, functions[i], nFunctionType, equalityVble[i]);
			if(slide_lemmas && nFunctionType == UNSURE) {
				int length = arrSolverFunctions[i].fn_smurf.pInitialState->vbles.nNumElts;
				int *vars = arrSolverFunctions[i].fn_smurf.pInitialState->vbles.arrElts;
				arrSolverVarsInFunction[i] = (int *)ite_calloc(length+1, sizeof(int), 9, "arrSolverVarsInFunction");
				arrSolverVarsInFunction[i][0] = 0; 
				for(int x = 0; x < length; x++) {
					arrSolverVarsInFunction[i][x+1] = arrIte2SolverVarMap[vars[x]];
					//fprintf(stderr, "%d ", arrSolverVarsInFunction[i][x+1]);
				}
				//fprintf(stderr, "\n");
				qsort(arrSolverVarsInFunction[i], length+1, sizeof(int), compfunc);
				arrSolverVarsInFunction[i][0] = length; 						
			}
		} else {
			d4_printf3("Skipping function %d type %d\n", i, nFunctionType);
			FnCreateSkippedFunction(i, nFunctionType);
			nNumUnresolvedFunctions--;
		}
   }
		
	// Display statistics.
   double fEndTime = get_runtime();
   ite_counters_f[BUILD_SMURFS] = fEndTime - fStartTime;
   
   d3_printf2 ("Time to build functions:  %4.3f secs.\n", 
         ite_counters_f[BUILD_SMURFS]);
   d4_printf4("SMURF States Statistic: %ld/%ld (%f hit rate)\n",
         (long)(ite_counters[SMURF_NODE_FIND] - ite_counters[SMURF_NODE_NEW]),
         (long)(ite_counters[SMURF_NODE_FIND]),
         ite_counters[SMURF_NODE_FIND]==0?0:1.0 * (ite_counters[SMURF_NODE_FIND] - ite_counters[SMURF_NODE_NEW]) / ite_counters[SMURF_NODE_FIND]);
   d2e_printf1("\rCreating functions ... Done                     \n");

   return SOLV_UNKNOWN;
}

