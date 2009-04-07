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

int Pos_replace = 0;
int Neg_replace = 0;
int Setting_Pos = 0;
int Setting_Neg = 0;

int T;
int F;
//llistStruct *amount = NULL;
Equiv *l;
infer *inferlist;
infer *lastinfer;
int notdone;
//int *tempint = NULL;
int num_inferences = 0;
int num_safe_assigns = 0;
float *var_score = NULL;
//int *num_funcs_var_occurs = NULL;
int str_length;
int preproc_did_nothing = 0;
long affected;
int length_size;
int variables_size;
BDDNodeStruct **xorFunctions;
int bdd_tempint_max=0;
int *bdd_tempint=NULL;
int *original_functionType;
int *original_equalityVble;
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
	
	//start_prep = get_runtime ();	//Start clock
												//The clock is start earlier now
											   //before reading the input file.	
	Pos_replace = 0;
	Neg_replace = 0;
	Setting_Pos = 0;
	Setting_Neg = 0;
	
	numinp = getNuminp ();
	F = numinp+3;
	T = numinp+2;

	//BDDNodeStruct **original_functions;

	original_functionType = (int *)ite_calloc(nmbrFunctions + 1, sizeof(int), 9, "original functionType");
	original_equalityVble = (int *)ite_calloc(nmbrFunctions + 1, sizeof(int), 9, "original equalityVble");

	for(int x = 0; x < nmbrFunctions; x++) {
	  original_functionType[x] = functionType[x];
	  original_equalityVble[x] = equalityVble[x];
	}

	if(original_functions == NULL) {
      original_functions = (BDDNode **)ite_calloc(nmbrFunctions+1, sizeof(BDDNode*), 9, "original_functions");
		for(int x = 0; x < nmbrFunctions; x++)
		  original_functions[x] = functions[x];
		original_numout = nmbrFunctions;
	}

	if(variablelist==NULL) {
		variablelist = (varinfo*)ite_calloc(numinp+1, sizeof(varinfo), 9, "variablelist");
		
		for (int x = 0; x < numinp + 1; x++) {
			  variablelist[x].equalvars = 0;
			  variablelist[x].replace = x;
			  variablelist[x].true_false = -1;
		}
	}

	length_size = nmbrFunctions;
	
	numout = nmbrFunctions;

	if(length != NULL) ite_free((void **)&length);
	length = (int *)ite_recalloc(NULL, 0, nmbrFunctions, sizeof(int), 9, "length");
   /*
	inferlist = new infer;
	inferlist->next = NULL;
   */
   inferlist = AllocateInference(0, 0, NULL);
	lastinfer = inferlist;
	if(variables != NULL) { 
		for (int x = 0; x < variables_size; x++) {
			if (variables[x].num != NULL)
			  delete [] variables[x].num;
		}
		ite_free((void **)&variables);
	}
	variables = (store *)ite_recalloc(NULL, 0, nmbrFunctions, sizeof(store), 9, "variables");
   variables_size = nmbrFunctions;
	
	Init_Repeats();
	
	//numinp = getNuminp();

	l = new Equiv (numinp + 1, nmbrFunctions, T, F);
	if(amount == NULL) amount = (llistStruct*)ite_calloc(numinp+1, sizeof(llistStruct), 9, "amount");
	
	//Working with preset variables.

	BDDNode *preset_bdd = true_ptr;
	int iter = 0;
	str_length = strlen(preset_variables_string);
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

	if(num_funcs_var_occurs == NULL) num_funcs_var_occurs = (int *)ite_calloc(numinp+1, sizeof(int), 9, "num_funcs_var_occurs");

	char p[100];
	for (int x = 0; x < nmbrFunctions; x++)
	  {
        D_3(
              if ((x % 1000) == 0) {
					  for(int iter = 0; iter<str_length; iter++)
						 d3_printf1("\b");
					  sprintf(p, "Rebuild %d/%d", x, nmbrFunctions);
					  str_length = strlen(p);
					  d3_printf1(p);
				  }
          )
		  variables[x].num = NULL;
		  int r=Rebuild_BDDx(x);
		  switch (r) {
			case TRIV_UNSAT:
			case TRIV_SAT:
			case PREP_ERROR: return r; 
			default: break;
		  }
	  }

/*	
	for (int x = 0; x < nmbrFunctions; x++) {
		for (int i = 0; i < length[x]; i++) {
         llist *newllist = AllocateLList(x, NULL);
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
*/

	DO_INFERENCES = OLD_DO_INFERENCES;

	//Do_Apply_Inferences();
	
	for (int x = 0; x < nmbrFunctions; x++) {
		D_3(
			 if ((x % 1000) == 0) {
				 for(int iter = 0; iter<str_length; iter++)
					d3_printf1("\b");
				 sprintf(p, "Rebuild %d/%d", x, nmbrFunctions);
				 str_length = strlen(p);
				 d3_printf1(p);
			 }
			 )
		int r=Rebuild_BDDx(x);
		switch (r) {
		  case TRIV_UNSAT:
		  case TRIV_SAT:
		  case PREP_ERROR: return r; 
		  default: break;
		}
	}
	
   //Do_Flow();
   //Do_Flow_Grouping();

	return ret;
}

extern int tier;

int
Triv_Unsat() {
	long long memory_used = get_memusage();
	int Total_inferences = Pos_replace + Neg_replace + Setting_Pos + Setting_Neg;
	//fprintf(stderr, "%d, %d, %d, %ld, %4.2f, %ldM, ", tier, num_safe_assigns, Total_inferences, numinp, get_runtime()-start_prep, memory_used/1024);

	d2_printf1 ("\nFormula is UNSATISFIABLE\n");
	functions[0] = false_ptr;
	nmbrFunctions = 0;
	return 0;
}

int
Finish_Preprocessing()
{
	long long memory_used = get_memusage();

	if(preproc_did_nothing == 1) return TRIV_SAT;
	int ret = PREP_NO_CHANGE;

	//Call Apply_Inferences to make sure all inferences were applied.
	if (DO_INFERENCES) {
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
	
	int *influence_times = (int *)ite_calloc(numinp + 1, sizeof(int), 9, "influence_times");

	if(arrVarTrueInfluences == NULL) {
		arrVarTrueInfluences = (double *)ite_calloc(numinp+1, sizeof(double), 9, "arrVarTrueInfluences");
		for(int i = 0; i < numinp+1; i++)
		  arrVarTrueInfluences[i] = 0.5;
	}

	
	//After this all equalvars should point to their respective supernodes
	//in the inverse tree (equivclass).
   //This would have to change if we replace variables differently.

	for (int i = 1; i <= numinp; i++) {
		if (variablelist[i].equalvars != 0 && variablelist[abs(variablelist[i].equalvars)].equalvars!=0) {
			if(variablelist[i].equalvars>0) variablelist[i].equalvars = variablelist[variablelist[i].equalvars].equalvars;
			else variablelist[i].equalvars = -variablelist[-variablelist[i].equalvars].equalvars;
			if(variablelist[i].equalvars > 0) {
				arrVarTrueInfluences[variablelist[i].equalvars]+=arrVarTrueInfluences[i];
				influence_times[variablelist[i].equalvars]++;
			} else {
				arrVarTrueInfluences[-variablelist[i].equalvars]+=1.0-arrVarTrueInfluences[i];
				influence_times[-variablelist[i].equalvars]++;
			}
		} else if(variablelist[i].equalvars != 0) {
			if (variablelist[i].equalvars>0) {
				arrVarTrueInfluences[variablelist[i].equalvars]+=arrVarTrueInfluences[i];
				influence_times[variablelist[i].equalvars]++;
			} else {
				arrVarTrueInfluences[-variablelist[i].equalvars]+=1.0-arrVarTrueInfluences[i];
				influence_times[-variablelist[i].equalvars]++;
			}
		}
	}
	
	for(int x = 0; x < nVarChoiceLevelsNum; x++) {
		for(int i = 0; arrVarChoiceLevels[x][i]!=0; i++) {
			if(variablelist[arrVarChoiceLevels[x][i]].equalvars != 0) {
				if(variablelist[arrVarChoiceLevels[x][i]].equalvars > 0) {
					arrVarChoiceLevels[x][i] = variablelist[arrVarChoiceLevels[x][i]].equalvars;
				} else {
					arrVarChoiceLevels[x][i] = -variablelist[arrVarChoiceLevels[x][i]].equalvars;
				}
			}
		}
	}

	for(int x = 0; x < numinp+1; x++)
	  arrVarTrueInfluences[x] = arrVarTrueInfluences[x] / (double)(1+influence_times[x]);

	ite_free((void**)&influence_times);

	int *isinVCL = (int *)ite_calloc(numinp + 1, sizeof(int), 9, "isinVCL");
	
	int count=-1;
	for(int x = 0; x < nVarChoiceLevelsNum; x++) {
		count=-1;
		for(int i = 0; arrVarChoiceLevels[x][i]!=0; i++) {
			count++;
			arrVarChoiceLevels[x][count] = arrVarChoiceLevels[x][i];
			if(isinVCL[arrVarChoiceLevels[x][i]] == 1 || variablelist[arrVarChoiceLevels[x][i]].true_false!=-1)
			  count--;
			else isinVCL[arrVarChoiceLevels[x][i]] = 1;
		}
		arrVarChoiceLevels[x][count+1] = 0;
	}
	
	count=-1;	
	for(int x = 0; x < nVarChoiceLevelsNum; x++) {
		count++;
		int *tmp = arrVarChoiceLevels[count];
		arrVarChoiceLevels[count] = arrVarChoiceLevels[x];
		if(arrVarChoiceLevels[x][0] == 0) {
			count--;
		}
		arrVarChoiceLevels[x] = tmp;
	}
	nVarChoiceLevelsNum = count+1;
	
	ite_free((void**)&isinVCL);
	
	d6_printf1("\n");
	for(int x = 0; x < nVarChoiceLevelsNum; x++) {
		for(int i = 0; arrVarChoiceLevels[x][i]!=0; i++)
		 d6_printf2("%d ", arrVarChoiceLevels[x][i]);
		d6_printf1("\n");
	}
	d6_printf1("\n");
	for(int x = 0; x < numinp+1; x++)
	  d6_printf3("%d:%4.4f ", x, arrVarTrueInfluences[x]);
	d6_printf1("\n");
	
	int Total_inferences = Pos_replace + Neg_replace + Setting_Pos + Setting_Neg;
	int div_zero = 0;
	if (Total_inferences == 0)
	  div_zero = 1;
	d3_printf3 ("Positive Replaces - %d (%d%%)\n", Pos_replace,
					(100 * Pos_replace) / (Total_inferences+div_zero));
	d3_printf3 ("Negative Replaces - %d (%d%%)\n", Neg_replace,
					(100 * Neg_replace) / (Total_inferences+div_zero));
	d3_printf3 ("Positive Sets     - %d (%d%%)\n", Setting_Pos,
					(100 * Setting_Pos) / (Total_inferences+div_zero));
	d3_printf3 ("Negative Sets     - %d (%d%%)\n", Setting_Neg,
					(100 * Setting_Neg) / (Total_inferences+div_zero));
	d3_printf2 ("Total Inferences  - %d (100%%)\n\n", Total_inferences);

   //fprintf(stderr, "%d, %d, %d, %ld, %4.2f, %ldM, ", tier, num_safe_assigns, Total_inferences, numinp, get_runtime()-start_prep, memory_used/1024);

	//Stats();
	
	//Need to release these pointers
	
	for (int x = 0; x < numinp + 1; x++) {
		DeallocateLLists(amount[x].head);
		amount[x].head = NULL;
		amount[x].tail = NULL;
	}
	ite_free((void**)&amount);
	amount = NULL;
	
	ite_free((void**)&num_funcs_var_occurs);

   DeallocateInferences(inferlist);
	inferlist = NULL;
	
	//delete [] tempint;
	//tempint = NULL;
	
	Delete_Repeats();
	
	//original_functions and variablelist need to be
	// deleted after backtrack.cc has run.
	// 
	//Done releasing pointers

	int var_nmbrFunctions = nmbrFunctions;
	
	if(countBDDs() == 0) nmbrFunctions = 0;

	//Need to remove any function that was set to True during the preprocessing of the BDDs
	count = -1;
	for (int x = 0; x < nmbrFunctions; x++) {
		count++;
		functions[count] = functions[x];
		functionType[count] = functionType[x];
		equalityVble[count] = equalityVble[x];
//		variables[count] = variables[x]; //Should delete the ones skipped over SEAN!!! LOOK HERE!!!
		length[count] = length[x];
		//parameterizedVars[count] = parameterizedVars[x];
		if (functions[x] == true_ptr) {
			count--; //Don't send function to the solver
		} else if (functions[x] == false_ptr) {
			/* this might happen but I already know about unsatisfiedness */
			ret = TRIV_UNSAT;
		}
	}
	nmbrFunctions = count + 1;
	numout = nmbrFunctions;

	delete l; //Delete the equiv_class
	l = NULL;

	for (int x = 0; x < var_nmbrFunctions; x++) {
		if (variables[x].num != NULL)
		  delete [] variables[x].num;
	}
	ite_free((void **)&variables);
	variables = NULL;
	
   ite_free((void**)&bdd_tempint); bdd_tempint_max = 0;
	ite_free((void**)&original_functionType);
	ite_free((void**)&original_equalityVble);

	if(countBDDs()!=nmbrFunctions){
		d3_printf2("Number of normal BDDs - %d\n", countBDDs());
		d3_printf2("Total number of BDDs  - %d\n", nmbrFunctions);
		d3_printf2("Number of Variables   - %ld\n", numinp);
	} else {
		d3_printf2("Number of BDDs      - %d\n", countBDDs());
		d3_printf2("Number of Variables - %ld\n", numinp);
	}
	
	D_3(fflush (stddbg);)
	  //printCircuitTree();
	  //printCircuit();

	if (countBDDs() == 0
		 //|| (countBDDs() < 1 && max_solutions == 1)
		 ) {
		ret = TRIV_SAT;
		//d1_printf1 ("Formula was trivially satisfiable.\n");
	}
	
   if(ite_counters_f[PREPROC_TIME] == 0) {
		ite_counters_f[PREPROC_TIME] = get_runtime() - start_prep;
		d3_printf2("Preprocessing Time: %5.3f seconds.\n",
					  ite_counters_f[PREPROC_TIME]);
	}
	d2_printf1("\rPreprocessing .... Done\n");
	
   return ret;
}

int add_newFunctions(BDDNode **new_bdds, int new_size, int **free_spots_pass) {

	if(new_size <= 0) return PREP_NO_CHANGE;
	
	int free_funcs = 0;
	(*free_spots_pass) = (int *)ite_calloc(new_size+1, sizeof(int), 9, "free_spots");

	int *free_spots = (*free_spots_pass);
	
	for(int x = 0; x < nmbrFunctions; x++) {
		if(functions[x] == true_ptr) {
			free_spots[free_funcs] = x;
			free_funcs++;
			if(free_funcs >= new_size) break;
		}
	}

	if(free_funcs < new_size) {
		int oldnumfuncs = nmbrFunctions;
		nmbrFunctions = nmbrFunctions+new_size-free_funcs;
		functions_alloc(nmbrFunctions);

		if(length_size<nmbrFunctions) {
			length = (int *)ite_recalloc(length, length_size, nmbrFunctions, sizeof(int), 9, "length");
			length_size = nmbrFunctions;
		}
		if(variables_size<nmbrFunctions) {
			variables = (store *)ite_recalloc(variables, variables_size, nmbrFunctions, sizeof(store), 9, "variables");
			variables_size = nmbrFunctions;
		}
		Init_Repeats();
		for(int x = 0;free_funcs < new_size; x++)
		  free_spots[free_funcs++] = oldnumfuncs+x;
	}
	
	bool OLD_DO_INFERENCES = DO_INFERENCES;
	DO_INFERENCES = 0;
	
	for(int x = 0; x < free_funcs; x++) {
		int y = free_spots[x];
		functions[y] = new_bdds[x];
		functionType[y] = UNSURE;
		equalityVble[y] = 0;
		int r=Rebuild_BDDx(y);
		switch (r) {
		 case TRIV_UNSAT:
		 case TRIV_SAT:
		 case PREP_ERROR: return r;
		 default: break;
		}
	}

	DO_INFERENCES = OLD_DO_INFERENCES;
	
	for(int x = 0; x < free_funcs; x++) {
		int y = free_spots[x];
		int r=Rebuild_BDDx(y);
		switch (r) {
		 case TRIV_UNSAT:
		 case TRIV_SAT:
		 case PREP_ERROR: return r;
		 default: break;
		}
	}

	return PREP_CHANGED;
	
}
