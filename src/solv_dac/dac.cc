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

int dac_num_vars;
int dac_num_expanded_vars;
int dac_numBDDs;
double *dac_var_values;
double *dac_pa_var_values;
double *dac_pb_var_values;
double *dac_qsort_var_values;

intlist *dac_var_instances;
intlist *dac_variables;
VecType *dac_pascal = NULL;

int max_num_bubble_vecs;
VecType *current_bubble;

VecType vec_one = 1;

/************************************/
/* Initialization & Allocation      */
/************************************/

int dac_initRAM(void) {
	int ret = NO_ERROR;
   dac_num_vars = getNuminp();
	dac_numBDDs = nmbrFunctions;
	
	int pascal_size = sizeof(VecType)*8;
	dac_pascal = (VecType *)ite_calloc(pascal_size, sizeof(VecType), 9, "dac_pascal");
	dac_pascal[0] = 1;
	dac_pascal[1] = 3;
	for(int x = 2; x < pascal_size; x++)
	  dac_pascal[x] = dac_pascal[x-1]<<vec_one;

	max_num_bubble_vecs = 2;

	int *tempint = NULL;
	int tempint_max = 0;
   int *tempmem = (int*)ite_calloc(dac_num_vars+1, sizeof(int), 9, "tempmem");
   
   dac_variables = (intlist *)ite_calloc(dac_numBDDs, sizeof(intlist), 9, "dac_variables");
   dac_num_expanded_vars = 0;
	int too_many_vars = 0;
	for(int x = 0; x < dac_numBDDs; x++) {
		int y = 0;
		unravelBDD (&y, &tempint_max, &tempint, functions[x]);
		if (y != 0) qsort (tempint, y, sizeof (int), compfunc);
		
		dac_variables[x].num = (int *)ite_calloc(y, sizeof(int), 9, "dac_variables[x].num");
		dac_variables[x].length = y;
		dac_num_expanded_vars += y;
      if(dac_variables[x].length > (sizeof(VecType)*8))
		  too_many_vars = 1;
		for (int i = 0; i < y; i++) {
			dac_variables[x].num[i] = tempint[i];
         tempmem[tempint[i]]++;
      }
	}
   
	dac_var_instances = (intlist *)ite_calloc(dac_num_vars+1, sizeof(intlist), 9, "dac_var_instances");

   for (int x = 0; x <= dac_num_vars; x++) {
		dac_var_instances[x].num = (int *)ite_calloc(tempmem[x], sizeof(int), 9, "dac_var_instances[x].num");
		dac_var_instances[x].length = 0;
	}
	ite_free((void **)&tempmem);

   int instance_num = 1;
   for (int x = 0; x < dac_numBDDs; x++) {
		for (int i = 0; i < dac_variables[x].length; i++) {
			dac_var_instances[dac_variables[x].num[i]].num[dac_var_instances[dac_variables[x].num[i]].length] = instance_num;
			dac_var_instances[dac_variables[x].num[i]].length++;
         instance_num++;
		}
	}

   instance_num = 1;
   for (int x = 0; x < dac_numBDDs; x++) {
		for (int i = 0; i < dac_variables[x].length; i++) {
         dac_variables[x].num[i] = instance_num;
         instance_num++;
      }
   }
   
	max_num_bubble_vecs = 2;
	current_bubble = (VecType *)ite_calloc(max_num_bubble_vecs, sizeof(VecType), 9, "current_bubble");
	
	dac_var_values = (double *)ite_calloc(dac_num_expanded_vars+1, sizeof(double), 9, "dac_var_values");
	dac_pa_var_values = (double *)ite_calloc(dac_num_expanded_vars+1, sizeof(double), 9, "dac_pa_var_values");
   dac_pb_var_values = (double *)ite_calloc(dac_num_expanded_vars+1, sizeof(double), 9, "dac_pb_var_values");

	ite_free((void **)&tempint); tempint_max = 0;

	if(too_many_vars) {
		d1_printf2("DAC solver found a BDD w/ more than %ld variables - this is not currently supported\n", sizeof(VecType)*8);
		return SOLV_UNKNOWN;
	}

	return ret;
}

void dac_freemem(void) {
   ite_free((void **)&dac_pascal);
	ite_free((void **)&dac_var_values);
	ite_free((void **)&dac_pa_var_values);
	ite_free((void **)&dac_pb_var_values);

	ite_free((void **)&current_bubble);

	for (int x = 0; x <= dac_num_vars; x++)
	  ite_free((void **)&dac_var_instances[x].num);
	ite_free((void **)&dac_var_instances);

	for (int x = 0; x < dac_numBDDs; x++)
	  ite_free((void **)&dac_variables[x].num);
	ite_free((void **)&dac_variables);
}

void dac_initStartingPoint() {
   for(int x = 1; x <= dac_num_expanded_vars; x++) {
		dac_var_values[x] = drand();
	}
}

/************************************/
/* Utilties                         */
/************************************/

extern double fStartTime;
extern double fEndTime;

bool dac_test_for_break() {
	if (nTimeLimit && (get_runtime() - fStartTime) > nTimeLimit) {
		d2_printf2("Bailling out because the Time limit of %lds ", nTimeLimit);
		d2_printf2("is smaller than elapsed time %.0fs\n", (get_runtime() - fStartTime));
		return 1;
	}
	if (nCtrlC) {
		d2_printf1("Breaking out of BDD-based Divide and Concur\n");
		return 1;
	}
	return 0;
}

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
		tmp_solution_info->nNumElts = dac_num_vars+1;
		tmp_solution_info->arrElts = (int *)ite_calloc(dac_num_vars+2, sizeof(int), 9, "tmp_solution_info->arrElts");
		
		for (int i = 0; i<=dac_num_vars; i++) {
			//tmp_solution_info->arrElts[i] = (dac_var_values[i]>=0.5)?BOOL_TRUE:BOOL_FALSE;
			tmp_solution_info->arrElts[i] = (dac_pb_var_values[i]==1)?BOOL_TRUE:BOOL_FALSE;
		}
	}
}

/************************************/
/* Bit-Vector Routines              */
/************************************/

void printVec_size (VecType x, VecType size) {
	if(size > (sizeof(VecType)*8)) size = sizeof(VecType)*8;
   for (int j=0 ; j < size ; j++) {
      if ((x & vec_one<<(size-vec_one)) == (vec_one<<(size-vec_one))) {
         d2_printf1("1");
      } else {
         d2_printf1("0");
      }
      x<<=vec_one;
   }
}

void printVec (VecType x) {
	printVec_size(x, sizeof(VecType)*8);
}

/************************************/
/* Majorization Routines            */
/************************************/

ITE_INLINE
bool majorizedBy(VecType i, VecType j) {

   VecType tmp = i;
   i = i & ~j;
   j = ~tmp & j;

   //   if(popcount(i) > popcount(j)) return 0;
   
   VecType high_order_bit_i = 1;
   while(high_order_bit_i <= i) high_order_bit_i<<=vec_one;
   high_order_bit_i>>=vec_one;
   
   VecType high_order_bit_j = 1;
   while(high_order_bit_j <= j) high_order_bit_j<<=vec_one;
   high_order_bit_j>>=vec_one;

   if(high_order_bit_i == 0) return 1;
   if(high_order_bit_j == 0) return 0;
   
   while(1) {
//      d2_printf1("\n(");
//      printVec(high_order_bit_i);
//      d2_printf1("     ");
//      printVec(high_order_bit_j);
//      d2_printf1(")");

      if(high_order_bit_i > high_order_bit_j) return 0;
      
      high_order_bit_i>>=vec_one;
      high_order_bit_j>>=vec_one;

      while((high_order_bit_i&i) == 0) { high_order_bit_i>>=vec_one; if(high_order_bit_i == 0) return 1;}
      while((high_order_bit_j&j) == 0) { high_order_bit_j>>=vec_one; if(high_order_bit_j == 0) return 0;}
   }
}

void test_majorization () {
	for (int x = 0; x < 1000000; x++) {
      VecType i = rand();
      VecType j = rand();
/*      d2_printf1("(");
      printVec(i);
      d2_printf1(" <=? ");
      printVec(j);
      d2_printf2(") : %d", majorizedBy(i, j));
      d2_printf2(" : %d\n", majorizedBy(j, i));*/
      majorizedBy(i, j);
   }
}

/************************************/
/* Bubble Routines                  */
/************************************/

int dac_compfunc (const void *x, const void *y) {
	int pp, qq;
	
	pp = *(const int *) x;
	qq = *(const int *) y;
	if (dac_qsort_var_values[pp] < dac_qsort_var_values[qq])
	  return -1;
	return 1;
}

void dac_print_bubble (int tail, int size) {
	d2_printf1("( ");
	for(int x = 0; x < tail; x++) {
		printVec_size(current_bubble[x], size);
		d2_printf1(" ");
	}
	d2_printf3(") %d %d\n", tail, max_num_bubble_vecs);
}

ITE_INLINE
double sum_vec(VecType vars_to_sum, int *vec_variables, double *Pa_var_values) {
	int x = 0;
	double sum = 0.0;
	while(vars_to_sum!=0) {
		if((vars_to_sum&vec_one) == vec_one) {
			if(Pa_var_values[vec_variables[x]] > 0.5)
			  sum+=-(0.5-Pa_var_values[vec_variables[x]]);
			else sum+=0.5-Pa_var_values[vec_variables[x]];
		}
		x++; vars_to_sum>>=vec_one;
	}
	return sum;
}

ITE_INLINE
int find_smallest_flip(int tail, int *vec_variables, double *Pa_var_values) {
	int low = sum_vec(current_bubble[0], vec_variables, Pa_var_values);
	int smallest_vec = 0;
	for(int x = 1; x < tail; x++) {
		int sum = sum_vec(current_bubble[x], vec_variables, Pa_var_values);
		if(sum < low) { low = sum; smallest_vec = x; }
	}
	return smallest_vec;
}

ITE_INLINE
void flip_vars(VecType vars_to_flip, int *vec_variables, double *Pa_next_var_values) {
	int x = 0;
	while(vars_to_flip!=0) {
		if((vars_to_flip&vec_one) == vec_one)
		   Pa_next_var_values[vec_variables[x]] = 1.0 - Pa_next_var_values[vec_variables[x]];
		x++; vars_to_flip>>=vec_one;
	}
}

//Traverse the BDD and returns True or False based on the values in dac_pb_var_values
//bool *dac_pb_var_values is global
ITE_INLINE
bool dac_traverseBDD(BDDNode *f, double *Pa_next_var_values) {
   int x = 0;
	while(1) {
		if(f == false_ptr) return 0;
		if(f == true_ptr)	return 1;
		if(Pa_next_var_values[f->variable] == 1)
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

//ITE_INLINE
void dac_make_multiflip (int function, double *Pa_var_values, double *Pa_next_var_values) {
	current_bubble[0] = 1;
	int tail = 1;
	int size = dac_variables[function].length;

	dac_qsort_var_values = Pa_var_values;
	qsort(dac_variables[function].num, dac_variables[function].length, sizeof(int), dac_compfunc);
	
//	dac_print_bubble (tail, size);

	VecType vars_to_flip = 0;
	int num_loops = 0;
	int max_tail = tail;
	while(tail>0) {
		//dac_gen_next_bubble (rand()%tail, &tail, size);
		int smallest_vec_num = find_smallest_flip(tail, dac_variables[function].num, Pa_var_values);
		vars_to_flip = current_bubble[smallest_vec_num];
//		printVec_size(vars_to_flip, size);
		flip_vars(vars_to_flip, dac_variables[function].num, Pa_next_var_values);
		if(dac_traverseBDD(functions[function], Pa_next_var_values)) break;
		else flip_vars(vars_to_flip, dac_variables[function].num, Pa_next_var_values);
		dac_gen_next_bubble (smallest_vec_num, &tail, size);
//		dac_print_bubble (tail, size);
		if(tail > max_tail) max_tail = tail;
		num_loops++;
	}
   D_7(
	  d2_printf2("BDD %d: ", function);
     printVec_size(vars_to_flip, size);
	  d2_printf3(" [%d, %d]\n", num_loops, max_tail);
	  for(int x = dac_variables[function].length-1; x >= 0; x--)
	    d2_printf2("%s ", s_name(dac_variables[function].num[x]));
	  d2_printf1("\n");
   );
}

/************************************/
/* Divide and Concur Routines       */
/************************************/

//Pa() function
int dac_Pa(double *Pa_var_values, double *Pa_next_var_values) {
   int isSAT = 1;
   for(int x = 0; x < dac_numBDDs; x++) {
      for(int y = 0; y < dac_variables[x].length; y++) {
         int var = dac_variables[x].num[y];
         Pa_next_var_values[var] = Pa_var_values[var]>0.5?1.0:0.0;
      }

      if(!dac_traverseBDD(functions[x], Pa_next_var_values)) {
         isSAT = 0;
         dac_make_multiflip (x, Pa_var_values, Pa_next_var_values);
      }
      if(dac_test_for_break()) break;
   }
   return isSAT;
}  

//Pb() function
void dac_Pb(double *Pb_var_values, double *Pb_next_var_values) {
   for(int x = 1; x <= dac_num_expanded_vars; x++) {
      double average = 0.0;
      for(int y = 0; y < dac_var_instances[x].length; y++) {
         int var = dac_var_instances[x].num[y];
         average+=Pb_var_values[var];
      }
      average = average/(double)dac_var_instances[x].length;
      for(int y = 0; y < dac_var_instances[x].length; y++) {
         int var = dac_var_instances[x].num[y];
         Pb_next_var_values[var] = average;
      }
   }
}

//vector multiply
ITE_INLINE
void dac_vec_multiply(double B, double *var_values, double *next_var_values) {
   for(int x = 1; x <= dac_num_expanded_vars; x++)
     next_var_values[x] = var_values[x]*B;
}

//vector add
ITE_INLINE
void dac_vec_add(double *var_values, double *next_var_values) {
   for(int x = 1; x <= dac_num_expanded_vars; x++)
     next_var_values[x] = var_values[x]+next_var_values[x];
}

/************************************/
/* Main                             */
/************************************/

int dacSolve() {
	int ret = NO_ERROR;
	if((ret = dac_initRAM()) != NO_ERROR) {
		dac_freemem();		
		return ret;
	}

	dac_initStartingPoint();
   
	fStartTime = get_runtime();
	int numsol = max_solutions;
	int numsuccesstry = 0;

	int dac_iterations = 0;
	
   while(numsol==0 || (numsuccesstry < numsol)) {
		
		char term_char = (char)term_getchar();
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
		
      //do stuff

      int isSAT = 0;
      
		if(isSAT) {
			dac_save_solution();
			numsuccesstry++;
		}
		
		dac_iterations++;
		
		if(dac_test_for_break()) break;
	}
	
	d2_printf2("Iterations = %d\n", dac_iterations);
	
	fEndTime = get_runtime() - fStartTime;
	dac_freemem();
	if (numsuccesstry!=0) return SOLV_SAT;
	return SOLV_UNKNOWN;
}
