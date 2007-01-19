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

int size = 10;

int Do_Prover3() {
	if(formatin != '3') return PREP_NO_CHANGE;
	d3_printf2("RECOMPUTING PROVER %d - \n", size);
	str_length = 0;
	bool OLD_DO_INFERENCES = DO_INFERENCES;
	DO_INFERENCES = 0;

	Pos_replace = Neg_replace = Setting_Pos = Setting_Neg = 0;

	/****** DELETING ARRAYS ********/
	ite_free((void**)&original_functionType);
	ite_free((void**)&original_equalityVble);
   ite_free((void**)&original_functions);
	for (int x = 0; x < numinp + 1; x++) {
		llist *k = amount[x].head;
		while (k != NULL) {
			llist *temp = k;
			k = k->next;
			delete temp;
		}
		amount[x].head = NULL;
		amount[x].tail = NULL;
	}
	free(amount);
	ite_free((void**)&num_funcs_var_occurs);
	for (int x = 0; x < nmbrFunctions; x++) {
		if (variables[x].num != NULL)
		  delete [] variables[x].num;
	}
	ite_free((void **)&variables);
	variables_size = 0;
	ite_free((void **)&length);
	length_size = 0;
	
	Delete_Repeats();
	for(int x = 1; x < numinp + 1; x++)
	  if(independantVars[x] == 2) {
		  variablelist[x].true_false = -1;
		  variablelist[x].equalvars = 0;
	  }
	
	/****** DONE DELETING ARRAYS ********/	

	//Grabbing new prover3 BDDS
   nmbrFunctions = 0;
	prover3_max_vars = size;
	void p3_done();
   p3_done();
	
	//Saving Inferences.
	int old_numinp = numinp;
	delete l;
	numinp = getNuminp();
	F = numinp+3;
	T = numinp+2;
	l = new Equiv (numinp + 1, nmbrFunctions, T, F);
	numinp = old_numinp;
	CreateInferences();
	numinp = getNuminp();
   ite_free((void **)&variablelist);
	
	/****** REALLOCATING ARRAYS *********/
	
	variablelist = new varinfo[numinp + 1];
	for (int x = 0; x < numinp + 1; x++) {
		variablelist[x].equalvars = 0;
		variablelist[x].replace = x;
		variablelist[x].true_false = -1;
	}
	ITE_NEW_CATCH(original_functions = new BDDNode *[nmbrFunctions + 1], "original functions");
	original_functionType = (int *)ite_calloc(nmbrFunctions + 1, sizeof(int), 2, "original functionType");
	original_equalityVble = (int *)ite_calloc(nmbrFunctions + 1, sizeof(int), 2, "original equalityVble");

	for(int x = 0; x < nmbrFunctions; x++) {
		original_functions[x] = functions[x];
		original_functionType[x] = functionType[x];
		original_equalityVble[x] = equalityVble[x];
	}
	original_numout = nmbrFunctions;
	length = (int *)ite_recalloc(NULL, 0, nmbrFunctions, sizeof(int), 9, "length");
	length_size = nmbrFunctions;
	variables = (store *)ite_recalloc(NULL, 0, nmbrFunctions, sizeof(store), 9, "variables");
	variables_size = nmbrFunctions;
   num_funcs_var_occurs = (int *)ite_calloc(numinp+1, sizeof(int), 9, "num_funcs_var_occurs");
	amount = (llistStruct*)calloc(numinp+1, sizeof(llistStruct));
	
	/****** DONE REALLOCATING ARRAYS ******/
	
	/****** INITIALIZING ARRAYS ***********/
	
	Init_Repeats();

	for (int x = 0; x < nmbrFunctions; x++) {
		int r=Rebuild_BDDx(x);
		switch (r) {
		 case TRIV_UNSAT:
		 case TRIV_SAT:
		 case PREP_ERROR: return r; 
		 default: break;
		}
	}

	/****** DONE INITIALIZING ARRAYS ******/
	
	DO_INFERENCES = OLD_DO_INFERENCES;
	
	switch (int r=Do_Apply_Inferences()) {
	 case TRIV_UNSAT:
	 case TRIV_SAT:
	 case PREP_ERROR: return r;
	 default: break;
	}
	
	d3_printf1("\n");
   d2e_printf1("\r                                         ");
	
	int ret = PREP_NO_CHANGE;
	
	return ret;
}
