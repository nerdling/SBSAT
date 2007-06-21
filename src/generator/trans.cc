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

void gen_matrix(int **matrix, int sizex, int sizey) {
	for(int x = 1; x <= sizex; x++) {
		matrix[x] = (int *)calloc(sizey+1, sizeof(int *));
		for(int y = 1; y <= sizey; y++) {
			matrix[x][y] = rand()%15;
			//fprintf(stdout, "%d", matrix[x][y]>1?8:matrix[x][y]);
		}
		//fprintf(stdout, "\n");
	}
}

void trans() {
	struct timeval tv;
	gettimeofday(&tv, NULL);
	fprintf(stderr, "seed = %d", tv.tv_usec);
	srand(tv.tv_usec);

	int sizex = 30;
	int sizey = 30;
	int **matrix = (int **)calloc(sizex+1, sizeof(int **));
   gen_matrix(matrix, sizex, sizey);
	fprintf(stdout, "p bdd %d %d\n", 100000, 100000);

	fprintf(stdout, "order(");
	for(int x = sizex; x > 0; x--)
	  fprintf(stdout, "p_%d ", x);
	fprintf(stdout, ")\n");

	/*
	for(int x = 1; x <= sizex; x++) {
		for(int y = 1; y <= sizey; y++) {
			fprintf(stdout, "exist(exist(exist(exist(\n");
		}
	}*/

//	fprintf(stdout, "and(\n");
	
	for(int x = 1; x <= sizex; x++) {
		for(int y = 1; y <= sizey; y++) {
			fprintf(stdout, "equ(matrix_%d_%d_1, %c)\n", x, y, matrix[x][y]==1?'t':'f');
			fprintf(stdout, "equ(matrix_%d_%d_m, %c)\n", x, y, matrix[x][y]==0?'t':'f');
			fprintf(stdout, "equ(matrix_%d_%d_0, %c)\n", x, y, matrix[x][y]>1?'t':'f');
			fprintf(stdout, "imp(matrix_%d_%d_0, not(trans_%d_%d))\n", x, y, x, y);
			fprintf(stdout, "imp(and(matrix_%d_%d_1, trans_%d_%d), p_%d)\n", x, y, x, y, x);
			fprintf(stdout, "imp(and(matrix_%d_%d_m, trans_%d_%d), not(p_%d))\n", x, y, x, y, x);
		}
	}
	
	for(int x = 1; x <= sizex; x++) {
		fprintf(stdout, "minmax(1, 1");
		for(int y = 1; y <= sizey; y++) {
			fprintf(stdout, ", trans_%d_%d", x, y);
		}
		fprintf(stdout, ")\n");
	}

	for(int x = 1; x <= sizex; x++) {
		fprintf(stdout, "minmax(1, 1");
		for(int y = 1; y <= sizey; y++) {
			fprintf(stdout, ", trans_%d_%d", y, x);
		}
		fprintf(stdout, ")\n");
	}

/*	
	fprintf(stdout, ")\n");
	
	for(int x = 1; x <= sizex; x++) {
		for(int y = 1; y <= sizey; y++) {
			fprintf(stdout, ", matrix_%d_%d_1), matrix_%d_%d_0), matrix_%d_%d_m), ", x, y, x, y, x, y);
			fprintf(stdout, "trans_%d_%d)\n", x, y);
		}
	}

	fprintf(stdout, "print_tree($1)\n");
*/
}
