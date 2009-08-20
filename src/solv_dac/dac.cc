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

/* walksat version 35hw4 */
/* version 1 by Bram Cohen 7/93 */
/* versions 2 - 35  by Henry Kautz */
/* versions 35h by Holger H Hoos -- all modifications marked "hh" */
/* version 35hw* by Sean Weaver -- modified to solve on BDDs instead of CNF */

#include "sbsat.h"
#include "sbsat_solver.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <sys/types.h>
#include <limits.h>
#include <signal.h>
#include <sys/times.h>
#include <sys/time.h>

extern double fStartTime;
extern double fEndTime;

int dac_numvars;
int dac_numBDDs;
double *dac_atom;

intlist *dac_occurance;
int *dac_length;
intlist *dac_variables;

/************************************/
/* Main                             */
/************************************/

int dac_test_for_break() {
	if (nTimeLimit && (get_runtime() - fStartTime) > nTimeLimit) {
		d2_printf2("Bailling out because the Time limit of %lds ", nTimeLimit);
		d2_printf2("is smaller than elapsed time %.0fs\n", (get_runtime() - fStartTime));
		return 1;
	}
	if (nCtrlC) {
		d2_printf1("Breaking out of BDD WalkSAT\n");
		return 1;
	}
	return 0;
}

void dac_initprob();
void dac_freemem();

int dacSolve() {
	/* Get values from the command line */

	dac_initprob(); /* initialized the BDD structures */
	fStartTime = get_runtime();
	while (1) {
      if(dac_test_for_break()) break;
      
      char term_char = term_getchar();
      if (term_char==' ') { //Display status
         d2_printf1("\b");
         //Print things
      } else if (term_char=='r') { //Force a random restart
         d2_printf1("\b");
         break;
      } else if (term_char=='q') { //Quit
         d2_printf1("\b");
         nCtrlC = 1;
         break;
      }
   }

	fEndTime = get_runtime() - fStartTime;
	dac_freemem();
	//if (numsuccesstry!=0) return SOLV_SAT;
	return SOLV_UNKNOWN;
}

void dac_initprob(void)
{
	dac_numvars = getNuminp();
	dac_numBDDs = nmbrFunctions;
	
	dac_atom = new double[dac_numvars+1];
	
	dac_occurance = new intlist[dac_numvars+1];
	dac_length = new int[dac_numBDDs];
	dac_variables = new intlist[dac_numBDDs];
	int *tempmem = new int[dac_numvars+1];
	int *tempint = NULL;
	int tempint_max = 0;
	
	for(int x = 0; x <= dac_numvars; x++) {
		tempmem[x] = 0;
	}
	
	for(int x = 0; x < dac_numBDDs; x++) {
		int y = 0;
		unravelBDD (&y, &tempint_max, &tempint, functions[x]);
		if (y != 0) qsort (tempint, y, sizeof (int), compfunc);
		
		dac_length[x] = y;
		dac_variables[x].num = new int[y]; //(int *)calloc(y, sizeof(int));
		dac_variables[x].length = y;
		for (int i = 0; i < y; i++) {
			dac_variables[x].num[i] = tempint[i];
			tempmem[tempint[i]]++;
		}
	}	

	//dac_length is done
	//dac_variables is done
	
	for (int x = 0; x <= dac_numvars; x++) {
		dac_occurance[x].num = new int[tempmem[x]];
		dac_occurance[x].length = 0;
	}
	delete [] tempmem;
	
	for (int x = 0; x < dac_numBDDs; x++) {
		for (int i = 0; i < dac_length[x]; i++) {
			dac_occurance[dac_variables[x].num[i]].num[dac_occurance[dac_variables[x].num[i]].length] = x;
			dac_occurance[dac_variables[x].num[i]].length++;
		}
	}
	
	//dac_occurance is done
	
	ite_free((void**)&tempint); tempint_max = 0;
}

void dac_freemem(void)
{
	delete [] dac_atom;
	
	for (int x = 0; x <= dac_numvars; x++)
	  delete [] dac_occurance[x].num;
	delete [] dac_occurance;
	
	for (int x = 0; x < dac_numBDDs; x++)
	  delete [] dac_variables[x].num;
	delete [] dac_variables;
	
	delete [] dac_length;

}

//Traverse the BDD and return if the current path (stored in dac_atom[])
//satisfies or unsatisfies BDD f
inline bool traverseBDD(BDDNode *f) {
	assert(!IS_TRUE_FALSE(f)); //Assume every BDD is more complex than true or false
	while(1) {
		if(dac_atom[f->variable] == 1) {
			f = f->thenCase;
		} else {
			f = f->elseCase;			  
		}
		if(f == false_ptr) return false;
		if(f == true_ptr)	return true;
	}
}

void dac_save_solution(void) {
	if (result_display_type) {
		/* create another node in solution chain */
		//Should really check against saving multiple solutions
		
		t_solution_info *tmp_solution_info;
		tmp_solution_info = (t_solution_info*)calloc(1, sizeof(t_solution_info));
		
		if (solution_info_head == NULL) {
			solution_info = tmp_solution_info;
			solution_info_head = solution_info;
		} else {
			solution_info->next = (struct _t_solution_info*)tmp_solution_info;
			solution_info = (t_solution_info*)(solution_info->next);
		}
		tmp_solution_info->nNumElts = dac_numvars+1;
		tmp_solution_info->arrElts = new int[dac_numvars+2];
		
		for (int i = 0; i<=dac_numvars; i++) {
			tmp_solution_info->arrElts[i] = (dac_atom[i]>0)?BOOL_TRUE:BOOL_FALSE;
		}
	}
}
