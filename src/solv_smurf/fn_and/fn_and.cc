#include "sbsat.h"
#include "sbsat_solver.h"
#include "solver.h"

int arrFnAndTypes[] = { 1, AND, AND_EQUAL, AND_EQU, AND_EQUAL_EQU, OR, OR_EQU, PLAINOR, PLAINOR_EQU, 0};


ITE_INLINE int
FnAndInit() 
{
   for(int j=0; arrFnAndTypes[j] != 0; j++)
   {
      int i=arrFnAndTypes[j];

      procCreateFunction[i] = AndCreateFunction;

      // Functino Stack
      procSave2Stack[i] = AndSave2Stack;
      procRestoreFromStack[i] = AndRestoreFromStack;

      // Create and Update Affected Functions per variable
      procCreateAFS[i] = AndCreateAFS;
      procAffectedVarList[i] = AndAffectedVarList;
      procUpdateAffectedFunction[i] = AndUpdateAffectedFunction;
      procUpdateAffectedFunction_Infer[i] = AndUpdateAffectedFunction_Infer;
      procUpdateFunctionInfEnd[i] = AndUpdateFunctionInfEnd;
      //procFunctionFree[0] = FnAndFree;

   }
   return NO_ERROR;
}
