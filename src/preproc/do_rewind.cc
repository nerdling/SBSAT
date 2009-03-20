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

extern int num_safe_assigns;
int tier = 1;

int Do_Rewind() {
	d3_printf1("REWINDING - ");
	str_length = 0;
	MAX_EXQUANTIFY_CLAUSES = 5000;
	MAX_EXQUANTIFY_VARLENGTH = 0;

	bool OLD_DO_INFERENCES = DO_INFERENCES;
	DO_INFERENCES = 0;

	long long memory_used = get_memusage();
	
	Pos_replace = Neg_replace = Setting_Pos = Setting_Neg = 0;

	int y = 0;
	for(int x = 0; x < nmbrFunctions; x++) {
		functions[x] = true_ptr;
		functionType[x] = UNSURE;
		equalityVble[x] = 0;
		Rebuild_BDDx(x);
	}

	delete l;
	l = new Equiv (numinp + 1, nmbrFunctions, T, F);
	
	//nmbrFunctions = 0;
	
	int *new_bdds;
	switch (int r = add_newFunctions(original_functions, original_numout, &new_bdds)) {
	 case TRIV_UNSAT:
	 case TRIV_SAT:
	 case PREP_ERROR:
		return r;
	 default: break;
	}

	ite_free((void**)&new_bdds);
	
	//fprintf(stderr, "%d %d\n", original_numout, nmbrFunctions); 
	
	CreateInferences();
	for(int x = 1; x < numinp + 1; x++) {
	  //if(variablelist[x].true_false == 2) {
		  variablelist[x].replace = x;
		  variablelist[x].equalvars = 0;
		  variablelist[x].true_false = -1;
	  //}
	}

	DO_INFERENCES = OLD_DO_INFERENCES;

	for (int x = 0; x < nmbrFunctions; x++) { //Need to do this twice
		int r=Rebuild_BDDx(x);                 //For the GaussE Table
		switch (r) {
		 case TRIV_UNSAT:
		 case TRIV_SAT:
		 case PREP_ERROR: return r; 
		 default: break;
		}
	}
	
	bdd_gc(1);
	
	switch (int r=Do_Apply_Inferences()) {
	 case TRIV_UNSAT:
	 case TRIV_SAT:
	 case PREP_ERROR: return r;
	 default: break;
	}

	d3_printf1("\n");
   d2e_printf1("\rPreprocessing Rw                         ");
	
	int ret = PREP_CHANGED;

	int Total_inferences = Pos_replace + Neg_replace + Setting_Pos + Setting_Neg;

	//fprintf(stderr, "%d, %d, %d, %ld, %4.2f, %lldM\n", tier, num_safe_assigns, Total_inferences, numinp, get_runtime()-start_prep, memory_used/1024);

	if (Total_inferences > num_inferences) num_inferences = Total_inferences;
	else {
		tier++;
		ret = PREP_NO_CHANGE;
	}
	
	//bdd_gc(1);

	for(int x = 0; x < nmbrFunctions; x++)
	  original_functions[x] = functions[x];
	
	return ret;
}
