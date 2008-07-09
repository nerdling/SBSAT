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

#include "sbsat.h"
#include "sbsat_preproc.h"

int Split_Large();

int max_bdds;
int num_bdds;
BDDNode **BDDFuncs;
char p[100];
int res_var;

int Do_Split() {
   d3_printf1("SPLITTING LARGE FUNCTIONS - ");
   int ret = PREP_NO_CHANGE;
	D_3(
		 sprintf(p, "{0:0/%d}", nmbrFunctions);
		 str_length = strlen(p);
		 d3_printf1(p);
	);
	ret = Split_Large ();	

	d3_printf1("\n");
   d2e_printf1("\r                                         ");
	return ret;
}

BDDNode *splitBDD(BDDNode *f) {
//	fprintf(stderr, "flag = %d, ", f->flag);
//	printBDDerr(f);
//	fprintf(stderr, "\n");
	
   if(IS_TRUE_FALSE(f)) return f;
	if(IS_TRUE_FALSE(f->thenCase) && IS_TRUE_FALSE(f->elseCase)) return f;
	if(f->flag == 0) return f;
	
	if(f->flag < 0) {
		//node has previously been split.
//		fprintf(stderr, "Returning 0, ");
//		printBDDerr(ite_var(-(f->flag)));
//		fprintf(stderr, "\n");
		return ite_var(-(f->flag));
	} else if(f->notCase->flag < 0) {
		//notCase has previously been split.
//		fprintf(stderr, "Returning 1, ");
//		printBDDerr(ite_var(f->notCase->flag));
//		fprintf(stderr, "\n");
		return ite_var(f->notCase->flag);
	} else if(f->flag < 10) {
		//Node not referenced enough to be interesting
		BDDNode *tmp = ite(ite_var(f->variable), splitBDD(f->thenCase), splitBDD(f->elseCase));
		tmp->flag = 0;
//		fprintf(stderr, "Returning 2, ");
//		printBDDerr(g);
//		fprintf(stderr, "\n");
		return tmp;
	}

	affected++;
	
	while(getsym_i(++res_var)!=NULL);
	int new_var = i_getsym_int(res_var, SYM_VAR);

	f->flag = -new_var;
	BDDFuncs[num_bdds] = ite_equ(ite_var(new_var), f);
	assert(BDDFuncs[num_bdds]->inferences == NULL);
	BDDFuncs[num_bdds++]->flag = 0;
	if(num_bdds >= max_bdds) {
		BDDFuncs = (BDDNode **)ite_recalloc(BDDFuncs, max_bdds, max_bdds+25, sizeof(BDDNode *), 9, "BDDFuncs");
		max_bdds += 25;
	}
//	fprintf(stderr, "Creating, ");
//	printBDDerr(BDDFuncs[num_bdds-1]);
//	fprintf(stderr, "\n");
	BDDNode *tmp = ite_var(new_var);
	tmp->flag = 0;
	
	return tmp;
	
}

//Assumes BDDFuncs has been allocated
int Split_ref_counts() {
	
	clear_all_bdd_flags();
	for(int j = 0; j < nmbrFunctions; j++) {
		if (functionType[j] == UNSURE && length[j] > do_split_max_vars) {
			//d3_printf2("\n%d: ", j);			//printBDD(functions[j]);			//d3_printf1("\n");
			
			marknodes(functions[j]);
			functions[j]->flag = 0;
//			fprintf(stderr, "Marked %d\n", j);
		}
	}
	//Nodes marked w/ rough reference counts

	int ret = PREP_CHANGED;
	res_var = numinp;
	
	while(ret == PREP_CHANGED) {
		ret = PREP_NO_CHANGE;
		for(int j = 0; j < nmbrFunctions; j++) {
			D_3(
				 if (j % 100 == 0) {
					 for(int iter = 0; iter<str_length; iter++)
						d3_printf1("\b");
					 sprintf(p, "***{%ld:%d/%d}", affected, j, nmbrFunctions);
					 str_length = strlen(p);
					 d3_printf1(p);
				 }
				 );

			if (functionType[j] == UNSURE && length[j] > do_split_max_vars) {
				//Split the BDD based on the reference counts.
//				fprintf(stderr, "Splitting %d, ", j);
//				printBDDerr(functions[j]);
//				fprintf(stderr, "\n");
				
				BDDNode *tmp = ite(ite_var(functions[j]->variable), splitBDD(functions[j]->thenCase), splitBDD(functions[j]->elseCase));
				tmp->flag = 0;
				
				if(functions[j]!=tmp) {
					functions[j] = tmp;
					ret = PREP_CHANGED;
				}
				
//				fprintf(stderr, "Finish with, ");
//				printBDDerr(functions[j]);
//				fprintf(stderr, "\n");
				
				assert(functions[j]->inferences == NULL);
				
				vars_alloc(numinp+num_bdds);
			}
		}
	}

	if(num_bdds != 0) {
		ret = PREP_CHANGED;

		bool OLD_DO_INFERENCES = DO_INFERENCES;
		DO_INFERENCES = 0;
		
		for(int j = 0; j < nmbrFunctions; j++) {
			if (functionType[j] == UNSURE && length[j] > do_split_max_vars) {
				switch (int r=Rebuild_BDDx(j)) { //This should not be causing any inferences.
				 case TRIV_UNSAT:
				 case TRIV_SAT:
				 case PREP_ERROR: 
					ret=r;
					return ret;
				 default: break;
				}
			}
		}

		DO_INFERENCES = OLD_DO_INFERENCES;
		
		int *new_bdds;
		switch (int r = add_newFunctions(BDDFuncs, num_bdds, &new_bdds)) {
		 case TRIV_UNSAT:
		 case TRIV_SAT:
		 case PREP_ERROR: 
			ret=r;
			return ret;
		 default: break;
		}
		
		ite_free((void**)&new_bdds);
		
		num_bdds = 0; //BDDFuncs is not free'd here. Can continue to use the same memory in this loop
		
		numinp = getNuminp();
	}

	return ret;
}

int nCk(int n, int k) {
	if(n == k) return 1;
	if(k == 0) return 1;
	return nCk(n-1, k) + nCk(n-1, k-1);	
}

void nCk_Sets(int n, int k, int *vars, int *whereat, int n_orig, BDDNode *bdd, int orig_bdd, int target_k) {
	if(length[orig_bdd] <= target_k) return;
	if (n==0 && k==0) {
		//printBDD(tempBDD);
		//d3_printf1("\n");
		for(int i = 0; i < (*whereat) && bdd != true_ptr; i++)
		  bdd = pruning(bdd, BDDFuncs[i]);
		if(bdd != true_ptr) {				
			BDDNode *tempBDD;
			tempBDD = pruning(functions[orig_bdd], bdd);
			if(tempBDD == functions[orig_bdd]) return;
			functions[orig_bdd] = tempBDD;
			Rebuild_BDDx(orig_bdd);
			//d3_printf2("whereat = %d: \n", (*whereat));
			BDDFuncs[(*whereat)] = bdd;
			(*whereat)++;
			if((*whereat) >= max_bdds) {
				BDDFuncs = (BDDNode **)ite_recalloc(BDDFuncs, max_bdds, max_bdds+25, sizeof(BDDNode *), 9, "BDDFuncs");
				max_bdds += 25;
				num_bdds = (*whereat)-1;
			}
		}
	} else {
		if (k>0) { 
			nCk_Sets(n-1,k-1, vars, whereat, n_orig, bdd, orig_bdd, target_k);
		}
		if (n>k) {
			BDDNode *bdd_0 = xquantify(bdd, vars[n_orig-n]);
			if(bdd_0 == true_ptr) return;
			nCk_Sets(n-1,k, vars, whereat, n_orig, bdd_0, orig_bdd, target_k);
		}
	}
}

BDDNode *level_x_decomp(BDDNode *f, int level) {
	if(f == true_ptr) return f;
	if(f->inferences != NULL) return Build_BDD_From_Inferences(f);
	//if(level == 0)	return Build_BDD_From_Inferences(f);
	int v = f->variable;
	BDDNode *r = level_x_decomp(f->thenCase, level-1);
	BDDNode *e = level_x_decomp(f->elseCase, level-1);
	if(r == e) return r;
	return ite_xvar_y_z(ite_var(v), r, e);
}

int Split_Large () {
	int ret = PREP_NO_CHANGE;

	max_bdds = 0;
	num_bdds = 0;

	int k_size = do_split_max_vars;

	BDDFuncs = (BDDNode **)ite_recalloc(NULL, max_bdds, max_bdds+10, sizeof(BDDNode *), 9, "BDDFuncs");
	max_bdds += 10;

	if(0) { //SEAN
		switch (int r = Split_ref_counts()) {
		 case TRIV_UNSAT:
		 case TRIV_SAT:
		 case PREP_ERROR: 
			ret=r;
			return r;
		 default: break;
		}
	}
	
	affected = 0;
	int old_nmbrFunctions = nmbrFunctions;
	
	for(int j = 0; j < old_nmbrFunctions; j++) {
		D_3(
			 if (j % 100 == 0) {
				 for(int iter = 0; iter<str_length; iter++)
					d3_printf1("\b");
				 sprintf(p, "{%ld:%d/%d}", affected, j, old_nmbrFunctions);
				 str_length = strlen(p);
				 d3_printf1(p);
			 }
			 );
		if (j % 100 == 0) {
			if (nCtrlC) {
 				d3_printf1("\nBreaking out of Splitting");
				//for(; j < nmbrFunctions; j++) SPLIT_REPEATS[x] = 0; ??
				nCtrlC = 0;
				break;
			}
			d2e_printf3("\rPreprocessing Sp %d/%d", j, old_nmbrFunctions);
		}
		if (functionType[j] == UNSURE && length[j] > k_size) {
			bool OLD_DO_INFERENCES = DO_INFERENCES;
			DO_INFERENCES = 0;
			
			//d3_printf2("\n%d: ", j);
			//printBDD(functions[j]);
			//d3_printf1("\n");
			
			//Maximum Split Size:
			//int num_splits = nCk(length[j], k_size);
			//d3_printf4("%d C %d = %d\n", length[j], k_size, num_splits);
			
			int whereat = 0;
			if(findandset_fnType(j) == 1)
			  continue;
			
			//d3_printf2("false paths:%d\n", countFalses (functions[j]));
			int *vars_copy = new int[length[j]];
			//This is necessary because variables[j].num is modified inside nCk_Sets
			for(int i = 0; i < length[j]; i++)
			  vars_copy[i] = variables[j].num[i];
			nCk_Sets(length[j], k_size, vars_copy, &whereat, length[j], functions[j], j, k_size);
			delete [] vars_copy;
			//d3_printf2("whereat = %d: \n", whereat);
			
			//add BDDFuncs to functions;
			if(whereat > 0) {
				affected++;
				ret = PREP_CHANGED;

				int *new_bdds;
				switch (int r = add_newFunctions(BDDFuncs, whereat, &new_bdds)) {
				 case TRIV_UNSAT:
				 case TRIV_SAT:
				 case PREP_ERROR: 
					ret=r;
					goto sp_bailout;
				 default: break;
				}
				
				ite_free((void**)&new_bdds);
				
				if(length[j] > k_size) {
					BDDNode *Conjunction = BDDFuncs[0];
					for(int x = 1; x < whereat; x++)
					  Conjunction = ite_and(Conjunction, BDDFuncs[x]);
					functions[j] = ite_or(functions[j], ite_not(Conjunction));
				}

				switch (int r=Rebuild_BDDx(j)) {
				 case TRIV_UNSAT:
				 case TRIV_SAT:
				 case PREP_ERROR: 
					ret=r;
					goto sp_bailout;
				 default: break;
				}
			 }
			DO_INFERENCES = OLD_DO_INFERENCES;
		}
		if (functionType[j] == UNSURE && length[j] > k_size) {
			//Function is still too large, must use alternative method (Break into CNF clauses).
			//Count the clauses
			bool OLD_DO_INFERENCES = DO_INFERENCES;
			DO_INFERENCES = 0;

			int num = countFalses (functions[j]);
			if(num == 1) functionType[j] = PLAINOR;
			else if(num > 1) {
				affected++;
				ret = PREP_CHANGED;
				intlist *list = new intlist[num];
				int listx = 0;
				int tempint_max = 0;
				int *tempint = NULL;
				findPathsToFalse (functions[j], &tempint_max, &tempint, list, &listx);
				ite_free((void**)&tempint); tempint_max = 0;
				if(listx >= max_bdds) {
					BDDFuncs = (BDDNode **)ite_recalloc(BDDFuncs, max_bdds, listx+1, sizeof(BDDNode *), 9, "BDDFuncs");
					max_bdds = listx+1;
					num_bdds = listx;
				}
				for (int i = 0; i < listx; i++) {
					BDDFuncs[i] = false_ptr;
					for(int x = list[i].length-1; x >= 0; x--)
					  BDDFuncs[i] = ite_or(BDDFuncs[i], ite_var(-list[i].num[x])); //Need to negate all literals to get CNF!
					delete [] list[i].num;
				}
				delete [] list;
				
				int *new_bdds;
				switch (int r = add_newFunctions(BDDFuncs, listx, &new_bdds)) {
				 case TRIV_UNSAT:
				 case TRIV_SAT:
				 case PREP_ERROR: 
					ret=r;
					goto sp_bailout;
				 default: break;
				}

				ite_free((void**)&new_bdds);
				
				equalityVble[j] = 0;
				functionType[j] = UNSURE;
				functions[j] = true_ptr;
				switch (int r=Rebuild_BDDx(j)) {
				 case TRIV_UNSAT:
				 case TRIV_SAT:
				 case PREP_ERROR: 
					ret=r;
					goto sp_bailout;
				 default: break;
				}
			}
			DO_INFERENCES = OLD_DO_INFERENCES;
		}
		
		//d3_printf2("\n%d: ", j);
		//printBDD(functions[j]);
		//d3_printf1("\n");
	}
	sp_bailout:;
	
	ite_free((void **)&BDDFuncs);
	return ret;
}
