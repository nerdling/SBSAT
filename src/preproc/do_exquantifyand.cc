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

			if(autark_BDD[i] != -1) continue;
			if(variablelist[i].true_false != -1 || variablelist[i].equalvars != 0)
			  continue;
			//fprintf(stderr, "%d\n", i);
         int amount_count = 0;
			int amount_num = -1;
			
			for (llist * k = amount[i].head; k != NULL; k = k->next) {
				if(functionType[k->num] != AUTARKY_FUNC) {
					amount_count++;
					if(amount_count == 1) amount_num = k->num;
				}
			}

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
			
			if (amount_count == x){// && independantVars[i]==0) {
				int j = amount_num;
				Quantify = functions[j];
				int out = 0;
				if(length[j]>MAX_EXQUANTIFY_VARLENGTH) out = 1;
				DO_INFERENCES = 0;
				int count1 = 1;
				for (llist * k = amount[i].head; k != NULL && out==0;) {
					int z = k->num;
					k = k->next;
					if(z == j) continue;
					if(functionType[z] == AUTARKY_FUNC) continue;
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

					count1++;
					
					Quantify = ite_and(Quantify, functions[z]);
					affected++;

					/*if(functions[z] == true_ptr) {
						fprintf(stderr, "z == true!!!");
						exit(0);
					}*/

					functions[z] = true_ptr;
					
					ret = PREP_CHANGED;
					
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
					if(bdd_length>MAX_EXQUANTIFY_VARLENGTH && count1!=amount_count) {
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
				if(ex_infer == 1 && (Quantify!=functions[j] || x==1)) {
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
							amount_count = 0;
							for (llist * k = amount[variables[j].num[v]].head; k != NULL; k = k->next) {
								if(functionType[k->num] != AUTARKY_FUNC) {
									amount_count++;
									if(amount_count > 1) break;
								}
							}
							if(amount_count == 1) {
								//d3_printf3("working on %d, %d\n", variables[j].num[v], length[j]);
								BDDNode *Quantify2 = functions[j];
								//Quantify out every Ex variable except variable v
								for(int y = 0; y < length[j]; y++) {
									amount_count = 0;
									for (llist * k = amount[variables[j].num[y]].head; k != NULL; k = k->next) {
										if(functionType[k->num] != AUTARKY_FUNC) {
											amount_count++;
											if(amount_count > 1) break;
										}
									}
									if(v!=y && amount[variables[j].num[y]].head->next == NULL) {
										//d3_printf2("%d ", variables[j].num[y]);
										Quantify2 = xquantify(Quantify2, variables[j].num[y]);
									}
								}
								//d3_printf1("\n");
								//printBDDerr(Quantify2);
								//d3_printf1("\n");
								int quant_var = variables[j].num[v];
								//If the autarky function gives var[j].num[v] as an inference
								//Then var[j].num[v] will be pointing to a different variable
								//so quant_var is necessary.
								BDDNode **BDDFuncs;
								BDDFuncs = (BDDNode **)ite_recalloc(NULL, 0, 1, sizeof(BDDNode *), 9, "BDDFuncs");
								BDDFuncs[0] = strip_x_BDD(Quantify2, quant_var);

								autark_BDD[quant_var] = 1;
								
								switch (int r=add_newFunctions(BDDFuncs, 1)) {
								 case TRIV_UNSAT:
								 case TRIV_SAT:
								 case PREP_ERROR: return r;
								 default: break;
								}
								ite_free((void **)&BDDFuncs);
								functionType[nmbrFunctions-1] = AUTARKY_FUNC;
								equalityVble[nmbrFunctions-1] = quant_var;
								switch (int r=Rebuild_BDDx(nmbrFunctions-1)) {
								 case TRIV_UNSAT:
								 case TRIV_SAT:
								 case PREP_ERROR: return r;
								 default: break;
								}
								//Check whether quant_var was applied as an inference.
								amount_count = 0;
								amount_num = -1;
								for (llist * k = amount[quant_var].head; k != NULL; k = k->next) {
									if(functionType[k->num] != AUTARKY_FUNC) {
										amount_count++;
										amount_num = k->num;
									}
									if(amount_count > 1) break;
								}
								if(amount_count == 1 && amount_num == j) {
									functions[j] = xquantify (functions[j], quant_var);
									for(int iter = 0; iter<str_length; iter++)
									  d3_printf1("\b");
									d3e_printf2 ("*{%d}", quant_var);
									str_length = 0;
									variablelist[quant_var].true_false = 2;
									ret = PREP_CHANGED;
								}
								switch (int r=Rebuild_BDDx(j)) {
								 case TRIV_UNSAT:
								 case TRIV_SAT:
								 case PREP_ERROR: return r;
								 default: break;
								}
								changed = 1;
								break;
							}
						}
						if(changed == 1) {
							SetRepeats(j);
						}
					}
				} else if(functions[j] == Quantify && x!=1) {
					DO_INFERENCES = 1;
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
					}
					DO_INFERENCES = 1;
					continue;
				} else {
					amount_count = 0;
					for (llist * k = amount[i].head; k != NULL; k = k->next) {
						if(functionType[k->num] != AUTARKY_FUNC) {
							amount_count++;
							if(amount_count == 1) amount_num = k->num;
							if(amount_count > 1) break;
						}
					}
					if(ex_infer == 0 && amount_count==1 && amount_num == j
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
						continue;
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

