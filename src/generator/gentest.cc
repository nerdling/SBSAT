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
#include "ite.h"

void makeXor(int variables, int functions, int length, int width);
void vanDerWaerden(char *vdw_type, int n, int k, int p);
void rn(char *rn_type, int n, int k, int l);

int main(int argc, char **argv) {
   if (argc > 1 && !strcmp(argv[1], "vdw")) { 
      if (argc < 6 || (argc > 2 && !strcmp(argv[2], "--help"))) {
         fprintf(stderr, "usage: %s vdw vdw_type n k p\n", argv[0]);
         fprintf(stderr, " where wdw_type is one of cnf, xcnf, cnf2, ite\n");
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
   if (argc > 1 && !strcmp(argv[1], "vdw")) { 
      if (argc < 6 || (argc > 2 && !strcmp(argv[2], "--help"))) {
         fprintf(stderr, "usage: %s xor #vars #functions length max_width\n", argv[0]);
         return 0;
      }
      int variables = atoi(argv[2]);
      int functions = atoi(argv[3]);
		int length = atoi(argv[4]);
		int width = atoi(argv[5]);
      makeXor(variables, functions, length, width);
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
	}

	fprintf(stderr, "usage: %s xor --help\n", argv[0]);
	fprintf(stderr, "usage: %s vdw --help\n", argv[0]);
	fprintf(stderr, "usage: %s rn --help\n", argv[0]);
	return 0;
}
