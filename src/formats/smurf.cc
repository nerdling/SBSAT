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
#include "sbsat_formats.h"

void bcount (char *p, int i) {
	int flip;
	for (int x = i; x > 0; x--) {
		flip = 1;
      for (int y = (x - 1); y >= 0; y--) {
			if (p[y] == '0') {
				flip = 0;
				break;
			}
		}
      if (flip) {
			if (p[x] == '0')
			  p[x] = '1';
			else
			  p[x] = '0';
		}
	}
	if (p[0] == '1')
	  p[0] = '0';
	else
	  p[0] = '1';
}

int getTruth (int *num, char *p, int y, BDDNode * ptr) {
	for (int x = y-1; x >= 0; x--) {
      while ((x >= 0) && (num[x] != ptr->variable)) {
			x--;
		}
      if (p[x] == '1')
		  ptr = ptr->thenCase;
      else
		  ptr = ptr->elseCase;
	}
	if (ptr == true_ptr)
	  return 1;
	else
	  return 0;
}

int getEquiv(int v, int *variables, int length) {
	int x = equalityVble[v];
	for(int i = 0; i < length; i++) {
	  if(abs(x) == variables[i]) return i;
	}
	dE_printf1("\nEquivilance variable not found in the variable list!\n");
	exit(0);
   return 0;
}

int *getANDLiterals(int v, int *variables, int length) {
	int *literals = new int[length];
	if (functionType[v] == AND || functionType[v] == OR) {
		int validated = 0;
		int torf;
		if(functionType[v]==AND) torf = equalityVble[v] > 0 ? 1 : 0;
		else torf = equalityVble[v] > 0 ? 0 : 1;
		BDDNode *true_and = set_variable (functions[v], abs(equalityVble[v]), torf);
		for (int x = 0; x < length; x++) {
			if (variables[x] == abs(equalityVble[v])) {
				literals[x] = torf > 0 ? abs(equalityVble[v]) : -abs(equalityVble[v]);
				validated = 1;
			} else if (set_variable (true_and, variables[x], 1) == false_ptr)
			  literals[x] = -variables[x];
			else if (set_variable (true_and, variables[x], 0) == false_ptr)
			  literals[x] = variables[x];
			else {
				dE_printf2("\nERROR: Function %d is incorrectly labeled type AND= or OR=\n", v);
				exit(1);
			}
		}
		if (validated == 0)	{
			dE_printf2("\nERROR: Equivalence variable not correct in function %d!\n", v);
			exit(1);
		}
	} else {
		fprintf(stderr, "\nFunction %d is not of type AND= or OR=\n", v);
		exit(1);
	}
	return literals;
}

void BDD_to_Smurfs () {
	char *p = (char *)calloc(8192, sizeof(char));
	store * integers;
	int *tempint=NULL;
   int tempint_max = 0;
	int v, y, i;
	int x;
	integers = (store *) calloc (nmbrFunctions + 1, sizeof (store));
	numinp = 0;

	int use_symtable = sym_all_int();
	//if(!use_symtable) {
   //  print_cnf_symtable();	
   //}

	for (x = 0; x < nmbrFunctions; x++) {
		if (functions[x] == false_ptr) {
			fprintf (foutputfile, "UNSATISFIABLE");
			return;
		}
		if (functions[x] == true_ptr)
		  continue;
		y = 0;
		unravelBDD (&y, &tempint_max, &tempint, functions[x]);
		
		//Sort the integers
		qsort (tempint, y, sizeof (int), compfunc);
		if (numinp < tempint[y - 1])
		  numinp = tempint[y - 1];
		
		integers[x].num = (int *) calloc (y + 1, sizeof (int));
		for (i = 0; i < y; i++)
		  integers[x].num[i] = tempint[i];
		integers[x].num[y] = 0;
	}
	fprintf (foutputfile, "%ld # Number of Input Variables\n", numinp);
	fprintf (foutputfile, "%d # Number of Output Variables\n", nmbrFunctions);
	for (x = 0; x < nmbrFunctions; x++)
	  fprintf (foutputfile, "1");
	fprintf (foutputfile, " # Output Vector\n");
	for (v = 0; v < nmbrFunctions; v++) {
		fprintf (foutputfile, "#\n%ld\n", v);
		if (functions[v] == true_ptr) {
			fprintf (foutputfile, "-1\n1\n");
			continue;
		}
		for (x = 0; integers[v].num[x] != 0; x++) {
			p[x] = '0';
			fprintf (foutputfile, "%d ", use_symtable?atoi(getsym_i(integers[v].num[x])->name):integers[v].num[x]);
		}
		fprintf (foutputfile, "-1\n");
		if (functionType[v] == PLAINOR) {
			fprintf(foutputfile, "plainor ");
			BDDNode *p_or = functions[v];
			for (x = 0; x < integers[v].num[x] != 0; x++) {
				if (set_variable (p_or, integers[v].num[x], 1) == true_ptr)
				  fprintf (foutputfile, "1");
				else if (set_variable (p_or, integers[v].num[x], 0) == true_ptr)
				  fprintf (foutputfile, "0");
				else {
					fprintf (stderr, "Error! %d", functionType[v]);
					fprintf (stderr, "\n");
					printBDDerr (functions[v]);
					fprintf (stderr, "\n");
					exit (1);
				}
			}
		} else if (functionType[v] == AND || functionType[v] == OR) {
			int validated = 0;
			int torf = equalityVble[v] > 0 ? 1 : 0;
			if ((torf == 1 && functionType[v] == AND) || 
				 (torf == 0 && functionType[v] == OR)) {
				fprintf (foutputfile, "and= ");
				BDDNode * true_and =
				  set_variable (functions[v], abs (equalityVble[v]), 1);
				for (x = 0; x < integers[v].num[x] != 0; x++) {
					if (integers[v].num[x] == abs (equalityVble[v])) {
						fprintf (foutputfile, "3");
						validated = 1;
						continue;
					}
					if (set_variable (true_and, integers[v].num[x], 0) == false_ptr)
					  fprintf (foutputfile, "1");
					else if (set_variable (true_and, integers[v].num[x], 1) == false_ptr)
					  fprintf (foutputfile, "0");
					else {
						fprintf (stderr, "Error! %d", functionType[v]);
						fprintf (stderr, "\n");
						printBDDerr (true_and);
						fprintf (stderr, "\n");
						printBDDerr (functions[v]);
						fprintf (stderr, "\n");
						exit (1);
					}
				}
				if (validated == 0) {
					fprintf (stderr, "\nERROR: Equivalence Variable Not Correct!\n");
					exit (1);
				}
			} else {
				fprintf (foutputfile, "or= ");
				BDDNode * true_or = set_variable (functions[v], abs (equalityVble[v]), 1);
				for (x = 0; x < integers[v].num[x] != 0; x++) {
					if (integers[v].num[x] == abs (equalityVble[v])) {
						fprintf (foutputfile, "3");
						continue;
					}
					if (set_variable (true_or, integers[v].num[x], 1) == true_ptr)
					  fprintf (foutputfile, "1");
					else if (set_variable (true_or, integers[v].num[x], 0) == true_ptr)
					  fprintf (foutputfile, "0");
					else {
						fprintf (stderr, "Error! %d", functionType[v]);
						fprintf (stderr, "\n");
						printBDDerr (true_or);
						fprintf (stderr, "\n");
						printBDDerr (functions[v]);
						fprintf (stderr, "\n");
						exit (1);
					}
				}
			}
		} else if (functionType[v] == PLAINXOR) {
			fprintf(foutputfile, "xor ");
			BDDNode *p_xor = functions[v];
			for (x = 0; x < integers[v].num[x] != 0; x++) {
				if (set_variable (p_xor, integers[v].num[x], 1) == true_ptr)
				  fprintf (foutputfile, "1");
				else if (set_variable (p_xor, integers[v].num[x], 0) == true_ptr)
				  fprintf (foutputfile, "0");
				else {
					p_xor = set_variable (p_xor, integers[v].num[x], 1);
					fprintf (foutputfile, "1");
				}
			}
/*		} else if (functionType[v] == MINMAX) {
			fprintf(foutputfile, "minmax ");

 //Waiting for ability to do positive and negative literals with minmax
 
         BDDNode *p_or = functions[v];
			for (x = 0; x < integers[v].num[x] != 0; x++) {
				if (set_variable (p_or, integers[v].num[x], 1) == true_ptr)
				  fprintf (foutputfile, "1");
				else if (set_variable (p_or, integers[v].num[x], 0) == true_ptr)
				  fprintf (foutputfile, "0");
				else {
					fprintf (stderr, "Error! %d", functionType[v]);
					fprintf (stderr, "\n");
					printBDDerr (functions[v]);
					fprintf (stderr, "\n");
					exit (1);
				}
			}
*/		} else {
			if(x > MAX_MAX_VBLES_PER_SMURF)
			  fprintf(stderr, "Function %ld has %ld variables and it may take a while to consider all 2^%ld truth table values\n", v, x, x);
			for (long long tvec = 0; tvec < ((long long)1 << x); tvec++) {
				fprintf (foutputfile, "%d", getTruth (integers[v].num, p, x, functions[v]));
				bcount (p, (x - 1));
			}
		}
		fprintf (foutputfile, "\n");
	}
	fprintf (foutputfile, "@");
	free(p);
	free(integers);
   ite_free((void**)&tempint); tempint_max = 0;
}

void Smurfs_to_BDD () {
	typedef struct {
		int *integers;
		char *tv;
		int andor;
	} arr;
	arr * vars = NULL;
	char string, *outvec;
	outvec = new char[100];
	int *tempint = NULL;
   int tempint_max = 0;
	int out = 0, line, i = 0;
	bool flag = false;
   long long y = 0;
   while (out < 3) {
		string = getc (finputfile);
		if ((string >= '0') && (string <= '9')) {
			i = 0;
			if (out == 2) {
				delete [] outvec;
				outvec = new char[numout + 1];
				vars = new arr[numout + 1];
			}
			while ((string != ' ') && (string != '#')) {
				outvec[i] = string;
				i++;
				string = getc (finputfile);
			}
			outvec[i] = 0;
			if (out == 0)
			  numinp = atoi (outvec);
			else if (out == 1)
			  numout = atoi (outvec);
			out++;
		}
	}
	while (string != '\n')
	  string = getc (finputfile);
	for (int x = 0; x < numout; x++) {
		line = -1;
		string = getc (finputfile);	// '#'
		string = getc (finputfile);	// '\n'
		fscanf (finputfile, "%d", &line);
		if (line != x) {
			fprintf (stderr, "\nFunction numbers do not match on function %d\n", line);
			fprintf (stderr, "Should be function %d\n", x);
			exit (1);
		}
      y = -1;
		do {
			y++;
			if (y>=tempint_max) {
				tempint = (int*)ite_recalloc((void*)tempint, tempint_max, tempint_max+100, sizeof(int), 9, "tempint");
				tempint_max += 100;
			}
			fscanf (finputfile, "%d", &tempint[y]);
		}
		while (tempint[y] != -1);
		vars[x].integers = new int[y + 2];
		for (i = 1; i <= y + 1; i++) {
			//vars[x].integers[i] = tempint[i - 1];
			vars[x].integers[i] = tempint[i-1]==0?0:i_getsym_int(tempint[i-1], SYM_VAR);
			if (vars[x].integers[i] == 0) {
				fprintf(stderr, "Variable numbers must be positive integers greater than zero.\n");
				exit(1);
			}
			if (vars[x].integers[i] > numinp) {
				fprintf(stderr, "Variable number %d is larger than the allowed %ld\n", vars[x].integers[i], numinp);
				exit(1);
			}
			//      fprintf(stderr, "%d|", vars[x].integers[i]);
		}
		vars[x].integers[0] = y;
		string = getc (finputfile);	// '\n'
		string = getc (finputfile);
		if ((string == '0') || (string == '1')) {
			vars[x].tv = new char[((long long)1 << y) + 1];
			vars[x].tv[0] = string;
			for(i = 1; i < ((long long)1 << y); i++) {
				string = getc (finputfile);
				if ((string != '0') && (string != '1')) {
					fprintf (stderr, "\nProblem with truth vector in function %d\n", line);
					exit (1);
				}
				vars[x].tv[i] = string;
			}
			vars[x].andor = UNSURE;
		} else if (string == 'p') {
			vars[x].tv = new char[y + 1];
			char test[10];
			for(i = 0; i < 7; i++) {
				string = fgetc (finputfile);
				test[i] = string;
			}
			test[7] = 0;
			if (strcmp (test, "lainor ")) {
				fprintf (stderr, "\nProblem with truth vector in function %d\n", line);
				exit (1);
			}
			for(i = 0; i < y; i++) {
				string = getc (finputfile);
				if ((string != '0') && (string != '1')) {
					fprintf (stderr, "\nProblem with truth vector in function %d\n", line);
					exit (1);
				}
				vars[x].tv[i] = string;
			}
			vars[x].andor = PLAINOR;
		} else if (string == 'a') {
			vars[x].tv = new char[y + 1];
			char test[10];
			for(i = 0; i < 4; i++) {
				string = fgetc (finputfile);
				test[i] = string;
			}
			test[4] = 0;
			if (strcmp (test, "nd= ")) {
				fprintf (stderr, "\nProblem with truth vector in function %d\n", line);
				exit (1);
			}
			for(i = 0; i < y; i++) {
				string = getc (finputfile);
				if ((string != '0') && (string != '1') && (string != '3')) {
					fprintf (stderr, "\nProblem with truth vector in function %d\n", line);
					exit (1);
				}
				vars[x].tv[i] = string;
			}
			vars[x].andor = AND;
		} else if (string == 'o') {
			vars[x].tv = new char[y + 1];
			char test[10];
			for(i = 0; i < 3; i++) {
				string = fgetc (finputfile);
				test[i] = string;
			}
			test[3] = 0;
			if (strcmp (test, "r= ")) {
				fprintf (stderr, "\nProblem with truth vector in function %d\n", line);
				exit (1);
			}
			for(i = 0; i < y; i++) {
				string = getc (finputfile);
				if ((string != '0') && (string != '1') && (string != '3')) {
					fprintf (stderr, "\nProblem with truth vector in function %d\n", line);
					exit (1);
				}
				vars[x].tv[i] = string;
			}
			vars[x].andor = OR;
		} else if (string == 'x') {
			vars[x].tv = new char[y + 1];
			char test[10];
			for(i = 0; i < 3; i++) {
				string = fgetc (finputfile);
				test[i] = string;
			}
			test[3] = 0;
			if (strcmp (test, "or ")) {
				fprintf (stderr, "\nProblem with truth vector in function %d\n", line);
				exit (1);
			}
			for(i = 0; i < y; i++) {
				string = getc (finputfile);
				if ((string != '0') && (string != '1')) {
					fprintf (stderr, "\nProblem with truth vector in function %d\n", line);
					exit (1);
				}
				vars[x].tv[i] = string;
			}
			vars[x].andor = PLAINXOR;
		} else {
			dE_printf2("\nProblem reading in smurf file on function %d\n", line);
			exit (1);
		}
	}
	if (flag)
	  numinp++;
	
   vars_alloc(numinp);
   functions_alloc(numout);
	
	for (int x = 0; x < numout; x++) {
		if ((x+1) % 1000 == 0) d2_printf3("\rDoing %d/%ld", x + 1, numout);
		if (vars[x].integers[0] == 0) {
			functions[x] = true_ptr;
			delete [] vars[x].integers;
			delete [] vars[x].tv;
			continue;
		}
		if (vars[x].andor == UNSURE) {
			int tmp_y = 0;
			int level = 0;
			int *reverse = new int[vars[x].integers[0] + 1];
			for (i = 0; i < vars[x].integers[0]; i++) {
				reverse[vars[x].integers[0] - i] = vars[x].integers[i + 1];
			}
			reverse[0] = vars[x].integers[0];
			functions[x] = ReadSmurf(&tmp_y, vars[x].tv, level, &(reverse[1]), reverse[0]);	//vars[x].integers);
			functionType[x] = vars[x].andor;
			delete [] reverse;
		} else if (vars[x].andor == PLAINOR) {
			functions[x] = false_ptr;
			for (y = 0; y < vars[x].integers[0]; y++) {
				if (vars[x].tv[y] == '0')
				  functions[x] = ite_or (functions[x], ite_var (-vars[x].integers[y + 1]));
				else if (vars[x].tv[y] == '1')
				  functions[x] = ite_or (functions[x], ite_var (vars[x].integers[y + 1]));
			}
			functionType[x] = PLAINOR;
		} else if (vars[x].andor == AND) {
			functions[x] = true_ptr;
			for (y = 0; y < vars[x].integers[0]; y++) {
				if (vars[x].tv[y] == '0')
				  functions[x] = ite_and (functions[x], ite_var (-vars[x].integers[y + 1]));
				else if (vars[x].tv[y] == '1')
				  functions[x] = ite_and (functions[x], ite_var (vars[x].integers[y + 1]));
				else if (vars[x].tv[y] == '3')
				  equalityVble[x] = vars[x].integers[y + 1];
			}
			functionType[x] = AND;
			independantVars[equalityVble[x]] = 0;
			functions[x] = ite_equ (ite_var (equalityVble[x]), functions[x]);
			equalityVble[x] = abs (equalityVble[x]);
		} else if (vars[x].andor == OR) {
			functions[x] = false_ptr;
			for (y = 0; y < vars[x].integers[0]; y++) {
				if (vars[x].tv[y] == '0')
				  functions[x] = ite_or (functions[x], ite_var (-vars[x].integers[y + 1]));
				else if (vars[x].tv[y] == '1')
				  functions[x] = ite_or (functions[x], ite_var (vars[x].integers[y + 1]));
				else if (vars[x].tv[y] == '3')
				  equalityVble[x] = vars[x].integers[y + 1];
			}
			functionType[x] = OR;
			independantVars[equalityVble[x]] = 0;
			functions[x] = ite_equ (ite_var (equalityVble[x]), functions[x]);
			equalityVble[x] = abs (equalityVble[x]);
		} else if (vars[x].andor == PLAINXOR) {
			functions[x] = false_ptr;
			for (y = 0; y < vars[x].integers[0]; y++) {
				if (vars[x].tv[y] == '0')
				  functions[x] = ite_xor (functions[x], ite_var (-vars[x].integers[y + 1]));
				else if (vars[x].tv[y] == '1')
				  functions[x] = ite_xor (functions[x], ite_var (vars[x].integers[y + 1]));
			}
			functionType[x] = PLAINXOR;
		}
		delete [] vars[x].integers;
		delete [] vars[x].tv;
		if (outvec[x] == '0')
		  functions[x] = ite_not (functions[x]);
	}
	delete [] vars;
	delete [] outvec;
	int count = -1;
	
	//Need to remove any clause that was set to True during the making of the BDDs
	for (long x = 0; x < numout; x++) {
		count++;
		functions[count] = functions[x];
		equalityVble[count] = equalityVble[x];
		functionType[count] = functionType[x];
		if (functions[x] == true_ptr)
		  count--;
		
		//printBDDerr(functions[x]);
		//fprintf(stderr, "\n\n");
	}
	fprintf (stderr, "\n");
	numout = count + 1;
	nmbrFunctions = numout;
   ite_free((void**)&tempint); tempint_max = 0;
}
