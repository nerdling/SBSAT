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

#include "sbsat.h"

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

void printBDDfile(BDDNode * bdd, FILE * fout) {
   if (bdd == true_ptr) {
      fprintf (fout, "T");
      return;
   }
   if (bdd == false_ptr) {
      fprintf (fout, "F");
      return;
   }
	fprintf (fout, "(");
   printBDDfile(bdd->thenCase, fout);
   //fprintf (stdout, "[%d]", bdd->variable);
	/* D_4(fprintf(fout, "[%s", s_name(bdd->variable));) */
	fprintf(fout, "[%d]", bdd->variable);
   printBDDfile(bdd->elseCase, fout);
   fprintf(fout, ")");
}
	 
void printBDD(BDDNode * bdd) { 
   if (bdd == true_ptr) {
      d2_printf1 ("T");
      return;
   }
   if (bdd == false_ptr) {
      d2_printf1 ("F");
      return;
   }
   d2_printf1 ("(");
   printBDD(bdd->thenCase);
   //fprintf (stdout, "[%d]", bdd->variable);
	d2_printf2("[%s", s_name(bdd->variable));
	d4_printf2("(%d)", bdd->variable);
	d2_printf1("]");
   printBDD(bdd->elseCase);
   d2_printf1(")");
}

/*
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
*/

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
   return ite_xvar_y_z(ite_var(integers[level]), r, e);
}

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
	d2_printf1("\n");
   for (x = 0; x < PRINT_TREE_WIDTH; x++)
      d2_printf1("-");
   d2_printf1("\n");
   for (x = 0; x < 16; x++)
      zoomarr[x] = 0;
   while ((y < z) && (y < 31)) {
      for (i = 0; i < NUM / 2; i++)
		  d2_printf1(" ");
      for (x = 0; x < (1 << level); x++) {
         if (tempint[y] == -1) {
				sprintf(aa, "T");
			}
         else if (tempint[y] == -2) {
				sprintf(aa, "F");
			} else if (tempint[y] == 0) {
				sprintf(aa, " ");
			} else if (y > 14) {
				sprintf(aa, "*%d", (*which_zoom)++);
            zoomarr[reference++] = y;
         } else {
				sprintf(aa, "%s", s_name(tempint[y]));
				D_3(sprintf(aa, "%d", tempint[y]););
				D_4(sprintf(aa, "%s(%d)", s_name(tempint[y]), tempint[y]););
				
			}
			int l = strlen(aa);
			if(l%2 == 0) {
				for(i = 0; i < l/2-1; i++)
				  d2_printf1("\b");
				d2_printf1(aa);
				if ((x + 1) < (1 << level))
				  for(i = l/2+1;i < NUM; i++)
					 d2_printf1(" ");
			} else {
				for(i = 0; i < l/2; i++)
				  d2_printf1("\b");
				d2_printf1(aa);
				if ((x + 1) < (1 << level))
				  for(i = l/2+1;i < NUM; i++)
					 d2_printf1(" ");
			}
			y++;
		}
      level++;
      NUM /= 2;
      d2_printf1("\n\n");
   }
   int now_zoom = *which_zoom;
   for (x = 0; (zoomarr[x] != 0) && (x < 16); x++) {
		d2_printf2("\n*%d ", now_zoom - reference + x);
		printBDDTree (findBranch (0, zoomarr[x], bdd), which_zoom);
      d2_printf1("\n");
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
      d2_printf2("Constraint #%d\n", i);
      printBDD (functions[i]);
      d2_printf1("\n");
      printBDDTree (functions[i], &which_zoom);
      d2_printf1("\n\n");
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

