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

int Pos_replace = 0;
int Neg_replace = 0;
int Setting_Pos = 0;
int Setting_Neg = 0;

llistStruct *amount;
Linear *l;
int *length = NULL;
infer *inferlist;
infer *lastinfer;
int notdone;
int *tempint = NULL;
float *var_score = NULL;
store *variables;
int *num_funcs_var_occurs = NULL;
int str_length;
int preproc_did_nothing = 0;
long affected;
BDDNodeStruct **xorFunctions;

int original_numout;
BDDNodeStruct **original_functions = NULL;

int Finish_Preprocessing();
int Init_Preprocessing();

double start_prep = 0;

int
Init_Preprocessing()
{
	if(nmbrFunctions == 0) {
		preproc_did_nothing = 1;
		return TRIV_SAT;
	}
	bool OLD_DO_INFERENCES = DO_INFERENCES;
	DO_INFERENCES = 0;

	int ret = PREP_NO_CHANGE;
	
	start_prep = get_runtime ();	//Start clock
	
	Pos_replace = 0;
	Neg_replace = 0;
	Setting_Pos = 0;
	Setting_Neg = 0;
	
	numinp = getNuminp ();

	//BDDNodeStruct **original_functions;

	if(original_functions == NULL) {
		ITE_NEW_CATCH(
						  original_functions = new BDDNode *[nmbrFunctions + 1],
						  "original functions");
		
		for(int x = 0; x < nmbrFunctions; x++)
		  original_functions[x] = functions[x];
		original_numout = nmbrFunctions;
	}
	
	if(variablelist==NULL) {
		variablelist = new varinfo[numinp + 1];
		
		for (int x = 0; x < numinp + 1; x++)
		  {
			  variablelist[x].equalvars = 0;
			  variablelist[x].replace = x;
			  variablelist[x].true_false = -1;
		  }
	}
	
	numout = nmbrFunctions;
	if(tempint != NULL) delete [] tempint;
   tempint = new int[5000];


	if(length != NULL) ite_free((void **)&variables);
	length = (int *)ite_recalloc(NULL, 0, nmbrFunctions, sizeof(int), 9, "length");
	//length = new int[nmbrFunctions];
	inferlist = new infer;
	inferlist->next = NULL;
	lastinfer = inferlist;
	variables = (store *)ite_recalloc(NULL, 0, nmbrFunctions, sizeof(store), 9, "variables");
	//variables = (store *)calloc(nmbrFunctions+1, sizeof(store));
	
	Init_Repeats();
	
	//numinp = getNuminp();

	l = new Linear (numinp + 1, T, F);
	amount = (llistStruct*)calloc(numinp+1, sizeof(llistStruct));
	
	//Working with preset variables.

	BDDNode *preset_bdd = true_ptr;
	int iter = 0;
	int str_length = strlen(preset_variables_string);
	int begin = iter;
	while(iter < str_length) {
		while(preset_variables_string[iter] == ' ' && iter < str_length) iter++;
		if(iter == str_length) break;
		int sign;
		char p = preset_variables_string[iter];
		if(p == '+') {
			sign = 1;
		} else if(p == '-') {
			sign = 0;
		} else {
			fprintf (stderr, "\nExpected '+' or '-' in --preset-variables, found '%c'", p);
			fprintf (stderr, "\nAll variables must be preceeded by either '+' or '-'...exiting\n");
			exit (1);
		}
		iter++;
		begin = iter;
		p = preset_variables_string[iter];
		while (((p >= 'a') && (p <= 'z')) || ((p >= 'A') && (p <= 'Z'))
				 || (p == '_') || ((p >= '0') && (p <= '9'))) {
			iter++;
			p = preset_variables_string[iter];
			if(iter == str_length) break;
		}
		if((p != ' ' && p != 0) || begin == iter) {
			fprintf(stderr, "\nUnexpected character '%c' in --preset-variables...exiting\n", p);
			exit(1);
		}
		preset_variables_string[iter] = 0;
		int intnum = i_getsym(preset_variables_string+begin, SYM_VAR);
		//fprintf(stderr, "{%d %s}", intnum, preset_variables_string+begin);
		if(intnum > numinp) {
			fprintf(stderr, "\nUnknown variable %s found in --preset-variables...exiting\n", preset_variables_string+begin);
			exit(1);
		}
		if(sign==0) intnum = -intnum;
		preset_bdd = ite_and(preset_bdd, ite_var(intnum));
		
		if(iter < str_length) preset_variables_string[iter] = ' ';
	}

	int bdd_length = 0;
	int *bdd_vars = NULL;
	switch (int r=Rebuild_BDD(preset_bdd, &bdd_length, bdd_vars)) {
	 case TRIV_UNSAT:
	 case TRIV_SAT:
	 case PREP_ERROR: return r; 
	 default: break;
	}
	delete [] bdd_vars;
	bdd_vars = NULL;

	//Done working with preset variables.
	
	for (int x = 0; x < nmbrFunctions; x++)
	  {
		  variables[x].num = NULL;
		  int r=Rebuild_BDDx(x);
		  switch (r) {
			case TRIV_UNSAT:
			case TRIV_SAT:
			case PREP_ERROR: return r; 
			default: break;
		  }
	  }

   num_funcs_var_occurs = (int *)ite_calloc(numinp+1, sizeof(int), 9, "num_funcs_var_occurs");
	
	for (int x = 0; x < nmbrFunctions; x++)
	  {
		  for (int i = 0; i < length[x]; i++)
			 {
				 llist *newllist = new llist;
				 newllist->num = x;
				 newllist->next = NULL;
				 if (amount[variables[x].num[i]].head == NULL)
					{
						num_funcs_var_occurs[variables[x].num[i]] = 1;
						amount[variables[x].num[i]].head = newllist;
						amount[variables[x].num[i]].tail = newllist;
					}
				 else
					{
						num_funcs_var_occurs[variables[x].num[i]]++;
						amount[variables[x].num[i]].tail->next = newllist;
						amount[variables[x].num[i]].tail = newllist;
					}
			 }
	  }
	
	DO_INFERENCES = OLD_DO_INFERENCES;

   //Do_Flow();
   //Do_Flow_Grouping();

	return ret;
}

int
Triv_Unsat()
{
  d2_printf1 ("\nFormula is UNSATISFIABLE\n");
  functions[0] = false_ptr;
  nmbrFunctions = 0;
  return 0;
}

int
Finish_Preprocessing()
{	
	if(preproc_did_nothing == 1) return TRIV_SAT;
	int ret = PREP_NO_CHANGE;
	
	//Call Apply_Inferences to make sure all inferences were applied.
	if (DO_INFERENCES)
	  {
		  /* need to handle the result
			* e.g. if error
			*/
        ret = Do_Apply_Inferences();
	  }
//	Do_Flow();
	
	int *tmp = new int[numinp+2];
	for(int x = 0; x < numinp+1; x++)
	  tmp[x] = 0;
	for(int x = 0; x < nmbrFunctions; x++) {
		if(variables[x].num!=NULL)
		  for(int y = 0; y < length[x]; y++) {
			  tmp[variables[x].num[y]] = 1;
		  }
	}
	for(int x = 0; x < numinp+1; x++)
	  if(tmp[x] == 0 && variablelist[x].true_false==-1)
		 variablelist[x].true_false = 3;
	delete []tmp;	
	
	/****** Shrink down the hash table ******/
	//printCircuit();
	//Stats(length, variables);
	//
	// cheat_replaceall (length, variables, variablelist);
	// doing cheat_replaceall a different way!
	// will conflict with the backend!
	// It is now being handled by the brancher
	// Do NOT uncomment...
	/****** Shrunk down the hash table ******/
	
	int Total_inferences = Pos_replace + Neg_replace + Setting_Pos + Setting_Neg;
	if (Total_inferences == 0)
	  Total_inferences++;
	d3_printf3 ("Positive Replaces - %d (%d%%)\n", Pos_replace,
					(100 * Pos_replace) / (Total_inferences));
	d3_printf3 ("Negative Replaces - %d (%d%%)\n", Neg_replace,
					(100 * Neg_replace) / (Total_inferences));
	d3_printf3 ("Positive Sets     - %d (%d%%)\n", Setting_Pos,
					(100 * Setting_Pos) / (Total_inferences));
	d3_printf3 ("Negative Sets     - %d (%d%%)\n\n", Setting_Neg,
					(100 * Setting_Neg) / (Total_inferences));
	
	//Stats(length, variables);
	
	//Need to release these pointers
	
	for (int x = 0; x < numinp + 1; x++)
	  {
		  llist *k = amount[x].head;
		  while (k != NULL)
			 {
				 llist *temp = k;
				 k = k->next;
				 delete temp;
			 }
		  amount[x].head = NULL;
		  amount[x].tail = NULL;
	  }
	free(amount);
	amount = NULL;
	
	ite_free((void**)&num_funcs_var_occurs);
	
	for (int x = 0; x < nmbrFunctions; x++)
	  {
		  if (variables[x].num != NULL)
			 delete [] variables[x].num;
	  }
	
	ite_free((void **)&variables);
	//free(variables);
	variables = NULL;

	while(inferlist != NULL) {
		infer *temp = inferlist;
		inferlist = inferlist->next;
		delete temp;
	}
	inferlist = NULL;
	
	delete [] tempint;
	tempint = NULL;
	delete l;
	l = NULL;
	
	Delete_Repeats();
	
	//original_functions and variablelist need to be
	// deleted after backtrack.cc has run.
	// 
	//Done releasing pointers

	//Need to remove any function that was set to True during the preprocessing of the BDDs
	
	int count = -1;
	for (long x = 0; x < nmbrFunctions; x++)
	  {
		  count++;
		  functions[count] = functions[x];
		  functionType[count] = functionType[x];
		  equalityVble[count] = equalityVble[x];
		  //parameterizedVars[count] = parameterizedVars[x];
		  length[count] = length[x];
		  
		  if (functions[x] == true_ptr)
			 count--;
		  if (functions[x] == false_ptr)
			 {
				 /* this might happen but I already know about unsatisfiness */
				 // ret = TRIV_UNSAT;
			 }
		  
	  }
	nmbrFunctions = count + 1;
	numout = nmbrFunctions;

	for (long x = 0; x < nmbrFunctions; x++) {
         if ((functionType[x] == AND && length[x] < AND_EQU_LIMIT)
          || (functionType[x] == OR && length[x] < OR_EQU_LIMIT)
          || (functionType[x] == PLAINOR && length[x] < PLAINOR_LIMIT)
          || (functionType[x] == PLAINXOR && length[x] < PLAINXOR_LIMIT)) 
            functionType[x] = UNSURE;
   }

	d3_printf3 ("Number of BDDs - %d\nNuminp = %ld\n", nmbrFunctions, numinp);
	
	D_3(fflush (stddbg);)
	  //printCircuitTree();
	  //printCircuit();
	  // 
	  if (nmbrFunctions == 0)
		 {
			 /* I need to announce this to the ite.cc!!! 
			  * how?
			  * ret = TRIV_SAT;
			  */
			 ret = TRIV_SAT;
			 //d1_printf1 ("Formula was trivially satisfiable.\n");
		 }
	
   ite_counters_f[PREPROC_TIME] = get_runtime() - start_prep;
   d3_printf2("Preprocessing Time: %5.3f seconds.\n",
					ite_counters_f[PREPROC_TIME]);
   d2_printf1("\rPreprocessing .... Done\n");

   return ret;
}

int add_newFunctions(BDDNode **new_bdds, int new_size) {
	int ret = PREP_NO_CHANGE;

	bool OLD_DO_INFERENCES = DO_INFERENCES;
	DO_INFERENCES = 0;
	
	nmbrFunctions = nmbrFunctions+new_size;
	functions_alloc(nmbrFunctions);

	length = (int *)ite_recalloc(length, nmbrFunctions-new_size, nmbrFunctions, sizeof(int), 9, "length");
	variables = (store *)ite_recalloc(variables, nmbrFunctions-new_size, nmbrFunctions, sizeof(store), 9, "variables");

	Init_Repeats();

	for(int x = nmbrFunctions-new_size; x < nmbrFunctions; x++) {		  
		functions[x] = new_bdds[x-(nmbrFunctions-new_size)];		
	}
	
	for (int x = nmbrFunctions-new_size; x < nmbrFunctions; x++) {
		variables[x].num = NULL;
		int r=Rebuild_BDDx(x);
		switch (r) {
		 case TRIV_UNSAT:
		 case TRIV_SAT:
		 case PREP_ERROR: return r;
		 default: break;
		}
	}
	
	for (int x = nmbrFunctions-new_size; x < nmbrFunctions; x++) {
		for (int i = 0; i < length[x]; i++) {
			llist *newllist = new llist;
			newllist->num = x;
			newllist->next = NULL;
			if (amount[variables[x].num[i]].head == NULL) {
				num_funcs_var_occurs[variables[x].num[i]] = 1;
				amount[variables[x].num[i]].head = newllist;
				amount[variables[x].num[i]].tail = newllist;
			} else {
				num_funcs_var_occurs[variables[x].num[i]]++;
				amount[variables[x].num[i]].tail->next = newllist;
				amount[variables[x].num[i]].tail = newllist;
			}
		}
	}

	DO_INFERENCES = OLD_DO_INFERENCES;

	if (DO_INFERENCES) {
		ret = Do_Apply_Inferences();
	}
	return ret;
}
