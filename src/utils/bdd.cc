/* =========FOR INTERNAL USE ONLY. NO DISTRIBUTION PLEASE ========== */

/*********************************************************************
 Copyright 1999-2004, University of Cincinnati.  All rights reserved.
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
 *  bdd.c (S.Weaver)
 ***********************************************************************/ 

#include "ite.h"

#define SORT_ITE_OPS
//#define ite_x_F_z(x,z) ite_x_y_F(ite_not(x), z)
#define ite_x_F_z(x,z) _ite_x_F_z(x,z)
//#define ite_x_T_z(x,z) ite_not(ite_x_y_F(ite_not(x), ite_not(z)))
#define ite_x_T_z(x,z) _ite_x_T_z(x,z)
//#define ite_x_y_T(x,y) ite_not(ite_x_y_F(x, ite_not(y)))
#define ite_x_y_T(x,y) _ite_x_y_T(x,y)

enum {
   BDD2XDD_FLAG_NUMBER,
   FORCE_REUSE_FLAG_NUMBER, /* if bigger than this => won't get reused */
   XQUANTIFY_FLAG_NUMBER, /* v dependent */
   UNRAVELBDD_FLAG_NUMBER, /* starting bdd dependent */
   COUNTX_FLAG_NUMBER, /* X dependent */
   SETVARIABLE_FLAG_NUMBER, /* torf+-num dependent */
   SETVARIABLEALL_FLAG_NUMBER, /* variable list dependent */
   NUMREPLACE_FLAG_NUMBER, /* var/replace dependent */
   NUMREPLACEALL_FLAG_NUMBER, /* var/replace list dependent */
   POSSIBLEINFER_FLAG_NUMBER,
	CLEANPOSSIBLE_FLAG_NUMBER,
   PRINTSHAREDBDDS_FLAG_NUMBER, 
	MAX_FLAG_NUMBER
};
int last_bdd_flag_number[MAX_FLAG_NUMBER] = {0,0,0,0,0,0,0,0,0,0};
int bdd_flag_number = 2;

//#define itetable_add_node itetable_find_or_add_node

inline void
start_bdd_flag_number(int last_bdd_flag)
{
   if (last_bdd_flag > FORCE_REUSE_FLAG_NUMBER ||
         last_bdd_flag_number[last_bdd_flag] != bdd_flag_number)
   {
      bdd_flag_number++;
      if (bdd_flag_number > 1000000000) {
         bdd_gc();
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
      //    fprintf(stderr, "\nVariable %d not found in translation array...quantifying it away\n", v);
      return mitosis (xquantify (bdd, v), structureBDD, newBDD);
   }
   v = newBDD[num];
   BDDNode * r = mitosis (bdd->thenCase, structureBDD, newBDD);
   BDDNode * e = mitosis (bdd->elseCase, structureBDD, newBDD);
   return ite (ite_var (v), r, e);
}

inline BDDNode *
MinMaxBDD_within(int set_vars, int min, int max) 
{
   if (set_vars >= min && set_vars <= max) return true_ptr;
   else return false_ptr;
}


BDDNode *MinMaxBDD(int *vars, int min, int max, int total_vars, int set_true) {
   int i,j;
   // make sure vars are sorted;
   //d4_printf5("MinMaxBDD(int *vars, int min=%d, int max=%d, int total_vars=%d, int set_true=%d)\n", min, max, total_vars, set_true);
   qsort(vars, total_vars, sizeof(int), revcompfunc);
   for(i=1;i<total_vars;i++) assert(vars[i] < vars[i-1]);
   BDDNode **arr = (BDDNode**)ite_calloc(total_vars+1, sizeof(BDDNode**), 2, "MinMaxBDD");
   BDDNode **prev_arr = (BDDNode**)ite_calloc(total_vars+1, sizeof(BDDNode**), 2, "MinMaxBDD");
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
	//if (num_left == 0) hoo don't need this one right
	int v = vars[num_left-1]; //vars should be sorted from least to greatest
	
	BDDNode *r = MinMaxBDD(vars, min, max, num_left-1, set_true+1); //Then Case
	BDDNode *e = MinMaxBDD(vars, min, max, num_left-1, set_true); //Else Case
	
	if(r == e) return r;
	return find_or_add_node (v, r, e);
}

int countnodes(BDDNode *f) {
   if(IS_TRUE_FALSE(f))
      return 0;
   return (countnodes(f->thenCase) + countnodes(f->elseCase));	
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

#define print_bdd(f) print_bdd1(f, 0)
void
print_bdd1 (BDDNode * f, int print_counter)
{
   int i;
   for (i = 0; i < print_counter; i++)
      d2_printf1 ("    ");
   if (IS_TRUE_FALSE(f))
   {
      if (f == false_ptr) {
         d2_printf1 ("F\n");
		} else if (f == true_ptr) {
         d2_printf1 ("T\n");
		}
      return;
   }
   if (!(IS_TRUE_FALSE(f->thenCase) && IS_TRUE_FALSE(f->elseCase)))
   {
      //      if ((BDD[BDD_num].v < 2) || (BDD[BDD_num].v >= next_var_list))
      //printf ("ite %d\n", f->variable);
		d2e_printf2("ite %s\n", s_name(f->variable));
		d3e_printf2("ite %s\n", s_name(f->variable));
		d4_printf3("ite %s (%d)\n", s_name(f->variable), f->variable);
      print_bdd1 (f->thenCase, print_counter + 1);
      print_bdd1 (f->elseCase, print_counter + 1);
      return;
   } else {
      //printf ("ite %d ", f->variable);
		d2e_printf2("ite %s ", s_name(f->variable));
		d3e_printf2("ite %s ", s_name(f->variable));
		d4_printf3("ite %s (%d) ", s_name(f->variable), f->variable);
      if (f->thenCase == true_ptr) {
         d2_printf1 ("T F\n");
		} else {
         d2_printf1 ("F T\n");
		}
      return;
   }
}
void
printBDDfile(BDDNode * bdd, FILE * fout)
{
   if (bdd == true_ptr)
   {
      fprintf (fout, "T");
      return;
   }
   if (bdd == false_ptr)
   {
      fprintf (fout, "F");
      return;
   }
   fprintf (fout, "(");
   printBDDfile(bdd->thenCase, fout);
   //fprintf (stdout, "[%d]", bdd->variable);
	fprintf(fout, "[%s", s_name(bdd->variable));
	d4_printf2("(%d)", bdd->variable);
	fprintf(fout, "]");
   printBDDfile(bdd->elseCase, fout);
   fprintf(fout, ")");
}
void printBDD(BDDNode * bdd) { 
	printBDDfile(bdd, stddbg);
}

void _printBDDshared(BDDNode *bdd, FILE *fout);
int bdd_shared_no=1;
void
printBDDshared(BDDNode *bdd, FILE *fout)
{
   start_bdd_flag_number(PRINTSHAREDBDDS_FLAG_NUMBER);
}

void
_printBDDshared(BDDNode * bdd, FILE * fout)
{
   if (bdd == true_ptr)
   {
      fprintf (fout, "T");
      return;
   }
   if (bdd == false_ptr)
   {
      fprintf (fout, "F");
      return;
   }
   if (bdd->flag == bdd_flag_number) {
      if (bdd->tmp_int == 0) bdd->tmp_int = bdd_shared_no++;
      fprintf (fout, "{%d}", bdd->tmp_int);
      return;
   }
   bdd->flag = bdd_flag_number;
   bdd->tmp_int = 0;
   fprintf (fout, "(");
   printBDDshared(bdd->thenCase, fout);
   //fprintf (stdout, "[%d]", bdd->variable);
	fprintf(fout, "[%s", s_name(bdd->variable));
	d4_printf2("(%d)", bdd->variable);
	fprintf(fout, "]");
   printBDDshared(bdd->elseCase, fout);
   fprintf(fout, ")");
}

//Make sure variable x does NOT occur in bdd
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
void
verifyCircuit (int x)
{
   for (int i = 0; i < nmbrFunctions; i++)
   {
      if(!verifyBDD (functions[i], x)) {
         fprintf (stderr, "\nPROBLEM: function %d contains variable %d\n", i, x);
         //exit(1);
      }
   }
}


//TESTING PURPOSES ONLY!!!
void
printBDDerr (BDDNode * bdd)
{
   if (bdd == true_ptr)
   {
      fprintf (stderr, "T");
      return;
   }
   if (bdd == false_ptr)
   {
      fprintf (stderr, "F");
      return;
   }
   fprintf (stderr, "(");
   printBDDerr (bdd->thenCase);
   fprintf (stderr, "[%d]", bdd->variable);
   printBDDerr (bdd->elseCase);
   fprintf (stderr, ")");
   if ((bdd->variable <= bdd->thenCase->variable)
         || (bdd->variable <= bdd->elseCase->variable))
   {
      fprintf (stderr, "OHNO!");
      exit (1);
   }
}
void
writeBDD (BDDNode * bdd, FILE * fout)
{
   if (bdd == true_ptr)
   {
      fprintf (fout, "T");
      return;
   }
   if (bdd == false_ptr)
   {
      fprintf (fout, "F");
      return;
   }
   fprintf (fout, "(");
   fprintf (fout, "[%d]", bdd->variable);
   writeBDD (bdd->thenCase, fout);
   writeBDD (bdd->elseCase, fout);
   fprintf (fout, ")");
}

BDDNode * readBDD (FILE * fin)
{
   char a = fgetc (fin);
   if (a == 'T')
      return true_ptr;
   if (a == 'F')
      return false_ptr;
   int v;
   fscanf (fin, "[%d]", &v);
   BDDNode * r = readBDD (fin);
   BDDNode * e = readBDD (fin);
   a = fgetc (fin);
   return find_or_add_node (v, r, e);
}


//level = 0 at start. y = 0 at start. integers[0...numints]
BDDNode *ReadSmurf (int *y, char *tv, int level, int *integers, int num_ints)
{
   if (level == num_ints - 1)
   {
      BDDNode * r = false_ptr, *e = false_ptr;
      if (tv[(*y)++] == '1')
         e = true_ptr;
      if (tv[(*y)++] == '1')
         r = true_ptr;
      if (r == e)
         return r;
      return find_or_add_node (integers[level], r, e);
   }
   BDDNode * e = ReadSmurf (y, tv, level + 1, integers, num_ints);
   BDDNode * r = ReadSmurf (y, tv, level + 1, integers, num_ints);
   if (r == e)
      return r;
   return ite (ite_var (integers[level]), r, e);
}

/* LOOK: what is this? */
int unravelBDDTree (int y, int tempint[100], BDDNode * func) 
{
   if (y > 99)
      return y;
   if (func == true_ptr)
   {
      tempint[y] = -1;
      return y;
   }
   if (func == false_ptr)
   {
      tempint[y] = -2;
      return y;
   }
   tempint[y] = func->variable;
   int r = unravelBDDTree (y * 2 + 1, tempint, func->thenCase);
   int e = unravelBDDTree (y * 2 + 2, tempint, func->elseCase);
   if ((r == -1) || (e == -1))
      return -1;
   if (r > e)
      return r;
   return e;
}

BDDNode * findBranch(int y, int z, BDDNode * func) 
{
   if (y == z)
      return func;
   if (IS_TRUE_FALSE(func))
      return func;
   BDDNode * r = findBranch(y * 2 + 1, z, func->thenCase);
   BDDNode * e = findBranch(y * 2 + 2, z, func->elseCase);
   if (r->variable > e->variable)
      return r;
   return e;
}


// Top level call should have *which_zoom equal to zero.
void
printBDDTree(BDDNode * bdd, int *which_zoom)
{
   int y = 0, z, NUM = PRINT_TREE_WIDTH, level = 0, x, i;
   char aa[10];
   int tempint[100];
   for (x = 0; x < 100; x++)
      tempint[x] = 0;
   z = unravelBDDTree (y, tempint, bdd);

   //    if(z == -1) {
   //            fprintf(stdout, "\nTOO MANY NODES!!!\n\n");
   //            return;
   //    }
   y = 0;
   int reference = 0;
   int zoomarr[16];
	fprintf(stdout, "\n");
   for (x = 0; x < PRINT_TREE_WIDTH; x++)
      fprintf (stdout, "-");
   fprintf (stdout, "\n");
   for (x = 0; x < 16; x++)
      zoomarr[x] = 0;
   while ((y < z) && (y < 31))
   {
      for (i = 0; i < NUM / 2; i++)
         fprintf (stdout, " ");
      for (x = 0; x < (1 << level); x++)
      {
         if (tempint[y] == -1) {
				sprintf(aa, "T");
/*				fprintf (stdout, "T");
				if ((x + 1) < (1 << level))
				  for(i = 1; i < NUM; i++)
					 fprintf(stdout, " ");
*/
			}
         else if (tempint[y] == -2) {
				sprintf(aa, "F");
			} else if (tempint[y] == 0) {
				sprintf(aa, " ");
			} else if (y > 14) {
				sprintf(aa, "*%d", (*which_zoom)++);
            zoomarr[reference++] = y;
         } else {
				sprintf(aa, "%d", tempint[y]);
			}
			int l = strlen(aa);
//			fprintf(stdout, "(%d)", l);
//			if(l > 0 && ((x + 1) <= (1 << level))) {
				if(l%2 == 0) {
					for(i = 0; i < l/2-1; i++)
					  fprintf(stdout, "\b");
					fprintf (stdout, aa);
					if ((x + 1) < (1 << level))
					  for(i = l/2+1;i < NUM; i++)
						 fprintf(stdout, " ");
				} else {
					for(i = 0; i < l/2; i++)
					  fprintf(stdout, "\b");
					fprintf (stdout, aa);
					if ((x + 1) < (1 << level))
					  for(i = l/2+1;i < NUM; i++)
						 fprintf(stdout, " ");
				}
//			}
			y++;
		}
      level++;
      NUM /= 2;
      fprintf (stdout, "\n\n");
   }
   int now_zoom = *which_zoom;
   for (x = 0; (zoomarr[x] != 0) && (x < 16); x++)
   {
      fprintf (stdout, "\n*%d ", now_zoom - reference + x);
      printBDDTree (findBranch (0, zoomarr[x], bdd), which_zoom);
      fprintf (stdout, "\n");
   }
}

void
printITEBDD (BDDNode * bdd)
{
   if (bdd == true_ptr)
   {
      fprintf (stdout, "T");
      return;
   }
   if (bdd == false_ptr)
   {
      fprintf (stdout, "F");
      return;
   }
   fprintf (stdout, "ITE(%d, ", bdd->variable);
   printITEBDD (bdd->thenCase);
   fprintf (stdout, ", ");
   if (bdd->elseCase == true_ptr)
      fprintf (stdout, "T)");

   else if (bdd->elseCase == false_ptr)
      fprintf (stdout, "F)");

   else
   {
      printITEBDD (bdd->elseCase);
      fprintf (stdout, ")");
   }
}
void
printSchlipfCircuit ()
{
   fprintf (stdout, "val BDDCirc = [\n\n");
   for (int x = 0; x < nmbrFunctions; x++)
   {
      printITEBDD (functions[x]);
      fprintf (stdout, ",\n\n");
   }
   fprintf (stdout, "T];\n");
}
void
printCircuitTree ()
{
   if (nmbrFunctions == 0)
   {
      fprintf (stderr, "\nCircuit contains 0 constraints\n");
      return;
   }
   for (int i = 0; i < nmbrFunctions; i++)
   {
      int which_zoom = 0;
      fprintf (stdout, "Constraint #%d\n", i);
      printBDD (functions[i]);
      fprintf (stdout, "\n");
      printBDDTree (functions[i], &which_zoom);
      fprintf (stdout, "\n\n");
   }
}


//Use this function like this -
//printCircuit();
void
printCircuit ()
{
   if (nmbrFunctions == 0)
   {
      fprintf (stderr, "\nCircuit contains 0 constraint\n");
      return;
   }
   for (int i = 0; i < nmbrFunctions; i++)
   {
      printBDDfile(functions[i], stddbg);
      fprintf(stddbg, "\n");
   }
}
void
writeCircuit ()
{
   FILE * fout;
   if ((fout = fopen ("bdd.tmp", "wb+")) == NULL)
   { 
      fprintf(stderr, "Can't open bdd.tmp for writting\n");
      exit(1);
   };
   if (nmbrFunctions == 0)
   {
      fprintf (stderr, "\nCircuit contains 0 constraints\n");
      return;
   }
   for (int i = 0; i < nmbrFunctions; i++)
   {
      writeBDD (functions[i], fout);
      fprintf (fout, "\n");
   }
   fclose (fout);
}
void
readCircuit ()
{
   FILE * fin;
   char a[10];
   if ((fin = fopen ("bdd.tmp", "rb")) == NULL)
   { 
      fprintf(stderr, "Can't open bdd.tmp for reading\n");
      exit(1);
   };
   for (int i = 0; i < nmbrFunctions; i++)
   {
      functions[i] = readBDD (fin);
      fgets (a, 10, fin);
   }
   fclose (fin);
   unlink ("bdd.tmp");
}

#ifdef NO_BDD_MACROS
int top_variable (BDDNode * x, BDDNode * y, BDDNode * z)
{
   if (x->variable > y->variable)
   {
      if (x->variable > z->variable)
         return x->variable;
      return z->variable;
   }
   if (y->variable > z->variable)
      return y->variable;
   return z->variable;
}

BDDNode * reduce_t (int v, BDDNode * x) {
   if (x->variable == v)
	  return x->thenCase;
   return x;
}

BDDNode * reduce_f (int v, BDDNode * x) {
   if (x->variable == v)
	  return x->elseCase;
   return x;
}
#endif

int splitXors() {
	int total_vars = numinp;
	int *xor_vars = new int[numinp+1]; //This could be made better in the future.
	int *nonxor_vars = new int[numinp+1]; //This could be made better in the future.
	for(long x = 0; x < numout; x++) {
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
		xorFunctions[x] = ite_xor(xor_part, ite_var(s_ptr->id));
#ifndef NDEBUG
      BDDNode *savedF = functions[x];
#endif

		functions[x] = ite_equ(nonxor_part, ite_var(s_ptr->id));
      
		//functions[x] = ite_xor(nonxor_part, ite_var(s_ptr->id));
		//functions[x] = ite_equ(nonxor_part, false_ptr);

      length[x] = length[x]-y; /* the linear part is y */
      //assert(equalityVble[x] == 0);
      equalityVble[x] = s_ptr->id;

#ifndef NDEBUG
      BDDNode *testBDD = ite_and(xorFunctions[x], functions[x]);
      testBDD = xquantify(testBDD, s_ptr->id);
      if (testBDD != savedF) {
         assert(0);
      }
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

BDDNode *xddxor(BDDNode *x, BDDNode *y) {
   BDDNode *cached = itetable_find_or_add_node(9, x, y, NULL);
   if (cached) return cached;

	if(x == false_ptr) return y;
	if(y == false_ptr) return x;
	if(x == true_ptr && y == true_ptr) return false_ptr;
	int v = (x->variable > y->variable) ? x->variable : y->variable;
	BDDNode *r = xddxor(xdd_reduce_t(v, x), xdd_reduce_t(v, y));
	BDDNode *e = xddxor(xdd_reduce_f(v, x), xdd_reduce_f(v, y));
	if(r == false_ptr) return itetable_add_node(9, x, y, e);
	//return find_or_add_node(v, r, e);
	return itetable_add_node(9, x, y, find_or_add_node(v, r, e));
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
	if (x == true_ptr) return true_ptr;
   if (x == false_ptr) return false_ptr;

   if (y == true_ptr) return true_ptr;
   if (y == false_ptr) return false_ptr;

   BDDNode * r;
   BDDNode * e;

   if (x->variable > y->variable) {
      r = constant_and(x->thenCase, y);
      if(r != false_ptr) return true_ptr;
		e = constant_and(x->elseCase, y);
		if(e != false_ptr) return true_ptr;
   } else if (x->variable == y->variable) {
      if (x == y) return true_ptr;
      else if (x->notCase == y) return false_ptr;
      else {
         r = constant_and(x->thenCase, y->thenCase);
			if(r != false_ptr) return true_ptr;
         e = constant_and(x->elseCase, y->elseCase);
			if(e != false_ptr) return true_ptr;
      }
   } else {
      r = constant_and(x, y->thenCase);
		if(r != false_ptr) return true_ptr;
      e = constant_and(x, y->elseCase);
		if(e != false_ptr) return true_ptr;
   } 

	if(r == e) return r;
	return true_ptr;
}

inline BDDNode *_and_dot(BDDNode *x, BDDNode *y);

inline
BDDNode *and_dot(BDDNode *x, BDDNode *y)
{
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














inline BDDNode *_ite_x_y_F(BDDNode *x, BDDNode *y);

inline
BDDNode *ite_x_y_F(BDDNode *x, BDDNode *y)
{
   if (y == true_ptr) return x;
   if (y == false_ptr) return false_ptr;
   return _ite_x_y_F(x, y);
}

inline
BDDNode *_ite_x_y_F(BDDNode *x, BDDNode *y)
{
   if (x == true_ptr) return y;
   if (x == false_ptr) return false_ptr;

   int v;
   BDDNode * r;
   BDDNode * e;

   if (x->variable > y->variable) {
      BDDNode *cached = itetable_find_or_add_node(1, x, y, NULL);
      if (cached) return cached;
      v = x->variable;
      r = _ite_x_y_F(x->thenCase, y);
      e = _ite_x_y_F(x->elseCase, y);
		// this happens but not often enough to bring a speedup
		if (r == x->thenCase && e == x->elseCase) return x;
   } else if (x->variable == y->variable) {
      if (x == y) return x;
      else if (x->notCase == y) return false_ptr;
      else {
         BDDNode *cached = itetable_find_or_add_node(1, x, y, NULL);
         if (cached) return cached;
         v = x->variable;
         if (y->thenCase == true_ptr) r=x->thenCase;
         else if (y->thenCase == false_ptr) r=false_ptr;
         else r = _ite_x_y_F(x->thenCase, y->thenCase);
         if (y->elseCase == true_ptr) e=x->elseCase;
         else if (y->elseCase == false_ptr) e=false_ptr;
         else e = _ite_x_y_F(x->elseCase, y->elseCase);
			// this happens but not often enough to bring a speedup
			if (r == x->thenCase && e == x->elseCase) return x;
			if (r == y->thenCase && e == y->elseCase) return y;
      }
   } else {
      BDDNode *cached = itetable_find_or_add_node(1, y, x, NULL);
      if (cached) return cached;
      v = y->variable;
      r = _ite_x_y_F(y->thenCase, x);
      e = _ite_x_y_F(y->elseCase, x);
		// this happens but not often enough to bring a speedup
		if (r == y->thenCase && e == y->elseCase) return y;
   } 

   if (r == e) return (r);
   return itetable_add_node(1, x, y, find_or_add_node(v, r, e));
   return find_or_add_node(v, r, e);
}

inline
BDDNode *OLD_ite_x_y_F(BDDNode *x, BDDNode *y)
{
   if (x == true_ptr) return y;
   if (x == false_ptr) return false_ptr;

   if (y == true_ptr) return x;
   if (y == false_ptr) return false_ptr;
  
   int v;
   BDDNode * r;
   BDDNode * e;

   if (x->variable > y->variable) {
      v = x->variable;
      r = ite_x_y_F(x->thenCase, y);
      e = ite_x_y_F(x->elseCase, y);
   } else if (x->variable == y->variable) {
      if (x == y) return x;
      else if (x->notCase == y) return false_ptr;
      else {
         v = x->variable;
         r = ite_x_y_F(x->thenCase, y->thenCase);
         e = ite_x_y_F(x->elseCase, y->elseCase);
      }
   } else {
      v = y->variable;
      r = ite_x_y_F(x, y->thenCase);
      e = ite_x_y_F(x, y->elseCase);
   } 

   if (r == e) return (r);
   // this happens but not often enough to bring a speedup
   //if (r == x->thenCase && e == x->elseCase && v==x->variable) return x; 
   //if (r == y->thenCase && e == y->elseCase && v==y->variable) return y;
   return find_or_add_node(v, r, e);
}

inline
BDDNode *_ite_x_y_T(BDDNode *x, BDDNode *y)
{
   if (x == true_ptr) return y;
   if (x == false_ptr) return true_ptr;

   if (y == true_ptr) return true_ptr;
   if (y == false_ptr) return ite_not(x);
  
   BDDNode *cached = itetable_find_or_add_node(2, x, y, NULL);
   if (cached) return cached;

   int v;
   BDDNode * r;
   BDDNode * e;

   if (x->variable > y->variable) {
      v = x->variable;
      r = _ite_x_y_T(x->thenCase, y);
      e = _ite_x_y_T(x->elseCase, y);
   } else if (x->variable == y->variable) {
      if (x == y) return true_ptr;
      else if (x->notCase == y) return y;
      else {
         v = x->variable;
         r = _ite_x_y_T(x->thenCase, y->thenCase);
         e = _ite_x_y_T(x->elseCase, y->elseCase);
      }
   } else {
      v = y->variable;
      r = _ite_x_y_T(x, y->thenCase);
      e = _ite_x_y_T(x, y->elseCase);
   } 

   if (r == e) return (r);
   return itetable_add_node(2, x, y, find_or_add_node(v, r, e));
   return find_or_add_node(v, r, e);
}

inline
BDDNode *_ite_x_T_z(BDDNode *x, BDDNode *z)
{
   if (x == true_ptr) return true_ptr;
   if (x == false_ptr) return z;

   if (z == true_ptr) return true_ptr;
   if (z == false_ptr) return x;
  
   BDDNode *cached = itetable_find_or_add_node(3, x, z, NULL);
   if (cached) return cached;

   int v;
   BDDNode * r;
   BDDNode * e;

   if (x->variable > z->variable) {
      v = x->variable;
      r = _ite_x_T_z(x->thenCase, z);
      e = _ite_x_T_z(x->elseCase, z);
   } else if (x->variable == z->variable) {
      if (x == z) return x;
      else if (x->notCase == z) return true_ptr;
      else {
         v = x->variable;
         r = _ite_x_T_z(x->thenCase, z->thenCase);
         e = _ite_x_T_z(x->elseCase, z->elseCase);
      }
   } else {
      v = z->variable;
      r = _ite_x_T_z(x, z->thenCase);
      e = _ite_x_T_z(x, z->elseCase);
   } 

   if (r == e) return (r);
   return itetable_add_node(3, x, z, find_or_add_node(v, r, e));
   return find_or_add_node(v, r, e);
}

inline
BDDNode *_ite_x_F_z(BDDNode *x, BDDNode *z)
{
   if (x == true_ptr) return false_ptr;
   if (x == false_ptr) return z;

   if (z == true_ptr) return ite_not(x);
   if (z == false_ptr) return false_ptr;
  
   int v;
   BDDNode * r;
   BDDNode * e;

   if (x->variable > z->variable) {
      v = x->variable;
      r = _ite_x_F_z(x->thenCase, z);
      e = _ite_x_F_z(x->elseCase, z);
   } else if (x->variable == z->variable) {
      if (x == z) return false_ptr;
      else if (x->notCase == z) return ite_not(x);
      else {
         v = x->variable;
         r = _ite_x_F_z(x->thenCase, z->thenCase);
         e = _ite_x_F_z(x->elseCase, z->elseCase);
      }
   } else {
      v = z->variable;
      r = _ite_x_F_z(x, z->thenCase);
      e = _ite_x_F_z(x, z->elseCase);
   } 

   if (r == e) return (r);
   return find_or_add_node(v, r, e);
}

inline
BDDNode *ite_x_y_ny(BDDNode *x, BDDNode *y)
{
   if (x == true_ptr) return y;
   if (x == false_ptr) return ite_not(y);

   if (y == true_ptr) return x;
   if (y == false_ptr) return ite_not(x);
  
//   BDDNode *cached = itetable_find_or_add_node(5, x, y, NULL);
//   if (cached) return cached;
   
   int v;
   BDDNode * r;
   BDDNode * e;

   if (x->variable > y->variable) {
      v = x->variable;
      r = ite_x_y_ny(x->thenCase, y);
      e = ite_x_y_ny(x->elseCase, y);
   } else if (x->variable == y->variable) {
      if (x == y) return true_ptr;
      else if (x->notCase == y) return false_ptr;
      else {
         v = x->variable;
         r = ite_x_y_ny(x->thenCase, y->thenCase);
         e = ite_x_y_ny(x->elseCase, y->elseCase);
      }
   } else {
      v = y->variable;
      r = ite_x_y_ny(x, y->thenCase);
      e = ite_x_y_ny(x, y->elseCase);
   } 

   if (r == e) return (r);
//   return itetable_add_node(5, x, z, find_or_add_node(v, r, e));
   return find_or_add_node(v, r, e);
}

BDDNode *ite(BDDNode * x, BDDNode * y, BDDNode * z) {
   if (x == true_ptr) return y;
   if (x == false_ptr) return z;

   if(y == true_ptr) {
      if(z == false_ptr) return x;
      if(z == true_ptr) return true_ptr;
      return ite_x_T_z(x, z);
   } else if(y == false_ptr) {
      if(z == true_ptr) return ite_not(x);
      if(z == false_ptr) return false_ptr;
      return ite_x_F_z(x, z);
	}

   if (z == true_ptr) {
      return ite_x_y_T(x, y);
   } else if (z == false_ptr) {
      return ite_x_y_F(x, y);
   }
	//int v = top_variable(x, y, z);
   int v;
   BDDNode * r;
   BDDNode * e;
	
	if (x->variable > y->variable) {
      if (x->variable > z->variable) {
			if(y == z) return y;
         v = x->variable;
         if (y->notCase == z) {
            r = ite_x_y_ny(x->thenCase, y);
            e = ite_x_y_ny(x->elseCase, y);
         } else {
            r = ite(x->thenCase, y, z); // <----
            e = ite(x->elseCase, y, z); // <----
         }
      } else if (x->variable == z->variable) {
         v = x->variable;
			if(x == z) {
				r = ite_x_y_F(x->thenCase, y);
				e = ite_x_y_F(x->elseCase, y);
         } else if (x->notCase == z) {
				r = ite_x_y_T(x->thenCase, y);
				e = ite_x_y_T(x->elseCase, y);
			} else {			
				r = ite(x->thenCase, y, z->thenCase); // <----
				e = ite(x->elseCase, y, z->elseCase); // <----
			}
      } else {
         v = z->variable;
			if(x == y) {
            r = ite_x_T_z(z->thenCase, x);
				e = ite_x_T_z(z->elseCase, x);
         } else if (x->notCase == y) {
            r = ite_x_F_z(x, z->thenCase);
				e = ite_x_F_z(x, z->elseCase);
			} else {
				r = ite(x, y, z->thenCase); // <----
				e = ite(x, y, z->elseCase); // <----
			}
      }
   } else if (y->variable < z->variable) {
		v = z->variable;
		if(x == y) {
			r = ite_x_T_z(z->thenCase, x);
			e = ite_x_T_z(z->elseCase, x);
      } else if (x->notCase == y) {
         r = ite_x_F_z(x, z->thenCase);
         e = ite_x_F_z(x, z->elseCase);
		} else {		
			r = ite(x, y, z->thenCase); // <----
			e = ite(x, y, z->elseCase); // <----
		}
   } else if (x->variable == z->variable) {
      if (y->variable == x->variable) {
			if(y == z) return y;
			v = x->variable;
         if (y->notCase == z) {
            r = ite_x_y_ny(x->thenCase, y->thenCase);
            e = ite_x_y_ny(x->elseCase, y->elseCase);
         } else if(x == y) {
				r = ite_x_T_z(x->thenCase, z->thenCase);
				e = ite_x_T_z(x->elseCase, z->elseCase);
         } else if (x->notCase == y) {
            r = ite_x_F_z(x->thenCase, z->thenCase);
            e = ite_x_F_z(x->elseCase, z->elseCase);
			} else if(x == z) {
				r = ite_x_y_F(x->thenCase, y->thenCase);
				e = ite_x_y_F(x->elseCase, y->elseCase);
         } else if (x->notCase == z) {
            r = ite_x_y_T(x->thenCase, y->thenCase);
            e = ite_x_y_T(x->elseCase, y->elseCase);
			} else {
				r = ite(x->thenCase, y->thenCase, z->thenCase); // <----
				e = ite(x->elseCase, y->elseCase, z->elseCase); // <----
			}
      } else {
			v = y->variable;
			if(x == z) {
				r = ite_x_y_F(y->thenCase, x);
				e = ite_x_y_F(y->elseCase, x);
         } else if (x->notCase == z) {
            r = ite_x_y_T(x, y->thenCase);
            e = ite_x_y_T(x, y->elseCase);
			} else {
				r = ite(x, y->thenCase, z); // <----
				e = ite(x, y->elseCase, z); // <----
			}
      }
   } else {
      if (y->variable == x->variable) {
			v = x->variable;
			if(x == y) {
				r = ite_x_T_z(x->thenCase, z);
				e = ite_x_T_z(x->elseCase, z);
         } else if (x->notCase == y) {
            r = ite_x_F_z(x->thenCase, z);
            e = ite_x_F_z(x->elseCase, z);
			} else {			
				r = ite(x->thenCase, y->thenCase, z); // <----
				e = ite(x->elseCase, y->elseCase, z); // <----
			}
      } else if (y->variable == z->variable) {
			if(y == z) return y;
         v = y->variable;		
         if (y->notCase == z) {
            r = ite_x_y_ny(x, y->thenCase);
            e = ite_x_y_ny(x, y->elseCase);
         } else {
            r = ite(x, y->thenCase, z->thenCase); // <----
            e = ite(x, y->elseCase, z->elseCase); // <----
         }
      } else {
			v = y->variable;
			if(x == z) {
				r = ite_x_y_F(y->thenCase, x);
				e = ite_x_y_F(y->elseCase, x);
         } else if (x->notCase == z) {
            r = ite_x_y_T(x, y->thenCase);
            e = ite_x_y_T(x, y->elseCase);
			} else {
				r = ite(x, y->thenCase, z); // <----
				e = ite(x, y->elseCase, z); // <----
			}
      }
   }

   if (r == e) return (r);
   return find_or_add_node(v, r, e);
}

#ifdef NO_BDD_MACROS
/*
BDDNode * ite_not(BDDNode * a)
{
   //return ite (a, false_ptr, true_ptr);
   return ite_x_F_T(a);
}
*/
BDDNode * ite_or_te(BDDNode * a)
{
   if (a->or_bdd != NULL) return a->or_bdd;
   return (a->or_bdd = ite_or(a->thenCase, a->elseCase));
}

BDDNode * ite_or (BDDNode * a, BDDNode * b)
{
#ifdef SORT_ITE_OPS
   if (a->variable < b->variable)
      return ite_x_T_z(b, /*true_ptr,*/ a);
   else
#endif
      return ite_x_T_z(a, /*true_ptr,*/ b);
}

BDDNode * ite_nor(BDDNode * a, BDDNode * b)
{
   return ite_not(ite_or (a, b));
}

BDDNode * ite_imp (BDDNode * a, BDDNode * b)
{
   return ite_x_y_T(a, b);//, true_ptr);
}

BDDNode * ite_nimp(BDDNode * a, BDDNode * b)
{
   return ite_not(ite_imp(a, b));
}

BDDNode * ite_xor(BDDNode * a, BDDNode * b)
{
#ifdef SORT_ITE_OPS
   if (a->variable < b->variable)
      return ite_x_y_ny(b, ite_not (a));//, a);
   else
#endif
   return ite_x_y_ny(a, ite_not (b));//, b);
}

BDDNode * ite_equ(BDDNode * a, BDDNode * b)
{
   return ite_not(ite_xor (a, b));
}

BDDNode * ite_and(BDDNode * a, BDDNode * b)
{
#ifdef SORT_ITE_OPS
   if (a->variable < b->variable)
      return ite_x_y_F(b, a); //, false_ptr);
   else
#endif
   return ite_x_y_F(a, b); //, false_ptr);
}

BDDNode * ite_nand (BDDNode * a, BDDNode * b)
{
#ifdef SORT_ITE_OPS
   if (a->variable < b->variable)
      return ite_not (ite_and (b, a));
   else
#endif
   return ite_not (ite_and (a, b));
}

BDDNode * ite_itequ (BDDNode * a, BDDNode * b, BDDNode * c, BDDNode * d) 
{
   return ite_equ (d, ite (a, b, c));
}


/***********************************************************************/ 
BDDNode * ite_var (int v)
{
   if (v < 0)
      return find_or_add_node (-v, false_ptr, true_ptr);
	else if (v > 0)
	  return find_or_add_node (v, true_ptr, false_ptr);
	else return false_ptr;
}

#endif // NO_BDD_MACROS


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
	long y = 0;
   long tempint_max = 0;
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
	
	if(length1 > 0) delete []vars1;
	if(length2 > 0) delete []vars2;
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
      return pruning_p2 (f, ite_or (c->thenCase, c->elseCase));	//xquantify(c, c->variable));
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
	while(bdd != false_ptr) {
		if(bdd->thenCase == true_ptr)
		  bdd = bdd->elseCase;
		if(bdd->elseCase == true_ptr)
		  bdd = bdd->thenCase;
		else return 0;
	}
	return 1;
}

int
OLD_nmbrVarsInCommon (int bddNmbr1, int bddNmbr2, int stopat)
{
   int bdd1pos = 0;
   int bdd2pos = 0;
   int varsSoFar = 0;
   int bdd1max = length[bddNmbr1] - stopat + 1;
   int bdd2max = length[bddNmbr2] - stopat + 1;

   if (bdd1max <= 0 || bdd2max <= 0) return 0;

   while (1)
   {
      int bdd1Var = variables[bddNmbr1].num[bdd1pos];
      int bdd2Var = variables[bddNmbr2].num[bdd2pos];
      if (bdd1Var < bdd2Var) {
         bdd1pos++;
         if (bdd1pos == bdd1max) return varsSoFar;
      }
      else 
         if (bdd1Var > bdd2Var) {
            bdd2pos++;
            if (bdd2pos == bdd2max) return varsSoFar;
         }
         else 
         {
            // assert(bdd1Var == bdd2Var);
            varsSoFar++;
            stopat--;
            if (stopat == 0) return varsSoFar;

            bdd1max++;
            bdd1pos++;
            if (bdd1pos == bdd1max) return varsSoFar;

            bdd2max++;
            bdd2pos++;
            if (bdd2pos == bdd2max) return varsSoFar;
         }
   }
   //return varsSoFar;
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
	if(length[bddNmbr1] == 0) return 0;
	if(length[bddNmbr2] == 0) return 0;
   bdd1max = length[bddNmbr1] - STOPAT + 1;
   bdd2max = length[bddNmbr2] - STOPAT + 1;

   //fprintf(stdout, "(%d)%d - %d\n", bddNmbr1, length[bddNmbr1], length[bddNmbr2]);

   while (1)
   {
      while (variables[bddNmbr1].num[bdd1pos] < variables[bddNmbr2].num[bdd2pos]) {
         if (++bdd1pos == bdd1max) return 0; 
      }

      while (variables[bddNmbr1].num[bdd1pos] > variables[bddNmbr2].num[bdd2pos]) {
         if (++bdd2pos == bdd2max) return 0; 
      }

      if (variables[bddNmbr1].num[bdd1pos] == variables[bddNmbr2].num[bdd2pos])
      {
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

/*
 CircuitStruct * split (CircuitStruct * Split_ircuit, int num, int torf)
 {
 CircuitStruct * NewCircuit = new CircuitStruct;
 NewCircuit->functions = new BDDNode *[SplitCircuit->nmbrFunctions + 1];
 NewCircuit->nmbrFunctions = SplitCircuit->nmbrFunctions;
 for (int i = 0; i < SplitCircuit->nmbrFunctions; i++)
 {
 NewCircuit->functions[i] =
 set_variable (SplitCircuit->functions[i], num, torf);
 }

//Need to remove any constraint that was set to True during the making of the BDDs
int count = -1;
int nmbrfuncts = NewCircuit->nmbrFunctions;
for (long x = 0; x < nmbrfuncts; x++)
{
count++;
NewCircuit->functions[count] = NewCircuit->functions[x];
if (NewCircuit->functions[x] == true_ptr)
count--;
if (NewCircuit->functions[x] == false_ptr)
{
fprintf (stderr, "\nFormula is UNSATISFIABLE\n");
NewCircuit->nmbrFunctions = 0;
NewCircuit->functions[0] = false_ptr;
return NewCircuit;
}
}
NewCircuit->nmbrFunctions = count + 1;
return NewCircuit;
}
*/

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
      }
	}
	xor_vars[xor_vars_idx] = 0;
	nonxor_vars[nonxor_vars_idx] = 0;
}

void OLD_unravelBDD (long *y, int tempint[5000], BDDNode * func) {
   if ((func == true_ptr) || (func == false_ptr))
	  return;
   tempint[*y] = func->variable;
   if ((*y) >= 4999) {
      //Sort and remove duplicates
      qsort (tempint, *y, sizeof (int), compfunc);
      int v = 0;
      for (int i = 1; i < (*y) + 1; i++) {
         v++;
         if (tempint[i] == tempint[i - 1])
			  v--;
         tempint[v] = tempint[i];
      }
      (*y) = v + 1;
      if (*y >= 4999) {
         fprintf(stderr, "Tooo big\n");
         exit(1);
      }
		
      //End sorting and duplicates
   } else (*y)++;
   OLD_unravelBDD (y, tempint, func->thenCase);
   OLD_unravelBDD (y, tempint, func->elseCase);
}

void NEW_unravelBDD(long *y, long *max, int **tempint, BDDNode * func);

void unravelBDD(long *y, long *max, int **tempint, BDDNode * func) {
  *y=0;
  // assert (no flag is set );
  start_bdd_flag_number(UNRAVELBDD_FLAG_NUMBER);
  NEW_unravelBDD(y, max, tempint, func);
  for (int i = 0;i<*y;i++) {
     // clear the flag
    sym_reset_flag((*tempint)[i]);
  }
}

void NEW_unravelBDD(long *y, long *max, int **tempint, BDDNode * func) {
   if (func->flag == bdd_flag_number) return;
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
	NEW_unravelBDD (y, max, tempint, func->thenCase);
	NEW_unravelBDD (y, max, tempint, func->elseCase);
}

BDDNode * _set_variable (BDDNode * f, int num, int torf);

BDDNode * set_variable_all_infs(BDDNode *f) 
{
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

void set_variable_all(llist * k, int num, int torf) 
{
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


BDDNode * _set_variable (BDDNode * f, int num, int torf)
{
	if (f->variable < num)
      return f;
   if (f->flag == bdd_flag_number) {
      return f->tmp_bdd;
   }
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

BDDNode * _num_replace (BDDNode * f, int var, int replace);

void num_replace_all(llist *k, int var, int replace) 
{
   start_bdd_flag_number(NUMREPLACEALL_FLAG_NUMBER);
   while(k != NULL) {
      functions[k->num] = _num_replace(functions[k->num], var, replace);
      k = k->next; 
   }
}

BDDNode * num_replace (BDDNode * f, int var, int replace) {
   start_bdd_flag_number(NUMREPLACE_FLAG_NUMBER);
   return _num_replace(f, var, replace);
}

BDDNode * _num_replace (BDDNode * f, int var, int replace) {
	if (var > f->variable)
	  return f;
   if (f->flag == bdd_flag_number) return f->tmp_bdd;
   f->flag = bdd_flag_number;
	if (var == f->variable) {
		if (replace > 0) {
			return (f->tmp_bdd = ite(ite_var(replace), f->thenCase, f->elseCase));
			BDDNode *itevar = ite_var(replace);
         BDDNode *cached = itetable_find_or_add_node(11, itevar, f, NULL);
         if (cached) return (f->tmp_bdd = cached);
         return (f->tmp_bdd = itetable_add_node(11, itevar, f, 
                  ite(itevar, f->thenCase, f->elseCase)));
      } else if (replace < 0) {
         return (f->tmp_bdd = ite(ite_var(-replace), f->elseCase, f->thenCase));
			BDDNode *itevar = ite_var(-replace);
         BDDNode *cached = itetable_find_or_add_node(11, itevar, f, NULL);
         if (cached) return (f->tmp_bdd = cached);
         return (f->tmp_bdd = itetable_add_node(11, itevar, f,
                  ite(itevar, f->elseCase, f->thenCase)));
      }
   }
   BDDNode * r = _num_replace (f->thenCase, var, replace);
   BDDNode * e = _num_replace (f->elseCase, var, replace);
   if (r == e)
	  return (f->tmp_bdd = r);
	//return ite (ite_var (f->variable), r, e);

	return (f->tmp_bdd = find_or_add_node(f->variable, r, e));
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
}

infer *_possible_infer_x(BDDNode *f, int x) {
	if(f->variable < x) return NULL; //This could happen on a path that doesn't involve x

   if(f->flag == bdd_flag_number) return copy_infer(f->tmp_infer);
   f->flag = bdd_flag_number;
	
	if(f->variable == x) {
		if(f->thenCase == true_ptr || f->elseCase == false_ptr) {
			infer *inference = new infer;
			inference->nums[0] = x;
			inference->nums[1] = 0;
			inference->next = NULL;
			f->tmp_infer = copy_infer(inference);
			return inference;
		} else if(f->elseCase == true_ptr || f->thenCase == false_ptr) {
			infer *inference = new infer;
			inference->nums[0] = -x;
			inference->nums[1] = 0;
			inference->next = NULL;
			f->tmp_infer = copy_infer(inference);
			return inference;
		} else {
			BDDNode *r_BDD;
			if(f->t_and_not_e_bdd != NULL) r_BDD = f->t_and_not_e_bdd;
			else r_BDD = and_dot(f->thenCase, ite_not(f->elseCase));
			                 //Ex_GetInfer(f->thenCase);
			if(r_BDD == false_ptr) {				  
				infer *inference = new infer;
				inference->nums[0] = -x;
				inference->nums[1] = 0;
				inference->next = NULL;
				f->tmp_infer = copy_infer(inference);
				return inference;
			}
			infer *head = NULL;
			infer *temp = NULL;
			infer *r = r_BDD->inferences;
			if(r == NULL) {
				head = new infer;
				head->nums[0] = 0;
				head->nums[1] = 0;
				head->next = NULL;
				f->tmp_infer = copy_infer(head);
				return head;
			}
			BDDNode *e_BDD;
			if(f->not_t_and_e_bdd != NULL) e_BDD = f->not_t_and_e_bdd;
			else e_BDD = and_dot(f->elseCase, ite_not(f->thenCase));
			                 //Ex_GetInfer(f->elseCase);
			if(e_BDD == false_ptr) {				  
				infer *inference = new infer;
				inference->nums[0] = x;
				inference->nums[1] = 0;
				inference->next = NULL;
				f->tmp_infer = copy_infer(inference);
				return inference;
			}
			infer *e = e_BDD->inferences;
			if(e == NULL) {
				//while (r!=NULL) { temp = r; r = r->next; delete temp;	}
				head = new infer;
				head->nums[0] = 0;
				head->nums[1] = 0;
				head->next = NULL;
				f->tmp_infer = copy_infer(head);
				return head;
			}
			
			for(infer *r_iter=r; r_iter!=NULL; r_iter=r_iter->next) {
				if(r_iter->nums[1] == 0) {
					for(infer *e_iter=e; e_iter!=NULL; e_iter=e_iter->next) {
						if(e_iter->nums[1] == 0) {
							if(r_iter->nums[0] == -e_iter->nums[0]) {
								temp = head;
								head = new infer;
								head->nums[0] = x;
								head->nums[1] = r_iter->nums[0];
								head->next = temp;
								//fprintf(stderr, "[%d, %d]", x, r_iter->nums[0]);
							}
						}
					}
				}
			}

			//while (r!=NULL) { temp = r; r = r->next; delete temp;	}
			//while (e!=NULL) { temp = e; e = e->next; delete temp;	}
			
			if(head == NULL) {
				head = new infer;
				head->nums[0] = 0;
				head->nums[1] = 0;
				head->next = NULL;
//			} else {
//				int w = 0;
//				printBDDTree(f, &w);
			}
			
			f->tmp_infer = copy_infer(head);
			return head;
		}
	} else {
		infer *r = _possible_infer_x(f->thenCase, x);
		if(r != NULL) 
		  if(r->nums[0] == 0){
			  f->tmp_infer = copy_infer(r);
			  return r;
		  }
		
		infer *e = _possible_infer_x(f->elseCase, x);
		if(e != NULL) {
			if(e->nums[0] == 0) {
				while (r!=NULL) { infer *temp = r; r = r->next; delete temp; }
				f->tmp_infer = copy_infer(e);
				return e;
			}
		}
		
		if(r == NULL) { f->tmp_infer = copy_infer(e); return e; }
		if(e == NULL) { f->tmp_infer = copy_infer(r); return r; }
		//If both are NULL, NULL is returned;
		
		if((r->nums[1] == 0 && e->nums[1] != 0) ||
			(r->nums[1] == 0 && e->nums[1] == 0 && r->nums[0] != abs(e->nums[0]))) {
			infer *head = new infer;
			head->nums[0] = 0;
			head->nums[1] = 0;
			head->next = NULL;
			while (r!=NULL) { infer *temp = r; r = r->next; delete temp; }
			while (e!=NULL) { infer *temp = e; e = e->next; delete temp; }
			f->tmp_infer = copy_infer(head);
			return head;
		} else if(r->nums[1] == 0 && e->nums[1] == 0 && r->nums[0] == e->nums[0]) {
			//Both sides are same inference, return r;
			while (e!=NULL) { infer *temp = e; e = e->next; delete temp; }
			f->tmp_infer = copy_infer(r);
			return r;
		} else if(r->nums[1] == 0 && e->nums[1] == 0 && r->nums[0] == -e->nums[0]) {
			//Sides are opposite inference, return {f->variable, r->nums[1]};
			r->nums[1] = r->nums[0]; //r->nums[1] = -e->nums[0];
			r->nums[0] = f->variable;
			while (e!=NULL) { infer *temp = e; e = e->next; delete temp; }
			f->tmp_infer = copy_infer(r);
			return r;
		}
		
		//They share equivalences of x = {y, -y}
		//or y = {x, -x}
		//return all matching equivalences

		infer *head = NULL;
		for(infer *r_iter = r; r_iter != NULL; r_iter=r_iter->next) {
			for(infer *e_iter = e; e_iter != NULL; e_iter=e_iter->next) {
				if(r_iter->nums[0] == e_iter->nums[0] && r_iter->nums[1] == e_iter->nums[1]) {
					infer *temp = head;
					head = new infer;
					head->nums[0] = r_iter->nums[0];
					head->nums[1] = r_iter->nums[1];
					head->next = temp;
					break;
				}
			}
		}
		if(head == NULL) {
			head = new infer;
			head->nums[0] = 0;
			head->nums[1] = 0;
			head->next = NULL;
		}
		infer *temp;
		while (r!=NULL) { temp = r; r = r->next; delete temp;	}
		while (e!=NULL) { temp = e; e = e->next; delete temp; }
		f->tmp_infer = copy_infer(head);
		return head;
	}	
}

void clean_possible_infer_x(BDDNode *f, int x) {
	if(f->variable < x) return;
   if(f->flag == bdd_flag_number) return;
   f->flag = bdd_flag_number;
	while (f->tmp_infer!=NULL) { infer *temp = f->tmp_infer; f->tmp_infer = f->tmp_infer->next; delete temp; }
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
   long tempint_max=0;
   long y, i;
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

