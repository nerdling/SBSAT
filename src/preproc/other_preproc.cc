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

int *P1_repeat;
int *P2_repeat;
int *Restct_repeat;
int *ReFPS_repeat;
int *St_repeat;
int *Dep_repeat;
int repeat_size = 0;

void Init_Repeats() {
	P1_repeat = (int *)ite_recalloc(P1_repeat, repeat_size, nmbrFunctions, sizeof(int), 9, "P1_repeat");
	P2_repeat = (int *)ite_recalloc(P2_repeat, repeat_size, nmbrFunctions, sizeof(int), 9, "P2_repeat");
	Restct_repeat = (int *)ite_recalloc(Restct_repeat, repeat_size, nmbrFunctions, sizeof(int), 9, "Restct_repeat");
	ReFPS_repeat = (int *)ite_recalloc(ReFPS_repeat, repeat_size, nmbrFunctions, sizeof(int), 9, "ReFPS_repeat");
	St_repeat = (int *)ite_recalloc(St_repeat, repeat_size, nmbrFunctions, sizeof(int), 9, "St_repeat");
	Dep_repeat = (int *)ite_recalloc(Dep_repeat, repeat_size, nmbrFunctions, sizeof(int), 9, "Dep_repeat");
	repeat_size = nmbrFunctions;
}

void Delete_Repeats() {
	repeat_size = 0;
	ite_free((void **)&P1_repeat);
	ite_free((void **)&P2_repeat);
	ite_free((void **)&Restct_repeat);
	ite_free((void **)&ReFPS_repeat);
	ite_free((void **)&St_repeat);
	ite_free((void **)&Dep_repeat);
	P1_repeat = NULL;
	P2_repeat = NULL;
	Restct_repeat = NULL;
	ReFPS_repeat = NULL;
	St_repeat = NULL;
	Dep_repeat = NULL;
}

void SetRepeats(int x) {
	P1_repeat[x] = 1;
	P2_repeat[x] = 1;
	Restct_repeat[x] = 1;
	ReFPS_repeat[x] = 1;
	St_repeat[x] = 1;
	Dep_repeat[x] = 1;
}

void UnSetRepeats(int x) {
	P1_repeat[x] = 0;
	P2_repeat[x] = 0;
	Restct_repeat[x] = 0;
	ReFPS_repeat[x] = 0;
	St_repeat[x] = 0;
	Dep_repeat[x] = 0;
}

int
amount_compfunc (const void *x, const void *y)
{
  store pp, qq;

  pp = *(const store *) x;
  qq = *(const store *) y;

  if (pp.length < qq.length)
    return 1;
  if (pp.length == qq.length)
#ifndef FORCE_STABLE_QSORT
    return 0;
#else
    {
    if (x < y) return -1;
    else if (x > y) return 1;
    else return 0;
    }
#endif
  return -1;
}

/* 
 *  int
 *  compfunc (const void *x, const void *y)
 *  ... moved to utils/utils.cc
 *
 *  infer 
 *  *GetInferFoAN(BDDNode *func) 
 *  ... moved to utils/utils.cc
 */

//IMPORTANT - SEAN
//could maybe speed up this function by handing nodes from r and e to
//inferarray instead of creating a new node and copying information???
infer *GetInfer (long *y, int *tempint, BDDNode * func)
{
	if ((func == true_ptr) || (func == false_ptr))
	  return NULL;
	tempint[*y] = func->variable;
	
	if ((*y) >= 4999)
	  {
		  //Sort and remove duplicates
		  qsort (tempint, *y, sizeof (int), compfunc);
		  int v = 0;
		  for (int i = 1; i < (*y) + 1; i++)
			 {
				 v++;
				 if (tempint[i] == tempint[i - 1])
					v--;
				 tempint[v] = tempint[i];
			 }
		  (*y) = v + 1;
		  //End sorting and duplicates
	  }
	else
	  (*y)++;
	
	infer *r = GetInfer (y, tempint, func->thenCase);
	infer *e = GetInfer (y, tempint, func->elseCase);
	
	//If this node is a leaf node, put either func->variable 
	//  or -(func->variable) as the first element in inferarray
	//  and return inferarray.
	if (IS_TRUE_FALSE(func->thenCase) && IS_TRUE_FALSE(func->elseCase))
	  {
		  //fprintf(stderr, "Found a leaf, making an inference\n");
		  //printBDDerr(func);
		  //fprintf(stderr, "\n");
		  infer *inferarray = new infer;
		  inferarray->next = NULL;
		  if (func->thenCase == false_ptr)
			 inferarray->nums[0] = -(func->variable);
		  else
			 inferarray->nums[0] = func->variable;
		  inferarray->nums[1] = 0;
		  return inferarray;
	  }
	
	//If the elseCase is false then add -(func->variable) to the front
	//  of the list r and return it.
	if (func->elseCase == false_ptr)
	  {
		  //fprintf(stderr, "e is false, so we pull up inferences from r\n");
		  //printBDDerr(func);
		  //fprintf(stderr, "\n");
		  infer *inferarray = new infer;
		  inferarray->nums[0] = func->variable;
		  inferarray->nums[1] = 0;
		  inferarray->next = r;
		  return inferarray;
	  }
	
	//If the thenCase is false then add (func->variable) to the front
	//  of the list e and return it.
	if (func->thenCase == false_ptr)
	  {
		  //fprintf(stderr, "r false, so we pull up inferences from e\n");
		  //printBDDerr(func);
		  //fprintf(stderr, "\n");
		  infer *inferarray = new infer;
		  inferarray->nums[0] = -(func->variable);
		  inferarray->nums[1] = 0;
		  inferarray->next = e;
		  return inferarray;
	  }
	
	//If either branch(thenCase or elseCase) carries true (is NULL)
	//then return NULL and we lose all our nice inferences 
	if ((r == NULL) || (e == NULL))
	  {
		  //fprintf(stderr, "One branch is true ");
		  infer *temp;
		  while (r != NULL)
			 {
				 temp = r;
				 r = r->next;
				 delete temp;
			 }
		  while (e != NULL)
			 {
				 temp = e;
				 e = e->next;
				 delete temp;
			 }
		  //fprintf(stderr, "so we just lost all inferences\n");
		  //printBDDerr(func);
		  //fprintf(stderr, "\n");
		  return NULL;
	  }
	
	//If none of the above cases then we have two lists(r and e) which we
	//  combine into inferarray and return.
	int notnull = 0;
	infer *inferarray = new infer;
	inferarray->nums[0] = 0;
	inferarray->nums[1] = 0;
	inferarray->next = NULL;
	infer *head = inferarray;
	infer *rhead = r;
	infer *ehead = e;
	
	//Pass 1...Search for simple complements
	//       fprintf(stderr, "Doing simple complement search\n");
	while ((r != NULL) && (e != NULL))
	  {
		  if ((r->nums[0] == -(e->nums[0])) && (r->nums[1] == 0)
				&& (e->nums[1] == 0))
			 {
				 //fprintf(stderr, "Found a simple complement - %d = %d\n", func->variable, r->nums[0]);
				 //printBDDerr(func);
				 //fprintf(stderr, "\n");
				 notnull = 1;
				 if (inferarray->next != NULL)
					inferarray = inferarray->next;
				 inferarray->next = new infer;
				 inferarray->nums[0] = func->variable;
				 inferarray->nums[1] = r->nums[0];
				 r = r->next;
				 e = e->next;
				 continue;
			 }
		  
		  //If first nums are different, increment one of them
		  if (abs (r->nums[0]) > abs (e->nums[0]))
			 {
				 e = e->next;
				 continue;
			 }
		  if (abs (e->nums[0]) > abs (r->nums[0]))
			 {
				 r = r->next;
				 continue;
			 }
		  
		  //Else if second nums are different, increment one of them
		  if (abs (r->nums[1]) > abs (e->nums[1]))
			 {
				 e = e->next;
				 continue;
			 }
		  
		  //None of the above.
		  r = r->next;
	  }
	r = rhead;
	e = ehead;
	
	//Pass 2...Search for equals on single and double variable inferences.
	//  ex1. 3 and 3 ... ex2. 4=7 and 4=7
	//       fprintf(stderr, "doing pass 2, searching for single and double variables\n");
	while ((r != NULL) && (e != NULL))
	  {
		  //If first nums of r and e are different, increment one of them
		  if (abs (r->nums[0]) > abs (e->nums[0]))
			 {
				 e = e->next;
				 continue;
			 }
		  if (abs (e->nums[0]) > abs (r->nums[0]))
			 {
				 r = r->next;
				 continue;
			 }
		  
		  //If nums 0 of both r and e are the same. Check for single equivalence
		  if ((r->nums[0] == e->nums[0]) && (r->nums[1] == 0)
				&& (e->nums[1] == 0))
			 {
				 //fprintf(stderr, "Found a single equivalence - %d\n", r->nums[0]);
				 //printBDDerr(func);
				 //fprintf(stderr, "\n");
				 notnull = 1;
				 if (inferarray->next != NULL)
					inferarray = inferarray->next;
				 inferarray->next = new infer;
				 inferarray->nums[0] = r->nums[0];
				 inferarray->nums[1] = 0;
				 r = r->next;
				 e = e->next;
				 continue;
			 }
		  
		  //if second nums of r and e are different, increment one of them
		  if (abs (r->nums[1]) > abs (e->nums[1]))
			 {
				 e = e->next;
				 continue;
			 }
		  if (abs (e->nums[1]) > abs (r->nums[1]))
			 {
				 r = r->next;
				 continue;
			 }
		  
		  //First nums and second nums of r and e are the same. 
		  //Check for double equivalence
		  if ((r->nums[1] == e->nums[1]) && (r->nums[1] != 0)
				&& (e->nums[1] != 0))
			 {
				 //fprintf(stderr, "Found a double equivalence  %d = %d\n", r->nums[0], r->nums[1]);
				 //printBDDerr(func);
				 //fprintf(stderr, "\n");
				 notnull = 1;
				 if (inferarray->next != NULL)
					inferarray = inferarray->next;
				 inferarray->next = new infer;
				 inferarray->nums[0] = r->nums[0];
				 inferarray->nums[1] = r->nums[1];
				 r = r->next;
				 e = e->next;
				 continue;
			 }
		  r = r->next;
	  }
	if (inferarray->next != NULL)
	  delete inferarray->next;
	inferarray->next = NULL;

	r = rhead;
	e = ehead;
	
	infer *temp;
	while (r != NULL) {
		temp = r;
		r = r->next;
		delete temp;
	}
	while (e != NULL) {
		temp = e;
		e = e->next;
		delete temp;
	}
	
	if (notnull)
	  return head;
	//fprintf(stderr, "Both inference lists were different all were lost\n");
	inferarray = head;
	while (inferarray != NULL) {
		temp = inferarray;
		inferarray = inferarray->next;
		delete temp;
	}
	rhead = NULL;
	ehead = NULL;
	inferarray = NULL;
	head = NULL;
	//No need to delete head because it pointed to inferarray
	return NULL;
}

void cheat_replaceall (int *&length, store * &variables, varinfo * &variablelist) {
	//Get the max number of input variables
	int tempint[5000];
	long y, v, i;
	for (int x = 0; x < numout; x++) {
		y = 0;
		unravelBDD (&y, tempint, functions[x]);
      qsort (tempint, y, sizeof (int), compfunc);
      if (y != 0) {
			v = 0;
			for (i = 1; i < y; i++) {
				v++;
				if (tempint[i] == tempint[i - 1])
				  v--;
				tempint[v] = tempint[i];
			}
			y = v + 1;
		}
		length[x] = y;
		if (variables[x].num != NULL)
		  delete variables[x].num;
      variables[x].num = new int[y + 1];	//(int *)calloc(y+1, sizeof(int));
      for (i = 0; i < y; i++)
		  variables[x].num[i] = tempint[i];
		variables[x].min = tempint[0];
		variables[x].max = tempint[y-1];
	}
	
	numinp = 0;
	for (int x = 0; x < numout; x++) {
      if (variables[x].num[length[x] - 1] > numinp)
		  numinp = variables[x].num[length[x] - 1];
	}
	
	store *amount = new store[numinp + 2];
	int *tempmem = new int[numinp + 2];
	
	for (int i = 0; i < numinp + 1; i++)
	  tempmem[i] = 0;
	
	for (int x = 0; x < nmbrFunctions; x++) {
      for (int i = 0; i < length[x]; i++) {
			tempmem[variables[x].num[i]]++;
		}
	}
	
	for (int x = 1; x < numinp + 1; x++) {
		amount[x].num = new int[tempmem[x] + 1];
      amount[x].length = 0;
	}
	
  for (int x = 0; x < nmbrFunctions; x++) {
	  for (int i = 0; i < length[x]; i++) {
		  amount[variables[x].num[i]].num[amount[variables[x].num[i]].length] = x;
		  amount[variables[x].num[i]].length++;
	  }
  }
	
	int replaceat = 0;
	for (int x = 1; x < numinp + 1; x++) {
      if (amount[x].length == 0)
		  continue;
      replaceat++;
      variablelist[replaceat].replace = x;
      //    fprintf(stderr, "Replacing %d with %d\n", x, replaceat);
      independantVars[replaceat] = independantVars[x];
      for (int z = 0; z < amount[x].length; z++)
		  cheat_replace (functions[amount[x].num[z]], x, replaceat);
      for (int z = 0; z < nmbrFunctions; z++) {
			if (equalityVble[z] == x)
			  equalityVble[z] = replaceat;
			else if (equalityVble[z] == -x)
			  equalityVble[z] = -replaceat;
			//if (parameterizedVars[z] != NULL)
			//{
			//   for (int i = 1; i <= parameterizedVars[z][0];
			//         i++)
			//   {
			//      if (parameterizedVars[z][i] == x)
			//         parameterizedVars[z][i] = replaceat;
			//   }
			//}
		}
	}
	
	/*      
	 for(int x = 0; x < numinp+1; x++)
	 independantVars[x] = 1; 
	 for(int x = 0; x < nmbrFunctions; x++) {
	 //fprintf(stdout, "here - %d %d\n", x, equalityVble[x]);
	 if(equalityVble[x]!=0) 
	 independantVars[equalityVble[x]] = 0;
	 }
	 */
	
	for (int x = 1; x < numinp + 1; x++)
	  delete amount[x].num;
	
	numinp = replaceat;
}

void findPathsToFalse (BDDNode *bdd, int *path, int pathx, intlist *list, int *listx) {
	if (bdd == false_ptr) {
		list[(*listx)].num = new int[pathx];
		for (int x = 0; x < pathx; x++) {
			list[(*listx)].num[x] = -path[x];
		}
		//list[(*listx)].num[pathx-1] = -list[(*listx)].num[pathx-1];
		list[(*listx)].length = pathx;
		(*listx)++;
		return;
	}
	if (bdd == true_ptr)
	  return;
	
	path[pathx] = bdd->variable;
	findPathsToFalse (bdd->thenCase, path, pathx + 1, list, listx);
	
	path[pathx] = -bdd->variable;
	findPathsToFalse (bdd->elseCase, path, pathx + 1, list, listx);
}

void
Stats (int length[], store variables[])
{
  int tempint[5000];
  long y, v, i;
  int z;
  intlist *clauses = new intlist[10000];

  z = 0;
  for (int x = 0; x < numout; x++)
    {
      int numx = countFalses (functions[x]);
      intlist *list = new intlist[numx];
      int pathx = 0, listx = 0;
      findPathsToFalse (functions[x], tempint, pathx, list,
			&listx);
      //      if (functionType[x] != AND
      //	  && functionType[x] != OR
      //	  && functionType[x] != ITE)
      //	{
		 for (int y = 0; y < listx; y++)
			{
				clauses[z].num = list[y].num;
				clauses[z].length = list[y].length;
				for (int a = 0; a < clauses[z].length; a++)
				  {
					          printf("%d ", clauses[z].num[a]);
				  }
				      printf("0\n");    

				z++;
			}
		//       fprintf(stdout, "\nend %d\n", x);
	 }/*
	else if (functionType[x] == ITE)
	  {
	  int znow = z;
	  printBDD (functions[x]);
	  printf ("\n%d BDD\n", x);
	  for (int y = 0; y < listx; y++)
	    {
	      clauses[z].num = list[y].num;
	      clauses[z].length = list[y].length;
	      for (int a = 0; a < clauses[z].length; a++)
		{
		  printf ("%d ", clauses[z].num[a]);
		}
	      printf ("0\n");
	      z++;
	    }
	  printf ("\n");
	  if (z - znow == 6)
	    z = makeAllResolutions (clauses, znow, z);
	  for (int y = znow; y < z; y++)
	    {
	      for (int a = 0; a < clauses[y].length; a++)
		{
		  printf ("%d ", clauses[y].num[a]);
		}
	      printf ("0\n");
	    }

	}
      else
	{
	  int var = equalityVble[x];	//EqualityVble can be + or -
	  int posneg = -1;	//So this is probably all 
	  int gotit = -1;	//messed up...
	  for (int y = 0; y < listx; y++)
	    {
	      for (int a = 0; a < list[y].length; a++)
		{
		  if (abs (list[y].num[a]) == var)
		    {
		      if (list[y].num[a] > 0)
			{
			  if (posneg == 1)
			    gotit = 1;
			  else
			    posneg = 1;
			}
		      else
			{
			  if (posneg == 0)
			    gotit = 0;
			  else
			    posneg = 0;
			}
		    }
		}
	      if (gotit != -1)
		break;
	    }
	  if (gotit == 0)
	    var = -var;
	  int cla1 = -1;
	  for (int y = 0; y < listx; y++)
	    {
	      if (list[y].length == length[x])
		{
		  for (int a = 0; a < list[y].length; a++)
		    {
		      if (list[y].num[a] == -var)
			cla1 = y;
		    }
		}
	    }
	  clauses[z].num = list[cla1].num;
	  clauses[z].length = list[cla1].length;
	  z++;

	  for (int y = 0; y < list[cla1].length; y++)
	    {
	      if (list[cla1].num[y] != -var)
	        {
	          clauses[z].num = new int[2];
	          clauses[z].num[0] = -list[cla1].num[y];
	          clauses[z].num[1] = var;
	          clauses[z].length = 2;
		  z++;
		}
	    }
	}
    }

  printf ("\n");
  for (int x = 0; x < z; x++)
    {
      for (int a = 0; a < clauses[x].length; a++)
	printf2 ("%d ", clauses[x].num[a]);
      printf1 ("0\n");
    }*/


	for (int x = 0; x < numout; x++)
	  {
		  y = 0;
		  unravelBDD (&y, tempint, functions[x]);
		  qsort (tempint, y, sizeof (int), compfunc);
		  if (y != 0)
			 {
				 v = 0;
				 for (i = 1; i < y; i++)
					{
						v++;
						if (tempint[i] == tempint[i - 1])
						  v--;
						tempint[v] = tempint[i];
					}
				 y = v + 1;
			 }
		  length[x] = y;
		  if (variables[x].num != NULL)
			 delete variables[x].num;
		  variables[x].num = new int[y + 1];	//(int *)calloc(y+1, sizeof(int));
		  for (i = 0; i < y; i++)
			 variables[x].num[i] = tempint[i];
	  }
	
	store *amount = new store[numinp + 1];
	int *tempmem = new int[numinp + 2];
	
	for (int i = 0; i < numinp + 2; i++)
	  tempmem[i] = 0;
	
	for (int x = 0; x < nmbrFunctions; x++)
	  {
		  for (int i = 0; i < length[x]; i++)
			 tempmem[variables[x].num[i]]++;
	  }
	
	for (int x = 0; x < numinp + 1; x++)
	  {
		  amount[x].num = new int[tempmem[x]];
		  amount[x].length = 0;
	  }
	
	for (int x = 0; x < nmbrFunctions; x++)
	  {
		  for (int i = 0; i < length[x]; i++)
			 {
				 amount[variables[x].num[i]].num[amount[variables[x].num[i]].
															length] = x;
				 amount[variables[x].num[i]].length++;
			 }
	  }
	
	for (int x = 1; x < numinp + 1; x++)
	  {
		  amount[x].num[0] = x;
	  }
	
	y = numinp + 1;
	qsort (amount, y, sizeof (store), amount_compfunc);
	
	for (int x = 0; x < numinp; x++)
	  {
		  fprintf (stdout, "Variable %d occurs %d times.\n", amount[x].num[0],
					  amount[x].length);
	  }
	
	for (int x = 0; x < numinp + 1; x++)
	  {
		  delete amount[x].num;
		  amount[x].num = new int[tempmem[x]];
		  amount[x].length = 0;
	  }
	
	for (int x = 0; x < nmbrFunctions; x++)
	  {
		  for (int i = 0; i < length[x]; i++)
			 {
				 amount[variables[x].num[i]].num[amount[variables[x].num[i]].
															length] = x;
				 amount[variables[x].num[i]].length++;
			 }
	  }
	
	fprintf (stdout, "\n");
	
	for (int x = 0; x < numout; x++)
	  {
		  y = 0;
		  unravelBDD (&y, tempint, functions[x]);
		  printBDD (functions[x]);
		  fprintf (stdout, "\nConstraint %d ", x);
		  if (functionType[x] == AND)
			 fprintf (stdout, "is an AND= function and ");
		  else if (functionType[x] == OR)
			 fprintf (stdout, "is an OR= function and ");
		  else if (functionType[x] == PLAINOR)
			 fprintf (stdout, "is a PLAINOR function and ");
		  else if (functionType[x] == ITE)
			 fprintf (stdout, "is an ITE= function and ");
		  else if (length[x] > PLAINOR_LIMIT) fprintf(stdout, "has more than PLAINOR_LIMIT (error) variables and ");
		  fprintf (stdout, "has %ld nodes and %d variables.\n", y, length[x]);
		  //if (parameterizedVars[x] != NULL)
		//	 {
		//		 printBDD (functions[x]);
		//		 fprintf (stdout, "\n");
		//		 for (int z = 0; z <= parameterizedVars[x][0]; z++)
		//			{
		//				fprintf (stdout, "%d|", parameterizedVars[x][z]);
		//			}
		//	 }
		  fprintf (stdout, "\n\n");
	  }
	
	fprintf (stdout, "\n");
	
	int ind = 0, dep = 0;
	for (int x = 1; x < numinp + 1; x++)
	  {
		  if (independantVars[x] == 1)
			 {
				 fprintf (stdout, "Variable %d is Independant\n", x);
				 ind++;
			 }
		  else
			 {
				 fprintf (stdout, "Variable %d is Dependant\n", x);
				 dep++;
			 }
	  }
	
	fprintf (stdout,
				"There are %d Independant Variables and %d Dependant Variables\n",
				ind, dep);
	
	for (int x = 1; x < numinp + 1; x++)
	  delete amount[x].num;
	delete amount;
	delete tempmem;
	fprintf (stderr, "\n");
	exit (1);
}

int
node_compfunc (const void *x, const void *y)
{
  store pp, qq;

  pp = *(const store *) x;
  qq = *(const store *) y;
  if (pp.dag < qq.dag)
    return -1;
  if (pp.dag == qq.dag)
#ifndef FORCE_STABLE_QSORT
    return 0;
#else
    {
    if (x < y) return 1;
    else if (x > y) return -1;
    else return 0;
    }
#endif
  return 1;
}

void
Sort_BDDs (int *tempint)
{
  long y = 0;
  store *sort = new store[nmbrFunctions];
  for (int x = 0; x < nmbrFunctions; x++)
    {
      unravelBDD (&y, tempint, functions[x]);
      sort[x].dag = y;
      sort[x].length = x;
    }
  qsort (sort, nmbrFunctions, sizeof (store), node_compfunc);

  BDDNode **funcs = new BDDNode *[nmbrFunctions + 1];
  for (int x = 0; x < nmbrFunctions; x++)
    funcs[x] = functions[x];
  for (int x = 0; x < nmbrFunctions; x++)
    functions[x] = funcs[sort[x].length];
  delete funcs;
}

