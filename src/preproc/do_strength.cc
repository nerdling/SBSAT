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
/*********************************************************
 *  preprocess.c (S. Weaver)
 *********************************************************/

#include "ite.h"
#include "preprocess.h"

int STRENGTH = 2;		//The number of vars that must be in common to
                     //Strengthen or BranchPrune two BDD's against
                     //eachother.
                     //The lower the number the stronger.

int Do_Strength() {
	int ret = PREP_NO_CHANGE;
	
	int *repeat_small = new int[nmbrFunctions + 1];

	for (int x = 0; x < nmbrFunctions; x++)
	  repeat_small[x] = St_repeat[x];

//   D_3(print_nonroller();)
	d3_printf1("STRENGTHENING - ");
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
				if (x % 100 == 0) {
					for(int iter = 0; iter<str_length; iter++)
					  d3_printf1("\b");
					sprintf(p, "{%ld:%d/%d}", affected, x, nmbrFunctions);
					str_length = strlen(p);
					d3_printf1(p);
				}
		  );
		  if (x % 100 == 0) {
			  if (nCtrlC) {
				  d3_printf1("\nBreaking out of Strengthening");
				  for(; x < nmbrFunctions; x++) St_repeat[x] = 0;
				  nCtrlC = 0;
				  break;
			  }
           d2e_printf3("\rPreprocessing St %d/%d", x, nmbrFunctions);
		  }
		  
		  St_repeat[x] = 0;
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
             if (variables[x].num[0] >= variables[j].num[length[j]-1])
                continue;
             if (variables[j].num[0] >= variables[x].num[length[x]-1]) 
                continue;
				 if (nmbrVarsInCommon (x, j, length, variables, STRENGTH) == 0) // < STRENGTH)
					continue;
				 
				 if ((functionType[x] != AND || length[x] < AND_EQU_LIMIT) 
					  && (functionType[x] != OR || length[x] < OR_EQU_LIMIT)
					  && (functionType[x] != PLAINOR || length[x] < PLAINOR_LIMIT)
					  && (functionType[x] != PLAINXOR || length[x] < PLAINXOR_LIMIT))
					{
						BDDNode *currentBDD =
						  strengthen (x, j, length, variables);
						if (currentBDD != functions[x])
						  {
							  //d2_printf1("*");
//                       D_3(print_roller();)
							  affected++; 
							  ret = PREP_CHANGED;
							  SetRepeats(x);
							  functions[x] = currentBDD;
							  functionType[x] = UNSURE;
							  switch (int r=Rebuild_BDDx(x)) {
								case TRIV_SAT: 
								case TRIV_UNSAT: 
								case PREP_ERROR: ret=r; goto st_bailout; /* As much as ... */
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
						  strengthen (j, x, length, variables);
						if (currentBDD != functions[j])
						  {
							  //d2_printf1 ("*");
//                       D_3(print_roller();)
							  affected++; 
							  ret = PREP_CHANGED;
							  SetRepeats(j);
							  functions[j] = currentBDD;
							  functionType[j] = UNSURE;
							  switch (int r=Rebuild_BDDx(j)) {
								case TRIV_SAT: 
								case TRIV_UNSAT: 
								case PREP_ERROR: ret=r; goto st_bailout; /* As much as ... */
								default: break;
							  }
						  }
					}
			 }
	  }
	st_bailout:

//   D_3(print_nonroller();)
	d3_printf1 ("\n");
   d2e_printf1("\r                                         ");
	delete [] repeat_small;
	return ret;
}

//Strengthen bdd1 using bdd2
BDDNode *strengthen(int bddNmbr1, int bddNmbr2, int *&length, store *&variables)
{	
    BDDNode *quantifiedBDD2 = functions[bddNmbr2];
    int bdd1pos = 0;
    int bdd2pos = 0;
    int bdd1Var = 0, bdd2Var = 0;
    while(bdd1pos < length[bddNmbr1] && bdd2pos < length[bddNmbr2]) {
          bdd1Var = variables[bddNmbr1].num[bdd1pos];
          bdd2Var = variables[bddNmbr2].num[bdd2pos];
          if (bdd1Var < bdd2Var)
             bdd1pos++;
          else if (bdd1Var == bdd2Var) {
                 bdd1pos++;
                 bdd2pos++;
          } else {
                 quantifiedBDD2 = xquantify(quantifiedBDD2, bdd2Var);
                 bdd2pos++;
          }
    }
    for (; bdd2pos < length[bddNmbr2]; bdd2pos++)
      quantifiedBDD2 = xquantify(quantifiedBDD2, variables[bddNmbr2].num[bdd2pos]);

    if(quantifiedBDD2 == functions[bddNmbr2]) {
		 functions[bddNmbr2] = true_ptr;
		 //d2_printf2("Removing %d ", bddNmbr2);
		 Rebuild_BDDx(bddNmbr2);
    }
    return ite_and(functions[bddNmbr1], quantifiedBDD2);
}
  
