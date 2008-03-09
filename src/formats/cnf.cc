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

/**********************************************************************
 *  cnf.c (S. Weaver)
 *  Routines for converting from CNF to BDD and from DNF to CNF
 **********************************************************************/  

#include "sbsat.h"
#include "sbsat_formats.h"

#define CNF_USES_SYMTABLE

int nNumClauses = 0;
int nNumCNFVariables = 0;

void cnf_process(Clause *pClauses);

void free_clauses(Clause *pClauses) {
	for(int x = 0; x < nNumClauses; x++) {
		if(pClauses[x].variables != NULL) ite_free((void **)&pClauses[x].variables);
	}
	ite_free((void **)&pClauses);	
}

int clscompfunc(const void *x, const void *y) {
	Clause pp, qq;
	
	pp = *(const Clause *)x;
	qq = *(const Clause *)y;
	
	//Compare the lengths of the clauses.
	if(pp.length != qq.length )
	  return (pp.length < qq.length ? -1 : 1);
	
	//The lengths of pp and qq are the same.
	//Now take a look at the variables in the clauses.
	for(int i = 0; i < pp.length; i++)
	  if(abs(pp.variables[i]) != abs(qq.variables[i]))
		 return (abs(pp.variables[i]) < abs(qq.variables[i]) ? -1 : 1);
	
	//If the two clauses contain the same variables, then consider the
	//literals and compare again. ( So, no abs() is used here ). This is done to make
	//removal of duplets easy.
	for(int i = 0; i < pp.length; i++)
	  if(pp.variables[i] != qq.variables[i])
		 return (pp.variables[i] < qq.variables[i] ? -1 : 1);
	
	//Default value if all is equal. ( Thus a duplet... )
#ifndef FORCE_STABLE_QSORT
	return 0;
#else
	  {
		  if (x < y) return -1;
		  else if (x > y) return 1;
		  else return 0;
	  }
#endif
	return 1;
}

int getNextSymbol_CNF (int *intnum) {
	char integers[20];
	int i = 0;
	int p = 0;
	while(1) {
		p = fgetc(finputfile);
		if(p == EOF) return ERR_IO_READ;
		if(p == 'c') {
			while(p != '\n') {
				d9_printf2("%c", p);
				p = fgetc(finputfile);
				if(p == EOF) return ERR_IO_READ;
			}
			d9_printf2("%c", p);
			p = fgetc(finputfile);
			if (p == EOF) return ERR_IO_READ;
			ungetc(p, finputfile);
			continue;
		} else if(p == 'p') {
			d9_printf2("%c", p);
			return IO_CNF_HEADER;
		} else if(((p >= '0') && (p <= '9')) || (p == '-')) {
			i = 0;
			while(((p >= '0') && (p <= '9')) || (p == '-')) {
				integers[i] = p;
				i++;
				d9_printf2("%c", p);
				p = fgetc(finputfile);
				if(p == EOF) break;
			}
			ungetc(p, finputfile);
			integers[i] = 0;
			*intnum = atoi(integers);
			return NO_ERROR;
		} else if(p >='A' && p<='z') {
			d9_printf2("%c", p);
			return ERR_IO_UNEXPECTED_CHAR;
		}
		d9_printf2("%c", p);
	}
}

int CNF_to_BDD() {
	
	//Global variables used in this function:
	//  int nNumCNFVariables, int nNumClauses
	
	if(feof(finputfile)) return ERR_IO_READ;

	int next_symbol;
	int ret = NO_ERROR;

	if (int i = fscanf(finputfile, "%d %d\n", &nNumCNFVariables, &nNumClauses) != 2) {
		fprintf(stderr, "Error while parsing CNF input: bad header %d %d %d\n", i, nNumCNFVariables, nNumClauses);
		exit(1);
	}

	d9_printf3(" cnf %d %d\n", nNumCNFVariables, nNumClauses);
	
	int nOrigNumClauses = nNumClauses;
	
	Clause *pClauses = (Clause*)ite_calloc(nNumClauses, sizeof(Clause), 9, "read_cnf()::pClauses");
	int tempint_max = 100;
	int *tempint = (int *)ite_calloc(tempint_max, sizeof(int *), 9, "read_cnf()::tempint");

	//Get and store the CNF clauses
	int x = 0;
	while(1) {
		if (x%1000 == 0) {
			d2_printf3("\rReading CNF %d/%d ... ", x, nNumClauses);
		}
		
		ret = getNextSymbol_CNF(&next_symbol);
		if(ret == ERR_IO_UNEXPECTED_CHAR) {
			fprintf(stderr, "Error while parsing CNF input: Clause %d\n", x);
			return ret;
		}
		
		if (x == nNumClauses) { //Should be done reading CNF
			if (ret == ERR_IO_READ) break; //CNF has been fully read in
			if (next_symbol == 0) break; //Extra 0 at end of file
			fprintf(stderr, "Warning while parsing CNF input: more than %d functions found\n", nOrigNumClauses);
			break;
		}

		if (ret != NO_ERROR) {
			fprintf(stderr, "Error while parsing CNF input: premature end of file, only %d functions found\n", x);
			return ret;
		} else {
			int y = 0;
			while(1) {
				if (y >= tempint_max) {
					tempint = (int*)ite_recalloc((void*)tempint, tempint_max, tempint_max+100, sizeof(int), 9, "read_cnf()::tempint");
					tempint_max += 100;
				}
				tempint[y] = next_symbol;
				if (tempint[y] == 0) break; //Clause has been terminated
				ret = getNextSymbol_CNF(&next_symbol);
				if (ret != NO_ERROR) {
					fprintf(stderr, "Error while parsing CNF input: Clause %d\n", x);
					return ret;
				}
				y++;
			}
			if(y==0) continue; //A '0' line -- no clause
			pClauses[x].length = y;
			pClauses[x].variables = (int *)ite_calloc(y+1, sizeof(int), 9, "read_cnf()::pClauses[x].variables");
			pClauses[x].subsumed = 0;
			pClauses[x].flag = -1;
			for(int z = 0; z < y; z++) {
#ifdef CNF_USES_SYMTABLE
				pClauses[x].variables[z] = i_getsym_int(tempint[z], SYM_VAR);
#else
				pClauses[x].variables[z] = tempint[z];
#endif
				if(abs(pClauses[x].variables[z]) > nNumCNFVariables) {
					d2e_printf1("Warning while parsing CNF input: There are more variables in input file than specified\n");
					nNumCNFVariables = abs(pClauses[x].variables[z]);
				}
			}
			x++;
		}
	}

	d2_printf3("\rReading CNF %d/%d ... \n", nNumClauses, nNumClauses);
	
	cnf_process(pClauses);
	
	ite_free((void **)&tempint); tempint_max = 0;

	d3_printf2("Number of BDDs - %d\n", nmbrFunctions);
	d2e_printf1("\rReading CNF ... Done                   \n");

	free_clauses(pClauses);
	
	return NO_ERROR;
}

void reduce_clauses(Clause *pClauses) {

	//Mark duplicate clauses
	for(int x = 0; x < nNumClauses-1; x++) {
		int isdup = 0;
		if(pClauses[x].length == pClauses[x+1].length) {
			isdup = 1;
			for(int y = 0; y < pClauses[x].length; y++) {
				if(pClauses[x].variables[y] != pClauses[x+1].variables[y]) {
					isdup = 0;
					break;
				}
			}
		}
		if(isdup == 1) {
			pClauses[x+1].subsumed = 1;
		}
	}
	
	//Remove subsumed clauses
	int subs = 0;
	for(int x = 0; x < nNumClauses; x++) {
		pClauses[x].length = pClauses[x+subs].length;
		pClauses[x].variables = pClauses[x+subs].variables;
		pClauses[x].subsumed = pClauses[x+subs].subsumed;
		
		if(pClauses[x+subs].subsumed == 1) {
			pClauses[x+subs].length = 0;
			ite_free((void **)&pClauses[x+subs].variables);
			pClauses[x+subs].variables = NULL;
			subs++;x--;nNumClauses--;continue;
		}
	}

	d2_printf2("Simplify removed %d clauses\n", subs);
}

void build_clause_BDDs(Clause *pClauses) {
	for(int x = 0; x < nNumClauses; x++) {
		if (x%1000 == 0)
		  d2_printf3("\rBuilding clause BDDs %d/%d ... ", x, nNumClauses);
		//qsort(pClauses[x].variables, pClauses[x].length, sizeof(int), abscompfunc); //This is done above
		//qsort(pClauses[x].variables, pClauses[x].length, sizeof(int), absrevcompfunc);
		BDDNode *pOrBDD = false_ptr;
		for(int y = 0; y < pClauses[x].length; y++) {
			pOrBDD = ite_or(pOrBDD, ite_var(pClauses[x].variables[y]));
		}
		functions_add(pOrBDD, PLAINOR, 0);
		pClauses[x].subsumed = 1;
	}
}

void find_and_build_xors(Clause *pClauses) {
	//Search for XORs - This code designed after a snippet of march_dl by M. Heule
	int xors_found = 0;
	for(int x = 0; x < nNumClauses; x++) {
		assert(pClauses[x].length > 0);
		if(pClauses[x].length>1) {
			int domain = 1<<(pClauses[x].length-1);
			if(domain<=1) break;
			if(domain+x > nNumClauses) break;
			
			int cont = 0;
			for(int y = 1; y < domain; y++) 
			  if(pClauses[x+y].length != pClauses[x].length) {
				  x += (y-1);
				  cont = 1;
				  break;
			  }
			if(cont == 1) continue;
			
			for(int y = 0; y < pClauses[x].length; y++)
			  if(abs(pClauses[x].variables[y]) != abs(pClauses[x+domain-1].variables[y])) {
				  cont = 1;
				  break;
			  }
			if(cont == 1) continue;
			
			int sign = 1;
			for(int y = 0; y < pClauses[x].length; y++)
			  sign *= pClauses[x].variables[y] < 0 ? -1 : 1;
			
			for(int y = 1; y < domain; y++) {
				int tmp = 1;
				for(int z = 0; z < pClauses[x+y].length; z++) //pClauses[x+y].length == pClauses[x].length
				  tmp *= pClauses[x+y].variables[z] < 0 ? -1 : 1;
				if(tmp != sign) {
					cont = 1;
					break;
				}
			}
			if(cont == 1) continue;
			
			xors_found++;
			BDDNode *pXorBDD = false_ptr;
			for(int y = 0; y < pClauses[x].length; y++) {
				pXorBDD = ite_xor(pXorBDD, ite_var(pClauses[x].variables[y]));
			}
			functions_add(pXorBDD, PLAINXOR, 0);
			
			for(int y = 0; y < domain; y++)
				pClauses[x+y].subsumed = 1;
			x += domain-1;
		}
	}
	
	d2_printf2("Found %d XOR functions\n", xors_found);
}

void find_and_build_andequals(Clause *pClauses) {
	
	int *twopos_temp = (int *)calloc(nNumCNFVariables+1, sizeof(int));
	int *twoneg_temp = (int *)calloc(nNumCNFVariables+1, sizeof(int));
	int *greaterpos_temp = (int *)calloc(nNumCNFVariables+1, sizeof(int));
	int *greaterneg_temp = (int *)calloc(nNumCNFVariables+1, sizeof(int));
	for(int x = 0; x < nNumClauses; x++) {
		if(pClauses[x].length == 2) {
			for(int y = 0; y < 2; y++) {
				if(pClauses[x].variables[y] > 0)
				  twopos_temp[pClauses[x].variables[y]]++;
				else
				  twoneg_temp[-pClauses[x].variables[y]]++;
			}
		}
		else if(pClauses[x].length > 2) {
			for(int y = 0; y < pClauses[x].length; y++) {
				if(pClauses[x].variables[y] > 0)
				  greaterpos_temp[pClauses[x].variables[y]]++;
				else
				  greaterneg_temp[-pClauses[x].variables[y]]++;
			}
		}
	}
	store *two_pos = (store *)calloc(nNumCNFVariables+1, sizeof(store));
	store *two_neg = (store *)calloc(nNumCNFVariables+1, sizeof(store));
      
	//two_pos and two_neg are lists that contain all the clauses
	//that are of length 2. two_pos contains every 2 variable clause
	//that has a positive variable, two_neg contains every 2
	//variable clause that has a negative variable. There will most likely
	//be some overlaps in the variable storing.
	//EX)
	//p cnf 3 3
	//2 3 0
	//-2 -3 0
	//-2 3 0
	//
	//two_pos will point to (2:3)   and (-2:3)
	//two_neg will point to (-2:-3) and (-2:3)
	store *greater_pos = (store *)calloc(nNumCNFVariables+1, sizeof(store));
	store *greater_neg = (store *)calloc(nNumCNFVariables+1, sizeof(store));
      
	//greater_pos and greater_neg are similar to two_pos and two_neg
	//except greater_pos and greater_neg contain all clauses with more
	//than 2 variables in length.
	
	//Storing appropriate array sizes...helps with memory!
	for(int x = 1; x <= nNumCNFVariables; x++) {
		two_pos[x].num = (int *)calloc(twopos_temp[x], sizeof(int));
		two_neg[x].num = (int *)calloc(twoneg_temp[x], sizeof(int));
		greater_pos[x].num = (int *)calloc(greaterpos_temp[x], sizeof(int));
		greater_neg[x].num = (int *)calloc(greaterneg_temp[x], sizeof(int));
	}
	//      free(twopos_temp);
	//      free(twoneg_temp);
	free(greaterpos_temp);
	free(greaterneg_temp);
	
	//This is where two_pos, two_neg, greater_pos, and greater_neg are
	//filled with clauses
	//SEAN! This could be sped up! Needs to be sped up!
	//Take into account that clauses are now sorted.
	
	for(int x = 0; x < nNumClauses; x++) {
		if(pClauses[x].length == 2) {
			if(pClauses[x].variables[0] > 0) {
				int y = two_pos[pClauses[x].variables[0]].length;
				two_pos[pClauses[x].variables[0]].num[y] = x;
				two_pos[pClauses[x].variables[0]].length++;
			} else {
				int y = two_neg[-pClauses[x].variables[0]].length;
				two_neg[-pClauses[x].variables[0]].num[y] = x;
				two_neg[-pClauses[x].variables[0]].length++;
			}
			if(pClauses[x].variables[1] > 0) {
				int y = two_pos[pClauses[x].variables[1]].length;;
				two_pos[pClauses[x].variables[1]].num[y] = x;
				two_pos[pClauses[x].variables[1]].length++;
			} else {
				int y = two_neg[-pClauses[x].variables[1]].length;;
				two_neg[-pClauses[x].variables[1]].num[y] = x;
				two_neg[-pClauses[x].variables[1]].length++;
			}
		} else if(pClauses[x].length > 2) {
			for(int z = 0; z < pClauses[x].length; z++) {
				if(pClauses[x].variables[z] > 0) {
					int y = greater_pos[pClauses[x].variables[z]].length;;
					greater_pos[pClauses[x].variables[z]].num[y] = x;
					greater_pos[pClauses[x].variables[z]].length++;
				} else {
					int y = greater_neg[-pClauses[x].variables[z]].length;
					greater_neg[-pClauses[x].variables[z]].num[y] = x;
					greater_neg[-pClauses[x].variables[z]].length++;
				}
			}
		}
	}

	int num_andequals_found = 0;
	
	//ok...this is where the and= and or= are sorted out.
	//I'll have to make a good explaination later
	//cause this was hard to work out.
	int and_or_do = 1;
	while(and_or_do) {
		and_or_do = 0;
		for(int x = 1; x <= nNumCNFVariables; x++) {
			
			if (x%100 == 1)
			  d2_printf4("\r %d AND/OR Search CNF %ld/%ld ...                                     ", num_andequals_found, x, nNumCNFVariables);
			
			if(two_pos[x].num != NULL) {
				int out = 0;
				for(int z = 0; z < greater_neg[x].length && (out != 1); z++) {

					if ((x+z)%100 == 1)
					  d2_printf5("\r %d AND/OR Search CNF %ld/%ld ... *** sub1 *** AND/OR Search CNF %ld ...       ", num_andequals_found, x, nNumCNFVariables, z);

					
					//if(pClauses[greater_neg[x].num[z]].length-1 > twopos_temp[x]) continue;
					if(pClauses[greater_neg[x].num[z]].flag == -1) {
						int y = 0;
						int count = 0;
						for(; y < pClauses[greater_neg[x].num[z]].length && count == y; y++) {
							for(int i = 0; i < two_pos[x].length; i++) {
								if(pClauses[two_pos[x].num[i]].flag == 2)
								  continue;
								if(((-pClauses[greater_neg[x].num[z]].variables[y] == pClauses[two_pos[x].num[i]].variables[0])
									 &&(pClauses[two_pos[x].num[i]].variables[0] != x))
									||((-pClauses[greater_neg[x].num[z]].variables[y] == pClauses[two_pos[x].num[i]].variables[1])
										&&(pClauses[two_pos[x].num[i]].variables[1] != x)))
								  {
									  pClauses[two_pos[x].num[i]].flag = 2;
									  count++; break;
								  }
							}
						}
						if(count == y - 1) {
							for(int i = 0; i < two_pos[x].length; i++) {
								if(pClauses[two_pos[x].num[i]].flag == 2) {
									pClauses[two_pos[x].num[i]].subsumed = 1;
								}
							}
							
							and_or_do = 1;
							pClauses[greater_neg[x].num[z]].flag = 1;
							pClauses[greater_neg[x].num[z]].subsumed = 1;
							out = 1;
							num_andequals_found++;

							BDDNode *pAndEqBDD = true_ptr;
							for(int y = 0; y < pClauses[greater_neg[x].num[z]].length; y++) {
								if(pClauses[greater_neg[x].num[z]].variables[y] != x)
								  pAndEqBDD = ite_or(pAndEqBDD, ite_var(pClauses[greater_neg[x].num[z]].variables[y]));
							}
							pAndEqBDD = ite_equ(ite_var(x), pAndEqBDD);
							functions_add(pAndEqBDD, AND_EQU, x);
							independantVars[equalityVble[greater_neg[x].num[z]]] = 0;
						}
						for(int i = 0; i < two_pos[x].length; i++)
						  pClauses[two_pos[x].num[i]].flag = -1;
					}
				}
			}
			if(two_neg[x].num != NULL) {
				int out = 0;
				for(int z = 0; z < greater_pos[x].length && (out != 1); z++) {

					if ((x+z)%100 == 1)
					  d2_printf5("\r %d AND/OR Search CNF %ld/%ld ... *** sub2 *** AND/OR Search CNF %ld ...       ", num_andequals_found, x, nNumCNFVariables, z);
					
					//if(pClauses[greater_pos[x].num[z]].length-1 > twoneg_temp[x]) continue;
					if(pClauses[greater_pos[x].num[z]].flag == -1) {
						int y = 0;
						int count = 0;
						for(; y < pClauses[greater_pos[x].num[z]].length && count == y; y++) {
							for(int i = 0; i < two_neg[x].length; i++) {
								if(pClauses[two_neg[x].num[i]].flag == 2)
								  continue;
								if(((pClauses[greater_pos[x].num[z]].variables[y] == -pClauses[two_neg[x].num[i]].variables[0])
									 &&(-pClauses[two_neg[x].num[i]].variables[0] != x))
									||((pClauses[greater_pos[x].num[z]].variables[y] == -pClauses[two_neg[x].num[i]].variables[1])
										&&(-pClauses[two_neg[x].num[i]].variables[1] != x)))
								  {
									  pClauses[two_neg[x].num[i]].flag = 2;
									  count++; break;
								  }
							}
						}
						if(count == y - 1) {
							for(int i = 0; i < two_neg[x].length; i++) {
								if(pClauses[two_neg[x].num[i]].flag == 2) {
									pClauses[two_neg[x].num[i]].subsumed = 1;
								}
							}

							and_or_do = 1;
							pClauses[greater_pos[x].num[z]].flag = 0;
							pClauses[greater_pos[x].num[z]].subsumed = 1;
							out = 1;
							num_andequals_found++;
							
							BDDNode *pOrEqBDD = false_ptr;
							for(int y = 0; y < pClauses[greater_pos[x].num[z]].length; y++) {
								if(pClauses[greater_pos[x].num[z]].variables[y] != x)
								  pOrEqBDD = ite_or(pOrEqBDD, ite_var(pClauses[greater_pos[x].num[z]].variables[y]));
							}
							pOrEqBDD = ite_equ(ite_var(-x), pOrEqBDD);
							functions_add(pOrEqBDD, OR_EQU, -x);
							independantVars[equalityVble[greater_pos[x].num[z]]] = 0;
						}
						for(int i = 0; i < two_neg[x].length; i++)
						  pClauses[two_neg[x].num[i]].flag = -1;
					}
				}
			}
		}
	}
	
	//Not needed anymore, free them!
	for(int x = 1; x < nNumCNFVariables + 1; x++) {
		free(two_pos[x].num);
		free(two_neg[x].num);
		free(greater_pos[x].num);
		free(greater_neg[x].num);
	}
	
	free(twopos_temp);
	free(twoneg_temp);
	
	free(two_pos);
	free(two_neg);
	free(greater_pos);
	free(greater_neg);
	
	for(int x = 0; x < nNumClauses; x++) {
		if(pClauses[x].flag != -1) {
			pClauses[x].subsumed = 1;
			pClauses[x].flag = -1;
		}
	}

	d2_printf2("Found %d AND=/OR= functions\n", num_andequals_found);

}

void cnf_process(Clause *pClauses) {

	d3_printf2("Number of Variables: %d\n", nNumCNFVariables);
	
	nmbrFunctions = 0;
	
	vars_alloc(nNumCNFVariables);
	functions_alloc(nNumClauses);
	
	if(DO_CLUSTER) {
	
		//Sort variables in each clause
		for(int x = 0; x < nNumClauses; x++)
		  qsort(pClauses[x].variables, pClauses[x].length, sizeof(int), abscompfunc);
		
		//Sort Clauses
		qsort(pClauses, nNumClauses, sizeof(Clause), clscompfunc);
		
		reduce_clauses(pClauses);
		
		find_and_build_xors(pClauses);
		
		reduce_clauses(pClauses);

		find_and_build_andequals(pClauses);
		
		reduce_clauses(pClauses);		  
	}
	
	build_clause_BDDs(pClauses);
	
}





/*		
      nNumClauses = y+1;
      int recurselen = num+1;
      recurse[recurselen].dag = -1;
      d3_printf1("Done Saving");
      int *threepos_temp = (int *)calloc(nNumCNFVariables+1, sizeof(int));
      int *threeneg_temp = (int *)calloc(nNumCNFVariables+1, sizeof(int));
      for(long x = 0; x < nNumClauses; x++) {
         for(y = 0; pClauses[x].variables[y] != 0; y++) {}
         pClauses[x].length = y;
         if(y == 3) {
            for(y = 0; y < 3; y++) {
               if(pClauses[x].variables[y] > 0)
                  threepos_temp[pClauses[x].variables[y]]++;
               else
                  threeneg_temp[-pClauses[x].variables[y]]++;
            }
         }
      }
      store *three_pos = (store *)calloc(nNumCNFVariables+1, sizeof(store));
      store *three_neg = (store *)calloc(nNumCNFVariables+1, sizeof(store));
      
		//Store appropriate array sizes to help with memory usage
		for(long x = 1; x < nNumCNFVariables+1; x++) {
			three_pos[x].variables = (int *)calloc(threepos_temp[x]+1, sizeof(int));
			three_neg[x].variables = (int *)calloc(threeneg_temp[x]+1, sizeof(int));
		}
      free(threepos_temp);
      free(threeneg_temp);
      count = 0;
      
		//Store all clauses with 3 variables so they can be clustered
		for(long x = 0; x < nNumClauses; x++) {
			if(pClauses[x].length == 3) {
				count++;
				for(i = 0; i < 3; i++) {
					if(pClauses[x].variables[i] < 0) {
						three_neg[-pClauses[x].variables[i]].variables[three_neg[-pClauses[x].variables[i]].length] = x;
						three_neg[-pClauses[x].variables[i]].length++;
					} else {
						three_pos[pClauses[x].variables[i]].variables[three_pos[pClauses[x].variables[i]].length] = x;
						three_pos[pClauses[x].variables[i]].length++;
					}
				}
			}
		}
      
		//v3 is in all clauses
		//if v0 is positive, both v0 positive clauses have v2, just sign changed
		//if v0 is negative, both v0 negative clauses have v1, just sign changed
		//the signs of v1 and v2 are the inverse of the sign of v3
		struct ite_3 {
			int pos;
			int neg;
			int v0;
			int v1;
		};
      struct save_4 {
			long vars[4];
		};
      ite_3 *v3_1 = (ite_3 *)calloc(1000, sizeof(ite_3));
      ite_3 *v3_2 = (ite_3 *)calloc(1000, sizeof(ite_3));
      
		//For Future, spend the time to actually calculate those 1000's
		//It wouldn't be toooo tough.
      int v3_1count;
      int v3_2count;
      save_4 *ites = (save_4 *)calloc(count/2+1, sizeof(save_4));
      d3_printf1 ("Finding ITEs");
      num = 0;
      for(int x = 0; x < nNumCNFVariables+1; x++) {
         if (x%1000 == 0)
            d2_printf3("\rITE Search CNF %d/%ld       ", x, nNumCNFVariables);
			v3_1count = 0;
			v3_2count = 0;
			for(i = 0; i < three_pos[x].length; i++) {
				//Finding clauses that have 1 negative variable
				//and clauses that have 2 negative variables
				count = 0;
				for(y = 0; y < 3; y++) {
					if(pClauses[three_pos[x].variables[i]].variables[y] < 0)
					  count++;
				}
				if(count == 1) {
					v3_1[v3_1count].pos = three_pos[x].variables[i];
					v3_1count++;
				} else if(count == 2) {
					v3_2[v3_2count].pos = three_pos[x].variables[i];
					v3_2count++;
				}
			}
			
			//Search through the clauses with 1 negative variable
			//  and try to find counterparts
			for(i = 0; i < v3_1count; i++) {
				out = 0;
				for(y = 0; (y < three_neg[x].length) && (!out); y++) {
					v3_1[i].v0 = -1;
					v3_1[i].v1 = -1;
					count = 0;
					for(z = 0; z < 3; z++) {
					  if(pClauses[v3_1[i].pos].variables[z] != x) {
					    for(j = 0; j < 3; j++) {
					      if((pClauses[v3_1[i].pos].variables[z] == pClauses[three_neg[x].variables[y]].variables[j])
						 &&(pClauses[v3_1[i].pos].variables[z] > 0))
						{
						  count++;
						  v3_1[i].v0 = z;
						} else if(-pClauses[v3_1[i].pos].variables[z] ==	pClauses[three_neg[x].variables[y]].variables[j])
						  //&&(pClauses[v3_1[i].pos].variables[z]<0))
						  {
						    count++;
						    v3_1[i].v1 = z;
						  }
					    }
					  }
					}
					if(count == 2) {
						//The counterpart clause to v3_1[i].pos is v3_1[i].neg
						v3_1[i].neg = three_neg[x].variables[y];
						out = 1;
					}
				}
				if(out == 0) {
				  v3_1[i].v0 = -1;
				  v3_1[i].v1 = -1;
				}
			}
			//Search through the clauses with 2 negative variables
			for(i = 0; i < v3_2count; i++) {
				out = 0;
				for(y = 0; (y < three_neg[x].length) && (!out); y++) {
					v3_2[i].v0 = -1;
					v3_2[i].v1 = -1;
					count = 0;
					for(z = 0; z < 3; z++) {
					  if(pClauses[v3_2[i].pos].variables[z] != x) {
					    for(j = 0; j < 3; j++) {
					      if((pClauses[v3_2[i].pos].variables[z] == pClauses[three_neg[x].variables[y]].variables[j])
						 &&(pClauses[v3_2[i].pos].variables[z] < 0))
						{
						  count++;
						  v3_2[i].v0 = z;
						} else if(-pClauses[v3_2[i].pos].variables[z] == pClauses[three_neg[x].variables[y]].variables[j])
						  //&&(-pClauses[v3_2[i].pos].variables[z]>0));
						  {
						    count++;
						    v3_2[i].v1 = z;
						  }
					    }
					  }
					}
					if(count == 2) {
					  //The counterpart clause to v3_2[i].pos is v3_2[i].neg
					  v3_2[i].neg = three_neg[x].variables[y];
					  out = 1;
					}
				}
				if(out == 0) {
				  v3_2[i].v0 = -1;
				  v3_2[i].v1 = -1;
				}
			}
			out = 0;
			for(i = 0; (i < v3_1count) && (!out); i++) {
				for(y = 0; (y < v3_2count) && (!out); y++) {
					if((v3_1[i].v0 == -1) || (v3_1[i].v1 == -1) 
					 ||(v3_2[y].v0 == -1) || (v3_2[y].v1 == -1))
					  continue;
					if(pClauses[v3_1[i].pos].variables[v3_1[i].v0] == -pClauses[v3_2[y].pos].variables[v3_2[y].v0]) {
						pClauses[v3_1[i].pos].length = -1;
						pClauses[v3_1[i].neg].length = -1;
						pClauses[v3_2[y].pos].length = -1;
						pClauses[v3_2[y].neg].length = -1;
						ites[num].vars[0] =  pClauses[v3_1[i].pos].variables[v3_1[i].v0];
						ites[num].vars[1] = -pClauses[v3_2[y].pos].variables[v3_2[y].v1];
						ites[num].vars[2] = -pClauses[v3_1[i].pos].variables[v3_1[i].v1];
						ites[num].vars[3] = x;
						for(z = 0; z < three_pos[x].length; z++) {
							count = 0;
							for(j = 0; j < 3; j++) {
								if((-pClauses[three_pos[x].variables[z]].variables[j] == ites[num].vars[1])
								 ||(-pClauses[three_pos[x].variables[z]].variables[j] == ites[num].vars[2]))
								  count++;
							}
							if(count == 2)
							  pClauses[three_pos[x].variables[z]].length = -1;
						}
						for(z = 0; z < three_neg[x].length; z++)	{
							count = 0;
							for(j = 0; j < 3; j++) {
								if((pClauses[three_neg[x].variables[z]].variables[j] == ites[num].vars[1])
								 ||(pClauses[three_neg[x].variables[z]].variables[j] == ites[num].vars[2]))
								  count++;
							}
							if(count == 2) pClauses[three_neg[x].variables[z]].length = -1;
						}
						num++;
						out = 1;
					}
				}
			}
		}
      for(int x = 1; x < nNumCNFVariables+1; x++) {
         free(three_pos[x].variables);
         free(three_neg[x].variables);
      }
      free(three_pos);
      free(three_neg);
      free(v3_1);
      free(v3_2);
      int num_ite = num;
      num = -1;

      for (long x = 0; x < nNumClauses+1; x++) {
         if(pClauses[x].length != -1) {
            num++;
            pClauses[num] = pClauses[x];
         }
      }
      nNumClauses = num;
      d3_printf2("Building ITE BDDs - %d\n", num_ite);

      vars_alloc(nNumCNFVariables);
      functions_alloc(nNumClauses+num_ite+recurselen+5+num_minmax+xors_found);
		
		//Creates BDD's for the ite_equal clauses
      for(long x = 0; x < num_ite; x++) {
         if (x%1000 == 0)
            d2_printf3("\rBuilding ITEs %ld/%d ...    ", x, num_ite);
         functions[x] = ite_itequ(ite_var(ites[x].vars[0]),
               ite_var(ites[x].vars[1]),
               ite_var(ites[x].vars[2]),
               ite_var(ites[x].vars[3]));
         functionType[x] = ITE_EQU;
         independantVars[ites[x].vars[3]] = 0;
         equalityVble[x] = ites[x].vars[3];
      }
      free(ites);
      
      //This or's all the variables in each individual clause
      d3_printf2("Building unclustered BDDs - %ld\n", nNumClauses);
		int num_uncluster_xor = 0;
      for(long x = 0; x < nNumClauses; x++) {
         if (x%1000 == 0)
            d2_printf3("\rBuilding unclustered and XOR BDDs %ld/%ld ... ", x, nNumClauses);
         //qsort(pClauses[x].variables, pClauses[x].length, sizeof(int), abscompfunc); //This is done above
         //qsort(pClauses[x].variables, pClauses[x].length, sizeof(int), absrevcompfunc);
         if(pClauses[x].isXor == 0) {
				functions[num_uncluster_xor+num_ite] = false_ptr;
				//	fprintf(stderr, "%d: %d ", x, pClauses[x].variables[0]);
				for(y = 0; y < pClauses[x].length; y++) {
					functions[num_uncluster_xor+num_ite] = ite_or(functions[num_uncluster_xor+num_ite], ite_var(pClauses[x].variables[y]));
					//	  fprintf(stderr, "%d ", pClauses[x].variables[y]);
				}
				//	fprintf(stderr, "0\n");
				functionType[num_uncluster_xor+num_ite] = PLAINOR;
				num_uncluster_xor++;
			} else if(pClauses[x].isXor == 1) {
				functions[num_uncluster_xor+num_ite] = false_ptr;
				//fprintf(stderr, "%d: %d ", x, pClauses[x].variables[0]);
				for(y = 0; y < pClauses[x].length; y++) {
					functions[num_uncluster_xor+num_ite] = ite_xor(functions[num_uncluster_xor+num_ite], ite_var(pClauses[x].variables[y]));
					//fprintf(stderr, "%d ", pClauses[x].variables[y]);
				}
            functionType[num_uncluster_xor+num_ite] = PLAINXOR;
				num_uncluster_xor++;
			} else {
				//do nothing
			}
      }
      nNumClauses = num_uncluster_xor+num_ite;
      
      //Creates BDD's for the and_equal and or_equal clauses
      d3_printf2("Building and= & or= BDDs - %d\n", recurselen);
		int num_and_or = 0;
      for(long x = 0; x < recurselen; x++) {
         if (x%1000 == 0)
            d2_printf3("\rBuilding and= & or= BDDs %ld/%d ... ", x, recurselen);
         //qsort(recurse[x].variables, recurse[x].length, sizeof(int), abscompfunc); //This is done above
         //qsort(recurse[x].variables, recurse[x].length, sizeof(int), absrevcompfunc);
			if(pClauses[x].isXor == 1) {
				functions[num_and_or+nNumClauses] = false_ptr;
				//	fprintf(stderr, "%d: %d ", x, pClauses[x].variables[0]);
				for(y = 0; y < pClauses[x].length; y++) {
					functions[num_and_or+nNumClauses] = ite_xor(functions[num_and_or+nNumClauses], ite_var(pClauses[x].variables[y]));
					//	  fprintf(stderr, "%d ", pClauses[x].variables[y]);
				}
				num_and_or++;
				functionType[num_and_or+nNumClauses] = PLAINXOR;
			}
         
			if(recurse[x].dag > 0) {
            functions[num_and_or+nNumClauses] = false_ptr;
            for(y = 0; y < recurse[x].length; y++) {
               if(recurse[x].variables[y] != recurse[x].andor)
                  functions[num_and_or+nNumClauses] = ite_or(functions[num_and_or+nNumClauses], ite_var(recurse[x].variables[y]));
            }
            functionType[num_and_or+nNumClauses] = OR;
            equalityVble[num_and_or+nNumClauses] = -recurse[x].andor;
            independantVars[equalityVble[num_and_or+nNumClauses]] = 0;
            functions[num_and_or+nNumClauses] = ite_equ(ite_var(-recurse[x].andor), functions[num_and_or+nNumClauses]);
				num_and_or++;
         } else {
            functions[num_and_or+nNumClauses] = true_ptr;
            for(y = 0; y < recurse[x].length; y++) {
               if(recurse[x].variables[y] != recurse[x].andor)
                  functions[num_and_or+nNumClauses] = ite_and(functions[num_and_or+nNumClauses], ite_var(-recurse[x].variables[y]));
            }
            functionType[num_and_or+nNumClauses] = AND;
            equalityVble[num_and_or+nNumClauses] = recurse[x].andor;
            independantVars[equalityVble[num_and_or+nNumClauses]] = 0;
            functions[num_and_or+nNumClauses] = ite_equ(ite_var(recurse[x].andor), functions[num_and_or+nNumClauses]);
				num_and_or++;
         }
      }
      free(recurse);
      nNumClauses = nNumClauses+num_and_or;
      d3_printf2("Building MinMax BDDs - %d\n", num_minmax);
      for(int x = 0; x < num_minmax; x++) {
         if (x % 1000 == 0) 
            d2_printf3("\rBuilding MinMax BDDs %d/%d ...               ", x, num_minmax);
         int set_true = 0;
         qsort(min_max_store[x].variables, min_max_store[x].length, sizeof(int), abscompfunc);
         functions[x+nNumClauses] = MinMaxBDD(min_max_store[x].variables, min_max_store[x].min, min_max_store[x].max, min_max_store[x].length, set_true);
			functionType[x+nNumClauses] = MINMAX;
      }
      ite_free((void**)&min_max_store);
      nNumClauses = nNumClauses+num_minmax;
   } else {
      vars_alloc(nNumCNFVariables);
      functions_alloc(nNumClauses+num_minmax);
		
      d3_printf2("Building unclustered BDDs - %ld\n", nNumClauses);
		
      for(long x = 1; x < nNumClauses + 1; x++) {
         if (x % 1000 == 1)
			  d2_printf3("\rBuilding unclustered BDDs %ld/%ld ...       ", x, nNumClauses);
         //qsort(pClauses[x].variables, pClauses[x].length, sizeof(int), abscompfunc); //This is done above
         //qsort(pClauses[x].variables, pClauses[x].length, sizeof(int), absrevcompfunc);
			functions[x-1] = false_ptr;
			//fprintf(stderr, "\n%d ", pClauses[x].variables[0]);
			for(y = 0; y < pClauses[x].length; y++) {
				functions[x - 1] = ite_or (functions[x-1], ite_var(pClauses[x].variables[y]));
				//fprintf(stderr, "%d ", pClauses[x].variables[y]);
			}
			functionType[x-1] = PLAINOR;	  
		}
		d3_printf2("Building MinMax BDDs - %d\n", num_minmax);
		for(int x = 0; x < num_minmax; x++) {
         if (x % 1000 == 0) 
			  d2_printf3("\rBuilding MinMax BDDs %d/%d ...       ", x, num_minmax);
         int set_true = 0;
         qsort(min_max_store[x].variables, min_max_store[x].length, sizeof(int), abscompfunc);
         functions[x+nNumClauses] = MinMaxBDD(min_max_store[x].variables, min_max_store[x].min, min_max_store[x].max, min_max_store[x].length, set_true);
			functionType[x+nNumClauses] = MINMAX;
		}
		ite_free((void**)&min_max_store);
		nNumClauses = nNumClauses+num_minmax;
   }
	count = -1;
	
	d3_printf1 ("Cleaning Up");
	
	//Need to remove any clause that was set to True during the making of the BDDs
	for(long x = 0; x < nNumClauses; x++) {
      count++;
      functions[count] = functions[x];
      equalityVble[count] = equalityVble[x];
      functionType[count] = functionType[x];
      if (functions[x] == true_ptr)
		  count--;
	}
	
	nNumClauses = count+1;
	nmbrFunctions = nNumClauses;
}	

*/

void DNF_to_CNF () {
	typedef struct {
		int num[50];
   } node1;
	node1 *integers;
	int lines = 0, len;
	int y = 0;
	if (fscanf(finputfile, "%ld %ld\n", &numinp, &numout) != 2) {
         fprintf(stderr, "Error while parsing DNF input: bad header\n");
         exit(1);
   };
	len = numout;
	integers = new node1[numout+1];
	for(int x = 0; x < len; x++) {
      y = 0;
      
      do {
			if (fscanf(finputfile, "%d", &integers[x].num[y]) != 1) {
            fprintf(stderr, "Error while parsing DNF input\n");
            exit(1);
         };
			y++;
			lines++;
		}
      while(integers[x].num[y - 1] != 0);
	}
	char string1[1024];
	lines = lines + 1;
	sprintf(string1, "p cnf %ld %d\n", numinp + len, lines);
	fprintf(foutputfile, "%s", string1);
	for(int y = 0; y < len; y++) {
      sprintf(string1, "%ld ", y + numinp + 1);
      fprintf(foutputfile, "%s", string1);
	}
	sprintf(string1, "0\n");
	fprintf(foutputfile, "%s", string1);
	for(int x = 0; x < numout; x++) {
      sprintf(string1, "%ld ", x + numinp + 1);
      fprintf(foutputfile, "%s", string1);
      for(int y = 0; integers[x].num[y] != 0; y++) {
			sprintf(string1, "%d ", -integers[x].num[y]);
			fprintf(foutputfile, "%s", string1);
		}
      sprintf(string1, "0\n");
      fprintf(foutputfile, "%s", string1);
      for(int y = 0; integers[x].num[y] != 0; y++) {
			sprintf(string1, "%ld ", -(x + numinp + 1));
			fprintf(foutputfile, "%s", string1);
			sprintf(string1, "%d ", integers[x].num[y]);
			fprintf(foutputfile, "%s", string1);
			sprintf(string1, "0\n");
			fprintf(foutputfile, "%s", string1);
		}
	}
	sprintf(string1, "c\n");
	fprintf(foutputfile, "%s", string1);
	//ite_free((void**)&integers);
}


void 
DNF_to_BDD () 
{
   store *dnf_integers = NULL;
   long dnf_numinp=0, dnf_numout=0;
   store *cnf_integers = NULL;
   long cnf_numinp=0, cnf_numout=0;
   int y = 0;
   if (fscanf(finputfile, "%ld %ld\n", &dnf_numinp, &dnf_numout) != 2) {
      fprintf(stderr, "Error while parsing DNF input: bad header\n");
      exit(1);
   }
   dnf_integers = (store*)ite_calloc(dnf_numout+1, sizeof(store), 9, "integers"); 
	d3_printf1("Storing DNF Inputs");
	for(int x = 0; x < dnf_numout; x++) {
		if (x%1000 == 1)
		  d2_printf3("\rReading DNF %d/%ld ... ", x, dnf_numout);
      y = 0;
      do {
         if (dnf_integers[x].num_alloc <= y) {
            dnf_integers[x].num = (int*)ite_recalloc((void*)dnf_integers[x].num, dnf_integers[x].num_alloc, 
                  dnf_integers[x].num_alloc+100, sizeof(int), 9, "integers[].num");
            dnf_integers[x].num_alloc += 100;
         }
         int intnum;
         if (fscanf(finputfile, "%d", &intnum) != 1) {
            fprintf(stderr, "Error while parsing DNF input\n");
            exit(1);
         }
         if(abs(intnum) > dnf_numinp) { //Could change this to be number of vars instead of max vars
            fprintf(stderr, "Variable in input file is greater than allowed:%ld...exiting\n", (long)dnf_numinp-2);
            exit(1);				
         }
#ifdef CNF_USES_SYMTABLE
         intnum = intnum==0?0:i_getsym_int(intnum, SYM_VAR);
#endif
         dnf_integers[x].num[y] = intnum;
         y++;
         cnf_numout++;
      } while(dnf_integers[x].num[y - 1] != 0);
      dnf_integers[x].max = y-1;
   }
   int extra=0;
   if (fscanf(finputfile, "%d", &extra) == 1) {
      fprintf(stderr, "Error while parsing DNF input: extra data\n");
      exit(1);
   }

   // DNF -> CNF
   cnf_numout = cnf_numout + 1; // add the big clause
   cnf_numinp = dnf_numinp + dnf_numout; // for every clause add a variable
   cnf_integers = (store*)ite_calloc(cnf_numout+1, sizeof(store), 9, "integers");  // the big clause

   int cnf_out_idx = 1; /* one based index? */
   cnf_integers[cnf_out_idx].num_alloc = dnf_numout+1;
   cnf_integers[cnf_out_idx].num = (int*)ite_calloc(cnf_integers[cnf_out_idx].num_alloc, sizeof(int), 9, "integers[].num");
	for(int y = 0; y < dnf_numout; y++) {
		if (y%1000 == 1)
		  d2_printf3("\rCreating Symbol Table Entries %d/%ld ... ", y, dnf_numout);
      cnf_integers[cnf_out_idx].num[y] = 
#ifdef CNF_USES_SYMTABLE
      i_getsym_int(y+dnf_numinp+1, SYM_VAR);
#else
      y + dnf_numinp + 1;
#endif
   }
   cnf_integers[cnf_out_idx].num[dnf_numout] = 0;
   cnf_out_idx++;

	for(int x = 0; x < dnf_numout; x++) {
		if (x%1000 == 1)
		  d2_printf3("\rTranslating DNF to CNF %d/%ld ...", x, dnf_numout);

		cnf_integers[cnf_out_idx].num_alloc = dnf_integers[x].max+2; /* plus clause var and 0 */
      cnf_integers[cnf_out_idx].num = (int*)ite_calloc(cnf_integers[cnf_out_idx].num_alloc, 
            sizeof(int), 9, "integers[].num");
      cnf_integers[cnf_out_idx].num[0] = cnf_integers[1].num[x]; // clause id

      int y;
      for(y = 0; dnf_integers[x].num[y] != 0; y++) {
         cnf_integers[cnf_out_idx].num[y+1] = -dnf_integers[x].num[y];
		}
      cnf_integers[cnf_out_idx].num[y+1] =  0;
      assert(y == dnf_integers[x].max);
      cnf_out_idx++;

      for(int y = 0; dnf_integers[x].num[y] != 0; y++) {
         cnf_integers[cnf_out_idx].num_alloc = 3;
         cnf_integers[cnf_out_idx].num = (int*)ite_calloc(cnf_integers[cnf_out_idx].num_alloc, 
            sizeof(int), 9, "integers[].num");
         cnf_integers[cnf_out_idx].num[0] = -cnf_integers[1].num[x]; // -clause id
         cnf_integers[cnf_out_idx].num[1] = dnf_integers[x].num[y];  
         cnf_integers[cnf_out_idx].num[2] = 0;
         cnf_out_idx++;
      }
	}
   assert(cnf_numout == cnf_out_idx-1);

	long x;
   numinp = cnf_numinp;
   numout = cnf_numout;

   for(x = 1; x <= cnf_numout; x++) {
      cnf_integers[x].length = cnf_integers[x].num_alloc-1;
      cnf_integers[x].dag =  -1;
   }

	DO_CLUSTER = 0;
//   cnf_process(cnf_integers, 0, NULL);
	
	for(x = 1; x < cnf_numout + 1; x++) {
      ite_free((void**)&cnf_integers[x].num);
	}
	ite_free((void**)&cnf_integers);

	for(x = 0; x < dnf_numout + 1; x++) {
      ite_free((void**)&dnf_integers[x].num);
	}
	ite_free((void**)&dnf_integers);
	
	d3_printf2("Number of BDDs - %ld\n", numout);
	d2_printf1("\rReading DNF ... Done                   \n");
}

/*
void DNF_to_CNF () {
	typedef struct {
		int num[50];
   } node1;
	node1 *integers;
	int lines = 0, len;
	int y = 0;
	fscanf(finputfile, "%ld %ld\n", &numinp, &numout);
	len = numout;
	integers = new node1[numout+1];
	for(int x = 0; x < len; x++) {
      y = 0;
      
      do {
			fscanf(finputfile, "%d", &integers[x].num[y]);
			y++;
			lines++;
		}
      while(integers[x].num[y - 1] != 0);
	}
	char string1[1024];
	lines = lines + 1;
	sprintf(string1, "p cnf %ld %d\n", numinp + len, lines);
	fprintf(foutputfile, "%s", string1);
	for(int y = 0; y < len; y++) {
      sprintf(string1, "%ld ", y + numinp + 1);
      fprintf(foutputfile, "%s", string1);
	}
	sprintf(string1, "0\n");
	fprintf(foutputfile, "%s", string1);
	for(int x = 0; x < numout; x++) {
      sprintf(string1, "%ld ", x + numinp + 1);
      fprintf(foutputfile, "%s", string1);
      for(int y = 0; integers[x].num[y] != 0; y++) {
			sprintf(string1, "%d ", -integers[x].num[y]);
			fprintf(foutputfile, "%s", string1);
		}
      sprintf(string1, "0\n");
      fprintf(foutputfile, "%s", string1);
      for(int y = 0; integers[x].num[y] != 0; y++) {
			sprintf(string1, "%ld ", -(x + numinp + 1));
			fprintf(foutputfile, "%s", string1);
			sprintf(string1, "%d ", integers[x].num[y]);
			fprintf(foutputfile, "%s", string1);
			sprintf(string1, "0\n");
			fprintf(foutputfile, "%s", string1);
		}
	}
	sprintf(string1, "c\n");
	fprintf(foutputfile, "%s", string1);
}
*/
