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

void add_tree (int level, int variable) {
	
	if(level == 1 && variable == 1) {
		fprintf(stdout, "Satisfiable\n");
		return;
	}
	if(level == 2 && variable == 2) {
		fprintf(stdout, "Satisfiable\n");
		return;
	}
	if(level <= 2) {
		fprintf(stdout, "Unsatisfiable\n");
		return;
	}
	if(1 << level < variable) {
		fprintf(stdout, "Unsatisfiable\n");
		return;
	}
	
	fprintf(stdout, "p bdd %d %d\n", 10000, 10000); //Figure out these numbers.
	
	fprintf(stdout, "equ(L1_b1, T)\nequ(L2_b1, F)\nequ(L2_b2, T)\n");
	
	for(int x = 3; x <= level; x++) {

		fprintf(stdout, "order(");
		for(int y = 1; y < x; y++) {
			for(int z = y; z < x; z++) {
			   fprintf(stdout, "L%d_choice1_L%d_b%d, ", x, z, y);
				fprintf(stdout, "L%d_choice2_L%d_b%d, ", x, z, y);
			}
//			fprintf(stdout, "L%d_b%d_c, ", x, y-1);
		}

//		fprintf(stdout, "L%d_b1, L%d_b1", x, x-1);
//		for(int y = 2; y < x; y++)
//		  fprintf(stdout, "L%d_b%d, L%d_b%d, ", x, y, x-1, y);

//		fprintf(stdout, "L%d_b%d, ", x, x);
 
		fprintf(stdout, ")\n");
		
		fprintf(stdout, "minmax(1, 1");
		for(int y = 1; y < x; y++)
		  fprintf(stdout, ", L%d_choice1_L%d", x, y);
		fprintf(stdout, ")\n");
		fprintf(stdout, "minmax(1, 1");

		for(int y = 1; y < x; y++)
		  fprintf(stdout, ", L%d_choice2_L%d", x, y);
		fprintf(stdout, ")\n");
		
		for(int y = 1; y < x; y++) {
			fprintf(stdout, "imp(L%d_choice1_L%d, minmax(1, 1", x, y);
			for(int z = y; z < x; z++)
			  fprintf(stdout, ", L%d_choice2_L%d", x, z);
			fprintf(stdout, "))\n");			
		}
		
		for(int y = 1; y < x; y++) {
			for(int z = 1; z <= y; z++)
			  fprintf(stdout, "equ(t_L%d_choice1_L%d_b%d_L%d_b%d, equ(L%d_choice1_L%d_b%d, L%d_b%d))\n", x, y, z, y, z, x, y, z, y, z);
			fprintf(stdout, "ite(L%d_choice1_L%d, ", x, y);
			if(y!=1) fprintf(stdout, "and(");
			for(int z = 1; z <= y; z++)
			  fprintf(stdout, "t_L%d_choice1_L%d_b%d_L%d_b%d, ", x, y, z, y, z);
			if(y!=1) fprintf(stdout, "), and(");
			for(int z = 1; z <= y; z++)
			  fprintf(stdout, "-L%d_choice1_L%d_b%d, ", x, y, z);
			fprintf(stdout, ")");
			if(y!=1) fprintf(stdout, ")");
			fprintf(stdout, "\n");

         for(int z = 1; z <= y; z++)
			  fprintf(stdout, "equ(t_L%d_choice2_L%d_b%d_L%d_b%d, equ(L%d_choice2_L%d_b%d, L%d_b%d))\n", x, y, z, y, z, x, y, z, y, z);
			fprintf(stdout, "ite(L%d_choice2_L%d, ", x, y);
			if(y!=1) fprintf(stdout, "and(");
			for(int z = 1; z <= y; z++)
			  fprintf(stdout, "t_L%d_choice2_L%d_b%d_L%d_b%d, ", x, y, z, y, z);
			if(y!=1) fprintf(stdout, "), and(");
			for(int z = 1; z <= y; z++)
			  fprintf(stdout, "-L%d_choice2_L%d_b%d, ", x, y, z);
			fprintf(stdout, ")");
			if(y!=1) fprintf(stdout, ")");
			fprintf(stdout, "\n");
		}
		
		for(int y = 1; y < x; y++) {
			fprintf(stdout, "equ(L%d_b%d, xor(", x, y);
			for(int z = y; z < x; z++) {
				fprintf(stdout, "L%d_choice1_L%d_b%d, ", x, z, y);
				fprintf(stdout, "L%d_choice2_L%d_b%d, ", x, z, y);					
			}
			fprintf(stdout, "L%d_b%d_c))\n", x, y-1);
		}
		fprintf(stdout, "equ(L%d_b%d, L%d_b%d_c)\n", x, x, x, x-1);
		fprintf(stdout, "var(-L%d_b0_c)\n", x);

		for(int y = 1; y < x; y++) {
			fprintf(stdout, "equ(L%d_b%d_c, minmax(2, 3, ", x, y);
			for(int z = y; z < x; z++) {
				fprintf(stdout, "L%d_choice1_L%d_b%d, ", x, z, y);
				fprintf(stdout, "L%d_choice2_L%d_b%d, ", x, z, y);
			}
			fprintf(stdout, "L%d_b%d_c))\n", x, y-1);
		}
		fprintf(stdout, "or(L%d_b%d, ", x, x);
		for(int y = x-1; y > 1; y--)
		  fprintf(stdout, "or(and(L%d_b%d, -L%d_b%d), and(equ(L%d_b%d, L%d_b%d), ", x, y, x-1, y, x, y, x-1, y);
		fprintf(stdout, "and(L%d_b1, -L%d_b1)", x, x-1);
		for(int y = x-1; y > 1; y--)
		  fprintf(stdout, "))");
		fprintf(stdout, ")\n");
	}	
	
	fprintf(stdout, "and(");
	for(int x = 1; x <= level; x++) {
		if((variable & 1) == 1) fprintf(stdout, "L%d_b%d, ", level, x);
		else fprintf(stdout, "-L%d_b%d, ", level, x);
		variable = variable >> 1;		
	}
	fprintf(stdout, ")\n");
}
