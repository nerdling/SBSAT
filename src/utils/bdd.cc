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

/***********************************************************************
 *  bdd.c (S.Weaver)
 ***********************************************************************/ 

#include "ite.h"

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

int countnodes(BDDNode *f) {
   if(IS_TRUE_FALSE(f))
      return 0;
   return (countnodes(f->thenCase) + countnodes(f->elseCase));	
}

BDDNode *MinMaxBDD(int *vars, int min, int max, int num_left, int set_true) {
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


void unmark (BDDNode * bdd)
{
   if (IS_TRUE_FALSE(bdd))
      return;

   //   if(bdd->t_var == 0) return; //Slower or faster?
   bdd->t_var = 0;
   unmark (bdd->thenCase);
   unmark (bdd->elseCase);
   return;
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
printBDDfile (BDDNode * bdd, FILE * fout)
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
   printBDD (bdd->thenCase);
   //fprintf (stdout, "[%d]", bdd->variable);
	fprintf(fout, "[%s", s_name(bdd->variable));
	d4_printf2("(%d)", bdd->variable);
	fprintf(fout, "]");
   printBDD (bdd->elseCase);
   fprintf (fout, ")");
}
void printBDD (BDDNode * bdd) { 
	printBDDfile(bdd, stddbg);
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

BDDNode * findBranch (int y, int z, BDDNode * func) 
{
   if (y == z)
      return func;
   if (IS_TRUE_FALSE(func))
      return func;
   BDDNode * r = findBranch (y * 2 + 1, z, func->thenCase);
   BDDNode * e = findBranch (y * 2 + 2, z, func->elseCase);
   if (r->variable > e->variable)
      return r;
   return e;
}


// Top level call should have *which_zoom equal to zero.
void
printBDDTree (BDDNode * bdd, int *which_zoom)
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
      printBDD (functions[i]);
      fprintf (stdout, "\n");
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
		if(length[x] <= PLAINXOR_LIMIT || functionType[x]!=UNSURE) continue; //???
		BDDNode *xdd = bdd2xdd(functions[x]);		
		countSingleXors(xdd, xor_vars, nonxor_vars);
		d4_printf2("\n%ld: ", x);
      /*
      D_4(
         printBDDfile(xdd, stddbg);
      )
      */
		BDDNode *ands = true_ptr;
		BDDNode *xor_part = false_ptr;
		int y = 0;
	   d4_printf2("%ld: non-linear: ", x);
		for(y=0; nonxor_vars[y]!=0; y++) {
			d4_printf2("%d ", nonxor_vars[y]);
		}
	   d4_printf2("\n%ld linear: ", x);
		for(y=0; xor_vars[y]!=0; y++) {
			d4_printf2("%d ", xor_vars[y]);
			ands = ite_and(ands, ite_var(-xor_vars[y]));
			xor_part = ite_xor(xor_part, ite_var(xor_vars[y]));
		}
		if(y < 3) continue;
      
		//if(IS_TRUE_FALSE(xor_part)) continue; /* unnecessary */
		BDDNode *nonxor_part = gcf(functions[x], ands);
      D_4(
         //d4_printf1("\n");
         //printBDDfile(functions[x], stddbg);
         d4_printf1("\n");
         printBDDfile(nonxor_part, stddbg);
         d4_printf1("\n");
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
	if(x == false_ptr) return y;
	if(y == false_ptr) return x;
	if(x == true_ptr && y == true_ptr) return false_ptr;
	int v = (x->variable > y->variable) ? x->variable : y->variable;
	BDDNode *r = xddxor(xdd_reduce_t(v, x), xdd_reduce_t(v, y));
	BDDNode *e = xddxor(xdd_reduce_f(v, x), xdd_reduce_f(v, y));
	if(r == false_ptr) return e;
	return find_or_add_node(v, r, e);
}

BDDNode *bdd2xdd(BDDNode *x) {
	if(x == false_ptr) return false_ptr;
	if(x == true_ptr) return true_ptr;
	int v = x->variable;
	BDDNode *r = bdd2xdd(xdd_reduce_t(v, x));
	BDDNode *e = bdd2xdd(xdd_reduce_f(v, x));
	r = xddxor(r, e);
	if(r == false_ptr) return e;
	return find_or_add_node(v, r, e);
}

BDDNode *ite (BDDNode * x, BDDNode * y, BDDNode * z) {
   if (x == true_ptr)
      return y;
   if (x == false_ptr)
      return z;
   int v = top_variable (x, y, z);
   BDDNode * r = ite (reduce_t (v, x), reduce_t (v, y), reduce_t (v, z));
   BDDNode * e = ite (reduce_f (v, x), reduce_f (v, y), reduce_f (v, z));
   if (r == e)
      return (r);
   return find_or_add_node (v, r, e);
}

#ifdef NO_BDD_MACROS
BDDNode * ite_not (BDDNode * a)
{
   return ite (a, false_ptr, true_ptr);
}

BDDNode * ite_or (BDDNode * a, BDDNode * b)
{
   return ite (a, true_ptr, b);
}

BDDNode * ite_nor (BDDNode * a, BDDNode * b)
{
   return ite_not (ite_or (a, b));
}

BDDNode * ite_imp (BDDNode * a, BDDNode * b)
{
   return ite (a, b, true_ptr);
}

BDDNode * ite_nimp (BDDNode * a, BDDNode * b)
{
   return ite_not (ite_imp (a, b));
}

BDDNode * ite_xor (BDDNode * a, BDDNode * b)
{
   return ite (a, ite_not (b), b);
}

BDDNode * ite_equ (BDDNode * a, BDDNode * b)
{
   return ite_not (ite_xor (a, b));
}

BDDNode * ite_and (BDDNode * a, BDDNode * b)
{
   return ite (a, b, false_ptr);
}

BDDNode * ite_nand (BDDNode * a, BDDNode * b)
{
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

BDDNode * restrictx (int bddNmbr1, int bddNmbr2, int *&length, store *&variables)
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
   //  if (f == c)
   //    return true_ptr;
   if ((c == true_ptr) || (f == false_ptr))
      return f;
   if (c == ite_not (f))
      return false_ptr;
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

BDDNode * remove_fpsx (int bddNmbr1, int bddNmbr2, int *&length, store *&variables)
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
      return pruning (f, ite_or (c->thenCase, c->elseCase));	//xquantify(c, c->variable));
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
	int tempint[5000];
	unravelBDD(&y, tempint, bddNmbr1);
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
	int length1 = y;
	int *vars1;
	if(length1 > 0) {
		vars1 = new int[length1+1];
		for(int i = 0; i < y; i++)
		  vars1[i] = tempint[i];
	}
	
	y = 0;
	unravelBDD(&y, tempint, bddNmbr2);
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
	int length2 = y;
	int *vars2;
	if(length2 > 0){
		vars2 = new int[length2+1];
		for(int i = 0; i < y; i++)
		  vars2[i] = tempint[i];
	}
	
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
   if ((c == true_ptr) || (f == true_ptr) || (f == false_ptr))
      return f;
   if (c == ite_not (f))
      return false_ptr;

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
int
countFalses (BDDNode * bdd)
{
   if (bdd == false_ptr)
      return 1;
   if (bdd == true_ptr)
      return 0;

   else
      return countFalses (bdd->thenCase) + countFalses (bdd->elseCase);
}

int
nmbrVarsInCommon (int bddNmbr1, int bddNmbr2, int *&length,
      store * &variables, int stopat)
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

BDDNode * xquantify (BDDNode * f, int v)
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
BDDNode * set_variable (BDDNode * BDD, int num, int torf)
{
   if (BDD->variable < num)
      return BDD;
   if (BDD->variable == num)
   {
      if (torf)
         return BDD->thenCase;

      else
         return BDD->elseCase;
   }
   BDDNode * r = set_variable (BDD->thenCase, num, torf);
   BDDNode * e = set_variable (BDD->elseCase, num, torf);
   if (r == e)
      return r;
   return find_or_add_node (BDD->variable, r, e);
}

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

void unravelBDD (long *y, int tempint[5000], BDDNode * func) {
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
		
      //End sorting and duplicates
   } else (*y)++;
   unravelBDD (y, tempint, func->thenCase);
   unravelBDD (y, tempint, func->elseCase);
}

BDDNode * num_replace (BDDNode * f, int var, int replace) {
   if (var > f->variable)
	  return f;
   if (var == f->variable) {
		if (replace > 0)
		  return ite(ite_var(replace), f->thenCase, f->elseCase);
      else if (replace < 0)
		  return ite (ite_var(-replace), f->elseCase, f->thenCase);
   }
   BDDNode * r = num_replace (f->thenCase, var, replace);
   BDDNode * e = num_replace (f->elseCase, var, replace);
   if (r == e)
	  return r;
   return ite (ite_var (f->variable), r, e);
	
   //return find_or_add_node(f->variable, r, e);
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
   int tempint[5000];
   long y, i;
   numinp = 0;
   for (int x = 0; x < nmbrFunctions; x++) {
      y = 0;
      unravelBDD (&y, tempint, functions[x]);
      if (y != 0) {
			for (i = 0; i < y; i++) {
            if (tempint[i] > numinp)
				  numinp = tempint[i];
         }
      }
   }
   return numinp;
}

int getNuminp () {
   numinp = 0;
   for (int x = 0; x < nmbrFunctions; x++) {
		if (functions[x]->variable > numinp)
		  numinp = functions[x]->variable;
	}
   return numinp;
}
