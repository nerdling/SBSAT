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

char getNextSymbol (int &intnum, BDDNode * &);

int markbdd_line;

struct defines_struct {
	char *string;
   BDDNode * bdd;
	int *vlist;
};

struct initbranch_level {
	char *string;
	double true_inf_weight;
};

struct initbranch_struct {
	int branch_level;
	int max_initbranch_level;
	int num_initbranch_level;
	initbranch_level *vars;
};

int initbranch_compfunc (const void *x, const void *y) {
  initbranch_struct pp, qq;

  pp = *(const initbranch_struct *) x;
  qq = *(const initbranch_struct *) y;
  if (pp.branch_level < qq.branch_level)
    return -1;
  if (pp.branch_level == qq.branch_level)
    return 0;
  return 1;
}

initbranch_struct *initbranch;

int max_initbranch;
int max_varlevel;

double *arrVarTrueInfluences;
int **arrVarChoiceLevels;
int nVarChoiceLevelsMax;
int nVarChoiceLevelsNum;

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
		//sprintf(macros, "%d", intnum);
//		intnum = i_getsym(macros, SYM_VAR);
//		Here I need to return an integer if it is actually an integer...
//		for the defines, and InititalBranch, and others that use integers.
//		Though I could fix those to use integers correctly......maybe.....
		d5_printf2("%s ", macros);
		if(intnum == 0) return false_ptr;
		return ite_var (intnum);
	}
	if (order == 'b')
	  return (bdd);
	if (order == 'x')
	  return NULL;
	if (order == ')')
	  return NULL;
	if (order != 'm') {
		fprintf(stderr, "\nFormula parse error...exiting:%d\n", markbdd_line);
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
			for (int v = 1; v <= defines[x].vlist[0]; v++) {
				BDDS[v] = putite (intnum, bdd);
				if(BDDS[v] == NULL) { fprintf(stderr, "\n#define '%s' requires %d arguments, found %d...exiting:%d\n", defines[x].string, defines[x].vlist[0], v-1, markbdd_line); exit (1); }
			}
			if(putite (intnum, bdd)!=NULL) { fprintf(stderr, "\n#define '%s' requires %d arguments, found > %d...exiting:%d\n", defines[x].string, defines[x].vlist[0], defines[x].vlist[0], markbdd_line); exit (1); }
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
		if(v1 == NULL) { fprintf(stderr, "\nKeyword 'ite' expects 3 arguments, found 0 (%s)...exiting:%d\n", macros, markbdd_line); exit (1); }
		v2 = putite (intnum, bdd);
		if(v2 == NULL) { fprintf(stderr, "\nKeyword 'ite' expects 3 arguments, found 1 (%s)...exiting:%d\n", macros, markbdd_line); exit (1); }
		v3 = putite (intnum, bdd);
		if(v3 == NULL) { fprintf(stderr, "\nKeyword 'ite' expects 3 arguments, found 2 (%s)...exiting:%d\n", macros, markbdd_line); exit (1); }
		if(putite(intnum, bdd) != NULL) { fprintf(stderr, "\nKeyword 'ite' expects 3 arguments, found > 3 (%s)...exiting:%d\n", macros, markbdd_line); exit (1); }
      strcpy (macros, "ite");
		return ite (v1, v2, v3);
	}
	if (!strcasecmp (macros, "var")) {
		BDDNode *v1 = putite(intnum, bdd);
		if(v1 == NULL) { fprintf(stderr, "\nKeyword 'var' expects 1 argument, found 0 (%s)...exiting:%d\n", macros, markbdd_line); exit (1); }
		if(putite(intnum, bdd)!=NULL) { fprintf(stderr, "\nKeyword 'var' expects 1 argument, found > 1 (%s)...exiting:%d\n", macros, markbdd_line); exit (1); }
      strcpy (macros, "var");
		return v1;
	}
	if (!strcasecmp (macros, "not")) {
		BDDNode *v1 = putite (intnum, bdd);
		if(v1 == NULL) { fprintf(stderr, "\nKeyword 'not' expects 1 argument, found 0 (%s)...exiting:%d\n", macros, markbdd_line); exit (1); }
		if(putite(intnum, bdd)!=NULL) { fprintf(stderr, "\nKeyword 'not' expects 1 argument, found > 1 (%s)...exiting:%d\n", macros, markbdd_line); exit (1); }
      strcpy (macros, "not");
		return ite_not(v1);
	}
	if (!strcasecmp (macros, "initial_branch")) {
		char p = ' ';
		int started_words = 0;
		int branch_level = 0;
		int found_level = 0;
		
		while (p != ')') {
         p = fgetc(finputfile);
			if (p == '\n') { markbdd_line++; }
			if (p == ';') {
				while (p != '\n') {
					p = fgetc(finputfile);
					if (p == EOF) {
						fprintf(stderr, "\nUnexpected EOF (%s)...exiting:%d\n", macros, markbdd_line);
						exit (1);
					}
				}
				markbdd_line++;
			}
			if (p == EOF)	{
				fprintf(stderr, "\nUnexpected EOF (%s)...exiting:%d\n",	macros, markbdd_line);
				exit (1);
			}
			int found_word = 0;
			int i = 0;
			macros[i++] = '^';
			while (((p >= 'a') && (p <= 'z')) || ((p >= 'A') && (p <= 'Z'))
					 || (p == '_') || ((p >= '0') && (p <= '9')) || (p == '[')
					 || (p == ']') || (p == '*') || (p=='?') || (p=='.') || (p=='^')) {
				started_words = 1;
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
					fprintf(stderr, "\nUnexpected EOF (%s)...exiting:%d\n",	macros, markbdd_line);
					exit (1);
				}
			}
			if(found_word == 1) {
				macros[i] = '$';
				macros[i+1] = 0;
				initbranch[branch_level].vars[initbranch[branch_level].num_initbranch_level].string = new char[i+2];
				strcpy(initbranch[branch_level].vars[initbranch[branch_level].num_initbranch_level].string, macros);
				d5_printf2(" %s", initbranch[branch_level].vars[initbranch[branch_level].num_initbranch_level].string);
				if(p == '%') {
					d5_printf1("%%");
					expect_integer = 1;
					BDDNode *v1 = putite(intnum, bdd);
					if(v1 == NULL) { fprintf(stderr, "\nKeyword 'initial_branch' needs a number between 0 and 100 after a '%%' (%s)...exiting:%d\n", macros, markbdd_line); exit (1);}
						  
					expect_integer = 0;
					double tweight = 0;
					if(v1 != false_ptr) {
						if (v1 != ite_var (v1->variable)) {
							fprintf(stderr, "\nKeyword 'initial_branch' needs a number between 0 and 100 after a '%%' (%s)...exiting:%d\n", macros, markbdd_line);
							exit (1);
						}
						if (v1->variable > 100) {
							fprintf(stderr, "\nKeyword 'initial_branch' needs a number between 0 and 100 after a '%%' (%s)...exiting:%d\n", macros, markbdd_line);
							exit (1);
						}
						tweight = ((double)v1->variable)/100.0;
					}
					d6_printf2(" %f  ", tweight);
					p = fgetc(finputfile);
					if(p == EOF) {
						fprintf(stderr, "\nUnexpected EOF (%s)...exiting:%d\n", macros, markbdd_line);
						exit(1);
					}
					if(p == '.') {
						d5_printf1("\b.");
						expect_integer = 1;
						BDDNode *v1 = putite(intnum, bdd);
						expect_integer = 0;
						double pweight = 0;
						if(v1!=false_ptr) {
							if(v1 == NULL) { fprintf(stderr, "\nKeyword 'initial_branch' needs a number between 0 and 100 after a '%%' (%s)...exiting:%d\n", macros, markbdd_line); exit(1); }
							
							if(v1 != ite_var(v1->variable)) {
								fprintf(stderr, "\nKeyword 'initial_branch' needs a number between 0 and 100 after a '%%' (%s)...exiting:%d\n", macros, markbdd_line);
								exit(1);
							}
							pweight = (double)v1->variable;
							
							for(int dec = 0; dec<strlen(macros)/*floor(pweight)!=0*/; dec++) {
								pweight=pweight/10.0;
							}
							pweight=pweight/100.0;                                                                                          
						}
						tweight+=pweight;
						d6_printf2(" %f ", tweight);
						if(tweight > 100) { fprintf(stderr, "\nKeyword 'initial_branch' needs a number between 0 and 100 after a '%%' (%s)...exiting:%d\n", macros, markbdd_line); exit(1); }
					} else {
						ungetc(p, finputfile);
					}
					initbranch[branch_level].vars[initbranch[branch_level].num_initbranch_level].true_inf_weight = tweight;
				} else {
					initbranch[branch_level].vars[initbranch[branch_level].num_initbranch_level].true_inf_weight = 0.5;
				}
					  
				initbranch[branch_level].num_initbranch_level++;
				if(initbranch[branch_level].num_initbranch_level >= initbranch[branch_level].max_initbranch_level) {
					initbranch[branch_level].vars = (initbranch_level *)ite_recalloc(initbranch[branch_level].vars, initbranch[branch_level].max_initbranch_level, initbranch[branch_level].max_initbranch_level+10, sizeof(initbranch_level), 9, "initbranch[branch_level].vars");
					initbranch[branch_level].max_initbranch_level+=10;
				}
			} else if(p == '%') {
				fprintf(stderr, "\nKeyword 'initial_branch' needs '%%' to follow directly after a variable (%s)...exiting:%d\n", macros, markbdd_line);
				exit (1);
			}
			if(p == '#') {
				d5_printf1("#");
				if(found_level == 1) {
					fprintf(stderr, "\nKeyword 'initial_branch' can only have one #level associated with it, found multiple (%s)...exiting:%d\n", macros, markbdd_line);
					exit (1);
				} else if(started_words == 1) {
					fprintf(stderr, "\nKeyword 'initial_branch' needs #level to be the first argument (%s)...exiting:%d\n", macros, markbdd_line);
					exit (1);
				} else {
					found_level = 1;  

					expect_integer = 1;
					BDDNode *v1 = putite(intnum, bdd);
					if(v1 == NULL) { fprintf(stderr, "\nKeyword 'initial_branch' needs a positive integer as a level after '#' (%s)...exiting:%d\n", macros, markbdd_line); exit (1); }
					expect_integer = 0;
					if (v1 != ite_var (v1->variable)) {
						fprintf(stderr, "\nKeyword 'initial_branch' needs a positive integer as a level after '#' (%s)...exiting:%d\n", macros, markbdd_line);
						exit (1);
					}
					if(v1->variable > 10000) {
						fprintf(stderr, "\nKeyword 'initial_branch' needs a positive integer < 10000 as a level after '#' (%s)...exiting:%d\n", macros, markbdd_line);
						exit(1);
					}
					if(v1->variable >= max_varlevel) max_varlevel = v1->variable+1;
					//search for branch_level in initbranch
					branch_level = -1;
					for(int i = 0; i < max_initbranch; i++) {
						if(initbranch[i].branch_level == v1->variable) {
							branch_level = i;
							break;
						}
					}
					if(branch_level == -1) {
						for(int i = 0; i < max_initbranch; i++) {
							if(initbranch[i].branch_level == 0) {
								initbranch[i].branch_level = v1->variable;
								branch_level = i;
								break;
							}
						}
					}
					if(branch_level == -1) {
						//Create a new initbranch[branch_level]
						initbranch = (initbranch_struct *)ite_recalloc(initbranch, max_initbranch, max_initbranch+1, sizeof(initbranch_struct), 9, "initbranch");
						branch_level = max_initbranch;
						max_initbranch++;
						initbranch[branch_level].branch_level = v1->variable;
						initbranch[branch_level].max_initbranch_level = 10;
						initbranch[branch_level].num_initbranch_level = 0;
						initbranch[branch_level].vars = (initbranch_level *)ite_recalloc(initbranch[branch_level].vars, 0, initbranch[branch_level].max_initbranch_level, sizeof(initbranch_level), 9, "initbranch[0].vars");
					}
				}
			}
		}
      //ungetc(p, finputfile);
		strcpy (macros, "initial_branch");
      return true_ptr;
	}
	if (!strcasecmp (macros, "define")) {
      if (getNextSymbol (intnum, bdd) != 'm') {
			fprintf(stderr, "\nUnnamed #define...exiting:%d\n", markbdd_line);
			exit (1);
		}
      if ((!strcasecmp (macros, "initial_branch")) || (!strcasecmp (macros, "define")) ||
			 (!strcasecmp (macros, "t")) || (!strcasecmp (macros, "f")) ||
			 (!strcasecmp(macros, "ite")) || (!strcasecmp(macros, "var")) ||
			 (!strcasecmp(macros, "not")) || (!strcasecmp(macros, "order")) ||
			 (!strcasecmp(macros, "truth_table")) || (!strcasecmp(macros, "mitosis")) ||
			 (!strcasecmp(macros, "minmax")) || (!strcasecmp(macros, "exist")) ||
			 (!strcasecmp(macros, "universe")) || (!strcasecmp(macros, "safe")) ||
			 (!strcasecmp(macros, "safe_eq")) || (!strcasecmp(macros, "safe_func")) ||
			 (!strcasecmp(macros, "remove_fps")) || (!strcasecmp(macros, "print_tree")) ||
			 (!strcasecmp(macros, "pprint_tree")) || (!strcasecmp(macros, "print_xdd")) ||
			 (!strcasecmp(macros, "print_flat_xdd")) || (!strcasecmp(macros, "print_dot_stdout")) ||
			 (!strcasecmp(macros, "print_dot")) || (!strcasecmp(macros, "gcf")) ||
			 (!strcasecmp(macros, "prune")) || (!strcasecmp(macros, "restrict")) ||
			 (!strcasecmp(macros, "strengthen")) || (!strcasecmp(macros, "nimp")) ||
			 (!strcasecmp(macros, "nand")) || (!strcasecmp(macros, "nor")) ||
			 (!strcasecmp(macros, "equ")) || (!strcasecmp(macros, "imp")) ||
			 (!strcasecmp(macros, "countT")) || (!strcasecmp(macros, "and")) ||
			 (!strcasecmp(macros, "xor")) || (!strcasecmp(macros, "or")) ||
		    (!strcasecmp(macros, "same")) || (!strcasecmp(macros, "print_smurf_dot_stdout"))) {
			fprintf(stderr, "\n%s is a reserved word...exiting:%d\n", macros, markbdd_line);
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
		d4_printf2("#define %s ", defines[whereat].string);
      int v = 0;
		order = getNextSymbol (intnum, bdd);
		if(order == ')') { fprintf(stderr, "\nKeyword 'define' missing argument before '#' (%s)...exiting:%d\n", macros, markbdd_line); exit (1); }
		
		int max_vlist = 0;
		defines[whereat].vlist = (int *)ite_recalloc(NULL, max_vlist, max_vlist+10, sizeof(int), 9, "defines[whereat].vlist");
		max_vlist+=10;

      while (order != '#') {
			if (order == ')') {
				order = getNextSymbol (intnum, bdd);
				if(order != '#') { fprintf(stderr, "\nProblem with #define %s (%s)...exiting:%d\n", defines[whereat].string, macros, markbdd_line); exit (1); }
				continue;
			}
			if (order != 'm') { fprintf(stderr, "\nProblem with #define %s (%s)...exiting:%d\n", defines[whereat].string, macros, markbdd_line); exit (1); }
			v++;
			intnum = i_getsym(macros, SYM_VAR);

			if(intnum >= numinp-2) {
				fprintf(stderr, "\nToo many symbols used (%s). Need to increase to greater than %ld...exiting:%d\n", macros, numinp-3, markbdd_line);
				exit (1);
			}

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
		if(defines[whereat].bdd == NULL) { fprintf(stderr, "\nKeyword 'define' missing argument after '#' (%s)...exiting:%d\n", macros, markbdd_line); exit (1); }
		
      int *tempint=NULL;
		int y = 0;
      int tempint_max = 0;
		unravelBDD (&y, &tempint_max, &tempint, defines[whereat].bdd);
      if (y != 0) qsort (tempint, y, sizeof (int), compfunc);
      
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
      ite_free((void**)&tempint); tempint_max=0;
      return defines[whereat].bdd;
	}
	if (!strcasecmp (macros, "order")) {
		BDDNode *v1;
		int count = 0;
		while ((v1 = putite (intnum, bdd))!=NULL) {
			if (v1 != ite_var (v1->variable)) {
				fprintf(stderr, "\nKeyword 'order' expects a list of positive variables terminated by ')'(%s)...exiting:%d\n", macros, markbdd_line);
				exit (1);
			}
			count++;
		}
		if(count < 1) {
			fprintf(stderr, "\nKeyword 'order' expects at least 1 argument, found 0 (%s)...exiting:%d\n", macros, markbdd_line);
			exit (1);
		}
		strcpy (macros, "order");
      return true_ptr;
	}
	if (!strcasecmp(macros, "truth_table")) {
		expect_integer = 1;
		BDDNode *v1 = putite(intnum, bdd);
		if(v1 == NULL) {
			fprintf(stderr, "\nKeyword 'truth_table' needs a positive integer as a first argument (%s)...exiting:%d\n", macros, markbdd_line);
			exit (1);
		}
		expect_integer = 0;
		if (v1 != ite_var (v1->variable)) {
			fprintf(stderr, "\nKeyword 'truth_table' needs a positive integer as a first argument (%s)...exiting:%d\n", macros, markbdd_line);
			exit (1);
		}			
      BDDNode *v2;
		int ints[50];
      for(int i = 1; i <= v1->variable; i++) {
			v2 = putite(intnum, bdd);
			if (v2 == NULL || v2 != ite_var (v2->variable)) {
				fprintf(stderr, "\nKeyword 'truth_table' needs %d positive integers (%s)...exiting:%d\n", v1->variable, macros, markbdd_line);
				exit (1);
			}			
			ints[i] = v2->variable;
		}
		ints[0] = v1->variable;
		char *tv = new char[(1 << v1->variable) +1];
		order = getNextSymbol (intnum, bdd);
		if(order == ')') { fprintf(stderr, "\n'Keyword 'truth_table' needs 2^%d, found 0...exiting:%d\n", ints[0], markbdd_line); exit(1); }

		for(long i = 0; i < 1 << v1->variable; i++) {
			d5_printf2("%c", macros[i]);
			if(macros[i] == 'T' || macros[i] == 't') {
				tv[i] = '1';
			} else if(macros[i] == 'F' || macros[i] == 'f'){
				tv[i] = '0';
			} else {		  
				fprintf(stderr, "\n'Keyword 'truth_table' needs 2^%d, found %ld...exiting:%d\n", ints[0], i, markbdd_line);
				exit(1);
			}
			tv[i+1] = 0;
		}
		int y = 0;
		int level = 0;
      strcpy (macros, "truth_table");
		BDDNode *tempBDD = ReadSmurf (&y, tv, level, &(ints[1]), ints[0]);
		delete [] tv;
		return tempBDD;
	}
	if (!strcasecmp(macros, "mitosis")) {
		BDDNode * v1 = putite (intnum, bdd);
      if(v1 == NULL) fprintf(stderr, "\nKeyword 'mitosis' expects at least 2 arguments, found 0 (%s)...exiting:%d\n", macros, markbdd_line);
		int *tempint=NULL;
      int tempint_max = 0;
      int y = 0;
      unravelBDD (&y, &tempint_max, &tempint, v1);
      if (y != 0) qsort (tempint, y, sizeof (int), compfunc);

      for (int i = y; i >= 0; i--)
		  tempint[i + 1] = tempint[i];
      tempint[0] = y;
      int *newBDD = new int[50];
      
		//      struct BDDNodeStruct **functions = new BDDNode*[50];      
		int x = 1;
		for (; x <= tempint[0]; x++) {
			BDDNode *v2 = (putite (intnum, bdd));
			if(v2 == NULL) { fprintf(stderr, "\nHere, keyword 'mitosis' expects %d arguments, found %d (%s)...exiting:%d\n", tempint[0]+1, x, macros, markbdd_line); exit (1); }
			if (v2 != ite_var (v2->variable)) {
				fprintf(stderr, "\nKeyword 'mitosis' expects a list of positive variables terminated by ')'(%s)...exiting:%d\n", macros, markbdd_line);
				exit (1);
			}
			newBDD[x] = v2->variable;
		}
		if(putite(intnum, bdd)!=NULL || x != tempint[0]+1) { fprintf(stderr, "\nHere, keyword 'mitosis' expects %d arguments, found %d (%s)...exiting:%d\n", tempint[0]+1, x+1, macros, markbdd_line); exit (1); }

      newBDD[0] = tempint[0];
      delete newBDD;
      strcpy (macros, "mitosis");
		BDDNode *tempBDD = mitosis (v1, tempint, newBDD);
      ite_free((void**)&tempint); tempint_max = 0;
      return tempBDD;
	}
	if (!strcasecmp (macros, "minmax")) {
		int min, max;
		expect_integer = 1;
		BDDNode * v1 = putite (intnum, bdd);
		if (v1 == NULL || v1!= ite_var (v1->variable) && v1!=false_ptr) {
			fprintf(stderr, "\nKeyword 'minmax' needs a positive integer as a first argument (%s)...exiting:%d\n", macros, markbdd_line);
			exit (1);
		}
		if(v1 == false_ptr) min = 0;
		else min = v1->variable;
		expect_integer = 1;
      BDDNode * v2 = putite (intnum, bdd);
		if (v2 == NULL || v2 != ite_var (v2->variable) && v2!=false_ptr) {
			fprintf(stderr, "\nKeyword 'minmax' needs a positive integer as a second argument (%s)...exiting:%d\n", macros, markbdd_line);
			exit (1);
		}
		expect_integer = 0;
		if(v2 == false_ptr) max = 0;
		else max = v2->variable;
		//if(max<min) {
	   //  fprintf(stderr, "\nKeyword 'minmax' cannot have it's second argument (%d) less than it's first argument (%d)...exiting:%d\n", max, min, markbdd_line);
		//	exit (1);
		//}

		int maxarguments = 10;
		int numarguments = 0;
		int *var_list = (int *)ite_calloc(maxarguments, sizeof(int), 9, "var_list");

		BDDNode *v3;
		while((v3 = putite (intnum, bdd))!=NULL) {
			if(numarguments >= maxarguments) {
				var_list = (int *)ite_recalloc(var_list, maxarguments, maxarguments+10, sizeof(int), 9, "var_list");
				maxarguments+=10;
			}
			if (v3 == ite_var (v3->variable)) {
				var_list[numarguments++] = v3->variable;
			} else {	fprintf(stderr, "\nKeyword 'minmax' needs positive variables as arguments (%s)...exiting:%d\n", macros, markbdd_line); exit (1); }
		}
		if(max > numarguments) { fprintf(stderr, "\nKeyword 'minmax' cannot have less than the specified max=%d variables (%s)...exiting:%d\n", max, macros, markbdd_line); exit (1); }
			
		int set_true = 0;
		qsort(var_list, numarguments, sizeof(int), abscompfunc);
		for(int x = 0; x < numarguments-1; x++)
		  if(var_list[x] == var_list[x+1]) {
			  fprintf(stderr, "\nFound duplicate variables in the argument list for keyword 'minmax' (%s)...exiting:%d\n", macros, markbdd_line); exit(1); }
		
		strcpy (macros, "minmax");
		BDDNode *tempBDD = MinMaxBDD(var_list, min, max, numarguments, set_true);
		ite_free((void **)&var_list);
		return tempBDD;
	}
	if (!strcasecmp (macros, "countT")) {
		BDDNode *v1 = putite (intnum, bdd);
		if(v1==NULL) { fprintf(stderr, "\nKeyword 'countT' expects at least 1 argument, found 0 (%s)...exiting:%d\n", macros, markbdd_line); exit (1); }
		
		int maxarguments = 10;
		int numarguments = 0;
		int *var_list = (int *)ite_calloc(maxarguments, sizeof(int), 9, "var_list");

		BDDNode *v2;
		while((v2 = putite (intnum, bdd))!=NULL) {
			if(numarguments >= maxarguments) {
				var_list = (int *)ite_recalloc(var_list, maxarguments, maxarguments+10, sizeof(int), 9, "var_list");
				maxarguments+=10;
			}
			if (v2 == ite_var (v2->variable)) {
				var_list[numarguments++] = v2->variable;
			} else {	fprintf(stderr, "\nKeyword 'countT' needs positive variables as arguments (%s)...exiting:%d\n", macros, markbdd_line); exit (1); }
		}

		int set_true = 0;
		qsort(var_list, numarguments, sizeof(int), abscompfunc);
      strcpy (macros, "countT");
		int numT = 0;
		if(numarguments != 0) numT = count_true_paths(v1, var_list, numarguments);
		fprintf(stdout, "\n|count = %d|\n", numT);
		ite_free((void **)&var_list);
		return true_ptr;
	}
	if (!strcasecmp (macros, "exist")) {
		BDDNode * v1 = putite (intnum, bdd);
		if(v1 == NULL) { fprintf(stderr, "\nKeyword 'exist' expects at least 2 arguments, found 0 (%s)...exiting:%d\n", macros, markbdd_line); exit (1); }
      BDDNode * v2 = putite (intnum, bdd);
		if(v2 == NULL) { fprintf(stderr, "\nKeyword 'exist' expects at least 2 arguments, found 1 (%s)...exiting:%d\n", macros, markbdd_line); ;exit (1); }
		if (v2 != ite_var (v2->variable)) {
			fprintf(stderr, "\nKeyword 'exist' expects a list of positive variables following the first argument (%s)...exiting:%d\n", macros, markbdd_line);
			exit (1);
		}
		v1 = xquantify(v1, v2->variable);
		while((v2 = putite(intnum, bdd))!=NULL) {
			if (v2 != ite_var (v2->variable)) {
				fprintf(stderr, "\nKeyword 'exist' expects a list of positive variables following the first argument (%s)...exiting:%d\n", macros, markbdd_line);
				exit (1);
			}
			v1 = xquantify(v1, v2->variable);
		}
      strcpy (macros, "exist");
      return v1;
	}
	if (!strcasecmp (macros, "universe")) {
      BDDNode * v1 = putite (intnum, bdd);
		if(v1 == NULL) { fprintf(stderr, "\nKeyword 'universe' expects at least 2 arguments, found 0 (%s)...exiting:%d\n", macros, markbdd_line); exit (1); }
      BDDNode * v2 = putite (intnum, bdd);
		if(v2 == NULL) { fprintf(stderr, "\nKeyword 'universe' expects at least 2 arguments, found 1 (%s)...exiting:%d\n", macros, markbdd_line); exit (1); }		
      if (v2 != ite_var (v2->variable)) {
			fprintf(stderr, "\nKeyword 'universe' expects a list of  positive variable following the first (%s)...exiting:%d\n", macros, markbdd_line);
			exit (1);
		}
		v1 = uquantify (v1, v2->variable);
		while((v2 = putite(intnum, bdd))!=NULL) {
			if (v2 != ite_var (v2->variable)) {
				fprintf(stderr, "\nKeyword 'universe' expects a list of  positive variable following the first (%s)...exiting:%d\n", macros, markbdd_line);
				exit (1);
			}
			v1 = uquantify (v1, v2->variable);
		}
		strcpy (macros, "universe");
      return v1;
	}
	if (!strcasecmp (macros, "safe")) {
		BDDNode * v1 = putite (intnum, bdd);
		if(v1 == NULL) { fprintf(stderr, "\nKeyword 'safe' expects 2 arguments, found 0 (%s)...exiting:%d\n", macros, markbdd_line); exit (1); }
      BDDNode * v2 = putite (intnum, bdd);
		if(v2 == NULL) { fprintf(stderr, "\nKeyword 'safe' expects 2 arguments, found 1 (%s)...exiting:%d\n", macros, markbdd_line); exit (1); }		
      if (v2 != ite_var (v2->variable)) {
			fprintf(stderr, "\nKeyword 'safe' needs a positive variable as a second argument (%s)...exiting:%d\n", macros, markbdd_line);
			exit (1);
		}
		if(putite (intnum, bdd)!=NULL) { fprintf(stderr, "\nKeyword 'safe' expects 2 arguments, found > 2 (%s)...exiting:%d\n", macros, markbdd_line); exit (1); }
      strcpy (macros, "safe");
      if (v1 == false_ptr)
		  return false_ptr;
      return safe_assign (v1, v2->variable);
	}
	if (!strcasecmp (macros, "safe_eq")) {
		BDDNode * v1 = putite (intnum, bdd);
		if(v1 == NULL) { fprintf(stderr, "\nKeyword 'safe_eq' expects 2 arguments, found 0 (%s)...exiting:%d\n", macros, markbdd_line); exit (1); }
      BDDNode * v2 = putite (intnum, bdd);
		if(v2 == NULL) { fprintf(stderr, "\nKeyword 'safe_eq' expects 2 arguments, found 1 (%s)...exiting:%d\n", macros, markbdd_line); exit (1); }		
      if (v2 != ite_var (v2->variable)) {
			fprintf(stderr, "\nKeyword 'safe_eq' needs a positive variable as a second argument (%s)...exiting:%d\n", macros, markbdd_line);
			exit (1);
		}
		if(putite (intnum, bdd)!=NULL) { fprintf(stderr, "\nKeyword 'safe_eq' expects 2 arguments, found > 2 (%s)...exiting:%d\n", macros, markbdd_line); exit (1); }
      strcpy (macros, "safe_eq");
      if (v1 == false_ptr)
		  return false_ptr;
      return safe_assign_eq (v1, v2->variable);
	}
	if (!strcasecmp (macros, "safe_func")) {
		BDDNode * v1 = putite (intnum, bdd);
		if(v1 == NULL) { fprintf(stderr, "\nKeyword 'safe_func' expects 2 arguments, found 0 (%s)...exiting:%d\n", macros, markbdd_line); exit (1); }
      BDDNode * v2 = putite (intnum, bdd);
		if(v2 == NULL) { fprintf(stderr, "\nKeyword 'safe_func' expects 2 arguments, found 1 (%s)...exiting:%d\n", macros, markbdd_line); exit (1); }
      if (v2 != ite_var (v2->variable)) {
			fprintf(stderr, "\nKeyword 'safe_func' needs a positive variable as a second argument (%s)...exiting:%d\n", macros, markbdd_line);
			exit (1);
		}
		if(putite (intnum, bdd)!=NULL) { fprintf(stderr, "\nKeyword 'safe_func' expects 2 arguments, found > 2 (%s)...exiting:%d\n", macros, markbdd_line); exit (1); }
      strcpy (macros, "safe_func");
      if (v1 == false_ptr)
		  return false_ptr;
      return safe_assign_func (v1, v2->variable);
	}
	if (!strcasecmp (macros, "remove_fps")) {
		BDDNode * v1 = putite (intnum, bdd);
		if(v1 == NULL) { fprintf(stderr, "\nKeyword 'remove_fps' expects 2 arguments, found 0 (%s)...exiting:%d\n", macros, markbdd_line); exit (1); }
      BDDNode * v2 = putite (intnum, bdd);
		if(v2 == NULL) { fprintf(stderr, "\nKeyword 'remove_fps' expects 2 arguments, found 1 (%s)...exiting:%d\n", macros, markbdd_line); exit (1); }		
		if(putite (intnum, bdd)!=NULL) { fprintf(stderr, "\nKeyword 'remove_fps' expects 2 arguments, found > 2 (%s)...exiting:%d\n", macros, markbdd_line); exit (1); }
      strcpy (macros, "remove_fps");
      return remove_fps (v1, v2);
	}
	if (!strcasecmp (macros, "pprint_tree")) {
      BDDNode * v1;
      v1 = putite (intnum, bdd);
		if(v1==NULL) { fprintf(stderr, "\nKeyword 'pprint_tree' expects 1 argument, found 0 (%s)...exiting:%d\n", macros, markbdd_line); exit (1); }
		if(putite (intnum, bdd)!=NULL) { fprintf(stderr, "\nKeyword 'pprint_tree' expects 1 argument, found > 1 (%s)...exiting:%d\n", macros, markbdd_line); exit (1); }
		d2_printf1("\n");
		for (int i = 0; i < PRINT_TREE_WIDTH; i++)
		  d2_printf1("-");
		d2_printf1("\n");
      print_bdd (v1);
		d2_printf1("\n");
      strcpy (macros, "pprint_tree");
      return v1;
	}
	if (!strcasecmp (macros, "print_tree")) {
      BDDNode * v1;
      int which_zoom = 0;
      v1 = putite (intnum, bdd);
		if(v1==NULL) { fprintf(stderr, "\nKeyword 'print_tree' expects 1 argument, found 0 (%s)...exiting:%d\n", macros, markbdd_line); exit (1); }
		if(putite (intnum, bdd)!=NULL) { fprintf(stderr, "\nKeyword 'print_tree' expects 1 argument, found > 1 (%s)...exiting:%d\n", macros, markbdd_line); exit (1); }
      printBDDTree (v1, &which_zoom);
      strcpy (macros, "print_tree");
      return v1;
	}
	if (!strcasecmp (macros, "print_xdd")) {
      BDDNode * v1;
      v1 = putite (intnum, bdd);
		if(v1==NULL) { fprintf(stderr, "\nKeyword 'print_xdd' expects 1 argument, found 0 (%s)...exiting:%d\n", macros, markbdd_line); exit (1); }
		if(putite (intnum, bdd)!=NULL) { fprintf(stderr, "\nKeyword 'print_xdd' expects 1 argument, found > 1 (%s)...exiting:%d\n", macros, markbdd_line); exit (1); }
		d2_printf1("\n");
		for (int i = 0; i < PRINT_TREE_WIDTH; i++)
		  d2_printf1("-");
		d2_printf1("\n");
		print_xdd_d(bdd2xdd(v1));
		d2_printf1(" = 1\n");
      strcpy (macros, "print_xdd");
      return v1;
	}
	if (!strcasecmp (macros, "print_flat_xdd")) {
      BDDNode * v1;
      v1 = putite (intnum, bdd);
		if(v1==NULL) { fprintf(stderr, "\nKeyword 'print_flat_xdd' expects 1 argument, found 0 (%s)...exiting:%d\n", macros, markbdd_line); exit (1); }
		if(putite (intnum, bdd)!=NULL) { fprintf(stderr, "\nKeyword 'print_flat_xdd' expects 1 argument, found > 1 (%s)...exiting:%d\n", macros, markbdd_line); exit (1); }
		d2_printf1("\n");
		for (int i = 0; i < PRINT_TREE_WIDTH; i++)
		  d2_printf1("-");
		d2_printf1("\n");
		int y = 0;
		int tempint_max = 0;
		int *tempint=NULL;
		unravelBDD(&y, &tempint_max, &tempint, v1);
		if(tempint!=NULL) ite_free((void **)&tempint);
		if(y == 0) { d2_printf1("0"); }
		else { print_flat_xdd(bdd2xdd(v1), y); }
		d2_printf1(" = 0\n");
      strcpy (macros, "print_flat_xdd");
      return v1;
	}
	if (!strcasecmp (macros, "print_smurf_dot_stdout")) {
		BDDNode *v1 = putite(intnum, bdd);
		if(v1 == NULL) { fprintf(stderr, "\nKeyword 'print_smurf_dot_stdout' expects at least 1 argument, found 0 (%s)...exiting:%d\n", macros, markbdd_line); exit (1); }

		int maxarguments = 10;
		int numarguments = 0;
		BDDNode **put_dot_bdds = (BDDNode **)ite_calloc(maxarguments, sizeof(BDDNode *), 9, "put_dot_bdds");
		put_dot_bdds[0] = v1;
		numarguments++;
		
		while ((v1 = putite (intnum, bdd))!=NULL) {
			if(numarguments >= maxarguments) {
				put_dot_bdds = (BDDNode **)ite_recalloc(put_dot_bdds, maxarguments, maxarguments+10, sizeof(BDDNode *), 9, "put_dot_bdds");
				maxarguments+=10;
			}
			put_dot_bdds[numarguments++] = v1;			
		}
      strcpy (macros, "print_smurf_dot_stdout");
		PrintSmurfs(put_dot_bdds, numarguments);
		ite_free((void**)&put_dot_bdds);
      return true_ptr;
	}
	if (!strcasecmp (macros, "print_dot_stdout")) {
		BDDNode *v1 = putite(intnum, bdd);
		if(v1 == NULL) { fprintf(stderr, "\nKeyword 'print_dot_stdout' expects at least 1 argument, found 0 (%s)...exiting:%d\n", macros, markbdd_line); exit (1); }

		int maxarguments = 10;
		int numarguments = 0;
		BDDNode **put_dot_bdds = (BDDNode **)ite_calloc(maxarguments, sizeof(BDDNode *), 9, "put_dot_bdds");
		put_dot_bdds[0] = v1;
		numarguments++;
		
		while ((v1 = putite (intnum, bdd))!=NULL) {
			if(numarguments >= maxarguments) {
				put_dot_bdds = (BDDNode **)ite_recalloc(put_dot_bdds, maxarguments, maxarguments+10, sizeof(BDDNode *), 9, "put_dot_bdds");
				maxarguments+=10;
			}
			put_dot_bdds[numarguments++] = v1;			
		}
      strcpy (macros, "print_dot_stdout");
		printBDDdot_stdout(put_dot_bdds, numarguments);
		ite_free((void**)&put_dot_bdds);
      return true_ptr;
	}
	if (!strcasecmp (macros, "print_dot")) {
		BDDNode *v1 = putite(intnum, bdd);
		if(v1 == NULL) { fprintf(stderr, "\nKeyword 'print_dot' expects at least 1 argument, found 0 (%s)...exiting:%d\n", macros, markbdd_line); exit (1); }

		int maxarguments = 10;
		int numarguments = 0;
		BDDNode **put_dot_bdds = (BDDNode **)ite_calloc(maxarguments, sizeof(BDDNode *), 9, "put_dot_bdds");
		put_dot_bdds[0] = v1;
		numarguments++;
		
		while ((v1 = putite (intnum, bdd))!=NULL) {
			if(numarguments >= maxarguments) {
				put_dot_bdds = (BDDNode **)ite_recalloc(put_dot_bdds, maxarguments, maxarguments+10, sizeof(BDDNode *), 9, "put_dot_bdds");
				maxarguments+=10;
			}
			put_dot_bdds[numarguments++] = v1;			
		}
      strcpy (macros, "print_dot");
		printBDDdot_file(put_dot_bdds, numarguments);
		ite_free((void**)&put_dot_bdds);
      return true_ptr;
	}
	if (!strcasecmp (macros, "same")) {
		BDDNode *v1 = putite(intnum, bdd);
		if(v1 == NULL) { fprintf(stderr, "\nKeyword 'same' expects at least 2 arguments, found 0 (%s)...exiting:%d\n", macros, markbdd_line); exit (1); }
		BDDNode *v2 = putite(intnum, bdd);
		if(v2 == NULL) { fprintf(stderr, "\nKeyword 'same' expects at least 2 arguments, found 1 (%s)...exiting:%d\n", macros, markbdd_line); exit (1); }
		v2 = ite_equ(v1, v2);
		BDDNode *v3;
		while ((v3 = putite (intnum, bdd))!=NULL)
			v2 = ite_and(v2, ite_equ(v1, v3));
		strcpy (macros, "same");
		return v2;
	}
	if (!strcasecmp (macros, "gcf")) {
      BDDNode * v1, *v2;
		v1 = putite (intnum, bdd);
		if(v1 == NULL) { fprintf(stderr, "\nKeyword 'gcf' expects 2 arguments, found 0 (%s)...exiting:%d\n", macros, markbdd_line); exit (1); }
      v2 = putite (intnum, bdd);
		if(v2 == NULL) { fprintf(stderr, "\nKeyword 'gcf' expects 2 arguments, found 1 (%s)...exiting:%d\n", macros, markbdd_line); exit (1); }
		if(putite (intnum, bdd)!=NULL) { fprintf(stderr, "\nKeyword 'gcf' expects 2 arguments, found > 2 (%s)...exiting:%d\n", macros, markbdd_line); exit (1); }
      strcpy (macros, "gcf");
      if (v1 == false_ptr)
		  return false_ptr;
      return gcf (v1, v2);
	}
	if (!strcasecmp (macros, "prune")) { //Same as restrict, below
		BDDNode * v1, *v2;
		v1 = putite (intnum, bdd);
		if(v1 == NULL) { fprintf(stderr, "\nKeyword 'prune' expects 2 arguments, found 0 (%s)...exiting:%d\n", macros, markbdd_line); exit (1); }
		v2 = putite (intnum, bdd);
		if(v2 == NULL) { fprintf(stderr, "\nKeyword 'prune' expects 2 arguments, found 1 (%s)...exiting:%d\n", macros, markbdd_line); exit (1); }
		if(putite (intnum, bdd)!=NULL) { fprintf(stderr, "\nKeyword 'prune' expects 2 arguments, found > 2 (%s)...exiting:%d\n", macros, markbdd_line); exit (1); }
		strcpy (macros, "prune");
		if (v1 == false_ptr)
		  return false_ptr;
		return pruning_p2 (v1, v2);
	}
	if (!strcasecmp (macros, "restrict")) { //Same as prune, above
		BDDNode * v1, *v2;
		v1 = putite (intnum, bdd);
		if(v1 == NULL) { fprintf(stderr, "\nKeyword 'restrict' expects 2 arguments, found 0 (%s)...exiting:%d\n", macros, markbdd_line); exit (1); }
		v2 = putite (intnum, bdd);
		if(v2 == NULL) { fprintf(stderr, "\nKeyword 'restrict' expects 2 arguments, found 1 (%s)...exiting:%d\n", macros, markbdd_line); exit (1); }
		if(putite (intnum, bdd)!=NULL) { fprintf(stderr, "\nKeyword 'restrict' expects 2 arguments, found > 2 (%s)...exiting:%d\n", macros, markbdd_line); exit (1); }
		strcpy (macros, "restrict");
		if (v1 == false_ptr)
		  return false_ptr;
		return pruning_p2 (v1, v2);
	}
	if (!strcasecmp (macros, "strengthen")) {
		BDDNode * v1, *v2;
		v1 = putite (intnum, bdd);
		if(v1 == NULL) { fprintf(stderr, "\nKeyword 'strengthen' expects 2 arguments, found 0 (%s)...exiting:%d\n", macros, markbdd_line); exit (1); }
		v2 = putite (intnum, bdd);
		if(v2 == NULL) { fprintf(stderr, "\nKeyword 'strengthen' expects 2 arguments, found 1 (%s)...exiting:%d\n", macros, markbdd_line); exit (1); }
		if(putite (intnum, bdd)!=NULL) { fprintf(stderr, "\nKeyword 'strengthen' expects 2 arguments, found > 2 (%s)...exiting:%d\n", macros, markbdd_line); exit (1); }
		strcpy (macros, "strengthen");
		if (v1 == false_ptr)
		  return false_ptr;
		return strengthen_fun (v1, v2);
	}
	if (!strcasecmp (macros, "nimp")) {
		BDDNode *v1 = putite(intnum, bdd);
		if(v1 == NULL) { fprintf(stderr, "\nKeyword 'nimp' expects at least 2 arguments, found 0 (%s)...exiting:%d\n", macros, markbdd_line); exit (1); }
		BDDNode *v2 = putite(intnum, bdd);
		if(v2 == NULL) { fprintf(stderr, "\nKeyword 'nimp' expects at least 2 arguments, found 1 (%s)...exiting:%d\n", macros, markbdd_line); exit (1); }
		v1 = ite_nimp(v1, v2);
		while ((v2 = putite (intnum, bdd))!=NULL)
			v1 = ite_nimp(v1, v2);
		strcpy (macros, "nimp");
		return v1;
	}
	if (!strcasecmp (macros, "nand")) {
		BDDNode *v1 = putite(intnum, bdd);
		if(v1 == NULL) { fprintf(stderr, "\nKeyword 'nand' expects at least 2 arguments, found 0 (%s)...exiting:%d\n", macros, markbdd_line); exit (1);	}
		BDDNode *v2 = putite(intnum, bdd);
		if(v2 == NULL) { fprintf(stderr, "\nKeyword 'nand' expects at least 2 arguments, found 1 (%s)...exiting:%d\n", macros, markbdd_line); exit (1); }
		v1 = ite_and(v1, v2);
		while ((v2 = putite (intnum, bdd))!=NULL)
			v1 = ite_and(v1, v2);
		strcpy (macros, "nand");
		return ite_not(v1);
	}
	if (!strcasecmp (macros, "nor")) {
		BDDNode *v1 = putite(intnum, bdd);
		if(v1 == NULL) { fprintf(stderr, "\nKeyword 'nor' expects at least 2 arguments, found 0 (%s)...exiting:%d\n", macros, markbdd_line); exit (1); }
		BDDNode *v2 = putite(intnum, bdd);
		if(v2 == NULL) { fprintf(stderr, "\nKeyword 'nor' expects at least 2 arguments, found 1 (%s)...exiting:%d\n", macros, markbdd_line); exit (1); }
		v1 = ite_or(v1, v2);
		while ((v2 = putite (intnum, bdd))!=NULL)
		  v1 = ite_or(v1, v2);
		strcpy (macros, "nor");
		return ite_not(v1);
	}
	if (!strcasecmp (macros, "equ") || !strcasecmp (macros, "noxor")) {
		BDDNode *v1 = putite(intnum, bdd);
		if(v1 == NULL) { fprintf(stderr, "\nKeyword 'equ' expects at least 2 arguments, found 0 (%s)...exiting:%d\n", macros, markbdd_line); exit (1); }
		BDDNode *v2 = putite(intnum, bdd);
		if(v2 == NULL) { fprintf(stderr, "\nKeyword 'equ' expects at least 2 arguments, found 1 (%s)...exiting:%d\n", macros, markbdd_line); exit (1); }
		v1 = ite_equ(v1, v2);
		while ((v2 = putite (intnum, bdd))!=NULL)
		  v1 = ite_equ(v1, v2);
		strcpy (macros, "equ");
		return v1;
	}
	if (!strcasecmp (macros, "imp")) {
		BDDNode *v1 = putite(intnum, bdd);
		if(v1 == NULL) { fprintf(stderr, "\nKeyword 'imp' expects at least 2 arguments, found 0 (%s)...exiting:%d\n", macros, markbdd_line); exit (1); }
		BDDNode *v2 = putite(intnum, bdd);
		if(v2 == NULL) { fprintf(stderr, "\nKeyword 'imp' expects at least 2 arguments, found 1 (%s)...exiting:%d\n", macros, markbdd_line); exit (1); }
		v1 = ite_imp(v1, v2);
		while ((v2 = putite (intnum, bdd))!=NULL)
		  v1 = ite_imp(v1, v2);
		strcpy (macros, "imp");
		return v1;
	}
	if (!strcasecmp (macros, "and")) {
		BDDNode *v1 = putite(intnum, bdd);
		if(v1 == NULL) { fprintf(stderr, "\nKeyword 'and' expects at least 2 arguments, found 0 (%s)...exiting:%d\n", macros, markbdd_line); exit (1); }
		BDDNode *v2 = putite(intnum, bdd);
		if(v2 == NULL) { fprintf(stderr, "\nKeyword 'and' expects at least 2 arguments, found 1 (%s)...exiting:%d\n", macros, markbdd_line); exit (1); }
		v1 = ite_and(v1, v2);
		while ((v2 = putite (intnum, bdd))!=NULL)
		  v1 = ite_and(v1, v2);
		strcpy (macros, "and");
		return v1;
	}
	if (!strcasecmp (macros, "xor")) {
		BDDNode *v1 = putite(intnum, bdd);
		if(v1 == NULL) { fprintf(stderr, "\nKeyword 'xor' expects at least 2 arguments, found 0 (%s)...exiting:%d\n", macros, markbdd_line); exit (1); }
		BDDNode *v2 = putite(intnum, bdd);
		if(v2 == NULL) { fprintf(stderr, "\nKeyword 'xor' expects at least 2 arguments, found 1 (%s)...exiting:%d\n", macros, markbdd_line); exit (1); }
		v1 = ite_xor(v1, v2);
		while ((v2 = putite (intnum, bdd))!=NULL)
			v1 = ite_xor(v1, v2);
		strcpy (macros, "xor");
		return v1;
	}
	if (!strcasecmp (macros, "or")) {
		BDDNode *v1 = putite(intnum, bdd);
		if(v1 == NULL) { fprintf(stderr, "\nKeyword 'or' expects at least 2 arguments, found 0 (%s)...exiting:%d\n", macros, markbdd_line); exit (1); }
		BDDNode *v2 = putite(intnum, bdd);
		if(v2 == NULL) { fprintf(stderr, "\nKeyword 'or' expects at least 2 arguments, found 1 (%s)...exiting:%d\n", macros, markbdd_line); exit (1); }
		v1 = ite_or(v1, v2);
		while ((v2 = putite (intnum, bdd))!=NULL)
			v1 = ite_or(v1, v2);
		strcpy (macros, "or");
		return v1;
	}

	int v = i_getsym(macros, SYM_VAR);

	if(v >= numinp-2) {
		fprintf(stderr, "\nToo many symbols used (%s). Need to increase to greater than %ld...exiting:%d\n", macros, numinp-3, markbdd_line);
		exit (1);
	}

	if(negate_it == 1) {
		negate_it = 0;
		return ite_var(-v);
	} else return ite_var(v);

	fprintf(stderr, "\nUnknown word (%s)...exiting:%d\n", macros,	markbdd_line);
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
			fprintf(stderr, "Formula parse error...exiting:%d\n", markbdd_line);
			exit (1);
		}
		if (p == ')') {
			return ')';
		}
      if (p == ';') {
			while (p != '\n') {
            p = fgetc(finputfile);
            if (p == EOF) {
					fprintf(stderr, "\nUnexpected EOF...exiting:%d\n", markbdd_line);
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
					fprintf(stderr, "\nUnexpected EOF...exiting:%d\n", markbdd_line);
					exit (1);
				}
			}
			if (p == '-') {
				fprintf(stderr, "\nDouble '-' found, variables cannot start with a '-'...exiting:%d\n", markbdd_line);
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
					fprintf(stderr, "\nUnexpected EOF (%s)...exiting:%d\n",	macros, markbdd_line);
					exit (1);
				}
			}
			ungetc (p, finputfile);

			macros[i] = 0;
			if(is_int == 1 && expect_integer == 1) {
				expect_integer = 0;
				intnum = atoi (macros);
				/*if (intnum > numinp) {
					fprintf(stderr, "\nVariable %d is larger than allowed (%ld)...exiting:%d\n",	intnum, numinp - 2, markbdd_line);
					exit (1);
				}*/
				if (negate_it) intnum = -intnum;
				return 'i';
			} else if(is_int == 0 && expect_integer == 1) {					  
				fprintf(stderr, "\nExpecting an integer, found %s...exiting:%d\n", macros, markbdd_line);
				exit(1);
			} else {
				if(!strcasecmp(macros, "define")) {
					fprintf(stderr, "\n'#' expected in front of keyword 'define'...exiting:%d\n", markbdd_line);
					exit (1);
				}
			}
			if(negate_it) d5_printf2("-%s ", macros);
			if(!negate_it) d5_printf2("%s ", macros);
			return 'm';
		}
      if (p == '#') {
         p = fgetc(finputfile);
         if (p == EOF) {
				fprintf(stderr, "\nUnexpected EOF...exiting:%d\n", markbdd_line);
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
						fprintf(stderr, "\nUnexpected EOF (%s)...exiting:%d\n",	macros, markbdd_line);
						exit (1);
					}
				}
				ungetc (p, finputfile);
				macros[i] = 0;
				if(strcasecmp(macros, "define")) {
					fprintf(stderr, "\n#define expected, found %s...exiting:%d\n", macros, markbdd_line);
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
				fprintf(stderr, "\nUnexpected EOF (%s)...exiting:%d\n", macros, markbdd_line);
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
						fprintf(stderr, "\nUnexpected EOF (%s)...exiting:%d\n",	integers, markbdd_line);
						exit (1);
					}
				}
				integers[i] = 0;
				intnum = atoi (integers) - 1;
				if (intnum > nmbrFunctions - 1) {
					fprintf(stderr, "\nFunction $%d has not yet been defined...exiting:%d\n", intnum + 1, markbdd_line);
					exit (1);
				}
				ungetc (p, finputfile);
				bdd = functions[intnum];
				return 'b';
			} else {
				fprintf(stderr, "\nFormula parse error...exiting:%d\n",	markbdd_line);
				exit (1);
			}
		}
	}
}

void bddloop () {
	fscanf (finputfile, "%ld %ld", &numinp, &numout);

	numinp += 3;
	markbdd_line = 1;
	int intnum = 0, keepnum = 0;
	BDDNode *bdd = NULL;
	int p = 0;

	max_initbranch = 1;
	max_varlevel = 1;
	initbranch = (initbranch_struct *)ite_recalloc(initbranch, 0, max_initbranch, sizeof(initbranch_struct), 9, "initbranch");
	initbranch[0].branch_level = 0;
	initbranch[0].max_initbranch_level = 10;
	initbranch[0].num_initbranch_level = 0;
	initbranch[0].vars = (initbranch_level *)ite_recalloc(initbranch[0].vars, 0, initbranch[0].max_initbranch_level, sizeof(initbranch_level), 9, "initbranch[0].vars");
	
	totaldefines = 0;
	max_defines = 100;
	defines = (defines_struct *)ite_recalloc(defines, 0, max_defines, sizeof(defines_struct), 9, "defines");
	
	max_integers = 20;
	integers = (char *)ite_recalloc(integers, 0, max_integers, sizeof(char), 9, "integers");
	
	max_macros = 20;
	macros = (char *)ite_recalloc(macros, 0, max_macros, sizeof(char), 9, "macros");
	
   vars_alloc(numinp+2);
   functions_alloc(numout+2);
	
	int *keep = (int *)ite_calloc(numout + 2, sizeof(int), 9, "keep");

	p = fgetc(finputfile);
	 
	d4_printf1("\n");
	
	while (1) {		//(p = fgetc(finputfile))!=EOF) 
		if(markbdd_line == 1) { d2_printf1("Reading 2"); }
      else {
			d2e_printf2("\rReading %d ", markbdd_line);
			d4_printf2("Reading %d ", markbdd_line);
		}
		if (p == '\n') {
			p = fgetc(finputfile);
			if(p == EOF) {
				goto Exit;
			}
			markbdd_line++;
			d4_printf1("\r");
			continue;
		}
      if (p == ';') {
			while (p != '\n') {
            p = fgetc(finputfile);
				if (p == EOF) {
					goto Exit;
				}
			}
			d4_printf1("\r");
			continue;
		}
		if (p == '	' || p == ' ' || p == ')' || p == '(') {
			while (p == '	' || p == ' ' || p == ')' || p == '(') {
            p = fgetc(finputfile);
				if (p == EOF) {
					goto Exit;
				}
			}
//			p = fgetc(finputfile);
			if(p == EOF) {
				goto Exit;
			}
				  
			d4_printf1("\r");
			continue;
		}
		if (p == '*') {
			BDDNode *temp = putite(intnum, bdd);
			if(temp == NULL) goto Exit;
			if ((strcasecmp (macros, "pprint_tree"))
				 && (strncasecmp (macros, "print_", 6))
				 && (strcasecmp (macros, "define"))
				 && (strcasecmp (macros, "order"))
				 && (strcasecmp (macros, "countT"))
				 && (strcasecmp (macros, "initial_branch"))) {
				keep[nmbrFunctions] = 1;
				keepnum++;
				functions[nmbrFunctions] = temp;
				nmbrFunctions++;
				if (nmbrFunctions > numout) {
					fprintf(stderr, "Too many functions, increase to larger than %ld...exiting:%d",	numout, markbdd_line);
					exit (1);
				}
			}
			d4_printf1("*");
		} else {
			ungetc (p, finputfile);
			if (p == ';')
			  continue;
			BDDNode * temp = putite (intnum, bdd);
			if(temp == NULL) goto Exit;
			if ((strcasecmp (macros, "pprint_tree"))
				 && (strncasecmp (macros, "print_", 6))
				 && (strcasecmp (macros, "define"))
				 && (strcasecmp (macros, "order"))
				 && (strcasecmp (macros, "countT"))
				 && (strcasecmp (macros, "initial_branch"))) {
				functions[nmbrFunctions] = temp;
				nmbrFunctions++;
				if (nmbrFunctions > numout) {
					fprintf(stderr, "Too many functions, increase to larger than %ld...exiting:%d",	numout, markbdd_line);
					exit (1);
				}
			}
		}
      D_4(
      if ((strcasecmp (macros, "pprint_tree"))
			 && (strncasecmp (macros, "print_", 6))
			 && (strcasecmp (macros, "define"))
			 && (strcasecmp (macros, "order"))
			 && (strcasecmp (macros, "countT"))
			 && (strcasecmp (macros, "initial_branch"))) {
			fprintf(stddbg, "BDD $%d: ", nmbrFunctions);
			printBDDfile (functions[nmbrFunctions - 1], stddbg);
			//fprintf(stddbg, "\n");
		}
      )

		p = fgetc (finputfile);
      if (p != '\n') {
			int continue_all = 0;
			if(p==';') continue_all = 1;
			else if (p!='	' && p!='\r' && p!=' ' && p!='(' && p!=')' && p!=',') {
				fprintf(stderr, "Error: Extra characters following line %d...exiting\n", markbdd_line);
				exit (1);
				continue_all = 1;
			}
         p = fgetc(finputfile);
         while (p != EOF) {
				if (p == '\n')
				  break;
				if (p == ';') continue_all = 1;
				if (continue_all == 0 && p!='	' && p!='\r' && p!=' ' && p!='(' && p!=')' && p!=',') {
					fprintf(stderr, "Error: Extra characters following line %d...exiting\n", markbdd_line);
					exit (1);
					continue_all = 1;
				}
            p = fgetc(finputfile);
         }
			if (p != '\n')
			  goto Exit;
		}
		d4_printf1("\n");
	}
	Exit:;

	int *initbranch_used = (int *)ite_calloc(numinp + 2, sizeof(int), 9, "initbranch_used");

	arrVarChoiceLevels = (int **)ite_calloc(max_varlevel, sizeof(int *), 9, "arrVarChoiceLevels");
	arrVarTrueInfluences = (double *)ite_calloc(numinp+1, sizeof(double), 9, "arrVarTrueInfluences");

	for(int i = 0; i < numinp+1; i++)
	  arrVarTrueInfluences[i] = 0.5;
	
	//Need to sort initbranch based on initbranch[x].branch_level
	//qsort(initbranch, max_initbranch, sizeof(initbranch_struct), initbranch_compfunc);
	
	for(int i = 0; i < max_initbranch; i++) {
		int max_CLevels = initbranch[i].num_initbranch_level+1;
		arrVarChoiceLevels[initbranch[i].branch_level] = (int *)ite_recalloc(arrVarChoiceLevels[initbranch[i].branch_level], 0, max_CLevels, sizeof(int), 9, "arrVarChoiceLevels[i]");
		//arrVarChoiceLevels[i] = (int *)ite_calloc(sizeof(int), 9, "arrVarChoiceLevels[i]");
		int nVarChoiceIter = 0;
		d9_printf3("[%d [%s]]", i, initbranch[i].vars[0].string);
		for(int x = 0; x < initbranch[i].num_initbranch_level; x++) {
			t_myregex myrg;
			sym_regex_init(&myrg, initbranch[i].vars[x].string);
			int id = sym_regex(&myrg);
			int looper = 0;
			while (id) {
				looper++;
				if(initbranch_used[id] == 0) {
					// found variable and the variable id is id
					initbranch_used[id] = 1;
					arrVarTrueInfluences[id] = initbranch[i].vars[x].true_inf_weight;
					arrVarChoiceLevels[initbranch[i].branch_level][nVarChoiceIter++] = id;
					//d5_printf5("%d indep=%d %s %s\n", looper, id, getsym_i(id)->name, initbranch[i].vars[x].string);
					d5_printf7("%d %d %s %s priority=%d true_inf=%4.6f\n", looper, id, getsym_i(id)->name, initbranch[i].vars[x].string, initbranch[i].branch_level, initbranch[i].vars[x].true_inf_weight);
					if(nVarChoiceIter >= max_CLevels) {
						arrVarChoiceLevels[initbranch[i].branch_level] = (int *)ite_recalloc(arrVarChoiceLevels[initbranch[i].branch_level], max_CLevels, max_CLevels+10, sizeof(int), 9, "arrVarChoiceLevels[i]");
						max_CLevels += 10;
					}
				}
				id = sym_regex(&myrg);
			}
			sym_regex_free(&myrg);
		}
		arrVarChoiceLevels[initbranch[i].branch_level][nVarChoiceIter] = 0;
	}

	if(nForceBackjumpLevel >= max_varlevel) {
		fprintf(stderr, "\n'--force-backjump-level %d' does not correspond to a level specified by 'initial_branch'...exiting\n", nForceBackjumpLevel);
		exit(1);
	}
	
	//Remove blank levels
	int count=-1;
	for(int x = 0; x < max_varlevel; x++) {
		count++;
		int *tmp = arrVarChoiceLevels[count];
		arrVarChoiceLevels[count] = arrVarChoiceLevels[x];
		if(!arrVarChoiceLevels[x] || arrVarChoiceLevels[x][0] == 0) {
			if(nForceBackjumpLevel == x) {
				fprintf(stderr, "\n'--force-backjump-level %d' does not correspond to a level specified by 'initial_branch'...exiting\n", nForceBackjumpLevel);
				exit(1);
			}
			count--;
		}
		if(nForceBackjumpLevel == x) { nForceBackjumpLevel = count; d9_printf2("nFBL = %d\n", nForceBackjumpLevel);}
		arrVarChoiceLevels[x] = tmp;
	}

	nVarChoiceLevelsMax = max_initbranch;
	nVarChoiceLevelsNum = count+1;

	//Also we must take equivalences into account in the preprocessor!
	
	if (keepnum == 0)
	  for (int x = 0; x < nmbrFunctions; x++)
		 keep[x] = 1;

	count = -1;
	
	//(x <= numout) means to throw away functions that come after
	//the cutoff part numout.
	for (int x = 0; (x < nmbrFunctions) && (x <= numout); x++) {
		count++;
      functions[count] = functions[x];
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
	for(int i = 0; i < max_initbranch; i++) {
		for(int x = 0; x < initbranch[i].num_initbranch_level; x++)
		  delete [] initbranch[i].vars[x].string;
		ite_free((void **)&initbranch[i].vars);
	}
	ite_free((void **)&initbranch);
	ite_free((void **)&initbranch_used);
	ite_free((void **)&keep);
	for(int x = 0; x < totaldefines; x++) {
	  ite_free((void **)&defines[x].string);
	  ite_free((void **)&defines[x].vlist);
	}
	ite_free((void **)&defines);
	ite_free((void **)&integers);
	ite_free((void **)&macros);
}
