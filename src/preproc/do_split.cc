/* =========FOR INTERNAL USE ONLY. NO DISTRIBUTION PLEASE ========== */

/*********************************************************************
 Copyright 1999-2003, University of Cincinnati.  All rights reserved.
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
/*********************************************************
 *  preprocess.c (S. Weaver)
 *********************************************************/

#include "ite.h"
#include "preprocess.h"

int Split_Large();

int Do_Split() {
   d3_printf1("SPLITTING LARGE FUNCTIONS - ");
   int cofs = PREP_CHANGED;
   int ret = PREP_NO_CHANGE;
	affected = 0;
	char p[100];
	D_3(
		 sprintf(p, "{0:0/%d}", nmbrFunctions);
		 str_length = strlen(p);
		 d3_printf1(p);
	);
	while (cofs!=PREP_NO_CHANGE) {
      cofs = Split_Large ();
      if(cofs == PREP_CHANGED) ret = PREP_CHANGED;
      else if(cofs == TRIV_UNSAT) {
			return TRIV_UNSAT;
		}
	}
	
	d3_printf1("\n");
   d2e_printf1("\r                                         ");
	return ret;
}

int nCk(int n, int k) {
	if(n == k) return 1;
	if(k == 0) return 1;
	return nCk(n-1, k) + nCk(n-1, k-1);	
}


int Split_Large () {
	int ret = PREP_NO_CHANGE;

	int max_bdds = 0;
	int num_bdds = 0;
	BDDNode **BDDFuncs = (BDDNode **)ite_recalloc(BDDFuncs, max_bdds, max_bdds+10, sizeof(BDDNode *), 9, "BDDFuncs");
	max_bdds += 10;
	
	for(int j = 0; j < nmbrFunctions; j++) {
		if (functionType[j] == UNSURE && length[j] > 10) { //10?
			d3_printf2("\n%d: ", j);
			printBDD(functions[j]);
			d3_printf1("\n");			
			int num_splits = nCk(length[j], 10);
			d3_printf4("%dC%d = %d\n", length[j], 10, num_splits);
			
			
			
			if(num_bdds >= max_bdds) {
				BDDFuncs = (BDDNode **)ite_recalloc(BDDFuncs, max_bdds, max_bdds+10, sizeof(BDDNode *), 9, "BDDFuncs");
				max_bdds += 10;
			}

			
			
			
			/*
			switch (int r=Rebuild_BDDx(j)) {
			 case TRIV_UNSAT:
			 case TRIV_SAT:
			 case PREP_ERROR: 
				ret=r;
					goto sp_bailout;
			 default: break;
			}
			*/
			
		}
	}

		

	sp_bailout:
	return ret;
}
