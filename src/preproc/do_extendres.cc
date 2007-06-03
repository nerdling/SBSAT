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

int Do_ExtendRes() {
	d3_printf1("CREATING EXTENDED RESOLVENTS - ");
	str_length = 0;

	int resolvents_max = 1000;
	BDDNode **resolvents = (BDDNode **)ite_calloc(resolvents_max, sizeof(BDDNode *), 9, "resolvents");
	int num_res = 0;
	
	//Create functions
	int res_var = numinp;
	for(int x = 1; x < numinp; x++) {
		if(amount[x].head==NULL) continue; //var does not exist in any bdds
		for(int y = x+1; y <= numinp; y++) {
			if(amount[y].head==NULL) continue; //var does not exist in any bdds
			while(getsym_i(++res_var)!=NULL);
			int new_var = i_getsym_int(res_var, SYM_VAR);
			BDDNode *resolvent = ite_equ(ite_var(new_var), ite_and(ite_var(x), ite_var(y)));
			if(num_res >= resolvents_max) {
				resolvents = (BDDNode **)ite_recalloc((void *)resolvents, resolvents_max, resolvents_max*2, sizeof(BDDNode *), 9, "resolvents");
				resolvents_max*=2;
			}
			resolvents[num_res++] = resolvent;
		}
	}

	//Added variables numinp+1 ... num_res-1
	vars_alloc(numinp+num_res);

	numinp = numinp+num_res;
	
	int *new_bdds;
	switch (int r = add_newFunctions(resolvents, num_res, &new_bdds)) {
	 case TRIV_UNSAT:
	 case TRIV_SAT:
	 case PREP_ERROR:
		return r;
	 default: break;
	}
	
	ite_free((void**)&new_bdds);
	
	numinp = getNuminp();
	
	//fprintf(stderr, "%d %d\n", original_numout, nmbrFunctions); 
	
	bdd_gc(1);

	d3_printf1("\n");
   d2e_printf1("\rPreprocessing Er                         ");
	
	int ret = PREP_CHANGED;

	return ret;
}
