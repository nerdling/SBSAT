/* =========FOR INTERNAL USE ONLY. NO DISTRIBUTION PLEASE ========== */

/*********************************************************************
 Copyright 1999-2003, University of Cincinnati.  All rights reserved.
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
 any trademark,  service mark, or the name of University of Cincinnati.


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
/*********************************************************
 *  preprocess.c (S. Weaver)
 *********************************************************/

#include "ite.h"


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
					symrec *ptr = getsym_i(x);
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
		d3_printf4("%d (%s) = %5.3f\n", x, (ptr&&ptr->name?ptr->name:"NULL"), var_score[x]);
	}

	ite_free((void *)equ_funcs);
	//ite_free((void *)var_score);
	ite_free((void *)var_mark);
}
