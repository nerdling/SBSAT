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

      // Functino Stack
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
};

