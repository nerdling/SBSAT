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

int arrFnSmurfXorTypes[] = { BDD_PART_BDDXOR, 0 };

ITE_INLINE int
FnSmurfXorInit()
{
   for(int j=0; arrFnSmurfXorTypes[j] != 0; j++)
   {
      int i=arrFnSmurfXorTypes[j];

      procCreateFunction[i] = SmurfCreateFunction;

      // Function Stack
      procSave2Stack[i] = SmurfSave2Stack;
      procRestoreFromStack[i] = SmurfRestoreFromStack;

      // Create and Update Affected Functions per variable
      procCreateAFS[i] = SmurfCreateAFS;
      procAffectedVarList[i] = SmurfAffectedVarList;
      procUpdateAffectedFunction[i] = SmurfUpdateAffectedFunction;
      procUpdateAffectedFunction_Infer[i] = SmurfUpdateAffectedFunction_Infer;
      procUpdateFunctionInfEnd[i] = SmurfUpdateFunctionInfEnd;
      //procFunctionFree[0] = FnXorFree;

   }
   { /* don't handle BDDXOR_BROKEN anymore */
      int i=BDDXOR_BROKEN;
      procCreateFunction[i] = NULL;

      // Functino Stack
      procSave2Stack[i] = NULL;
      procRestoreFromStack[i] = NULL;

      // Create and Update Affected Functions per variable
      procCreateAFS[i] = NULL;
      procAffectedVarList[i] = NULL;
      procUpdateAffectedFunction[i] = NULL;
      procUpdateAffectedFunction_Infer[i] = NULL;
      procUpdateFunctionInfEnd[i] = NULL;
      //procFunctionFree[0] = FnXorFree;
   }
   return NO_ERROR;
}

