#include "sbsat.h"
#include "sbsat_solver.h"
#include "solver.h"

int *arrChoicePointHint = NULL;
int nChoicePointHintIndex = 0;

/* convert these to MACROS */

void InitChoicePointHint()
{
   arrChoicePointHint = (int*)ite_calloc(nNumVariables, sizeof(int), 2, "");
}

void FreeChoicePointHint()
{
   ite_free((void**)&arrChoicePointHint);
}

void AddChoicePointHint(int x)
{
   arrChoicePointHint[nChoicePointHintIndex++] = x;
}

int GetChoicePointHint()
{
   while(nChoicePointHintIndex && 
         arrChoicePointHint[nChoicePointHintIndex-1] != BOOL_UNKNOWN)
      nChoicePointHintIndex--;
   if (nChoicePointHintIndex == 0) return 0;
   return arrChoicePointHint[--nChoicePointHintIndex];
}

void EmptyChoicePointHint()
{
   nChoicePointHintIndex = 0;
}
