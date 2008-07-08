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

extern int MAX_EXQUANTIFY_CLAUSES;
extern int MAX_EXQUANTIFY_VARLENGTH;
extern int num_safe_assigns;

int ExSafeCluster();
int check_bdd_for_safe(int x, int ret);
int check_bdd_for_safe_eq(int x, int ret);

int Do_ExSafeCluster() {
	MAX_EXQUANTIFY_CLAUSES += 5;
	MAX_EXQUANTIFY_VARLENGTH += cluster_step_increase;
	d3_printf2 ("EX-SAFE CLUSTERING %d - ", countBDDs());
	str_length = 0;
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

	if(countBDDs() == 0) return TRIV_SAT;
	return ret;
}

typedef struct rand_list {
	int num;
	int size;
	int prob;
};

int rlistfunc1 (const void *x, const void *y) {
	rand_list pp, qq;
	pp = *(const rand_list *) x;
	qq = *(const rand_list *) y;
	if (pp.size < qq.size)
	  return -1;
	if (pp.size == qq.size) {
		if(pp.prob < qq.prob)
		  return -1;
		if(pp.prob > qq.prob)
		  return 1;		  
		return 0;
	}
	return 1;
}

int ExSafeCluster () {
	int ret = PREP_NO_CHANGE;
	
	rand_list *rlist = (rand_list*)ite_calloc(numinp+1, sizeof(rand_list), 9, "rlist");
	
	for(int i = 1;i < numinp+1; i++) {
		rlist[i].num = i;
		rlist[i].size = num_funcs_var_occurs[i];
		rlist[i].prob = random()%(numinp*numinp);
	}
	
	qsort(rlist, numinp+1, sizeof(rand_list), rlistfunc1);

	if (enable_gc) bdd_gc(); //Hit it!
//	for (int x = 1; x <= MAX_EXQUANTIFY_CLAUSES; x++) {
		for (int rnum = 1; rnum <= numinp; rnum++) {
		//for (int i = numinp; i > 0; i--) {
			int i = rlist[rnum].num;
			if(i == 0) continue;
			char p[100];
			int j, count1;			
			
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
				goto es_bailout;
			}
			
			if (i % ((numinp/100)+1) == 0) {
				d2e_printf3("\rPreprocessing Es %d/%ld ", i, numinp);
			}
			
			if(variablelist[i].true_false != -1 || variablelist[i].equalvars != 0)
			  continue;

			//fprintf(stderr, "%d\n", i);

			if(num_funcs_var_occurs[i] == 0) continue;
			assert(amount[i].head != NULL);
			j = amount[i].head->num;
			
			//do safe assign stuff here.
			//switch (int r=check_bdd_for_safe(j, ret)) {
			switch (int r=check_bdd_for_safe_eq(j, ret)) {
			 case TRIV_UNSAT:
			 case TRIV_SAT:
			 case PREP_ERROR:
				ret = r; goto es_bailout;
			 default: break;
			}
			
			count1 = 1;
			for (llist * k = amount[i].head; k != NULL;) {
				int z = k->num;
				k = k->next;
				if(z == j) continue;
				D_3(
					 for(int iter = 0; iter<str_length; iter++)
					 d3_printf1("\b");
					 sprintf(p, "(%d:%d/%d[%d])",i, count1, num_funcs_var_occurs[i], countBDDs());
					 str_length = strlen(p);
					 d3_printf1(p);
					 );

				if (nCtrlC) break;
				if(length[z] > MAX_EXQUANTIFY_VARLENGTH) break;
				if(length[j] > MAX_EXQUANTIFY_VARLENGTH) break;
				
				count1++;
				
				functions[j] = ite_and(functions[j], functions[z]);
				affected++;

				functions[z] = true_ptr;
				
				ret = PREP_CHANGED;
				
				switch (int r=Rebuild_BDDx(z)) {
				 case TRIV_UNSAT: 
				 case TRIV_SAT: 
				 case PREP_ERROR: 
					ret = r; goto es_bailout;
				 default: break;
				}
				UnSetRepeats(z);
				
				switch (int r=Rebuild_BDDx(j)) {
				 case TRIV_UNSAT: 
				 case TRIV_SAT: 
				 case PREP_ERROR: 
					ret = r; goto es_bailout;
				 default: break;
				}
				
				//do safe assign stuff here.
				//switch (int r=check_bdd_for_safe(j, ret)) {
				switch (int r=check_bdd_for_safe_eq(j, ret)) {
				 case TRIV_UNSAT: 
				 case TRIV_SAT: 
				 case PREP_ERROR: 
					ret = r; goto es_bailout;
				 default: break;
				}
			}
			if(num_funcs_var_occurs[i] != 1) {
				D_3(
					 for(int iter = 0; iter<str_length; iter++)
					 d3_printf1("\b");
					 sprintf(p, "(%d:%d/%d[%d])",i, count1, num_funcs_var_occurs[i], countBDDs());
					 str_length = strlen(p);
					 d3_printf1(p);
					 );
			}
		}
	es_bailout:

	ite_free((void**)&rlist);
	
	for(int x = 0; x < nmbrFunctions; x++)
	  Es_repeat[x] = 1;

	return ret;
}

int check_bdd_for_safe_eq(int x, int ret) {
	for(int l = 0; l < length[x]; l++) {
		int h = variables[x].num[l];
		if(amount[h].head == NULL) continue;

		BDDNode *safeVal = safe_assign_eq_all(functions, amount, h);
		
		if(safeVal!=true_ptr) {
			//fprintf(stderr, "{%d = ", h);
			//printBDDerr(safeVal);
			//fprintf(stderr, "}");
			num_safe_assigns++;
			if(safeVal == false_ptr)
			  safeVal = random()%2?ite_var(h):ite_var(-h); //Either value is safe, set h=true
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
		} else if (amount[h].head->next == NULL) {
			int o = amount[h].head->num;
			for(int iter = 0; iter<str_length; iter++)
			  d3_printf1("\b");
			d3e_printf2 ("*{%d}", h);
			d4_printf3 ("*{%s(%d)}", s_name(h), h);
			str_length = 0;// strlen(p);
			functions[o] = xquantify (functions[o], h);
			variablelist[h].true_false = 2;
			switch (int r=Rebuild_BDDx(o)) {
			 case TRIV_UNSAT:
			 case TRIV_SAT: 
			 case PREP_ERROR: 
				ret = r; return ret; /* as much as I hate gotos */
			 default: break;
			}
			SetRepeats(o);
			equalityVble[o] = 0;
			functionType[o] = UNSURE;
			ret = PREP_CHANGED;
			l = 0;
		}
	}	
	return ret;
}

int check_bdd_for_safe(int x, int ret) {
	for(int l = 0; l < length[x]; l++) {
		int h = variables[x].num[l];
		if(amount[h].head == NULL) continue;

		BDDNode *safeVal = safe_assign_all(functions, amount, h);
		
		if(safeVal!=false_ptr) {
			//fprintf(stderr, "{%d = ", h);
			//printBDDerr(safeVal);
			//fprintf(stderr, "}");
			num_safe_assigns++;
			if(safeVal == true_ptr)
			  safeVal = random()%2?ite_var(h):ite_var(-h); //Either value is safe, set h=true
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
		} else if (amount[h].head->next == NULL) {
			int o = amount[h].head->num;
			for(int iter = 0; iter<str_length; iter++)
			  d3_printf1("\b");
			d3e_printf2 ("*{%d}", h);
			d4_printf3 ("*{%s(%d)}", s_name(h), h);
			str_length = 0;// strlen(p);
			functions[o] = xquantify (functions[o], h);
			variablelist[h].true_false = 2;
			switch (int r=Rebuild_BDDx(o)) {
			 case TRIV_UNSAT:
			 case TRIV_SAT: 
			 case PREP_ERROR: 
				ret = r; return ret; /* as much as I hate gotos */
			 default: break;
			}
			SetRepeats(o);
			equalityVble[o] = 0;
			functionType[o] = UNSURE;
			ret = PREP_CHANGED;
			l = 0;
		}
	}	
	return ret;
}

/*

typedef struct rand_list {
	int *num;
	int length;
	int v;
	int d;
};

int rlistfunc1 (const void *x, const void *y) {
	rand_list pp, qq;
	pp = *(const rand_list *) x;
	qq = *(const rand_list *) y;
	if (pp.d > qq.d) return -1;
	if (pp.d == qq.d) {
		if(pp.v > qq.v) return 1;
		if(pp.v == qq.v) return 0;
	}
	return 1;
}

int calculate_d_and_v(rand_list *rlist, int max_cluster_size) {
	int rlist_index = 0;
	int *var_list = (int *)ite_calloc(numinp, sizeof(int), 9, "var_list");
	int *exq_list = (int *)ite_calloc(numinp, sizeof(int), 9, "var_list");
	for(int i = 0; i < nmbrFunctions; i++) {
		if(functions[i] == true_ptr) continue;
		int funcs_in_cluster = 0;
		for(int j = i+1; (j < nmbrFunctions) && (funcs_in_cluster < max_cluster_size); j++) {
			if(functions[j] == true_ptr) continue;
			funcs_in_cluster++;
			int n_vars = 0;
			int n_exq_vars = 0;
			int x = i;
			for(; x <= j; x++) {
				if(length[x] > MAX_EXQUANTIFY_VARLENGTH) break;
				for(int y = 0; y < length[x]; y++) {
					int can_quantify_y = 1;
					for (llist * k = amount[y].head; k != NULL; k = k->next) {
						if(k->num < i || k->num > j) {
							can_quantify_y = 0;
							break;
						}
					}
					if(can_quantify_y==1) {
						int z = 0;
						for(; z < n_exq_vars; z++) if(exq_list[z] == y) break;
						if(z == n_exq_vars) exq_list[n_exq_vars++] = y;
					} else {
						int z = 0;
						for(; z < n_vars; z++) if(var_list[z] == y) break;
						if(z == n_vars) var_list[n_vars++] = y;
					}
				}
			}
			if(x == j+1) {
				rlist[rlist_index].num1 = i;
				rlist[rlist_index].num2 = j;
				rlist[rlist_index].v = n_vars;
				rlist[rlist_index++].d = n_exq_vars;
			}
		}
	}
	ite_free((void**)&var_list);
	ite_free((void**)&exq_list);
	return rlist_index;
}
 
int ExSafeCluster () {
	int ret = PREP_NO_CHANGE;

	int max_cluster_size = 20;
	rand_list *rlist = (rand_list*)ite_calloc(nmbrFunctions*max_cluster_size, sizeof(rand_list), 9, "rlist");

	int rlist_size = calculate_d_and_v(rlist, max_cluster_size);

	qsort(rlist, rlist_size-1, sizeof(rand_list), rlistfunc1);

	char *used = (char *)ite_calloc(nmbrFunctions, sizeof(char), 9, "used");
	
	if (enable_gc) bdd_gc(); //Hit it!
	for (int rnum = 0; rnum < rlist_size; rnum++) {
		int i = rlist[rnum].num1;
		int j = rlist[rnum].num2;
		int x = i;
		for(; x <= j; x++) {
			if(length[x] > MAX_EXQUANTIFY_VARLENGTH) break;
			if(used[x]) break;			  
		}
		if(x != j+1) continue;
		
		char p[100];
		D_3(
			 if (rnum % ((rlist_size/100)+1) == 0) {
				 for(int iter = 0; iter<str_length; iter++)
					d3_printf1("\b");
				 sprintf(p, "{%d %d,%d:%d/%d->%d,%d}", countBDDs(), i, j, 0, j-i, rlist[rnum].d, rlist[rnum].v);
				 str_length = strlen(p);
				 d3_printf1(p);
			 }
		);
		if (nCtrlC) {
			d3_printf1("Breaking out of Safe + Existential Quantification\n");
			ret = PREP_NO_CHANGE;
			nCtrlC = 0;
			goto es_bailout;
		}
		
		if (rnum % ((rlist_size/100)+1) == 0) {
			d2e_printf3("\rPreprocessing Es %d/%ld ", rnum, rlist_size);
		}

		//do safe assign stuff here.
		switch (int r=check_bdd_for_safe(i, ret)) {
		 case TRIV_UNSAT: 
		 case TRIV_SAT: 
		 case PREP_ERROR: 
			ret = r; goto es_bailout;
		 default: break;
		}

		for(x = i+1; x <= j; x++) {
			if (nCtrlC) break;

			//do safe assign stuff here.
			switch (int r=check_bdd_for_safe(x, ret)) {
			 case TRIV_UNSAT: 
			 case TRIV_SAT: 
			 case PREP_ERROR: 
				ret = r; goto es_bailout;
			 default: break;
			}
			
			if(length[i] > MAX_EXQUANTIFY_VARLENGTH) break;
			if(length[x] > MAX_EXQUANTIFY_VARLENGTH) break;

			functions[i] = ite_and(functions[i], functions[x]);
			affected++;

			used[x] = 1;
			functions[x] = true_ptr;
			
			ret = PREP_CHANGED;
			
			switch (int r=Rebuild_BDDx(x)) {
			 case TRIV_UNSAT: 
			 case TRIV_SAT: 
			 case PREP_ERROR: 
				ret = r; goto es_bailout;
			 default: break;
			}
			UnSetRepeats(x);
			equalityVble[x] = 0;
			functionType[x] = UNSURE;
			
			switch (int r=Rebuild_BDDx(i)) {
			 case TRIV_UNSAT: 
			 case TRIV_SAT: 
			 case PREP_ERROR: 
				ret = r; goto es_bailout;
			 default: break;
			}
			
			//do safe assign stuff here.
			switch (int r=check_bdd_for_safe(i, ret)) {
			 case TRIV_UNSAT: 
			 case TRIV_SAT: 
			 case PREP_ERROR: 
				ret = r; goto es_bailout;
			 default: break;
			}
			
			D_3(
				 for(int iter = 0; iter<str_length; iter++)
				 d3_printf1("\b");
				 sprintf(p, "{%d %d,%d:%d/%d->%d,%d}", countBDDs(), i, j, x-i, j-i, rlist[rnum].d, rlist[rnum].v);
				 str_length = strlen(p);
				 d3_printf1(p);
			);
		}
	}

	es_bailout:
	
	ite_free((void**)&rlist);
	ite_free((void**)&used);
	
	for(int x = 0; x < nmbrFunctions; x++)
	  Es_repeat[x] = 1;
	
	return ret;
}
*/
