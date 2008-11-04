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

int prob = 0;

void makeXor(int variables, int functions, int length, int width, int fixed) {
	struct timeval tv;
	int *pick = new int[length+(length/4)];
	gettimeofday(&tv, NULL);
	srand(tv.tv_usec);
	fprintf(stdout, "p xor %d %d\n", variables, functions);
	//fprintf(stdout, "p bdd %d %d\n", variables, functions);
	if(width == 1) fixed = 0;

	//fprintf(stdout, "and(\n");
	for(int x = 0; x < functions; x++) {
		int var;
      int y;
      /* pick variables */
		for(y = 0; y < (length+(length/4)); y++) {
			pick[y] = (rand() % variables) + 1;
		}
      /* linear part */
		int lin_max = 6*length/7;
		if(fixed == 1) lin_max = 0;
		if(width == 1) lin_max = length;
		
		if(prob == 1) {
			for(y = 1; y <= variables; y++) {
				if((rand() % 2))
				  fprintf(stdout, "x%d ", y);
			}
		} else if(prob == 2) {
			fprintf(stdout, "xor(");
			for(y = 1; y <= variables; y++) {
				if((rand() % 2))
				  fprintf(stdout, "x%d ", y);
			}
		} else {
			for(y = 0; y < lin_max; y++) {
				var = pick[(rand() % (length+(length/7)))];
				//var = pick[y];
				fprintf(stdout, "x%d ", var);
			}
		}

      /* non-linear part */
		if(width > 1) {
			for(/*cont*/; y < length; y++) {
				int rand_width = (rand() % width) + 1;
				if(fixed == 1) rand_width = width;
				if(rand_width == 1) {
					for(int z = lin_max; z < length; z++) {
						var = pick[z];
						fprintf(stdout, "x%d", var);
					}
				} else {
					for(int z = 0; z < rand_width; z++) {
						var = (rand() % variables) + 1;//pick[(rand() % (length+(length/7)))];
						fprintf(stdout, "x%d", var);
					}
				}
				fprintf(stdout, " ");
			}
		}
		int equalvar = rand() % 2;
		fprintf(stdout, "= %d\n", equalvar);
		//fprintf(stdout, "%c)\n", equalvar?'T':'F');
	}

	//fprintf(stdout, ")\n");
	
	delete [] pick;
}

