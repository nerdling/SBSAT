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
 *  markbdd.c (S. Weaver)
 *  BDD tools
 *********************************************************/  

#include "ite.h"
#include "formats.h"

char getNextSymbol (int &intnum, BDDNode * &);

int no_independent;
int markbdd_line;

struct defines_struct {
	char *string;
   BDDNode * bdd;
	int *vlist;
};

int max_defines;
int totaldefines;
defines_struct *defines;
int max_integers;
char *integers;
int max_macros;
char *macros;
int negate_it;
int expect_integer = 0;

BDDNode *putite(int intnum, BDDNode * bdd)
{
	char order = getNextSymbol (intnum, bdd);
	if (order == 'i') { //This becomes unnecessary
		if(intnum >= max_macros) {
			macros = (char *)ite_recalloc(macros, max_macros, intnum+2, sizeof(char), 9, "macros");
			max_macros=intnum+2;
		}
		sprintf(macros, "%d", intnum);
//		intnum = i_getsym(macros, SYM_VAR);
//		Here I need to return an integer if it is actually an integer...
//		for the defines, and InititalBranch, and others that use integers.
//		Though I could fix those to use integers correctly......maybe.....
		d4_printf2("%d ", intnum);
		return ite_var (intnum);
	}
	if (order == 'b')
	  return (bdd);
	if (order == 'x')
	  return true_ptr;
	if (order != 'm') {
		fprintf (stderr, "\nFormula parse error...exiting:%d\n", markbdd_line);
		//I don't think this can actually happen...
		exit (1);
	}
	if (!strcasecmp (macros, "t"))
	  return true_ptr;
	if (!strcasecmp (macros, "f"))
	  return false_ptr;

	//Search the defines for this word...
	for (int x = 0; x < totaldefines; x++) {
		if (!strcasecmp (macros, defines[x].string)) {
			BDDNode **BDDS = new BDDNode *[defines[x].vlist[0] + 1];
			for (int v = 1; v <= defines[x].vlist[0]; v++)
			  BDDS[v] = putite (intnum, bdd);
			BDDNode *returnbdd = f_mitosis(defines[x].bdd, BDDS, defines[x].vlist);
			//			BDDNode * returnbdd = f_apply (defines[x].bdd, BDDS);
			delete [] BDDS;
			strcpy (macros, defines[x].string);
			return returnbdd;
		}
	} //Must be a variable or a reserved word...
	if (!strcasecmp (macros, "ite")) {
		BDDNode * v1, *v2, *v3;
		v1 = putite (intnum, bdd);
		v2 = putite (intnum, bdd);
		v3 = putite (intnum, bdd);
		functionType[nmbrFunctions] = UNSURE;
      strcpy (macros, "ite");
		return ite (v1, v2, v3);
	}
	if (!strcasecmp (macros, "var")) {
		BDDNode *v1 = putite(intnum, bdd);
      strcpy (macros, "var");
		return v1;
	}
	if (!strcasecmp (macros, "not")) {
		BDDNode *v1 = putite (intnum, bdd);
      strcpy (macros, "not");
		return ite_not(v1);
	}
	if (!strcasecmp (macros, "initial_branch")) {

		char p = ' ';
		while (p != ')') {
         p = fgetc(finputfile);
			if (p == '\n') { markbdd_line++; }
			if (p == EOF)	{
				fprintf (stderr, "\nUnexpected EOF (%s)...exiting:%d\n",	macros, markbdd_line);
				exit (1);
			}
			int found_word = 0;
			int i = 0;
			macros[i++] = '^';
			while (((p >= 'a') && (p <= 'z')) || ((p >= 'A') && (p <= 'Z'))
					 || (p == '_') || ((p >= '0') && (p <= '9')) || (p == '[')
					 || (p == ']') || (p == '*')) {
				found_word = 1;
				if(p == '*') macros[i++] = '.';
				macros[i] = p;
				i++;
				if(i >= max_macros) {
					macros = (char *)ite_recalloc(macros, max_macros, max_macros+10, sizeof(char), 9, "macros");
					max_macros+=10;
				}
            p = fgetc(finputfile);
            if (p == EOF) {
					fprintf (stderr, "\nUnexpected EOF (%s)...exiting:%d\n",	macros, markbdd_line);
					exit (1);
				}
			}
			if(found_word == 1) {
				macros[i] = '$';
				macros[i+1] = 0;
				t_myregex myrg;
				sym_regex_init(&myrg, macros);
				int id = sym_regex(&myrg);
				int looper = 0;
				while (id) {
					looper++;
					// found variable and the variable id is id
					independantVars[id] = 1;
					//fprintf(stderr, "%d indep=%d %s %s\n", looper, id, getsym_i(id)->name, macros);
					id = sym_regex(&myrg);
					no_independent = 0;
				}
				sym_regex_free(&myrg);
			}
		}
      strcpy (macros, "initialbranch");
      return true_ptr;
	}
	if (!strcasecmp (macros, "define")) {
      if (getNextSymbol (intnum, bdd) != 'm') {
			fprintf (stderr, "\nUnnamed #define...exiting:%d\n", markbdd_line);
			exit (1);
		}
      if ((!strcasecmp (macros, "initialbranch")) || (!strcasecmp (macros, "define"))
			 || (!strcasecmp (macros, "t")) || (!strcasecmp (macros, "f"))) {
			fprintf (stderr, "\n%s is a reserved word...exiting:%d\n", macros, markbdd_line);
			exit (1);
		}
      int whereat;
      for (whereat = 0; whereat < totaldefines; whereat++) {
			if (!strcasecmp (macros, defines[whereat].string))
			  break;
		}

		if(whereat==totaldefines) {
			defines[whereat].string = (char *)ite_calloc(strlen(macros)+1, sizeof(char), 9, "defines[].string");
			strcpy (defines[whereat].string, macros);
		}
		
		//      int x;
		//      for(x = 0; macros[x]!=0; x++)
		//        defines[whereat].string[x] = macros[x];
		//      defines[whereat].string[x] = 0;      
		// 
		d3e_printf2("#define %s ", defines[whereat].string);
      int v = 0;
		order = getNextSymbol (intnum, bdd);
		
		int max_vlist = 0;
		defines[whereat].vlist = (int *)ite_recalloc(defines[whereat].vlist, max_vlist, max_vlist+10, sizeof(int), 9, "defines[whereat].vlist");
		max_vlist+=10;

      while (order != '#') {
			if (order != 'm') {
				fprintf (stderr, "\nProblem with #define %s...exiting:%d\n", defines[whereat].string, markbdd_line);
				exit (1);
			}
			v++;
			intnum = i_getsym(macros, SYM_VAR);
			for(int iter = 1; iter < v; iter++)
			  if(defines[whereat].vlist[iter] == intnum) {
				  fprintf(stderr, "\nCannot use the same variable (%s) twice in argument list in #define %s...exiting:%d\n", getsym_i(intnum)->name, defines[whereat].string, markbdd_line);
				  exit(1);
			  }
			if(v >= max_vlist) {
				defines[whereat].vlist = (int *)ite_recalloc(defines[whereat].vlist, max_vlist, max_vlist+10, sizeof(int), 9, "defines[whereat].vlist");
				max_vlist+=10;
			}

			defines[whereat].vlist[v] = intnum;
			order = getNextSymbol (intnum, bdd);
		}
      defines[whereat].vlist[0] = v;
      defines[whereat].bdd = putite (intnum, bdd);

		long int y = 0;
      int tempint[5000];
		unravelBDD (&y, tempint, defines[whereat].bdd);
      qsort (tempint, y, sizeof (int), compfunc);
      if (y != 0) {
			int v = 0;
			for (int i = 1; i < y; i++) {
				v++;
				if (tempint[i] == tempint[i - 1])
				  v--;
				tempint[v] = tempint[i];
			}
			y = v + 1;
		}
      
		for (int i = 0; i < y; i++) {
			int found = 0;
			for(int b = 1; b <= defines[whereat].vlist[0]; b++) {
				if(tempint[i] == defines[whereat].vlist[b]) 
				  { found = 1; break; }
			}
			if(found == 0) {
				fprintf(stderr, "\nVariable %s is not in argument list for #define %s...exiting:%d\n", getsym_i(tempint[i])->name, defines[whereat].string, markbdd_line);
				exit(1);
			}
		}

		//below: could compare the lengths and just say, "unused variables in the argument list"
		//but that's not as descriptive
		
		for (int b = 1; b <= defines[whereat].vlist[0]; b++) {
			int found = 0;
			for(int i = 0; i <y; i++) {
				if(tempint[i] == defines[whereat].vlist[b]) 
				  { found = 1; break; }
			}
			if(found == 0) {
				fprintf(stderr, "\nVariable %s is not used in #define %s...exiting:%d\n", getsym_i(defines[whereat].vlist[b])->name, defines[whereat].string, markbdd_line);
				exit(1);
			}
		}

      if (whereat == totaldefines) {
			totaldefines++;
			if(totaldefines >= max_defines) {
				defines = (defines_struct *)ite_recalloc(defines, max_defines, max_defines+10, sizeof(defines_struct), 9, "defines");
				max_defines+=10;
			}
		}
      strcpy (macros, "define");
      return defines[whereat].bdd;
	}
	if (!strcasecmp (macros, "add_state")) {
		fprintf (stderr, "\nKeyword 'add_state' has been deprecated...exiting:%d\n", markbdd_line);
		exit (1);
	}
	if (!strcasecmp(macros, "truth_table")) {
		expect_integer = 1;
		BDDNode *v1 = putite(intnum, bdd);
		expect_integer = 0;
      BDDNode *v2;
		if (v1 != ite_var (v1->variable)) {
			fprintf (stderr, "\nKeyword 'truth_table' needs a positive integer as a first argument (%s)...exiting:%d\n", macros, markbdd_line);
			exit (1);
		}			
		int ints[50];
      for(int i = 1; i <= v1->variable; i++) {
			v2 = putite(intnum, bdd);
			if (v2 != ite_var (v2->variable)) {
				fprintf (stderr, "\nKeyword 'truth_table' needs %d positive integers (%s)...exiting:%d\n", v1->variable, macros, markbdd_line);
				exit (1);
			}			
			ints[i] = v2->variable;
		}
		ints[0] = v1->variable;
		char *tv = new char[1 << v1->variable];
		order = getNextSymbol (intnum, bdd);

		for(long i = 0; i < 1 << v1->variable; i++) {
			d4_printf2("%c", macros[i]);
			if(macros[i] == 'T' || macros[i] == 't') {
				tv[i] = '1';
			} else if(macros[i] == 'F' || macros[i] == 'f'){
				tv[i] = '0';
			} else {		  
				fprintf(stderr, "\n'Keyword 'truth_table' needs 2^%d, found %ld...exiting:%d\n", ints[0], i, markbdd_line);
				exit(0);
			}
			tv[i+1] = 0;
		}
		int y = 0;
		int level = 0;
		functionType[nmbrFunctions] = UNSURE;
      strcpy (macros, "truth_table");
	   return ReadSmurf (&y, tv, level, &(ints[1]), ints[0]);
	}
	if (!strcasecmp(macros, "mitosis")) {
		BDDNode * v1 = putite (intnum, bdd);
      long int y = 0;
      int tempint[5000];
      unravelBDD (&y, tempint, v1);
      qsort (tempint, y, sizeof (int), compfunc);
      if (y != 0) {
			int v = 0;
			for (int i = 1; i < y; i++) {
				v++;
				if (tempint[i] == tempint[i - 1])
				  v--;
				tempint[v] = tempint[i];
			}
			y = v + 1;
		}
      for (int i = y; i >= 0; i--)
		  tempint[i + 1] = tempint[i];
      tempint[0] = y;
      int *newBDD = new int[50];
      
		//      struct BDDNodeStruct **functions = new BDDNode*[50];      
		for (int x = 1; x <= tempint[0]; x++) {
			expect_integer = 1;
			newBDD[x] = (putite (intnum, bdd))->variable;
		}
		expect_integer = 0;

      newBDD[0] = tempint[0];
      delete newBDD;
		functionType[nmbrFunctions] = UNSURE;
      strcpy (macros, "mitosis");
      return mitosis (v1, tempint, newBDD);
	}
	if (!strcasecmp (macros, "minmax")) {
		int min, max;
		expect_integer = 1;
		BDDNode * v1 = putite (intnum, bdd);
		if (v1 != ite_var (v1->variable)) {
			fprintf (stderr, "\nKeyword 'minmax' needs a positive integer as a first argument (%s)...exiting:%d\n", macros, markbdd_line);
			exit (1);
		}
		expect_integer = 1;
		BDDNode * v2 = putite (intnum, bdd);
		if (v2 != ite_var (v2->variable) && v2!=false_ptr) {
			fprintf (stderr, "\nKeyword 'minmax' needs a positive integer as a second argument (%s)...exiting:%d\n", macros, markbdd_line);
			exit (1);
		}
		if(v2 == false_ptr) min = 0;
		else min = v2->variable;
		expect_integer = 1;
      BDDNode * v3 = putite (intnum, bdd);
		if (v3 != ite_var (v3->variable) && v3!=false_ptr) {
			fprintf (stderr, "\nKeyword 'minmax' needs a positive integer as a third argument (%s)...exiting:%d\n", macros, markbdd_line);
			exit (1);
		}
		expect_integer = 0;
		if(v3 == false_ptr) max = 0;
		else max = v3->variable;
		if(max<min) {
			fprintf (stderr, "\nKeyword 'minmax' cannot have it's third argument (%d) less than it's second argument (%d)...exiting:%d\n", max, min, markbdd_line);
			exit (1);
		}

		int *var_list = new int[v1->variable];
		for (unsigned int x = 0; x < (unsigned int)v1->variable; x++) {
			BDDNode *v4 = putite (intnum, bdd);
			if (v4 != ite_var (v4->variable)) {
				//CHANGE LATER? all positive? or not?
				fprintf (stderr, "\nKeyword 'minmax' needs positive variables as arguments (%s)...exiting:%d\n", macros, markbdd_line);
				exit (1);
			}
			var_list[x] = v4->variable;
		}
		int set_true = 0;
		qsort(var_list, v1->variable, sizeof(int), abscompfunc);
		functionType[nmbrFunctions] = MINMAX;
      strcpy (macros, "minmax");
		return MinMaxBDD(var_list, min, max, v1->variable, set_true);
	}
	if (!strcasecmp (macros, "exist")) {
		BDDNode * v1 = putite (intnum, bdd);
      BDDNode * v2 = putite (intnum, bdd);
		functionType[nmbrFunctions] = UNSURE;
      if (v1 == false_ptr)
		  return false_ptr;
      if (v2 != ite_var (v2->variable)) {
			fprintf (stderr, "\nKeyword 'exist' needs a positive integer as a second argument (%s)...exiting:%d\n", macros, markbdd_line);
			exit (1);
		}
      strcpy (macros, "exist");
      return xquantify (v1, v2->variable);
	}
	if (!strcasecmp (macros, "universe")) {
      BDDNode * v1 = putite (intnum, bdd);
      BDDNode * v2 = putite (intnum, bdd);
		functionType[nmbrFunctions] = UNSURE;
      if (v1 == false_ptr)
		  return false_ptr;
      if (v2 != ite_var (v2->variable)) {
			fprintf (stderr, "\nKeyword 'universe' needs a positive integer as a second argument (%s)...exiting:%d\n", macros, markbdd_line);
			exit (1);
		}
		strcpy (macros, "universe");
      return uquantify (v1, v2->variable);
	}
	if (!strcasecmp (macros, "nor")) {
      BDDNode * v1, *v2;
      v1 = putite (intnum, bdd);
      v2 = putite (intnum, bdd);
		functionType[nmbrFunctions] = UNSURE;
      strcpy (macros, "nor");
      return ite_nor (v1, v2);
	}
	if (!strncasecmp (macros, "nor", 3)) {
		int numarguments = 0;
      for (unsigned int x = 3; x < strlen (macros); x++) {
			if (macros[x] < '0' || macros[x] > '9') {
				fprintf (stderr, "\nNon integer value found following keyword 'nor' (%s)...exiting:%d\n",	macros, markbdd_line);
				exit (1);
			}
			numarguments = 10*numarguments + macros[x] - '0';
		}
      BDDNode * v1 = putite (intnum, bdd);
      for (int x = 0; x < numarguments - 1; x++)
		  v1 = ite_or (v1, putite (intnum, bdd));
      v1 = ite_not(v1);
		functionType[nmbrFunctions] = UNSURE;
      strcpy (macros, "nor#");
      return v1;
	}
	if (!strcasecmp (macros, "pprint_tree")) {
      BDDNode * v1;
      v1 = putite (intnum, bdd);
		fprintf(stdout, "\n");
		for (int i = 0; i < PRINT_TREE_WIDTH; i++)
		  fprintf (stdout, "-");
		fprintf(stdout, "\n");
      print_bdd (v1);
		fprintf(stdout, "\n");
      strcpy (macros, "pprint_tree");
      return v1;
	}
	if (!strcasecmp (macros, "print_tree")) {
      BDDNode * v1;
      int which_zoom = 0;
      v1 = putite (intnum, bdd);
      printBDDTree (v1, &which_zoom);
      strcpy (macros, "print_tree");
      return v1;
	}
	if (!strcasecmp (macros, "gcf")) {
      BDDNode * v1, *v2;
      v1 = putite (intnum, bdd);
      v2 = putite (intnum, bdd);
		functionType[nmbrFunctions] = UNSURE;
      strcpy (macros, "gcf");
      if (v1 == false_ptr)
		  return false_ptr;
      return gcf (v1, v2);
	}
	if (!strcasecmp (macros, "prune")) { //Same as restrict, below
		BDDNode * v1, *v2;
		v1 = putite (intnum, bdd);
		v2 = putite (intnum, bdd);
		functionType[nmbrFunctions] = UNSURE;
		strcpy (macros, "prune");
		if (v1 == false_ptr)
		  return false_ptr;
		return p2 (v1, v2);
	}
	if (!strcasecmp (macros, "restrict")) { //Same as prune, above
		BDDNode * v1, *v2;
		v1 = putite (intnum, bdd);
		v2 = putite (intnum, bdd);
		functionType[nmbrFunctions] = UNSURE;
		strcpy (macros, "restrict");
		if (v1 == false_ptr)
		  return false_ptr;
		return p2 (v1, v2);
	}
	if (!strcasecmp (macros, "nimp")) {
      BDDNode * v1, *v2;
      v1 = putite (intnum, bdd);
      v2 = putite (intnum, bdd);
		functionType[nmbrFunctions] = UNSURE;
      strcpy (macros, "nimp");
      return ite_nimp (v1, v2);
	}
	if (!strncasecmp (macros, "nimp", 4)) {
		int numarguments = 0;
      for (unsigned int x = 4; x < strlen (macros); x++) {
			if (macros[x] < '0' || macros[x] > '9') {
				fprintf (stderr, "\nNon integer value found following keyword 'nimp' (%s)...exiting:%d\n", macros, markbdd_line);
				exit (1);
			}
			numarguments = 10*numarguments + macros[x] - '0';
		}
      BDDNode * v1 = putite (intnum, bdd);
      for (int x = 0; x < numarguments - 1; x++)
		  v1 = ite_imp (v1, putite (intnum, bdd));
      v1 = ite_not(v1);
		functionType[nmbrFunctions] = UNSURE;
      strcpy (macros, "nimp#");
      return v1;
	}
	if (!strcasecmp (macros, "nand")) {
      BDDNode * v1, *v2;
      v1 = putite (intnum, bdd);
      v2 = putite (intnum, bdd);
		functionType[nmbrFunctions] = UNSURE;
      strcpy (macros, "nand");
      return ite_nand (v1, v2);
	}
	if (!strncasecmp (macros, "nand", 4)) {
		int numarguments = 0;
      for (unsigned int x = 4; x < strlen (macros); x++) {
			if (macros[x] < '0' || macros[x] > '9') {
				fprintf (stderr, "\nNon integer value found following keyword 'nand' (%s)...exiting:%d\n", macros, markbdd_line);
				exit (1);
			}
			numarguments = 10*numarguments + macros[x] - '0';
		}
      BDDNode * v1 = putite (intnum, bdd); 
      for (int x = 0; x < numarguments - 1; x++)
		  v1 = ite_and (v1, putite (intnum, bdd));
      v1 = ite_not(v1);
		functionType[nmbrFunctions] = UNSURE;
      strcpy (macros, "nand#");
      return v1;
	}
	if (!strcasecmp (macros, "equ")) {
		BDDNode * v1, *v2;
      v1 = putite (intnum, bdd);
      v2 = putite (intnum, bdd);
		functionType[nmbrFunctions] = UNSURE;
      strcpy (macros, "equ");
      return ite_equ (v1, v2);
	}
	if (!strncasecmp (macros, "equ", 4)) {
		int numarguments = 0;
      for (unsigned int x = 4; x < strlen (macros); x++) {
			if (macros[x] < '0' || macros[x] > '9') {
				fprintf (stderr, "\nNon integer value found following keyword 'equ' (%s)...exiting:%d\n", macros, markbdd_line);
				exit (1);
			}
			numarguments = 10*numarguments + macros[x] - '0';
		}
      BDDNode * v1 = putite (intnum, bdd);
      for (int x = 0; x < numarguments - 1; x++)
		  v1 = ite_equ (v1, putite (intnum, bdd));
		functionType[nmbrFunctions] = UNSURE;
      strcpy (macros, "equ#");
      return v1;
	}
	if (!strcasecmp (macros, "imp")) {
      BDDNode * v1, *v2;
      v1 = putite (intnum, bdd);
      v2 = putite (intnum, bdd);
		functionType[nmbrFunctions] = UNSURE;
		strcpy (macros, "imp");
      return ite_imp (v1, v2);
	}
	if (!strncasecmp (macros, "imp", 3)) {
		int numarguments = 0;
      for (unsigned int x = 3; x < strlen (macros); x++) {
			if (macros[x] < '0' || macros[x] > '9') {
				fprintf (stderr, "\nNon integer value found following keyword 'imp' (%s)...exiting:%d\n",	macros, markbdd_line);
				exit (1);
			}
			numarguments = 10*numarguments + macros[x] - '0';
		}
      BDDNode * v1 = putite (intnum, bdd);
      for (int x = 0; x < numarguments - 1; x++)
		  v1 = ite_imp (v1, putite (intnum, bdd));
		functionType[nmbrFunctions] = UNSURE;
      strcpy (macros, "imp#");
      return v1;
	}
	if (!strcasecmp (macros, "and")) {
      BDDNode * v1, *v2;
      v1 = putite (intnum, bdd);
      v2 = putite (intnum, bdd);
		functionType[nmbrFunctions] = UNSURE;
      strcpy (macros, "and");
      return ite_and (v1, v2);
	}
	if (!strncasecmp (macros, "and", 3)) {
		int numarguments = 0;
      for (unsigned int x = 3; x < strlen (macros); x++) {
			if (macros[x] < '0' || macros[x] > '9') {
				fprintf (stderr, "\nNon integer value found following keyword 'and' (%s)...exiting:%d\n",	macros, markbdd_line);
				exit (1);
			}
			numarguments = 10*numarguments + macros[x] - '0';
		}
      BDDNode * v1 = putite (intnum, bdd);
      for (int x = 0; x < numarguments - 1; x++)
		  v1 = ite_and (v1, putite (intnum, bdd));
		functionType[nmbrFunctions] = UNSURE;
      strcpy (macros, "and#");
      return v1;
	}
	if (!strcasecmp (macros, "xor")) {
      BDDNode * v1, *v2;
      v1 = putite (intnum, bdd);
      v2 = putite (intnum, bdd);
		functionType[nmbrFunctions] = UNSURE;
      strcpy (macros, "xor");
      return ite_xor (v1, v2);
	}
	if (!strncasecmp (macros, "xor", 3)) {
		int numarguments = 0;
      for (unsigned int x = 3; x < strlen (macros); x++) {
			if (macros[x] < '0' || macros[x] > '9') {
				fprintf (stderr, "\nNon integer value found following keyword 'xor' (%s)...exiting:%d\n",	macros, markbdd_line);
				exit (1);
			}
			numarguments = 10*numarguments + macros[x] - '0';
		}
      BDDNode * v1 = putite (intnum, bdd);
      for (int x = 0; x < numarguments - 1; x++)
		  v1 = ite_xor (v1, putite (intnum, bdd));
		functionType[nmbrFunctions] = UNSURE;
      strcpy (macros, "xor#");
      return v1;
	}
	if (!strcasecmp (macros, "or")) {
      BDDNode * v1, *v2;
      v1 = putite (intnum, bdd);
      v2 = putite (intnum, bdd);
		functionType[nmbrFunctions] = UNSURE;
      strcpy (macros, "or");
      return ite_or (v1, v2);
	}
	if (!strncasecmp (macros, "or", 2)) {
		int numarguments=0;
      for (unsigned int x = 2; x < strlen (macros); x++) {
			if (macros[x] < '0' || macros[x] > '9') {
				fprintf (stderr, "\nNon integer value found following keyword 'or' (%s)...exiting:%d\n", macros, markbdd_line);
				exit (1);
			}
			numarguments = 10*numarguments + macros[x] - '0';
		}
      BDDNode * v1 = putite (intnum, bdd);
      for (int x = 0; x < numarguments - 1; x++)
		  v1 = ite_or (v1, putite (intnum, bdd));
		functionType[nmbrFunctions] = UNSURE;
      strcpy (macros, "or#");
      return v1;
	}

	int v = i_getsym(macros, SYM_VAR);

	if(v > numinp) {
		numinp+=10;
		vars_alloc(numinp+2); //should recalloc
		for(int iter = 0; iter<10; iter++)
		  independantVars[numinp-iter] = 0;
	}

	if(negate_it == 1) {
		negate_it = 0;
		return ite_var(-v);
	} else return ite_var(v);

	fprintf (stderr, "\nUnknown word (%s)...exiting:%d\n", macros,	markbdd_line);
	exit (1);
	return NULL;
}

char getNextSymbol (int &intnum, BDDNode *&bdd) {
	int i = 0;
	int p = 0;
	while (1) {
      p = fgetc(finputfile);
		if (p == '\n') { markbdd_line++; }
      if (p == EOF) {
			//fprintf(stderr, "\nUnexpected EOF (%s)...exiting\n", macros);
			return 'x';
		}
      if (p == '*') {
			fprintf (stderr, "Formula parse error...exiting:%d\n", markbdd_line);
			exit (1);
		}
      if (p == ';') {
			while (p != '\n') {
            p = fgetc(finputfile);
            if (p == EOF) {
					fprintf (stderr, "\nUnexpected EOF...exiting:%d\n", markbdd_line);
					exit (1);
				}
			}
			ungetc(p, finputfile);
			continue;
		}
      if (((p >= 'a') && (p <= 'z')) || ((p >= 'A') && (p <= 'Z'))
			 || (p == '_') || ((p >= '0') && (p <= '9')) || (p == '-')) {
			negate_it = 0;
			if (p == '-') {
				negate_it = 1;
            p = fgetc(finputfile);
            if (p == EOF) {
					fprintf (stderr, "\nUnexpected EOF...exiting:%d\n", markbdd_line);
					exit (1);
				}
			}
			if (p == '-') {
				fprintf (stderr, "\nDouble '-' found, variables cannot start with a '-'...exiting:%d\n", markbdd_line);
				exit (1);
			}
			int is_int = 1;
			while (((p >= 'a') && (p <= 'z')) || ((p >= 'A') && (p <= 'Z'))
					 || (p == '_') || ((p >= '0') && (p <= '9'))) {
				macros[i] = p;
				i++;
				if (((p >= 'a') && (p <= 'z')) || ((p >= 'A') && (p <= 'Z'))
					 || (p == '_')) {
					is_int = 0;
				}
				if(i >= max_macros) {
					macros = (char *)ite_recalloc(macros, max_macros, max_macros+10, sizeof(char), 9, "macros");
					max_macros+=10;
				}
            p = fgetc(finputfile);
            if (p == EOF) {
					fprintf (stderr, "\nUnexpected EOF (%s)...exiting:%d\n",	macros, markbdd_line);
					exit (1);
				}
			}
			ungetc (p, finputfile);

			macros[i] = 0;
			if(is_int == 1 && expect_integer == 1) {
				expect_integer = 0;
				intnum = atoi (macros);
				if (intnum > numinp) {
					fprintf (stderr, "\nVariable %d is larger than allowed (%ld)...exiting:%d\n",	intnum, numinp - 2, markbdd_line);
					exit (1);
				}
				if (negate_it) intnum = -intnum;
				return 'i';
			} else if(is_int == 0 && expect_integer == 1) {					  
				fprintf(stderr, "\nExpecting an integer, found %s...exiting:%d\n", macros, markbdd_line);
				exit(1);
			} else {
				if(!strcasecmp(macros, "define")) {
					fprintf (stderr, "\n'#' expected in front of keyword 'define'...exiting:%d\n", markbdd_line);
					exit (1);
				}
			}
			if(negate_it) d4_printf2("-%s ", macros);
			if(!negate_it) d4_printf2("%s ", macros);
			return 'm';
		}
      if (p == '#') {
         p = fgetc(finputfile);
         if (p == EOF) {
				fprintf (stderr, "\nUnexpected EOF...exiting:%d\n", markbdd_line);
				exit (1);
			}
			if(p == 'd') { //d for #define, no other operator starts with d
				while (((p >= 'a') && (p <= 'z')) || ((p >= 'A') && (p <= 'Z'))
						 || (p == '_') || ((p >= '0') && (p <= '9'))) {
					macros[i] = p;
					i++;
					if(i >= max_macros) {
						macros = (char *)ite_recalloc(macros, max_macros, max_macros+10, sizeof(char), 9, "macros");
						max_macros+=10;
					}
               p = fgetc(finputfile);
               if (p == EOF) {
						fprintf (stderr, "\nUnexpected EOF (%s)...exiting:%d\n",	macros, markbdd_line);
						exit (1);
					}
				}
				ungetc (p, finputfile);
				macros[i] = 0;
				if(strcasecmp(macros, "define")) {
					fprintf (stderr, "\n#define expected, found %s...exiting:%d\n", macros, markbdd_line);
					exit (1);
				}
				return 'm';
			} else {
				ungetc(p, finputfile);
				return '#';
			}
		}
      if (p == '$') {
         p = fgetc(finputfile);
         if (p == EOF) {
				fprintf (stderr, "\nUnexpected EOF (%s)...exiting:%d\n", macros, markbdd_line);
				exit (1);
			}
			if ((p >= '1') && (p <= '9')) {
				i = 0;
				while ((p >= '0') && (p <= '9')) {
					integers[i] = p;
					i++;
					if(i >= max_integers) {
						integers = (char *)ite_recalloc(integers, max_integers, max_integers+10, sizeof(char), 9, "integers");
						max_integers+=10;
					}
               p = fgetc(finputfile);
               if (p == EOF) {
						fprintf (stderr, "\nUnexpected EOF (%s)...exiting:%d\n",	integers, markbdd_line);
						exit (1);
					}
				}
				integers[i] = 0;
				intnum = atoi (integers) - 1;
				if (intnum > nmbrFunctions - 1) {
					fprintf (stderr, "\nFunction $%d has not yet been defined...exiting:%d\n", intnum + 1, markbdd_line);
					exit (1);
				}
				ungetc (p, finputfile);
				bdd = functions[intnum];
				return 'b';
			} else {
				fprintf (stderr, "\nFormula parse error...exiting:%d\n",	markbdd_line);
				exit (1);
			}
		}
	}
}

void bddloop () {
	fscanf (finputfile, "%ld %ld", &numinp, &numout);
	numinp += 2;
	markbdd_line = 1;
	int intnum = 0, keepnum = 0;
	BDDNode * bdd = NULL;
	int p = 0;

	totaldefines = 0;
	max_defines = 100;
	defines = (defines_struct *)ite_recalloc(defines, 0, max_defines, sizeof(defines_struct), 9, "defines");
	
	max_integers = 20;
	integers = (char *)ite_recalloc(integers, 0, max_integers, sizeof(char), 9, "integers");
	
	max_macros = 20;
	macros = (char *)ite_recalloc(macros, 0, max_macros, sizeof(char), 9, "macros");
	
	no_independent = 1;
	
   vars_alloc(numinp+2);
   functions_alloc(numout+2);
	
	int *keep = (int *)ite_calloc(numout + 2, sizeof(int), 9, "keep");

	for (int x = 0; x < numinp + 1; x++) {
		independantVars[x] = 0;
	}
	
	p = fgetc(finputfile);
	 
	d3_printf1("\n");
	
	while (1) {		//(p = fgetc(finputfile))!=EOF) 
		if(markbdd_line == 1) { d2_printf1("Reading 2"); }
      else {
			d2e_printf2("\rReading %d ", markbdd_line);
			d3_printf2("Reading %d ", markbdd_line);
		}
		if (p == '\n') {
			p = fgetc(finputfile);
			if(p == EOF) {
				goto Exit;
			}
			markbdd_line++;
			d3_printf1("\r");
			continue;
		}
      if (p == ';') {
			while (p != '\n') {
            p = fgetc(finputfile);
				if (p == EOF) {
					goto Exit;
				}
			}
			d3_printf1("\r");
			continue;
		}
		if (p == ' ') {
			while (p != ' ') {
            p = fgetc(finputfile);
				if (p == EOF) {
					goto Exit;
				}
			}
			p = fgetc(finputfile);
			if(p == EOF) {
				goto Exit;
			}
				  
			d3_printf1("\r");
			continue;
		}
		if (p == '*') {
			BDDNode *temp = putite(intnum, bdd);
			if ((strcasecmp (macros, "pprint_tree"))
				 && (strcasecmp (macros, "print_tree"))
				 && (strcasecmp (macros, "define"))
				 && (strcasecmp (macros, "initialbranch"))) {
				keep[nmbrFunctions] = 1;
				keepnum++;
				functions[nmbrFunctions] = temp;
				nmbrFunctions++;
				if (nmbrFunctions > numout) {
					fprintf (stderr, "Too many functions, increase to larger than %ld...exiting:%d",	numout, markbdd_line);
					exit (1);
				}
			}
			d3_printf1("*");
		} else {
			ungetc (p, finputfile);
			if (p == ';')
			  continue;
			BDDNode * temp = putite (intnum, bdd);
			if ((strcasecmp (macros, "pprint_tree"))
				 && (strcasecmp (macros, "print_tree"))
				 && (strcasecmp (macros, "define"))
				 && (strcasecmp (macros, "initialbranch"))) {
				functions[nmbrFunctions] = temp;
				nmbrFunctions++;
				if (nmbrFunctions > numout) {
					fprintf (stderr, "Too many functions, increase to larger than %ld...exiting:%d",	numout, markbdd_line);
					exit (1);
				}
			}
		}
      D_3(
      if ((strcasecmp (macros, "pprint_tree"))
			 && (strcasecmp (macros, "print_tree"))
			 && (strcasecmp (macros, "define"))
			 && (strcasecmp (macros, "initialbranch"))) {
			fprintf (stddbg, "BDD $%d: ", nmbrFunctions);
			printBDDfile (functions[nmbrFunctions - 1], stddbg);
			//fprintf (stddbg, "\n");
		}
      )

		p = fgetc (finputfile);
      if (p != '\n') {
			int continue_all = 0;
			if(p==';') continue_all = 1;
			else if (p!=' ' && p!='(' && p!=')' && p!=',') {
				fprintf(stderr, "Error: Extra characters following line %d...exiting\n", markbdd_line);
				exit(0);
				continue_all = 1;
			}
         p = fgetc(finputfile);
         while (p != EOF) {
				if (p == '\n')
				  break;
				if (p == ';') continue_all = 1;
				if (continue_all == 0 && p!=' ' && p!='(' && p!=')' && p!=',') {
					fprintf(stderr, "Error: Extra characters following line %d...exiting\n", markbdd_line);
					exit(0);
					continue_all = 1;
				}
            p = fgetc(finputfile);
         }
			if (p != '\n')
			  goto Exit;
		}
		d3_printf1("\n");
	}
	Exit:;
	if (keepnum == 0)
	  for (int x = 0; x < nmbrFunctions; x++)
		 keep[x] = 1;
	int count = -1;
	if (no_independent) {
		for (int x = 0; x < numinp + 1; x++) {
			independantVars[x] = 1;
		}
	}
	
	//  for(int x = 0; x < numinp+1; x++) {
	//    fprintf(foutputfile, "%d %d|", x, independantVars[x]);
	//  }
	// 
	//(x <= numout) means to throw away functions that come after
	//the cutoff part numout.
	for (int x = 0; (x < nmbrFunctions) && (x <= numout); x++) {
		count++;
      functions[count] = functions[x];
      functionType[count] = functionType[x];
      equalityVble[count] = equalityVble[x];
      //parameterizedVars[count] = parameterizedVars[x];
		if ((functions[x] == true_ptr) || (keep[x] == 0)) {
			count--;
		}
      else if (functions[x] == false_ptr) {
			//fprintf(stderr, "\nProblem is Unsatisfiable...exiting:%d\n", x+1, markbdd_line);
			//exit(1);
		}
	}
	nmbrFunctions = count + 1;
	if (nmbrFunctions == 0) {
		//fprintf(stderr, "\nProblem is a Tautology...exiting\n");
		//exit(1);
	}
	d2_printf1("\rReading ITE ... Done\n");
	ite_free((void **)&keep);
	for(int x = 0; x < totaldefines; x++)
	  ite_free((void **)&defines[x].string);
	ite_free((void **)&defines);
	ite_free((void **)&integers);
	ite_free((void **)&macros);
}
