
#include "sbsat.h"
#include "sbsat_solver.h"
#include "solver.h"

int arrFnMinMaxTypes[] = { MINMAX, 0 };

ITE_INLINE int
FnMinMaxInit()
{
   for(int j=0; arrFnMinMaxTypes[j] != 0; j++)
   {
      int i=arrFnMinMaxTypes[j];

      procCreateFunction[i] = MinMaxCreateFunction;

      // Functino Stack
      procSave2Stack[i] = MinMaxSave2Stack;
      procRestoreFromStack[i] = MinMaxRestoreFromStack;

      // Create and Update Affected Functions per variable
      procCreateAFS[i] = MinMaxCreateAFS;
      procAffectedVarList[i] = MinMaxAffectedVarList;
      procUpdateAffectedFunction[i] = MinMaxUpdateAffectedFunction;
      procUpdateAffectedFunction_Infer[i] = MinMaxUpdateAffectedFunction_Infer;
      procUpdateFunctionInfEnd[i] = MinMaxUpdateFunctionInfEnd;
      //procFunctionFree[0] = FnMinMaxFree;

   }
   return NO_ERROR;
};
