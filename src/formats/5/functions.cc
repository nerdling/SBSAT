/* =========FOR INTERNAL USE ONLY. NO DISTRIBUTION PLEASE ========== */

/*********************************************************************
 Copyright 1999-2004, University of Cincinnati.  All rights reserved.
 By using this software the USER indicates that he or she has read,
 understood and will comply with the following:

 --- University of Cincinnati hereby grants USER nonexclusive permission
 to use, copy and/or modify this software for internal, noncommercial,
 research purposes only. Any distribution, including commercial sale
 or license, of this software, copies of the software, its associated
 documentation and/or modifications of either is strictly prohibited
 without the prior consent of University of Cincinnati.  Title to copyright
 to this software and its associated documentation shall at all times
 remain with University of Cincinnati.  Appropriate copyright notice shall
 be placed on all software copies, and a complete copy of this notice
 shall be included in all copies of the associated documentation.
 No right is  granted to use in advertising, publicity or otherwise
 any trademark, service mark, or the name of University of Cincinnati.


 --- This software and any associated documentation is provided "as is"

 UNIVERSITY OF CINCINNATI MAKES NO REPRESENTATIONS OR WARRANTIES, EXPRESS
 OR IMPLIED, INCLUDING THOSE OF MERCHANTABILITY OR FITNESS FOR A
 PARTICULAR PURPOSE, OR THAT  USE OF THE SOFTWARE, MODIFICATIONS, OR
 ASSOCIATED DOCUMENTATION WILL NOT INFRINGE ANY PATENTS, COPYRIGHTS,
 TRADEMARKS OR OTHER INTELLECTUAL PROPERTY RIGHTS OF A THIRD PARTY.

 University of Cincinnati shall not be liable under any circumstances for
 any direct, indirect, special, incidental, or consequential damages
 with respect to any claim by USER or any third party on account of
 or arising from the use, or inability to use, this software or its
 associated documentation, even if University of Cincinnati has been advised
 of the possibility of those damages.
*********************************************************************/

#include "sbsat.h"
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

