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

int MAX_EXQUANTIFY_CLAUSES = 20;	  //Number of BDDs a variable appears in
                                   //to quantify that variable away.
int MAX_EXQUANTIFY_VARLENGTH = 5; //Limits size of number of vars in 
                                   //constraints created by ExQuantify
//!

int ExQuantifyAnd();

int countBDDs() {
	int count = 0;
	for(int x = 0; x < nmbrFunctions; x++)
	  if(functions[x]!=true_ptr) count++;
	return count;
}

int Do_ExQuantifyAnd() {
	MAX_EXQUANTIFY_CLAUSES += 5;
	MAX_EXQUANTIFY_VARLENGTH +=5;
	d3_printf1 ("ANDING AND EXISTENTIALLY QUANTIFYING - ");
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
		cofs = ExQuantifyAnd ();
		if(cofs == PREP_CHANGED) ret = PREP_CHANGED;
		else if(cofs == TRIV_UNSAT) {
			return TRIV_UNSAT;
		}
	}
	d3_printf1 ("\n");
	d2e_printf1 ("\r                                      ");
	return ret;
}

int ExQuantifyAnd () {
	int ret = PREP_NO_CHANGE;
	
	BDDNode *Quantify;
	
	if (enable_gc) bdd_gc(); //Hit it!
	
	for (int x = 1; x <= MAX_EXQUANTIFY_CLAUSES; x++) {
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
				d3_printf1("Breaking out of Anding Existential Quantification\n");
				ret = PREP_NO_CHANGE;
				nCtrlC = 0;
				goto ea_bailout;
			}
			
			if (i % ((numinp/100)+1) == 0) {
				d2e_printf3("\rPreprocessing Ea %d/%ld ", i, numinp);
			}

			if(variablelist[i].true_false != -1 || variablelist[i].equalvars != 0)
			  continue;
			//fprintf(stderr, "%d\n", i);
			
			int amount_count = 0;
			for (llist * k = amount[i].head; k != NULL; k = k->next)
			  amount_count++;

			if(amount_count == 0) {
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
			
			if (amount_count <= x) {
				int j = amount[i].head->num;
				Quantify = functions[j];
				int out = 0;
				if(length[j]>MAX_EXQUANTIFY_VARLENGTH) continue;
				DO_INFERENCES = 0;
				int count1 = 0;

				for (llist * k = amount[i].head->next; k != NULL;) {
					int z = k->num;
					count1++;
					k = k->next;
					D_3(
						 for(int iter = 0; iter<str_length; iter++)
						   d3_printf1("\b");
						 sprintf(p, "(%d:%d/%d[%d])",i, count1, amount_count, countBDDs());
						 str_length = strlen(p);
						 d3_printf1(p);
					);
					if (nCtrlC) {
						out = 1;
						break;
					}

					if(length[z] > MAX_EXQUANTIFY_VARLENGTH){
						out = 1;
						break;
					}

					Quantify = ite_and(Quantify, functions[z]);
					affected++;
					
					functions[z] = true_ptr;
					switch (int r=Rebuild_BDDx(z)) {
					 case TRIV_UNSAT: 
					 case TRIV_SAT: 
					 case PREP_ERROR: 
						ret = r; goto ea_bailout; /* as much as I hate gotos */
					 default: break;
					}
					UnSetRepeats(z);
					equalityVble[z] = 0;
					functionType[z] = UNSURE;
					
					ret = PREP_CHANGED;

					int bdd_length = 0;
					int *bdd_vars = NULL;
					switch (int r=Rebuild_BDD(Quantify, &bdd_length, bdd_vars)) {
					 case TRIV_UNSAT:
					 case TRIV_SAT:
					 case PREP_ERROR: return r;
					 default: break;
					}
					delete [] bdd_vars;
					bdd_vars = NULL;
					if(bdd_length>MAX_EXQUANTIFY_VARLENGTH) {
						out = 1;
						break;
					}
				}
				if(amount_count != 1) {
					D_3(
						 for(int iter = 0; iter<str_length; iter++)
						   d3_printf1("\b");
						 sprintf(p, "(%d:%d/%d[%d])",i, count1, amount_count, countBDDs());
						 str_length = strlen(p);
						 d3_printf1(p);
					);
				}
				if(ret != PREP_CHANGED && x!=1) {
					DO_INFERENCES = 1;
					continue;
				}
				if(out) {
					if(functions[j] == Quantify) { 
						DO_INFERENCES = 1;
						continue;
					}
					functions[j] = Quantify;
					switch (int r=Rebuild_BDDx(j)) {
					 case TRIV_UNSAT: 
					 case TRIV_SAT: 
					 case PREP_ERROR: 
						ret = r; goto ea_bailout; /* as much as I hate gotos */
					 default: break;
					}
					ret = PREP_CHANGED;
				} else {
					if(functions[j] != Quantify) {
						functions[j] = Quantify;
						switch (int r=Rebuild_BDDx(j)) {
						 case TRIV_UNSAT: 
						 case TRIV_SAT: 
						 case PREP_ERROR: 
							ret = r; goto ea_bailout; /* as much as I hate gotos */
						 default: break;
						}
					} else if(Ea_repeat[j] == 0) continue;
					Ea_repeat[j] = 0;
					
					if(ex_infer == 1) {
						//Check for direct inferences.
						DO_INFERENCES = 1;
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
								//If variable occurs in only this BDD.
								if(amount[variables[j].num[v]].head->next == NULL) {
									infer *x_infers = NULL;
									x_infers = possible_infer_x(functions[j], variables[j].num[v]);
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
										ret = PREP_CHANGED;
										changed = 1;										
										break;
									}
								}
							}
						}
						
						changed = 1;
						while(changed == 1) {
							changed = 0;
							Quantify = functions[j];
							for(int v = 0; v < length[j]-1; v++) {
								//If variable occurs in only this BDD.
								if(amount[variables[j].num[v]].head->next == NULL) {
									Quantify = xquantify (Quantify, variables[j].num[v]);
									int bdd_length = 0;
									int *bdd_vars = NULL;
									switch (int r=Rebuild_BDD(Quantify, &bdd_length, bdd_vars)) {
									 case TRIV_UNSAT:
									 case TRIV_SAT:
									 case PREP_ERROR: return r;
									 default: break;
									}
									
									int j_length = length[j];
									if(inferlist->next != NULL) {
										switch (int r=Do_Apply_Inferences()) {
										 case TRIV_UNSAT:
										 case TRIV_SAT:
										 case PREP_ERROR: return r;
										 default: break;
										}
									}
									if(length[j]!=j_length) { 
										//functions[j] changed, restart the 'for' loop
										ret = PREP_CHANGED;
										changed = 1;
										break;
									}

									int y = v+1; int z = 0;
									while (z < bdd_length) {
										if (variables[j].num[y] < bdd_vars[z]) {
											if(amount[variables[j].num[y]].head->next == NULL) {
												//variables[j].num[y] dropped out.
												BDDNode *inferBDD = ite_var(variables[j].num[y]);
												int tmp_length = 0;
												int *tmp_vars = NULL;
												switch (int r=Rebuild_BDD(inferBDD, &tmp_length, tmp_vars)) {
												 case TRIV_UNSAT:
												 case TRIV_SAT:
												 case PREP_ERROR: return r;
												 default: break;
												}
												delete [] tmp_vars;
												tmp_vars = NULL;
												ret = PREP_CHANGED;
												changed = 1;
											}
											y++;
										} else if (variables[j].num[y] > bdd_vars[z]) {
											z++;
										} else if (variables[j].num[y] == bdd_vars[z]) {
											if(amount[variables[j].num[y]].head->next == NULL && changed == 0) {
												//Try to find a possible inference for this variable
												infer *x_infers = NULL;
												x_infers = possible_infer_x(Quantify, variables[j].num[y]);
												assert(x_infers!=NULL);
												if(x_infers==NULL) fprintf(stderr, "OHNO!\n");
												
												if(x_infers->nums[0] != 0) {
													BDDNode *inferBDD = true_ptr;
													if(x_infers->nums[1] == 0)
													  inferBDD = ite_and(inferBDD, ite_var(x_infers->nums[0]));
													else
													  inferBDD = ite_and(inferBDD, ite_equ(ite_var(x_infers->nums[0]), ite_var(x_infers->nums[1])));
													while(x_infers!=NULL) {
														infer *temp = x_infers; x_infers = x_infers->next; delete temp;
													}
													
													int tmp_length = 0;
													int *tmp_vars = NULL;
													switch (int r=Rebuild_BDD(inferBDD, &tmp_length, tmp_vars)) {
													 case TRIV_UNSAT:
													 case TRIV_SAT:
													 case PREP_ERROR: return r;
													 default: break;
													}
													delete [] tmp_vars;
													tmp_vars = NULL;
													switch (int r=Do_Apply_Inferences()) {
													 case TRIV_UNSAT:
													 case TRIV_SAT:
													 case PREP_ERROR: return r;
													 default: break;
													}
													changed = 1;
													ret = PREP_CHANGED;
													y = length[j];
													break;
												}
											}
											y++; z++;
										}
									}
									
									while(y < length[j]) {
										if(amount[variables[j].num[y]].head->next == NULL) {
											//variables[j].num[y] dropped out.
											BDDNode *inferBDD = ite_var(variables[j].num[y]);
											int tmp_length = 0;
											int *tmp_vars = NULL;
											switch (int r=Rebuild_BDD(inferBDD, &tmp_length, tmp_vars)) {
											 case TRIV_UNSAT:
											 case TRIV_SAT:
											 case PREP_ERROR: return r;
											 default: break;
											}
											delete [] tmp_vars;
											tmp_vars = NULL;
											ret = PREP_CHANGED;
											changed = 1;
										}
										y++;
									}

									delete [] bdd_vars;
									bdd_vars = NULL;

									if(changed == 1) {
										switch (int r=Do_Apply_Inferences()) {
										 case TRIV_UNSAT:
										 case TRIV_SAT:
										 case PREP_ERROR: return r;
										 default: break;
										}
										break;
									}
								}
							}
						}
					} else {
						for(int iter = 0; iter<str_length; iter++)
						  d3_printf1("\b");
						d3_printf2 ("*{%d}", i);
						str_length = 0;// strlen(p);
						functions[j] = xquantify (functions[j], i);
						variablelist[i].true_false = 2;
						SetRepeats(j);
				
						DO_INFERENCES = 1;
						
						switch (int r=Rebuild_BDDx(j)) {
						 case TRIV_UNSAT:
						 case TRIV_SAT: 
						 case PREP_ERROR: 
							ret = r; goto ea_bailout; /* as much as I hate gotos */
						 default: break;
						}
						equalityVble[j] = 0;
						functionType[j] = UNSURE;
						ret = PREP_CHANGED;
						goto ea_bailout; /* as much as I hate gotos */
					}
				}
			}
		}
	}
	ea_bailout:

	for(int x = 0; x < nmbrFunctions; x++)
	  Ea_repeat[x] = 1;
	return ret;
}
