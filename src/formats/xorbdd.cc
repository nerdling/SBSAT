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

struct xor_of_ands {
	int *vars_in_and;
	int curr_length;
	xor_of_ands *next_and;
};

int are_xdd_comp(BDDNode *x, BDDNode *y);

void print_xdd_d(BDDNode *xdd) {
	if(xdd == true_ptr) {
		d2_printf1("1");
	} else if(xdd->thenCase == xdd->elseCase) {
//		d2_printf1("(");
//		d2e_printf2("x[%s] + 1", s_name(xdd->variable));
//		d3e_printf2("x[%d] + 1", xdd->variable);
//		d4_printf3("x[%s(%d)] + 1", s_name(xdd->variable), xdd->variable);
//		d2_printf1(")");

		d2e_printf2("-x[%s]", s_name(xdd->variable));
		d3e_printf2("-x[%d]", xdd->variable);
		d4_printf3("-x[%s(%d)]", s_name(xdd->variable), xdd->variable);
		
		if(xdd->thenCase != true_ptr) {
			d2_printf1("*(");
			print_xdd_d(xdd->thenCase);
			d2_printf1(")");
		}
	} else if(!IS_TRUE_FALSE(xdd->elseCase) && are_xdd_comp(xdd->thenCase, xdd->elseCase)) {
//		d2_printf1("(");
//		d2e_printf2("x[%s] + 1", s_name(xdd->variable));
//		d3e_printf2("x[%d] + 1", xdd->variable);
//		d4_printf3("x[%s(%d)] + 1", s_name(xdd->variable), xdd->variable);
//		d2_printf1(")");

		d2e_printf2("-x[%s]", s_name(xdd->variable));
		d3e_printf2("-x[%d]", xdd->variable);
		d4_printf3("-x[%s(%d)]", s_name(xdd->variable), xdd->variable);
		
		if(xdd->thenCase != true_ptr) {
			d2_printf1("*(");
			print_xdd_d(xdd->thenCase); //MUST BE THENCASE (I think)
			d2_printf1(")");
		}
		d2_printf1(" + 1");
	} else {
		if(!IS_TRUE_FALSE(xdd->thenCase)) {
			d2e_printf2("x[%s]*(", s_name(xdd->variable));
			d3e_printf2("x[%d]*(", xdd->variable);
			d4_printf3("x[%s(%d)]*(", s_name(xdd->variable), xdd->variable);
			
			print_xdd_d(xdd->thenCase);
			d2_printf1(")");
		} else {
			d2e_printf2("x[%s]", s_name(xdd->variable));
			d3e_printf2("x[%d]", xdd->variable);
			d4_printf3("x[%s(%d)]", s_name(xdd->variable), xdd->variable);
		}	
		if(xdd->elseCase == true_ptr) {
			d2_printf1(" + 1");
		} else if(xdd->elseCase!=false_ptr) {
			d2_printf1(" + ");
			print_xdd_d(xdd->elseCase);
		}
	}
}

void print_xdd(BDDNode *xdd) {
	if(xdd == true_ptr) fprintf(foutputfile, "1");
	if(!IS_TRUE_FALSE(xdd->thenCase)) {
		fprintf(foutputfile, "x[%d]*(", xdd->variable);
		print_xdd(xdd->thenCase);
		fprintf(foutputfile, ")");
	} else {
		fprintf(foutputfile, "x[%d]", xdd->variable);
	}
	if(xdd->elseCase == true_ptr)
	  fprintf(foutputfile, " + 1");
	else if(xdd->elseCase!=false_ptr) {
		fprintf(foutputfile, " + ");
		print_xdd(xdd->elseCase);
	}
}

void print_xdd_err(BDDNode *xdd){
	if(xdd == true_ptr) fprintf(stderr, "1");
	if(!IS_TRUE_FALSE(xdd->thenCase)) {
		fprintf(stderr, "x[%d]*(", xdd->variable);
		print_xdd_err(xdd->thenCase);
		fprintf(stderr, ")");
	} else {
		fprintf(stderr, "x[%d]", xdd->variable);
	}
	if(xdd->elseCase == true_ptr)
	  fprintf(stderr, " + 1");
	else if(xdd->elseCase!=false_ptr) {
		fprintf(stderr, " + ");
		print_xdd_err(xdd->elseCase);
	}
}

void print_xor_of_ands(xor_of_ands *top_xor) {
	for(int x = 0; x < top_xor->curr_length-1; x++) {
		if(top_xor->vars_in_and[x] == -1) { // This should never happen
			fprintf(stderr, "XDD is malformed...exiting\n");
			exit(0);
		} else {
			d2e_printf2("x[%s]*", s_name(top_xor->vars_in_and[x]));
			d3e_printf2("x[%d]*", top_xor->vars_in_and[x]);
			d4_printf3("x[%s(%d)]*", s_name(top_xor->vars_in_and[x]), top_xor->vars_in_and[x]);
		}
	}
	if(top_xor->vars_in_and[top_xor->curr_length-1] == -1) fprintf(foutputfile, "1");
	else {
		d2e_printf2("x[%s]", s_name(top_xor->vars_in_and[top_xor->curr_length-1]));
		d3e_printf2("x[%d]", top_xor->vars_in_and[top_xor->curr_length-1]);
		d4_printf3("x[%s(%d)]", s_name(top_xor->vars_in_and[top_xor->curr_length-1]), top_xor->vars_in_and[top_xor->curr_length-1]);
	}
	for(xor_of_ands *tmp = top_xor->next_and; tmp!=NULL; tmp = tmp->next_and) {
		d2_printf1(" + ");
		
		for(int x = 0; x < tmp->curr_length-1; x++) {
			if(tmp->vars_in_and[x] == -1) { // This should never happen
				fprintf(stderr, "XDD is malformed...exiting\n");
				exit(0);
			} else {
				d2e_printf2("x[%s]*", s_name(tmp->vars_in_and[x]));
				d3e_printf2("x[%d]*", tmp->vars_in_and[x]);
				d4_printf3("x[%s(%d)]*", s_name(tmp->vars_in_and[x]), tmp->vars_in_and[x]);
			}
		}
		if(tmp->vars_in_and[tmp->curr_length-1] == -1) { 
			d2_printf1("1");
		} else {
			d2e_printf2("x[%s]", s_name(tmp->vars_in_and[tmp->curr_length-1]));
			d3e_printf2("x[%d]", tmp->vars_in_and[tmp->curr_length-1]);
			d4_printf3("x[%s(%d)]", s_name(tmp->vars_in_and[tmp->curr_length-1]), tmp->vars_in_and[tmp->curr_length-1]);
		}
	}
}

int print_xor_of_ands_file_ver2(xor_of_ands *top_xor, int use_symtable) {
	int ret = 0;;
	for(int x = 0; x < top_xor->curr_length-1; x++) {
		if(top_xor->vars_in_and[x] == -1) { // This should never happen
			fprintf(stderr, "XDD is malformed...exiting\n");
			return -1;
		}
		else fprintf(foutputfile, "x%d", use_symtable?atoi(getsym_i(top_xor->vars_in_and[x])->name):top_xor->vars_in_and[x]);
	}
	if(top_xor->vars_in_and[top_xor->curr_length-1] == -1) ret = 1;
	else fprintf(foutputfile, "x%d", use_symtable?atoi(getsym_i(top_xor->vars_in_and[top_xor->curr_length-1])->name):top_xor->vars_in_and[top_xor->curr_length-1]);
	for(xor_of_ands *tmp = top_xor->next_and; tmp!=NULL; tmp = tmp->next_and) {
		fprintf(foutputfile, " ");
		
		for(int x = 0; x < tmp->curr_length-1; x++) {
			if(tmp->vars_in_and[x] == -1) { // This should never happen
				fprintf(stderr, "XDD is malformed...exiting\n");
				exit(0);
			}
			else fprintf(foutputfile, "x%d", use_symtable?atoi(getsym_i(tmp->vars_in_and[x])->name):tmp->vars_in_and[x]);
		}
		if(tmp->vars_in_and[tmp->curr_length-1] == -1) ret = 1;
		else fprintf(foutputfile, "x%d", use_symtable?atoi(getsym_i(tmp->vars_in_and[tmp->curr_length-1])->name):tmp->vars_in_and[tmp->curr_length-1]);
	}
	return ret;
}

void print_xor_of_ands_file(xor_of_ands *top_xor, int use_symtable) {
	for(int x = 0; x < top_xor->curr_length-1; x++) {
		if(top_xor->vars_in_and[x] == -1) { // This should never happen
			fprintf(stderr, "XDD is malformed...exiting\n");
			exit(0);
		}
		else fprintf(foutputfile, "x[%d]*", use_symtable?atoi(getsym_i(top_xor->vars_in_and[x])->name):top_xor->vars_in_and[x]);
	}
	if(top_xor->vars_in_and[top_xor->curr_length-1] == -1) fprintf(foutputfile, "1");
	else fprintf(foutputfile, "x[%d]", use_symtable?atoi(getsym_i(top_xor->vars_in_and[top_xor->curr_length-1])->name):top_xor->vars_in_and[top_xor->curr_length-1]);
	for(xor_of_ands *tmp = top_xor->next_and; tmp!=NULL; tmp = tmp->next_and) {
		fprintf(foutputfile, " + ");
		
		for(int x = 0; x < tmp->curr_length-1; x++) {
			if(tmp->vars_in_and[x] == -1) { // This should never happen
				fprintf(stderr, "XDD is malformed...exiting\n");
				exit(0);
			}
			else fprintf(foutputfile, "x[%d]*", use_symtable?atoi(getsym_i(tmp->vars_in_and[x])->name):tmp->vars_in_and[x]);
		}
		if(tmp->vars_in_and[tmp->curr_length-1] == -1) fprintf(foutputfile, "1");
		else fprintf(foutputfile, "x[%d]", use_symtable?atoi(getsym_i(tmp->vars_in_and[tmp->curr_length-1])->name):tmp->vars_in_and[tmp->curr_length-1]);
	}
}

xor_of_ands *invert_xor_of_ands(xor_of_ands *top_xor) {
	for(xor_of_ands *tmp = top_xor; ; tmp = tmp->next_and) {
		if(tmp->next_and == NULL) {
			tmp->next_and = (xor_of_ands*)ite_calloc(1, sizeof(xor_of_ands), 9, "tmp->next_and");
			tmp->next_and->vars_in_and = (int *)ite_calloc(1, sizeof(int), 9, "tmp->next_and->vars_in_and");
			tmp->next_and->curr_length = 1;
			tmp->next_and->vars_in_and[0] = -1;
			break;
		}
		if(tmp->next_and->next_and == NULL && tmp->next_and->vars_in_and[tmp->next_and->curr_length-1] == -1) {
			ite_free((void **)tmp->next_and);
			tmp->next_and = NULL;
			break;
		}
	}
	return top_xor;
}

void free_xor_of_ands(xor_of_ands *top_xor) {
	for(xor_of_ands *tmp = top_xor; tmp!=NULL; ) {
		xor_of_ands *free_me = tmp;
		tmp = tmp->next_and;
		ite_free((void **)free_me);
	}
}

xor_of_ands *get_flat_xdd(BDDNode *xdd, int size) {
	//base case
	xor_of_ands *top_xor;
	if(xdd == true_ptr) {
		top_xor = (xor_of_ands*)ite_calloc(1, sizeof(xor_of_ands), 9, "top_xor");
		top_xor->vars_in_and = (int *)ite_calloc(size, sizeof(int), 9, "tmp->next_and->vars_in_and");
		top_xor->curr_length = 1;
		top_xor->vars_in_and[0] = -1;
	} else if(xdd == false_ptr) return NULL;
	else {
		top_xor = get_flat_xdd(xdd->thenCase, size);
		
		if(top_xor == NULL) { //This shouldn't happen with properly formed xdds
			fprintf(stderr, "XDD is malformed...exiting\n");
			exit(0);
		}
		for(xor_of_ands *tmp = top_xor; ; tmp = tmp->next_and) {
			//Multiply the top variable into all equations in top_xor
			if(tmp->curr_length == 1 && tmp->vars_in_and[0] == -1)
			  tmp->vars_in_and[0] = xdd->variable;
			else tmp->vars_in_and[tmp->curr_length++] = xdd->variable;
			if(tmp->next_and == NULL) {
				if(xdd->elseCase != false_ptr) {
					//XOR the two lists together
					tmp->next_and = get_flat_xdd(xdd->elseCase, size);
				}
				break;
			}
		}
	}
	return top_xor;
}

void print_flat_xdd(BDDNode *xdd, int size) {
	xor_of_ands *top_xor = get_flat_xdd(xdd, size);
	if(top_xor == NULL) return;
	//print_xor_of_ands(top_xor);
	top_xor = invert_xor_of_ands(top_xor);
	print_xor_of_ands(top_xor);
	free_xor_of_ands(top_xor);	
}

void print_flat_xdd_file(BDDNode *xdd, int size) {
	int use_symtable = sym_all_int();
	xor_of_ands *top_xor = get_flat_xdd(xdd, size);
	if(top_xor == NULL) return;
	//print_xor_of_ands_file(top_xor);
	top_xor = invert_xor_of_ands(top_xor);
	print_xor_of_ands_file(top_xor, use_symtable);
	free_xor_of_ands(top_xor);	
}

void print_flat_xor_file(BDDNode *xdd, int size, int use_symtable) {
	xor_of_ands *top_xor = get_flat_xdd(xdd, size);
	if(top_xor == NULL) return;
	//print_xor_of_ands_file(top_xor);
	top_xor = invert_xor_of_ands(top_xor);
	int ret = print_xor_of_ands_file_ver2(top_xor, use_symtable);
	if(ret == 0) fprintf(foutputfile, " = 0");
	else if(ret == 1) fprintf(foutputfile, "= 1");
	else if(ret == -1) {
		fprintf(stderr, "Error: True function found while parsing XDDs, this should not happen...exiting\n");
		exit(0);
	}
				 
	//if(top_xor->vars_in_and[top_xor->curr_length-1] == -1) fprintf(foutputfile, " = 1");
	//else fprintf(foutputfile, " = 0");
	free_xor_of_ands(top_xor);	
}

void printLinearFormat() {
	int use_symtable = sym_all_int();
	fprintf(foutputfile, "%ld\n", numinp);
	fprintf(foutputfile, "[\n");
	BDDNode *xdd = bdd2xdd(functions[0]);
	print_flat_xdd_file(xdd, length[0]);
	fprintf(foutputfile, "\n");
	for(int x=1; x < nmbrFunctions; x++) {
		xdd = bdd2xdd(functions[x]);
		fprintf(foutputfile, ",\n");
		print_flat_xdd_file(xdd, length[x]);
		fprintf(foutputfile, "\n");
	}

	for(int x=1; x < numinp; x++) {
		fprintf(foutputfile, ",\n");
		fprintf(foutputfile, "x[%d]^2 + x[%d]\n", use_symtable?atoi(getsym_i(x)->name):x,
                                                use_symtable?atoi(getsym_i(x)->name):x);
	}
	fprintf(foutputfile, "];\n");
}

void print_xor_symtable() {
	for(int x = 1; x <=numinp; x++) {
		fprintf(foutputfile, "; %d = %s\n", x, getsym_i(x)->name);
	}
}

void printXORFormat() {
	fprintf(foutputfile, "p xor %ld %d\n", numinp, nmbrFunctions);
	int use_symtable = sym_all_int();
	if(use_symtable == 0)
	  print_xor_symtable();		
	
	for(int x=0; x < nmbrFunctions; x++) {
		if(functions[x] == true_ptr) continue;
		BDDNode *xdd = bdd2xdd(functions[x]);
		print_flat_xor_file(xdd, length[x], use_symtable);
		fprintf(foutputfile, "\n");
	}
}

char getNextSymbol (char *&, int &intnum, BDDNode * &);

int xorbdd_line;

int intnum;

char getNextSymbol () {
	char integers[20];
	int i = 0;
	int p = 0;
	while (1) {
      p = fgetc(finputfile);
		if (p == EOF) {
			//fprintf(stderr, "\nUnexpected (unsigned int)EOF...exiting\n");
			return 'x';
		}
		if (feof(finputfile)) return 'x';
		if (p == ';') {
			while (p != '\n') {
            p = fgetc(finputfile);
            if (p == EOF) {
					return 'x';
					//fprintf (stderr, "\nUnexpected (unsigned int)EOF...exiting:%d\n", xorbdd_line);
					//exit (0);
				}
			}
         p = fgetc(finputfile);
         if (p == EOF) {
				return 'x';	  
				//fprintf (stderr, "\nUnexpected (unsigned int)EOF...exiting:%d\n", xorbdd_line);
				//exit (0);
			}
			ungetc (p, finputfile);
			continue;
		}
		if (p == '=') {
         p = fgetc(finputfile);
			if (p == EOF) {
				fprintf (stderr, "\nUnexpected (unsigned int)EOF...exiting:%d\n", xorbdd_line);
				exit (0);
			}
			if (p != ' ') {
				fprintf (stderr, "\n' ' expected...exiting:%d\n", xorbdd_line);
				exit (0);
			}
         p = fgetc(finputfile);
			if (p == EOF) {
				fprintf (stderr, "\nUnexpected (unsigned int)EOF...exiting:%d\n", xorbdd_line);
				exit (0);
			}
			char ret = p;
			if ((p != '0') && (p != '1')) {
				fprintf (stderr, "\n0 or 1 expected...exiting:%d\n", xorbdd_line);
				exit (0);
			}
			
			while (p != '\n' && p != ';') {
            p = fgetc(finputfile);
            if (p == EOF) {
					ungetc (p, finputfile);
					return ret;
				}
				if(p!=' ' && p!='\n' && p!=';') {
					fprintf(stderr, "\nTrailing character (%c) found...exiting:%d\n", p, xorbdd_line);
					exit(0);
				}
			}
			
			ungetc (p, finputfile);
			return ret;
		}
		if (p == ' ') {
			return ' ';
		}
		if (p == 'x') {
         p = fgetc(finputfile);
         if (p == EOF) {
				fprintf (stderr, "\nUnexpected (unsigned int)EOF...exiting:%d\n", xorbdd_line);
				exit (0);
			}
			//integers[0] = 'x';
			if ((p >= '1') && (p <= '9')) {
				i = 0;
				while ((p >= '0') && (p <= '9')) {
					integers[i] = p;
					i++;
               p = fgetc(finputfile);
               if (p == EOF) {
						fprintf (stderr, "\nUnexpected (unsigned int)EOF...exiting:%d\n", xorbdd_line);
						exit (0);
					}
				}
				ungetc (p, finputfile);
				integers[i] = 0;
				//intnum = atoi (integers+1);
				intnum = i_getsym(integers, SYM_VAR);
				if (intnum > numinp) {
					fprintf (stderr, "\nVariable %d is larger than allowed (%ld)...exiting:%d\n",	intnum, numinp - 2, xorbdd_line);
					exit (0);
				}
				return 'i';
			} else {
				fprintf (stderr, "\nInteger expected after x...exiting:%d\n", xorbdd_line);
				exit (0);
			}
		} if(p == '-') {
			fprintf (stderr, "\nNegative variables not supported...exiting:%d\n", xorbdd_line);
			exit (0);
		}
		if (p == 'i' || p == 'I'){
			int i = 0;
			char macros[14];
			while ((((p >= 'a') && (p <= 'z')) || ((p >= 'A') && (p <= 'Z')))
					 && (i < 13)) {
				macros[i] = p;
				i++;
            p = fgetc(finputfile);
            if (p == EOF) {
					fprintf (stderr, "\nUnexpected (unsigned int)EOF (%s)...exiting:%d\n", macros, xorbdd_line);
					exit (1);
				}
			}
			macros[i] = 0;
			if(strcasecmp (macros, "initialbranch")) {
				fprintf (stderr, "\nUnknown keyword (%s) found while looking for initialbranch...exiting:%d\n", macros, xorbdd_line);
				exit (1);
			}
			int p = 0;
			char integers[10];
			int secondnum = 0;
			i = 0;
			int openbracket_found = 0;
			int stop_openbracket = 0;
			Initialbranch:;
			while ((p != '\n')	&& !((p == ')') && (openbracket_found))) {
            p = fgetc(finputfile);
				if (p == EOF)	{
					fprintf (stderr, " %d %d ", openbracket_found, stop_openbracket);
					fprintf (stderr, "\nUnexpected (unsigned int)EOF (%s)...exiting:%d\n",	macros, xorbdd_line);
					exit (1);
				}
				if (p == '(') {
					if (!stop_openbracket)
					  openbracket_found = 1;
					continue;
				}
				if ((p >= '0') && (p <= '9')) {
					i = 0;
					while ((p >= '0') && (p <= '9')) {
						integers[i] = p;
						i++;
                  p = fgetc(finputfile);
                  if (p == EOF) {
							fprintf (stderr, "\nUnexpected (unsigned int)EOF (%s)...exiting:%d\n", macros, xorbdd_line);
							exit (1);
						}
					}
					integers[i] = 0;
					intnum = atoi (integers);
					if (intnum > numinp) {
						fprintf (stderr, "\nVariable %d is larger than allowed(%ld)...exiting:%d\n", intnum, numinp - 2, xorbdd_line);
						exit (1);
					}
					if (p == '.') {
                  p = fgetc(finputfile);
						if (p == EOF) {
							fprintf (stderr, "\nUnexpected (unsigned int)EOF (%s)...exiting:%d\n",	macros, xorbdd_line);
							exit (1);
						}
						if (p == '.') {
                     p = fgetc(finputfile);
                     if (p == EOF) {
								fprintf (stderr, "\nUnexpected (unsigned int)EOF (%s)...exiting:%d\n", macros, xorbdd_line);
								exit (1);
							}
							if ((p >= '0') && (p <= '9')) {
								i = 0;
								while ((p >= '0') && (p <= '9')) {
									integers[i] = p;
									i++;
                           p = fgetc(finputfile);
                           if (p == EOF) {
										fprintf (stderr, "\nUnexpected (unsigned int)EOF (%s)...exiting:%d\n",	macros, xorbdd_line);
										exit (1);
									}
								}
								ungetc (p, finputfile);
								integers[i] = 0;
								secondnum = atoi (integers);
								if (secondnum > numinp) {
									fprintf (stderr, "\nVariable %d is larger than allowed (%ld)...exiting:%d\n",	secondnum, numinp - 2, xorbdd_line);
									exit (1);
								}
								for (int x = intnum; x <= secondnum; x++)
								  independantVars[x] = 1;
								stop_openbracket = 1;
							} else {
								fprintf (stderr, "\nNumber expected after '%d..'  exiting:%d\n", intnum, xorbdd_line);
								exit (1);
							}
						} else {
							ungetc (p, finputfile);
							stop_openbracket = 1;
							independantVars[intnum] = 1;
						}
					} else {
						ungetc (p, finputfile);
						stop_openbracket = 1;
						independantVars[intnum] = 1;
					}
				}
			}
			if ((p == '\n') && (openbracket_found == 1)) {
				p = ' ';
				goto Initialbranch;
			}
			ungetc (p, finputfile);
			return 'b';
		}
		if(p!='\n') {
			fprintf(stderr, "\nUnexpected character (%c)...exiting:%d\n", p, xorbdd_line);
			exit(0);
		}
	}
}

void xorloop () {
	fscanf (finputfile, "%ld %ld\n", &numinp, &numout);
	numinp += 2;
	xorbdd_line = 1;

	vars_alloc(numinp+2);
	functions_alloc(numout+2);

	int temp_vars = numinp+2;
	
	BDDNode **bdds = (BDDNode **)ite_calloc(1000, sizeof(BDDNode *), 9, "bdds - xorformat");
	int bdds_size = 1000;
	int p = 0;

	for (int x = 0; x < numinp + 1; x++) {
		independantVars[x] = 1;
	}
	
	while (1) {				//(p = fgetc(finputfile))!=(unsigned int)EOF) 
		xorbdd_line++;
      d2_printf3("\rReading XOR %d/%ld", xorbdd_line, (long)numout);
		if(p == ';') {
			while (p != '\n') {
            p = fgetc(finputfile);
            if (p == EOF) {
					goto Exit;
				}
			}
         p = fgetc(finputfile);
         if(p == EOF) {
				goto Exit;
			}
			if(p!=';') {
				ungetc(p, finputfile);
			}
			continue;
		}
		if(p=='\n') {
			while (p != '\n') {
            p = fgetc(finputfile);
            if (p == EOF) {
					goto Exit;
				}
			}
			if(p==';') continue;
			ungetc(p, finputfile);
		}
      p = fgetc(finputfile);
      if(p == EOF) {
			goto Exit;
		}
		ungetc(p, finputfile);
		p = getNextSymbol();
		if(p == 'x') goto Exit;
		if(p == 'b') continue; //InitialBranch condition
		else if(p != 'i') {
			fprintf (stderr, "\nx# expected...exiting:%d\n", xorbdd_line);
			exit (0);	
		}
		
		int nmbrxors = 0;
		int num_spaces = 0;
		int *vars = (int *)calloc(numinp, sizeof(int));
		while((p != '0') && (p != '1')) {
			//Get it!
			bdds[nmbrxors] = ite_var(intnum);
			int num_ands = 0;
			while(p == 'i') {
				bdds[nmbrxors] = ite_and(bdds[nmbrxors], ite_var(intnum));
				int intnum_prev = intnum;
				p = getNextSymbol();
				if(p == 'i') { vars[intnum] = 1; vars[intnum_prev] = 1;}
				if(p == 'x') {
					fprintf (stderr, "\nUnexpected (unsigned int)EOF, '=' expected...exiting:%d\n", xorbdd_line);
					exit (0);	
				}
				num_spaces = 0;
				num_ands++;
			}
			if(p!=' ') {
				fprintf (stderr, "\n' ' expected...exiting:%d\n", xorbdd_line);
				exit (0);	
			}
			num_spaces++;
			if(num_spaces > 1) {
				fprintf(stderr, "\nOnly one space between characters is allowed...exiting:%d\n", xorbdd_line);
				exit(0);
			}
			nmbrxors++;
			if(nmbrxors >= bdds_size) {
				bdds = (BDDNode **)ite_recalloc(bdds, bdds_size, bdds_size+1000, sizeof(BDDNode *), 9, "bdds - xorformat");
				bdds_size+=1000;
				fprintf(stderr, "Warning: Large equation found, increasing storage...\n");
			}
						 
			p = getNextSymbol();
			if(p == 'x') {
				fprintf (stderr, "\nUnexpected (unsigned int)EOF, '=' expected...exiting:%d\n", xorbdd_line);
				exit (0);
			}
		}

      //Add the new function
      char tmp_char[100];      
      int max_size_xor = 0;
      for(int i = 0; i < nmbrxors; i++) {
         int size_xor = 0;
         for(BDDNode *tmp = bdds[i]; !IS_TRUE_FALSE(tmp); tmp = tmp->thenCase) {
            if(++size_xor > 1) { max_size_xor++; break; }
         }
      }
           
		BDDNode *excess = false_ptr;
		if(max_size_xor < 10) {
         for(int i = 0; i < nmbrxors; i++) {
            excess = ite_xor(excess, bdds[i]);
            //excess = xddxor(excess, bdd2xdd(bdds[i]));
         }
      } else {
         for(int i = 0; i < nmbrxors; i++) {
            int size_xor = 0;
            for(BDDNode *tmp = bdds[i]; !IS_TRUE_FALSE(tmp); tmp = tmp->thenCase) 
              if(++size_xor > 1) break;
            if(size_xor > 1) {
               sprintf(tmp_char, "%d", temp_vars++);
               int new_var = i_getsym(tmp_char, SYM_VAR);
               functions_add(ite_equ(ite_var(new_var), bdds[i]), AND_EQU, new_var);
               independantVars[new_var] = 0;
               excess = ite_xor(excess, ite_var(new_var));
            } else {
               excess = ite_xor(excess, bdds[i]);
            }
         }
      }
		
		free(vars);

      if(p=='0') {
//         excess = xddnot(excess);
         excess = ite_not(excess);
      } else if(p=='1') {
        //Do nothing
      } else {
         fprintf (stderr, "\n0 or 1 expected...exiting:%d\n", xorbdd_line);
         exit (0);	
      }
      functionType[nmbrFunctions] = UNSURE;
//		functionType[nmbrFunctions] = XDD;
      functions[nmbrFunctions] = excess;
      d4_printf2("BDD $%d: ", nmbrFunctions);
      D_4(printBDDfile(functions[nmbrFunctions], stddbg);)
         d4_printf1("\n");
      nmbrFunctions++;
		
		if (nmbrFunctions > (numout * 2)) {
			fprintf (stderr, "Too many functions, increase to larger than %ld...exiting:%d",	numout, xorbdd_line);
			exit (0);
		}
		
		p = fgetc (finputfile);
		if (p != '\n') {
         p = fgetc(finputfile);
         while (p != EOF && p!= '\n')
            p = fgetc(finputfile);
			if (p != '\n')
			  goto Exit;
		} else ungetc (p, finputfile);
	}
	//d4_printf1("\n");
	Exit:;

	numinp = numinp+temp_vars;
	
/*	for(int x = 1; x < nmbrFunctions; x++) {
		fprintf(stderr, "%d\n", x);
		bdd_gc(1);
		//functions[0] = ite_and(functions[0], functions[x]);
		//functions[0] = xddand(functions[0], functions[x]);
		for(int y = 1; y <= numinp; y++) { 
			functions[0] = set_variable_xdd(functions[x], y, 1);
			functions[0] = set_variable_xdd(functions[x], y, 0);
		}

		functions[x] = true_ptr;
	}
*/	
   d2_printf1("\rReading XOR ... Done\n");
	ite_free((void **)&bdds); bdds_size = 0;
}
