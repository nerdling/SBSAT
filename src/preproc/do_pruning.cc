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
/*********************************************************
 *  preprocess.c (S. Weaver)
 *********************************************************/

#include "ite.h"
#include "preprocess.h"

extern int STRENGTH;
void Sort_BDDs (int *tempint);

#ifdef DO_PRUNING_FN
# error "DO_PRUNING_FN defined -- undefine it"
#endif
#ifdef P1
# define DO_PRUNING_FN Do_Pruning_1
# define PRUNE_REPEATS P1_repeat
#else
#ifdef P2
# define DO_PRUNING_FN Do_Pruning_2
# define PRUNE_REPEATS P2_repeat
#else
#ifdef RESTRICT
# define DO_PRUNING_FN Do_Pruning_Restrict
# define PRUNE_REPEATS Restct_repeat
#else
# define DO_PRUNING_FN Do_Pruning
# define REMOVE_FPS
# define PRUNE_REPEATS ReFPS_repeat
#endif
#endif
#endif

//#define P1 
//#define P2
//#define RESTRICT
//#define REMOVE_FPS (default)

int DO_PRUNING_FN() {
	int ret = PREP_NO_CHANGE;
	
	int *repeat_small = new int[nmbrFunctions + 1];
	
	for (int x = 0; x < nmbrFunctions; x++)
	  repeat_small[x] = PRUNE_REPEATS[x];

   D_3(print_roller_init(););
	d3_printf1 ("BRANCH PRUNING -  ");
	for (int x = 0; x < nmbrFunctions; x++)
	  {
        if (x % 100 == 0)
           d2e_printf3("\rPreprocessing Pr %d/%d", x, nmbrFunctions);
        PRUNE_REPEATS[x] = 0;
		  if (functions[x] == true_ptr)
			 continue;
		  for (int j = x + 1; j < nmbrFunctions; j++)
			 {
				 //fprintf(stderr, "\n\n");
				 //printBDDerr(functions[x]);
				 //fprintf(stderr, "\n");
				 
				 if (functions[j] == true_ptr)
					continue;
				 if ((!repeat_small[j] && !repeat_small[x]))
					continue;
				 if (nmbrVarsInCommon (x, j, length, variables, STRENGTH) < STRENGTH)
					continue;
				 
				 if ((functionType[x] != AND || length[x] < AND_EQU_LIMIT)
					  && (functionType[x] != OR || length[x] < OR_EQU_LIMIT)
					  && (functionType[x] != PLAINOR || length[x] < PLAINOR_LIMIT)
					  && (functionType[x] != PLAINXOR || length[x] < PLAINXOR_LIMIT))
					{
						BDDNode *currentBDD =
#ifdef P1
						  pruning (functions[x], functions[j]);
#endif
#ifdef P2
						p2 (functions[x], functions[j]);
#endif
#ifdef RESTRICT
						restrictx(x, j, length, variables);
#endif
#ifdef REMOVE_FPS
						remove_fpsx(x, j, length, variables);
#endif
						if (currentBDD != functions[x])
						  {
							  //d2_printf1 ("*");
                       D_3(print_roller(););

							  ret = PREP_CHANGED;
							  SetRepeats(x);
							  functions[x] = currentBDD;
							  functionType[x] = UNSURE;
							  switch (int r=Rebuild_BDDx(x)) {
								case TRIV_SAT: 
								case TRIV_UNSAT: 
								case PREP_ERROR: ret=r; goto pr_bailout; /* As much as ... */
								default: break;
							  }
						  }
					}
				 
				 if ((functionType[j] != AND || length[j] < AND_EQU_LIMIT)
					  && (functionType[j] != OR || length[j] < OR_EQU_LIMIT)
					  && (functionType[j] != PLAINOR || length[j] < PLAINOR_LIMIT)
					  && (functionType[j] != PLAINXOR || length[j] < PLAINXOR_LIMIT))
					{
						BDDNode *currentBDD =
#ifdef P1
						  pruning (functions[j], functions[x]);
#endif
#ifdef P2
						p2 (functions[j], functions[x]);
#endif
#ifdef RESTRICT
						restrictx(j, x, length, variables);
#endif
#ifdef REMOVE_FPS
						remove_fpsx(j, x, length, variables);
#endif
						if (currentBDD != functions[j])
						  {
							  //d2_printf1 ("*");
                       D_3(print_roller(););

							  ret = PREP_CHANGED;
							  SetRepeats(j);
							  functions[j] = currentBDD;
							  functionType[j] = UNSURE;
							  switch (int r=Rebuild_BDDx(j)) {
								case TRIV_SAT: 
								case TRIV_UNSAT: 
								case PREP_ERROR: ret=r; goto pr_bailout; /* As much as ... */
								default: break;
							  }
						  }
					}
			 }
	  }
	pr_bailout:

   D_3(print_nonroller(););
	d3_printf1("\n");
   d2e_printf1("\r                                         ");
	delete [] repeat_small;
	return ret;
}
