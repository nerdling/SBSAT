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
				DO_INFERENCES = 1;
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
			
			//if (amount_count <= x) {
			if (amount_count == x){// && independantVars[i]==0) {
				int j = amount[i].head->num;
				Quantify = functions[j];
				int out = 0;
				//if(length[j]>MAX_EXQUANTIFY_VARLENGTH) continue;
				if(length[j]>MAX_EXQUANTIFY_VARLENGTH) out = 1;
				DO_INFERENCES = 0;
				int count1 = 0;

				for (llist * k = amount[i].head->next; k != NULL && out==0;) {
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
						ret = r; goto ea_bailout;
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
				if(ex_infer == 1 && (functions[j]!=Quantify || x==1)) {
					int needs_slimming = 1;
					functions[j] = Quantify;
					equalityVble[j] = 0;
					functionType[j] = UNSURE;
					DO_INFERENCES = 1;
					switch (int r=Rebuild_BDDx(j)) {
					 case TRIV_UNSAT: 
					 case TRIV_SAT: 
					 case PREP_ERROR: 
						ret = r; goto ea_bailout; /* as much as I hate gotos */
					 default: break;
					}
					
					int changed = 1;
					//d3_printf1("Starting...");
					while(changed == 1) {
						//d3_printf2("[%d]", j);
						//str_length = 0;
						changed = 0;
						//d3_printf1("going...");
						for(int v = 0; v < length[j]; v++) {
							//If variable v occurs in only this BDD.
							if(amount[variables[j].num[v]].head->next == NULL) {
								//d3_printf3("working on %d, %d\n", variables[j].num[v], length[j]);
								BDDNode *Quantify2 = functions[j];
								//Quantify out every Ex variable except variable v
								int bdd_length;
								int *bdd_vars = NULL;
								for(int y = 0; y < length[j]; y++) {
									if(v!=y && amount[variables[j].num[y]].head->next == NULL) {
										//d3_printf2("%d ", variables[j].num[y]);
										Quantify2 = xquantify(Quantify2, variables[j].num[y]);
									}
								}
								//d3_printf1("\n");
								//printBDD(Quantify2);
								//d3_printf1("\n");
								
								if(bdd_vars == NULL) {
									bdd_length = 0;
									switch (int r=Rebuild_BDD(Quantify2, &bdd_length, bdd_vars)) {
									 case TRIV_UNSAT:
									 case TRIV_SAT:
									 case PREP_ERROR: return r;
									 default: break;
									}
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
								
								if(y == bdd_length || bdd_length == 0) { //If variable dropped out, then set it to True
									//d3_printf1("setting true ");
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
								x_infers = possible_infer_x(Quantify2, variables[j].num[v]);
								assert(x_infers!=NULL);
								
								if(x_infers->nums[0] != 0) {
									BDDNode *inferBDD = true_ptr;
									if(x_infers->nums[1] == 0)
									  inferBDD = ite_and(inferBDD, ite_var(x_infers->nums[0]));
									else
									  inferBDD = ite_and(inferBDD, ite_equ(ite_var(x_infers->nums[0]), ite_var(x_infers->nums[1])));
                           DeallocateInferences(x_infers);
                           /*
									while(x_infers!=NULL) {
										infer *temp = x_infers; x_infers = x_infers->next; delete temp;
									}
                           */
									
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
								} else if (x_infers != NULL) {
                           DeallocateInferences(x_infers);
                        }
							}
						}
						if(changed == 1) {
							ret = PREP_CHANGED;
							SetRepeats(j);
							needs_slimming = 0;
						}
						if(out == 0) needs_slimming = 0;
					}
					if(needs_slimming == 1) {// && length[j]>30) {
						//Shouldn't be just i that can be quantified away, 
						//Should be some variable in this BDD that can be
						//Quantified away, i is not always even a choice!!!
						for(int v = 0; v < length[j] && needs_slimming; v++) {
							if(amount[variables[j].num[v]].head->next == NULL &&
								independantVars[variables[j].num[v]] == 2) {
								for(int iter = 0; iter<str_length; iter++)
								  d3_printf1("\b");
								d3e_printf2 ("*{%d}", variables[j].num[v]);
								d4_printf3 ("*{%s(%d)}", s_name(variables[j].num[v]), variables[j].num[v]);
								str_length = 0;// strlen(p);
								functions[j] = xquantify (functions[j], variables[j].num[v]);
								variablelist[variables[j].num[v]].true_false = 2;
								SetRepeats(j);
								
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
								needs_slimming = 0;
							}
						}
						if(needs_slimming) {
							int quant_var = 0;
							for(int v = 0; v < length[j]; v++) {
								if(amount[variables[j].num[v]].head->next == NULL) {
									quant_var = variables[j].num[v];
									break;
								}
							}
							if(quant_var != 0) {
								for(int iter = 0; iter<str_length; iter++)
								  d3_printf1("\b");
								d3e_printf2 ("*{%d}", quant_var);
								d4_printf3 ("*{%s(%d)}", s_name(quant_var), quant_var);
								str_length = 0;// strlen(p);
								functions[j] = xquantify (functions[j], quant_var);
								variablelist[quant_var].true_false = 2;
								SetRepeats(j);
								
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
								//goto ea_bailout; /* as much as I hate gotos */									  
							}
						}
					}
				} else if(functions[j] == Quantify && x!=1) {
					DO_INFERENCES = 1;
/*					switch (int r=Do_Apply_Inferences()) {
					 case TRIV_UNSAT:
					 case TRIV_SAT:
					 case PREP_ERROR: return r;
					 default: break;
					}
*/
					continue;
				} else if(out == 1){
					if(functions[j]!=Quantify) {
						functions[j] = Quantify;
						equalityVble[j] = 0;
						functionType[j] = UNSURE;
						switch (int r=Rebuild_BDDx(j)) {
						 case TRIV_UNSAT: 
						 case TRIV_SAT: 
						 case PREP_ERROR: 
							ret = r; goto ea_bailout; /* as much as I hate gotos */
						 default: break;
						}
						ret = PREP_CHANGED;
					}
					DO_INFERENCES = 1;
					continue;
				} else if(ex_infer == 0 && amount[i].head->next==NULL && amount[i].head->num == j
							 && inferlist->next==NULL) {
					//Triple Extra Protection!
					functions[j] = Quantify;
					for(int iter = 0; iter<str_length; iter++)
					  d3_printf1("\b");
					d3e_printf2 ("*{%d}", i);
					d4_printf3 ("*{%s(%d)}", s_name(i), i);
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
					continue;
					//goto ea_bailout; /* as much as I hate gotos */
				} else {
					functions[j] = Quantify;
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
					continue;
				}
			}
		}
	}
	ea_bailout:
	
	for(int x = 0; x < nmbrFunctions; x++)
	  Ea_repeat[x] = 1;
	return ret;
}
