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

int SafeAssign_Loop();

int Do_SafeAssign() {
	d3_printf2 ("SAFE ASSIGN %d - ", countBDDs());
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
		cofs = SafeAssign_Loop();
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

int SafeAssign_Loop() {
	int ret = PREP_NO_CHANGE;

	BDDNode *Quantify;
	int count1 = 0;
	
	if (enable_gc) bdd_gc(); //Hit it!
	for (int i = 1; i <= numinp; i++) {
		char p[100];
		
		if (nCtrlC) {
			d3_printf1("Breaking out of SafeAssign\n");
			ret = PREP_NO_CHANGE;
			nCtrlC = 0;
			goto sa_bailout;
		}
		
		if (i % ((numinp/100)+1) == 0) {
			d2e_printf3("\rPreprocessing Sa %d/%ld ", i, numinp);
		}

		D_3(
			 for(int iter = 0; iter<str_length; iter++)
			   d3_printf1("\b");
			 sprintf(p, "(%d:%d/%d[%d])", count1, i, numinp, countBDDs());
			 str_length = strlen(p);
			 d3_printf1(p);
		);

		if(variablelist[i].true_false !=-1 || variablelist[i].equalvars!=0)
		  continue;

		BDDNode *safeVal = safe_assign_all(functions, amount, i);
		
		if(safeVal!=false_ptr) {
			if(safeVal == true_ptr) safeVal = ite_var(i); //Either value is safe, set i=true
			count1++;
			BDDNode *inferBDD = safeVal;
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
		}
	} 

sa_bailout:;

//	for(int x = 0; x < nmbrFunctions; x++)
//	  Ea_repeat[x] = 1;

	return ret;
}
