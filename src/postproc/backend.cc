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

extern char tracer_tmp_filename[256];

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
  default: break;
  }

}

void GetSolution(int oldnuminp, int nMaxVbleIndex, t_solution_info *solution_info) {
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
	
	delete [] length;
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
Backend_CNF_NoSolver (int oldnuminp, int *original_variables)
{
	ProcessSolution(oldnuminp, original_variables);
	
	getExInferences(original_variables, oldnuminp);
	
	for (int i = 1; i <= oldnuminp; i++)
	  {
		  ShowResultLine(foutputfile, NULL, i, 0, original_variables[i]);
	  }
	fprintf(foutputfile, "\n");
	//1 = True  0 = False  -1 = Don't Care
}

void Backend_CNF (int nMaxVbleIndex, int oldnuminp, int *original_variables) {
	int *old_orig_vars = (int *)calloc(oldnuminp+1, sizeof(int));
	varinfo *old_variablelist = (varinfo *)calloc(oldnuminp+1, sizeof(varinfo));
	for(int x = 0; x <= oldnuminp; x++) {
		old_orig_vars[x] = original_variables[x];
		old_variablelist[x].true_false = variablelist[x].true_false;
		old_variablelist[x].equalvars = variablelist[x].equalvars;
	}

	int num_sol = 1;
	for(solution_info = solution_info_head; solution_info!=NULL; solution_info = solution_info->next) {
		if (result_display_type) 
		  fprintf(foutputfile, "\n// Solution #%d\n", num_sol++);
		for (int x = 0; x < original_numout; x++)
		  functions[x] = original_functions[x];
		nmbrFunctions = original_numout;
		
		for (int x = 0; x <= oldnuminp; x++) {
			original_variables[x] = old_orig_vars[x];
			variablelist[x].true_false = old_variablelist[x].true_false;
			variablelist[x].equalvars = old_variablelist[x].equalvars;
		}
		
		GetSolution(oldnuminp, nMaxVbleIndex, solution_info);
	
		ProcessSolution(oldnuminp, original_variables);
		
		getExInferences(original_variables, oldnuminp);
		
		for (int i = 1; i <= oldnuminp; i++) {
			ShowResultLine(foutputfile, NULL, i, 0, original_variables[i]);
		}
		fprintf(foutputfile, "\n");
	}	
	free(old_orig_vars);
	free(old_variablelist);
	//1 = True  0 = False  -1 = Don't Care
}

void
Backend_Trace_NoSolver (int oldnuminp, int *original_variables, Tracer * tracer)
{
  ProcessSolution(oldnuminp, original_variables);

  getExInferences(original_variables, oldnuminp);	

  if (tracer)  
     tracer->getSymbols (original_variables, oldnuminp);
  else
     d3_printf1("no tracer -- skipping getSymbols\n");
}

void Backend_Trace (int nMaxVbleIndex, int oldnuminp,
						  int *original_variables, Tracer * tracer) {
	int *old_orig_vars = (int *)calloc(oldnuminp+1, sizeof(int));
	varinfo *old_variablelist = (varinfo *)calloc(oldnuminp+1, sizeof(varinfo));
	for(int x = 0; x <= oldnuminp; x++) {
		old_orig_vars[x] = original_variables[x];
		old_variablelist[x].true_false = variablelist[x].true_false;
		old_variablelist[x].equalvars = variablelist[x].equalvars;
	}

	int num_sol = 1;
	for(solution_info = solution_info_head; solution_info!=NULL; solution_info = solution_info->next) {
		if (result_display_type) 
		  fprintf(foutputfile, "\n// Solution #%d\n", num_sol++);

		for (int x = 0; x < original_numout; x++)
		  functions[x] = original_functions[x];
		nmbrFunctions = original_numout;
		
		for (int x = 0; x <= oldnuminp; x++) {
			original_variables[x] = old_orig_vars[x];
			variablelist[x].true_false = old_variablelist[x].true_false;
			variablelist[x].equalvars = old_variablelist[x].equalvars;
		}
		
		GetSolution(oldnuminp, nMaxVbleIndex, solution_info);
	
		ProcessSolution(oldnuminp, original_variables);
		
		getExInferences(original_variables, oldnuminp);
		
		//We might have trouble calling tracer multiple times...
      if (tracer) {
         tracer->getSymbols (original_variables, oldnuminp);
         finalCheck(tracer, original_variables);
      }
	}
	free(old_orig_vars);
   free(old_variablelist);
	//1 = True  0 = False  -1 = Don't Care
}

// Input: expression such as "a,b, or(c,d,and(e,f)), g)" and a pointer to
//   one of the comma separated arguments.
// Output: the text up to the next appropriate comma (into arg) and a pointer
//   to the beginning of the next argument.
char *
getArg (char *text, char *arg)
{
	int nparen = 0;
	while (*text == ' ')
	  text++;
	if (*text == 0 || *text == ';' || *text == '\n')
	  {
		  delete arg;
		  *arg = 0;
		  return NULL;
	  }
	while (1)
	  {
		  if (*text == ',')
			 {
				 if (nparen <= 0)
					{
						delete arg;
						*arg = 0;
						text++;
						while (*text == ' ')
						  text++;
						return text;
					}
				 else
					{
						*arg++ = *text++;
					}
			 }
		  else if (*text == ';' || *text == '\n')
			 {
				 if (nparen <= 0)
					{
						delete arg;
						*arg = 0;
						text++;
						while (*text == ' ')
						  text++;
						return text;
					}
				 else
					{
						fprintf (stderr,
									"getArg: semicolon seen in nesting of parens\n");
						exit (1);
					}
			 }
		  else if (*text == '(')
			 {
				 *arg++ = *text++;
				 nparen++;
			 }
		  else if (*text == ')')
			 {
				 if (nparen <= 0)
					{
						delete arg;
						*arg = 0;
						text++;
						while (*text == ' ')
						  text++;
						return text;
					}
				 else
					{
						*arg++ = *text++;
						nparen--;
					}
			 }
		  else
			 {
				 *arg++ = *text++;
			 }
	  }
}

// Input: equ expression such as "x = and(or(a,b),c);" in buffer
// Output: "and(...)"
char *
  opNestOf (char *buffer, char *arg)
{
	char *args;
	
	for (; *buffer != '='; buffer++);
	buffer++;
	for (; *buffer == ' '; buffer++);
	args = buffer;
	while (*buffer != '(')
	  *arg++ = *buffer++;
	*arg++ = *buffer++;
	int nparen = 1;
	while (1)
	  {
		  if (*buffer == '(')
			 nparen++;
		  if (*buffer == ')')
			 nparen--;
		  if (nparen <= 0)
			 {
				 //delete arg;
				 *arg = 0;
				 return args;
			 }
		  *arg++ = *buffer++;
	  }
}

// returns 1 if true, -1 if don't care, 0 if false
int truthOf (char *buffer, Tracer *tracer, int *original_variables)
{
   int left_v, rght_v;
   Integer *equ_var;
   char *first, /**second,*/ *ptr, *arg = (char *) calloc (1, 1024);
   Hashtable *symbols = tracer->getHashTable ();
   
   StringTokenizer *s = new StringTokenizer ("new", " ");
   s->renewTokenizer (buffer, " ");
   if (!s->hasMoreTokens ()){
		free(arg);
		delete s;
      return 1;
	}
   first = s->nextToken ();
   if (strlen(first) == 9 && !strncmp (first, "are_equal", 9)) {
      int value = -1;
      for (ptr = buffer; *ptr != '('; ptr++);	// points to first argument
      ptr++;
      while ((ptr = getArg (ptr, arg)) != NULL) {
			rght_v = truthOf (arg, tracer, original_variables);
			if (value != -1 && rght_v != -1 && value != rght_v){
				if(arg!=NULL) free(arg);
				delete s;				  
				return 0;
			}
			if (value > -1)
			  value = rght_v;
      }
		if(arg!=NULL) free(arg);
		delete s;
      return value;
   } else if (strlen(first) == 3 && !strncmp (first, "not", 3)) {
      for (ptr = buffer; *ptr != '('; ptr++);	// points to first argument
      ptr++;
      ptr = getArg (ptr, arg);
      if (arg == NULL) {
			delete s;
			return -1;
		}
      rght_v = truthOf (arg, tracer, original_variables);
      if (rght_v == -1) {
			if(arg!=NULL) free(arg);
			delete s;
			return -1;
		}
      if (rght_v == 0) {
			if(arg!=NULL) free(arg);
			delete s;
			return 1;
		} else {
			if(arg!=NULL) free(arg);
			delete s;
		  return -1;
		}
   } else if (strlen(first) == 3 && !strncmp (first, "and", 3)) {
      int value = -1;
      for (ptr = buffer; *ptr != '('; ptr++);	// points to first argument
      ptr++;
      while ((ptr = getArg (ptr, arg))) {
			if (arg == NULL)
			  break;
			rght_v = truthOf (arg, tracer, original_variables);
			value = rght_v;
			if (rght_v == 0) {
				if(arg!=NULL) free(arg);
				delete s;
				return 0;
			}
			if (ptr == NULL)
			  break;
      }
		if(arg!=NULL) free(arg);
		delete s;
      return value;
   } else if (strlen(first) == 2 && !strncmp (first, "or", 2)) {
      int value = -1;
      for (ptr = buffer; *ptr != '('; ptr++);	// points to first argument
      ptr++;
      while ((ptr = getArg (ptr, arg))) {
			if (arg == NULL)
			  break;
			rght_v = truthOf (arg, tracer, original_variables);
			value = rght_v;
			if (rght_v == 1) {
				if(arg!=NULL) free(arg);
				delete s;
				return 1;
			}
			if (ptr == NULL)
			  break;
      }
		if(arg!=NULL) free(arg);
		delete s;
      return value;
   } else if (strlen(first) == 3 && !strncmp (first, "ite", 3)) {
      int ite, ife, els;
      for (ptr = buffer; *ptr != '('; ptr++);	// points to first argument
      ptr++;
      ptr = getArg (ptr, arg);
      if (arg == NULL)
		  ite = -1;
      else
		  ite = truthOf (arg, tracer, original_variables);
      if (ptr == NULL) {
			fprintf (stderr, "backend: ite has wrong number of variables\n");
			exit (1);
      }
      ptr = getArg (ptr, arg);
      if (arg == NULL)
		  ife = -1;
      else
		  ife = truthOf (arg, tracer, original_variables);
      if (ptr == NULL) {
			fprintf (stderr, "backend: ite has wrong number of variables\n");
			exit (1);
      }
      ptr = getArg (ptr, arg);
      if (arg == NULL)
		  els = -1;
      else
		  els = truthOf (arg, tracer, original_variables);
      if (ptr == NULL) {
			fprintf (stderr, "backend: ite has wrong number of variables\n");
			exit (1);
      }
      if (ife == 1 && els == 1) {
			if(arg!=NULL) free(arg);
			delete s;
			return 1;
		}
      if (ife == 0 && els == 0) {
			if(arg!=NULL) free(arg);
			delete s;
			return 0;
		}
      if (ite == 1) {
			if(arg!=NULL) free(arg);
			delete s;
			return ife;
		}
      if (ite == 0) {
			if(arg!=NULL) free(arg);
			delete s;
			return els;
		}
		if(arg!=NULL) free(arg);
		delete s;
      return -1;
   } else if (!s->hasMoreTokens ()) {
		// Must be a variable
      if ((equ_var = (Integer *) symbols->get (first)) == NULL) {
			// object is not in hashtable
			if(arg!=NULL) free(arg);
			delete s;
			return -1;
      }
      int h;
      int v = equ_var->intValue ();
      if (v == abs (v))
		  h = 1;
      else
		  h = 0;
      if (original_variables[abs (v)] == h)
		  left_v = 1;
      else if (original_variables[abs (v)] < 0)
		  left_v = -1;
      else
		  left_v = 0;
		if(arg!=NULL) free(arg);
		delete s;
      return left_v;
   } else if (*(/*second =*/ s->nextToken ()) == '=') {
      if ((equ_var = (Integer *) symbols->get (first)) == NULL) {
			fprintf(stderr, "\nError:object is not in hashtable\n");
			exit (1);
			if(arg!=NULL) free(arg);
			delete s;
			return -1;
      }
      int h;
      int v = equ_var->intValue ();
      if (v == abs (v))
		  h = 1;
      else
		  h = 0;
      if (original_variables[abs (v)] == h)
		  left_v = 1;
      else if (original_variables[abs (v)] < 0)
		  left_v = -1;
      else
		  left_v = 0;
      int rght_v =
		  truthOf (opNestOf (buffer, arg), tracer, original_variables);
      if (left_v == rght_v) {
			if(arg!=NULL) free(arg);
			delete s;
			return 1;
		}
      if (left_v == -1) {
			if(arg!=NULL) free(arg);
			delete s;
			return 1;
		}
      if (rght_v == -1) {
			if(arg!=NULL) free(arg);
			delete s;
			return 1;
		}
		if(arg!=NULL) free(arg);
		delete s;
      return 0;
   } else {
      // Probably a module function - not handled yet
		if(arg!=NULL) free(arg);
		delete s;
      return -1;
   }
}

void finalCheck (Tracer * tracer, int *original_variables)
{
   int fails = 0;
   bool flag = false;
   FILE *fd;
   StringTokenizer *t = new StringTokenizer ("new", " ");
   char *first, buffer[4096];
	
   d2_printf1 ("Checking returned result... ");
   if ((fd = fopen (tracer_tmp_filename, "rb")) == NULL) {
      dE_printf2 ("Whoops! %s not found.\n", tracer_tmp_filename);
      exit (1);
   }
	
   while (fgets (buffer, 2047, fd)) {
		
      t->renewTokenizer (buffer, " =(,;");
      first = t->nextToken ();
      
      if (!strncmp (first, "STRUCTURE", 9)) {
			flag = true;
			continue;
      }
      if (!flag)
		  continue;
      if (!strncmp (first, "ENDMODULE", 9)) {
			if (fails == 0) {
				d2_printf1 ("Solution verified\n");
			} else {
				dE_printf2 ("Solution Not Verified! %d fails were found\n", fails);
				exit(1);
			}
			fclose(fd);
			delete t;
			return;
      }
      if (!strncmp (first, "&&begingroup", 12))
		  continue;
      if (!strncmp (first, "&&endgroup", 10))
		  continue;
      if (!truthOf (buffer, tracer, original_variables)) {
			dE_printf2 ("Fail: %s\n", buffer);
			fails++;
      }
   }
	fclose(fd);
	delete t;
	return;
}

void
Verify_Solver(Tracer *tracer)
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

  if (formatin == 't') 
   {
     Backend_Trace(numinp, oldnuminp, original_variables, tracer);
   } 
  else 
   {
     Backend_CNF(numinp, oldnuminp, original_variables);
   }

	while(solution_info_head!=NULL) {
		solution_info = solution_info_head;
		solution_info_head = solution_info_head->next;
		free(solution_info->arrElts);
		free(solution_info);
	}
	solution_info = NULL;
        delete [] original_variables;
	original_variables = NULL;
}

void
Verify_NoSolver(Tracer *tracer)
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

  if (formatin == 't')
   {
     if (tracer) {
        Backend_Trace_NoSolver(oldnuminp, original_variables, tracer);
        finalCheck(tracer, original_variables);
     }
   }
  else
   {
     Backend_CNF_NoSolver(oldnuminp, original_variables);
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
	delete [] length;
	length = NULL;
}

