#include "sbsat.h"
#include "sbsat_solver.h"
#include "solver.h"

int arrFnXorSmurfTypes[] = { XOR_PART_BDDXOR, 0 };

ITE_INLINE int
FnXorSmurfInit()
{
   for(int j=0; arrFnXorSmurfTypes[j] != 0; j++)
   {
      int i=arrFnXorSmurfTypes[j];

      procCreateFunction[i] = XorCreateFunction;

      // Functino Stack
      procSave2Stack[i] = XorSave2Stack;
      procRestoreFromStack[i] = XorRestoreFromStack;

      // Create and Update Affected Functions per variable
      procCreateAFS[i] = XorCreateAFS;
      procAffectedVarList[i] = XorAffectedVarList;
      procUpdateAffectedFunction[i] = XorUpdateAffectedFunction;
      procUpdateAffectedFunction_Infer[i] = XorUpdateAffectedFunction_Infer;
      procUpdateFunctionInfEnd[i] = XorUpdateFunctionInfEnd;
      //procFunctionFree[0] = FnXorFree;

   }
   return NO_ERROR;
};

