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
#include <sys/time.h>
#include "ite.h"

void makeXor(int variables, int functions, int length, int width) {
	struct timeval tv;
	int *pick = new int[length+(length/4)];
	gettimeofday(&tv, NULL);
	srand(tv.tv_usec);
	fprintf(stdout, "p xor %d %d\n", variables, functions);
	for(int x = 0; x < functions; x++) {
		int var;
      int y;
      /* pick variables */
		for(y = 0; y < (length+(length/4)); y++) {
			pick[y] = (rand() % variables) + 1;
		}
      /* linear part */
      int lin_max = 6*length/7;
      for(y = 0; y < lin_max; y++) {
				var = pick[(rand() % (length+(length/7)))];
				fprintf(stdout, "x%d ", var);
      }
      /* non-linear part */
		for(/*cont*/; y < length; y++) {
			int rand_width = (rand() % width) + 1;
			for(int z = 0; z < rand_width; z++) {
				var = pick[(rand() % (length+(length/7)))];
				fprintf(stdout, "x%d", var);
			}
			fprintf(stdout, " ");
		}
		int equalvar = rand() % 2;
		fprintf(stdout, "= %d\n", equalvar);
	}
   delete [] pick;
}

