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
 * parameterizedVars[] = NULL
 * parameterGroup[] = 0
 *
 */ 

int
bdd_circuit_init(int n_vars, int n_fns) /* numinp, numout */
{
   int x;

   ZERO_ONE_VAR = n_vars + 3;

   bdd_init();

   /* n_vars -- numinp */
   independantVars = (int *)calloc(n_vars, sizeof(int));
   if (independantVars == NULL) 
   {
      dE_printf1("Unable to allocate memory for independant vars.\n");
      exit(1);
   }
   for(x = 0; x < n_vars; x++) 
   {
      independantVars[x] = 1;
   }


   /* n_fns -- numout */
   nmbrFunctions = 0;
   equalityVble = (int *)calloc(n_fns, sizeof(int));
   if (equalityVble == NULL) 
   {
      dE_printf1("Unable to allocate memory for equalityeVble.\n");
      exit(1);
   }

   functions = (BDDNode **)calloc(n_fns, sizeof(BDDNode*));
   if (functions == NULL) 
   {
      dE_printf1("Unable to allocate memory for functions.\n");
      exit(1);
   }

   xorFunctions = (BDDNode **)calloc(n_fns, sizeof(BDDNode*));
   if (xorFunctions == NULL) 
   {
      dE_printf1("Unable to allocate memory for xorFunctions.\n");
      exit(1);
   }

   functionType = (int *)calloc(n_fns, sizeof(int));
   if (functionType == NULL) 
   {
      dE_printf1("Unable to allocate memory for functionType.\n");
      exit(1);
   }

   parameterizedVars = (int **)calloc(n_fns, sizeof(int *));
   if (parameterizedVars == NULL) 
   {
      dE_printf1("Unable to allocate memory for parameterizedVars.\n");
      exit(1);
   }

   parameterGroup = (int *)calloc(n_fns, sizeof(int));
   if (parameterGroup == NULL) 
   {
      dE_printf1("Unable to allocate memory for parameterGroup.\n");
      exit(1);
   }

   //for(x = 0; x < n_fns; x++) 
   //{
		//functionType[x] = UNSURE;
      //parameterizedVars[x] = NULL;
      //parameterGroup[x] = 0;
   //}

  return 0;
}

void
bdd_circuit_free()
{
   free(independantVars);
   independantVars = NULL;
   free(equalityVble);
   equalityVble = NULL;
   free(functions);
   functions = NULL;
   free(xorFunctions);
   xorFunctions = NULL;
   free(functionType);
   functionType = NULL;
   free(parameterizedVars);
   parameterizedVars = NULL;
   free(parameterGroup);
   parameterGroup = NULL;
}

