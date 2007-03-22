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

int SafeSearchPreCluster_Loop();
void find_safe_assign(BDDNode ***phi, int *V, int *curr_assign, int level, int max_level,
							 BDDNode **curr_bdds);
int SafeSearch(int *V, int length_v);
int choice_points;

int VARS_PER_BDD_LIMIT = 0;
int BDDS_PER_VAR_LIMIT = 5;

int Do_SafeSearch() {
	d3_printf2 ("SAFE SEARCH CLUSTERING %d - ", countBDDs());
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
		cofs = SafeSearchPreCluster_Loop();
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

int rlistfunc_2 (const void *x, const void *y) {
	rand_list pp, qq;
	pp = *(const rand_list *) x;
	qq = *(const rand_list *) y;
	if (pp.prob < qq.prob)
	  return -1;
	if (pp.prob == qq.prob)
	  return 0;
	return 1;
}

//Jared Ziegler helped with this...
void nextSubset(bool *vector, int length) {
	if(vector[length-1] == 1) {
		int last_one_pos = length-1;
		int count_ones = 1;
		int found_zero = 0;
		int next_one_pos = -1;
		
		for(int x = length-2; x>=0; x--) {
			if(vector[x] == 1) {
				if(found_zero) {
					next_one_pos = x;
					break;
				}
				count_ones++;
				last_one_pos = x;
			} else if(vector[x] == 0) {
				found_zero = 1;
			}
		}
		//increment the 1 and shift all 1's down to 'next_one_pos+1'
		
		for(int x = last_one_pos; x<(count_ones+last_one_pos); x++) {
			vector[x] = 0;
		}
		
		if(next_one_pos >= 0) vector[next_one_pos] = 0;
		vector[next_one_pos+1] = 1;
		for(int x = next_one_pos+1; x<=(count_ones+next_one_pos+1); x++) {
			vector[x] = 1;
		}
	} else {
		for(int x = length-2; x>=0; x--) {
			if(vector[x] == 1) {
				vector[x] = 0;
				vector[x+1] = 1;
				break;
			}
		}
	}
}

int SafeSearchPreCluster_Loop() {
	int ret = PREP_NO_CHANGE;
	VARS_PER_BDD_LIMIT+=5;
	BDDS_PER_VAR_LIMIT+=5;

	int length_touch;
	
	rand_list *rlist = (rand_list*)ite_calloc(numinp+1, sizeof(rand_list), 9, "rlist");
	
	for(int i = 1;i < numinp+1; i++) {
		rlist[i].num = i;
		rlist[i].prob = random()%(numinp*numinp);
	}
	 
	qsort(rlist, numinp+1, sizeof(rand_list), rlistfunc_2);
	
	if(enable_gc) bdd_gc(); //Hit it!

	int *vars_touched = (int *)ite_calloc(numinp+1, sizeof(int), 9, "vars_touched for safesearch");
	int length_varst = 0;
	
	for (int x = 1; x <= BDDS_PER_VAR_LIMIT; x++) {
		for (int rnum = 1; rnum <= numinp; rnum++) {
			//for (int i = numinp; i > 0; i--) {
			int i = rlist[rnum].num;
			if(i == 0) continue;

			int q;
			for(q = 0; q < length_varst; q++)
			  if(vars_touched[q] == i) break;
			if(q == length_varst)
			  vars_touched[length_varst++]=i;
			
			
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
				d3_printf1("Breaking out of SafeSearch Clustering\n");
				ret = PREP_NO_CHANGE;
				nCtrlC = 0;
				goto ss_bailout;
			}
			
			if (i % ((numinp/100)+1) == 0) {
				d2e_printf3("\rPreprocessing Ss %d/%ld ", i, numinp);
			}
			
			
			if(variablelist[i].true_false != -1 || variablelist[i].equalvars != 0)
			  continue;
			//fprintf(stderr, "%d\n", i);
			
			if (num_funcs_var_occurs[i] == x) {
				assert(amount[i].head != NULL);
				int j = amount[i].head->num;
				int count1 = 1;
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
					
					if (nCtrlC)	break;

					if(length[z] > VARS_PER_BDD_LIMIT || length[j] > VARS_PER_BDD_LIMIT) {
						break;
					} else {
						int q;
						for(q = 0; q < length_varst; q++)
						  if(vars_touched[q] == i) break;
						if(q == length_varst)
						  vars_touched[length_varst++]=i;
					}
					
					count1++;
					
					functions[j] = ite_and(functions[j], functions[z]);
					functions[z] = true_ptr;

					switch (int r=Rebuild_BDDx(z)) {
					 case TRIV_UNSAT:
					 case TRIV_SAT:
					 case PREP_ERROR:
						ret = r; goto ss_bailout;
					 default: break;
					}
					
					UnSetRepeats(z);
					
					switch (int r=Rebuild_BDDx(j)) {
					 case TRIV_UNSAT:
					 case TRIV_SAT:
					 case PREP_ERROR:
						ret = r; goto ss_bailout; /* as much as I hate gotos */
					 default: break;
					}
										
					affected++;
					
					ret = PREP_CHANGED;
					
					k = amount[i].head; //Must do this because Rebuild_BDDx can modify amount
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
		}
	}

	//Remove any variables from V which were just inferred.

	length_touch = 0;
	
	for(int i = 0; i < length_varst; i++) {
		//Don't add variables into the V array that don't exist in any BDDs
		//fprintf(stdout, "%d, ", i);
		if(variablelist[vars_touched[i]].true_false !=-1 || variablelist[vars_touched[i]].equalvars!=0)
		  continue;
		//if(rand()%5==0) continue;
		//if(rand()%2==0 || rand()%2==0 || rand()%2) continue;
		vars_touched[length_touch++] = vars_touched[i];
	}
	//fprintf(stdout, "\n");

	if(length_touch > 0) {
		int *V = (int *)ite_calloc(length_touch, sizeof(int), 9, "V for safesearch");
		
		bool *vector = (bool*)ite_calloc(length_touch, sizeof(bool), 9, "vector");
		
		for(int x = 0; x < 1; x++)
		  vector[x] = 1;
		
		//for(int x = 0; x < 3100; x++) {
		int length_V = 0;
		int old_length_V = 0;
		int infs = 0;
		do {
			length_V = 0;
			for(int y = 0; y < length_touch; y++) {
				if(vector[y] == 1) {
					if(variablelist[vars_touched[y]].true_false !=-1 || variablelist[vars_touched[y]].equalvars!=0)
					  continue;
					V[length_V++] = vars_touched[y];
					//fprintf(stderr, "%d, ", vars_touched[y]);
				}
			}
			//fprintf(stderr, "\n");
			if(length_V == 0) break;
			int r;
			switch (r=SafeSearch(V, length_V)) {
			 case TRIV_UNSAT:
			 case TRIV_SAT:
			 case PREP_ERROR: return r;
			 default: break;
			}
			if(r == PREP_CHANGED) {
				ret = r;
				infs++;
				if(infs > 100) break;
			}
			nextSubset(vector, length_touch);
			if(old_length_V < length_V) {
				fprintf(stderr, " %d/%d ", length_V, length_touch);
				old_length_V = length_V;
			}
		} while(length_V < 3);//length_touch);
		
		ite_free((void **) &V);
		fprintf(stdout, "\n");
	}
	
	ss_bailout:
	
	ite_free((void **) &vars_touched);
	
	return ret;
}

int SafeSearch(int *V, int length_V) {
	int ret = PREP_NO_CHANGE;
	
	if (enable_gc) bdd_gc(); //Hit it!

	//fprintf(stdout, "Searching on:");
	for(int i = 0; i < length_V; i++) {
	//	fprintf(stdout, "%d, ", V[i]);
	}
	//fprintf(stdout, "\n");
	
	int max_level = length_V;
	
	if(max_level <= 0) return ret;
	
	BDDNode ***phi = (BDDNode ***)ite_calloc(max_level, sizeof(BDDNode **), 9, "phi***");
	
	phi[max_level-1] = (BDDNode **)ite_calloc(nmbrFunctions, sizeof(BDDNode *), 9, "phi[x]**");
	for(int x = 0; x < nmbrFunctions; x++) {
		phi[max_level-1][x] = functions[x];			  
	}

	//This could be made quicker if I used the 'amount' variable
	for(int i = max_level-1; i > 0; i--) {
		phi[i-1] = (BDDNode **)ite_calloc(nmbrFunctions, sizeof(BDDNode *), 9, "phi[x]**");
		for(int x = 0; x < nmbrFunctions; x++) {
			phi[i-1][x] = xquantify(phi[i][x], V[i]);
		}
		//			fprintf(stderr, "\nLevel %d: ", i);
		//			for(int x=0; x < nmbrFunctions; x++) {
		//				printBDDerr(phi[i][x]);
		//				fprintf(stderr, "\n");
		//			}
	}
	
	int *curr_assign = (int *)ite_calloc(max_level+1, sizeof(int), 9, "curr_assign");
	BDDNode **curr_bdds = (BDDNode **)ite_calloc(nmbrFunctions, sizeof(BDDNode *), 9, "curr_bdds");
	int level = 0;
	choice_points = 0;
	find_safe_assign(phi, V, curr_assign, level, max_level, curr_bdds);
	//should return an assignment in curr_assign
	//if curr_assign[0] == 0, no assignment was found
	if(curr_assign[0] != 0) {
		fprintf(stdout, "Solution Found\n");
		//Apply solution:
		BDDNode *solution = true_ptr;
		for(int x = 0; x < max_level; x++) {
			if(curr_assign[x] == 1)
			  solution = ite_and(solution, ite_var(V[x]));
			if(curr_assign[x] == -1)
			  solution = ite_and(solution, ite_var(-V[x]));
		}			
		assert(curr_assign[max_level] == 2);
		
		BDDNode *inferBDD = solution;
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
		//ret = PREP_NO_CHANGE;
		switch (int r=Do_Apply_Inferences()) {
		 case TRIV_UNSAT:
		 case TRIV_SAT:
		 case PREP_ERROR: return r;
		 default: break;
		}
	} else {
		//			fprintf(stderr, "UNSAT\n");
		//			return TRIV_UNSAT;
		ret = PREP_NO_CHANGE;
	}
	
	for(int x = max_level-1; x >= 0; x--) {
		ite_free((void **) &phi[x]);
	}
	ite_free((void **) &phi);
	ite_free((void **) &curr_assign);
	ite_free((void **) &curr_bdds);
	return ret;
}

//curr_assign must be size max_level+2
//level is inititally 1
//curr_bdds is calloc'd size nmbrFunctions
//phi is ex_quantify triangle
//max_level is numinp

void find_safe_assign(BDDNode ***phi, int *V, int *curr_assign, int level, int max_level,
							 BDDNode **curr_bdds) {
	if(level == max_level) {
		//Solution found!!!
		curr_assign[max_level] = 2;
		return;
	}

	for(int x = 0; x < nmbrFunctions; x++)
		curr_bdds[x] = phi[level][x];

//	fprintf(stderr, "\nLevel %d %d: ", level, V[level]);
//	for(int x=0; x < nmbrFunctions; x++) {
//		printBDDerr(curr_bdds[x]);
//		fprintf(stderr, "\n");
//	}
	
	//apply curr_assign to curr_bdds
	for(int x = level-1; x >= 0 ; x--) {
		if(curr_assign[x] == 1)
		  set_variable_all_bdds(amount[V[x]].head, V[x], 1, curr_bdds);
		if(curr_assign[x] == -1)
		  set_variable_all_bdds(amount[V[x]].head, V[x], 0, curr_bdds);
	}
	
	BDDNode *safeVal = safe_assign_all(curr_bdds, amount, V[level]);

//	fprintf(stderr, "\n");
//	for(int x=0; x < max_level; x++)
//	  fprintf(stderr, "%d=%d, ", V[x], curr_assign[x]);
//	fprintf(stderr, "\n");

//	for(int x=0; x < nmbrFunctions; x++) {
//		printBDDerr(curr_bdds[x]);
//		fprintf(stderr, "\n");
//	}
	
	if(safeVal == false_ptr) {
		curr_assign[level] = 0;
		return;		
	}

	//if(level<10)
	  //fprintf(stderr, "\rLevel: %d", level);

/*	
	if(choice_points++ % 10000 == 0) {
		double progress = 0.0;
		int y = 1;
		for(int x = 0; x < 16; x++) {
			y*=2;
			if(curr_assign[x] == -1)
			  progress+=(double)1/y;
		}
		fprintf(stdout, "\rChoicepoints: %d, %4.3f%% done", choice_points, progress*100.0);
	}
*/

	if(ite_and(safeVal, ite_var(V[level])) == ite_var(V[level])) {
		curr_assign[level] = 1;
		find_safe_assign(phi, V, curr_assign, level+1, max_level, curr_bdds);
		if(curr_assign[level+1] != 0) return;
	}

	if(ite_and(safeVal, ite_var(-V[level])) == ite_var(-V[level])) {
		curr_assign[level] = -1;
		find_safe_assign(phi, V, curr_assign, level+1, max_level, curr_bdds);
		if(curr_assign[level+1] != 0) return;
	}

	curr_assign[level] = 0;
	return;
}
