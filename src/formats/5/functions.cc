#include "ite.h"

#define MAX_FUNCTION 1000000
//int nmbrFunctions=0;
//BDDNode **functions=NULL;
//int *functionType=NULL;
//int *equalityVble=NULL;
int normal_bdds=0;
int spec_fn_bdds=0;

void
functions_add(BDDNode *bdd, int fn_type, int equal_var)
{
  
  switch (fn_type) {
   case UNSURE: fn_type=UNSURE; break;
   case AND_EQU: fn_type=AND; break;
   case OR_EQU: fn_type=OR; break;
   case ITE_EQU: fn_type=ITE; break;
   case EQU: fn_type=EQU; break;
   case PLAINOR: fn_type=PLAINOR; break;
   default: fprintf(stderr, "unknown function type: %d\n", fn_type);
            fn_type=UNSURE;
  }

  if (fn_type==UNSURE) normal_bdds++;
  else spec_fn_bdds++;

  equalityVble[nmbrFunctions] = equal_var;
  functionType[nmbrFunctions] = fn_type;
  functions[nmbrFunctions] = bdd;
  nmbrFunctions++;
}

void
functions_init()
{


}

void
functions_create()
{


}
