#include "sbsat.h"
#include "sbsat_solver.h"
#include "solver.h"

ITE_INLINE void
InitFunctions()
{
   for(int i=0;procFnInit[i]!=NULL;i++)
      procFnInit[i]();
}


ITE_INLINE void
FreeFunctions()
{
//   for(int i=0;procFnInit[i]!=NULL;i++)
//      procFnFree[i]();
}
