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
	
	void bdd_gc();
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
				nCtrlC = 0;
				goto ea_bailout;
			}
			
			if (i % ((numinp/100)+1) == 0) {
				d2e_printf3("\rPreprocessing Ea %d/%ld ", i, numinp);
			}

			if(variablelist[i].true_false == 2) continue;
			//fprintf(stderr, "%d\n", i);
			
			int amount_count = 0;
			for (llist * k = amount[i].head; k != NULL; k = k->next)
			  amount_count++;
			
			if ((amount_count <= x) && (amount_count > 0)) {
				int j = amount[i].head->num;
				Quantify = functions[j];
				int out = 0;
				if(x==1) ret = PREP_CHANGED;
				if(length[j]>MAX_EXQUANTIFY_VARLENGTH && ret!=PREP_CHANGED) continue;
				bool OLD_DO_INFERENCES = DO_INFERENCES;
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
				}
				if(ret != PREP_CHANGED) {
					DO_INFERENCES = OLD_DO_INFERENCES; 
					continue;
				}
				if(out) {
					if(functions[j] == Quantify) continue;
					functions[j] = Quantify;
				} else {
					functions[j] = Quantify;
					switch (int r=Rebuild_BDDx(j)) {
					 case TRIV_UNSAT: 
					 case TRIV_SAT: 
					 case PREP_ERROR: 
						ret = r; goto ea_bailout; /* as much as I hate gotos */
					 default: break;
					}
					
					infer *x_infers = NULL;
					if(ex_infer == 1) x_infers = possible_infer_x(functions[j], i);
					
					if(x_infers == NULL && ex_infer == 1) {
						//do nothing. Variable dropped out or was inferred during ANDing.
					} else if(ex_infer == 0 || x_infers->nums[0] == 0) {
						for(int iter = 0; iter<str_length; iter++)
						  d3_printf1("\b");
						d3_printf2 ("*{%d}", i);
						str_length = 0;// strlen(p);
						//functions[j] = xquantify (functions[j], i);
						variablelist[i].true_false = 2;
						SetRepeats(j);
						while(x_infers!=NULL) {
							infer *temp = x_infers; x_infers = x_infers->next; delete temp;
						}
					} else {
						BDDNode *inferBDD = true_ptr;
						//while (x_infers!=NULL) {
						//fprintf(stderr, "%d|%d, %d|", j, x_infers->nums[0], x_infers->nums[1]);
						if(x_infers->nums[1] == 0)
						  inferBDD = ite_and(inferBDD, ite_var(x_infers->nums[0]));
						else
						  inferBDD = ite_and(inferBDD, ite_equ(ite_var(x_infers->nums[0]), ite_var(x_infers->nums[1])));
						//infer *temp = x_infers; x_infers = x_infers->next; delete temp;
						//}
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
					}
				}
				
				DO_INFERENCES = OLD_DO_INFERENCES;
				
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
	ea_bailout:
	return ret;
}
