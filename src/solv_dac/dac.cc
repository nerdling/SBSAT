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
double *dac_var_values;
bool *dac_pb_var_values;

intlist *dac_occurance;
intlist *dac_variables;
VecType *dac_pascal = NULL;

int max_num_bubble_vecs = 2;
VecType *current_bubble;


/************************************/
/* Main                             */
/************************************/

void dac_gen_next_bubble (int curr, int *tail, int size);
void dac_make_multiflip (int function);

int dac_compfunc (const void *x, const void *y) {
	int pp, qq;
	
	pp = *(const int *) x;
	qq = *(const int *) y;
	if (dac_var_values[pp] < dac_var_values[qq])
	  return -1;
	return 1;
}

bool dac_test_for_break() {
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

void printVec (VecType x) {
   for (unsigned int j=0 ; j < sizeof(VecType)*8 ; j++) {
      if (x % 2) {
         d2_printf1("1");
      } else {
         d2_printf1("0");
      }
      x>>=1;
   }
}

void printVec_size (VecType x, unsigned int size) {
   for (unsigned int j=0 ; j < size ; j++) {
      if ((x & (unsigned int)(1<<(size-1))) == (unsigned int)(1<<(size-1))) {
         d2_printf1("1");
      } else {
         d2_printf1("0");
      }
      x<<=1;
   }
}

ITE_INLINE
bool majorizedBy(VecType i, VecType j) {

   VecType tmp = i;
   i = i & ~j;
   j = ~tmp & j;

   //   if(popcount(i) > popcount(j)) return 0;
   
   VecType high_order_bit_i = 1;
   while(high_order_bit_i <= i) high_order_bit_i<<=1;
   high_order_bit_i>>=1;
   
   VecType high_order_bit_j = 1;
   while(high_order_bit_j <= j) high_order_bit_j<<=1;
   high_order_bit_j>>=1;

   if(high_order_bit_i == 0) return 1;
   if(high_order_bit_j == 0) return 0;
   
   while(1) {
//      d2_printf1("\n(");
//      printVec(high_order_bit_i);
//      d2_printf1("     ");
//      printVec(high_order_bit_j);
//      d2_printf1(")");

      if(high_order_bit_i > high_order_bit_j) return 0;
      
      high_order_bit_i>>=1;
      high_order_bit_j>>=1;

      while((high_order_bit_i&i) == 0) { high_order_bit_i>>=1; if(high_order_bit_i == 0) return 1;}
      while((high_order_bit_j&j) == 0) { high_order_bit_j>>=1; if(high_order_bit_j == 0) return 0;}
   }
}

void dac_print_bubble (int tail, int size) {
	d2_printf1("( ");
	for(int x = 0; x < tail; x++) {
		printVec_size(current_bubble[x], size);
		d2_printf1(" ");
	}
	d2_printf3(") %d %d\n", tail, max_num_bubble_vecs);
}

int dacSolve() {
	/* Get values from the command line */

	dac_initprob(); /* initialized the BDD structures */
	fStartTime = get_runtime();
   long long n = 0;
	while (n < 1){//0000000) {
/*      if(dac_test_for_break()) break;
      
      char term_char = (char)term_getchar();
      if (term_char==' ') { //Display status
         d2_printf1("\b");
         d2_printf2("%d", n);
         //Print things
      } else if (term_char=='r') { //Force a random restart
         d2_printf1("\b");
         break;
      } else if (term_char=='q') { //Quit
         d2_printf1("\b");
         nCtrlC = 1;
         break;
      }
*/
      VecType i = rand();
      VecType j = rand();
/*      d2_printf1("(");
      printVec(i);
      d2_printf1(" <=? ");
      printVec(j);
      d2_printf2(") : %d", majorizedBy(i, j));
      d2_printf2(" : %d\n", majorizedBy(j, i));*/
      majorizedBy(i, j);
      n++;
   }
   
	dac_make_multiflip (0);
	
	fEndTime = get_runtime() - fStartTime;
	dac_freemem();
	//if (numsuccesstry!=0) return SOLV_SAT;
	return SOLV_UNKNOWN;
}

void dac_gen_pascal(int size) {
	assert(dac_pascal==NULL);
	assert(size <= 64);
	dac_pascal = (VecType *)ite_calloc(size, sizeof(VecType), 9, "dac_pascal");
	dac_pascal[0] = 1;
	dac_pascal[1] = 3;
	for(int x = 2; x < size; x++)
	  dac_pascal[x] = dac_pascal[x-1]<<1;
}

void dac_initprob(void) {
	dac_numvars = getNuminp();
	dac_numBDDs = nmbrFunctions;
	
	dac_gen_pascal(64); //????
	
	dac_occurance = (intlist *)ite_calloc(dac_numvars+1, sizeof(intlist), 9, "dac_occurance");

	dac_variables = (intlist *)ite_calloc(dac_numBDDs, sizeof(intlist), 9, "dac_variables");

	max_num_bubble_vecs = 2;
	current_bubble = (VecType *)ite_calloc(max_num_bubble_vecs, sizeof(VecType), 9, "current_bubble");
	
	int *tempmem = (int*)ite_calloc(dac_numvars+1, sizeof(int), 9, "tempmem");
	int *tempint = NULL;
	int tempint_max = 0;
	
	for(int x = 0; x <= dac_numvars; x++) {
		tempmem[x] = 0;
	}
	
	for(int x = 0; x < dac_numBDDs; x++) {
		int y = 0;
		unravelBDD (&y, &tempint_max, &tempint, functions[x]);
		if (y != 0) qsort (tempint, y, sizeof (int), compfunc);
		
		dac_variables[x].num = (int *)ite_calloc(y, sizeof(int), 9, "dac_variables[x].num");
		dac_variables[x].length = y;
		for (int i = 0; i < y; i++) {
			dac_variables[x].num[i] = tempint[i];
			tempmem[tempint[i]]++;
		}
	}	

	//dac_variables is done
	
	dac_var_values = (double *)ite_calloc(dac_numvars+1, sizeof(double), 9, "dac_var_values");
	
	dac_pb_var_values = (bool *)ite_calloc(dac_numvars+1, sizeof(bool), 9, "dac_pb_var_values");

	for(int x = 1; x <= dac_numvars; x++) {
		drand();
	}
	
	for (int x = 0; x <= dac_numvars; x++) {
		dac_occurance[x].num = (int *)ite_calloc(tempmem[x], sizeof(int), 9, "dac_occurance[x].num");
		dac_occurance[x].length = 0;
	}
	delete [] tempmem;
	
	for (int x = 0; x < dac_numBDDs; x++) {
		for (int i = 0; i < dac_variables[x].length; i++) {
			dac_occurance[dac_variables[x].num[i]].num[dac_occurance[dac_variables[x].num[i]].length] = x;
			dac_occurance[dac_variables[x].num[i]].length++;
		}
	}
	
	//dac_occurance is done
	
	ite_free((void**)&tempint); tempint_max = 0;
}

double sum_vec(VecType vars_to_sum, int *vec_variables) {
	int x = 0;
	double sum = 0.0;
	while(vars_to_sum!=0) {
		if((vars_to_sum&1) == 1)
		  sum+=dac_var_values[vec_variables[x]];
		x++; vars_to_sum>>=1;
	}
	return sum;
}

int find_smallest_flip(int tail, int *vec_variables) {
	int low = sum_vec(current_bubble[0], vec_variables);
	int smallest_vec = 0;
	for(int x = 1; x < tail; x++) {
		int sum = sum_vec(current_bubble[x], vec_variables);
		if(sum < low) { low = sum; smallest_vec = x; }
	}
	return smallest_vec;
}

void flip_vars(VecType vars_to_flip, int *vec_variables) {
	int x = 0;
	while(vars_to_flip!=0) {
		if((vars_to_flip&1) == 1)
		  dac_pb_var_values[vec_variables[x]] = 1 - dac_pb_var_values[vec_variables[x]];
		x++; vars_to_flip>>=1;
	}
}

//Pb() function
//double *dac_var_values is global
ITE_INLINE
double dac_average(int *dac_vars_to_average, int size) {
   double average = 0.0;
   for(int x = 0; x < size; x++)
     average+=dac_var_values[dac_vars_to_average[x]];
   return average/size;
}

//Traverse the BDD and returns True or False based on the values in dac_pb_var_values
//bool *dac_pb_var_values is global
ITE_INLINE
bool dac_traverseBDD(BDDNode *f) {
   int x = 0;
	while(1) {
		if(f == false_ptr) return 0;
		if(f == true_ptr)	return 1;
		if(dac_pb_var_values[f->variable] == 1)
		  f = f->thenCase;
		else f = f->elseCase;
	}
}

ITE_INLINE
void dac_gen_next_bubble (int curr, int *tail, int size) {
   //Add non-majorized non-majorizing vectors
	if((*tail) == 0) return;
	//d2_printf2("choose %d\n", curr);
	for(int x = 0; x < size; x++) {
		if((*tail) >= max_num_bubble_vecs) {
			current_bubble = (VecType *)ite_recalloc(current_bubble, max_num_bubble_vecs, (*tail)+1, sizeof(VecType), 9, "current_bubble");
			max_num_bubble_vecs = (*tail)+1;
		}
		current_bubble[*tail] = current_bubble[curr] ^ dac_pascal[x];
		if(!majorizedBy(current_bubble[*tail], current_bubble[curr])) {
			for(int y = 0; y < (*tail); y++) {
				if(y == curr) continue;
				if(majorizedBy(current_bubble[y], current_bubble[*tail])) {
					(*tail)--;
					break;
				}
			}
			(*tail)++;
		}		
	}
	//Remove selected multi-flip
	current_bubble[curr] = current_bubble[(*tail)-1];
	(*tail)--;
}

ITE_INLINE
void dac_make_multiflip (int function) {
	if(dac_traverseBDD(functions[function])) return;
	
	current_bubble[0] = 1;
	int tail = 1;
	int size = dac_variables[function].length;
	
	qsort(dac_variables[function].num, dac_variables[function].length, sizeof(int), dac_compfunc);
	
//	dac_print_bubble (tail, size);

	VecType vars_to_flip = 0;
	int num_loops = 0;
	int max_tail = tail;
	for(int x = 0; tail>0; x++) {
		//dac_gen_next_bubble (rand()%tail, &tail, size);
		int smallest_vec_num = find_smallest_flip(tail, dac_variables[function].num);
		vars_to_flip = current_bubble[smallest_vec_num];
//		printVec_size(vars_to_flip, size);
		flip_vars(vars_to_flip, dac_variables[function].num);
		if(dac_traverseBDD(functions[function])) break;
		else flip_vars(vars_to_flip, dac_variables[function].num);
		dac_gen_next_bubble (smallest_vec_num, &tail, size);
//		dac_print_bubble (tail, size);
		if(tail > max_tail) max_tail = tail;
		num_loops++;
	}
	printVec_size(vars_to_flip, size);
	d2_printf3(" [%d, %d]\n", num_loops, max_tail);
	for(int x = dac_variables[function].length-1; x >= 0; x--)
	  d2_printf2("%s ", s_name(dac_variables[function].num[x]));
	d2_printf1("\n");
}

//double *dac_var_values is global
void dac_save_solution() {
	if (result_display_type) {
		/* create another node in solution chain */
		//Should really check against saving multiple solutions
		
		t_solution_info *tmp_solution_info;
		tmp_solution_info = (t_solution_info*)ite_calloc(1, sizeof(t_solution_info), 9, "tmp_solution_info");
		
		if (solution_info_head == NULL) {
			solution_info = tmp_solution_info;
			solution_info_head = solution_info;
		} else {
			solution_info->next = (struct _t_solution_info*)tmp_solution_info;
			solution_info = (t_solution_info*)(solution_info->next);
		}
		tmp_solution_info->nNumElts = dac_numvars+1;
		tmp_solution_info->arrElts = (int *)ite_calloc(dac_numvars+2, sizeof(int), 9, "tmp_solution_info->arrElts");
		
		for (int i = 0; i<=dac_numvars; i++) {
			tmp_solution_info->arrElts[i] = (dac_var_values[i]>=0.5)?BOOL_TRUE:BOOL_FALSE;
		}
	}
}

void dac_freemem(void) {
   ite_free((void **)&dac_pascal);
	ite_free((void **)&dac_var_values);
	ite_free((void **)&current_bubble);
	
	for (int x = 0; x <= dac_numvars; x++)
	  ite_free((void **)&dac_occurance[x].num);
	ite_free((void **)&dac_occurance);
	
	for (int x = 0; x < dac_numBDDs; x++)
	  ite_free((void **)&dac_variables[x].num);
	ite_free((void **)&dac_variables);
}
