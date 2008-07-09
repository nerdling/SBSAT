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

#include "sbsat.h"
#include "sbsat_preproc.h"

char *P1_repeat;
char *P2_repeat;
char *Restct_repeat;
char *ReFPS_repeat;
char *St_repeat;
char *Sa_repeat;
char *Dep_repeat;
char *Steal_repeat;
char *Ea_repeat;
char *Es_repeat;
int repeat_size = 0;

void Init_Repeats() {
	if(repeat_size >= nmbrFunctions) return;
	P1_repeat = (char *)ite_recalloc(P1_repeat, repeat_size, nmbrFunctions, sizeof(char), 9, "P1_repeat");
	P2_repeat = (char *)ite_recalloc(P2_repeat, repeat_size, nmbrFunctions, sizeof(char), 9, "P2_repeat");
	Restct_repeat = (char *)ite_recalloc(Restct_repeat, repeat_size, nmbrFunctions, sizeof(char), 9, "Restct_repeat");
	ReFPS_repeat = (char *)ite_recalloc(ReFPS_repeat, repeat_size, nmbrFunctions, sizeof(char), 9, "ReFPS_repeat");
	St_repeat = (char *)ite_recalloc(St_repeat, repeat_size, nmbrFunctions, sizeof(char), 9, "St_repeat");
	Sa_repeat = (char *)ite_recalloc(Sa_repeat, repeat_size, nmbrFunctions, sizeof(char), 9, "Sa_repeat");
	Dep_repeat = (char *)ite_recalloc(Dep_repeat, repeat_size, nmbrFunctions, sizeof(char), 9, "Dep_repeat");
	Steal_repeat = (char *)ite_recalloc(Steal_repeat, repeat_size, nmbrFunctions, sizeof(char), 9, "Steal_repeat");
	Ea_repeat = (char *)ite_recalloc(Ea_repeat, repeat_size, nmbrFunctions, sizeof(char), 9, "Ea_repeat");
	Es_repeat = (char *)ite_recalloc(Es_repeat, repeat_size, nmbrFunctions, sizeof(char), 9, "Es_repeat");
	repeat_size = nmbrFunctions;
}

void Delete_Repeats() {
	repeat_size = 0;
	ite_free((void **)&P1_repeat);
	ite_free((void **)&P2_repeat);
	ite_free((void **)&Restct_repeat);
	ite_free((void **)&ReFPS_repeat);
	ite_free((void **)&St_repeat);
	ite_free((void **)&Sa_repeat);
	ite_free((void **)&Dep_repeat);
	ite_free((void **)&Steal_repeat);
	ite_free((void **)&Ea_repeat);
	ite_free((void **)&Es_repeat);
}

void SetRepeats(int x) {
	P1_repeat[x] = 1;
	P2_repeat[x] = 1;
	Restct_repeat[x] = 1;
	ReFPS_repeat[x] = 1;
	St_repeat[x] = 1;
	Sa_repeat[x] = 1;
	Dep_repeat[x] = 1;
	Steal_repeat[x] = 1;
	Ea_repeat[x] = 1;
	Es_repeat[x] = 1;
}

void UnSetRepeats(int x) {
	P1_repeat[x] = 0;
	P2_repeat[x] = 0;
	Restct_repeat[x] = 0;
	ReFPS_repeat[x] = 0;
	St_repeat[x] = 0;
	Sa_repeat[x] = 0;
	Dep_repeat[x] = 0;
	Steal_repeat[x] = 0;
	Ea_repeat[x] = 0;
	Es_repeat[x] = 0;
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

int countBDDs() {
	int count = 0;
	for(int x = 0; x < nmbrFunctions; x++)
	  if(functions[x]!=true_ptr) count++;
	return count;
}

BDDNode *strip_x_BDD(BDDNode *bdd, int x) {
	int bdd_length = 0;
	int *bdd_vars = NULL;
	Rebuild_BDD(bdd, &bdd_length, bdd_vars);
	if(bdd_length < 3)
	  return bdd;
	if(x == bdd_vars[0])
		return collect_x(bdd, x);
	int new_x = bdd_vars[0];
	int new_v0 = bdd_vars[bdd_length-1] + 1;
	BDDNode *bdd_r = num_replace(bdd, new_x, new_v0);
	bdd_r = num_replace(bdd_r, x, new_x);
	BDDNode *bdd_x = collect_x(bdd_r, new_x);
	bdd_x = num_replace(bdd_x, new_x, x);
	bdd_x = num_replace(bdd_x, new_v0, new_x);
	
	delete [] bdd_vars;
	bdd_vars = NULL;
	
	return bdd_x;
}

BDDNode *strip_x (int bdd, int x) {
	if(length[bdd] < 3) {
		BDDNode *ret_bdd = functions[bdd];
		functions[bdd] = true_ptr;
		int OLD_DO_INFERENCES = DO_INFERENCES;
		DO_INFERENCES = 0;
		Rebuild_BDDx(bdd);
		DO_INFERENCES = OLD_DO_INFERENCES;
		return ret_bdd;
	}
	if(x == variables[bdd].num[0]) {
		BDDNode *f = functions[bdd];
		BDDNode *f_x = collect_x(f, x);
		f = pruning(f, f_x);
		functions[bdd] = f;
		int OLD_DO_INFERENCES = DO_INFERENCES;
		DO_INFERENCES = 0;
		//printBDD(functions[bdd]);
		Rebuild_BDDx(bdd);
		DO_INFERENCES = OLD_DO_INFERENCES;
		return f_x;
	}

	int new_x = variables[bdd].num[0]; //Really if variable 1 doesn't exist
	                                   //in this BDD then I don't have to do
	                                   //two replaces
	int new_v0 = variables[bdd].num[length[bdd]-1] + 1;
	BDDNode *f = num_replace(functions[bdd], variables[bdd].num[0], new_v0);
	f = num_replace(f, x, new_x);
	BDDNode *f_x = collect_x(f, new_x);
	f = pruning(f, f_x);
	f = num_replace(f, new_v0, variables[bdd].num[0]);
	f_x = num_replace(f_x, new_x, x);
	f_x = num_replace(f_x, new_v0, variables[bdd].num[0]);
	functions[bdd] = f;
	int OLD_DO_INFERENCES = DO_INFERENCES;
	DO_INFERENCES = 0;
	//printBDD(functions[bdd]);
	Rebuild_BDDx(bdd);
	DO_INFERENCES = OLD_DO_INFERENCES;
	return f_x;
}

BDDNode *collect_x (BDDNode *f, int x) {
	if(f->variable < x) return true_ptr;
	if(f->variable == x) return f;
	BDDNode *r = collect_x(f->thenCase, x);
	BDDNode *e = collect_x(f->elseCase, x);
	if(r == e) return r;
	if(f->thenCase == false_ptr) r = false_ptr;
	if(f->elseCase == false_ptr) e = false_ptr;
	return find_or_add_node (f->variable, r, e);	
}

infer *NEW_GetInfer(int *y, int *max, int **tempint, BDDNode * func);

// start
infer *GetInfer(int *y, int *max, int **tempint, BDDNode * func)
{
   *y=0;
   // assert (no flag is set );
   infer *ret = NEW_GetInfer(y, max, tempint, func);
   for (int i = 0;i<*y;i++) {
      // clear the flag
      sym_reset_flag((*tempint)[i]);
   }
   return ret;
}

infer *NEW_GetInfer(int *y, int *max, int **tempint, BDDNode * func)
{
	if ((func == true_ptr) || (func == false_ptr))
	  return NULL;
   if (sym_is_flag(func->variable) == 0)
   {
      if (*y >= *max) {
         *tempint = (int*)ite_recalloc(*(void**)tempint, *max, *max+100, sizeof(int), 9, "tempint");
         *max += 100;
      }
      (*tempint)[*y] = func->variable;
      sym_set_flag(func->variable);
      (*y)++;
   };
	infer *r = NEW_GetInfer(y, max, tempint, func->thenCase);
	infer *e = NEW_GetInfer(y, max, tempint, func->elseCase);
	
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

infer *Ex_GetInfer(BDDNode * func)
{
	if ((func == true_ptr) || (func == false_ptr))
	  return NULL;
	
	infer *r = Ex_GetInfer(func->thenCase);
	infer *e = Ex_GetInfer(func->elseCase);
	
	if (IS_TRUE_FALSE(func->thenCase) && IS_TRUE_FALSE(func->elseCase)) {
		infer *inferarray = new infer;
		inferarray->next = NULL;
		if (func->thenCase == false_ptr)
		  inferarray->nums[0] = -(func->variable);
		else
		  inferarray->nums[0] = func->variable;
		inferarray->nums[1] = 0;
		return inferarray;
	}
	
	if (func->elseCase == false_ptr) {
		infer *inferarray = new infer;
		inferarray->nums[0] = func->variable;
		inferarray->nums[1] = 0;
		inferarray->next = r;
		return inferarray;
	}
	
	if (func->thenCase == false_ptr) {
		infer *inferarray = new infer;
		inferarray->nums[0] = -(func->variable);
		inferarray->nums[1] = 0;
		inferarray->next = e;
		return inferarray;
	}

	//If either branch(thenCase or elseCase) carries true (is NULL)
	//Push up the inferences.
	//if (r == NULL) return e;
	//if (e == NULL) return r;
   if(r == NULL || e == NULL) return NULL;
	//If both are NULL, NULL is returned
	
	//If none of the above cases then we have two lists(r and e) which we
	//return all like single inferences that occur on both side of this node.

	infer *head = NULL;
	for(infer *r_iter = r; r_iter != NULL; r_iter=r_iter->next) {
		for(infer *e_iter = e; e_iter != NULL; e_iter=e_iter->next) {
			if(r_iter->nums[0] == e_iter->nums[0]) {
				infer *temp = head;
				head = new infer;
				head->nums[0] = r_iter->nums[0];
				head->nums[1] = r_iter->nums[1];
				head->next = temp;
				break;
			}
		}
	}
	infer *temp;
	while (r!=NULL) { temp = r; r = r->next; delete temp;	}
	while (e!=NULL) { temp = e; e = e->next; delete temp; }
	return head;
}

void cheat_replaceall() {
	//Get the max number of input variables
	int *tempint=NULL;
   int tempint_max = 0;
	int x, y, i;
	for (x = 0; x < numout; x++) {
		y = 0;
		unravelBDD (&y, &tempint_max, &tempint, functions[x]);
      if (y != 0) qsort (tempint, y, sizeof (int), compfunc);
		length[x] = y;
		if (variables[x].num != NULL)
		  delete variables[x].num;
      variables[x].num = new int[y + 1];	//(int *)calloc(y+1, sizeof(int));
      variables[x].num_alloc = y+1;
      for (i = 0; i < y; i++)
		  variables[x].num[i] = tempint[i];
		variables[x].min = tempint[0];
		variables[x].max = tempint[y-1];
	}
   ite_free((void**)&tempint); tempint_max = 0;
	
	numinp = 0;
	for (x = 0; x < numout; x++) {
      if (variables[x].num[length[x] - 1] > numinp)
		  numinp = variables[x].num[length[x] - 1];
	}
	
	store *amount2 = new store[numinp + 2];
	int *tempmem = new int[numinp + 2];
	
	for (i = 0; i < numinp + 1; i++) tempmem[i] = 0;
	
	for(x = 0; x < nmbrFunctions; x++) {
      for(i = 0; i < length[x]; i++) {
			tempmem[variables[x].num[i]]++;
		}
	}
	
	for (x = 1; x < numinp + 1; x++) {
		amount2[x].num = new int[tempmem[x] + 1];
      amount2[x].length = 0;
	}
	
  for (x = 0; x < nmbrFunctions; x++) {
	  for (i = 0; i < length[x]; i++) {
		  amount2[variables[x].num[i]].num[amount2[variables[x].num[i]].length] = x;
		  amount2[variables[x].num[i]].length++;
	  }
  }
	
	int replaceat = 0;
	for (x = 1; x < numinp + 1; x++) {
      if (amount2[x].length == 0)
		  continue;
      replaceat++;
      variablelist[replaceat].replace = x;
      //    fprintf(stderr, "Replacing %d with %d\n", x, replaceat);
      independantVars[replaceat] = independantVars[x];
      for (int z = 0; z < amount2[x].length; z++)
		  cheat_replace (functions[amount2[x].num[z]], x, replaceat);
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
	 for(x = 0; x < numinp+1; x++)
	 independantVars[x] = 1; 
	 for(x = 0; x < nmbrFunctions; x++) {
	 //fprintf(stdout, "here - %d %d\n", x, equalityVble[x]);
	 if(equalityVble[x]!=0) 
	 independantVars[equalityVble[x]] = 0;
	 }
	 */
	
	for (x = 1; x < numinp + 1; x++)
	  delete amount2[x].num;
	
	numinp = replaceat;
}

void findPathsToX (BDDNode *bdd, int *path_max, int **path, int pathx, intlist *list, int *listx, BDDNode *X) {
	if (bdd == X) {
		list[(*listx)].num = new int[pathx];
		for (int x = 0; x < pathx; x++) {
			list[(*listx)].num[x] = (*path)[x];
		}
		//list[(*listx)].num[pathx-1] = -list[(*listx)].num[pathx-1];
		list[(*listx)].length = pathx;
		(*listx)++;
		return;
	}
	if (IS_TRUE_FALSE(bdd))
	  return;
	
	if (pathx >= *path_max) {
		*path = (int*)ite_recalloc(*(void**)path, *path_max, *path_max+10, sizeof(int), 9, "path");
		*path_max += 10;
	}
	
	(*path)[pathx] = bdd->variable;
	findPathsToX (bdd->thenCase, path_max, path, pathx + 1, list, listx, X);
	(*path)[pathx] = -bdd->variable;
	findPathsToX (bdd->elseCase, path_max, path, pathx + 1, list, listx, X);
}

void findPathsToFalse (BDDNode *bdd, int *path_max, int **path, intlist *list, int *listx) {
	int pathx = 0;
	findPathsToX (bdd, path_max, path, pathx, list, listx, false_ptr);
}

void findPathsToTrue (BDDNode *bdd, int *path_max, int **path, intlist *list, int *listx) {
	int pathx = 0;
	findPathsToX (bdd, path_max, path, pathx, list, listx, true_ptr);
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
Sort_BDDs (int *tempint_max, int **tempint)
{
  int y = 0;
  store *sort = new store[nmbrFunctions];
  for (int x = 0; x < nmbrFunctions; x++)
    {
      unravelBDD (&y, tempint_max, tempint, functions[x]);
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

