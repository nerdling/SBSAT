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
#include "sbsat.h"

struct timeval tv1;
struct timezone tzp1;

void makeXor(int variables, int functions, int length, int width, int fixed);
void vanDerWaerden(char *vdw_type, int n, int k, int p);
void rn(char *rn_type, int n, int k, int l);
void slider2(char *out_type, int n, int sat);
void rand_BDD(char *out_type, int num_vars, int num_funcs, int vars_per_func);
void trans(int size, int mult, int seed);
void trans_acl2(int size, int mult, int seed);
void trans_acl2_fn(int size, int mult, int seed);
void trans_acl2_reorder(int size, int mult, int seed);
void trans_acl2_reorder2(int size, int mult, int seed);
void trans_new_acl2(int size, int mult, int seed);
void trans_old_acl2(int size, int mult, int seed);
void add_tree(int level, int variable);
void rksat(int n, int m, int k, int seed);

int main(int argc, char **argv) {
   if (argc > 1 && !strcmp(argv[1], "vdw")) { 
      if (argc < 6 || (argc > 2 && !strcmp(argv[2], "--help"))) {
         fprintf(stderr, "usage: %s vdw vdw_type n k p\n", argv[0]);
         fprintf(stderr, " where vdw_type is one of cnf, xcnf, cnf2, ite\n");
         fprintf(stderr, "       n is number of variables\n");
         fprintf(stderr, "       k is number of buckets\n");
         fprintf(stderr, "       p is progression size\n");
         return 0;
      }
		int n = atoi(argv[3]);
		int k = atoi(argv[4]);
		int p = atoi(argv[5]);
      vanDerWaerden(argv[2], n, k, p);
		return 0;
	} else
	  if (argc > 1 && !strcmp(argv[1], "xor")) { 
		  if (argc < 7 || (argc > 2 && !strcmp(argv[2], "--help"))) {
			  fprintf(stderr, "usage: %s xor #vars #functions length max_width fixed(0/1)\n", argv[0]);
			  return 0;
		  }
		  int variables = atoi(argv[2]);
		  int functions = atoi(argv[3]);
		  int length = atoi(argv[4]);
		  int width = atoi(argv[5]);
		  int fixed = atoi(argv[6]);
		  makeXor(variables, functions, length, width, fixed);
		  return 0;
	  } else
	  if (argc > 1 && !strcmp(argv[1], "rksat")) {
		  if (argc < 5 || (argc > 2 && !strcmp(argv[2], "--help"))) {
			  fprintf(stderr, "usage: %s rksat n m k [seed]\n", argv[0]);
			  fprintf(stderr, "       n - variables\n");
			  fprintf(stderr, "       k - clauses\n");
			  fprintf(stderr, "       k - vars per clause\n");
			  return 0;
		  }
		  int n = atoi(argv[2]);
		  int m = atoi(argv[3]);
		  int k = atoi(argv[4]);

		  int seed = 0;
		  
		  if(argc == 6)
			 seed = atoi(argv[5]);
		  rksat(n, m, k, seed);
		  return 0;
	  } else	  
	  if (argc > 1 && !strcmp(argv[1], "rn")) { 
		  if (argc < 5 || (argc > 2 && !strcmp(argv[2], "--help"))) {
			  fprintf(stderr, "usage: %s rn n k l\n", argv[0]);
			  fprintf(stderr, "       n - vertices\n");
			  fprintf(stderr, "       k - aquatances\n");
			  fprintf(stderr, "       l - strangers\n");
			  return 0;
		  }
		  int n = atoi(argv[2]);
		  int k = atoi(argv[3]);
		  int l = atoi(argv[4]);
		  rn("cnf", n, k, l);
		  return 0;
	  } else
	  if (argc > 1 && !strcmp(argv[1], "slider2")) { 
		  if (argc < 3 || (argc > 2 && !strcmp(argv[2], "--help"))) {
			  fprintf(stderr, "usage: %s size sat\n", argv[0]);
			  fprintf(stderr, "       size - num variables\n");
			  //fprintf(stderr, "       sat - 0/1 = un/sat\n");
			  return 0;
		  }
		  int size = atoi(argv[2]);
		  int sat = atoi(argv[3]);
		  slider2("ite", size, sat);
		  return 0;
	  } else
	  if (argc > 1 && !strcmp(argv[1], "trans")) {
		  if (argc < 4 || (argc > 2 && !strcmp(argv[2], "--help"))) {
			  fprintf(stderr, "usage: %s trans size mult [seed]\n", argv[0]);
			  fprintf(stderr, "       size - row and column size of matrix\n");
			  fprintf(stderr, "       mult - a big value gives more zero entries\n");
			  return 0;
		  }
		  int size = atoi(argv[2]);
		  int mult = atoi(argv[3]);
		  int seed = 0;
		  if(argc == 5)
			 seed = atoi(argv[4]);
		  trans(size, mult,seed);
		  return 0;
	  } else
	  if (argc > 1 && !strcmp(argv[1], "trans-acl2")) {
		  if (argc < 4 || (argc > 2 && !strcmp(argv[2], "--help"))) {
			  fprintf(stderr, "usage: %s trans-acl2 size mult [seed]\n", argv[0]);
			  fprintf(stderr, "       size - row and column size of matrix\n");
			  fprintf(stderr, "       mult - a big value gives more zero entries\n");
			  return 0;
		  }
		  int size = atoi(argv[2]);
		  int mult = atoi(argv[3]);
		  int seed = 0;
		  if(argc == 5)
			 seed = atoi(argv[4]);
		  trans_acl2(size, mult,seed);
		  return 0;
	} else
	  if (argc > 1 && !strcmp(argv[1], "trans-acl2-fn")) {
		  if (argc < 4 || (argc > 2 && !strcmp(argv[2], "--help"))) {
			  fprintf(stderr, "usage: %s trans-acl2-fn size mult [seed]\n", argv[0]);
			  fprintf(stderr, "       size - row and column size of matrix\n");
			  fprintf(stderr, "       mult - a big value gives more zero entries\n");
			  return 0;
		  }
		  int size = atoi(argv[2]);
		  int mult = atoi(argv[3]);
		  int seed = 0;
		  if(argc == 5)
			 seed = atoi(argv[4]);
		  trans_acl2_fn(size, mult,seed);
		  return 0;
	  } else
	  if (argc > 1 && !strcmp(argv[1], "trans-acl2-reorder")) {
		  if (argc < 4 || (argc > 2 && !strcmp(argv[2], "--help"))) {
			  fprintf(stderr, "usage: %s trans-acl2-reorder size mult [seed]\n", argv[0]);
			  fprintf(stderr, "       size - row and column size of matrix\n");
			  fprintf(stderr, "       mult - a big value gives more zero entries\n");
			  return 0;
		  }
		  int size = atoi(argv[2]);
		  int mult = atoi(argv[3]);
		  int seed = 0;
		  if(argc == 5)
			 seed = atoi(argv[4]);
		  trans_acl2_reorder(size, mult,seed);
		  return 0;
	  } else
	  if (argc > 1 && !strcmp(argv[1], "trans-acl2-reorder2")) {
		  if (argc < 4 || (argc > 2 && !strcmp(argv[2], "--help"))) {
			  fprintf(stderr, "usage: %s trans-acl2-reorder2 size mult [seed]\n", argv[0]);
			  fprintf(stderr, "       size - row and column size of matrix\n");
			  fprintf(stderr, "       mult - a big value gives more zero entries\n");
			  return 0;
		  }
		  int size = atoi(argv[2]);
		  int mult = atoi(argv[3]);
		  int seed = 0;
		  if(argc == 5)
			 seed = atoi(argv[4]);
		  trans_acl2_reorder2(size, mult,seed);
		  return 0;
	  } else
	  if (argc > 1 && !strcmp(argv[1], "trans-new-acl2")) {
		  if (argc < 4 || (argc > 2 && !strcmp(argv[2], "--help"))) {
			  fprintf(stderr, "usage: %s trans-new-acl2 size mult [seed]\n", argv[0]);
			  fprintf(stderr, "       size - row and column size of matrix\n");
			  fprintf(stderr, "       mult - a big value gives more zero entries\n");
			  return 0;
		  }
		  int size = atoi(argv[2]);
		  int mult = atoi(argv[3]);
		  int seed = 0;
		  if(argc == 5)
			 seed = atoi(argv[4]);
		  trans_new_acl2(size, mult,seed);
		  return 0;
	  } else
	  if (argc > 1 && !strcmp(argv[1], "trans-old-acl2")) {
		  if (argc < 4 || (argc > 2 && !strcmp(argv[2], "--help"))) {
			  fprintf(stderr, "usage: %s trans-old-acl2 size mult [seed]\n", argv[0]);
			  fprintf(stderr, "       size - row and column size of matrix\n");
			  fprintf(stderr, "       mult - a big value gives more zero entries\n");
			  return 0;
		  }
		  int size = atoi(argv[2]);
		  int mult = atoi(argv[3]);
		  int seed = 0;
		  if(argc == 5)
			 seed = atoi(argv[4]);
		  trans_old_acl2(size, mult,seed);
		  return 0;
	  } else
	  if (argc > 1 && !strcmp(argv[1], "add_tree")) {
		  if (argc < 4 || (argc > 2 && !strcmp(argv[2], "--help"))) {
			  fprintf(stderr, "usage: %s add_tree size\n", argv[0]);
			  fprintf(stderr, "       level - level at which variable occurs\n");
			  fprintf(stderr, "       variable - variable to search for at level\n");
			  return 0;
		  }
		  int level = atoi(argv[2]);
		  int variable = atoi(argv[3]);
		  add_tree(level, variable);
		  return 0;
	} else
	  if (argc > 1 && !strcmp(argv[1], "rbdd")) {
		  if (argc < 5 || (argc > 2 && !strcmp(argv[2], "--help"))) {
         fprintf(stderr, "usage: %s rbdd v b vpb\n", argv[0]);
			  fprintf(stderr, "       v - variables\n");
			  fprintf(stderr, "       b - bdds\n");
			  fprintf(stderr, "       vpb - variables per bdd\n");
			  return 0;
		  }
		  int v = atoi(argv[2]);
		  int b = atoi(argv[3]);
		  int vpb = atoi(argv[4]);
		  
		  if(v < 2 || b < 1 || vpb < 2 || vpb > v) {
			  fprintf(stderr, "usage: %s rbdd v b vpb\n", argv[0]);
			  fprintf(stderr, "       v - variables\n");
			  fprintf(stderr, "       b - bdds\n");
			  fprintf(stderr, "       vpb - variables per bdd\n");
			  fprintf(stderr, "Error! Either (v < 2), (b < 1), (vpb < 2), or (vpb > v)\n");
			  return 0;
		  }
		  
		  //rand_BDD("ite", v, b, vpb);
		  return 0;
	}
	
	fprintf(stderr, "usage: %s xor --help\n", argv[0]);
	fprintf(stderr, "usage: %s vdw --help\n", argv[0]);
	fprintf(stderr, "usage: %s rn --help\n", argv[0]);
	fprintf(stderr, "usage: %s slider2 --help\n", argv[0]);
	fprintf(stderr, "usage: %s rbdd --help\n", argv[0]);
	fprintf(stderr, "usage: %s trans --help\n", argv[0]);
	fprintf(stderr, "usage: %s add_tree --help\n", argv[0]);
	fprintf(stderr, "usage: %s rksat --help\n", argv[0]);
	return 0;
}
