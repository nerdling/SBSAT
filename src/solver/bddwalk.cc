/* =========FOR INTERNAL USE ONLY. NO DISTRIBUTION PLEASE ========== */

/*********************************************************************
 Copyright 1999-2004, University of Cincinnati.  All rights reserved.
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

/* walksat version 35hw4 */
/* version 1 by Bram Cohen 7/93 */
/* versions 2 - 35  by Henry Kautz */
/* versions 35h by Holger H Hoos -- all modifications marked "hh" */
/* version 35hw* by Sean Weaver -- modified to solve on BDDs instead of CNF */

#include "ite.h"
#include "solver.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <sys/types.h>
#include <limits.h>
#include <signal.h>
#include <sys/times.h>
#include <sys/time.h>

#define BIG 100000000

#define NOVALUE -1
#define INIT_PARTIAL 1
#define HISTMAX 101		/* length of histogram of tail */

#define TRUE 1
#define FALSE 0

#define MAX_NODES_PER_BDD 5000

int numvars; //number of variables
int numBDDs; //number of BDDs (numout | nmbrFunctions)

int numliterals;

BDDState *previousState;    /* records which BDDs are true, and which are false */

int pathmake, pathbreak;

int *falseBDDs; 		/* clauses which are false */
int *wherefalse;  	/* where each clause is listed in false */
int *wlength;			/* number of variables in each BDD */
int *lowfalse;
int *numtruelit;		/* number of true literals in each clause */

intlist *wvariables;  /* a list of variables in each BDD, could be used to */
                      /* makes 'wlength' an unnecessary variable */

intlist *occurance;	/* where each variable occurs */
                     /* indexed as occurance[variable].num[occurance_num] */
                     /* numoccurance is now occurance[variable].length */
							/* which is the number of times each variable occurs */

int *atom;		      /* value of each atom */ 
int *lowatom;
int *solution;

int *changed;		/* step at which atom was last flipped */

int *breakcount;	/* number of clauses that become unsat if var if flipped */
int *makecount;	/* number of clauses that become sat if var if flipped */

int numfalse;		/* number of false clauses */

int *varstoflip;
int *best_to_flip;
int best_to_flip_length;
int *second_to_flip;
int second_to_flip_length;

double *true_var_weights;

float true_weight_mult = 1.5;
float taboo_max = 6;
float taboo_length = taboo_max-1;//2;
float true_weight_taboo = 0.5;
float true_weight_max = 0.5;

float path_factor = 1; //Number of random paths choosen = total number of variables * path_factor;
//Maybe a special case is needed for clauses?

/************************************/
/* Global flags and parameters      */
/************************************/

int numerator = 10;//NOVALUE;	/* make random flip with numerator/denominator frequency */

int denominator = 100;

int wp_numerator = 10;//NOVALUE;	/* walk probability numerator/denominator */
int wp_denominator = 1000;

long int numflip;		/* number of changes so far */

long int numlook;
int numrun = BIG;
int cutoff = 100000; //BIG
int base_cutoff = 1000000;
int target = 0;
int numtry = 0;			/* total attempts at solutions */
int numsol = NOVALUE;	        /* stop after this many tries succeeds */

/* Histogram of tail */

long int tailhist[HISTMAX];	/* histogram of num unsat in tail of run */
int tail = 3;
int tail_start_flip;

/* Randomization */

int seed;			/* seed for random */
struct timeval tv;
struct timezone tzp;

double expertime;
long flips_this_solution;
long int lowbad;		/* lowest number of bad clauses during try */
long int totalflip = 0;		/* total number of flips in all tries so far */
long int totalsuccessflip = 0;	/* total number of flips in all tries which succeeded so far */
int numsuccesstry = 0;		/* total found solutions */

long x;
long integer_sum_x = 0;
double sum_x = 0.0;
double sum_x_squared = 0.0;
double mean_x;
double second_moment_x;
double variance_x;
double std_dev_x;
double std_error_mean_x;
double seconds_per_flip;
int r;
int sum_r = 0;
double sum_r_squared = 0.0;
double mean_r;
double variance_r;
double std_dev_r;
double std_error_mean_r;

double avgfalse;
double sumfalse;
double sumfalse_squared;
double second_moment_avgfalse, variance_avgfalse, std_dev_avgfalse, ratio_avgfalse;
double f;
double sample_size;

double sum_avgfalse = 0.0;
double sum_std_dev_avgfalse = 0.0;
double mean_avgfalse;
double mean_std_dev_avgfalse;
int number_sampled_runs = 0;
double ratio_mean_avgfalse;

double suc_sum_avgfalse = 0.0;
double suc_sum_std_dev_avgfalse = 0.0;
double suc_mean_avgfalse;
double suc_mean_std_dev_avgfalse;
int suc_number_sampled_runs = 0;
double suc_ratio_mean_avgfalse;

double nonsuc_sum_avgfalse = 0.0;
double nonsuc_sum_std_dev_avgfalse = 0.0;
double nonsuc_mean_avgfalse;
double nonsuc_mean_std_dev_avgfalse;
int nonsuc_number_sampled_runs = 0;
double nonsuc_ratio_mean_avgfalse;

/* Noise level */
int samplefreq = 1;

int picknoveltyplus(void);

int countunsat(void);
void verifymbcount();
void init_CountFalses();
void initprob(void);                 /* create a new problem */
void freemem(void);
void flipatoms(void);

void Fill_Weights(void);
void save_low_assign(void);
void save_solution(void);

void print_current_assign(void);

void print_statistics_header(void);
void update_statistics_start_try(void);
void update_and_print_statistics_end_try(void);
void update_statistics_end_flip(void);
void print_statistics_final(void);
void print_sol_cnf(void);

/************************************/
/* Main                             */
/************************************/

int walkSolve()
{
	numinp = getNuminp ();
	if(numsol == NOVALUE) numsol = max_solutions; //get max_solutions from the command line
	//int oldnuminp = numinp;
	gettimeofday(&tv,&tzp);
	seed = (( tv.tv_sec & 0177 ) * 1000000) + tv.tv_usec;
	if (numsol==NOVALUE || numsol>numrun) numsol = numrun;
	srandom(seed);
	initprob(); //must create array that tells in what BDDs a variable occurs...
	print_statistics_header();
	expertime = get_runtime();
	while ((numsuccesstry < numsol) && (numtry < numrun)) {
		numtry++;
		init_CountFalses();
		update_statistics_start_try();
		numflip = 0;
		numlook = 0;

		for(int i = 1; i < numinp; i++) {
			//fprintf(stderr, "%d ", atom[i]==1?atoi(s_name(i)):-atoi(s_name(i)));
		}
		//fprintf(stderr, "\n");

		while((numfalse > target) && (numflip < cutoff)) {
			numflip+=picknoveltyplus();
			flipatoms();
			if (nCtrlC) {
				d3_printf1("Breaking out of BDD WalkSAT\n");
				break;
			}
			update_statistics_end_flip();
		}
		update_and_print_statistics_end_try();
		if (nCtrlC) {
			nCtrlC = 0;
			break;
		}
	}
	expertime = get_runtime() - expertime;
	print_statistics_final();
	freemem();
	if (numsuccesstry!=0) return SOLV_SAT;
	return SOLV_UNKNOWN;
}

void initprob(void)
{
	//Number of variables in each BDD.
	//Number of times each variable occurs, (per node or per BDD or per 2^level????)
	//Set up an array of pointers where index 1 is variable, index 2 is the BDD it occurs in.
	
	numvars = numinp = getNuminp();
	numBDDs = nmbrFunctions;
	
	atom = new int[numvars+1];
	lowatom = new int[numvars+1];
	solution = new int[numvars+1];
	
	occurance = new intlist[numvars+1];
	wlength = new int[numBDDs+1];
	wvariables = new intlist[numBDDs+1];
	int *tempmem = new int[numvars+1];
	int *tempint = NULL; //new int[MAX_NODES_PER_BDD];
	long tempint_max = 0;
	
	changed = new int[numvars+1];
	breakcount = new int[numvars+1];
	makecount = new int[numvars+1];	
	true_var_weights = new double[numvars+1];
	previousState = new BDDState[numBDDs+1]; //intlist[numBDDs+1];
	wherefalse = new int[numBDDs+1];
	falseBDDs = new int[numBDDs+1];
	varstoflip = new int[numvars+1];
	best_to_flip = new int[numvars+1];
	second_to_flip = new int[numvars+1];
	
	for(int x = 0; x < taboo_length; x++)
	  true_weight_taboo = true_weight_taboo / true_weight_mult;
	for(int x = 0; x < taboo_max; x++)
	  true_weight_max = true_weight_max / true_weight_mult;
	
	for(int x = 0; x < numvars+1; x++) {
		tempmem[x] = 0;
		true_var_weights[x] = 0.5;
	}
	
	for(int x = 0; x < numBDDs; x++) {
		long y = 0;
		unravelBDD (&y, &tempint_max, &tempint, functions[x]);
		if (y != 0) qsort (tempint, y, sizeof (int), compfunc);
		
		wlength[x] = y;
		previousState[x].visited = 0;
		previousState[x].IsSAT = 0;
		wvariables[x].num = new int[y+1];	//(int *)calloc(y+1, sizeof(int));
		wvariables[x].length = y;
		for (int i = 0; i < y; i++) {
			wvariables[x].num[i] = tempint[i];
			tempmem[tempint[i]]++;
		}
	}	
	
	for (int x = 0; x < numvars+1; x++)
	  {
		  occurance[x].num = new int[tempmem[x]+1];
		  occurance[x].length = 0;
	  }
	delete [] tempmem;
	
	for (int x = 0; x < numBDDs; x++)
	  {
		  for (int i = 0; i < wlength[x]; i++)
			 {
				 occurance[wvariables[x].num[i]].num[occurance[wvariables[x].num[i]].length] = x;
				 occurance[wvariables[x].num[i]].length++;
			 }
	  }
	
	//Need "variables" for later! Don't delete!
	
	//occurance is done.
	//length is done.
	
	Fill_Weights(); //Each BDDNode now has the density of True's below it stored.

	ite_free((void**)&tempint); tempint_max = 0;
}

void freemem(void)
{
	delete [] atom;
	delete [] lowatom;
	delete [] solution;
	
	for (int x = 0; x < numvars+1; x++)
	  delete [] occurance[x].num;
	delete [] occurance;
	
	for (int x = 0; x < numBDDs; x++)
	  delete [] wvariables[x].num;
	delete [] wvariables;
	
	delete [] wlength;
	
	delete [] changed;
	delete [] breakcount;
	delete [] makecount;
	delete [] true_var_weights;
	
	delete [] previousState;
		
	delete [] wherefalse;
	delete [] falseBDDs;
	delete [] varstoflip;
	delete [] best_to_flip;
	delete [] second_to_flip;

}

void print_statistics_header(void)
{
    printf("numvars = %i, numBDDs = %i\n",numvars,numBDDs);
    printf("wff read in\n\n");
    printf("    lowest     final       avg     noise     noise     total                 avg      mean      mean\n");
    printf("    #unsat    #unsat     noise   std dev     ratio     flips              length     flips     flips\n");
    printf("      this      this      this      this      this      this   success   success     until       std\n");
    printf("       try       try       try       try       try       try      rate     tries    assign       dev\n\n");

    fflush(stdout);

}

float weight_bdd(BDDNode *f) {
   if (f->density > -1)
	  return f->density;
   float t = weight_bdd(f->thenCase);
   float e = weight_bdd(f->elseCase);
   f->density = t*true_var_weights[f->variable] + e*(1-true_var_weights[f->variable]);
   f->tbr_weight = t*true_var_weights[f->variable] / f->density;
   //f->fbr_weight = 1-tbr_weight; Unnecessary for our purposes
   return f->density;
}

void Fill_Weights() {
   for(int x = 0; x < numBDDs; x++)
     unmark(functions[x]);
   true_ptr->density = 1;
   false_ptr->density = 0;
   for(int x = 0; x < nmbrFunctions; x++) {
      weight_bdd(functions[x]);
   }
}

bool traverseBDD(BDDNode *f) {
	while(!IS_TRUE_FALSE(f)) {
		if(atom[f->variable] == TRUE) {
			f = f->thenCase;
		} else {
			f = f->elseCase;			  
		}
	}
	if(f == false_ptr) return false;
	assert(f == true_ptr);
	return true;
}

//Find a path to true by following the current assignment and backtracking when a false is hit.
int findTrue(BDDNode *f) {
	int y = 1;
	while(!IS_TRUE_FALSE(f)) {
		if((atom[f->variable] == TRUE) && (f->thenCase != false_ptr)) {
			f = f->thenCase;
		} else if((atom[f->variable] == TRUE) && (f->thenCase == false_ptr)) {
			varstoflip[y++] = f->variable;
			f = f->elseCase;
		} else if((atom[f->variable] == FALSE) && (f->elseCase != false_ptr)) {
			f = f->elseCase;
		} else {
			varstoflip[y++] = f->variable;
			f = f->elseCase;  
		}
	}
	return y;
}

int verifyunsat(void) {
	for(int i = 0; i < numBDDs; i++) {
		if(!traverseBDD(functions[i])) {
			if(falseBDDs[wherefalse[i]] != i) {
				fprintf(stderr, "\ni=%d:", i);
				printBDDerr(functions[i]);
				fprintf(stderr, "\n");
				return 0;
			}
		}
	}
	return 1;
}

int verifyunsat(int i) {
	if(!traverseBDD(functions[i])) {
		if(falseBDDs[wherefalse[i]] != i) {
			fprintf(stderr, "\ni=%d:", i);
			printBDDerr(functions[i]);
			fprintf(stderr, "\n");
			return 0;
		}
	}
	return 1;
}

void init_CountFalses() {
	int i;
	numfalse = 0;
	best_to_flip_length = 0;
	second_to_flip_length = 0;
	
	for(i = 0;i < numvars+1;i++) {
		changed[i] = -BIG;
		breakcount[i] = 0;
		makecount[i] = 0;
	}
	
	for(i = 1;i < numvars+1;i++)
	  atom[i] = random()%2;  //This makes a random truth assignment
	
	/* Initialize breakcount, makecount, and previousState in the following: */
	
	for(i = 0;i < numBDDs;i++) {
		previousState[i].visited = -1;
		if(!traverseBDD(functions[i])) {
			previousState[i].IsSAT = 0;
			wherefalse[i] = numfalse;
			falseBDDs[numfalse] = i;
			numfalse++;
		} else {
			previousState[i].IsSAT = 1;
		}
	}
	
	//	for(int x = 1; x < numvars+1; x++) {
	//   fprintf(stderr, "\n%d: bc=%d, mc=%d, diff=%d", x, breakcount[x], makecount[x], (makecount[x] - breakcount[x]));
	// }
	//	fprintf(stderr, "\n");
}

int getRandomTruePath(int x) {
	//This function finds a random path to True in BDD x and stores it in varstoflip[]
	//varstoflip[] consists of only variables in the path
	//  with opposite value of the current assignment
	BDDNode *bdd = functions[x];
	int len = wlength[x]; //Number of variables in this BDD
	int path_length = 0;
	for(int i = 0; i < len; i++) {
		int wvar = wvariables[x].num[len-i-1];
		//If the BDDNode we are looking at has the same variable as the next variable
		//to pick a value for, then:
		if(bdd->variable == wvar) {
			if(bdd->elseCase == false_ptr) {
				//variable wvar is forced to True
				if(atom[wvar] == 0)
				  varstoflip[path_length++] = wvar;
				bdd = bdd->thenCase;
			} else if(bdd->thenCase == false_ptr) {
				//variable wvar is forced to False
				if(atom[wvar] == 1)
				  varstoflip[path_length++] = wvar;
				bdd = bdd->elseCase;
			} else if (true_var_weights[wvar] > 1-true_weight_taboo) {
				//Taboo forces variable wvar to True
				if(atom[wvar] == 0)
				  varstoflip[path_length++] = wvar;
				bdd = bdd->thenCase;
			} else if(true_var_weights[wvar] < true_weight_taboo) {
				//Taboo forces variable wvar to False
				if(atom[wvar] == 1)
				  varstoflip[path_length++] = wvar;
				bdd = bdd->elseCase;
			} else {
				//Choose this variable's direction based on it's weight
				if((random()%100) < (bdd->tbr_weight * 100)) {
					//Variable wvar is chosen True
					if(atom[wvar] == 0)
					  varstoflip[path_length++] = wvar;
					bdd = bdd->thenCase;
				} else {
					//Variable wvar is chosen False
					if(atom[wvar] == 1)
					  varstoflip[path_length++] = wvar;
					bdd = bdd->elseCase;
				}
			}
		} else { //if(wvar != bdd->variable)
			//This variable is not along the path that has been chosen
			//We have a few options:
			//  (1) We can leave the variable alone
			//  (2) We can set the variable randomly
			//  (3) We can set the variable based on it's taboo weight
			
			//Choose this variable's direction based on it's weight
			//(1)
			//if((random()%2) > 0) {

			//(2)
			//if(atom[wvar] == 1) {

			//(3)
			if((random()%100) < (true_var_weights[wvar]*100)) {
			  if(atom[wvar] == 0)
				 varstoflip[path_length++] = wvar;
			} else {
				if(atom[wvar] == 1)
				 varstoflip[path_length++] = wvar;
			}
		}
	}
	varstoflip[path_length] = 0;
	
	//Verify the path:
	//if(bdd!=true_ptr) {
	//	fprintf(stderr, "ERROR! PATH DOES NOT SATISFY BDD\n");
	//	printBDDerr(bdd);
	//	exit(0);		  
	//}
	
	//Print the path:
	//printBDDerr(functions[x]);
	//for(int i = 0; i < path_length; i++) {
	//	fprintf(stderr, "%d ", varstoflip[i]);
	//}
	//fprintf(stderr, "\n");

	return path_length;
}

void getmbcountTruePath() {
	numlook++;
	pathmake = 0; pathbreak = 0;
	
	//Temporarily set all variables in the global solution
	for(int i = 0; varstoflip[i]!=0; i++) {
		//Flipping all variables in varstoflip[]
		atom[varstoflip[i]] = 1-atom[varstoflip[i]];
	}
	
	for(int x = 0; varstoflip[x]!=0; x++) {		
		int numocc = occurance[varstoflip[x]].length;
		int *occptr = occurance[varstoflip[x]].num;
		
		for(int i = 0; i < numocc ;i++) {
			int cli = *(occptr++);
			//cli = occurance[toflip].num[i];
			if(previousState[cli].visited == numlook) continue;
			previousState[cli].visited = numlook;
			//Removing the influence every variable in this BDD has on this BDD.
			if(traverseBDD(functions[cli])) {
				//BDD is SAT
				if(previousState[cli].IsSAT == 0) {
					pathmake++;
				}
			} else {
				if(previousState[cli].IsSAT == 1) {
					pathbreak++;
				}
			}
		}
	}
	
	//Reset all variables in the global solution
	for(int i = 0; varstoflip[i]!=0; i++) {
		//Unflipping all variables in varstoflip[]
		atom[varstoflip[i]] = 1-atom[varstoflip[i]];
	}
}

int picknoveltyplus(void)
{
	int try_agains = 0;
	again:;
	double diff = 0.0;
	int youngest_birthdate, best=-1, second_best=-1;
	double best_diff, second_best_diff;
	int best_make = 0, second_make = 0;
	int tofix, BDDsize;
	int flippath = 0;
	
	tofix = falseBDDs[random()%numfalse]; //Maybe in the future not so random...
	                                      //Could be based on size of BDD (how many paths to true?)
	BDDsize = wlength[tofix];  

//	fprintf(stderr, "\ntofix=%d\n", tofix);
	
	int path_length = 0;

	/* hh: inserted modified loop breaker: */
	//fprintf(stderr, "\ntofix: %d\n", tofix);
	if ((random()%wp_denominator < wp_numerator)) {
		flippath = 0;

		//#1
		path_length = getRandomTruePath(tofix);
		
		//#2
		//for(path_length = 0; x < wlength[tofix]; path_length++) {
		//	if((random()%2) > 0)
		//	  varstoflip[path_length++] = wvariables[tofix].num[path_length];
		//}
		//varstoflip[path_length] = 0;
		
		//#3
		//varstoflip[0] = random()%numvars+1;
		//varstoflip[1] = 0;
		//path_length = 1;

		//#4
		//varstoflip[0] = wvariables[tofix].num[random()%BDDsize];
		//varstoflip[1] = 0;
		//path_length = 1;
	} else {
		youngest_birthdate = -1;
		best_diff = -1000000.0;
		second_best_diff = -1000000.0;

		unmark(functions[tofix]);
		weight_bdd(functions[tofix]);
		for(int i = 0; i < wlength[tofix]*path_factor; i++) {
			path_length = getRandomTruePath(tofix);
			
			getmbcountTruePath();
			//diff = pathmake - pathbreak;
			diff = -pathbreak;
			//fprintf(stderr, "{mp=%d, bp=%d, diff=%f}", pathmake, pathbreak, diff);
			
			if (diff > best_diff || (diff == best_diff && pathmake > best_make)) {// && changed[path] < changed[best])) {
				/* found new best, demote best to 2nd best */
				for(int i = 0; i < best_to_flip_length+1; i++)
				  second_to_flip[i] = best_to_flip[i];
				second_to_flip_length = best_to_flip_length;
				
				for(int i = 0; i<path_length+1; i++)
				  best_to_flip[i] = varstoflip[i];
				best_to_flip_length = path_length;
				
				second_best_diff = best_diff;
				best_diff = diff;
				best_make = pathmake;
			}
			else if (diff > second_best_diff || (diff == second_best_diff && pathmake > second_make)) {// && changed[var] < changed[second_best])){
				/* found new second best */
				for(int i = 0; i < path_length+1; i++)
				  second_to_flip[i] = varstoflip[i];
				second_to_flip_length = path_length;
				
				second_best_diff = diff;
				second_make = pathmake;
			}
		}

		try_agains++;

		if(second_best == -1) flippath = 1;
		else if(best_diff == second_best_diff) { second_best = best; best = -1; flippath = 2; }
		else if ((random()%denominator < numerator)) flippath = 2;
		else flippath = 1;
		if(try_agains < 20) {
			if(best_diff < -(numfalse/3) && flippath == 1) { 
				path_factor++;
				goto again;
			}
			if(second_best_diff < -(numfalse/2) && flippath == 2) {
				path_factor++;
				goto again;
			}
		}
	}

	//path_factor = 1;
	path_factor = 10-(((numfalse)/numBDDs)*10)+1;
	
	if(flippath == 1) { //Choose best path
		for(int i = 0; i<best_to_flip_length+1; i++)
		  varstoflip[i] = best_to_flip[i];
		path_length = best_to_flip_length;
	} else if(flippath == 2) { //Choose second_best path
		for(int i = 0; i<second_to_flip_length+1; i++)
		  varstoflip[i] = second_to_flip[i];
		path_length = second_to_flip_length;
	}
	
	return path_length;
}

void flipatoms() {
	numlook++;
	int toflip;
	int flipiter = 0;

	//Decrease the taboo value of each variable
	for(int i = 0; i < numvars; i++) {
		if(true_var_weights[i] == 0.5) continue;
		if(true_var_weights[i] < 0.5) { true_var_weights[i] *= true_weight_mult; }
		if(true_var_weights[i] > 0.5) { true_var_weights[i] = 1-((1-true_var_weights[i])*true_weight_mult); }
		//			fprintf(stderr, "%4.3f ", true_var_weights[i]);
	}
	//		fprintf(stderr, "\n");
	
	while(varstoflip[flipiter]!=0) {
		toflip = varstoflip[flipiter];
		flipiter++;
		atom[toflip] = 1-atom[toflip];
		changed[toflip] = numflip;

		if(atom[toflip] == 0)
		  true_var_weights[toflip] = true_weight_max;
		else
		  true_var_weights[toflip] = 1-true_weight_max;
	}
	
	flipiter = 0;
	while(varstoflip[flipiter]!=0) {
		toflip = varstoflip[flipiter];
		flipiter++;
		int numocc = occurance[toflip].length;
		int *occptr = occurance[toflip].num;

		for(int i = 0; i < numocc ;i++) {
			int cli = *(occptr++);
			//cli = occurance[toflip].num[i];
			if(previousState[cli].visited == numlook) continue;
			previousState[cli].visited = numlook;
			//Removing the influence every variable in this BDD has on this BDD.
			if(traverseBDD(functions[cli])) {
				//BDD is SAT
				if(previousState[cli].IsSAT == 0) {
					previousState[cli].IsSAT = 1;
					numfalse--;
					//fprintf(stderr, "removing fB=%d, moving fB=%d to spot %d\n", falseBDDs[wherefalse[cli]], falseBDDs[numfalse], wherefalse[cli]);
					falseBDDs[wherefalse[cli]] = falseBDDs[numfalse];
					wherefalse[falseBDDs[numfalse]] = wherefalse[cli];
				}
			} else {
				if(previousState[cli].IsSAT == 1) {
					previousState[cli].IsSAT = 0;
					//fprintf(stderr, "adding fB=%d to spot %d\n", cli, numfalse);
					falseBDDs[numfalse] = cli;
					wherefalse[cli] = numfalse;
					numfalse++;
				}
			}
		}
	}
}

void print_current_assign(void) {
	int i;
	
	printf("Begin assign at flip = %ld\n", numflip);
	for (i=1; i<=numvars; i++){
		printf(" %d", atom[i]==0 ? -i : i);
		if (i % 10 == 0) printf("\n");
	}
	if ((i-1) % 10 != 0) printf("\n");
	printf("End assign\n");
}

void update_statistics_start_try(void)
{
	int i;
	
	lowbad = numfalse;
	
	sample_size = 0;
	sumfalse = 0.0;
	sumfalse_squared = 0.0;
	
	for (i=0; i<HISTMAX; i++)
	  tailhist[i] = 0;
	if (tail_start_flip == 0){
		tailhist[numfalse < HISTMAX ? numfalse : HISTMAX - 1] ++;
	}
}

void update_statistics_end_flip(void)
{
	if (numfalse < lowbad){
		lowbad = numfalse;
	}
	if (numflip >= tail_start_flip){
		tailhist[(numfalse < HISTMAX) ? numfalse : (HISTMAX - 1)] ++;
		if ((numflip % samplefreq) == 0){
			sumfalse += numfalse;
			sumfalse_squared += numfalse * numfalse;
			sample_size ++;
		}
	}
}

void update_and_print_statistics_end_try(void)
{
	totalflip += numflip;
	x += numflip;
	r ++;

	if (sample_size > 0){
		avgfalse = sumfalse/sample_size;
		second_moment_avgfalse = sumfalse_squared / sample_size;
		variance_avgfalse = second_moment_avgfalse - (avgfalse * avgfalse);
		if (sample_size > 1) { variance_avgfalse = (variance_avgfalse * sample_size)/(sample_size - 1); }
		std_dev_avgfalse = sqrt(variance_avgfalse);
		
		ratio_avgfalse = avgfalse / std_dev_avgfalse;
		
		sum_avgfalse += avgfalse;
		sum_std_dev_avgfalse += std_dev_avgfalse;
		number_sampled_runs += 1;
		
		if (numfalse == 0){
			suc_number_sampled_runs += 1;
			suc_sum_avgfalse += avgfalse;
			suc_sum_std_dev_avgfalse += std_dev_avgfalse;
		} else {
			nonsuc_number_sampled_runs += 1;
			nonsuc_sum_avgfalse += avgfalse;
			nonsuc_sum_std_dev_avgfalse += std_dev_avgfalse;
		}
	} else{
		avgfalse = 0;
		variance_avgfalse = 0;
		std_dev_avgfalse = 0;
		ratio_avgfalse = 0;
	}
	
	if(numfalse == 0){
		
		save_solution();
		numsuccesstry++;
		
		totalsuccessflip += numflip;
		integer_sum_x += x;
		sum_x = (double) integer_sum_x;
		sum_x_squared += ((double)x)*((double)x);
		mean_x = sum_x / numsuccesstry;
		if (numsuccesstry > 1){
			second_moment_x = sum_x_squared / numsuccesstry;
			variance_x = second_moment_x - (mean_x * mean_x);
			/* Adjustment for small small sample size */
			variance_x = (variance_x * numsuccesstry)/(numsuccesstry - 1);
			std_dev_x = sqrt(variance_x);
			std_error_mean_x = std_dev_x / sqrt((double)numsuccesstry);
		}
		sum_r += r;
		mean_r = ((double)sum_r)/numsuccesstry;
		sum_r_squared += ((double)r)*((double)r);
		
		x = 0;
		r = 0;
	}

	printf(" %9li %9i %9.2f %9.2f %9.2f %9li %9i",
			 lowbad,numfalse,avgfalse, std_dev_avgfalse,ratio_avgfalse,numflip, (numsuccesstry*100)/numtry);
	if (numsuccesstry > 0){
		printf(" %9li", totalsuccessflip/numsuccesstry);
		printf(" %9.2f", mean_x);
		if (numsuccesstry > 1){
			printf(" %9.2f", std_dev_x);
		}
	}
//	printf("%d", numflip);
	printf("\n");
	
	if(numfalse == 0 && countunsat() != 0){
		fprintf(stderr, "Program error, verification of solution fails!\n");
		exit(-1);
	}
	
	fflush(stdout);
}

void print_statistics_final(void)
{
	seconds_per_flip = expertime / totalflip;
	printf("\ntotal elapsed seconds = %f\n", expertime);
	printf("average flips per second = %ld\n", (long)(totalflip/expertime));
	printf("number solutions found = %d\n", numsuccesstry);
	printf("final success rate = %f\n", ((double)numsuccesstry * 100.0)/numtry);
	printf("average length successful tries = %li\n", numsuccesstry ? (totalsuccessflip/numsuccesstry) : 0);
	if (numsuccesstry > 0)
	  {
		  printf("mean flips until assign = %f\n", mean_x);
		  if (numsuccesstry>1){
			  printf("  variance = %f\n", variance_x);
			  printf("  standard deviation = %f\n", std_dev_x);
			  printf("  standard error of mean = %f\n", std_error_mean_x);
		  }
		  printf("mean seconds until assign = %f\n", mean_x * seconds_per_flip);
		  if (numsuccesstry>1){
			  printf("  variance = %f\n", variance_x * seconds_per_flip * seconds_per_flip);
			  printf("  standard deviation = %f\n", std_dev_x * seconds_per_flip);
			  printf("  standard error of mean = %f\n", std_error_mean_x * seconds_per_flip);
		  }
		  printf("mean restarts until assign = %f\n", mean_r);
		  if (numsuccesstry>1){
			  variance_r = (sum_r_squared / numsuccesstry) - (mean_r * mean_r);
			  if (numsuccesstry > 1) variance_r = (variance_r * numsuccesstry)/(numsuccesstry - 1);	   
			  std_dev_r = sqrt(variance_r);
			  std_error_mean_r = std_dev_r / sqrt((double)numsuccesstry);
			  printf("  variance = %f\n", variance_r);
			  printf("  standard deviation = %f\n", std_dev_r);
			  printf("  standard error of mean = %f\n", std_error_mean_r);
		  }
	  }
	
	if (number_sampled_runs){
      mean_avgfalse = sum_avgfalse / number_sampled_runs;
      mean_std_dev_avgfalse = sum_std_dev_avgfalse / number_sampled_runs;
      ratio_mean_avgfalse = mean_avgfalse / mean_std_dev_avgfalse;
		
      if (suc_number_sampled_runs){
			suc_mean_avgfalse = suc_sum_avgfalse / suc_number_sampled_runs;
			suc_mean_std_dev_avgfalse = suc_sum_std_dev_avgfalse / suc_number_sampled_runs;
			suc_ratio_mean_avgfalse = suc_mean_avgfalse / suc_mean_std_dev_avgfalse;
      } else {
			suc_mean_avgfalse = 0;
			suc_mean_std_dev_avgfalse = 0;
			suc_ratio_mean_avgfalse = 0;
      }
		
      if (nonsuc_number_sampled_runs){
			nonsuc_mean_avgfalse = nonsuc_sum_avgfalse / nonsuc_number_sampled_runs;
			nonsuc_mean_std_dev_avgfalse = nonsuc_sum_std_dev_avgfalse / nonsuc_number_sampled_runs;
			nonsuc_ratio_mean_avgfalse = nonsuc_mean_avgfalse / nonsuc_mean_std_dev_avgfalse;
      } else {
			nonsuc_mean_avgfalse = 0;
			nonsuc_mean_std_dev_avgfalse = 0;
			nonsuc_ratio_mean_avgfalse = 0;
      }
		
      printf("final noise level statistics\n");
      printf("    statistics over all runs:\n");
      printf("      overall mean average noise level = %f\n", mean_avgfalse);
      printf("      overall mean noise std deviation = %f\n", mean_std_dev_avgfalse);
      printf("      overall ratio mean noise to mean std dev = %f\n", ratio_mean_avgfalse);
      printf("    statistics on successful runs:\n");
      printf("      successful mean average noise level = %f\n", suc_mean_avgfalse);
      printf("      successful mean noise std deviation = %f\n", suc_mean_std_dev_avgfalse);
      printf("      successful ratio mean noise to mean std dev = %f\n", suc_ratio_mean_avgfalse);
      printf("    statistics on nonsuccessful runs:\n");
      printf("      nonsuccessful mean average noise level = %f\n", nonsuc_mean_avgfalse);
      printf("      nonsuccessful mean noise std deviation = %f\n", nonsuc_mean_std_dev_avgfalse);
      printf("      nonsuccessful ratio mean noise to mean std dev = %f\n", nonsuc_ratio_mean_avgfalse);
	}
	
	if (numsuccesstry > 0){
		printf("ASSIGNMENT FOUND\n");
		//print_sol_cnf();
	} else
	  printf("ASSIGNMENT NOT FOUND\n");
}

void print_sol_cnf(void)
{
    int i;
    for(i = 1;i < numvars+1;i++)
	printf("v %i\n", solution[i] == 1 ? i : -i);
}

void
save_solution(void)
{
  if (result_display_type) {
    /* create another node in solution chain */
    //Should really check against saving multiple solutions

    t_solution_info *tmp_solution_info;
    tmp_solution_info = (t_solution_info*)calloc(1, sizeof(t_solution_info));
    
    if (solution_info_head == NULL) {
      solution_info = tmp_solution_info;
      solution_info_head = solution_info;
    } else {
      solution_info->next = (struct _t_solution_info*)tmp_solution_info;
      solution_info = (t_solution_info*)(solution_info->next);
    }
    tmp_solution_info->nNumElts = numvars+1;
    tmp_solution_info->arrElts = new int[numvars+2];
    
    for (int i = 0; i<=numvars; i++) {
      tmp_solution_info->arrElts[i] = (atom[i]>0)?BOOL_TRUE:BOOL_FALSE;
    }
  }
  
  int i;
  
  for (i=1; i<=numvars; i++)
    solution[i] = atom[i];
  //	for(i = 1;i <= numvars+1;i++) {
  //		printf("v %i\n", solution[i] == 1 ? i : -i);
  //		printf("v %d\n", solution_info->arrElts[i]);
  //	}
  
}

int countunsat(void)
{
	int i, unsat;
	
	unsat = 0;
	for (i=0;i < numBDDs;i++)
	  {
		  if(!traverseBDD(functions[i]))
			 {
//				 printBDDerr(functions[i]);
//				 fprintf(stderr, "\n");
				 unsat++;
			 }
	  }
	return unsat;
}

void verifymbcount() {
	int *b1 = new int[numvars+1];
	int *m1 = new int[numvars+1];

	for(int x = 0; x < numvars+1; x++) {
		b1[x] = 0;
		m1[x] = 0;		  
	}
	
	for(int i = 0;i < numBDDs;i++) {
		BDDNode *f = functions[i];
		if(traverseBDD(f)) {
			while(!IS_TRUE_FALSE(f)) {
				if(atom[f->variable] == 1) {
					if(!traverseBDD(f->elseCase)) {
						b1[f->variable]++;
					}
					f = f->thenCase;
				} else {
					if(!traverseBDD(f->thenCase)) {
						b1[f->variable]++;
					}
					f = f->elseCase;
				}
			}
		} else {
			while(!IS_TRUE_FALSE(f)) {
				if(atom[f->variable] == 1) {
					if(traverseBDD(f->elseCase)) {
						m1[f->variable]++;
					}
					f = f->thenCase;
				} else {
					if(traverseBDD(f->thenCase)) {
						m1[f->variable]++;
					}
					f = f->elseCase;
				}
			}
		}
	}
	for(int x = 1; x < numvars+1; x++) {
		if(b1[x] != breakcount[x] || m1[x] != makecount[x]) {
			fprintf(stderr, "inconsistent");
			exit(1);			
		}
	}
}

