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

BDDNode *tmp_equ_var(BDDNode *p);
int functions_add(BDDNode *bdd, int fn_type, int equal_var);

enum {
   BDD2XDD_FLAG_NUMBER,
   FORCE_REUSE_FLAG_NUMBER, /* if bigger than this => won't get reused */
   XQUANTIFY_FLAG_NUMBER, /* v dependent */
   UNRAVELBDD_FLAG_NUMBER, /* starting bdd dependent */
   COUNTX_FLAG_NUMBER, /* X dependent */
   SETVARIABLE_FLAG_NUMBER, /* torf+-num dependent */
   SETVARIABLEXDD_FLAG_NUMBER, /* torf+-num dependent */
   SETVARIABLEALL_FLAG_NUMBER, /* variable list dependent */
   NUMREPLACE_FLAG_NUMBER, /* var/replace dependent */
   NUMREPLACEALL_FLAG_NUMBER, /* var/replace list dependent */
   POSSIBLEINFER_FLAG_NUMBER,
   POSSIBLEBDD_FLAG_NUMBER,
	POSSIBLE_BDD_FLAG_NUMBER,
	SAFEBDD_FLAG_NUMBER,
	CLEANPOSSIBLE_FLAG_NUMBER,
   PRINTSHAREDBDDS_FLAG_NUMBER,
   REDUCEDREPLACE_FLAG_NUMBER,
   MESSAGEPASSING_FLAG_NUMBER,
   MAX_FLAG_NUMBER
};
int last_bdd_flag_number[MAX_FLAG_NUMBER] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
int bdd_flag_number = 2;

inline void
start_bdd_flag_number(int last_bdd_flag)
{
   if (last_bdd_flag > FORCE_REUSE_FLAG_NUMBER ||
         last_bdd_flag_number[last_bdd_flag] != bdd_flag_number)
   {
      bdd_flag_number++;
      if (bdd_flag_number > 1000000000) {
         bdd_gc(); // SEAN!!! Warning, this could be unsafe.
         for(int i=0;i<MAX_FLAG_NUMBER;i++)
            last_bdd_flag_number[i] = 0;
         bdd_flag_number = 3;
      }
      last_bdd_flag_number[last_bdd_flag] = bdd_flag_number;
   }
}

BDDNode *f_mitosis (BDDNode *f, BDDNode **x, int *structureBDD)
{
   if (IS_TRUE_FALSE(f))
      return f;

   int v, num = 0;
   v = f->variable;
	
   //  fprintf(stderr, "v %d|", v);
   for (int iter = 1; iter <= structureBDD[0]; iter++)
	  if (structureBDD[iter] == v) {
		  num = iter;
		  break;
	  }
   if (num == 0) {
      fprintf(stderr, "\nVariable %s not found in translation array...quantifying it away\n", getsym_i(v)->name);
		return ite_or(f_mitosis(f->thenCase, x, structureBDD), f_mitosis (f->elseCase, x, structureBDD));
   }
	return (ite (x[num], f_mitosis (f->thenCase, x, structureBDD), f_mitosis (f->elseCase, x, structureBDD)));
}


BDDNode * mitosis (BDDNode * bdd, int *structureBDD, int *newBDD)
{
   if (IS_TRUE_FALSE(bdd))	//Takes care of True and False
      return bdd;
   int v, num = 0;
   v = bdd->variable;

   //  fprintf(stderr, "v %d|", v);
   for (int x = 1; x <= structureBDD[0]; x++)
      if (structureBDD[x] == v)
      {
         num = x;
         break;
      }
   if (num == 0)
   {
      //fprintf(stderr, "\nVariable %d not found in translation array...quantifying it away\n", v);
      return mitosis (xquantify (bdd, v), structureBDD, newBDD);
   }
   v = newBDD[num];
   BDDNode * r = mitosis (bdd->thenCase, structureBDD, newBDD);
   BDDNode * e = mitosis (bdd->elseCase, structureBDD, newBDD);
   return ite_xvar_y_z (ite_var (v), r, e);
}

BDDNode *shared_structure (BDDNode *bdd1, int bdd1num, int bdd2num) {
	if (IS_TRUE_FALSE(bdd1)) //Takes care of True and False
	  return bdd1;
	int v, num = -1;
	v = bdd1->variable;
	
	//fprintf(stderr, "v %d|", v);
	for (int x = 0; x < length[bdd1num]; x++)
	  if (variables[bdd1num].num[x] == v) {
		  num = x;
		  break;
	  }
	
	if (num == -1) {
		fprintf(stderr, "\nVariable %d not found in translation array...quantifying it away\n", v);
		return shared_structure (xquantify (bdd1, v), bdd1num, bdd2num);
	}
	
	v = variables[bdd2num].num[num];
	BDDNode * r = shared_structure (bdd1->thenCase, bdd1num, bdd2num);
	BDDNode * e = shared_structure (bdd1->elseCase, bdd1num, bdd2num);
	return ite_xvar_y_z (ite_var (v), r, e);
}

inline BDDNode *
MinMaxBDD_within(int set_vars, int min, int max) 
{
   if (set_vars >= min && set_vars <= max) return true_ptr;
   else return false_ptr;
}

BDDNode *MinMaxBDD(int *vars, int min, int max, int total_vars, int set_true) {
   int i,j;
	if(max < min) return false_ptr;
   
	// make sure vars are sorted;
   //d4_printf5("MinMaxBDD(int *vars, int min=%d, int max=%d, int total_vars=%d, int set_true=%d)\n", min, max, total_vars, set_true);
   qsort(vars, total_vars, sizeof(int), revcompfunc);
   for(i=1;i<total_vars;i++) assert(vars[i] < vars[i-1]);
   BDDNode **arr = (BDDNode**)ite_calloc(total_vars+1, sizeof(BDDNode*), 2, "MinMaxBDD");
   BDDNode **prev_arr = (BDDNode**)ite_calloc(total_vars+1, sizeof(BDDNode*), 2, "MinMaxBDD");
   for(i=total_vars;i>=0;i--) 
   {
      arr[total_vars] = MinMaxBDD_within(i, min, max);
      for(j=total_vars-1;j>=i;j--)
      {
         if (prev_arr[j+1] == arr[j+1]) arr[j]=arr[j+1];
         else arr[j] = find_or_add_node(vars[j], prev_arr[j+1], arr[j+1]);
      }
      for(j=total_vars;j>=i;j--) prev_arr[j] = arr[j];
   }
   BDDNode *result = arr[0];
   free(arr);
   free(prev_arr);
   return result;
}

BDDNode *_MinMaxBDD(int *vars, int min, int max, int num_left, int set_true) {
	//fprintf(stderr, "v=%d min=%d max=%d num_left=%d set_true=%d\n", vars[num_left-1], min, max, num_left, set_true);
	if (num_left+set_true < min) return false_ptr;
	if (set_true > max) return false_ptr;
	if ((set_true >= min) && (num_left+set_true <= max)) return true_ptr;
	//if (num_left == 0) // I don't need this one 
	int v = vars[num_left-1]; //vars should be sorted from least to greatest
	
	BDDNode *r = _MinMaxBDD(vars, min, max, num_left-1, set_true+1); //Then Case
	BDDNode *e = _MinMaxBDD(vars, min, max, num_left-1, set_true); //Else Case
	
	if(r == e) return r;
	return find_or_add_node (v, r, e);
}

int countnodes(BDDNode *f) {
   if(IS_TRUE_FALSE(f))
      return 0;
   return (countnodes(f->thenCase) + countnodes(f->elseCase) + 1);
}

int _count_true_paths(BDDNode *f, int *vars, int n_vars, int level) {
	if(f == true_ptr) return 1<<(n_vars-level);
	if(f == false_ptr) return 0;

	int misses = level;
	while(f->variable != vars[level]) level++;

	if(f->flag != 0) return f->flag<<(level-misses);
	
	int num = _count_true_paths(f->thenCase, vars, n_vars, level+1);
	num += _count_true_paths(f->elseCase, vars, n_vars, level+1);
	f->flag = num;
	num = num<<(level-misses);
	return num;	
}

int count_true_paths(BDDNode *f, int *vars, int n_vars) {
   clear_all_bdd_flags();
	qsort(vars, n_vars, sizeof(int), revcompfunc);
	return _count_true_paths(f, vars, n_vars, 0);
}

void marknodes(BDDNode *f) {
	if(IS_TRUE_FALSE(f))
	  return;

	f->flag++;
	//fprintf(stderr, "Flag = %d\n", f->flag);
	if(f->flag > 1)
	  return;

	marknodes(f->thenCase);
	marknodes(f->elseCase);
	return;
}

float mark_trues(BDDNode *f) {
	if (f->density > -1)
		return f->density;
	float t = mark_trues(f->thenCase);
	float e = mark_trues(f->elseCase);
	f->density = (t + e) / 2;
	f->tbr_weight = t / f->density;
	//f->fbr_weight = 1-tbr_weight; Unnecessary for our purposes
	return f->density;
}

void unmark (BDDNode * f) {
   if (IS_TRUE_FALSE(f))
	  return;
	if(f->density == -1) return; //Slower or faster?
   f->density = -1;
   unmark (f->thenCase);
   unmark (f->elseCase);
   return;
}

void Fill_Density() {
	for(int x = 0; x < nmbrFunctions; x++)
	  unmark(functions[x]);
	true_ptr->density = 1;
	false_ptr->density = 0;
	for(int x = 0; x < nmbrFunctions; x++) {
		functions[x]->density = mark_trues(functions[x]);
	}
}

/*
int HammingDistance (BDDNode *f, BDDNode *c) {
	if(f == true_ptr && c == true_ptr) return 0;
	if(c == false_ptr) return -1;
	//Could (should) store hamming distance on nodes.
	//if(c->hamming > -2)
}
*/

extern double *mp_pos_biases;
extern double *mp_neg_biases;

double _calculateNeed(BDDNode *f) {
	if(f == true_ptr) return 0.0;
	if(f == false_ptr) return 1.0;
	if(f->flag == bdd_flag_number) return f->tbr_weight;
	f->flag = bdd_flag_number;
	double r = _calculateNeed(f->thenCase);
	double e = _calculateNeed(f->elseCase);
	return (f->tbr_weight = (r*mp_pos_biases[f->variable]) + (e*mp_neg_biases[f->variable]));
}

double calculateNeed(BDDNode *f, int v, int pos) {
	BDDNode *vBDD = set_variable (f, v, pos);
	start_bdd_flag_number(MESSAGEPASSING_FLAG_NUMBER);
	return _calculateNeed(vBDD);
}

int are_oppos(BDDNode *f, BDDNode *c) {
	if(f->notCase != NULL && f->notCase == c) {
		return 1;
	} else if (f != c->notCase) return 0;
	if(f == true_ptr) {
		if(c == false_ptr) return 1;
		else return 0;
	}
	if(f == false_ptr) {
	  if(c == true_ptr) return 1;
	  else return 0;
	}
	if(f->variable == c->variable) {
		int i = are_oppos(f->thenCase, c->thenCase);
		if(i == 0) return 0;
		return are_oppos(f->elseCase, c->elseCase);		
	}
	return 0;
}

BDDNode * f_apply (BDDNode * f, BDDNode ** x)
{
   if (IS_TRUE_FALSE(f))
      return f;
   return (ite (x[f->variable], f_apply (f->thenCase, x), f_apply (f->elseCase, x)));
}

//Make sure variable x does NOT occur in this bdd
int
verifyBDD (BDDNode * bdd, int x)
{
   if (bdd == true_ptr)
      return 1;
   if (bdd == false_ptr)
      return 1;
   if(verifyBDD (bdd->thenCase, x) == 0) return 0;
   if (abs (x) == bdd->variable)
   {
      fprintf (stderr, "\nPROBLEM\n");
      return 0;
   }
   if(verifyBDD (bdd->elseCase, x) == 0)
      return 0;
   return 1;
}


//Make sure variable x does NOT occur in any bdd
void verifyCircuit (int x) {
   for (int i = 0; i < nmbrFunctions; i++) {
		if(functionType[i] == AUTARKY_FUNC) continue;
      if(!verifyBDD (functions[i], x)) {
         fprintf (stderr, "\nPROBLEM: function %d contains variable %d\n", i, x);
         //exit(1);
      }
   }
}

int splitXors() {
	int total_vars = numinp;
	int *xor_vars = new int[numinp+1]; //This could be made better in the future.
	int *nonxor_vars = new int[numinp+1]; //This could be made better in the future.
	int old_nmbrFunctions = nmbrFunctions; //Must do this because nmbrFunctions
	                                       //grows as we add in extra xor constraints.
	for(int x = 0; x < old_nmbrFunctions; x++) {
		if(length[x] <= functionTypeLimits[PLAINXOR] || functionType[x]!=UNSURE) continue; //???
		BDDNode *xdd = bdd2xdd(functions[x]);		
		countSingleXors(xdd, xor_vars, nonxor_vars);
		d5_printf2("\n%ld: ", x);
      /*
      D_4(
         printBDDfile(xdd, stddbg);
      )
      */
		BDDNode *ands = true_ptr;
		BDDNode *xor_part = false_ptr;
		int y = 0;
	   d5_printf2("%ld: non-linear: ", x);
		for(y=0; nonxor_vars[y]!=0; y++) {
			d5_printf2("%d ", nonxor_vars[y]);
		}
	   d5_printf2("\n%ld linear: ", x);
		for(y=0; xor_vars[y]!=0; y++) {
			d5_printf2("%d ", xor_vars[y]);
			ands = ite_and(ands, ite_var(-xor_vars[y]));
			xor_part = ite_xor(xor_part, ite_var(xor_vars[y]));
		}
		if(y < 3) continue;
      
		//if(IS_TRUE_FALSE(xor_part)) continue; /* unnecessary */
		BDDNode *nonxor_part = gcf(functions[x], ands);
      D_5(
         //d4_printf1("\n");
         //printBDDfile(functions[x], stddbg);
         d5_printf1("\n");
         printBDDfile(nonxor_part, stddbg);
         d5_printf1("\n");
         printBDDfile(xor_part, stddbg);
      )


      if(IS_TRUE_FALSE(nonxor_part)) {
         //functionType[x] = PLAINXOR; /* mark it as plain exor */
         continue;
      }
      d4_printf2("Adding split function %ld\n", x);
      symrec *s_ptr = tputsym(SYM_VAR);

		total_vars++;

#ifndef NDEBUG
      //BDDNode *savedF = functions[x];
#endif

		//xorFunctions[x] = ite_xor(xor_part, ite_var(s_ptr->id));
		//functions[x] = ite_equ(nonxor_part, ite_var(s_ptr->id));
      //length[x] = length[x]-y; /* the linear part is y */
      //equalityVble[x] = s_ptr->id;
      
		int bdd_part_fnid = functions_add(ite_equ(nonxor_part, ite_var(s_ptr->id)), 
            BDD_PART_BDDXOR, s_ptr->id);
		int xor_part_fnid = functions_add(ite_xor(xor_part, ite_var(s_ptr->id)), 
            XOR_PART_BDDXOR, s_ptr->id);
		vars_alloc(s_ptr->id);
		variablelist[s_ptr->id].true_false = -1; //Crazy thing that must be done.
      functionType[x] = BDDXOR_BROKEN;
      functionProps[bdd_part_fnid].bdd_part_bddxor.fn = x;
      functionProps[bdd_part_fnid].bdd_part_bddxor.xor_part = xor_part_fnid;
      functionProps[xor_part_fnid].xor_part_bddxor.fn = x;
      functionProps[xor_part_fnid].xor_part_bddxor.bdd_part = bdd_part_fnid;
      functionProps[x].bddxor_broken.bdd_part = bdd_part_fnid;
      functionProps[x].bddxor_broken.xor_part = xor_part_fnid;
#ifndef NDEBUG
      /*
      BDDNode *testBDD = ite_and(xorFunctions[x], functions[x]);
      testBDD = xquantify(testBDD, s_ptr->id);
      if (testBDD != savedF) {
         assert(0);
      }
      */
#endif
	}
	delete [] xor_vars;
	delete [] nonxor_vars;
	return total_vars;
}

BDDNode *xdd_reduce_t(int v, BDDNode *x) {
  if(x->variable == v)
	 return x->thenCase;
  return false_ptr;
}

BDDNode *xdd_reduce_f(int v, BDDNode *x) {
  if(x->variable == v)
	 return x->elseCase;
  return x;
}

int are_xdd_comp(BDDNode *x, BDDNode *y) {
	while(!IS_TRUE_FALSE(x) && !IS_TRUE_FALSE(y)) {
		if(x->variable != y->variable) return 0;
		if(x->thenCase != y->thenCase) return 0;
		x = x->elseCase;
		y = y->elseCase;		
	}
	if(x == true_ptr) {
		if(y == false_ptr)
		  return 1;
		else return 0;
	} else if(x == false_ptr)
		  if(y == true_ptr)
			 return 1;
	return 0;
}

BDDNode *xddnot(BDDNode *x) {
	if(x == true_ptr) return false_ptr;
	if(x == false_ptr) return true_ptr;
	return find_or_add_node(x->variable, x->thenCase, xddnot(x->elseCase));
}

BDDNode *xddxor(BDDNode *x, BDDNode *y) {
   BDDNode *cached = itetable_find_or_add_node(9, x, y, NULL);
   if (cached) return cached;

	if(x == false_ptr) return y;
	if(y == false_ptr) return x;
	if(x == true_ptr) return xddnot(y);
	if(y == true_ptr) return xddnot(x);
	int v = (x->variable > y->variable) ? x->variable : y->variable;
	BDDNode *r = xddxor(xdd_reduce_t(v, x), xdd_reduce_t(v, y));
	BDDNode *e = xddxor(xdd_reduce_f(v, x), xdd_reduce_f(v, y));
	if(r == false_ptr) return itetable_add_node(9, x, y, e);
	//return find_or_add_node(v, r, e);
	return itetable_add_node(9, x, y, find_or_add_node(v, r, e));
}

BDDNode *xddand(BDDNode *x, BDDNode *y) {
	BDDNode *cached = itetable_find_or_add_node(15, x, y, NULL);
   if (cached) return cached;
	if(x == false_ptr) return false_ptr;
	if(y == false_ptr) return false_ptr;
	if(x == true_ptr) return y;
	if(y == true_ptr) return x;
	if(x == y) return x;
	if(are_xdd_comp(x,y)) return false_ptr;
	
	int v;
	BDDNode *r, *e;
	
	if(x->variable > y->variable) {
		v = x->variable;
		r = xddand(x->thenCase, y);
		e = xddand(x->elseCase, y);
	} else if(x->variable < y->variable) {
		v = y->variable;
		r = xddand(y->thenCase, x);
		e = xddand(y->elseCase, x);
	} else { //x->variable == y->variable
		v = x->variable;
		e = xddand(x->elseCase, y->elseCase);
		if(x->thenCase == true_ptr) {
			r = xddxor(
						  xddxor(y->thenCase, y->elseCase),
						  xddand(y->thenCase, x->elseCase));
		} else if(y->thenCase == true_ptr) {
			r = xddxor(
						  xddxor(x->thenCase, x->elseCase),
						  xddand(x->thenCase, y->elseCase));
		} else {
			r = xddxor(
						  xddand(
									xddxor(x->thenCase, x->elseCase),
									xddxor(y->thenCase, y->elseCase)
									),
						  e
						  );
		}
	}

	if(r == false_ptr) return itetable_add_node(15, x, y, e);
	return itetable_add_node(15, x, y, find_or_add_node(v, r, e));
}

BDDNode *set_variable_xdd_noflag(BDDNode * f, int num, int torf) {
	if (f->variable < num) return f;
	if (f->variable == num) {
		if(torf) return xddxor(f->thenCase, f->elseCase);
		else return f->elseCase;                                                        
	}
	
	BDDNode *r = set_variable_xdd_noflag(f->thenCase, num, torf);                         
	BDDNode *e = set_variable_xdd_noflag(f->elseCase, num, torf);                         
	if (r == false_ptr) return e;
	return find_or_add_node(f->variable, r, e);                                       
}

BDDNode *_set_variable_xdd (BDDNode *f, int num, int torf);

BDDNode * set_variable_xdd (BDDNode *f, int num, int torf) {
   start_bdd_flag_number(SETVARIABLEXDD_FLAG_NUMBER);
   return _set_variable_xdd(f, num, torf);
}

BDDNode * _set_variable_xdd (BDDNode * f, int num, int torf) {
	if (f->variable < num) return f;
	
   if (f->flag == bdd_flag_number) return f->tmp_bdd;
	//if (f->notCase->flag == bdd_flag_number) return f->notCase->tmp_bdd->notCase; //XDDs don't currently have a notCase
   f->flag = bdd_flag_number;
   
	if (f->variable == num) {
      if (torf) return (f->tmp_bdd = xddxor(f->thenCase, f->elseCase));
      else return (f->tmp_bdd = f->elseCase);
   }
   BDDNode * r = _set_variable_xdd (f->thenCase, num, torf);
   BDDNode * e = _set_variable_xdd (f->elseCase, num, torf);
   if (r == false_ptr) return (f->tmp_bdd = e);
   return (f->tmp_bdd = find_or_add_node (f->variable, r, e));
}
  
BDDNode *_bdd2xdd(BDDNode *x);

BDDNode *bdd2xdd(BDDNode *x) {
   start_bdd_flag_number(BDD2XDD_FLAG_NUMBER);
   return _bdd2xdd(x);
}

BDDNode *_bdd2xdd(BDDNode *x) {
	if(x == false_ptr) return false_ptr;
	if(x == true_ptr) return true_ptr;

   if (x->flag == bdd_flag_number) return x->tmp_bdd;
   x->flag = bdd_flag_number;

	int v = x->variable;
	BDDNode *r = _bdd2xdd(xdd_reduce_t(v, x));
	BDDNode *e = _bdd2xdd(xdd_reduce_f(v, x));
	r = xddxor(r, e);
	if(r == false_ptr) return (x->tmp_bdd = e);
	return (x->tmp_bdd = find_or_add_node(v, r, e));
}

inline BDDNode *constant_and(BDDNode *x, BDDNode *y) {
   if (x == false_ptr) return false_ptr;
   if (y == false_ptr) return false_ptr;

	if (x == true_ptr) return true_ptr;
   if (y == true_ptr) return true_ptr;

	BDDNode * r;
	BDDNode * e;

	if (x->variable > y->variable) {
      BDDNode *cached = itetable_find_or_add_node(13, x, y, NULL);
      if (cached) return cached;
		r = constant_and(x->thenCase, y);
		if(r!=false_ptr) return itetable_add_node(13, x, y, true_ptr);//true_ptr;
		e = constant_and(x->elseCase, y);
	} else if (x->variable == y->variable) {
		if (x == y) return true_ptr;
		else if (x->notCase == y) return false_ptr;
		else {
			BDDNode *cached = itetable_find_or_add_node(13, x, y, NULL);
			if (cached) return cached;
			r = constant_and(x->thenCase, y->thenCase);
			if(r!=false_ptr) return itetable_add_node(13, x, y, true_ptr);//true_ptr;
			e = constant_and(x->elseCase, y->elseCase);
		}
	} else {
      BDDNode *cached = itetable_find_or_add_node(13, x, y, NULL);
      if (cached) return cached;
		r = constant_and(x, y->thenCase);
		if(r!=false_ptr) return itetable_add_node(13, x, y, true_ptr);//true_ptr;
		e = constant_and(x, y->elseCase);
	}

	if(e!=false_ptr) return itetable_add_node(13, x, y, true_ptr);//true_ptr;
	return itetable_add_node(13, x, y, false_ptr);
	return false_ptr;
}

inline BDDNode *_and_dot(BDDNode *x, BDDNode *y);

BDDNode *and_dot(BDDNode *x, BDDNode *y) {
   if (y == true_ptr) return x;
   if (y == false_ptr) return false_ptr;
   return _and_dot(x, y);
}

inline
BDDNode *_and_dot(BDDNode *x, BDDNode *y)
{
   if (x == true_ptr) return y;
   if (x == false_ptr) return false_ptr;

   int v;
   BDDNode * r;
   BDDNode * e;

   if (x->variable > y->variable) {
      BDDNode *cached = itetable_find_or_add_node(12, x, y, NULL);
      if (cached) return cached;
      v = x->variable;
      r = _and_dot(x->thenCase, y);
      if(r->inferences == NULL && r!=false_ptr) {
			r = true_ptr; //maybe unnecessary?
			e = constant_and(x->elseCase, y);
		} else {
			e = _and_dot(x->elseCase, y);
			if(e->inferences == NULL && e!=false_ptr) {
				if(r != false_ptr)
				  r = true_ptr; //maybe unnecessary?
			}
		}
		if (r == x->thenCase && e == x->elseCase) return x;
   } else if (x->variable == y->variable) {
      if (x == y) return x;
      else if (x->notCase == y) return false_ptr;
      else {
         BDDNode *cached = itetable_find_or_add_node(12, x, y, NULL);
         if (cached) return cached;
         v = x->variable;
         if (y->thenCase == true_ptr) r=x->thenCase;
         else if (y->thenCase == false_ptr) r=false_ptr;
         else r = _and_dot(x->thenCase, y->thenCase);
			if(r->inferences == NULL && r!=false_ptr) {
				r = true_ptr; //maybe unnecessary?
				e = constant_and(x->elseCase, y);
			} else {
				if (y->elseCase == true_ptr) e=x->elseCase;
				else if (y->elseCase == false_ptr) e=false_ptr;
				else e = _and_dot(x->elseCase, y->elseCase);
				if(e->inferences == NULL && e!=false_ptr) {
					if(r != false_ptr)
					  r = true_ptr; //maybe unnecessary?
				}
			}
			// this happens but not often enough to bring a speedup
			if (r == x->thenCase && e == x->elseCase) return x;
			if (r == y->thenCase && e == y->elseCase) return y;
      }
   } else {
      BDDNode *cached = itetable_find_or_add_node(12, y, x, NULL);
      if (cached) return cached;
      v = y->variable;
      r = _and_dot(y->thenCase, x);
      if(r->inferences == NULL && r!=false_ptr) {
			r = true_ptr; //maybe unnecessary?
			e = constant_and(x->elseCase, y);
		} else {
			e = _and_dot(y->elseCase, x);
			if(e->inferences == NULL && e!=false_ptr) {
				if(r != false_ptr)
				  r = true_ptr; //maybe unnecessary?
			}
		}
		// this happens but not often enough to bring a speedup
		if (r == y->thenCase && e == y->elseCase) return y;
   } 

   if (r == e) return (r);
   return itetable_add_node(12, x, y, find_or_add_node(v, r, e));
   return find_or_add_node(v, r, e);
}

// c is not allowed to be false.
BDDNode * gcf (BDDNode * f, BDDNode * c)
{
   if (f == c)
      return true_ptr;
   if ((c == true_ptr) || (f == true_ptr) || (f == false_ptr))
      return f;

   // We know that f & c are both BDD's with top variables.
   int v = c->variable;
   if ((f->variable) > v)
      v = f->variable;

   //v is the top variable of f & c.
   if (reduce_f (v, c) == false_ptr)
      return gcf (reduce_t (v, f), reduce_t (v, c));
   if (reduce_t (v, c) == false_ptr)
      return gcf (reduce_f (v, f), reduce_f (v, c));
   BDDNode * r = gcf (reduce_t (v, f), reduce_t (v, c));
   BDDNode * e = gcf (reduce_f (v, f), reduce_f (v, c));
   if (r == e)
      return r;
   return find_or_add_node (v, r, e);
}

BDDNode * restrictx (int bddNmbr1, int bddNmbr2)
{
   BDDNode *quantifiedBDD2 = functions[bddNmbr2];
   int bdd1pos = 0;
   int bdd2pos = 0;
   int bdd1Var = 0, bdd2Var = 0;
   while(bdd1pos < length[bddNmbr1] && bdd2pos < length[bddNmbr2]) {
      bdd1Var = variables[bddNmbr1].num[bdd1pos];
      bdd2Var = variables[bddNmbr2].num[bdd2pos];
      if (bdd1Var < bdd2Var)
         bdd1pos++;
      else if (bdd1Var == bdd2Var) {
         bdd1pos++;
         bdd2pos++;
      } else {
         quantifiedBDD2 = xquantify(quantifiedBDD2, bdd2Var);
         bdd2pos++;
      }
   }
   for (; bdd2pos < length[bddNmbr2]; bdd2pos++)
      quantifiedBDD2 = xquantify(quantifiedBDD2, variables[bddNmbr2].num[bdd2pos]);
   if(quantifiedBDD2 == functions[bddNmbr2]) {
      functions[bddNmbr2] = true_ptr;
      length[bddNmbr2] = 0;
      if(variables[bddNmbr2].num!=NULL)
         delete variables[bddNmbr2].num;
      variables[bddNmbr2].num = NULL;
      //  fprintf(stderr, "Removing %d ", bddNmbr2);
      return ite_and(functions[bddNmbr1], quantifiedBDD2);
   }
   return restrict (functions[bddNmbr1], quantifiedBDD2);
}

//Adds false paths from one BDD to another
BDDNode *restrict (BDDNode * f, BDDNode * c)
{
	//if (f == c)
	//  return true_ptr;
   if ((c == true_ptr) || (f == false_ptr))
      return f;
   if (c == ite_not (f))
	  if (f == true_ptr)
      return c;

   // We know that f & c are both BDD's with top variables.
   if (f->variable < c->variable)
      return restrict (f, ite_or (c->thenCase, c->elseCase));	//xquantify(c, c->variable));
   int v = f->variable;


   //  int v = c->variable;
   //if (f->variable < c->variable)
   //  v = f->variable;
   // 
   //v is the top variable of f & c.
   if (reduce_f (v, c) == false_ptr) {
      if (reduce_t(v, f) == true_ptr) return c;
      return restrict (reduce_t (v, f), reduce_t (v, c));
   }
   if (reduce_t (v, c) == false_ptr) {
      if (reduce_f(v, f) == true_ptr) return c;
      return restrict (reduce_f (v, f), reduce_f (v, c));
   }
   BDDNode * r = restrict (reduce_t (v, f), reduce_t (v, c));
   BDDNode * e = restrict (reduce_f (v, f), reduce_f (v, c));
   if (r == e)
      return r;
   return find_or_add_node (v, r, e);
}

BDDNode * remove_fpsx (int bddNmbr1, int bddNmbr2)
{
   BDDNode *quantifiedBDD2 = functions[bddNmbr2];
   int bdd1pos = 0;
   int bdd2pos = 0;
   int bdd1Var = 0, bdd2Var = 0;
   while(bdd1pos < length[bddNmbr1] && bdd2pos < length[bddNmbr2]) {
      bdd1Var = variables[bddNmbr1].num[bdd1pos];
      bdd2Var = variables[bddNmbr2].num[bdd2pos];
      if (bdd1Var < bdd2Var)
         bdd1pos++;
      else if (bdd1Var == bdd2Var) {
         bdd1pos++;
         bdd2pos++;
      } else {
         quantifiedBDD2 = xquantify(quantifiedBDD2, bdd2Var);
         bdd2pos++;
      }
   }
   for (; bdd2pos < length[bddNmbr2]; bdd2pos++)
      quantifiedBDD2 = xquantify(quantifiedBDD2, variables[bddNmbr2].num[bdd2pos]);
   if(quantifiedBDD2 == functions[bddNmbr2]) {
      functions[bddNmbr2] = true_ptr;
      length[bddNmbr2] = 0;
      if(variables[bddNmbr2].num!=NULL)
         delete variables[bddNmbr2].num;
      variables[bddNmbr2].num = NULL;
      //  fprintf(stderr, "Removing %d ", bddNmbr2);
      return ite_and(functions[bddNmbr1], quantifiedBDD2);
   }
   return remove_fps (functions[bddNmbr1], quantifiedBDD2);
}

//Removes redundant false paths in BDD c from BDD f.
BDDNode *remove_fps(BDDNode * f, BDDNode * c)
{
   if ((f == c)||(f == true_ptr)||(c == false_ptr))
      return true_ptr;
   if ((c == true_ptr) || (f == false_ptr))
      return f;
   //I know that this may lead to the notion that not all redundant
   //false paths are removed. But, when this process is done iteratively
   //over the entire set of BDDs it will guarantee that all redundant
   //false paths are removed and it will tend to cluster the false
   //paths, placing them higher in the BDD structure, instead of splitting
   //a false path into multiple BDDs.

   // We know that f & c are both BDD's with top variables.
   if (f->variable < c->variable)
      return remove_fps(f, ite_or (c->thenCase, c->elseCase));	//xquantify(c, c->variable));
   int v = f->variable;

   //  int v = c->variable;
   //if (f->variable < c->variable)
   //  v = f->variable;
   //
   //v is the top variable of f & c.
   if (reduce_f (v, c) == false_ptr) {
      return remove_fps(reduce_t (v, f), reduce_t (v, c));
   }
   if (reduce_t (v, c) == false_ptr) {
      return remove_fps(reduce_f (v, f), reduce_f (v, c));
   }
   BDDNode * r = remove_fps(reduce_t (v, f), reduce_t (v, c));
   BDDNode * e = remove_fps(reduce_f (v, f), reduce_f (v, c));
   if (r == e)
      return r;
   return find_or_add_node (v, r, e);
}

BDDNode * pruning (BDDNode * f, BDDNode * c)
{
   if (f == c)
      return true_ptr;
   if ((c == true_ptr) || (f == true_ptr) || (f == false_ptr))
      return f;
	if (c == ite_not (f))
	  return false_ptr;
	
	// We know that f & c are both BDD's with top variables.
   if (f->variable < c->variable)
      return pruning (f, ite_or_te(c));	//xquantify(c, c->variable));
      //return pruning (f, ite_or (c->thenCase, c->elseCase));	//xquantify(c, c->variable));
   int v = f->variable;

   //v is the top variable of f & c.
   if (reduce_f (v, c) == false_ptr)
      return pruning (reduce_t (v, f), reduce_t (v, c));
   if (reduce_t (v, c) == false_ptr)
      return pruning (reduce_f (v, f), reduce_f (v, c));
   BDDNode * r = pruning (reduce_t (v, f), reduce_t (v, c));
   BDDNode * e = pruning (reduce_f (v, f), reduce_f (v, c));
   if (r == e)
      return r;
   return find_or_add_node (v, r, e);
}

BDDNode *strengthen_fun(BDDNode *bddNmbr1, BDDNode *bddNmbr2)
{	
	int y = 0;
   int tempint_max = 0;
   int *tempint=NULL;
        
	unravelBDD(&y, &tempint_max, &tempint, bddNmbr1);
	if (y != 0) qsort (tempint, y, sizeof (int), compfunc);
	int length1 = y;
	int *vars1;
	if(length1 > 0) {
		vars1 = new int[length1+1];
		for(int i = 0; i < y; i++)
		  vars1[i] = tempint[i];
	}
	
	y = 0;
	unravelBDD(&y, &tempint_max, &tempint, bddNmbr2);
	if (y != 0) qsort (tempint, y, sizeof (int), compfunc);
	int length2 = y;
	int *vars2;
	if(length2 > 0){
		vars2 = new int[length2+1];
		for(int i = 0; i < y; i++)
		  vars2[i] = tempint[i];
	}
        
   ite_free((void**)&tempint); tempint_max = 0;

	BDDNode *quantifiedBDD2 = bddNmbr2;
	int bdd1pos = 0;
	int bdd2pos = 0;
	int bdd1Var = 0, bdd2Var = 0;
	while(bdd1pos < length1 && bdd2pos < length2) {
		bdd1Var = vars1[bdd1pos];
		bdd2Var = vars2[bdd2pos];
		if (bdd1Var < bdd2Var)
		  bdd1pos++;
		else if (bdd1Var == bdd2Var) {
			bdd1pos++;
			bdd2pos++;
		} else {
			quantifiedBDD2 = xquantify(quantifiedBDD2, bdd2Var);
			bdd2pos++;
		}
	}
	for (; bdd2pos < length2; bdd2pos++)
	  quantifiedBDD2 = xquantify(quantifiedBDD2, vars2[bdd2pos]);
	
	if(length1 > 0) delete [] vars1;
	if(length2 > 0) delete [] vars2;
	return ite_and(bddNmbr1, quantifiedBDD2);
}

BDDNode * pruning_p2 (BDDNode * f, BDDNode * c)
{
   if (f == c)
      return true_ptr;
	if (c == ite_not (f))
	  return false_ptr;
   if ((c == true_ptr) || (f == true_ptr) || (f == false_ptr))
      return f;

   // We know that f & c are both BDD's with top variables.
   if (f->variable < c->variable)
      return pruning_p2 (f, ite_or_te(c));//ite_or (c->thenCase, c->elseCase));	//xquantify(c, c->variable));
   int v = f->variable;

   //v is the top variable of f & c.
   if (reduce_f (v, c) == false_ptr)
      return pruning_p2 (reduce_t (v, f), reduce_t (v, c));
   if (reduce_t (v, c) == false_ptr)
      return pruning_p2 (reduce_f (v, f), reduce_f (v, c));
   BDDNode * r = pruning_p2 (reduce_t (v, f), reduce_t (v, c));
   BDDNode * e = pruning_p2 (reduce_f (v, f), reduce_f (v, c));
   if (r == e)
      return r;
   return find_or_add_node (v, r, e);
}

BDDNode *Build_BDD_From_Inferences (BDDNode *c) 
{
	BDDNode *tempBDD = true_ptr;
	if(c->inferences != NULL) {
		for(infer *iterator = c->inferences; iterator != NULL; iterator = iterator->next) {
			if (iterator->nums[1] == 0) {
				tempBDD = ite_and(tempBDD, ite_var(iterator->nums[0]));
			} else {
				tempBDD = ite_and(tempBDD, ite_equ(ite_var(iterator->nums[0]), ite_var(iterator->nums[1])));
			}
		//fprintf(stderr, "%d|%d, %d|", x, iterator->nums[0], iterator->nums[1]);
		}
	}
	return tempBDD;
}

BDDNode * steal (BDDNode * f, BDDNode * c)
{
	if (c == ite_not (f))
	  return false_ptr;
	if ((c == true_ptr) || (f == false_ptr))
      return f;
   if (f == c)
	  return true_ptr;
	if (f == true_ptr) {
		BDDNode *Inf_bdd = Build_BDD_From_Inferences(c);
		//Only copy over inferences with variables that were originally in f.
		//BDDNode *Inf_BDD = Build_BDD_From_Inferences_Restricted(c, f_variables); 
      
		//For last two, go back one recursive call, then do these operations.
      //BDDNode *Inf_bdd = strengthen(f, c);
		//BDDNode *Inf_bdd = ite_and(f, c);		
		return Inf_bdd;
	}
   // We know that f & c are both BDD's with top variables.
   if (f->variable < c->variable)
      return steal (f, ite_or (c->thenCase, c->elseCase));	//xquantify(c, c->variable));
   int v = f->variable;

   //v is the top variable of f & c.
   if (reduce_f (v, c) == false_ptr && reduce_f(v, f) == false_ptr) {
      //BDDNode *Inf_bdd = ite_and(Build_BDD_From_Inferences(f->thenCase), Build_BDD_From_Inferences(f->elseCase));
      //BDDNode *steal_bdd = steal (reduce_t (v, f), reduce_t (v, c));
		//return ite(v, steal_bdd, Inf_bdd);
      //return steal (reduce_t(v, f), reduce_t(v, c));
	}
//   if (reduce_t (v, c) == false_ptr && reduce_t(v, f) == false_ptr)
      //return steal (reduce_f (v, f), reduce_f (v, c));
   BDDNode * r = steal (reduce_t (v, f), reduce_t (v, c));
   BDDNode * e = steal (reduce_f (v, f), reduce_f (v, c));
   if (r == e)
      return r;
   return find_or_add_node (v, r, e);
}

BDDNode * pruning_p1(BDDNode * f, BDDNode * c)
{
   if (f == c)
      return true_ptr;
   if ((f->variable < c->variable) || (c == true_ptr))
      return f;
   if (f->variable == c->variable)
   {
      if (c->thenCase == false_ptr)
         return f->elseCase;
      if (c->elseCase == false_ptr)
         return f->thenCase;
      BDDNode * r = pruning_p1 (f->thenCase, c->thenCase);
      BDDNode * e = pruning_p1 (f->elseCase, c->elseCase);
      if (r == e)
         return r;
      return find_or_add_node (c->variable, r, e);
   }

   //  if(f->variable < c->variable)
   BDDNode * r = pruning_p1 (f->thenCase, c);
   BDDNode * e = pruning_p1 (f->elseCase, c);
   if (r == e)
      return r;
   return find_or_add_node (f->variable, r, e);
}

int _countX(BDDNode *bdd, BDDNode *X);

int countX(BDDNode *bdd, BDDNode *X) {
   start_bdd_flag_number(COUNTX_FLAG_NUMBER);
   return _countX(bdd, X);
}

int _countX(BDDNode *bdd, BDDNode *X) {
   if (bdd->flag == bdd_flag_number) return bdd->tmp_int;
   bdd->flag = bdd_flag_number;

	if(bdd == X) return (bdd->tmp_int = 1);
	if(IS_TRUE_FALSE(bdd)) return (bdd->tmp_int = 0);
   if(bdd->variable <= X->variable) return (bdd->tmp_int = 0);
	return (bdd->tmp_int = _countX(bdd->thenCase, X) + _countX(bdd->elseCase, X));	
}

int countFalses (BDDNode * bdd) {
	return countX(bdd, false_ptr);
}

int countTrues (BDDNode * bdd) {
	return countX(bdd, true_ptr);
}

int isOR(BDDNode *bdd) {
	if(IS_TRUE_FALSE(bdd)) return 0;
	int size = 0;
	while(bdd != false_ptr) {
		if(bdd->thenCase == true_ptr)
		  bdd = bdd->elseCase;
		else if(bdd->elseCase == true_ptr)
		  bdd = bdd->thenCase;
		else return 0;
		size++;
	}
	return size>1;
}

int isAND(BDDNode *bdd) {
	if(IS_TRUE_FALSE(bdd)) return 0;
	int size = 0;
	while(bdd != true_ptr) {
		if(bdd->thenCase == false_ptr)
		  bdd = bdd->elseCase;
		else if(bdd->elseCase == false_ptr)
		  bdd = bdd->thenCase;
		else return 0;
		size++;		
	}
	return size>1;
}

//returns 0 for NO
//returns positive equality variable for YES
//returns negative equality variable for OR_EQU
int isAND_EQU(BDDNode *bdd, int *bdd_vars, int bdd_len) {
	if(IS_TRUE_FALSE(bdd)) return 0;
	for(int x = 0; x < bdd_len; x++) {
		BDDNode *t_equ_bdd = set_variable(bdd, bdd_vars[x], 1);
		BDDNode *f_equ_bdd = set_variable(bdd, bdd_vars[x], 0);
		if(isAND(t_equ_bdd) && isOR(f_equ_bdd) && (bdd == ite_equ(ite_var(bdd_vars[x]), t_equ_bdd)))
		  return bdd_vars[x];
		if(isOR(t_equ_bdd) && isAND(f_equ_bdd) && (bdd == ite_equ(ite_var(bdd_vars[x]), t_equ_bdd)))
		  return -bdd_vars[x];
	}
	return 0;
}

int isXOR(BDDNode *bdd) {
	if(IS_TRUE_FALSE(bdd)) return 0;
	int size = 0;
	for(;!IS_TRUE_FALSE(bdd);bdd = bdd->thenCase) {
		if(bdd->thenCase != bdd->elseCase->notCase) return 0;
		size++;
	}
	return size>1;	
}

int isITE(BDDNode *bdd) {
	//SEAN!!! Write it.
	return 1;	
}

int isMIN_MAX(BDDNode *bdd, int *bdd_vars, int bdd_len) {
	//fprintf(stderr, "HERE\n");

	if(IS_TRUE_FALSE(bdd)) return 0;
	int max = -1;
	BDDNode *tmp_bdd;
	for(tmp_bdd = bdd; !IS_TRUE_FALSE(tmp_bdd); tmp_bdd = tmp_bdd->thenCase) 
	  max++;
	if(tmp_bdd != false_ptr) return 0;

	int min = -1;
	for(tmp_bdd = bdd; !IS_TRUE_FALSE(tmp_bdd); tmp_bdd = tmp_bdd->elseCase)
	  min++;
	if(tmp_bdd == true_ptr) min = 0;
	else if(tmp_bdd != false_ptr) return 0;
   else min = bdd_len-min;
	
	if(min > max) return 0;
	
	//Now have the min and max
	
	//A slow way to do it!
	//fprintf(stderr, "min=%d, max=%d, len=%d\n", min, max, bdd_len);

	int set_true = 0;
	if(bdd == MinMaxBDD(bdd_vars, min, max, bdd_len, set_true))
	  return 1;	
	return 0;
}

int isNEG_MIN_MAX(BDDNode *bdd, int *bdd_vars, int bdd_len) {
	//fprintf(stderr, "HERE\n");

	if(IS_TRUE_FALSE(bdd)) return 0;
	int max = -1;
	BDDNode *tmp_bdd;
	for(tmp_bdd = bdd; !IS_TRUE_FALSE(tmp_bdd); tmp_bdd = tmp_bdd->thenCase) 
	  max++;
	if(tmp_bdd != true_ptr) return 0;

	int min = -1;
	for(tmp_bdd = bdd; !IS_TRUE_FALSE(tmp_bdd); tmp_bdd = tmp_bdd->elseCase)
	  min++;
	if(tmp_bdd == false_ptr) min = 0;
	else if(tmp_bdd != true_ptr) return 0;
   else min = bdd_len-min;
	
	if(min > max) return 0;
	
	//Now have the min and max
	
	//A slow way to do it!
	//fprintf(stderr, "min=%d, max=%d, len=%d\n", min, max, bdd_len);

	int set_true = 0;
	if(bdd == MinMaxBDD(bdd_vars, min, max, bdd_len, set_true)->notCase)
	  return 1;	
	return 0;
}

//To set the fnType the function must be UNSURE to start with
int findandset_fnType(int x) {
	if(functionType[x] != UNSURE) return 1;
	
	if (length[x] >= functionTypeLimits[PLAINOR] &&
		 isOR(functions[x])) {
		functionType[x] = PLAINOR;
		return 1;
	}
	
	if (length[x] >= functionTypeLimits[PLAINXOR] &&
		 isXOR(functions[x])) {
		functionType[x] = PLAINXOR;
		return 1;
	}
	
	if (length[x] >= functionTypeLimits[AND]) {
		int equ_var = isAND_EQU(functions[x], variables[x].num, length[x]);
		if(equ_var != 0) {
			if(equ_var > 0) {
				functionType[x] = AND;
				equalityVble[x] = equ_var;
				//independantVars[equ_var] = reverse_independant_dependant;
			} else { //equ_var < 0 for sure
				functionType[x] = OR;
				equalityVble[x] = -equ_var;
				//independantVars[-equ_var] = reverse_independant_dependant;
			}
			return 1;
		}
	}

	if (length[x] >= functionTypeLimits[MINMAX] &&
		 isMIN_MAX(functions[x], variables[x].num, length[x])) {
		functionType[x] = MINMAX;
		return 1;
	}

	if (length[x] >= functionTypeLimits[NEG_MINMAX] &&
		 isNEG_MIN_MAX(functions[x], variables[x].num, length[x])) {
		functionType[x] = NEG_MINMAX;
		return 1;
	}

	return 0;
}

//To set the fnType the function must be UNSURE to start with
int findandret_fnType(BDDNode *bdd, int *bdd_length, int *&bdd_vars) {
	if(IS_TRUE_FALSE(bdd)) return UNSURE;
	
   int *bdd_tempint = NULL;
	int bdd_tempint_max = 0;
	int y = 0;
	unravelBDD(&y, &bdd_tempint_max, &bdd_tempint, bdd);
	int *tempint = bdd_tempint;
	if (y != 0) qsort (tempint, y, sizeof (int), compfunc);
	
	(*bdd_length) = y;
	bdd_vars = new int[y + 1]; //(int *)calloc(y+1, sizeof(int));
	for (int i = 0; i < y; i++)
	  bdd_vars[i] = tempint[i];
	ite_free((void**)&bdd_tempint);
	
	if ((*bdd_length) >= functionTypeLimits[PLAINOR] &&
		 isOR(bdd)) {
		return PLAINOR;
	}

	if ((*bdd_length) >= functionTypeLimits[PLAINAND] &&
		 isAND(bdd)) {
		return PLAINAND;
	}

	if ((*bdd_length) >= functionTypeLimits[PLAINXOR] &&
		 isXOR(bdd)) {
		return PLAINXOR;
	}
	
	if ((*bdd_length) >= functionTypeLimits[AND]) {
		int equ_var = isAND_EQU(bdd, bdd_vars, (*bdd_length));
		if(equ_var != 0) {
				if(equ_var > 0) {
				return AND;
			} else { //equ_var < 0 for sure
				return OR;
			}
		}
	}

	if ((*bdd_length) >= functionTypeLimits[MINMAX] &&
		 isMIN_MAX(bdd, bdd_vars, (*bdd_length))) {
		return MINMAX;
	}

	if ((*bdd_length) >= functionTypeLimits[MINMAX] &&
		 isNEG_MIN_MAX(bdd, bdd_vars, (*bdd_length))) {
		return NEG_MINMAX;
	}

	return UNSURE;
}

int
nmbrVarsInCommon(int bddNmbr1, int bddNmbr2, int STOPAT)
{
   static int bdd1pos;
   static int bdd2pos;
   static int varsSoFar;
   static int bdd1max;
   static int bdd2max;

   bdd1pos = 0;
   bdd2pos = 0;
   varsSoFar = 0;
	if(length[bddNmbr1] < STOPAT) return 0;
	if(length[bddNmbr2] < STOPAT) return 0;
   bdd1max = length[bddNmbr1] - STOPAT + 1;
   bdd2max = length[bddNmbr2] - STOPAT + 1;

   //fprintf(stdout, "(%d)%d - %d\n", bddNmbr1, length[bddNmbr1], length[bddNmbr2]);

   while (1) {
      while (variables[bddNmbr1].num[bdd1pos] < variables[bddNmbr2].num[bdd2pos]) {
         if (++bdd1pos == bdd1max) return 0; 
      }
		
      while (variables[bddNmbr1].num[bdd1pos] > variables[bddNmbr2].num[bdd2pos]) {
         if (++bdd2pos == bdd2max) return 0; 
      }
		
      if (variables[bddNmbr1].num[bdd1pos] == variables[bddNmbr2].num[bdd2pos]) {
         if (++varsSoFar == STOPAT) return 1; 
			
         bdd1max++; bdd1pos++;
         bdd2max++; bdd2pos++;
      }
   }
}


BDDNode * _xquantify (BDDNode * f, int v);
BDDNode * xquantify (BDDNode * f, int v)
{
   start_bdd_flag_number(XQUANTIFY_FLAG_NUMBER);
   return _xquantify(f, v);
}

BDDNode * _xquantify (BDDNode * f, int v)
{
   if (f->flag == bdd_flag_number) return f->tmp_bdd;
	//NotCase is not correct to use in this function.
   f->flag = bdd_flag_number;
	
   BDDNode *var = ite_var(v);
   BDDNode *cached = itetable_find_or_add_node(10, f, var, NULL);
   if (cached) return (f->tmp_bdd = cached);

   if (f->variable == v)
      return (f->tmp_bdd = ite_or_te(f));
      //return (f->tmp_bdd = ite_or (f->thenCase, f->elseCase));

   if (v > f->variable)		//Does v exist in f?
      return (f->tmp_bdd = f);			      //no, then leave
   BDDNode *r = _xquantify (f->thenCase, v);
   BDDNode *e = _xquantify (f->elseCase, v);

	if (r == e)
      return (f->tmp_bdd = r);
   return (f->tmp_bdd = itetable_add_node(10, f, var, find_or_add_node (f->variable, r, e)));
}

BDDNode * OLD_xquantify (BDDNode * f, int v)
{
   if (f->variable == v)
      return ite_or (f->thenCase, f->elseCase);

   if (v > f->variable)		//Does v exist in f?
      return f;			      //no, then leave
   BDDNode *r = xquantify (f->thenCase, v);
   BDDNode *e = xquantify (f->elseCase, v);

	if (r == e)
      return r;
   return find_or_add_node (f->variable, r, e);
}

//SEAN HERE!!! - should update this to do everything xquantify does
BDDNode * uquantify (BDDNode * f, int v)
{
   if (f->variable == v)
      return ite_and (f->thenCase, f->elseCase);

   if (v > f->variable)		//Does v exist in f?
      return f;			//no, then leave
   BDDNode * r = uquantify (f->thenCase, v);
   BDDNode * e = uquantify (f->elseCase, v);

   if (r == e)
      return r;
   return find_or_add_node (f->variable, r, e);
}


//to xor variable 3 and variable 5 together you do this -
//functions[0] = ite_xor(ite_var( 3 ), ite_var( 5 ));

//to xor variable 7 and BDD function 0 you do this -
//functions[1] = ite_xor(ite_var( 7 ), functions[0]);

//all binary operations work the same, and, or, equ, ect...
//'not' takes just one argument and works like this -
//functions[2] = ite_not(ite_var( 3 ));

//gcf is generalized co-factoring and takes as input two BDDs
//BDDNode *gcf(BDDNode *f, BDDNode *c)

//xquantify is exist-quantify and takes as input one BDD and one integer
//BDDNode *xquantify(BDDNode *f, int v)

////////////////////

int is_in_set(int var, int *set, int max) {
   for (int i = 0; i<max; i++) {
      if (set[i] == var) return i;
   }
   return -1;
}

void collect_nonxors(BDDNode *x, int *xor_vars, int *xor_vars_idx, int *nonxor_vars, int *nonxor_vars_idx) {
int idx;
   if (!IS_TRUE_FALSE(x->thenCase))
      collect_nonxors(x->thenCase, xor_vars, xor_vars_idx, nonxor_vars, nonxor_vars_idx);
   if (!IS_TRUE_FALSE(x->elseCase))
      collect_nonxors(x->elseCase, xor_vars, xor_vars_idx, nonxor_vars, nonxor_vars_idx);

   if ((idx=is_in_set(x->variable, xor_vars, *xor_vars_idx))!=-1) {
      (*xor_vars_idx)--;
      xor_vars[idx] = xor_vars[*xor_vars_idx];
      nonxor_vars[(*nonxor_vars_idx)++] = x->variable;
   } else 
   if (is_in_set(x->variable, nonxor_vars, *nonxor_vars_idx)==-1) {
      nonxor_vars[(*nonxor_vars_idx)++] = x->variable;
   }
}

void countSingleXors(BDDNode *x, int *xor_vars, int *nonxor_vars) {
	int xor_vars_idx = 0;
	int nonxor_vars_idx = 0;
	for(; !IS_TRUE_FALSE(x); x = x->elseCase) {
		if(x->thenCase == true_ptr) {
         if (is_in_set(x->variable, nonxor_vars, nonxor_vars_idx) == -1) {
            xor_vars[xor_vars_idx++] = x->variable;
         }
		} else {
         if (!IS_TRUE_FALSE(x->thenCase))
            collect_nonxors(x->thenCase, xor_vars, &xor_vars_idx, nonxor_vars, &nonxor_vars_idx);
			int idx;
			if((idx=is_in_set(x->variable, xor_vars, xor_vars_idx))!=-1) {
				xor_vars_idx--;
				xor_vars[idx] = xor_vars[xor_vars_idx];
				nonxor_vars[nonxor_vars_idx++] = x->variable;
			} else 
			  if(is_in_set(x->variable, nonxor_vars, nonxor_vars_idx)==-1) {
				  nonxor_vars[nonxor_vars_idx++] = x->variable;
			  }
		}
	}
	xor_vars[xor_vars_idx] = 0;
	nonxor_vars[nonxor_vars_idx] = 0;
}

void _unravelBDD(int *y, int *max, int **tempint, BDDNode * func);

void unravelBDD(int *y, int *max, int **tempint, BDDNode * func) {
  *y=0;
  // assert (no flag is set );
  start_bdd_flag_number(UNRAVELBDD_FLAG_NUMBER);
  _unravelBDD(y, max, tempint, func);
  for (int i = 0;i<*y;i++) {
     // clear the flag
    sym_reset_flag((*tempint)[i]);
  }
}

void _unravelBDD(int *y, int *max, int **tempint, BDDNode * func) {
   if (func->flag == bdd_flag_number) return;
	if (func->notCase->flag == bdd_flag_number) return;
   func->flag = bdd_flag_number;

	if ((func == true_ptr) || (func == false_ptr))
	  return;
   assert(func->variable > 0);
	if (sym_is_flag(func->variable) == 0) {
		if (*y >= *max) {
			*tempint = (int*)ite_recalloc(*(void**)tempint, *max, *max+100, sizeof(int), 9, "tempint");
			*max += 100;
		}
		(*tempint)[*y] = func->variable;
		sym_set_flag(func->variable);
		(*y)++;
	}
	_unravelBDD (y, max, tempint, func->thenCase);
	_unravelBDD (y, max, tempint, func->elseCase);
}

BDDNode * _set_variable (BDDNode * f, int num, int torf);

BDDNode * set_variable_all_infs(BDDNode *f) {
   int num,torf;
   infer *head = f->inferences;
   while(head != NULL) {
      start_bdd_flag_number(SETVARIABLEALL_FLAG_NUMBER);
      if (head->nums[1] == 0) {
        if (head->nums[0] < 0) { 
           num=-head->nums[0]; 
           torf=0; 
        } else { 
           num=head->nums[0]; 
           torf=1; 
        }
        f = _set_variable(f, num, torf);
      }
      head = head->next;
   }
   return f;
}

void set_variable_all_bdds(llist * k, int num, int torf, BDDNode **bdds) {
   start_bdd_flag_number(SETVARIABLEALL_FLAG_NUMBER);
	while(k != NULL) {
      bdds[k->num] = _set_variable(bdds[k->num], num, torf);
      k = k->next;
   }
}

void set_variable_all(llist * k, int num, int torf) {
   start_bdd_flag_number(SETVARIABLEALL_FLAG_NUMBER);
   while(k != NULL) {
      functions[k->num] = _set_variable(functions[k->num], num, torf);
      k = k->next;
   }
}

BDDNode * set_variable (BDDNode * f, int num, int torf) {
   start_bdd_flag_number(SETVARIABLE_FLAG_NUMBER);
   return _set_variable(f, num, torf);
}

BDDNode * _set_variable (BDDNode * f, int num, int torf) {
	if (f->variable < num) return f;

   if (f->flag == bdd_flag_number) return f->tmp_bdd;
	if (f->notCase->flag == bdd_flag_number) return f->notCase->tmp_bdd->notCase;
   f->flag = bdd_flag_number;
   
	if (f->variable == num) {
      if (torf)
		  return (f->tmp_bdd = f->thenCase);
      else
		  return (f->tmp_bdd = f->elseCase);
   }
   BDDNode * r = _set_variable (f->thenCase, num, torf);
   BDDNode * e = _set_variable (f->elseCase, num, torf);
   if (r == e)
	  return (f->tmp_bdd = r);
   return (f->tmp_bdd = find_or_add_node (f->variable, r, e));
}

BDDNode *set_variable_noflag(BDDNode * f, int num, int torf) {
	if (f->variable < num) return f;                                                  
	if (f->variable == num) {
		if (torf) return f->thenCase;                                                   
		else return f->elseCase;                                                        
	}
	
	BDDNode *r = set_variable_noflag(f->thenCase, num, torf);                         
	BDDNode *e = set_variable_noflag(f->elseCase, num, torf);                         
	if (r == e) return r;                                                             
	return find_or_add_node(f->variable, r, e);                                       
}
  
BDDNode * _num_replace (BDDNode * f, int var, int replace);

void num_replace_all(llist *k, int var, int replace) 
{
   start_bdd_flag_number(NUMREPLACEALL_FLAG_NUMBER);
   while(k != NULL) {
      functions[k->num] = _num_replace(functions[k->num], var, replace);
      k = k->next; 
   }
}

BDDNode *_safe_assign(BDDNode *f, int v);

BDDNode *safe_assign(BDDNode *f, int v) {
	start_bdd_flag_number(SAFEBDD_FLAG_NUMBER);
	return _safe_assign(f, v);	
}

BDDNode *_safe_assign(BDDNode *f, int v) {
	if(v > f->variable) return true_ptr;
	if(IS_TRUE_FALSE(f)) return true_ptr;	
	if(f->flag == bdd_flag_number) return f->tmp_bdd;
	if(f->notCase->flag == bdd_flag_number) return f->notCase->tmp_bdd->notCase;
   f->flag = bdd_flag_number;
	
	BDDNode *itevar = ite_var(v);
	BDDNode *cached = itetable_find_or_add_node(14, itevar, f, NULL);
	if (cached) return (f->tmp_bdd = cached);

	if(v == f->variable) {
		BDDNode *r = constant_and(f->thenCase, f->elseCase->notCase);
		BDDNode *e = constant_and(f->thenCase->notCase, f->elseCase);
		//return (f->tmp_bdd = ite(ite_var(v), e->notCase, r->notCase));
		return (f->tmp_bdd = itetable_add_node(14, itevar, f,	ite(ite_var(v), e->notCase, r->notCase)));
	}
	
	BDDNode *r = _safe_assign(f->thenCase, v);
	if(r == false_ptr) //return (f->tmp_bdd = false_ptr);
	  return (f->tmp_bdd = itetable_add_node(14, itevar, f, false_ptr));
	//return (f->tmp_bdd = ite_and(r, _safe_assign(f->elseCase, v)));
	return (f->tmp_bdd = itetable_add_node(14, itevar, f, ite_and(r, _safe_assign(f->elseCase, v))));
}

BDDNode *safe_assign_eq(BDDNode *f, int v) {
	BDDNode *cached = itetable_find_or_add_node(16, ite_var(v), f, NULL);
	if(cached) return cached;
   BDDNode *safe_infs = itetable_add_node(16, ite_var(v), f,
														ite(ite_var(v), ite_and(set_variable(f, v, 1),
																						ite_not(set_variable(f, v, 0))),
															             ite_and(ite_not(set_variable(f, v, 1)),
																						set_variable(f, v, 0))));
	return safe_infs;

	if(safe_infs == false_ptr) return false_ptr;
	infer *inferlist = safe_infs->inferences;
	if(inferlist == NULL) return true_ptr;
	while(inferlist!=NULL) {
		if(abs(inferlist->nums[0]) == v && inferlist->nums[1] == 0) {
			safe_infs = ite_and(safe_infs, ite_var(inferlist->nums[0]));
		} else if(abs(inferlist->nums[0]) == v && inferlist->nums[1] != 0) {
			safe_infs = ite_and(safe_infs, ite_equ(ite_var(inferlist->nums[0]), ite_var(inferlist->nums[1])));
		} else if(abs(inferlist->nums[1]) == v) {
			safe_infs = ite_and(safe_infs, ite_equ(ite_var(inferlist->nums[0]), ite_var(inferlist->nums[1])));
		}
		inferlist = inferlist->next;
	}
	
	return safe_infs;
}

//SEAN!!! CONTINUE WORKING ON THIS
BDDNode *_safe_assign_func(BDDNode *, BDDNode *);

BDDNode *safe_assign_func(BDDNode *f, int x) {
	BDDNode *x_true = set_variable(f, x, 1);
	BDDNode *x_false = set_variable(f, x, 0);
	BDDNode *factor = _safe_assign_func(x_true, x_false);
	if(factor == NULL) return true_ptr;
	return ite(ite_var(x), factor, ite_not(factor));
}

BDDNode *_safe_assign_func(BDDNode *x_true, BDDNode *x_false) {
	if(x_true == ite_not(x_false)) return x_true;
	if(x_true == false_ptr) return ite_not(x_false);//false_ptr;
	if(x_false == false_ptr) return x_true; //true_ptr;
	if(x_true == x_false) return NULL; //Break early, returning null
	BDDNode *r;
	BDDNode *e;
	if(x_true->variable > x_false->variable) {
		return _safe_assign_func(xquantify(x_true, x_true->variable), x_false);
	} else if(x_true->variable < x_false->variable) {
		return _safe_assign_func(x_true, xquantify(x_false, x_false->variable));
	} else { //x_true->variable == x_false->variable
		r = _safe_assign_func(x_true->thenCase, x_false->thenCase);
		if(r == NULL) return NULL;
		e = _safe_assign_func(x_true->elseCase, x_false->elseCase);
		if(e == NULL) return NULL;		  
	}
	if(r == e) return r;
	return find_or_add_node(x_true->variable, r, e);
} 


BDDNode *safe_assign_all(BDDNode **bdds, llistStruct *amount, int v) {
	BDDNode *safeVal = true_ptr;
	for (llist * k = amount[v].head; k != NULL; k = k->next) {
		BDDNode *ex_bdd = bdds[k->num];
		for (int x = 0; x < length[k->num]; x++) {
			if(variables[k->num].num[x] == v) continue;
			if(amount[variables[k->num].num[x]].head->next == NULL)
			  ex_bdd = xquantify(ex_bdd, variables[k->num].num[x]);
		}
		
		safeVal = ite_and(safe_assign(ex_bdd, v), safeVal);
		if(safeVal == false_ptr) break;
	}
	return safeVal;
}

BDDNode *safe_assign_eq_all(BDDNode **bdds, llistStruct *amount, int v) {
	BDDNode *safe_infs = false_ptr;
	for (llist * k = amount[v].head; k != NULL; k = k->next) {
		BDDNode *ex_bdd = bdds[k->num];
		for (int x = 0; x < length[k->num]; x++) {
			if(variables[k->num].num[x] == v) continue;
			if(amount[variables[k->num].num[x]].head->next == NULL)
			  ex_bdd = xquantify(ex_bdd, variables[k->num].num[x]);
		}

		BDDNode *temp_safe = safe_assign_eq(ex_bdd, v);
		if(temp_safe == false_ptr) continue;
		
		BDDNode *tempBDD = true_ptr;
		for(infer *iterator = temp_safe->inferences; iterator != NULL; iterator = iterator->next) {
			if (abs(iterator->nums[0]) == v && iterator->nums[1] == 0) {
				tempBDD = ite_and(tempBDD, ite_var(iterator->nums[0]));
			} else if(abs(iterator->nums[0]) == v || abs(iterator->nums[1]) == v) {
				tempBDD = ite_and(tempBDD, ite_equ(ite_var(iterator->nums[0]), ite_var(iterator->nums[1])));
			}
			//fprintf(stderr, "%d|%d, %d|", x, iterator->nums[0], iterator->nums[1]);
		}
		safe_infs = ite_or(safe_infs, tempBDD);		
		
		if(safe_infs->inferences == NULL) return true_ptr;
	}
	
	if(safe_infs == false_ptr) return false_ptr;
	BDDNode *equiv = true_ptr;
	for(infer *inferlist = safe_infs->inferences; inferlist!=NULL; inferlist = inferlist->next) {
		if(abs(inferlist->nums[0]) == v && inferlist->nums[1] == 0) {
			return ite_var(inferlist->nums[0]);
		} else if(equiv == true_ptr) {
			if(equiv == true_ptr && abs(inferlist->nums[0]) == v && inferlist->nums[1] != 0)
			  equiv = ite_equ(ite_var(inferlist->nums[0]), ite_var(inferlist->nums[1]));
			else if(abs(inferlist->nums[1]) == v)
			  equiv = ite_equ(ite_var(inferlist->nums[0]), ite_var(inferlist->nums[1]));
		}
	}
	
	return equiv;
}

BDDNode * num_replace (BDDNode * f, int var, int replace) {
   start_bdd_flag_number(NUMREPLACE_FLAG_NUMBER);
   return _num_replace(f, var, replace);
}

BDDNode * _num_replace (BDDNode * f, int var, int replace) {
	if (var > f->variable)
	  return f;
   if (f->flag == bdd_flag_number) return f->tmp_bdd;
	if (f->notCase->flag == bdd_flag_number) return f->notCase->tmp_bdd->notCase;
   f->flag = bdd_flag_number;
	if (var == f->variable) {
		if (replace > 0) {
			//return (f->tmp_bdd = ite(ite_var(replace), f->thenCase, f->elseCase));
			return (f->tmp_bdd = ite_xvar_y_z(ite_var(replace), f->thenCase, f->elseCase));
			BDDNode *itevar = ite_var(replace);
         BDDNode *cached = itetable_find_or_add_node(11, itevar, f, NULL);
         if (cached) return (f->tmp_bdd = cached);
         //return (f->tmp_bdd = itetable_add_node(11, itevar, f, 
         //         ite(itevar, f->thenCase, f->elseCase)));
         return (f->tmp_bdd = itetable_add_node(11, itevar, f, 
                  ite_xvar_y_z(itevar, f->thenCase, f->elseCase)));
      } else if (replace < 0) {
         //return (f->tmp_bdd = ite(ite_var(-replace), f->elseCase, f->thenCase));
			return (f->tmp_bdd = ite_xvar_y_z(ite_var(-replace), f->elseCase, f->thenCase));
			BDDNode *itevar = ite_var(-replace);
         BDDNode *cached = itetable_find_or_add_node(11, itevar, f, NULL);
         if (cached) return (f->tmp_bdd = cached);
         //return (f->tmp_bdd = itetable_add_node(11, itevar, f,
         //         ite(itevar, f->elseCase, f->thenCase)));
         return (f->tmp_bdd = itetable_add_node(11, itevar, f,
                  ite_xvar_y_z(itevar, f->elseCase, f->thenCase)));
      }
   }
   BDDNode * r = _num_replace (f->thenCase, var, replace);
   BDDNode * e = _num_replace (f->elseCase, var, replace);
   if (r == e)
	  return (f->tmp_bdd = r);
	//return (f->tmp_bdd = itetable_add_node(11, ite_var(f->variable), f, ite (ite_var (f->variable), r, e)));
	return (f->tmp_bdd = itetable_add_node(11, ite_var(f->variable), f, ite_xvar_y_z(ite_var (f->variable), r, e)));

	//return (f->tmp_bdd = find_or_add_node(f->variable, r, e));
}


//SEAN FIX THIS!!!
BDDNode *_func_replace(BDDNode *f, BDDNode *replace);

BDDNode *func_replace(BDDNode *f, BDDNode *replace) {

	return _func_replace(f, replace);
}

BDDNode *_func_replace(BDDNode *f, BDDNode *replace) {
	
	return f;
}

BDDNode *_possible_BDD_x(BDDNode *f, int x);
	
BDDNode *possible_BDD_x(BDDNode *f, int x) {
   start_bdd_flag_number(POSSIBLEBDD_FLAG_NUMBER);
   BDDNode *result = _possible_BDD_x(f, x);
	return result;
}

BDDNode *_possible_BDD_x(BDDNode *f, int x) {
	if(f->variable < x) return true_ptr; //This could happen on a path that doesn't involve x

   if(f->flag == bdd_flag_number) return f->tmp_bdd;
   f->flag = bdd_flag_number;
   assert(f->tmp_infer == NULL);
	
	if(f->variable == x) {
		BDDNode *r;
		//if(f->t_and_not_e_bdd != NULL) r = f->t_and_not_e_bdd;
		//else
		  r = and_dot(f->thenCase, ite_not(f->elseCase));
//		if(r == false_ptr) {				  
//			return f->tmp_bdd = ite_var(-x); //Question! Maybe better to comment this out.
//		}
		BDDNode *e;
		//if(f->not_t_and_e_bdd != NULL) e = f->not_t_and_e_bdd;
		//else
		  e = and_dot(f->elseCase, ite_not(f->thenCase));
//		if(e == false_ptr) {
//			return f->tmp_bdd = ite_var(x); //Question! Maybe better to comment this out.
//		}

		return (f->tmp_bdd = find_or_add_node(x, r, e));

	} else {
		BDDNode *r = _possible_BDD_x(f->thenCase, x);
		BDDNode *e = _possible_BDD_x(f->elseCase, x);

		//if(IS_TRUE_FALSE(r)) return (f->tmp_bdd = e);
		if(r == true_ptr) return (f->tmp_bdd = e);
		//The above leaves some inferences in.
		//if(IS_TRUE_FALSE(e)) return (f->tmp_bdd = r);
		if(e == true_ptr) return (f->tmp_bdd = r);
		//The above leaves some inferences in.
		if(r == e) return (f->tmp_bdd = r);
		return (f->tmp_bdd = find_or_add_node(f->variable, r, e));
	}
}

BDDNode * _possible_BDD (BDDNode * f, int v);
BDDNode * possible_BDD (BDDNode * f, int v) {
	start_bdd_flag_number(POSSIBLE_BDD_FLAG_NUMBER);
	return _possible_BDD(f, v);
}

BDDNode *_possible_BDD(BDDNode *f, int v) {
	if (f->flag == bdd_flag_number) return f->tmp_bdd;
	//NotCase is not correct to use in this function.
	f->flag = bdd_flag_number;
	
	BDDNode *var = ite_var(v);
	BDDNode *cached = itetable_find_or_add_node(8, f, var, NULL);
	if (cached) return (f->tmp_bdd = cached);
	
	if(f->variable < v) return (f->tmp_bdd = false_ptr);
	//if(f->variable < v) return (f->tmp_bdd = var);
	if(f->variable == v) {
		BDDNode *r, *e;
		//if(f->t_and_not_e_bdd != NULL) r = f->t_and_not_e_bdd;
		//else f->t_and_not_e_bdd =
		  r = ite_and(f->thenCase, ite_not(f->elseCase));
		//if(f->not_t_and_e_bdd != NULL) e = f->not_t_and_e_bdd;
		//else f->not_t_and_e_bdd =
		  e = ite_and(ite_not(f->thenCase), f->elseCase);
		//return (f->tmp_bdd = ite(ite_var(v), r, e));
		return (f->tmp_bdd = ite_xvar_y_z(ite_var(v), r, e));
	}

	BDDNode *r = _possible_BDD(f->thenCase, v);
	BDDNode *e = _possible_BDD(f->elseCase, v);
	
	if(r == e) return (f->tmp_bdd = r);
	//return (f->tmp_bdd = itetable_add_node(8, f, var, ite(ite_var(f->variable), r, e)));
	return (f->tmp_bdd = itetable_add_node(8, f, var, ite_xvar_y_z(ite_var(f->variable), r, e)));
}

infer *_possible_infer_x(BDDNode *f, int x);
void clean_possible_infer_x(BDDNode *f, int x);
	
infer *possible_infer_x(BDDNode *f, int x) {
   start_bdd_flag_number(POSSIBLEINFER_FLAG_NUMBER);
   infer *result = _possible_infer_x(f, x);
   start_bdd_flag_number(CLEANPOSSIBLE_FLAG_NUMBER);
	clean_possible_infer_x(f, x);
	return result;
}

infer *copy_infer(infer *inference) {
   /*
   infer *result = new infer;
   infer *tmp_infer = result;   
   for(infer *infer_iter=inference; infer_iter!=NULL; infer_iter=infer_iter->next) { 
      tmp_infer->next = new infer;
      tmp_infer = tmp_infer->next;
      tmp_infer->nums[0] = infer_iter->nums[0];
      tmp_infer->nums[1] = infer_iter->nums[1];
   }     
   tmp_infer->next = NULL;
   tmp_infer = result;
   result = result->next;
   delete tmp_infer;

   return result;
   better: ?*/
   infer *head = NULL;
   infer **infs = &(head);
	for(infer *infer_iter=inference; infer_iter!=NULL; infer_iter=infer_iter->next)
   {
      *infs = AllocateInference(infer_iter->nums[0], infer_iter->nums[1], NULL);
      infs = &((*infs) -> next);
   }
	return head;
}

infer *_possible_infer_x(BDDNode *f, int x) {
	if(f->variable < x) return NULL; //This could happen on a path that doesn't involve x

   if(f->flag == bdd_flag_number) return copy_infer(f->tmp_infer);
   f->flag = bdd_flag_number;
   assert(f->tmp_infer == NULL);
	
	if(f->variable == x) {
		if(f->thenCase == true_ptr || f->elseCase == false_ptr) {
         /*
			infer *inference = new infer;
			inference->nums[0] = x;
			inference->nums[1] = 0;
			inference->next = NULL;
			f->tmp_infer = copy_infer(inference);
			return inference;
         */
         f->tmp_infer = AllocateInference(x, 0, NULL);
			return copy_infer(f->tmp_infer);
		} else if(f->elseCase == true_ptr || f->thenCase == false_ptr) {
         /*
			infer *inference = new infer;
			inference->nums[0] = -x;
			inference->nums[1] = 0;
			inference->next = NULL;
			f->tmp_infer = copy_infer(inference);
			return inference;
         */
         f->tmp_infer = AllocateInference(-x, 0, NULL);
			return copy_infer(f->tmp_infer);
		} else {
			BDDNode *r_BDD;
			//if(f->t_and_not_e_bdd != NULL) r_BDD = f->t_and_not_e_bdd;
			//else
			  r_BDD = and_dot(f->thenCase, ite_not(f->elseCase));
			                 //Ex_GetInfer(f->thenCase);
			if(r_BDD == false_ptr) {				  
            /*
				infer *inference = new infer;
				inference->nums[0] = -x;
				inference->nums[1] = 0;
				inference->next = NULL;
				f->tmp_infer = copy_infer(inference);
				return inference;
            */
            f->tmp_infer = AllocateInference(-x, 0, NULL);
				return copy_infer(f->tmp_infer);
			}
			infer *head = NULL;
			//infer *temp = NULL;
			infer *r = r_BDD->inferences;
			if(r == NULL) {
            /*
				head = new infer;
				head->nums[0] = 0;
				head->nums[1] = 0;
				head->next = NULL;
				f->tmp_infer = copy_infer(head);
				return head;
            */
            f->tmp_infer = AllocateInference(0, 0, NULL);
				return copy_infer(f->tmp_infer);
			}
			BDDNode *e_BDD;
			//if(f->not_t_and_e_bdd != NULL) e_BDD = f->not_t_and_e_bdd;
			//else 
			  e_BDD = and_dot(f->elseCase, ite_not(f->thenCase));
			                 //Ex_GetInfer(f->elseCase);
			if(e_BDD == false_ptr) {
            /*
				infer *inference = new infer;
				inference->nums[0] = x;
				inference->nums[1] = 0;
				inference->next = NULL;
				f->tmp_infer = copy_infer(inference);
				return inference;
            */
            f->tmp_infer = AllocateInference(x, 0, NULL);
				return copy_infer(f->tmp_infer);
			}
			infer *e = e_BDD->inferences;
			if(e == NULL) {
				//while (r!=NULL) { temp = r; r = r->next; delete temp;	}
            /*
				head = new infer;
				head->nums[0] = 0;
				head->nums[1] = 0;
				head->next = NULL;
				f->tmp_infer = copy_infer(head);
				return head;
            */
            f->tmp_infer = AllocateInference(0, 0, NULL);
				return copy_infer(f->tmp_infer);
			}
			
			for(infer *r_iter=r; r_iter!=NULL; r_iter=r_iter->next) {
				if(r_iter->nums[1] == 0) {
					for(infer *e_iter=e; e_iter!=NULL; e_iter=e_iter->next) {
						if(e_iter->nums[1] == 0) {
							if(r_iter->nums[0] == -e_iter->nums[0]) {
                        /*
								temp = head;
								head = new infer;
								head->nums[0] = x;
								head->nums[1] = r_iter->nums[0];
								head->next = temp;
                        */
                        head = AllocateInference(x, r_iter->nums[0], head);
								//fprintf(stderr, "[%d, %d]", x, r_iter->nums[0]);
							}
						}
					}
				}
			}

         // watch out: r, e are REAL inferences -- dont delete them
         // DeallocateInferences(r);
         // DeallocateInferences(e);
			
			if(head == NULL) {
            /*
            head = new infer;
				head->nums[0] = 0;
				head->nums[1] = 0;
				head->next = NULL;
            */
            head = AllocateInference(0, 0, NULL);
//			} else {
//				int w = 0;
//				printBDDTree(f, &w);
			}
		   /*	
			f->tmp_infer = copy_infer(head);
			return head;
         */
			f->tmp_infer = head;
			return copy_infer(f->tmp_infer);
		}
	} else {
		infer *r = _possible_infer_x(f->thenCase, x);
		if(r != NULL) 
		  if(r->nums[0] == 0) {
			  f->tmp_infer = r;
			  return copy_infer(f->tmp_infer);
		  }
		
		infer *e = _possible_infer_x(f->elseCase, x);
		if(e != NULL) {
			if(e->nums[0] == 0) {
            // while (r!=NULL) { infer *temp = r; r = r->next; delete temp; }
            DeallocateInferences(r);
				f->tmp_infer = e;
				return copy_infer(f->tmp_infer);
			}
		}
		
		if(r == NULL) { f->tmp_infer = e; return copy_infer(f->tmp_infer); }
		if(e == NULL) { f->tmp_infer = r; return copy_infer(f->tmp_infer); }
		//If both are NULL, NULL is returned;
		
		if((r->nums[1] == 0 && e->nums[1] != 0) ||
			(r->nums[1] == 0 && e->nums[1] == 0 && r->nums[0] != abs(e->nums[0]))) {
         /*
			infer *head = new infer;
			head->nums[0] = 0;
			head->nums[1] = 0;
			head->next = NULL;
			while (r!=NULL) { infer *temp = r; r = r->next; delete temp; }
			while (e!=NULL) { infer *temp = e; e = e->next; delete temp; }
			f->tmp_infer = copy_infer(head);
			return head;
         */
         DeallocateInferences(r);
         DeallocateInferences(e);
         f->tmp_infer = AllocateInference(0, 0, NULL);
         return copy_infer(f->tmp_infer);
		} else if(r->nums[1] == 0 && e->nums[1] == 0 && r->nums[0] == e->nums[0]) {
			//Both sides are same inference, return r;
         /*
			while (e!=NULL) { infer *temp = e; e = e->next; delete temp; }
			f->tmp_infer = copy_infer(r);
			return r;
         */
         DeallocateInferences(e);
         f->tmp_infer = r;
         return copy_infer(f->tmp_infer);
		} else if(r->nums[1] == 0 && e->nums[1] == 0 && r->nums[0] == -e->nums[0]) {
			//Sides are opposite inference, return {f->variable, r->nums[1]};
			r->nums[1] = r->nums[0]; //r->nums[1] = -e->nums[0];
			r->nums[0] = f->variable;
         /*
			while (e!=NULL) { infer *temp = e; e = e->next; delete temp; }
			f->tmp_infer = copy_infer(r);
			return r;
         */
         DeallocateInferences(e);
         f->tmp_infer = r;
         return copy_infer(f->tmp_infer);
		}
		
		//They share equivalences of x = {y, -y}
		//or y = {x, -x}
		//return all matching equivalences

		infer *head = NULL;
		for(infer *r_iter = r; r_iter != NULL; r_iter=r_iter->next) {
			for(infer *e_iter = e; e_iter != NULL; e_iter=e_iter->next) {
				if(r_iter->nums[0] == e_iter->nums[0] && r_iter->nums[1] == e_iter->nums[1]) {
               /*
					infer *temp = head;
					head = new infer;
					head->nums[0] = r_iter->nums[0];
					head->nums[1] = r_iter->nums[1];
					head->next = temp;
               */
               head = AllocateInference(r_iter->nums[0], r_iter->nums[1], head);
					break;
				}
			}
		}
		if(head == NULL) {
         /*
			head = new infer;
			head->nums[0] = 0;
			head->nums[1] = 0;
			head->next = NULL;
         */
         head = AllocateInference(0, 0, NULL);
		}
      /*
		infer *temp;
		while (r!=NULL) { temp = r; r = r->next; delete temp;	}
		while (e!=NULL) { temp = e; e = e->next; delete temp; }
		f->tmp_infer = copy_infer(head);
		return head;
      */
      DeallocateInferences(r);
      DeallocateInferences(e);
      f->tmp_infer = head;
      return copy_infer(f->tmp_infer);
	}
}

void clean_possible_infer_x(BDDNode *f, int x) {
	if(f->variable < x) return;
	if(f->flag == bdd_flag_number) return;
	f->flag = bdd_flag_number;
	DeallocateInferences(f->tmp_infer); f->tmp_infer = NULL;
	//while (f->tmp_infer!=NULL) { infer *temp = f->tmp_infer; f->tmp_infer = f-$
	clean_possible_infer_x(f->thenCase, x);
	clean_possible_infer_x(f->elseCase, x);
}

void cheat_replace (BDDNode * f, int var, int replace) {
   if (var > f->variable)
	  return;
   if (var == f->variable) {
      f->variable = replace;
      return;
   }
   cheat_replace (f->thenCase, var, replace);
   cheat_replace (f->elseCase, var, replace);
}

int getoldNuminp () {
   int *tempint=NULL;
   int tempint_max=0;
   int y, i;
   numinp = 0;
   for (int x = 0; x < nmbrFunctions; x++) {
      y = 0;
      unravelBDD (&y, &tempint_max, &tempint, functions[x]);
      if (y != 0) {
			for (i = 0; i < y; i++) {
            if (tempint[i] > numinp)
				  numinp = tempint[i];
         }
      }
   }
   ite_free((void**)&tempint); tempint_max = 0;
   return numinp;
}

int getNuminp () {
   int temp_numinp = 0;
   for (int x = 0; x < nmbrFunctions; x++) {
		if (functions[x]->variable > temp_numinp)
		  temp_numinp = functions[x]->variable;
	}
   return temp_numinp;
}
