/* =========FOR INTERNAL USE ONLY. NO DISTRIBUTION PLEASE ========== */

/*********************************************************************
 Copyright 1999-2004, University of Cincinnati.  All rights reserved.
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

ITE_INLINE char * StringFromFunctionType(int nFuncType);


ITE_INLINE int
CreateFunctions()
{
   double fStartTime = get_runtime();

   d9_printf1("CreateFunctions\n");

   nNumFuncs = nmbrFunctions;
   nNumUnresolvedFunctions = nNumFuncs; 
   arrSolverFunctions = (SolverFunction*)ite_calloc(nmbrFunctions, sizeof(SolverFunction), 2, "SolverFunctions");

   for (int i = 0; i < nmbrFunctions; i++)
   {
      int nFunctionType = functionType[i];
      BDDNodeStruct *pFunc = functions[i];
      if (pFunc == false_ptr)  return SOLV_UNSAT;

      procCreateFunction[nFunctionType](i, functions[i], nFunctionType, equalityVble[i]);
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

