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

#include "sbsat.h"
#include "sbsat_preproc.h"

extern int MAX_EXQUANTIFY_CLAUSES;
extern int MAX_EXQUANTIFY_VARLENGTH;

int ExSafeCluster();

int Do_ExSafeCluster() {
	MAX_EXQUANTIFY_CLAUSES += 5;
	MAX_EXQUANTIFY_VARLENGTH +=5;
	d3_printf2 ("EX-SAFE CLUSTERING %d - ", countBDDs());
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
		cofs = ExSafeCluster ();
		if(cofs == PREP_CHANGED) ret = PREP_CHANGED;
		else if(cofs == TRIV_UNSAT) {
			return TRIV_UNSAT;
		}
	}
	d3_printf1 ("\n");
	d2e_printf1 ("\r                                      ");

	if(ret == PREP_CHANGED && countBDDs() == 0) return TRIV_SAT;
	return ret;
}

typedef struct rand_list {
	int num;
	int prob;
};

int rlistfunc1 (const void *x, const void *y) {
	rand_list pp, qq;
	pp = *(const rand_list *) x;
	qq = *(const rand_list *) y;
	if (pp.prob < qq.prob)
	  return -1;
	if (pp.prob == qq.prob)
	  return 0;
	return 1;
}

int ExSafeCluster () {
	int ret = PREP_NO_CHANGE;
	
	BDDNode *Quantify;
	
	rand_list *rlist = (rand_list*)ite_calloc(numinp+1, sizeof(rand_list), 9, "rlist");
	
	for(int i = 1;i < numinp+1; i++) {
		rlist[i].num = i;
		rlist[i].prob = random()%(numinp*numinp);
	}
	
	qsort(rlist, numinp+1, sizeof(rand_list), rlistfunc1);

	int safe_assign_count = 0;
	
	if (enable_gc) bdd_gc(); //Hit it!
	for (int x = 1; x <= MAX_EXQUANTIFY_CLAUSES; x++) {
		for (int rnum = 1; rnum <= numinp; rnum++) {
		//for (int i = numinp; i > 0; i--) {
			int i = rlist[rnum].num;
			if(i == 0) continue;
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
			
			//if(autark_BDD[i] != -1) continue;
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
				continue;
				//Variable dropped out, set it to True.
				//d3_printf3("\n%d dropped out, %d=T\n", i, i);
				//str_length = 0;
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


			for(int n = 1; n < numinp+1; n++) {
				if(amount[n].head == NULL) continue;
				if(amount[n].head->next == NULL) {

					BDDNode *safeVal = safe_assign_all(functions, amount, n);
					
					if(safeVal!=false_ptr) {
						fprintf(stderr, "{%d = ", n);
						printBDDerr(safeVal);
						fprintf(stderr, "}");
						if(safeVal == true_ptr) safeVal = ite_var(n); //Either value is safe, set h=true
						BDDNode *inferBDD = safeVal;
						int bdd_length = 0;
						int *bdd_vars = NULL;
						switch (int r=Rebuild_BDD(inferBDD, &bdd_length, bdd_vars)) {
						 case TRIV_UNSAT:
						 case TRIV_SAT:
						 case PREP_ERROR: return r;
						 default: break;
						}
						
						switch (int r=Do_Apply_Inferences()) {
						 case TRIV_UNSAT:
						 case TRIV_SAT:
						 case PREP_ERROR: return r;
						 default: break;
						}
						
						delete [] bdd_vars;
						bdd_vars = NULL;
					} else {
						int j = amount[n].head->num;
						for(int iter = 0; iter<str_length; iter++)
						  d3_printf1("\b");
						d3e_printf2 ("*{!%d!}", n);
						d4_printf3 ("*{%s(%d)}", s_name(n), n);
						str_length = 0;// strlen(p);
						functions[j] = xquantify (functions[j], n);
						variablelist[n].true_false = 2;
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
					}
				}
			}
			
			if (1 || amount_count <= x){// && independantVars[i]==0) {
				int j = amount_num;
				assert(functionType[j]!=AUTARKY_FUNC);
				int out = 0;
				if(length[j]>MAX_EXQUANTIFY_VARLENGTH) out = 1;
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
				
					functions[j] = ite_and(functions[j], functions[z]);
					affected++;
					
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

					switch (int r=Rebuild_BDDx(j)) {
					 case TRIV_UNSAT: 
					 case TRIV_SAT: 
					 case PREP_ERROR: 
						ret = r; goto ea_bailout;
					 default: break;
					}
					UnSetRepeats(j);
					equalityVble[j] = 0;
					functionType[j] = UNSURE;
					
					//do safe assign stuff here.

					for(int l = 0; l < length[j]; l++) {
						int h = variables[j].num[l];
						if(amount[h].head == NULL) continue;
						if(amount[h].head->next == NULL) continue;
						
						BDDNode *safeVal = safe_assign_all(functions, amount, h);
						  
						if(safeVal!=false_ptr) {
							fprintf(stderr, "{%d = ", h);
							printBDDerr(safeVal);
							fprintf(stderr, "}");
							if(safeVal == true_ptr) safeVal = ite_var(h); //Either value is safe, set h=true
							BDDNode *inferBDD = safeVal;
							int bdd_length = 0;
							int *bdd_vars = NULL;
							switch (int r=Rebuild_BDD(inferBDD, &bdd_length, bdd_vars)) {
							 case TRIV_UNSAT:
							 case TRIV_SAT:
							 case PREP_ERROR: return r;
							 default: break;
							}

							switch (int r=Do_Apply_Inferences()) {
							 case TRIV_UNSAT:
							 case TRIV_SAT:
							 case PREP_ERROR: return r;
							 default: break;
							}
								
							delete [] bdd_vars;
							bdd_vars = NULL;
							l = 0;
						}
					}

					if(length[j]>MAX_EXQUANTIFY_VARLENGTH && count1!=amount_count) {
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
					continue;
				}
				
				if(out == 1){
					switch (int r=Rebuild_BDDx(j)) {
					 case TRIV_UNSAT:
					 case TRIV_SAT: 
					 case PREP_ERROR: 
						ret = r; goto ea_bailout; /* as much as I hate gotos */
					 default: break;
					}
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
					if(amount_count==1 && amount_num == j && inferlist->next==NULL) {
						//Triple Extra Protection!
						for(int iter = 0; iter<str_length; iter++)
						  d3_printf1("\b");
						d3e_printf2 ("*{%d}", i);
						d4_printf3 ("*{%s(%d)}", s_name(i), i);
						str_length = 0;// strlen(p);
						functions[j] = xquantify (functions[j], i);
						variablelist[i].true_false = 2;
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
						continue;
						//goto ea_bailout; /* as much as I hate gotos */
					} else {
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

	ite_free((void**)&rlist);
	
	for(int x = 0; x < nmbrFunctions; x++)
	  Ea_repeat[x] = 1;

	return ret;
}

