#include "ite.h"
#include "bddnode.h"
#include "symtable.h"
#include "functions.h"

int normal_bdds=0;
int spec_fn_bdds=0;

BDDNode *tmp_equ_var(BDDNode *p) 
{
    symrec *s_ptr = tputsym(SYM_VAR); 
    BDDNode *ret=ite_vars(s_ptr); 
    BDDNode *e=ite_equ(ret, p); 
    functions_add(e, UNSURE, s_ptr->id); 
    return ret;
}

void
functions_add(BDDNode *bdd, int fn_type, int equal_var)
{
 if (nmbrFunctions >= functions_max) 
  functions_alloc(nmbrFunctions+100);

  switch (fn_type) {
   case UNSURE: fn_type=UNSURE; break;
   case AND_EQU: fn_type=AND; break;
   case OR_EQU: fn_type=OR; break;
   case ITE_EQU: fn_type=ITE; break;
   case EQU: fn_type=EQU; break;
   case PLAINOR: fn_type=PLAINOR; break;
   case LIMP_EQU: fn_type=UNSURE; break;
   case LNIMP_EQU: fn_type=UNSURE; break;
   case RIMP_EQU: fn_type=UNSURE; break;
   case RNIMP_EQU: fn_type=UNSURE; break;
   case XOR_EQU: fn_type=UNSURE; break;
   case NAND_EQU: fn_type=UNSURE; break;
   case NOR_EQU: fn_type=UNSURE; break;
   case EQU_EQU: fn_type=UNSURE; break;
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

