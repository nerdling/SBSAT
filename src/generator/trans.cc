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

int *rows;

void gen_matrix(int **matrix, int sizex, int sizey, int mult) {
	for(int x = 1; x <= sizex; x++) {
		int row = -1;
		matrix[x] = (int *)calloc(sizey+1, sizeof(int *));
		for(int y = 1; y <= sizey; y++) {
			matrix[x][y] = rand()%mult;
			if(x == y) matrix[x][y] = 1;
			//fprintf(stdout, "%d", matrix[x][y]>1?8:matrix[x][y]);

			if(matrix[x][y]==1 && row==-1) row = 1;
			else if(matrix[x][y]==1 && row==0) row = 2;
			else if(matrix[x][y]==0 && row==-1) row = 0;
			else if(matrix[x][y]==0 && row==1) row = 2;
		}
		rows[x] = row;
		//fprintf(stdout, "\n");
	}
}

void trans(int size, int mult) {
	struct timeval tv;
	gettimeofday(&tv, NULL);
	fprintf(stderr, "seed = %d\n", tv.tv_usec);
	srand(tv.tv_usec);
	//srand(497546);
	
	int sizex = size;
	int sizey = size;
	int **matrix = (int **)calloc(sizex+1, sizeof(int **));
	rows = (int*)calloc(sizex+1, sizeof(int*));
   gen_matrix(matrix, sizex, sizey, mult);
	fprintf(stdout, "p bdd %d %d\n", 6*sizex*sizey, (5*sizex*sizey)+2*sizex+1);

//	fprintf(stdout, "order(");

//	for(int x = 1; x <= sizex; x++) {
//		for(int y = 1; y <= sizey; y++) {
//			if(matrix[x][y]<=1) fprintf(stdout, "trans_%d_%d ", x, y);
//		}
//		fprintf(stdout, "p_%d ", x);
//	}

	
//	for(int x = sizex; x > 0; x--)
//	  fprintf(stdout, "p_%d ", x);

//	fprintf(stdout, ")\n");

	for(int x = sizex; x > 0; x--) {
		for(int y = sizey; y > 0; y--) {
			if(matrix[y][x]<=1) fprintf(stdout, "exist(");
		}
		fprintf(stdout, "and(");
	}
	
	fprintf(stdout, "\n");
	
	for(int x = 1; x <= sizex; x++) {
		if(rows[x]==1) {
			//fprintf(stdout, "var(p_%d)\n", x);
		} else if(rows[x]==0) {
			//fprintf(stdout, "not(p_%d)\n", x);
		} else if(rows[x]==-1) {
			fprintf(stderr, "Empty Row - Unsat file\n");
			exit(0);
		} else {
			for(int y = 1; y <= sizey; y++) {
				//if(matrix[x][y]>1) fprintf(stdout, "not(trans_%d_%d)\n", x, y);
				//else 
				if(matrix[x][y]==1) fprintf(stdout, "imp(trans_%d_%d, p_%d)\n", x, y, x);
				else if(matrix[x][y]==0) fprintf(stdout, "imp(trans_%d_%d, not(p_%d))\n", x, y, x);
			}
		}
	}
	
	for(int x = 1; x <= sizex; x++) {
		fprintf(stdout, "minmax(1, 1");
		for(int y = 1; y <= sizey; y++) {
			if(matrix[x][y]<=1) fprintf(stdout, ", trans_%d_%d", x, y);
		}
		fprintf(stdout, ")\n");
	}

	for(int x = 1; x <= sizex; x++) {
		fprintf(stdout, "minmax(1, 1");
		for(int y = 1; y <= sizey; y++) {
			if(matrix[y][x]<=1) fprintf(stdout, ", trans_%d_%d", y, x);
		}
		
		//fprintf(stdout, ")\n");
		fprintf(stdout, "))\n");
		
		for(int y = 1; y <= sizey; y++) {
			if(matrix[y][x]<=1) fprintf(stdout, ", trans_%d_%d)", y, x);
		}
		fprintf(stdout, "\n");
	}

	//fprintf(stdout, "print_tree($1)\n");
}
