#include "sbsat.h"
#include "sbsat_solver.h"
#include "solver.h"

int arrFnSmurfAuTypes[] = { AUTARKY_FUNC, 0 };

t_smurf_au_chain *arrSmurfAuChain;

SmurfAuState *pTrueSmurfAuState = NULL;  // Pointer to the SmurfAu state
//representing the Boolean function 'true'.

ITE_INLINE int
FnSmurfAuInit() 
{
   for(int j=0; arrFnSmurfAuTypes[j] != 0; j++)
   {
      int i=arrFnSmurfAuTypes[j];

      procCreateFunction[i] = SmurfAuCreateFunction;

      // Function Stack
      procSave2Stack[i] = SmurfAuSave2Stack;
      procRestoreFromStack[i] = SmurfAuRestoreFromStack;

      // Create and Update Affected Functions per variable
      procCreateAFS[i] = SmurfAuCreateAFS;
      procAffectedVarList[i] = SmurfAuAffectedVarList;
      procUpdateAffectedFunction[i] = SmurfAuUpdateAffectedFunction;
      procUpdateAffectedFunction_Infer[i] = SmurfAuUpdateAffectedFunction_Infer;
      procUpdateFunctionInfEnd[i] = SmurfAuUpdateFunctionInfEnd;
      //procFunctionFree[0] = FnSmurfAuFree;
   }

   // Initialize info regarding the 'true' function.
   pTrueSmurfAuState = AllocateSmurfAuState();

   return NO_ERROR;
}

ITE_INLINE void
SmurfAu_Free() {
   d9_printf1("FreeSmurfAuFactory\n");
   FreeSmurfAuStatePool();
}

