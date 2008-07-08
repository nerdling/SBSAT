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

int MAX_EXQUANTIFY_CLAUSES = 5000;
int MAX_EXQUANTIFY_VARLENGTH = 0;

int ExQuantifyAnd();

int Do_ExQuantifyAnd() {
	MAX_EXQUANTIFY_CLAUSES += 5;
	MAX_EXQUANTIFY_VARLENGTH +=cluster_step_increase;
	d3_printf2 ("EXQUANTIFY AND %d - ", countBDDs());
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

	if(countBDDs() == 0) return TRIV_SAT;
	return ret;
}

typedef struct rand_list {
	int num;
	int size;
	int prob;
};

int rlistfunc (const void *x, const void *y) {
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

int ExQuantifyAnd () {
	int ret = PREP_NO_CHANGE;
	
	rand_list *rlist = (rand_list*)ite_calloc(numinp+1, sizeof(rand_list), 9, "rlist");
	
	int amount_count;

	for(int i = 1;i < numinp+1; i++) {
		rlist[i].num = i;

		rlist[i].size = num_funcs_var_occurs[i];
		
		rlist[i].prob = random()%(numinp*numinp);
	}
	
	qsort(rlist, numinp+1, sizeof(rand_list), rlistfunc);

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
				goto ea_bailout;
			}
			
			if (i % ((numinp/100)+1) == 0) {
				d2e_printf3("\rPreprocessing Ea %d/%ld ", i, numinp);
			}
			
			if(variablelist[i].true_false != -1 || variablelist[i].equalvars != 0)
			  continue;
			//fprintf(stderr, "%d\n", i);
			
			if(num_funcs_var_occurs[i] == 0) {
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

			assert(amount[i].head != NULL);
			j = amount[i].head->num;
			count1 = 1;
			for (llist * k = amount[i].head; k != NULL;) {
				int z = k->num;
				k = k->next;
				if(z == j) continue;
				D_3(
					 for(int iter = 0; iter<str_length; iter++)
					 d3_printf1("\b");
					 sprintf(p, "(%d:%d/%d[%d])",i, count1, amount_count, countBDDs());
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
					ret = r; goto ea_bailout;
				 default: break;
				}
				UnSetRepeats(z);
				
				switch (int r=Rebuild_BDDx(j)) {
				 case TRIV_UNSAT: 
				 case TRIV_SAT: 
				 case PREP_ERROR: 
					ret = r; goto ea_bailout;
				 default: break;
				}
				
				for(int l = 0; l < length[j]; l++) {
					int h = variables[j].num[l];
					if (num_funcs_var_occurs[h] == 1) {
						for(int iter = 0; iter<str_length; iter++)
						  d3_printf1("\b");
						d3e_printf2 ("*{%d}", h);
						d4_printf3 ("*{%s(%d)}", s_name(h), h);
						str_length = 0;// strlen(p);
						functions[j] = xquantify (functions[j], h);
						variablelist[h].true_false = 2;
						switch (int r=Rebuild_BDDx(j)) {
						 case TRIV_UNSAT:
						 case TRIV_SAT: 
						 case PREP_ERROR: 
							ret = r; goto ea_bailout; /* as much as I hate gotos */
						 default: break;
						}
						SetRepeats(j);
						equalityVble[j] = 0;
						functionType[j] = UNSURE;
						l = 0;
					}
				}
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
			if(num_funcs_var_occurs[i] == 1) {
				assert(amount[i].head != NULL);
				j = amount[i].head->num;
				
				for(int iter = 0; iter<str_length; iter++)
				  d3_printf1("\b");
				d3e_printf2 ("*{%d}", i);
				d4_printf3 ("*{%s(%d)}", s_name(i), i);
				str_length = 0;// strlen(p);
				functions[j] = xquantify (functions[j], i);
				variablelist[i].true_false = 2;
				
				switch (int r=Rebuild_BDDx(j)) {
				 case TRIV_UNSAT:
				 case TRIV_SAT: 
				 case PREP_ERROR: 
					ret = r; goto ea_bailout;
				 default: break;
				}
				ret = PREP_CHANGED;
			}
		}
	ea_bailout:

	ite_free((void**)&rlist);
	
	for(int x = 0; x < nmbrFunctions; x++)
	  Ea_repeat[x] = 1;

	return ret;
}
