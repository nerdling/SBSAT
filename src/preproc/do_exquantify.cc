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
			if(variablelist[i].true_false != -1 || variablelist[i].equalvars != 0) {
				//if(variablelist[i].true_false == 2) verifyCircuit(i);
				continue;
			}
			
			if(num_funcs_var_occurs[i] == 0) { //This is possibly unsound
				//Variable dropped out, set it to True.
				continue;
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
			} else if(num_funcs_var_occurs[i] == 1) {
				//Only quantify away variables from unknown functions, or functions
				//who have the quantified variable as the 'head' or LHS variable of 
				//their function...a LHS variable is a 'Left Hand Side' variable.
				assert(amount[i].head != NULL);
				int j = amount[i].head->num;
				if ((length[j] < functionTypeLimits[functionType[j]]) || (i == abs(equalityVble[j])) || (functionType[j] == PLAINOR) || (functionType[j] == PLAINXOR)) {
					affected++;
					if(ex_infer == 1) {
						switch (int r=check_bdd_for_safe(j, ret)) {
						//switch (int r=check_bdd_for_safe_eq(j, ret)) {
						 case TRIV_UNSAT:
						 case TRIV_SAT:
						 case PREP_ERROR:
							ret = r; goto ex_bailout;
						 default: break;
						}
					} else {
						for(int iter = 0; iter<str_length; iter++)
						  d3_printf1("\b");
						d3e_printf2 ("*{%d}", i);
						d4_printf3("*{%s(%d)}", s_name(i), i);
						str_length = 0;// strlen(p);
						functions[j] = xquantify (functions[j], i);
						variablelist[i].true_false = 2;
						SetRepeats(j);
						equalityVble[j] = 0;
						
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
	} while (loop_again > 0);
	
ex_bailout:
	return ret;
}
