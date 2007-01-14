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
#include "prototypes.h"

void check_BDDvbles(BDDNode *bdd, int *vbles, int vbles_size) {
	if(IS_TRUE_FALSE(bdd)) return;
	for(int i=0; i<vbles_size;i++) {
		if(vbles[i] == bdd->variable) {
			vbles[i] = 0;
			break;
		}
	}
	check_BDDvbles(bdd->thenCase, vbles, vbles_size);
	check_BDDvbles(bdd->elseCase, vbles, vbles_size);
}

void rand_BDD(char *output_type, int num_vars, int num_funcs, int vars_per_func) {
	struct timeval tv;
	gettimeofday(&tv, NULL);
	srand(tv.tv_usec);
   
	printf("p bdd %d %d\n", num_vars, num_funcs);
   printf("; automatically generated %d random BDDs with %d vars.\n", num_funcs, num_vars);
   printf("; Disclaimer: no formal analysis was done to verify SAT or UNSAT.\n");
	printf("; Each BDD contains exactly %d variables.\n", vars_per_func);
	printf("; No variable is initially forced by a single BDD.\n");

	itetable_init();
	bdd_init();
	enable_gc = 1;
	
	int *vbles = (int *)calloc(vars_per_func, sizeof(int));
	char *ttable = (char *)calloc(1<<vars_per_func, sizeof(char));
	for(int x=0; x<num_funcs; x++) {
		BDDNode *bdd;
		int no_inferences = 0;
		int all_variables = 0;
		while((no_inferences==0) || (all_variables==0)) {
			all_variables = 0;
			no_inferences = 0;
			for(int y=0; y<vars_per_func; y++) {
				vbles[y] = (rand()%num_vars-1)+2;
				for(int z=0; z<y; z++) {
					if(vbles[z] == vbles[y]) {
						y--;
						break;
					}
				}
			}
			
			qsort(vbles, vars_per_func, sizeof(int), revcompfunc);

			//for(int y=0; y<vars_per_func; y++)
			//  fprintf(stdout, "%d, ", vbles[y]);
			//fprintf(stdout, "\n");
			
			for(int y=0; y<1<<vars_per_func; y++)  {
				if(rand()%2 == 1) ttable[y] = '1';
				else ttable[y] = '0';
				//fprintf(stderr, "%c", ttable[y]);
			}
			
			//BDD must not contain an inference at the top level
			int y = 0;
			int level = 0;
			bdd = ReadSmurf(&y, ttable, level, vbles, vars_per_func);
			if(bdd->inferences == NULL) no_inferences = 1;
			
			//BDD must also contain all variables in vbles.
			check_BDDvbles(bdd, vbles, vars_per_func);

			for(y=0; y<vars_per_func; y++)
				if(vbles[y]!=0) break;

			if(y == vars_per_func) all_variables = 1;

		}
		printITEBDD(bdd);
		printf("\n");
		
		  
	}

}
