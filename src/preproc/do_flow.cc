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

#include "sbsat.h"
#include "sbsat_preproc.h"


void Do_Flow() {
	var_score = (float *)ite_calloc(numinp+1, sizeof(float), 9, "var_score");
	int *var_mark = (int *)ite_calloc(numinp+1, sizeof(int), 9, "var_mark");

	int *equ_funcs = (int *)ite_calloc(numinp+1, sizeof(int), 9, "equ_funcs");
	
	int count = 0;
	for(int x = 1; x <= numinp; x++) {
		if(independantVars[x] == 1) {
			count++;
			var_mark[x] = 1;
			if(num_funcs_var_occurs == 0) 
			  {
//				  fprintf(stderr, "\nnon-existent variable\n");
				  continue;
			  }
			  var_score[x] = 1/((float)num_funcs_var_occurs[x]);
		} else {
			for(int y = 0; y < nmbrFunctions; y++) {
				if(equalityVble[y] == x) equ_funcs[x] = y;				
			}
		}
	}
//fprintf(stderr, "\ncount = %d\n", count);
	int go_again = 1;
	int times = 20;
	while(go_again == 1 && times > 0) {
		times--;
		go_again = 0;

		for(int x = 1; x <= numinp; x++) {
			if (var_mark[x] == 0) {
//				fprintf(stderr, "trying %d\n", x);
				float score = 0;
				int out = 0;
				for(int y = 0; y < length[equ_funcs[x]]; y++) {
					if(variables[equ_funcs[x]].num[y] == x) continue;
					if(var_score[variables[equ_funcs[x]].num[y]] == 0) {
//						fprintf(stderr, "%d has not yet been set\n", variables[equ_funcs[x]].num[y]);
						out = 1;
						break;
					}
					score+=var_score[variables[equ_funcs[x]].num[y]];
				}
				if(out == 1) {
					go_again = 1;
					continue;
				}
				if((num_funcs_var_occurs[x] - 1) == 0) {
					fprintf(stddbg, "\ndivide by zero in Do_Flow\n");
					break;
					//exit(0);
				}
				var_score[x] = score/((float)(num_funcs_var_occurs[x] - 1));
				if(var_score[x] == 0 && x != numinp+1) {
//					symrec *ptr = getsym_i(x);
//					fprintf(stderr, "%s", ptr->name);
//					fprintf(stderr, "PROBLEM: Zero weight variable");
					//exit(0);
				}
				var_mark[x] = 1;
			}
		}
	}

	for(int x = 0; x <= numinp; x++) {
		symrec *ptr = getsym_i(x);
		var_score[x] = var_score[x] + 1;
		d5_printf4("%d (%s) = %5.3f\n", x, (ptr&&ptr->name?ptr->name:"NULL"), var_score[x]);
	}

	ite_free((void**)&equ_funcs);
	//ite_free((void**)&var_score);
	ite_free((void**)&var_mark);
}

void Get_Cutx(int x, int *influence, int *equ_funcs, int depth) {
	if(depth == 0) {
		for(int y = 0; y < length[equ_funcs[x]]; y++) {
			if(x != variables[equ_funcs[x]].num[y])
			  influence[variables[equ_funcs[x]].num[y]]++;
		}
	} else {
		for(int y = 0; y < length[equ_funcs[x]]; y++) {
//			d5_printf3("{%d %d}", x, y);
			if(x != variables[equ_funcs[x]].num[y]) {
				if(independantVars[variables[equ_funcs[x]].num[y]] == 1)
				  influence[variables[equ_funcs[x]].num[y]]++;
				else Get_Cutx(variables[equ_funcs[x]].num[y], influence, equ_funcs, depth-1);
			}
		}
	}
}

void Do_Flow_Grouping() {
	
	struct message {
		int depth;
		int *influence;
	};
	
	var_score = (float *)ite_calloc(numinp+1, sizeof(float), 9, "var_score");
	int *var_mark = (int *)ite_calloc(numinp+1, sizeof(int), 9, "var_mark");

	int *equ_funcs = (int *)ite_calloc(numinp+1, sizeof(int), 9, "equ_funcs");
	
	int indep_count = 0;
	int dep_count = 0;
	for(int x = 1; x <= numinp; x++) {
		if(independantVars[x] == 1) {
			indep_count++;
			var_mark[x] = 1;
			if(num_funcs_var_occurs == 0) 
			  {
//				  fprintf(stderr, "\nnon-existent variable\n");
				  continue;
			  }
			  var_score[x] = 1/((float)num_funcs_var_occurs[x]);
		} else {
			dep_count++;
			for(int y = 0; y < nmbrFunctions; y++) {
				if(equalityVble[y] == x) equ_funcs[x] = y;				
			}
		}
	}
//	fprintf(stderr, "\nindep_count = %d\n", indep_count);
//	fprintf(stderr, "\ndep_count = %d\n", dep_count);
	
	message *influences = (message *)ite_calloc(numinp+1, sizeof(message), 9, "influences");
	int *indep_mapping = (int *)ite_calloc(numinp+1, sizeof(int), 9, "indep_mapping");
	int mapping_count = 1;
	
	for(int x = 1; x <= numinp; x++) {
		influences[x].influence = (int *)ite_calloc(indep_count+1, sizeof(int), 9, "influences[x].influence");
		influences[x].depth = 0; //useless cause calloc makes this equal 0
		if(independantVars[x] == 1) { 
			influences[x].influence[mapping_count] = 1;
			indep_mapping[mapping_count++] = x;
		}
	}

	int go_again = 1;
	int times = 20;
	while(go_again == 1 && times > 0) {
		times--;
		go_again = 0;

		for(int x = 1; x <= numinp; x++) {
			if (var_mark[x] == 0) {
//				fprintf(stderr, "trying %d\n", x);
				float score = 0;
				int out = 0;
				for(int y = 0; y < length[equ_funcs[x]]; y++) {
					if(variables[equ_funcs[x]].num[y] == x) continue;
					if(var_score[variables[equ_funcs[x]].num[y]] == 0) {
//						fprintf(stderr, "%d has not yet been set\n", variables[equ_funcs[x]].num[y]);
						out = 1;
						break;
					}
					score+=var_score[variables[equ_funcs[x]].num[y]];
				}
				if(out == 1) {
					go_again = 1;
					continue;
				}
				if((num_funcs_var_occurs[x] - 1) == 0) {
					fprintf(stddbg, "\ndivide by zero in Do_Flow\n");
					break;
					//exit(0);
				}
				var_score[x] = score/((float)(num_funcs_var_occurs[x] - 1));
				if(var_score[x] == 0 && x != numinp+1) {
//					symrec *ptr = getsym_i(x);
//					fprintf(stderr, "%s", ptr->name);
//					fprintf(stderr, "PROBLEM: Zero weight variable");
					//exit(0);
				}
				for(int y = 0; y < length[equ_funcs[x]]; y++) {
					for(int z = 0; z < indep_count; z++) {
						if(variables[equ_funcs[x]].num[y] == x) continue;
						if(influences[variables[equ_funcs[x]].num[y]].influence[z] > 0) {
							influences[x].influence[z]++;
							if(influences[x].depth <= influences[variables[equ_funcs[x]].num[y]].depth)
							  influences[x].depth = influences[variables[equ_funcs[x]].num[y]].depth+1;
						}
					}
				}
				var_mark[x] = 1;
			}
		}
	}

	for(int x = 1; x <= numinp; x++) {
		symrec *ptr = getsym_i(x);
		var_score[x] = var_score[x] + 1; //So no score == 0
		d5_printf5("%d{%d} (%s) = %5.3f ", x, influences[x].depth, (ptr&&ptr->name?ptr->name:"NULL"), var_score[x]);
		if(independantVars[x] == 0) {
			for(int y = 0; y <= indep_count; y++) {
				if(influences[x].influence[y] > 0) {
					ptr = getsym_i(indep_mapping[y]);
					d5_printf3("%d (%s), ", indep_mapping[y], (ptr&&ptr->name?ptr->name:"NULL"));
				}
			}
		}
		d5_printf1("\n");
	}

	/**********************************************************/
	//looking for min cut for each high flow gate backing up 3 or 4

	int CutSize = 10;
	
	d5_printf2("\nCut Size of %d\n", CutSize);
	int *influence = (int *)ite_calloc(numinp+1, sizeof(int), 9, "influence");
	
	for(int x = 1; x <= numinp; x++) {
		int count = numinp+1;
		int smallest = 0;
		if(independantVars[x] == 0) {
			for(int curr_CutSize = CutSize; curr_CutSize >= 0; curr_CutSize--) {
				for(int y = 1; y <= numinp; y++)
				  influence[y] = 0;
//				symrec *ptr = getsym_i(x);
//				d5_printf5("%d{%d} (%s) = %5.3f ", x, influences[x].depth, (ptr&&ptr->name?ptr->name:"NULL"), var_score[x]);
				Get_Cutx(x, influence, equ_funcs, curr_CutSize);
				int temp_count = 0;
				for(int y = 1; y <= numinp; y++) {
					if(influence[y] > 0) {
						temp_count++;
//						ptr = getsym_i(y);
//						d5_printf3("%d (%s), ", y, (ptr&&ptr->name?ptr->name:"NULL"));
					}
				}
				if(count > temp_count) {
					count = temp_count;
					smallest = curr_CutSize;
					symrec *ptr = getsym_i(x);
					d5_printf3("\n%d %d ", count, smallest);
					d5_printf5("%d{%d} (%s) = %5.3f ", x, influences[x].depth, (ptr&&ptr->name?ptr->name:"NULL"), var_score[x]);
					for(int y = 1; y <= numinp; y++) {
						if(influence[y] > 0) {
							temp_count++;
							ptr = getsym_i(y);
							d5_printf3("%d (%s), ", y, (ptr&&ptr->name?ptr->name:"NULL"));
						}
					}
				}
			}
		}
	}
	d5_printf1("\n");
	
	ite_free((void **)&influence);
	
	/**********************************************************/
	//End min cut thing
	
	for(int x = 0; x < dep_count; x++) {
		ite_free((void**)&influences[x].influence);
	}

	ite_free((void**)&indep_mapping);
   ite_free((void**)&influences);
	ite_free((void**)&equ_funcs);
	//ite_free((void**)&var_score); //This should be freed after the brancher returns a solution!
	ite_free((void**)&var_mark);
}
