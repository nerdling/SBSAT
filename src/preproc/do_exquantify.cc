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

int ExQuantify();

int Do_ExQuantify() {
   d3_printf1("EXISTENTIALLY QUANTIFYING - ");
   int cofs = PREP_CHANGED;
   int ret = PREP_NO_CHANGE;
	affected = 0;
	char p[100];
	D_3(
		 sprintf(p, "{0:0/%ld}", numinp);
		 str_length = strlen(p);
		 d3_printf1(p);
	);
	while (cofs!=PREP_NO_CHANGE) {
      cofs = ExQuantify ();
      if(cofs == PREP_CHANGED) ret = PREP_CHANGED;
      else if(cofs == TRIV_UNSAT) {
			return TRIV_UNSAT;
		}
	}
	
	d3_printf1("\n");
   d2e_printf1("\r                                         ");
	return ret;
}

int ExQuantify () {
	int ret = PREP_NO_CHANGE;
	
	int loop_again;
	do {
		loop_again = 0;
		for (int i = 1; i < numinp + 1; i++) {
			char p[100];
			D_3(
				 if (i % ((numinp/100)+1) == 0) {
					 for(int iter = 0; iter<str_length; iter++)
						d3_printf1("\b");
					 sprintf(p, "{%ld:%d/%ld}", affected, i, numinp);
					 str_length = strlen(p);
					 d3_printf1(p);
				 }
				 );
			
			if (nCtrlC) {
				d3_printf1("\nBreaking out of Existential Quantification\n");
				nCtrlC = 0;
				break;
			}
			
			if (i % ((numinp/100)+1) == 0) {
				d2e_printf3("\rPreprocessing Ex %d/%ld ", i, numinp);
			}
			if(variablelist[i].true_false != -1 || variablelist[i].equalvars != 0)
			  continue;
			
			if(amount[i].head == NULL) {
				//Variable dropped out, set it to True.
				BDDNode *inferBDD = ite_var(i);
				int bdd_length = 0;
				int *bdd_vars = NULL;
				switch (int r=Rebuild_BDD(inferBDD, &bdd_length, bdd_vars)) {
				 case TRIV_UNSAT:
				 case TRIV_SAT:
				 case PREP_ERROR: return r;
				 default: break;
				}
				delete [] bdd_vars;
				bdd_vars = NULL;
				ret = PREP_CHANGED;
				switch (int r=Do_Apply_Inferences()) {
				 case TRIV_UNSAT:
				 case TRIV_SAT:
				 case PREP_ERROR: return r;
				 default: break;
				}
				continue;
			}
			
			if(amount[i].head->next == NULL){
				int j = amount[i].head->num;
				//Only quantify away variables from unknown functions, or functions
				//who have the quantified variable as the 'head' or LHS variable of 
				//their function...a LHS variable is a 'Left Hand Side' variable.
				if ((length[j] < functionTypeLimits[functionType[j]]) || (i == abs(equalityVble[j])) || (functionType[j] == PLAINOR)) {
					affected++;
					if(ex_infer == 1) {
						//Check for direct inferences.
						switch (int r=Do_Apply_Inferences()) {
						 case TRIV_UNSAT:
						 case TRIV_SAT:
						 case PREP_ERROR: return r;
						 default: break;
						}
						int changed = 1;
						while(changed == 1) {
							changed = 0;
                     for(int v = 0; v < length[j]; v++) {
								//If variable v occurs in only this BDD.
								if(amount[variables[j].num[v]].head->next == NULL) {
									BDDNode *Quantify = functions[j];
									//Quantify out every Ex variable except variable v
									for(int y = 0; y < length[j]; y++) {
										if(v!=y && amount[variables[j].num[y]].head->next == NULL)
										  Quantify = xquantify(Quantify, variables[j].num[y]);
									}
									
									int bdd_length = 0;
									int *bdd_vars = NULL;
									switch (int r=Rebuild_BDD(Quantify, &bdd_length, bdd_vars)) {
									 case TRIV_UNSAT:
									 case TRIV_SAT:
									 case PREP_ERROR: return r;
									 default: break;
									}
									
									if(inferlist->next != NULL) {
										switch (int r=Do_Apply_Inferences()) {
										 case TRIV_UNSAT:
										 case TRIV_SAT:
										 case PREP_ERROR: return r;
										 default: break;
										}
										delete[] bdd_vars;
										changed = 1;
										break;
									}
									
									
									int y;
									for(y = 0; y < bdd_length; y++)
									  if(bdd_vars[y] == variables[j].num[v]) break;
									delete [] bdd_vars;
									bdd_vars = NULL;
									
									if(y == bdd_length || bdd_length == 0) {
										//If variable dropped out, then set it to True
										BDDNode *inferBDD = ite_var(variables[j].num[v]);
										bdd_length = 0;
										bdd_vars = NULL;
										switch (int r=Rebuild_BDD(inferBDD, &bdd_length, bdd_vars)) {
										 case TRIV_UNSAT:
										 case TRIV_SAT:
										 case PREP_ERROR: return r;
										 default: break;
										}
										delete [] bdd_vars;
										bdd_vars = NULL;
										switch (int r=Do_Apply_Inferences()) {
										 case TRIV_UNSAT:
										 case TRIV_SAT:
										 case PREP_ERROR: return r;
										 default: break;
										}
										changed = 1;
										break; //Inference applied, start over.
									}
									
									infer *x_infers = NULL;
									x_infers = possible_infer_x(Quantify, variables[j].num[v]);
									assert(x_infers!=NULL);
									if(x_infers->nums[0] != 0) {
										BDDNode *inferBDD = true_ptr;
										if(x_infers->nums[1] == 0)
										  inferBDD = ite_and(inferBDD, ite_var(x_infers->nums[0]));
										else
										  inferBDD = ite_and(inferBDD, ite_equ(ite_var(x_infers->nums[0]), ite_var(x_infers->nums[1])));
										while(x_infers!=NULL) {
											infer *temp = x_infers; x_infers = x_infers->next; delete temp;
										}
										
										int bdd_length = 0;
										int *bdd_vars = NULL;
										switch (int r=Rebuild_BDD(inferBDD, &bdd_length, bdd_vars)) {
										 case TRIV_UNSAT:
										 case TRIV_SAT:
										 case PREP_ERROR: return r;
										 default: break;
										}
										delete [] bdd_vars;
										bdd_vars = NULL;
										
										switch (int r=Do_Apply_Inferences()) {
										 case TRIV_UNSAT:
										 case TRIV_SAT:
										 case PREP_ERROR: return r;
										 default: break;
										}
										changed = 1;
										break;
									}
								}
							}
							if(changed == 1) ret = PREP_CHANGED;
						}
					} else {  //If ex-infer != 1
						if(amount[i].head->next == NULL && amount[i].head->num == j) {
							for(int iter = 0; iter<str_length; iter++)
							  d3_printf1("\b");
							d3_printf2 ("*{%d}", i);
							str_length = 0;// strlen(p);
							functions[j] = xquantify (functions[j], i);
							variablelist[i].true_false = 2;
							SetRepeats(j);
							
							//fprintf(stderr, "\n");
							//printBDDerr(functions[j]);
							//fprintf(stderr, "\n");
							ret = PREP_CHANGED;
							switch (int r=Rebuild_BDDx(j)) {
							 case TRIV_UNSAT:
							 case TRIV_SAT:
							 case PREP_ERROR: 
								ret=r;
								goto ex_bailout;
							 default: break;
							}
							loop_again = 1;
						}
					}
				}
			}
		}
	} while (loop_again > 0);
	
ex_bailout:
	return ret;
}
