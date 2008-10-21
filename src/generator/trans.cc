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
int *columns;

void print_var_list_reorder2(int **matrix,int sizex, int sizey){
  fprintf(stdout,"'(");

  
  for(int x = 1; x <= sizex; x++){
    for(int y = 1; y < x; y++) {
      if(matrix[y][x]<=1) {
	fprintf(stdout, " trans_%d_%d ", y, x);
      }
      
    }
    for(int y = 1; y <= x; y++) {
      if(matrix[x][y]<=1) {
	fprintf(stdout, " trans_%d_%d ", x, y);
      }
    
    }
  }

  for(int y = 1; y <= sizey; y++){
       fprintf(stdout,"p_%d ",y);
  }

  fprintf(stdout,")\n");
}

void print_var_list_reorder(int **matrix,int sizex, int sizey){
  fprintf(stdout,"'(");

  
  for(int x = 1; x <= sizex; x++){
    for(int y = 1; y <= sizey; y++){
       if(matrix[x][y]<=1)
	 fprintf(stdout,"trans_%d_%d ",x,y);
    }
  }
  
  for(int y = 1; y <= sizey; y++){
       fprintf(stdout,"p_%d ",y);
  }

  fprintf(stdout,")\n");
}

void print_var_list_fn(int **matrix,int sizex, int sizey){

  for(int y = 1; y <= sizey; y++){
       fprintf(stdout,"p_%d ",y);
  }
  
  for(int x = 1; x <= sizex; x++){
    for(int y = 1; y <= sizey; y++){
       if(matrix[x][y]<=1)
	 fprintf(stdout,"trans_%d_%d ",x,y);
    }
  }
  
}



void print_var_list(int **matrix,int sizex, int sizey){
  fprintf(stdout,"'(");
  for(int y = 1; y <= sizey; y++){
       fprintf(stdout,"p_%d ",y);
  }
  
  for(int x = 1; x <= sizex; x++){
    for(int y = 1; y <= sizey; y++){
       if(matrix[x][y]<=1)
	 fprintf(stdout,"trans_%d_%d ",x,y);
    }
  }
  

  fprintf(stdout,")\n");
}

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
	for(int y = 1; y <= sizey; y++) {
		int column = -1;
		for(int x = 1; x <= sizex; x++) {
			if(matrix[x][y]==1 && column==-1) column = 1;
			else if(matrix[x][y]==1 && column==0) column = 2;
			else if(matrix[x][y]==0 && column==-1) column = 0;
			else if(matrix[x][y]==0 && column==1) column = 2;
		}
		columns[y] = column;
	}
}


void gen_matrix_acl2(int **matrix, int** var_matrix, int sizex, int sizey, int mult) {
  int count = sizex;
	for(int x = 1; x <= sizex; x++) {
		int row = -1;
		matrix[x] = (int *)calloc(sizey+1, sizeof(int *));
		var_matrix[x] =  (int *)calloc(sizey+1, sizeof(int *));
		for(int y = 1; y <= sizey; y++) {
			matrix[x][y] = rand()%mult;
			if(x == y) matrix[x][y] = 1;
			if(matrix[x][y] == 1 || matrix[x][y] == 0){
			  var_matrix[x][y] = count;
			  count++;
			}else var_matrix[x][y] = -1;

			//fprintf(stdout, "%d", matrix[x][y]);
			if(matrix[x][y]==1 && row==-1) row = 1;
			else if(matrix[x][y]==1 && row==0) row = 2;
			else if(matrix[x][y]==0 && row==-1) row = 0;
			else if(matrix[x][y]==0 && row==1) row = 2;
		}
		rows[x] = row;
		//fprintf(stdout, "\n");
	}
	/*
	for(int x = 1; x <= sizex; x++) {
	  for(int y = 1; y <= sizey; y++) {
	    	fprintf(stdout, "%d", var_matrix[x][y]);
	  }
	  fprintf(stdout, "\n");
	}
	*/

	for(int y = 1; y <= sizey; y++) {
		int column = -1;
		for(int x = 1; x <= sizex; x++) {
			if(matrix[x][y]==1 && column==-1) column = 1;
			else if(matrix[x][y]==1 && column==0) column = 2;
			else if(matrix[x][y]==0 && column==-1) column = 0;
			else if(matrix[x][y]==0 && column==1) column = 2;
		}
		columns[y] = column;
	}
}


void gen_matrix_acl2_reorder2(int **matrix, int** var_matrix, int sizex, int sizey, int mult) {


	for(int x = 1; x <= sizex; x++) {
		int row = -1;
		matrix[x] = (int *)calloc(sizey+1, sizeof(int *));
		var_matrix[x] =  (int *)calloc(sizey+1, sizeof(int *));
		for(int y = 1; y <= sizey; y++) {
			matrix[x][y] = rand()%mult;
			if(x == y) matrix[x][y] = 1;

			if(matrix[x][y]==1 && row==-1) row = 1;
			else if(matrix[x][y]==1 && row==0) row = 2;
			else if(matrix[x][y]==0 && row==-1) row = 0;
			else if(matrix[x][y]==0 && row==1) row = 2;
		}
		rows[x] = row;
		//fprintf(stdout, "\n");
	}

	for(int y = 1; y <= sizey; y++) {
		int column = -1;
		for(int x = 1; x <= sizex; x++) {
			if(matrix[x][y]==1 && column==-1) column = 1;
			else if(matrix[x][y]==1 && column==0) column = 2;
			else if(matrix[x][y]==0 && column==-1) column = 0;
			else if(matrix[x][y]==0 && column==1) column = 2;
		}
		columns[y] = column;
	}

	int count = 0;
	for(int x = 1; x <= sizex; x++) {
		for(int y = 1; y < x; y++) {
			if(matrix[y][x] == 1 || matrix[y][x] == 0){
			  var_matrix[y][x] = count;
			  count++;
			}else var_matrix[y][x] = -1;
		}

		for(int y = 1; y <= x; y++) {
			if(matrix[x][y] == 1 || matrix[x][y] == 0){
			  var_matrix[x][y] = count;
			  count++;
			}else var_matrix[x][y] = -1;
		}

	}
	
}

void gen_matrix_acl2_reorder(int **matrix, int** var_matrix, int sizex, int sizey, int mult) {
  int count = 0;
	for(int x = 1; x <= sizex; x++) {
		int row = -1;
		matrix[x] = (int *)calloc(sizey+1, sizeof(int *));
		var_matrix[x] =  (int *)calloc(sizey+1, sizeof(int *));
		for(int y = 1; y <= sizey; y++) {
			matrix[x][y] = rand()%mult;
			if(x == y) matrix[x][y] = 1;
			if(matrix[x][y] == 1 || matrix[x][y] == 0){
			  var_matrix[x][y] = count;
			  count++;
			}else var_matrix[x][y] = -1;

			//fprintf(stdout, "%d", matrix[x][y]);
			if(matrix[x][y]==1 && row==-1) row = 1;
			else if(matrix[x][y]==1 && row==0) row = 2;
			else if(matrix[x][y]==0 && row==-1) row = 0;
			else if(matrix[x][y]==0 && row==1) row = 2;
		}
		rows[x] = row;
		//fprintf(stdout, "\n");
	}
	/*
	for(int x = 1; x <= sizex; x++) {
	  for(int y = 1; y <= sizey; y++) {
	    	fprintf(stdout, "%d", var_matrix[x][y]);
	  }
	  fprintf(stdout, "\n");
	}
	*/

	for(int y = 1; y <= sizey; y++) {
		int column = -1;
		for(int x = 1; x <= sizex; x++) {
			if(matrix[x][y]==1 && column==-1) column = 1;
			else if(matrix[x][y]==1 && column==0) column = 2;
			else if(matrix[x][y]==0 && column==-1) column = 0;
			else if(matrix[x][y]==0 && column==1) column = 2;
		}
		columns[y] = column;
	}
}


void decrement_var_matrix(int** var_matrix,int x,int y,int sizex,int sizey){
	




  for(int i=1;i<=sizex;i++){
    for(int j=1;j<=sizey;j++){
      if(i<x) continue;
      if(i==x && j<y) continue;
      if(var_matrix[i][j] != -1)
	var_matrix[i][j]--;
    }
  }


	

}


void decrement_var_matrix_reorder2(int** var_matrix,int x,int y,int sizex,int sizey){

  int curr_val = var_matrix[x][y];


  for(int i = 1; i <= sizex; i++) {
    for(int j = 1; j <= sizey; j++){
      if (var_matrix[i][j] >= curr_val)
	var_matrix[i][j]--;
    }
  }

} 

void decrement_var_matrix_reorder(int** var_matrix,int x,int y,int sizex,int sizey){

  for(int i=1;i<=sizex;i++){
    for(int j=1;j<=sizey;j++){
      if(i<x) continue;
      if(i==x && j<y) continue;
      if(var_matrix[i][j] != -1)
	var_matrix[i][j]--;
    }
  }
}

void update_vars_reorder2(int** var_matrix,int** matrix,int x,int sizex,int sizey){
  
  for(int y = 1; y <= x; y++) {
    
    if(x!=y && matrix[y][x]<=1) {
      decrement_var_matrix_reorder2(var_matrix,y,x,sizex,sizey);
    }
    
  }
  for(int y = 1; y <= x; y++) {
    if(matrix[x][y]<=1) {
      decrement_var_matrix_reorder2(var_matrix,x,y,sizex,sizey);
    }
    
  }

  
  for(int y = 1; y <= x; y++) {
    if(x!=y && matrix[y][x]<=1) {		    
      matrix[y][x] = 3;
      var_matrix[y][x] = -1;
    }
    if(matrix[x][y]<=1) {
      matrix[x][y] = 3;
      var_matrix[y][x] = -1;
    }
  }
}



void update_vars_reorder(int** var_matrix,int** matrix,int x,int sizex,int sizey){
  
  for(int y = 1; y <= x; y++) {
    
    if(x!=y && matrix[y][x]<=1) {
      decrement_var_matrix_reorder(var_matrix,y,x,sizex,sizey);
    }
    
  }
  for(int y = 1; y <= x; y++) {
    if(matrix[x][y]<=1) {
      decrement_var_matrix_reorder(var_matrix,x,y,sizex,sizey);
    }
    
  }

  
  for(int y = 1; y <= x; y++) {
    if(x!=y && matrix[y][x]<=1) {		    
      matrix[y][x] = 3;
      var_matrix[y][x] = -1;
    }
    if(matrix[x][y]<=1) {
      matrix[x][y] = 3;
      var_matrix[y][x] = -1;
    }
  }
}



void update_vars(int** var_matrix,int** matrix,int x,int sizex,int sizey){

  for(int y = 1; y <= x; y++) {
    
    if(x!=y && matrix[y][x]<=1) {
      decrement_var_matrix(var_matrix,y,x,sizex,sizey);
    }
    
  }
  for(int y = 1; y <= x; y++) {
    if(matrix[x][y]<=1) {
      decrement_var_matrix(var_matrix,x,y,sizex,sizey);
    }
    
  }


  for(int y = 1; y <= x; y++) {
    if(x!=y && matrix[y][x]<=1) {		    
      matrix[y][x] = 3;
      var_matrix[y][x] = -1;
    }
    if(matrix[x][y]<=1) {
      matrix[x][y] = 3;
      var_matrix[y][x] = -1;
    }
  }
}


void trans(int size, int mult, int seed) {
	struct timeval tv;
	gettimeofday(&tv, NULL);
	if(seed == 0) seed = tv.tv_usec;
	//seed = 414115;
	//seed = 981369;
	
	fprintf(stderr, "seed = %d\n", seed);
	srand(seed);
	
	
	int sizex = size;
	int sizey = size;
	int **matrix = (int **)calloc(sizex+1, sizeof(int **));
	rows = (int*)calloc(sizex+1, sizeof(int*));
	columns = (int*)calloc(sizey+1, sizeof(int*));
	gen_matrix(matrix, sizex, sizey, mult);
	fprintf(stdout, "p bdd %d %d\n", 6*sizex*sizey, (5*sizex*sizey)+2*sizex+1);

	//fprintf(stdout, "initial_branch(#1 p_*)\n");
	fprintf(stdout, "initial_branch(#1 ");
	for(int x = 1; x <= sizex; x++)
		fprintf(stdout, "p_%d ", x);
	fprintf(stdout, ")\n");
	//fprintf(stdout, "order(");

	//for(int x = 1; x <= sizex; x++) {
	//	for(int y = 1; y <= sizey; y++) {
	//		if(matrix[x][y]<=1) fprintf(stdout, "trans_%d_%d ", x, y);
	//	}
	//	fprintf(stdout, "p_%d ", x);
	//}

	//for(int x = sizex; x > 0; x--)
	//  fprintf(stdout, "p_%d ", x);

	//fprintf(stdout, ")\n");

	//for(int x = sizex; x > 0; x--) {
	//	for(int y = 1; y <= x; y++) {
	//		if(matrix[x][y]<=1) fprintf(stdout, "exist(");
	//		if(x!=y && matrix[y][x]<=1) fprintf(stdout, "exist(");
	//	}
	//	fprintf(stdout, "and(");
	//}
	
	fprintf(stdout, "\n");
	
	for(int y = 1; y <= sizey; y++) {
	  	if(columns[y]==1) {
			//fprintf(stdout, "var(p_%d)\n", y);
		} else if(columns[y]==0) {
			//fprintf(stdout, "not(p_%d)\n", y);
		} else if(columns[y]==-1) {
			fprintf(stderr, "Column %d is empty - unsatisfiable\n", y);
			exit(0);
		} else { //columns[y] >= 2
	  
			for(int x = 1; x <= sizex; x++) {
				if(matrix[x][y]==1) fprintf(stdout, "imp(trans_%d_%d, p_%d)\n", x, y, y);
				else if(matrix[x][y]==0) fprintf(stdout, "imp(trans_%d_%d, not(p_%d))\n", x, y, y);
			}
				}
	}
	
	for(int x = 1; x <= sizex; x++) {
		fprintf(stdout, "minmax(1, 1");
		for(int y = 1; y <= sizey; y++) {
			if(matrix[x][y]<=1) fprintf(stdout, ", trans_%d_%d", x, y);
		}
		fprintf(stdout, ")\n");
		
		fprintf(stdout, "minmax(1, 1");
		for(int y = 1; y <= sizey; y++) {
			if(matrix[y][x]<=1) fprintf(stdout, ", trans_%d_%d", y, x);
		}
		fprintf(stdout, ")\n");
		//fprintf(stdout, "))\n");		

		//for(int y = 1; y <= x; y++) {
		//	if(matrix[x][y]<=1) fprintf(stdout, ", trans_%d_%d)", x, y);
		//	if(x!=y && matrix[y][x]<=1) fprintf(stdout, ", trans_%d_%d)", y, x);
		//}

		//fprintf(stdout, "\n");
	}

	//fprintf(stdout, "print_tree($1)\n");
	//fprintf(stdout, "countT($1, ");
	//for(int x = sizex; x > 0; x--)
	//	  fprintf(stdout, "p_%d ", x);
	//fprintf(stdout, ")\n");

}


void trans_acl2_fn(int size, int mult,int seed) {
	struct timeval tv;
	gettimeofday(&tv, NULL);
	if(seed == 0) seed = tv.tv_usec;
	//	seed = 414115;
	//seed = 981369;
	
	fprintf(stderr, "seed = %d\n", seed);
	srand(seed);
	
	int sizex = size;
	int sizey = size;
	int **matrix = (int **)calloc(sizex+1, sizeof(int **));
	rows = (int*)calloc(sizex+1, sizeof(int*));
	columns = (int*)calloc(sizey+1, sizeof(int*));

	int **var_matrix = (int **)calloc(sizex+1, sizeof(int **));
	gen_matrix_acl2(matrix, var_matrix,sizex, sizey, mult);
   //	fprintf(stdout, "p bdd %d %d\n", 6*sizex*sizey, (5*sizex*sizey)+2*sizex+1);

   //	fprintf(stdout, "initial_branch(#1 p_*)\n");
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

	fprintf(stdout,"(ctv ");

	
	fprintf(stdout, "\n");
	fprintf(stdout, "(defn m-and-fn (");
	print_var_list_fn(matrix,sizex,sizey);
	fprintf(stdout,")\n");

	

	fprintf(stdout, "(multi-and `(\n");
	for(int y = 1; y <= sizey; y++) {
	  /* 	  		if(columns[y]==1) {
			//fprintf(stdout, "var(p_%d)\n", y);
		} else if(columns[y]==0) {
			//fprintf(stdout, "not(p_%d)\n", y);
		} else if(columns[y]==-1) {
			fprintf(stderr, "Column %d is empty - unsatisfiable\n", y);
			exit(0);
		} else { //columns[y] >= 2
	  */
       		for(int x = 1; x <= sizex; x++) {
		  if(matrix[x][y]==1) fprintf(stdout, ",(q-implies-ite trans_%d_%d p_%d)\n", x,y, y);
		  else if(matrix[x][y]==0) fprintf(stdout, ",(q-implies-ite trans_%d_%d (q-not p_%d))\n", x,y, y);
			}
		//					}
	}
	fprintf(stdout,"))\n");

	for(int x = sizex; x > 0; x--) {
	  if(x != sizex)
	    fprintf(stdout, "(q-and ");
		for(int y = 1; y <= x; y++) {
		  if(matrix[x][y]<=1) { fprintf(stdout, "(q-exists-shrink "); break;}
		  if(x!=y && matrix[y][x]<=1) { fprintf(stdout, "(q-exists-shrink "); break;}
		}
		//	fprintf(stdout, "(q-and ");
	}


	for(int x = 1; x <= sizex; x++) {
		fprintf(stdout, "\n(q-and (minmax 1 1 `(");
		for(int y = 1; y <= sizey; y++) {
			if(matrix[x][y]<=1) fprintf(stdout, " ,(qvar-n %d)", var_matrix[x][y]);
		}
		fprintf(stdout, "))\n");
		
		fprintf(stdout, "(minmax 1 1 `(");
		for(int y = 1; y <= sizey; y++) {
		  if(matrix[y][x]<=1) fprintf(stdout, " ,(qvar-n %d)", var_matrix[y][x]);
		}
		fprintf(stdout, "))))\n");

		fprintf(stdout,"'(");

		for(int y = 1; y <= x; y++) {

		  if(x!=y && matrix[y][x]<=1) {
		    fprintf(stdout, " trans_%d_%d ", y, x);
		  }

		}
		for(int y = 1; y <= x; y++) {
		  if(matrix[x][y]<=1) {
		    fprintf(stdout, " trans_%d_%d ", x, y);
		  }

		}		

		fprintf(stdout, ")\n");
		
		print_var_list(matrix,sizex,sizey);
		update_vars(var_matrix,matrix,x,sizex,sizey);



		fprintf(stdout,")");


	}
	fprintf(stdout,")\n");
	//fprintf(stdout,"'(");
	//	for(int x = 1; x <= sizex; x++){
	//  for(int y = 1; y <= sizey; y++){
	//    fprintf(stdout,"trans_%d_%d ",x,y);
	//  }
	//}
	//fprintf(stdout,")\n");
	/*
	fprintf(stdout,"'(");
	for(int y = 1; y <= sizey; y++){
	  fprintf(stdout,"p_%d ",y);
	}
	for(int x = 1; x <= sizex; x++){
	  for(int y = 1; y <= sizey; y++){
	    fprintf(stdout,"trans_%d_%d ",x,y);
	  }
	}
	fprintf(stdout,")))\n");
	*/
	//fprintf(stdout, "print_tree($1)\n");
	//fprintf(stdout, "countT($1, ");
	//for(int x = sizex; x > 0; x--)
	//	  fprintf(stdout, "p_%d ", x);
	//fprintf(stdout, ")\n");

}



void trans_acl2(int size, int mult,int seed) {
	struct timeval tv;
	gettimeofday(&tv, NULL);
	if(seed == 0) seed = tv.tv_usec;
	//	seed = 414115;
	//seed = 981369;
	
	fprintf(stderr, "seed = %d\n", seed);
	srand(seed);
	
	int sizex = size;
	int sizey = size;
	int **matrix = (int **)calloc(sizex+1, sizeof(int **));
	rows = (int*)calloc(sizex+1, sizeof(int*));
	columns = (int*)calloc(sizey+1, sizeof(int*));

	int **var_matrix = (int **)calloc(sizex+1, sizeof(int **));
	gen_matrix_acl2(matrix, var_matrix,sizex, sizey, mult);
   //	fprintf(stdout, "p bdd %d %d\n", 6*sizex*sizey, (5*sizex*sizey)+2*sizex+1);

   //	fprintf(stdout, "initial_branch(#1 p_*)\n");
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

	fprintf(stdout,"(ctv ");



	for(int x = sizex; x > 0; x--) {
	  if(x != sizex)
	    fprintf(stdout, "(q-and ");
		for(int y = 1; y <= x; y++) {
		  if(matrix[x][y]<=1) { fprintf(stdout, "(q-exists-shrink "); break;}
		  if(x!=y && matrix[y][x]<=1) { fprintf(stdout, "(q-exists-shrink "); break;}
		}
		//	fprintf(stdout, "(q-and ");
	}
	
	fprintf(stdout, "\n");
	fprintf(stdout, "(q-and (multi-and `(\n");
	for(int y = 1; y <= sizey; y++) {
	  /* 	  		if(columns[y]==1) {
			//fprintf(stdout, "var(p_%d)\n", y);
		} else if(columns[y]==0) {
			//fprintf(stdout, "not(p_%d)\n", y);
		} else if(columns[y]==-1) {
			fprintf(stderr, "Column %d is empty - unsatisfiable\n", y);
			exit(0);
		} else { //columns[y] >= 2
	  */
       		for(int x = 1; x <= sizex; x++) {
				if(matrix[x][y]==1) fprintf(stdout, ",(q-implies-ite (qvar-n %d) (qvar-n %d))\n", var_matrix[x][y], y-1);
				else if(matrix[x][y]==0) fprintf(stdout, ",(q-implies-ite (qvar-n %d) (q-not (qvar-n %d)))\n", var_matrix[x][y], y-1);
			}
		//					}
	}
	fprintf(stdout,"))\n");
	for(int x = 1; x <= sizex; x++) {
		fprintf(stdout, "\n(q-and (minmax 1 1 `(");
		for(int y = 1; y <= sizey; y++) {
			if(matrix[x][y]<=1) fprintf(stdout, " ,(qvar-n %d)", var_matrix[x][y]);
		}
		fprintf(stdout, "))\n");
		
		fprintf(stdout, "(minmax 1 1 `(");
		for(int y = 1; y <= sizey; y++) {
		  if(matrix[y][x]<=1) fprintf(stdout, " ,(qvar-n %d)", var_matrix[y][x]);
		}
		fprintf(stdout, "))))\n");

		fprintf(stdout,"'(");

		for(int y = 1; y <= x; y++) {

		  if(x!=y && matrix[y][x]<=1) {
		    fprintf(stdout, " trans_%d_%d ", y, x);
		  }

		}
		for(int y = 1; y <= x; y++) {
		  if(matrix[x][y]<=1) {
		    fprintf(stdout, " trans_%d_%d ", x, y);
		  }

		}		

		fprintf(stdout, ")\n");
		
		print_var_list(matrix,sizex,sizey);
		update_vars(var_matrix,matrix,x,sizex,sizey);



		fprintf(stdout,")");


	}
	fprintf(stdout,")\n");
	//fprintf(stdout,"'(");
	//	for(int x = 1; x <= sizex; x++){
	//  for(int y = 1; y <= sizey; y++){
	//    fprintf(stdout,"trans_%d_%d ",x,y);
	//  }
	//}
	//fprintf(stdout,")\n");
	/*
	fprintf(stdout,"'(");
	for(int y = 1; y <= sizey; y++){
	  fprintf(stdout,"p_%d ",y);
	}
	for(int x = 1; x <= sizex; x++){
	  for(int y = 1; y <= sizey; y++){
	    fprintf(stdout,"trans_%d_%d ",x,y);
	  }
	}
	fprintf(stdout,")))\n");
	*/
	//fprintf(stdout, "print_tree($1)\n");
	//fprintf(stdout, "countT($1, ");
	//for(int x = sizex; x > 0; x--)
	//	  fprintf(stdout, "p_%d ", x);
	//fprintf(stdout, ")\n");

}


void trans_acl2_reorder(int size, int mult,int seed) {
	struct timeval tv;
	gettimeofday(&tv, NULL);
	if(seed == 0) seed = tv.tv_usec;
	//	seed = 414115;
	//seed = 981369;
	
	fprintf(stderr, "seed = %d\n", seed);
	srand(seed);
	
	int sizex = size;
	int sizey = size;
	int **matrix = (int **)calloc(sizex+1, sizeof(int **));
	rows = (int*)calloc(sizex+1, sizeof(int*));
	columns = (int*)calloc(sizey+1, sizeof(int*));

	int **var_matrix = (int **)calloc(sizex+1, sizeof(int **));
	gen_matrix_acl2_reorder(matrix, var_matrix,sizex, sizey, mult);
fprintf(stdout,"\n");
	for(int x=1;x<sizex;x++){
	  for(int y=1;y<sizey;y++){
	    fprintf(stdout,"%d ",matrix[x][y]);
	  }
	  fprintf(stdout,"\n");
	}
fprintf(stdout,"\n");
   //	fprintf(stdout, "p bdd %d %d\n", 6*sizex*sizey, (5*sizex*sizey)+2*sizex+1);

   //	fprintf(stdout, "initial_branch(#1 p_*)\n");
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

	fprintf(stdout,"(ctv ");



	for(int x = sizex; x > 0; x--) {
	  if(x != sizex)
	    fprintf(stdout, "(q-and ");
		for(int y = 1; y <= x; y++) {
		  if(matrix[x][y]<=1) { fprintf(stdout, "(q-exists-shrink "); break;}
		  if(x!=y && matrix[y][x]<=1) { fprintf(stdout, "(q-exists-shrink "); break;}
		}
	}
	
	fprintf(stdout, "\n");

	//	for(int y = 1; y <= sizey; y++) {
	  /* 	  		if(columns[y]==1) {
			//fprintf(stdout, "var(p_%d)\n", y);
		} else if(columns[y]==0) {
			//fprintf(stdout, "not(p_%d)\n", y);
		} else if(columns[y]==-1) {
			fprintf(stderr, "Column %d is empty - unsatisfiable\n", y);
			exit(0);
		} else { //columns[y] >= 2
	  */
	//  		for(int x = 1; x <= sizex; x++) {
	//				if(matrix[x][y]==1) fprintf(stdout, ",(q-implies-ite (qvar-n %d) (qvar-n %d))\n", var_matrix[x][y], y-1);
				//				else if(matrix[x][y]==0) fprintf(stdout, ",(q-implies-ite (qvar-n %d) (q-not (qvar-n %d)))\n", var_matrix[x][y], y-1);
	//			}
		//					}
	//	}


	int first = 1;
	int varcount;
	for(int x = 1; x <= sizex; x++) {
	  varcount = 0;
	  fprintf(stdout,"\n");

	  for(int i=1;i<=sizex;i++){
	    for(int j=1;j<=sizey;j++){
	      if(matrix[i][j] <= 1) varcount++;
	    }
	  }

		for(int y = 1; y <= x; y++) {
		  if(matrix[x][y] <= 1 &&
		     columns[y]!=1 &&
		     columns[y]!=0)  fprintf(stdout,"(q-and ");
		  if(x!=y && matrix[y][x]<=1 &&
		     columns[x]!=1 &&
		     columns[x]!=0)  fprintf(stdout,"(q-and ");
		}

		if(first){
		  

		  for(int y = 1; y < sizey; y++) {
		    if(columns[y]==1) {
		      fprintf(stdout, "\n(q-and (qvar-n %d)\n", y-1+varcount);
		    } else if(columns[y]==0) {
		      fprintf(stdout, "\n(q-and (qvar-n %d)\n", y-1+varcount);
		    }
		  }
		}

		fprintf(stdout, "(q-and \n(minmax 1 1 `(");
		for(int y = 1; y <= sizey; y++) {

	
			if(matrix[x][y]<=1) fprintf(stdout, " ,(qvar-n %d)", var_matrix[x][y]);
		}
		fprintf(stdout, "))\n");
		
		fprintf(stdout, "(minmax 1 1 `(");
		for(int y = 1; y <= sizey; y++) {
		  if(matrix[y][x]<=1) fprintf(stdout, " ,(qvar-n %d)", var_matrix[y][x]);
		}
		fprintf(stdout, ")))\n");

		if(first){
		  

		  for(int y = 1; y < sizey; y++) {
		    if(columns[y]==1) {
		      fprintf(stdout, ")");
		    } else if(columns[y]==0) {
		      fprintf(stdout, ")");
		    }
		  }
		}


		for(int y = 1; y < x; y++) {
		  if(columns[y]==1) {
		    //fprintf(stdout, "\nvar(%d)\n", y-1+varcount);
		  } else if(columns[y]==0) {
		    //fprintf(stdout, "\nnot(%d)\n", y-1+varcount);
		  } else if(columns[y]==-1) {
		    fprintf(stderr, "\nColumn %d is empty - unsatisfiable\n", y-1+varcount);
		    exit(0);
		  }else{

		  if(matrix[x][y]==1) fprintf(stdout, "\n(q-implies-ite (qvar-n %d) (qvar-n %d)))", var_matrix[x][y], y-1+varcount);
		  else if(matrix[x][y]==0) fprintf(stdout, "\n(q-implies-ite (qvar-n %d) (q-not (qvar-n %d))))", var_matrix[x][y], y-1+varcount);
		  }
		}
		for(int y = 1; y <= x; y++) {
		  if(columns[x]==1) {
		    //fprintf(stdout, "\nvar(%d)\n", x-1+varcount);
		  } else if(columns[x]==0) {
		    //fprintf(stdout, "\nnot(%d)\n", x-1+varcount);
		  } else if(columns[x]==-1) {
		    fprintf(stderr, "\nColumn %d is empty - unsatisfiable\n", x-1+varcount);
		    exit(0);
		  }else{

		    if(matrix[y][x]==1) fprintf(stdout, "\n(q-implies-ite (qvar-n %d) (qvar-n %d)))", var_matrix[y][x], x-1+varcount);
		    else if(matrix[y][x]==0) fprintf(stdout, "\n(q-implies-ite (qvar-n %d) (q-not (qvar-n %d))))", var_matrix[y][x], x-1+varcount);
		  }
		}


		if(!first){
		  fprintf(stdout, "\n)\n");
		}
		else first = 0;

		fprintf(stdout,"'(");



		for(int y = 1; y <= x; y++) {

		  if(x!=y && matrix[y][x]<=1) {
		    fprintf(stdout, " trans_%d_%d ", y, x);
		  }

		}
		for(int y = 1; y <= x; y++) {
		  if(matrix[x][y]<=1) {
		    fprintf(stdout, " trans_%d_%d ", x, y);
		  }

		}		

		fprintf(stdout, ")\n");
		
		print_var_list_reorder(matrix,sizex,sizey);
		update_vars(var_matrix,matrix,x,sizex,sizey);



		fprintf(stdout,")");


	}
	fprintf(stdout,")\n");
}


void trans_acl2_reorder2(int size, int mult,int seed) {
	struct timeval tv;
	gettimeofday(&tv, NULL);
	if(seed == 0) seed = tv.tv_usec;
	//	seed = 414115;
	//seed = 981369;
	
	fprintf(stderr, "seed = %d\n", seed);
	srand(seed);
	
	int sizex = size;
	int sizey = size;
	int **matrix = (int **)calloc(sizex+1, sizeof(int **));
	rows = (int*)calloc(sizex+1, sizeof(int*));
	columns = (int*)calloc(sizey+1, sizeof(int*));

	int **var_matrix = (int **)calloc(sizex+1, sizeof(int **));
	gen_matrix_acl2_reorder2(matrix, var_matrix,sizex, sizey, mult);
fprintf(stdout,"\n");
	for(int x=1;x<sizex;x++){
	  for(int y=1;y<sizey;y++){
	    fprintf(stdout,"%d ",matrix[x][y]);
	  }
	  fprintf(stdout,"\n");
	}
fprintf(stdout,"\n");
	fprintf(stdout,"(ctv ");



	for(int x = sizex; x > 0; x--) {
	  if(x != sizex)
	    fprintf(stdout, "(q-and ");
		for(int y = 1; y <= x; y++) {
		  if(matrix[x][y]<=1) { fprintf(stdout, "(q-exists-shrink "); break;}
		  if(x!=y && matrix[y][x]<=1) { fprintf(stdout, "(q-exists-shrink "); break;}
		}
	}
	
	fprintf(stdout, "\n");

	//	for(int y = 1; y <= sizey; y++) {
	  /* 	  		if(columns[y]==1) {
			//fprintf(stdout, "var(p_%d)\n", y);
		} else if(columns[y]==0) {
			//fprintf(stdout, "not(p_%d)\n", y);
		} else if(columns[y]==-1) {
			fprintf(stderr, "Column %d is empty - unsatisfiable\n", y);
			exit(0);
		} else { //columns[y] >= 2
	  */
	//  		for(int x = 1; x <= sizex; x++) {
	//				if(matrix[x][y]==1) fprintf(stdout, ",(q-implies-ite (qvar-n %d) (qvar-n %d))\n", var_matrix[x][y], y-1);
				//				else if(matrix[x][y]==0) fprintf(stdout, ",(q-implies-ite (qvar-n %d) (q-not (qvar-n %d)))\n", var_matrix[x][y], y-1);
	//			}
		//					}
	//	}


	int first = 1;
	int varcount;
	for(int x = 1; x <= sizex; x++) {

	  varcount = 0;
	  fprintf(stdout,"\n");

	  for(int i=1;i<=sizex;i++){
	    for(int j=1;j<=sizey;j++){
	      if(matrix[i][j] <= 1) varcount++;
	    }
	  }
	  

	  fprintf(stdout,"\n");

		for(int y = 1; y <= x; y++) {
		  if(matrix[x][y] <= 1 &&
		     columns[y]!=1 &&
		     columns[y]!=0)  fprintf(stdout,"(q-and ");
		  if(x!=y && matrix[y][x]<=1 &&
		     columns[x]!=1 &&
		     columns[x]!=0)  fprintf(stdout,"(q-and ");
		}

		if(first){
		  

		  for(int y = 1; y < sizey; y++) {
		    if(columns[y]==1) {
		      fprintf(stdout, "\n(q-and (qvar-n %d)\n", y+varcount-1);
		    } else if(columns[y]==0) {
		      fprintf(stdout, "\n(q-and (qvar-n %d)\n", y+varcount-1);
		    }
		  }
		}

		fprintf(stdout, "(q-and \n(minmax 1 1 `(");
		for(int y = 1; y <= sizey; y++) {

	
			if(matrix[x][y]<=1) fprintf(stdout, " ,(qvar-n %d)", var_matrix[x][y]);
		}
		fprintf(stdout, "))\n");
		
		fprintf(stdout, "(minmax 1 1 `(");
		for(int y = 1; y <= sizey; y++) {
		  if(matrix[y][x]<=1) fprintf(stdout, " ,(qvar-n %d)", var_matrix[y][x]);
		}
		fprintf(stdout, ")))\n");
		if(first){
		  

		  for(int y = 1; y < sizey; y++) {
		    if(columns[y]==1) {
		      fprintf(stdout, ")");
		    } else if(columns[y]==0) {
		      fprintf(stdout, ")");
		    }
		  }
		}


		for(int y = 1; y < x; y++) {
		  if(columns[y]==1) {
		    //fprintf(stdout, "\nvar(%d)\n", y+varcount-1);
		  } else if(columns[y]==0) {
		    //fprintf(stdout, "\nnot(%d)\n", y+varcount-1);
		  } else if(columns[y]==-1) {
		    fprintf(stderr, "\nColumn %d is empty - unsatisfiable\n", y);
		    exit(0);
		  }else{

		  if(matrix[x][y]==1) fprintf(stdout, "\n(q-implies-ite (qvar-n %d) (qvar-n %d)))", var_matrix[x][y], y+varcount-1);
		  else if(matrix[x][y]==0) fprintf(stdout, "\n(q-implies-ite (qvar-n %d) (q-not (qvar-n %d))))", var_matrix[x][y], y+varcount-1);
		  }
		}
		for(int y = 1; y <= x; y++) {
		  if(columns[x]==1) {
		    //fprintf(stdout, "\nvar(%d)\n", x+varcount-1);
		  } else if(columns[x]==0) {
		    //fprintf(stdout, "\nnot(%d)\n", x+varcount-1);
		  } else if(columns[x]==-1) {
		    fprintf(stderr, "\nColumn %d is empty - unsatisfiable\n", x+varcount-1);
		    exit(0);
		  }else{

		    if(matrix[y][x]==1) fprintf(stdout, "\n(q-implies-ite (qvar-n %d) (qvar-n %d)))", var_matrix[y][x], x+varcount-1);
		    else if(matrix[y][x]==0) fprintf(stdout, "\n(q-implies-ite (qvar-n %d) (q-not (qvar-n %d))))", var_matrix[y][x], x+varcount-1);
		  }
		}



		if(!first){
		  fprintf(stdout, "\n)");
		}
		else first = 0;

		fprintf(stdout, "\n");

		fprintf(stdout,"'(");


		for(int y = 1; y <= x; y++) {

		  if(x!=y && matrix[y][x]<=1) {
		    fprintf(stdout, " trans_%d_%d ", y, x);
		  }

		}
		for(int y = 1; y <= x; y++) {
		  if(matrix[x][y]<=1) {
		    fprintf(stdout, " trans_%d_%d ", x, y);
		  }

		}		

		fprintf(stdout, ")\n");
		
		print_var_list_reorder2(matrix,sizex,sizey);
		update_vars_reorder2(var_matrix,matrix,x,sizex,sizey);



		fprintf(stdout,")");


	}
	fprintf(stdout,")\n");
}


void trans_old_acl2(int size, int mult,int seed) {
	struct timeval tv;
	gettimeofday(&tv, NULL);
	if(seed == 0) seed = tv.tv_usec;
	//	seed = 414115;
	//seed = 981369;
	
	fprintf(stderr, "seed = %d\n", seed);
	srand(seed);
	
	int sizex = size;
	int sizey = size;
	int **matrix = (int **)calloc(sizex+1, sizeof(int **));
	rows = (int*)calloc(sizex+1, sizeof(int*));
	columns = (int*)calloc(sizey+1, sizeof(int*));
	gen_matrix(matrix, sizex, sizey, mult);
   //	fprintf(stdout, "p bdd %d %d\n", 6*sizex*sizey, (5*sizex*sizey)+2*sizex+1);

   //	fprintf(stdout, "initial_branch(#1 p_*)\n");
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

	fprintf(stdout,"(ctv ");

	for(int x = sizex; x > 0; x--) {
	  if(x != sizex)
	    fprintf(stdout, "(q-and ");
		for(int y = 1; y <= x; y++) {
		  if(matrix[x][y]<=1) { fprintf(stdout, "(q-exists-shrink "); break;}
		  if(x!=y && matrix[y][x]<=1) { fprintf(stdout, "(q-exists-shrink "); break;}
		}
		//	fprintf(stdout, "(q-and ");
	}
	
	fprintf(stdout, "\n");
	fprintf(stdout, "(q-and (car (qnorm-list `(,(multi-and `(\n");
	for(int y = 1; y <= sizey; y++) {
	  /* 	  		if(columns[y]==1) {
			//fprintf(stdout, "var(p_%d)\n", y);
		} else if(columns[y]==0) {
			//fprintf(stdout, "not(p_%d)\n", y);
		} else if(columns[y]==-1) {
			fprintf(stderr, "Column %d is empty - unsatisfiable\n", y);
			exit(0);
		} else { //columns[y] >= 2
	  */
       		for(int x = 1; x <= sizex; x++) {
				if(matrix[x][y]==1) fprintf(stdout, "(implies trans_%d_%d p_%d)\n", x, y, y);
				else if(matrix[x][y]==0) fprintf(stdout, "(implies trans_%d_%d (not p_%d))\n", x, y, y);
			}
		//					}
	}
	fprintf(stdout,")))\n");
	print_var_list(matrix,sizex,sizey);
	fprintf(stdout,"))\n");
	for(int x = 1; x <= sizex; x++) {
		fprintf(stdout, "\n(car (qnorm-list `((and ,(minmax 1 1 '(");
		for(int y = 1; y <= sizey; y++) {
			if(matrix[x][y]<=1) fprintf(stdout, " trans_%d_%d", x, y);
		}
		fprintf(stdout, "))\n");
		
		fprintf(stdout, ",(minmax 1 1 '(");
		for(int y = 1; y <= sizey; y++) {
			if(matrix[y][x]<=1) fprintf(stdout, " trans_%d_%d", y, x);
		}
		fprintf(stdout, "))))\n");
		//fprintf(stdout, "))\n");		
		print_var_list(matrix,sizex,sizey);
		fprintf(stdout,")))\n");
		fprintf(stdout,"'(");

		for(int y = 1; y <= x; y++) {

		  if(x!=y && matrix[y][x]<=1) {
		    fprintf(stdout, " trans_%d_%d ", y, x);
		    //print_var_list(sizex,sizey);
		    // fprintf(stdout,")\n");
		  }

		}
		for(int y = 1; y <= x; y++) {
		  if(matrix[x][y]<=1) {
		    fprintf(stdout, " trans_%d_%d ", x, y);
		    //print_var_list(sizex,sizey);
		    //fprintf(stdout,")\n");
		  }

		}		

		fprintf(stdout, ")\n");
		
		print_var_list(matrix,sizex,sizey);\
		fprintf(stdout,")");

		for(int y = 1; y <= x; y++) {
		  if(x!=y && matrix[y][x]<=1) {		    
		    matrix[y][x] = 3;
		  }
		  if(matrix[x][y]<=1) {
		    matrix[x][y] = 3;
		  }
		}		


	}
	fprintf(stdout,")\n");
	//fprintf(stdout,"'(");
	//	for(int x = 1; x <= sizex; x++){
	//  for(int y = 1; y <= sizey; y++){
	//    fprintf(stdout,"trans_%d_%d ",x,y);
	//  }
	//}
	//fprintf(stdout,")\n");
	/*
	fprintf(stdout,"'(");
	for(int y = 1; y <= sizey; y++){
	  fprintf(stdout,"p_%d ",y);
	}
	for(int x = 1; x <= sizex; x++){
	  for(int y = 1; y <= sizey; y++){
	    fprintf(stdout,"trans_%d_%d ",x,y);
	  }
	}
	fprintf(stdout,")))\n");
	*/
	//fprintf(stdout, "print_tree($1)\n");
	//fprintf(stdout, "countT($1, ");
	//for(int x = sizex; x > 0; x--)
	//	  fprintf(stdout, "p_%d ", x);
	//fprintf(stdout, ")\n");

}


void trans_new_acl2(int size, int mult,int seed) {
	struct timeval tv;
	gettimeofday(&tv, NULL);
	if(seed == 0) seed = tv.tv_usec;
	//	seed = 414115;
	//seed = 981369;
	
	fprintf(stderr, "seed = %d\n", seed);
	srand(seed);
	
	int sizex = size;
	int sizey = size;
	int **matrix = (int **)calloc(sizex+1, sizeof(int **));
	rows = (int*)calloc(sizex+1, sizeof(int*));
	columns = (int*)calloc(sizey+1, sizeof(int*));
	gen_matrix(matrix, sizex, sizey, mult);


	fprintf(stdout,"(ctv ");

	for(int x = sizex; x > 0; x--) {
	  if(x != sizex)
	    fprintf(stdout, "(q-and ");
		for(int y = 1; y <= x; y++) {
		  if(matrix[x][y]<=1) { fprintf(stdout, "(q-exists "); break;}
		  if(x!=y && matrix[y][x]<=1) { fprintf(stdout, "(q-exists "); break;}
		}

	}
	
	fprintf(stdout, "\n");
	fprintf(stdout, "(q-and (qnorm1 (to-if (multi-and `(\n");
	for(int y = 1; y <= sizey; y++) {
	  /* 	  		if(columns[y]==1) {
			//fprintf(stdout, "var(p_%d)\n", y);
		} else if(columns[y]==0) {
			//fprintf(stdout, "not(p_%d)\n", y);
		} else if(columns[y]==-1) {
			fprintf(stderr, "Column %d is empty - unsatisfiable\n", y);
			exit(0);
		} else { //columns[y] >= 2
	  */
       		for(int x = 1; x <= sizex; x++) {
				if(matrix[x][y]==1) fprintf(stdout, "(implies trans_%d_%d p_%d)\n", x, y, y);
				else if(matrix[x][y]==0) fprintf(stdout, "(implies trans_%d_%d (not p_%d))\n", x, y, y);
			}
		//					}
	}
	fprintf(stdout,")))\n");
	print_var_list(matrix,sizex,sizey);
	fprintf(stdout,")\n");
	for(int x = 1; x <= sizex; x++) {
		fprintf(stdout, "\n(qnorm1 (to-if `(and ,(minmax 1 1 '(");
		for(int y = 1; y <= sizey; y++) {
			if(matrix[x][y]<=1) fprintf(stdout, " trans_%d_%d", x, y);
		}
		fprintf(stdout, "))\n");
		
		fprintf(stdout, ",(minmax 1 1 '(");
		for(int y = 1; y <= sizey; y++) {
			if(matrix[y][x]<=1) fprintf(stdout, " trans_%d_%d", y, x);
		}
		fprintf(stdout, "))))\n");

		print_var_list(matrix,sizex,sizey);
		fprintf(stdout,")\n");
		print_var_list(matrix,sizex,sizey);
		fprintf(stdout,")\n");
		fprintf(stdout,"'(");

		for(int y = 1; y <= x; y++) {

		  if(x!=y && matrix[y][x]<=1) {
		    fprintf(stdout, " trans_%d_%d ", y, x);
		  }

		}
		for(int y = 1; y <= x; y++) {
		  if(matrix[x][y]<=1) {
		    fprintf(stdout, " trans_%d_%d ", x, y);
		  }

		}		

		fprintf(stdout, ")\n");
		
		print_var_list(matrix,sizex,sizey);\
		fprintf(stdout,")");

	}
	fprintf(stdout,")\n");

}


