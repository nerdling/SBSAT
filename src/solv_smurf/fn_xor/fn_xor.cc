

#include "sbsat.h"
#include "sbsat_solver.h"
#include "solver.h"

int arrFnXorTypes[] = { PLAINXOR, 0 };

ITE_INLINE int
FnXorInit()
{
   for(int j=0; arrFnXorTypes[j] != 0; j++)
   {
      int i=arrFnXorTypes[j];

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
