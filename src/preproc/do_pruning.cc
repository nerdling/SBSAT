/* =========FOR INTERNAL USE ONLY. NO DISTRIBUTION PLEASE ========== */

/*********************************************************************
 Copyright 1999-2007, University of Cincinnati.  All rights reserved.
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
#include "sbsat_preproc.h"

void Sort_BDDs (long *tempint_max, int **tempint);

#ifdef DO_PRUNING_FN
# error "DO_PRUNING_FN defined -- undefine it"
#endif
#ifdef P1
# define DO_PRUNING_FN Do_Pruning_1
# define PRUNE_REPEATS P1_repeat
#else
#ifdef PRUNING_PR
# define DO_PRUNING_FN Do_Pruning
# define PRUNE_REPEATS P2_repeat
#else
#ifdef RESTRICT
# define DO_PRUNING_FN Do_Pruning_Restrict
# define PRUNE_REPEATS Restct_repeat
#else
#ifdef P2
# define DO_PRUNING_FN Do_Pruning_2
# define PRUNE_REPEATS P2_repeat
#else
#ifdef STEAL
# define DO_PRUNING_FN Do_Steal
# define PRUNE_REPEATS Steal_repeat
#else
# define DO_PRUNING_FN Do_Pruning_FPS
# define REMOVE_FPS
# define PRUNE_REPEATS ReFPS_repeat
#endif
#endif
#endif
#endif
#endif

int DO_PRUNING_FN() {
	int ret = PREP_NO_CHANGE;
	
	int *repeat_small = new int[nmbrFunctions + 1];
	
	for (int x = 0; x < nmbrFunctions; x++)
	  repeat_small[x] = PRUNE_REPEATS[x];

//   D_3(print_roller_init(););
	d3_printf1 ("BRANCH PRUNING - ");
	affected = 0;
	char p[100];
	D_3(
		 sprintf(p, "{0:0/%d}", nmbrFunctions);
		 str_length = strlen(p);
		 d3_printf1(p);
	);
	for (int x = 0; x < nmbrFunctions; x++)
	  {
		  D_3(
				if (x % ((nmbrFunctions/100)+1) == 0) {
					for(int iter = 0; iter<str_length; iter++)
					  d3_printf1("\b");
					sprintf(p, "{%ld:%d/%d}", affected, x, nmbrFunctions);
					str_length = strlen(p);
					d3_printf1(p);
				}
		  );
		  if (nCtrlC) {
			  d3_printf1("\nBreaking out of Branch Pruning");
			  for(; x < nmbrFunctions; x++) PRUNE_REPEATS[x] = 0;;
			  nCtrlC = 0;
			  break;
		  }
		  if (x % ((nmbrFunctions/100)+1) == 0) {
			  d2e_printf3("\rPreprocessing Pr %d/%d", x, nmbrFunctions);
		  }
		  
        PRUNE_REPEATS[x] = 0;
		  if (functions[x] == true_ptr)
			 continue;
		  for (int j = x + 1; j < nmbrFunctions; j++)
			 {
				 if (functions[j] == true_ptr)
					continue;
				 if ((!repeat_small[j] && !repeat_small[x]))
					continue;
             if (variables[x].min >= variables[j].max)
                continue;
             if (variables[j].min >= variables[x].max) 
                continue;
             if (nmbrVarsInCommon (x, j, STRENGTH) == 0)
                continue;
             if (length[x] < functionTypeLimits[functionType[x]])
             /*
				 if ((functionType[x] != AND || length[x] < AND_EQU_LIMIT)
					  && (functionType[x] != OR || length[x] < OR_EQU_LIMIT)
					  && (functionType[x] != PLAINOR || length[x] < PLAINOR_LIMIT)
					  && (functionType[x] != PLAINXOR || length[x] < PLAINXOR_LIMIT))
              */
					{

						//fprintf(stderr, "\n\n%d: ", x);
						//printBDDerr(functions[x]);
						//fprintf(stderr, "\n");
						
						BDDNode *currentBDD =
#ifdef P1
						  pruning_p1 (functions[x], functions[j]);
#endif
#ifdef P2
						pruning_p2 (functions[x], functions[j]);
#endif
#ifdef STEAL
						  steal (functions[x], functions[j]);
#endif
#ifdef PRUNING_PR
						pruning (functions[x], functions[j]);
#endif						
#ifdef RESTRICT
						restrictx(x, j);
#endif
#ifdef REMOVE_FPS
						remove_fpsx(x, j);
#endif
						if (currentBDD != functions[x])
						  {
							  //printBDDerr(currentBDD);
							  //fprintf(stderr, "\n");

							  //d2_printf1 ("*");
//                       D_3(print_roller(););
                       affected++;
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
				 
             if (length[j] < functionTypeLimits[functionType[j]]) {
					 //fprintf(stderr, "\n\n%d: ", j);
					 //printBDDerr(functions[j]);
					 //fprintf(stderr, "\n");
					 BDDNode *currentBDD =
#ifdef P1
						pruning_p1 (functions[j], functions[x]);
#endif
#ifdef P2
					 pruning_p2 (functions[j], functions[x]);
#endif
#ifdef STEAL
					 steal (functions[j], functions[x]);
#endif
#ifdef PRUNING_PR
					 pruning (functions[j], functions[x]);
#endif
#ifdef RESTRICT
					 restrictx(j, x);
#endif
#ifdef REMOVE_FPS
					 remove_fpsx(j, x);
#endif
					 if (currentBDD != functions[j]) {
						 //printBDDerr(currentBDD);
						 //fprintf(stderr, "\n");
						 //d2_printf1 ("*");
						 //                       D_3(print_roller(););
						 affected++;
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
	
	//   D_3(print_nonroller(););
	d3_printf1("\n");
   d2e_printf1("\r                                         ");
	delete [] repeat_small;
	return ret;
}
