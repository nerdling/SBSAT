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
#include "sbsat_postproc.h"
#include "sbsat_solver.h"

/*
extern BDDNodeStruct **original_functions;
extern int original_numout;
extern int *arrSolution;
extern int gnMaxVbleIndex;
extern t_solution_info *solution_info;
extern t_solution_info *solution_info_head;
*/

void
ShowResultLine(FILE *fout, char *var, int var_idx, int negative, int value)
{
  char var_str[32];

  if (negative==1)
  {
	 if (value==BOOL_FALSE) value = BOOL_TRUE; else
         if (value==BOOL_TRUE ) value = BOOL_FALSE;
  }

  if (var == NULL) {
	  var = var_str;
	  sprintf(var, "%d", var_idx);
  }

	switch (result_display_type) {
	 case 4:
	 case 1:
		/* raw result format */
		switch (value) {
		 case BOOL_TRUE : fprintf (fout,  "%s", var); break;
		 case BOOL_FALSE: fprintf (fout, "-%s", var); break;
		 case -1: fprintf (fout, "*%s", var); break;
		 default: fprintf (fout, negative?"%s=%s":"%s=-%s", var, s_name(value)); break;
		}
		fprintf(fout, " ");
		break;
	 case 2:
		/* fancy/franco result format */
		fprintf (fout,  "%s\t%c(%d)\tval:", var, (negative?'-':' '), var_idx);
		switch (value) {
		 case BOOL_TRUE : fprintf (fout, "T"); break;
		 case BOOL_FALSE: fprintf (fout, "F"); break;
		 case -1: fprintf(fout, "-"); break;
		 default: fprintf (fout, "%d", abs(value)); break;
		}
		fprintf(fout, "\n");
		break;
	 case 3:
		if(value==BOOL_TRUE || value == BOOL_FALSE || value==-1)
		  fprintf(fout, "%c", value==BOOL_TRUE?'+':value==BOOL_FALSE?'-':'?');
		else fprintf(fout, "%d", negative?-value:value);
		break;
	 case 5:
		fprintf(fout, "%c", value==BOOL_TRUE?'1':value==BOOL_FALSE?'0':'?');
		break;
	 default: break;
	}
}

void GetSolution(int oldnuminp) {
	for (int i = 1; i <= oldnuminp; i++)
	  {
		  //if(variablelist[variablelist[i].replace].true_false == 2
		  //  || variablelist[variablelist[i].replace].true_false == 3)
		  //  continue;
		  if(variablelist[variablelist[i].replace].true_false !=-1) continue;

		  switch (solution_info->arrElts[i])
			 {
			  case BOOL_TRUE:
				 variablelist[variablelist[i].replace].true_false = 1;
				 break;
				 
			  case BOOL_FALSE:
				 variablelist[variablelist[i].replace].true_false = 0;
				 break;
				 
			  default:
				 if(variablelist[variablelist[i].replace].true_false == -1)
					variablelist[variablelist[i].replace].true_false = -1;
				 break;
			 }
	  }
	for (int i = 1; i <= numinp; i++) {
		if(variablelist[variablelist[i].replace].true_false == 2) {
			variablelist[variablelist[i].replace].true_false = -1;
			//variablelist[variablelist[i].replace].equalvars = 0;
		}
	}

	//for (int i = 1; i <= oldnuminp; i++) {
	//	fprintf(stderr, "%d = %d/%d\n", i, variablelist[i].true_false, variablelist[i].equalvars);
   //}
	
}

void ProcessSolution(int oldnuminp, int *original_variables) {
	int i;

	for (i = 1; i <= numinp; i++) {
		if (variablelist[i].equalvars != 0) {
			if ((variablelist[i].true_false==3)) {
				variablelist[i].true_false = -1;
			} else if ((variablelist[abs(variablelist[i].equalvars)].true_false!=-1)) {
				if (variablelist[i].equalvars<0) {
					variablelist[i].true_false = 
					  variablelist[-(variablelist[i].equalvars)].true_false==0?1:0;
				} else if((variablelist[abs(variablelist[i].equalvars)].true_false!=2)) {
					variablelist[i].true_false =
					  variablelist[variablelist[i].equalvars].true_false==0?0:1;
				} else {
					//nothing
				}
			} else {
				if (variablelist[i].equalvars>0)
				  variablelist[i].equalvars = variablelist[variablelist[i].equalvars].equalvars;
				else
				  variablelist[i].equalvars = -variablelist[-variablelist[i].equalvars].equalvars;
			} 
		}
	}
	
	for (i = 1; i <= numinp; i++)
	  {
		  if(original_variables[i]==-1)// && variablelist[i].true_false!=2)
			 original_variables[i] = variablelist[i].true_false;
		  if(original_variables[i] == 3 && variablelist[i].equalvars==0) {
			  original_variables[i] = -1;
			  variablelist[i].true_false = -1;
		  }
		  //fprintf(stderr, "%d = %d/%d ", i, variablelist[i].true_false, variablelist[i].equalvars);
		  //fprintf(stderr, "%d = %d\n ", i, original_variables[i]);
		  //fprintf(stderr, "%d = %d\n", i, arrSolution[i]);
	  }
	//1 = True  0 = False  -1 = Don't Care
}

void getExInferences(int *original_variables, int oldnuminp) {

	int numBDDs = nmbrFunctions;
	DO_INFERENCES = 0; //don't call the other routine inside of ReBuildBDDx()

	ite_free((void **)&length);
	//delete [] length;
	length = NULL;

	int ret = Init_Preprocessing();
	if(ret == TRIV_UNSAT) {
		fprintf(stderr, "\nError verifying solution\n");
		exit(0);
	}

	ret = Do_Apply_Inferences_backend();
	if(ret == TRIV_UNSAT) {
		fprintf(stderr, "\nError verifying solution\n");
		exit(0);
	}
	
	ret = CreateInferences();
	if(ret == TRIV_UNSAT) {
		fprintf(stderr, "\nError verifying solution\n");
		exit(0);
	}
	
	//for(int z = 0; z < 100; z++)
	ret = Do_Apply_Inferences_backend();
	if(ret == TRIV_UNSAT) {
		fprintf(stderr, "\nError verifying solution\n");
		exit(0);
	}
	
	for (int x = 0; x < numBDDs; x++) {
		if(functions[x] == true_ptr) continue;
		for(int y = 0; y < length[x]; y++) {
			//fprintf(stderr, "(%d)", variables[x].num[y]);
			BDDNode *inferBDD = ite_var(variables[x].num[y]);
			int bdd_length = 0;
			int *bdd_vars = NULL;
			DO_INFERENCES = 1;
			switch (Rebuild_BDD(inferBDD, &bdd_length, bdd_vars)) {
			 case TRIV_UNSAT:
			 case TRIV_SAT:
			 case PREP_ERROR: fprintf(stderr, "\nError verifying solution\n");
				               exit(0);
			 default: break;
			}
			delete [] bdd_vars;
			bdd_vars = NULL;
			ret = PREP_CHANGED;
			switch (Do_Apply_Inferences()) {
			 case TRIV_UNSAT:
			 case TRIV_SAT:
			 case PREP_ERROR: fprintf(stderr, "\nError verifying solution\n");
				               exit(0);
			 default: break;
			}
			x = -1;
			break;//continue;
			
/*			functions[0] = ite_and(functions[0], ite_var(variables[x].num[y]));
			ret = Rebuild_BDDx(0);
			if(ret == TRIV_UNSAT) {
				fprintf(stderr, "\nError verifying solution\n");
				exit(0);
			}
			ret = Do_Apply_Inferences_backend();
			if(ret == TRIV_UNSAT) {
				fprintf(stderr, "\nError verifying solution\n");
				exit(0);
			}
			x = -1;
			break;
 */
		}
	}
	

	//This loop sets unknown variables and takes care
	//of subsequent equivalences
	//Then I can remove the equivalence printing in the result.

	for (int x = 0; x <= numinp; x++) {
		if(variablelist[x].true_false == -1) {
			if (variablelist[x].equalvars>0)
			  variablelist[x].true_false = variablelist[variablelist[x].equalvars].true_false;
			else if(variablelist[x].equalvars<0)
			  variablelist[x].true_false = 1-variablelist[-variablelist[x].equalvars].true_false;
		} else if(variablelist[x].true_false == 2) {
			if(variablelist[x].equalvars == 0) {
				variablelist[x].true_false = 1;
			} else {
				if (variablelist[x].equalvars>0)
				  variablelist[x].true_false = variablelist[variablelist[x].equalvars].true_false;
				else
				  variablelist[x].true_false = 1-variablelist[-variablelist[x].equalvars].true_false;
			}
		}
		if(original_variables[x] == 2) original_variables[x] = -1;
	}
	
	ProcessSolution(oldnuminp, original_variables);
	
	Finish_Preprocessing();
}


void
Backend_NoSolver (int oldnuminp, int *original_variables)
{
	ProcessSolution(oldnuminp, original_variables);
	
	getExInferences(original_variables, oldnuminp);
	
   if (result_display_type == 4) fprintf(foutputfile, "v ");

	for (int i = 1; i <= numinp; i++) {
		if (result_display_type == 4 && (i%20) == 0) fprintf(foutputfile, "\nv ");
		int negate = 0;
		if(original_variables[i] == 2) original_variables[i] = -1;
		if(original_variables[i] == -1 && variablelist[i].equalvars>0) {
			original_variables[i] = variablelist[i].equalvars;
		} else if(original_variables[i] == -1 && variablelist[i].equalvars<0) {
			original_variables[i] = variablelist[i].equalvars;
			negate = 1;
		}
		ShowResultLine(foutputfile, s_name(i), i, negate, original_variables[i]);
	}
   if (result_display_type == 4) fprintf(foutputfile, " 0");
	fprintf(foutputfile, "\n");
	//1 = True  0 = False  -1 = Don't Care
}


void 
Backend(int oldnuminp, int *original_variables)
{
	int *old_orig_vars = (int *)calloc(numinp+1, sizeof(int));
	varinfo *old_variablelist = (varinfo *)calloc(numinp+1, sizeof(varinfo));
	for(int x = 0; x <= numinp; x++) {
		old_orig_vars[x] = original_variables[x];
		old_variablelist[x].true_false = variablelist[x].true_false;
		old_variablelist[x].equalvars = variablelist[x].equalvars;
	}

	int num_sol = 1;
	for(solution_info = solution_info_head; solution_info!=NULL; solution_info = solution_info->next) {
		//if (result_display_type && ite_counters[NUM_SOLUTIONS] > 1) 
		  //fprintf(foutputfile, "\n// Solution #%d\n", num_sol++);
		for (int x = 0; x < original_numout; x++) {
			functions[x] = original_functions[x];
			functionType[x] = UNSURE;
		}
		nmbrFunctions = original_numout;
		
		for (int x = 0; x <= numinp; x++) {
			original_variables[x] = old_orig_vars[x];
			variablelist[x].true_false = old_variablelist[x].true_false;
			variablelist[x].equalvars = old_variablelist[x].equalvars;
		}
		
		GetSolution(oldnuminp);
	
		ProcessSolution(oldnuminp, original_variables);
		
		getExInferences(original_variables, oldnuminp);
		
      if (result_display_type == 4) fprintf(foutputfile, "v ");
      for (int i = 1; i <= numinp; i++) {
         if (result_display_type == 4 && (i%20) == 0) fprintf(foutputfile, "\nv ");
			int negate = 0;
			if(original_variables[i] == 2) original_variables[i] = -1;
			if(original_variables[i] == -1 && variablelist[i].equalvars>0) {
				original_variables[i] = variablelist[i].equalvars;
			} else if(original_variables[i] == -1 && variablelist[i].equalvars<0) {
				original_variables[i] = variablelist[i].equalvars;
				negate = 1;
			} 
			ShowResultLine(foutputfile, s_name(i), i, negate, original_variables[i]);
		}
      if (result_display_type == 4) fprintf(foutputfile, " 0");
      fprintf(foutputfile, "\n");
	}	
	free(old_orig_vars);
	free(old_variablelist);
	//1 = True  0 = False  -1 = Don't Care
}

void
Verify_Solver()
{

	for (int x = 0; x < original_numout; x++) {
		functions[x] = original_functions[x];
		functionType[x] = UNSURE;
	}
	nmbrFunctions = original_numout;
	
	int oldnuminp = numinp;
	numinp = getNuminp();
	oldnuminp = oldnuminp<numinp?oldnuminp:numinp;
	int *original_variables;
	
	ITE_NEW_CATCH(
	  original_variables = new int[numinp + 1],
	  "input variables");
	
	for (int x = 0; x <= numinp; x++)
     original_variables[x] = -1;

	nmbrFunctions = original_numout;

	if(result_display_type) {
		Backend(oldnuminp, original_variables);
	}

	while(solution_info_head!=NULL) {
		solution_info = solution_info_head;
		solution_info_head = solution_info_head->next;
		delete [] solution_info->arrElts;
		ite_free((void**)&solution_info);
	}
	solution_info = NULL;
	delete [] original_variables;
	original_variables = NULL;
	ite_free((void **)&variablelist);
	variablelist = NULL;
	ite_free((void **)&original_functions);
	original_functions = NULL;
	ite_free((void **)&length);
	//delete [] length;
	length = NULL;
}

void
Verify_NoSolver()
{

	for (int x = 0; x < original_numout; x++) {
		functions[x] = original_functions[x];
		functionType[x] = UNSURE;
	}
	nmbrFunctions = original_numout;
	
	numinp = getNuminp();
	
	int *original_variables;
	
	ITE_NEW_CATCH(
	  original_variables = new int[numinp + 1],
	  "input variables");
	
	for (int x = 0; x <= numinp; x++)
     original_variables[x] = -1;

	if (result_display_type) {
     Backend_NoSolver(numinp, original_variables);
	}
	
	while(solution_info_head!=NULL) {
		solution_info = solution_info_head;
		solution_info_head = solution_info_head->next;
		delete solution_info->arrElts;
		delete solution_info;
	}
	solution_info = NULL;
   delete [] original_variables;
	original_variables = NULL;
	ite_free((void**)&variablelist);
   ite_free((void**)&original_functions);
	ite_free((void **)&length);
	//delete [] length;
	length = NULL;
}

