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
 any trademark, service mark, or the name of University of Cincinnati.


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

#include <stdio.h>
#include <sys/time.h>
#include "sbsat.h"

/* Ramsey numbers R(k,l)=n
 *
 * find n for which in Kn when 2-colored there is no Kk or Kl
 *
 * for a graph of four nodes (1,2,3,4) the edges will be translated to cnf variables
 * (1,2) - 1
 * (1,3) - 2
 * (1,4) - 3
 * (2,3) - 4
 * (2,4) - 5
 * (3,4) - 6
 *
 * 1. internally assign a number to every possible edge in the graph
 * 2. create all possible Kk graphs and prevent them from happening by reversing the polarity
 * 3. create all possible Kl graphs and prevent them from happening by reversing the polarity
 *
 * so for R(3,3)=4 the problem should look like:
 * p cnf 6 8
 * 4  5  6 0
 * 2  3  6 0
 * 1  3  5 0
 * 1  2  4 0
 * -4 -5 -6 0
 * -2 -3 -6 0
 * -1 -3 -5 0
 * -1 -2 -4 0
 *
 * This is very naive approach. There is plenty of room for improvement 
 * such as symetry breaking to mention one.
 */

#define CNF 0

int rn_formula_type = CNF;

int arr[100];
int edges[100][100];
int print_clauses_k=0;
int print_clauses_n=0;
char print_clauses_c=0;

void
print_clauses(int current, int out)
{
   if (out == print_clauses_k) {
      for(int i=0;i<out;i++)
         for(int j=i+1;j<out;j++) {
            fprintf(stdout, "%c%d ", print_clauses_c, edges[arr[i]][arr[j]]);
//            fprintf(stdout, "(%d,%d) ", arr[i], arr[j]);
         }
      fprintf(stdout, "0\n");
      return;
   }
   if (current > print_clauses_n) return;
   print_clauses(current+1, out);
   arr[out] = current;
   print_clauses(current+1, out+1);
}

int edge_counter = 1;
int clause_counter = 1;

int C(int k, int n) {
   long x=1;
   for (int i=0;i<k;i++) {
      x *= n;
      n--;
   }
   for (int j=0;j<k;j++) {
      x /= (j+1);
   }
   return x;
}

void rn(char *rn_type, int n, int k, int l) {
   if (!strcmp(rn_type, "cnf")) rn_formula_type = CNF; else 
   {
      fprintf(stderr, "Unknown rn formula type\n");
      exit(1);
   }

   // define a number for all pairs n x n
   for(int i=1;i<=n;i++)
      for(int j=i+1;j<=n;j++)
         edges[i][j] = edge_counter++;

   clause_counter = C(k,n)+C(l,n); // k choose n + l choose n
   fprintf(stdout, "p cnf %d %d\n", edge_counter-1, clause_counter);
   
   // choose all combinations from n of a length k and enter them with positive sign
   print_clauses_n = n;
   print_clauses_k = k;
   print_clauses_c = ' ';
   print_clauses(1, 0);

   // choose all combinations from n of a length l and enter them with negative sign
   print_clauses_n = n;
   print_clauses_k = l;
   print_clauses_c = '-';
   print_clauses(1, 0);
}
