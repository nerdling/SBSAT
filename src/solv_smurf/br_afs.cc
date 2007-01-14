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

OneAFS *arrAFSBufferFn = NULL;

ITE_INLINE int 
AFSInit()
   // Create an array of AffectedFuncsStructs.
   // The i-th structure in the array will indicate the functions
   // (constraints) which mention the i-th variable.
   // In the case where the function is a special func
   // (i.e. not a regular Smurf) the role of the variable in
   // the function will be indicated.
{
   arrAFS = (AffectedFuncsStruct*)ite_calloc(gnMaxVbleIndex+1, sizeof(AffectedFuncsStruct),
         9, "arrAFS");

   int nNumElementsFn = 0;

   // For each variable, count the number of funcs
   for (int nFnId = 0; nFnId < nmbrFunctions; nFnId++)
   {
      //t_smurf_chain *arrSmurfChain;

      int *arr1=NULL, num1=0, *arr2=NULL, num2=0;
      if (procAffectedVarList[arrSolverFunctions[nFnId].nType])
         procAffectedVarList[arrSolverFunctions[nFnId].nType](nFnId, &arr1, &num1, &arr2, &num2);

      nNumElementsFn += num1+num2;
      while(num1>0) {
         num1--;
         assert(arr1[num1] <= gnMaxVbleIndex);
         arrAFS[arr1[num1]].nNumOneAFS++;
      }
      while(num2>0) {
         num2--;
         assert(arr2[num2] <= gnMaxVbleIndex);
         arrAFS[arr2[num2]].nNumOneAFS++;
      }
   }

   arrAFSBufferFn = (OneAFS*)ite_calloc(nNumElementsFn, sizeof(OneAFS), 2, "AFS Fn");
   int nAFSBufferFn = 0;

   // Allocate arrays for each variable.
   for (int nVble = 0; nVble <= gnMaxVbleIndex; nVble++)
   {
      if (arrAFS[nVble].nNumOneAFS > 0) {
         arrAFS[nVble].arrOneAFS = arrAFSBufferFn + nAFSBufferFn;
         nAFSBufferFn += arrAFS[nVble].nNumOneAFS;
      }
      if (NO_LEMMAS == 0) LemmaCreateAFS(nVble);
   }

   // Setup two arrays of indicies used to keep track of
   // where to put the next piece of information into
   // arrAFS[nVble].arrRegSmurfsAffected
   // or arrAFS[nVble].arrSpecFuncsAffected.
   int *arrFuncIndexForVble = (int*)ite_calloc(gnMaxVbleIndex+1, sizeof(int),
         9, "arrSpecialFuncIndexForVble");

   // Now fill in the information concerning which variables are mentioned
   // in which functions.
  
   for (int nFnId = 0; nFnId < nmbrFunctions; nFnId++)
   {
      int *arr1=NULL, num1=0, *arr2=NULL, num2=0;
      if (procAffectedVarList[arrSolverFunctions[nFnId].nType])
         procAffectedVarList[arrSolverFunctions[nFnId].nType](nFnId, &arr1, &num1, &arr2, &num2);

      nNumElementsFn += num1+num2;
      while(num1>0) {
         num1--;
         int nVarId=arr1[num1];
         procCreateAFS[arrSolverFunctions[nFnId].nType]
            (nFnId, nVarId, arrFuncIndexForVble[nVarId]++);
      }
      while(num2>0) {
         num2--;
         int nVarId=arr2[num2];
         procCreateAFS[arrSolverFunctions[nFnId].nType]
            (nFnId, nVarId, arrFuncIndexForVble[nVarId]++);
      }
   }

   ite_free((void**)&arrFuncIndexForVble);

   return 0;
}



ITE_INLINE void
AFSFree()
{
   ite_free((void**)&arrAFSBufferFn);
   ite_free((void**)&arrAFS);
}

