/* =========FOR INTERNAL USE ONLY. NO DISTRIBUTION PLEASE ========== */

/*********************************************************************
 Copyright 1999-2003, University of Cincinnati.  All rights reserved.
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
 any trademark,  service mark, or the name of University of Cincinnati.


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

#include "ite.h"

/*
 * initializes
 * independantVars[] = 1
 *
 * nmbrFunctions = 0
 * equalityVble[]
 * functions[]
 * functionType[] = UNSURE
 *
 */ 


int functions_max = 0;
int vars_max = 0;

int
functions_alloc(int n_fns)
{
   /* n_fns -- numout */
   equalityVble = (int *)ite_recalloc((void*)equalityVble, functions_max, 
         n_fns, sizeof(int), 9, "equalityVble");
   functions = (BDDNode **)ite_recalloc((void*)functions, functions_max,
         n_fns, sizeof(BDDNode*), 9, "functions");
   xorFunctions = (BDDNode **)ite_recalloc((void*)xorFunctions, functions_max, 
         n_fns, sizeof(BDDNode*), 9, "functions");
   functionType = (int *)ite_recalloc((void*)functionType, functions_max,
         n_fns, sizeof(int), 9, "functionType");

   functions_max = n_fns;
   return 0;
}

int
vars_alloc(int n_vars)
{

   /* n_vars -- numinp */
   independantVars = (int *)ite_recalloc((void*)independantVars, vars_max,
         n_vars, sizeof(int), 9, "independantVars");

   for(int x = vars_max; x < n_vars; x++) 
   {
      independantVars[x] = 1;
   }
   vars_max = n_vars;
   return 0;
}


void
bdd_circuit_free()
{
   ite_free((void**)&independantVars);
   ite_free((void**)&equalityVble);
   ite_free((void**)&functions);
   ite_free((void**)&xorFunctions);
   ite_free((void**)&functionType);
}

