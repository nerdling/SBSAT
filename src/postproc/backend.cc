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
/********************************************************************
 *  backend.c (S.Weaver, J.Franco) Performs checking and reformating
 *  operations after a solution to a given problem is attained.
 *  Function finalCheck checks that the input lines are all not
 *  falsified by the returned assignment (if an assignment is
 *  returned).  Functions truthOf, getArg and opNestOf are used only
 *  by finalCheck.  Backend_Trace translates the values of internal 
 *  variables to values of original variables.
 ********************************************************************/

#include "ite.h"
#include "preprocess.h"
#include "postproc.h"

extern BDDNodeStruct **original_functions;
extern int original_numout;
extern int *arrSolution;
extern int gnMaxVbleIndex;
extern t_solution_info *solution_info;
extern t_solution_info *solution_info_head;

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
         default: fprintf (fout, "*%s", var); break;
         }
         fprintf(fout, " ");
	break;
  case 2:
        /* fancy/franco result format */
        fprintf (fout,  "%s\t%c(%d)\tval:", var, (negative?'-':' '), var_idx);
        switch (value) {
        case BOOL_TRUE : fprintf (fout, "T"); break;
        case BOOL_FALSE: fprintf (fout, "F"); break;
        default: fprintf (fout, "-"); break;
        }
        fprintf(fout, "\n");
	break;
  case 3:
        fprintf(fout, "%c", value==BOOL_TRUE?'+':value==BOOL_FALSE?'-':'?');
   break;
  default: break;
  }

}

void GetSolution(int oldnuminp, int nMaxVbleIndex) {
	for (int i = 1; i <= nMaxVbleIndex; i++)
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
					variablelist[variablelist[i].replace].true_false = 0;
				 break;
			 }
	  }
	for (int i = 1; i < oldnuminp+1; i++) {
		if(variablelist[variablelist[i].replace].true_false == 2) {
			variablelist[variablelist[i].replace].true_false = -1;
			variablelist[variablelist[i].replace].equalvars = 0;
		}
	}

	//for (int i = 1; i <= oldnuminp+1; i++) {
	//	fprintf(stderr, "%d = %d/%d\n", i, variablelist[i].true_false, variablelist[i].equalvars);
   //}
	
}

void ProcessSolution(int oldnuminp, int *original_variables) {
	int i;

	for (i = 1; i <= oldnuminp; i++) {
		if (variablelist[i].equalvars != 0) {
			if ((variablelist[i].true_false==3)) {
				variablelist[i].true_false = -1;
			} else if ((variablelist[abs(variablelist[i].equalvars)].true_false!=-1)) {
				if (variablelist[i].equalvars<0) {
					variablelist[i].true_false = 
					  variablelist[-(variablelist[i].equalvars)].true_false==0?1:0;
				} else {
					variablelist[i].true_false =
					  variablelist[variablelist[i].equalvars].true_false==0?0:1;
				}
			} else {
				if (variablelist[i].equalvars>0)
				  variablelist[i].equalvars = variablelist[variablelist[i].equalvars].equalvars;
				else
				  variablelist[i].equalvars = -variablelist[-variablelist[i].equalvars].equalvars;
			} 
		}
	}
	
	for (i = 1; i <= oldnuminp; i++)
	  {
		  if(original_variables[i]==-1)// && variablelist[i].true_false!=2)
			 original_variables[i] = variablelist[i].true_false;
		  if(original_variables[i] == 3 && variablelist[i].equalvars==0) {
			  original_variables[i] = 0;
			  variablelist[i].true_false = 0;
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
			functions[0] = ite_and(functions[0], ite_var(variables[x].num[y]));
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
		}
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
   for (int i = 1; i <= oldnuminp; i++)
	  {
        if (result_display_type == 4 && (i%20) == 0) fprintf(foutputfile, "\nv ");
		  ShowResultLine(foutputfile, s_name(i), i, 0, original_variables[i]);
	  }
   if (result_display_type == 4) fprintf(foutputfile, " 0");
	fprintf(foutputfile, "\n");
	//1 = True  0 = False  -1 = Don't Care
}

void 
Backend(int nMaxVbleIndex, int oldnuminp, int *original_variables) 
{
	int *old_orig_vars = (int *)calloc(oldnuminp+1, sizeof(int));
	varinfo *old_variablelist = (varinfo *)calloc(oldnuminp+1, sizeof(varinfo));
	for(int x = 0; x <= oldnuminp; x++) {
		old_orig_vars[x] = original_variables[x];
		old_variablelist[x].true_false = variablelist[x].true_false;
		old_variablelist[x].equalvars = variablelist[x].equalvars;
	}

	int num_sol = 1;
	for(solution_info = solution_info_head; solution_info!=NULL; solution_info = solution_info->next) {
		if (result_display_type && ite_counters[NUM_SOLUTIONS] > 1) 
		  fprintf(foutputfile, "\n// Solution #%d\n", num_sol++);
		for (int x = 0; x < original_numout; x++)
		  functions[x] = original_functions[x];
		nmbrFunctions = original_numout;
		
		for (int x = 0; x <= oldnuminp; x++) {
			original_variables[x] = old_orig_vars[x];
			variablelist[x].true_false = old_variablelist[x].true_false;
			variablelist[x].equalvars = old_variablelist[x].equalvars;
		}
		
		GetSolution(oldnuminp, nMaxVbleIndex);
	
		ProcessSolution(oldnuminp, original_variables);
		
		getExInferences(original_variables, oldnuminp);
		
      if (result_display_type == 4) fprintf(foutputfile, "v ");
      for (int i = 1; i <= oldnuminp; i++) {
         if (result_display_type == 4 && (i%20) == 0) fprintf(foutputfile, "\nv ");
         ShowResultLine(foutputfile, s_name(i), i, 0, original_variables[i]);
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
  int oldnuminp = numinp;
  int *original_variables;

  /* 
   * original_variables
   */

  ITE_NEW_CATCH(
  original_variables = new int[numinp + 1],
  "input variables");

  for (int x = 0; x <= numinp; x++)
     original_variables[x] = -1;

  nmbrFunctions = original_numout;

  Backend(numinp, oldnuminp, original_variables);

	while(solution_info_head!=NULL) {
		solution_info = solution_info_head;
		solution_info_head = solution_info_head->next;
		free(solution_info->arrElts);
		free(solution_info);
	}
	solution_info = NULL;
	delete [] original_variables;
	original_variables = NULL;
	delete [] variablelist;
	variablelist = NULL;
	delete [] original_functions;
	original_functions = NULL;
	ite_free((void **)&length);
	//delete [] length;
	length = NULL;
}

void
Verify_NoSolver()
{
  int oldnuminp = numinp;
  int *original_variables;

  /* 
   * original_variables
   */
	
  ITE_NEW_CATCH(
  original_variables = new int[numinp + 1],
  "input variables");

  for (int x = 0; x <= numinp; x++)
     original_variables[x] = -1;

  for (int x = 0; x < original_numout; x++)
    functions[x] = original_functions[x];
  nmbrFunctions = original_numout;

  if (result_display_type) {
     Backend_NoSolver(oldnuminp, original_variables);
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
	delete [] variablelist;
	variablelist = NULL;
	delete [] original_functions;
	original_functions = NULL;

	ite_free((void **)&length);
	//delete [] length;
	length = NULL;
}

