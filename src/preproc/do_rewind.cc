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

int Do_Rewind() {
	d3_printf1("REWINDING - ");
	str_length = 0;
	MAX_EXQUANTIFY_CLAUSES = 20;
	MAX_EXQUANTIFY_VARLENGTH = 5;
	
	bool OLD_DO_INFERENCES = DO_INFERENCES;
	DO_INFERENCES = 0;

	Pos_replace = Neg_replace = Setting_Pos = Setting_Neg = 0;

	int y = 0;
	for(int x = 0; x < nmbrFunctions; x++) {
		if(functionType[x] == AUTARKY_FUNC) {
			functions[y] = functions[x];
			functionType[y] = AUTARKY_FUNC;
			equalityVble[y] = equalityVble[x];
			//fprintf(stderr, "keeping autark %d at %d\n", x, y);
			y++;
		} else {
			functions[x] = true_ptr;
			functionType[x] = UNSURE;
			equalityVble[x] = 0;
			Rebuild_BDDx(x);
		}
	}

	for(int x = 0; x < original_numout; x++) {
		functions[y] = original_functions[x];
		functionType[y] = original_functionType[x];
		equalityVble[y] = original_equalityVble[x];
		y++;
	}
	
	//fprintf(stderr, "%d %d\n", original_numout, nmbrFunctions); 
	
	nmbrFunctions = y;

	delete l;
	l = new Linear (numinp + 1, T, F);
	
	CreateInferences();
	for(int x = 1; x < numinp + 1; x++)
	  if(variablelist[x].true_false == 2) {
		  variablelist[x].replace = x;
		  variablelist[x].equalvars = 0;
		  variablelist[x].true_false = -1;
	  }

	//Very important that this comes after CreateInferences()
	//Because the inferences that come from the autarky BDDs
	//may not be valid.
	for (int x = 0; x < nmbrFunctions; x++) {
		int r=Rebuild_BDDx(x);
		switch (r) {
		 case TRIV_UNSAT:
		 case TRIV_SAT:
		 case PREP_ERROR: return r; 
		 default: break;
		}
	}
	
	DO_INFERENCES = OLD_DO_INFERENCES;
	
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
	if (Total_inferences > num_inferences) num_inferences = Total_inferences;
	else ret = PREP_NO_CHANGE;
	fprintf(stderr, "%d/%ld ", Total_inferences, numinp);

	//bdd_gc(1);
	
	return ret;
}
