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

/***********************************************************************
 *  bdd2cnf.c (J. Franco, Sean Weaver)
 *  Function bdd2cnf converts an input BDD to a CNF expression and puts
 *  the result into a file.  
 *  Function printBDDToCNF3SAT converts a collection of BDDs to 3-SAT CNF.
 *  Function printBDDToCNF converts a collection of BDDs to CNF.
 ***********************************************************************/

#include "sbsat.h"
#include "sbsat_formats.h"

#define F (numinp+3)
#define T (numinp+2)

int use_symtable = 0;

void cnf_print_independent_vars() {
	fprintf(foutputfile, "c Independent Variables: ");
	for(int x = 1; x <= numinp; x++) {
		if(independantVars[x] == 1-reverse_independant_dependant) 
		  fprintf (foutputfile, "%d ", use_symtable?-atoi(getsym_i(x)->name):x);
	}
	fprintf(foutputfile, "0\n");
}

void print_cnf_symtable() {
	for(int x = 1; x <= numinp; x++)
	  fprintf(foutputfile, "c %d = %s\n", x, getsym_i(x)->name);
}

void getMaxNo (BDDNode * bdd, int *no) {
   if (bdd == false_ptr || bdd == true_ptr)
      return;
   if (*no < bdd->variable)
      *no = bdd->variable;
   bdd->tmp_int = 0;
   getMaxNo (bdd->thenCase, no);
   getMaxNo (bdd->elseCase, no);
}

// Find the maximum numbered variable in a given bdd. Uses "getMaxNo" above 
int getMaxVarNoInBDD (BDDNode * bdd) {
  int number = -1;
  getMaxNo (bdd, &number);
  return number;
}

// A recursive post-order traversal of a BDD (bdd), beginning at its root, to  
// compose CNF clauses representing each node.  Clauses are placed in a file   
// called "bdd_tmp.cnf".  On the first visit to a BDD node, the code assigns   
// a new (*curr) variable which is placed in the node's tmp_int field (assumed   
// initially to be 0) unless the node has thenCase and elseCase pointing to    
// true,false or false,true.  If the thenCase and elseCase pointers are set    
// to true,false, or false,true then tmp_int gets the number of the node's bdd   
// variable (bdd->variable).  If the thenCase and elseCase pointers are set    
// to false,true then t_sgn is set to false (it is otherwise true).  Also on   
// the first visit, deeper visits to children are invoked.  Upon returning,    
// up to four clauses are generated for the current node.  On future visits    
// to this node no other clauses will be generated.  The clauses merely state  
// logically the if-then-else conditions of the branch represented by the node.
// Thus they look like this:                                                   
//   (i, v, !e) & (i, !v, !t) & (!i, v, e) & (!i, !v, t)                       
// where i is the made-up "if" variable, t is the made-up "then" variable, e   
// is the made-up "else" variable, and v is the actual variable of the node's  
// branch.  Thus, the simple bdd representing:                                 
//   4 = ite(1,2,3);                                                           
// Will produce the following clausal output in "bdd_tmp.cnf":                 
//   5 3 4 0    
//   5 -3 -4 0  
//   -5 3 -4 0  
//   -5 -3 4 0  
//   6 2 4 0    
//   6 -2 -4 0  
//   -6 2 -4 0  
//   -6 -2 4 0  
//   7 1 -5 0   
//   7 -1 -6 0  
//   -7 1 5 0   
//   -7 -1 6 0  
// Inputs:                                                                     
//   bdd:    the root of a BDD                                                 
//   curr:   first available number for naming a made-up variable              
//   cl_cnt: will contain a count of clauses formed                            
//   ft:     file "bdd_tmp.cnf"                                                

void bdd2cnf (BDDNode * bdd, int *curr, int *cl_cnt, FILE * ft) {
   int i, t, e,			// made-up vars: i=if, t=then, e=else
      v;			// Actual variable of node
   char buffer[1024];		// For composing lines to be written to "ft"
   
   if (bdd == false_ptr || bdd == true_ptr) {
      fprintf(stderr, "Something is wrong: Formula is garbage.\n");
      exit (1);
   }

   if (bdd->thenCase == true_ptr && bdd->elseCase == false_ptr) {
      bdd->tmp_int = use_symtable?atoi(getsym_i(bdd->variable)->name):bdd->variable;
      return;
   } else if (bdd->thenCase == false_ptr && bdd->elseCase == true_ptr) {
      bdd->tmp_int = use_symtable?-atoi(getsym_i(bdd->variable)->name):-bdd->variable;
      return;
   } else {
      // Setup values for at most four variables involved at a BDD node    
      // v (actual): actual variable assigned to the node                  
      // e (else):   made-up variable: true iff else branch is             
      // t (then):   made-up variable: true iff then branch is             
      // i (if):     made-up variable: true iff v=T and t=T or v=F and e=T 
      e = bdd->elseCase->tmp_int;
      if (e == 0) {
			if (bdd->elseCase == true_ptr) {
				e = T;
			} else if (bdd->elseCase == false_ptr) {
				e = F;
			} else {
				bdd2cnf (bdd->elseCase, curr, cl_cnt, ft);
				e = bdd->elseCase->tmp_int;
			}
      }
		
      t = bdd->thenCase->tmp_int;
      if (t == 0) {
			if (bdd->thenCase == true_ptr) {
				t = T;
			} else if (bdd->thenCase == false_ptr) {
				t = F;
			} else {
				bdd2cnf (bdd->thenCase, curr, cl_cnt, ft);
				t = bdd->thenCase->tmp_int;
			}
      }
		
      i = bdd->tmp_int;
      if (i == 0) {
			i = (*curr)++;
			bdd->tmp_int = i;
      }
		
      v = use_symtable?atoi(getsym_i(bdd->variable)->name):bdd->variable;
		
      // Ready to output clauses to file:                    
      // Generally it's:                                     
      // (i, v, !e) & (i, !v, !t) & (!i, v, e) & (!i, !v, t) 
      // First clause:
      if (e == T) {
			if (abs (i) < abs (v))
			  sprintf (buffer, "%d %d 0\n", i, v);
			else
			  sprintf (buffer, "%d %d 0\n", v, i);
			if (fputs (buffer, ft) < 0) {
				fprintf(stderr, "Error writing to bdd_tmp.cnf\n");
				unlink ("bdd_tmp.cnf");
				exit (1);
			}
			(*cl_cnt)++;
      } else if (e != F) {
			if (abs (i) < abs (v)) {
				if (abs (i) < abs (e)) {
					if (abs (v) < abs (e))
					  sprintf (buffer, "%d %d %d 0\n", i, v, -e);
					else
					  sprintf (buffer, "%d %d %d 0\n", i, -e, v);
				} else {
					sprintf (buffer, "%d %d %d 0\n", -e, i, v);
				}
			} else {
				if (abs (e) < abs (v)) {
					sprintf (buffer, "%d %d %d 0\n", -e, v, i);
				} else {
					if (abs (i) < abs (e))
					  sprintf (buffer, "%d %d %d 0\n", v, i, -e);
					else
					  sprintf (buffer, "%d %d %d 0\n", v, -e, i);
				}
			}
			if (fputs (buffer, ft) < 0) {
				fprintf(stderr, "Error writing to bdd_tmp.cnf\n");
				unlink ("bdd_tmp.cnf");
				exit (1);
			}
			(*cl_cnt)++;
      }
      // Second clause:
      if (t == T) {
			if (abs (i) < abs (v))
			  sprintf (buffer, "%d -%d 0\n", i, v);
			else
			  sprintf (buffer, "-%d %d 0\n", v, i);
			if (fputs (buffer, ft) < 0) {
				fprintf(stderr, "Error writing to bdd_tmp.cnf\n");
				unlink ("bdd_tmp.cnf");
				exit (1);
			}
			(*cl_cnt)++;
      } else if (t != F) {
			if (abs (i) < abs (v)) {
				if (abs (i) < abs (t)) {
					if (abs (v) < abs (t))
					  sprintf (buffer, "%d -%d %d 0\n", i, v, -t);
					else
					  sprintf (buffer, "%d %d -%d 0\n", i, -t, v);
				} else {
					sprintf (buffer, "%d %d -%d 0\n", -t, i, v);
				}
			} else {
				if (abs (t) < abs (v)) {
					sprintf (buffer, "%d -%d %d 0\n", -t, v, i);
				} else {
					if (abs (i) < abs (t))
					  sprintf (buffer, "-%d %d %d 0\n", v, i, -t);
					else
					  sprintf (buffer, "-%d %d %d 0\n", v, -t, i);
				}
			}
			if (fputs (buffer, ft) < 0) {
				fprintf(stderr, "Error writing to bdd_tmp.cnf\n");
				unlink ("bdd_tmp.cnf");
				exit (1);
			}
			(*cl_cnt)++;
      }
      // Third clause:
      if (e == F) {
			if (abs (i) < abs (v))
			  sprintf (buffer, "%d %d 0\n", -i, v);
			else
			  sprintf (buffer, "%d %d 0\n", v, -i);
			if (fputs (buffer, ft) < 0) {
				fprintf(stderr, "Error writing to bdd_tmp.cnf\n");
				unlink ("bdd_tmp.cnf");
				exit (1);
			}
			(*cl_cnt)++;
      } else if (e != T) {
			if (abs (i) < abs (v)) {
				if (abs (i) < abs (e)) {
					if (abs (v) < abs (e))
					  sprintf (buffer, "%d %d %d 0\n", -i, v, e);
					else
					  sprintf (buffer, "%d %d %d 0\n", -i, e, v);
				} else {
					sprintf (buffer, "%d %d %d 0\n", e, -i, v);
				}
			} else {
				if (abs (e) < abs (v)) {
					sprintf (buffer, "%d %d %d 0\n", e, v, -i);
				} else {
					if (abs (i) < abs (e))
					  sprintf (buffer, "%d %d %d 0\n", v, -i, e);
					else
					  sprintf (buffer, "%d %d %d 0\n", v, e, -i);
				}
			}
			if (fputs (buffer, ft) < 0) {
				fprintf(stderr, "Error writing to bdd_tmp.cnf\n");
				unlink ("bdd_tmp.cnf");
				exit (1);
			}
			(*cl_cnt)++;
      }
      // Fourth clause:
      if (t == F) {
			if (abs (i) < abs (v))
			  sprintf (buffer, "%d -%d 0\n", -i, v);
			else
			  sprintf (buffer, "-%d %d 0\n", v, -i);
			if (fputs (buffer, ft) < 0) {
				fprintf(stderr, "Error writing to bdd_tmp.cnf\n");
				unlink ("bdd_tmp.cnf");
				exit (1);
			}
			(*cl_cnt)++;
      } else if (t != T) {
			if (abs (i) < abs (v)) {
				if (abs (i) < abs (t)) {
					if (abs (v) < abs (t))
					  sprintf (buffer, "%d -%d %d 0\n", -i, v, t);
					else
					  sprintf (buffer, "%d %d -%d 0\n", -i, t, v);
				} else {
					sprintf (buffer, "%d %d -%d 0\n", t, -i, v);
				}
			} else {
				if (abs (t) < abs (v)) {
					sprintf (buffer, "%d -%d %d 0\n", t, v, -i);
				} else {
					if (abs (i) < abs (t))
					  sprintf (buffer, "-%d %d %d 0\n", v, -i, t);
					else
					  sprintf (buffer, "-%d %d %d 0\n", v, t, -i);
				}
			}
			if (fputs (buffer, ft) < 0) {
				fprintf(stderr, "Error writing to bdd_tmp.cnf\n");
				unlink ("bdd_tmp.cnf");
				exit (1);
			}
			(*cl_cnt)++;
      }
   }
}

// Makes a CNF expression from a given _ircuit "c"
// Output is standard out                                                      
// Input is a _ircuit "c" consisting of BDDs with variables identified as ints 
// Process depends on creating new (made-up) variables, at most one for each   
//    node of the BDDs                                                         
// Uses "getMaxVarNoInBDD" to find the first int available for use as a        
//    made-up variable.  That information is needed so we do not "run-over"    
//    original variables when creating new ones.                               
// Then bdd2cnf is invoked to build a file of clauses.                         
// Finally the CNF output is built using the clause and variable counts        
//    obtained from bdd2cnf to create a preamble and the contents of           
//    "bdd_tmp.cnf"                                                            

void printBDDToCNF3SAT () {
   FILE *ft;  char buffer[1024];
   int clause_cnt = 0, t_cnt, max_var_no = 0;
   char tmp_bdd_filename[256];
   get_freefile("bdd_tmp.cnf", temp_dir, tmp_bdd_filename, 255);

	use_symtable = sym_all_int();
	
   for (int i = 0; i < nmbrFunctions; i++) {
      //if (functionType[i] != 0) {
      // fprintf(stderr, 
      //    "Unable to deal with any structure except BDDs now\n");
      // exit (1);
      //}
      if (max_var_no < (t_cnt = getMaxVarNoInBDD (functions[i])))
	 max_var_no = t_cnt;
   }
   
   max_var_no++;
   if ((ft = fopen (tmp_bdd_filename, "wb+")) == NULL) {
      fprintf(stderr, "Cannot open bdd_tmp.cnf for writing\n");
      exit(1);
   }

   for (int i = 0; i < nmbrFunctions; i++) {
      //printBDD (functions[i]);
      bdd2cnf (functions[i], &max_var_no, &clause_cnt, ft);
      sprintf (buffer, "%d 0\n", functions[i]->tmp_int);
      if (fputs (buffer, ft) < 0) {
			fprintf(stderr, "Error writing to bdd_tmp.cnf\n");
			unlink ("bdd_tmp.cnf");
			exit (1);
      }
      clause_cnt++;
   }

//   fputs ("c end\n", ft);
   fclose (ft);
   if ((ft = fopen (tmp_bdd_filename, "rb")) == NULL) {
      fprintf(stderr, "Cannot open bdd_tmp.cnf for reading\n");
      exit(1);
   }

	if(!use_symtable) {
		print_cnf_symtable();
	}
   
	if(print_independent_vars) {
		cnf_print_independent_vars();
	}
	
	fprintf (foutputfile, "p cnf %d %d\n", max_var_no - 1, clause_cnt);
   while (fgets (buffer, 1023, ft) != NULL)
      fprintf (foutputfile, "%s", buffer);
   fclose (ft);
   unlink (tmp_bdd_filename);
}

// Top level call has n=m - it returns all 1-0 strings of m bits with 
// exactly p 1s. 
Recd *masker (int p, int n, int m) { // p = number of 1s, n = number of bits
   if (p == n || p == 0) {
      Recd *tmp = new Recd;
      tmp->next = NULL;
      tmp->data = (char *)calloc(1,sizeof(char)*m);
      for (int i=0 ; i < p ; i++) tmp->data[i] = 1;
      return tmp;
   }
   Recd *z1 = masker (p, n-1, m);
   Recd *z2 = masker (p-1, n-1, m);
   Recd *ptr;
   for (ptr = z2 ; ptr->next != NULL; ptr = ptr->next) {
      ptr->data[n-1] = 1;
   }
   ptr->data[n-1] = 1;
   ptr->next = z1;
   return z2;
}

// Cleans up any Recd linked list.
void getRidOfRecdList (Recd *list) {
   for (Recd *ptr=list ; ptr != NULL ; ) {
      Recd *tmp = ptr->next;
      delete ptr->data;
      delete ptr;
      ptr = tmp;
   }
}

// Returns an integer which is the value of a choose b 
int choose (int a, int b) {
   if (a > b) return 0;
   if (a == 0 || a == b) return 1;
   long num = b;
   long den = (a < b-a) ? a : b-a;
   long c = den;
   if (den == 0) return 1;
   for (int i=1 ; i < c ; i++) num *= (long)(b-i);
   for (int i=1 ; i < c ; i++) den *= (long)(c-i);
   return (int)(num/den);
}

int countNumberLiterals (Recd *clauses, int no_vars) {
   int literals = 0;
   Recd *ptr = clauses;
   for ( ; ptr != NULL ; ptr=ptr->next) {
      for (int i=0 ; i < no_vars ; i++) if (ptr->data[i] != 2) literals++;
   }
   return literals;
}

int countNumberClauses (Recd *clauses) {
   int m = 0;
   Recd *ptr = clauses;
   for ( ; ptr != NULL ; ptr=ptr->next) m++;
   return m;
}

Recd *applyBoundedResolution (Recd *clauses,int k,bool *changes, int no_vars) {
   int pivot, i, comlits;
   char *tmp;
   Recd *front = clauses;
   Recd *anchor;
   Recd *sweep;
   Recd *resolvent;
	
   if (clauses == NULL) return NULL;

   if ((tmp = (char *)calloc(1, sizeof(char)*no_vars)) == NULL) {
      fprintf(stderr,"Out of memory - B\n");
      exit (0);
   }

   // fprintf(stderr,"resolving...\n");
   //j=0;
   for (anchor = clauses ; anchor->next != NULL ; anchor = anchor->next) {
      // fprintf(stderr,"\r%d     ",j++);
      for (sweep = anchor->next ; sweep != NULL ; sweep = sweep->next) {
	 pivot = -1;
	 comlits = 0;
	 for (i=0 ; i < no_vars ; i++) {
	    if (anchor->data[i] == 2) {
	       tmp[i] = sweep->data[i];
	       if (tmp[i] != 2) comlits++;
	    } else if (sweep->data[i] == 2) {
	       tmp[i] = anchor->data[i];
	       if (tmp[i] != 2) comlits++;
	    } else if ((sweep->data[i] == 0 && anchor->data[i] == 1) ||
		       (sweep->data[i] == 1 && anchor->data[i] == 0)) {
	       if (pivot != -1) break;
	       pivot = i;
	       tmp[i] = 2;
	    } else if (sweep->data[i] == anchor->data[i]) { // must be 0 or 1
	       tmp[i] = sweep->data[i];
	       comlits++;
	    }
	 }
	 if (i == no_vars && comlits <= k) {
	    resolvent = new Recd;
	    resolvent->next = front;
	    front = resolvent;
	    if ((front->data = (char*)calloc(1, sizeof(char)*no_vars))==NULL) {
	       fprintf(stderr,"Out of memory - C\n");
	       exit (0);
	    }
	    for (int i=0 ; i < no_vars ; i++) front->data[i] = tmp[i];
	 }
      }
   }
   delete tmp;
   if (front != clauses) *changes = true; else *changes = false;
   return front;
}

Recd *applySubsumption (Recd *clauses, bool *change, int no_vars) {
   Recd *cursor, *tester, *front, *ptr;
   int i/*,j*/;
	
   if (clauses == NULL) return NULL;
	
   *change = false;
   front = new Recd;
   front->next = clauses;
	
   // fprintf(stderr,"\nsubsuming...\n");
   //j=0;
   for (cursor = clauses ; cursor != NULL ; cursor = cursor->next) {
      // fprintf(stderr,"\r%d     ",j++);
      for (tester = front ; tester->next != NULL ; ) {
	 if (tester->next == cursor) { tester = tester->next; continue; }
	 ptr = tester->next;
	 for (i=0 ; i < no_vars ; i++) {
	    if (cursor->data[i] < 2 && cursor->data[i] != ptr->data[i]) break;
	 }
	 if (i == no_vars) {
	    *change = true;
	    tester->next = ptr->next;
	    delete ptr->data;
	    delete ptr;
	 } else {
	    tester = tester->next;
	 }
      }
   }
   return front->next;
}

// Limit of 31 on no_vars, unchecked
Recd *resolve (int *truth_table, int sign, int no_vars, int no_outmp_ints) {
   Recd ***clauses = NULL;

   // For each i < no_vars, build a linked list for each set of 
   // clauses with i 1's.  This make Quine McClusky style resolutions
   // quicker to identify.
   clauses = (Recd ***)calloc(1, sizeof(Recd **)*(no_vars+1));
   for (int i=0 ; i <= no_vars ; i++) {
      clauses[i] = (Recd **)calloc(1, sizeof(Recd *)*(no_vars+1));
      for (int j=0 ; j <= no_vars ; j++) {
	 clauses[i][j] = NULL;
      }
   }

   // For all truth table entries matching sign, build the 1-0 pattern 
   // corresponding to that entry.
   for (int i=0 ; i < (1 << no_vars) ; i++) {
      if (truth_table[i] != sign) continue;
      Recd *tmp = new Recd;
      tmp->data = (char *)calloc(1,sizeof(char)*no_vars);
      int n_ones = 0;
      for (int j=0 ; j < no_vars ; j++) {
         tmp->data[j] = (i/(1 << j) % 2);
	 if (tmp->data[j] == 1) n_ones++;
      }
      tmp->next = clauses[n_ones][0];
      tmp->used = false;
      clauses[n_ones][0] = tmp;
   }

   //  Test: print all clauses by number of 1s  
	/*
   for (int i = 0 ; i <= no_vars ; i++) {
      fprintf(stdout, "\n----------------\nLength %d\n", i);
      for (int k=0 ; k <= no_vars ; k++) {
	 Recd *ptr = clauses[i][k];
	 while (ptr != NULL) {
	    for (int j=0 ; j < no_vars ; j++)
	       if (ptr->data[j]) fprintf(stdout,"1"); else fprintf(stdout,"0");
	    fprintf(stdout,"\n");
	    ptr = ptr->next;
	 }
      }
   }
	*/
   // resolve clauses containing same number i of don't cares
   for (int i=0 ; i <= no_vars ; i++) {
      // resolve clauses between groups of j and j+1 1's
      for (int j=0 ; j < no_vars ; j++) {
	 Recd *ptr2, *tp2;
	 for (ptr2=clauses[j][i] ; ptr2 != NULL ; ptr2=ptr2->next) {
	    for (tp2=clauses[j+1][i] ; tp2 != NULL ; tp2=tp2->next) {
	       int pivot = -1;  // if res possible, has position of pivot
	       int ii;
	       // Both clauses have the same number of 2's hence all we
	       // have to look for is whether data positions differ in
	       // more than one place.
	       for (ii=0 ; ii < no_vars ; ii++) {
		  if ((ptr2->data[ii] == 0 && tp2->data[ii] == 1) ||
		      (ptr2->data[ii] == 1 && tp2->data[ii] == 0)) {
		     if (pivot == -1) { 
			pivot = ii;
		     } else { 
			pivot = -2 ;  // Forget it, cannot resolve
			break; 
		     }
		  } else if (ptr2->data[ii] != tp2->data[ii]) {
		     pivot = -2;
		     break;
		  }
	       }
	       if (ii == no_vars) {  // resolution is possible
		  if (pivot < 0) {
		     fprintf(stderr,"Error: pivot cannot be negative.\n");
		     exit(0);
		  }
		  ptr2->used = true;
		  tp2->used = true;
		  // Check whether this one already exists
		  Recd *ptr1;
		  for (ptr1=clauses[j][i+1]; ptr1 != NULL ; ptr1=ptr1->next){
		     int l;
		     for (l=0 ; l < no_vars ; l++) {
			if (l == pivot && ptr1->data[l] != 2) break;
			else if (l != pivot && 
				 ptr1->data[l] != ptr2->data[l]) break;
		     }
		     if (l == no_vars) break;
		  }
		  // If it is not already there then add it in
		  if (ptr1 == NULL) {
		     Recd *tmp = new Recd;
		     tmp->data = (char *)calloc(1,sizeof(char)*no_vars);
		     for (int k=0 ; k < no_vars ; k++) {
			if (k == pivot)
			   tmp->data[k] = 2;
			else
			   tmp->data[k] = ptr2->data[k];
		     }
		     tmp->next = clauses[j][i+1];
		     tmp->used = false;
		     clauses[j][i+1] = tmp;
		  }
	       }
	    }
	 }            
      }
   }

   // fprintf(stderr,"Clauses: %d literals: %d\n",ccount, lcount);
   // Assemble all the clauses into one pile
   // delete those which are used
   Recd *ret = NULL;
   for (int i=0 ; i <= no_vars ; i++) {
      for (int j=0 ; j <= no_vars ; j++) {
	 Recd *ptr;
	 for (ptr=clauses[i][j]; ptr != NULL ; ) {
	    Recd *tptr = ptr->next;
	    if (ptr->used == false) {
	       ptr->next = ret;
	       ret = ptr;
	    } else {
	       delete ptr->data;
	       delete ptr;
	    }
	    ptr = tptr;
	 }
      }
      delete clauses[i];
   }
   delete clauses;

   /**************/
   bool changeRes, changeSub;
   int occount=0, olcount=0;
   int lcount = countNumberLiterals (ret, no_vars);
   int ccount = countNumberClauses (ret);

   while (true) {
      ret = applyBoundedResolution (ret, no_vars, &changeRes, no_vars);
      ret = applySubsumption (ret, &changeSub, no_vars);
      lcount = countNumberLiterals (ret, no_vars);
      ccount = countNumberClauses (ret);
      if (ccount == occount && lcount == olcount) break;
      occount = ccount;
      olcount = lcount;
   }
   /**************/

   // Test: print all clauses
   /*
   fprintf(stdout,"\nOutput:\n-------\n");
   for (Recd *ptr=ret ; ptr != NULL ; ptr=ptr->next) {
      for (int j=0 ; j < no_vars ; j++) 
	 if (ptr->data[j] == 1) fprintf(stdout,"1"); 
	 else if (ptr->data[j] == 0) fprintf(stdout,"0");
	 else fprintf(stdout,"2");
      fprintf(stdout,"\n");
   }
   */

   return ret;
}

void printBDDToCNFQM () {
   int *tempint=NULL, z;
   int tempint_max=0;
	
   func_object **funcs;
   int no_outmp_ints;
   
   z = 0;
   for (int x = 0; x < numout; x++) {
      int numx = countFalses (functions[x]);
      z+=numx;
   } //Count the clauses

   no_outmp_ints = z;
	
   funcs = (func_object **)calloc(1,sizeof(func_object *)*nmbrFunctions);
   for (int i=0 ; i < nmbrFunctions ; i++) funcs[i] = new func_object;
	
   numinp = getNuminp ();
   
	z = 0;
   
   for (int x = 0; x < numout; x++) {
      fprintf(stderr, "[%d]     \r", x);
      int no_vars = 0;
		
      int y = 0;
      unravelBDD(&y, &tempint_max, &tempint, functions[x]);
      if (y != 0) qsort(tempint, y, sizeof (int), compfunc);
      
      no_vars = y;
      funcs[x]->no_vars = no_vars;
      funcs[x]->var_list = (int *)calloc(1,sizeof(int)*no_vars);
      char *p = new char[no_vars+1];
      for (int i = 0; i < no_vars; i++) {
			funcs[x]->var_list[i] = tempint[i];
			p[i] = '0';
      }
      
      int *truth_table = new int[1 << no_vars];
      
		//printBDDerr(functions[x]);
		//fprintf(stdout, "\n");
      for (int tvec = 0; tvec < (1 << no_vars); tvec++) {
			truth_table[tvec] = 
			  getTruth (funcs[x]->var_list, p, no_vars, functions[x]);
			//fprintf(stdout, "%d ", truth_table[tvec]);
			bcount (p, (no_vars - 1));
      }
		//fprintf(stdout, "\n");
      delete p;
      
      int *literals;
      int equiv;
      // Fully QM and resolve the clauses
      switch (functionType[x]) {
			//case NAND:
			//fprintf(stdout,"NAND\n");
			//break;
			//case NOR:
			// fprintf(stdout,"NOR\n");
			//break;
		 case OR:
		 case AND:
			// Assume a -1,+1 array of size no_vars showing signs of literals
         // and an index showing which position is the equivalence variable
	 		// The signs of the literals are opposite from the actual value of
			// the literal.
			literals = getANDLiterals(x, funcs[x]->var_list, no_vars);
			// index into literals of left var
			equiv = getEquiv(x, funcs[x]->var_list, no_vars);
			
			funcs[x]->reduced0 = new Recd;
			funcs[x]->reduced0->data = (char *)calloc(1,sizeof(char)*no_vars);
			funcs[x]->reduced0->next = NULL;
			
			// The long clause
			for (int i=0 ; i < no_vars ; i++) {
				if ((literals[i] > 0 && equiv != i) ||
					 (literals[i] < 0 && equiv == i))
				  funcs[x]->reduced0->data[i] = 1;
				else
				  funcs[x]->reduced0->data[i] = 0;
			}
			
			// The short clauses
			for (int i=0 ; i < no_vars ; i++) {
				if (equiv == i) continue;
				Recd *tmp = funcs[x]->reduced0;
				funcs[x]->reduced0 = new Recd;
				funcs[x]->reduced0->data = (char *)calloc(1,sizeof(char)*no_vars);
				funcs[x]->reduced0->next = tmp;
				for (int j=0 ; j < no_vars ; j++) {
					if (j == i) 
					  funcs[x]->reduced0->data[j] = (literals[j] > 0) ? 0 : 1;
					else if (j == equiv)
					  funcs[x]->reduced0->data[j] = (literals[j] > 0) ? 1 : 0;
					else funcs[x]->reduced0->data[j] = 2;
				}
			}

			/*
			fprintf(stdout,"Long and:\n-----------\n");
			fprintf(stdout,"Literals:");
			for (int i=0 ; i < no_vars ; i++) fprintf(stdout, "%d ", literals[i]);
			fprintf(stdout, "equiv:%d\n", equiv);
			fprintf(stdout,"-----------\n");
			for (Recd *ptr=funcs[x]->reduced0 ; ptr != NULL ; ptr=ptr->next) {
				for (int i=0 ; i < no_vars ; i++) {
					fprintf(stdout,"%d ", ptr->data[i]);
				}
				fprintf(stdout,"\n");
			}
			fprintf(stdout,"-----------\n");
			*/
			
			delete literals;
			literals = NULL;
			break;
		 default:
			funcs[x]->reduced0 = resolve (truth_table, 0, no_vars, no_outmp_ints);
			break;
      }
      delete truth_table;
   }
   ite_free((void**)&tempint); tempint_max = 0;
   
   z = 0;
   for(int x = 0; x < numout; x++) {
      Recd *res = funcs[x]->reduced0;
      for ( ; res != NULL; res=res->next) {
			z++;
      }
   }
   no_outmp_ints = z;

	use_symtable = sym_all_int(); //returns the maximum integer used, or 0 if any non integers are used.
	if(!use_symtable) {
		print_cnf_symtable();
	}

	if(print_independent_vars) {
		cnf_print_independent_vars();
	}
	
   numinp = getNuminp ();

	if(use_symtable)
	  fprintf(foutputfile, "p cnf %ld %d\n", use_symtable, no_outmp_ints);
	else
	  fprintf(foutputfile, "p cnf %ld %d\n", numinp, no_outmp_ints);
	
   for(int x = 0; x < numout; x++) {
		Recd *res = funcs[x]->reduced0;
		for ( ; res != NULL; res=res->next) {
			for(int i = 0; i < funcs[x]->no_vars; i++) {
				int *vlst = funcs[x]->var_list;
				int literal = (int) res->data[i];
				if(literal < 2) {
					if(literal == 1) fprintf(foutputfile, "%d ", use_symtable?-atoi(getsym_i(vlst[i])->name):-vlst[i]);
					else fprintf(foutputfile, "%d ", use_symtable?atoi(getsym_i(vlst[i])->name):vlst[i]);
				}
			}
			fprintf(foutputfile, "0\n");
      }
      getRidOfRecdList (res);		
   }
	fprintf(foutputfile, "c");
}

void printBDDToCNF () {
   int *tempint = NULL, z;
   int tempint_max = 0;
	
   int no_outmp_ints;
   
   z = 0;
   for (int x = 0; x < numout; x++) {
      int numx = countFalses (functions[x]);
      z+=numx;
   } //Count the clauses

   no_outmp_ints = z;
	
   numinp = getNuminp ();
   
   intlist *false_paths = new intlist[no_outmp_ints+1];
   z = 0;
   
   for (int x = 0; x < numout; x++) {
      fprintf(stderr, "[%d]     \r", x);
      int no_vars = 0;
		int numx = countFalses (functions[x]);
      intlist *list = new intlist[numx];
      int listx = 0;
      findPathsToFalse (functions[x], &tempint_max, &tempint, list, &listx);
		
      for (int i = 0; i < listx; i++) {
			false_paths[z].num = list[i].num;
			false_paths[z].length = list[i].length;
			if(false_paths[z].length > no_vars) no_vars = false_paths[z].length;
			for (int a = 0; a < false_paths[z].length; a++) {
				//fprintf(stdout, "%d ", false_paths[z].num[a]);
			}
			//fprintf(stdout, "0\n");
			z++;
      }
		delete list;
	}

	ite_free((void**)&tempint); tempint_max = 0;
	
	use_symtable = sym_all_int(); //returns the maximum integer used, or 0 if any non integers are used.
	if(!use_symtable) {
		print_cnf_symtable();
	}

	if(print_independent_vars) {
		cnf_print_independent_vars();
	}
	
	if(use_symtable)
	  fprintf(foutputfile, "p cnf %ld %d\n", use_symtable, no_outmp_ints);
	else
	  fprintf(foutputfile, "p cnf %ld %d\n", numinp, no_outmp_ints);
	
	for (int x = 0; x < no_outmp_ints; x++) {
		for (int a = false_paths[x].length-1; a >= 0; a--) {
			int lit = false_paths[x].num[a];
			if(lit > 0)
			  fprintf (foutputfile, "%d ", use_symtable?-atoi(getsym_i(lit)->name):-lit); //Need to negate all literals!
			else
			  fprintf (foutputfile, "%d ", use_symtable?atoi(getsym_i(-lit)->name):-lit); //Need to negate all literals!
		}
		fprintf (foutputfile, "0\n");
		delete false_paths[x].num;
	}
//	fprintf(foutputfile, "c end\n");
	delete false_paths;
}
