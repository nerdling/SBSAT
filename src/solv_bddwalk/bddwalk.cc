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

/* walksat version 35hw4 */
/* version 1 by Bram Cohen 7/93 */
/* versions 2 - 35  by Henry Kautz */
/* versions 35h by Holger H Hoos -- all modifications marked "hh" */
/* version 35hw* by Sean Weaver -- modified to solve on BDDs instead of CNF */

#include "sbsat.h"
#include "sbsat_solver.h"

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
#define HISTMAX 101		/* length of histogram of tail */

#define TRUE 1
#define FALSE 0

#define MAX_NODES_PER_BDD 5000

int numvars; //number of variables
int numBDDs; //number of BDDs (numout | nmbrFunctions)

BDDState *previousState;    /* records which BDDs are true, and which are false */

int *falseBDDs; 		/* All false BDDs are referenced here */
int *wherefalse;  	/* where each BDD is listed in falseBDDs */
int *wlength;			/* number of variables in each BDD */

intlist *wvariables;  /* a list of variables in each BDD */

intlist *occurance;	/* where each variable occurs */
                     /* indexed as occurance[variable].num[occurance_num] */
                     /* the number of BDDs each variable occurs in */
							/* is occurance[variable].length */

int *atom;		      /* value of each atom, the current assignment */

int breakcount; /* number of BDDs that become unsat if var if flipped */
int makecount;	 /* number of BDDs that become sat if var if flipped */

int numfalse;	 /* number of currently unsatisfied BDDs */

int *varstoflip;           /* variables to be flipped next */
int *best_to_flip;         /* best true path found so far */
int best_to_flip_length;   /* length of best_to_flip */
int *second_to_flip;       /* second best true path found so far */
int second_to_flip_length; /* length of second_to_flip */

double *true_var_weights;         /* holds the taboo weights */
float true_weight = 0.5;          /* the prob. of picking a variable to be true */
float true_weight_multi = NOVALUE;/* multiplier to decrease the probability */
                                  /* of flipping a variable */
float taboo_max=NOVALUE;          /* how many multi-flips until a variable's */
                                  /* weight will return to normal (true_weight) */
float taboo_length = NOVALUE;     /* how many multi-flips will pass before */
                                  /* a variable is allowed to be flipped again */
float true_weight_taboo = true_weight; /* used to determine the probability of flipping */
                                       /* a variable again after it's just been flipped */
float true_weight_max = true_weight; /* used to determine the availiability of flipping */
                                     /* a variable again after it's just been flipped */
                                     /* This is like using a taboo list */
float path_factor; /* Number of random paths choosen = total number of variables in this BDD * path_factor */
                   /* path_factor is dynamically modified during the search */
                   /* more paths will be choosen when there are less unsatisfied BDDS to choose from */

/**********************************************************/
/* ANOV & ANOV+ parameters                                */
/**********************************************************/

int invPhi = 5;
int invTheta = 6;

int lastAdaptFlip = 0;
int lastAdaptNumFalse = 0;

/************************************/
/* Global flags and parameters      */
/************************************/

float noise; /* choose second_to_flip with noise frequency */

int wp_numerator = NOVALUE; /* choose a random path to true with numerator/denominator frequency */
int wp_denominator = 10000;

long int numflip; /* number of single variable flips so far */
long int numlook; /* used to make sure we don't traverse BDDs we've already */
                  /* traversed this same flip */

int numrun = BIG;
int cutoff = 0; /* number of flips per random restart (on the command line --cutoff) */
int target = 0; /* number of BDDs left to be satisfied for a solution */
int numtry = 0; /* total attempts at solutions */
int numsol = 0; /* stop after finding this many solutions (on the command line --max-solutions) */

/* Histogram of tail */

long int tailhist[HISTMAX];	/* histogram of num unsat in tail of run */
int tail = 3;
int tail_start_flip;

/* Randomization */

int seed;			/* seed for random */
struct timeval tv;
struct timezone tzp;

extern double fStartTime;
extern double fEndTime;
long int lowbad;		/* lowest number of bad clauses during try */
long int totalflip = 0;		/* total number of flips in all tries so far */
long int totalsuccessflip = 0;	/* total number of flips in all tries which succeeded so far */
int numsuccesstry = 0;		/* total found solutions */

long flips_x;
long integer_sum_x = 0;
double sum_x = 0.0;
double sum_x_squared = 0.0;
double mean_x;
double second_moment_x;
double variance_x;
double std_dev_x;
double std_error_mean_x;
double seconds_per_flip;
int flips_r;
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

int pickrandom(void);
int picknoveltyplus(void);

void adaptNoveltyNoise(void);

int countunsat(void);
void init_CountFalses();
void initprob(void); /* Initialize data structures for the problem */
void freemem(void);
void flipatoms(void);

void Fill_Weights(void);
void save_solution(void);

void print_current_assign(void);

void print_statistics_header(void);
void update_statistics_start_try(void);
void update_and_print_statistics_end_try(void);
void print_statistics_mid_try(void);
void update_statistics_end_flip(void);
void print_statistics_final(void);

/************************************/
/* Main                             */
/************************************/

int test_for_break() {
	if (nTimeLimit && (get_runtime() - fStartTime) > nTimeLimit) {
		d2_printf2("Bailling out because the Time limit of %lds ", nTimeLimit);
		d2_printf2("is smaller than elapsed time %.0fs\n", (get_runtime() - fStartTime));
		return 1;
	}
	if (nCtrlC) {
		d2_printf1("Breaking out of BDD WalkSAT\n");
		return 1;
	}
	return 0;
}

int walkSolve() {
	/* Get values from the command line */
	numsol = max_solutions;
	cutoff = BDDWalkCutoff;
	true_weight_multi = (float)BDDWalktaboo_multi;
	taboo_max = (float)BDDWalktaboo_max;
	taboo_length = taboo_max-1.0;
	int (*heurpick)(); //pointer to function
	if(BDDWalkHeur=='a') heurpick = &picknoveltyplus;
	else if(BDDWalkHeur == 'n') heurpick = &picknoveltyplus;
	else if(BDDWalkHeur == 'r')  heurpick = &pickrandom;

	wp_numerator = (int)(BDDWalk_wp_prob*(float)wp_denominator);
	noise = BDDWalk_prob;
	
	gettimeofday(&tv,&tzp);
	seed = (( tv.tv_sec & 0177 ) * 1000000) + tv.tv_usec;
	if (numsol==0 || numsol>numrun) numsol = numrun;
	srandom(seed);
	initprob(); /* initialized the BDD structures */
	print_statistics_header();
	fStartTime = get_runtime();
	while ((numsuccesstry < numsol) && (numtry < numrun)) {
		numtry++;
		init_CountFalses();
		update_statistics_start_try();
		numflip = 0;
		numlook = 0;

		while((numfalse > target) && (numflip < cutoff)) {
			numflip+=heurpick();
			flipatoms();
			if(BDDWalkHeur=='a') adaptNoveltyNoise();
			update_statistics_end_flip();
			
			if(test_for_break()) break;

			char term_char = term_getchar();
			if (term_char==' ') { //Display status
				d2_printf1("\b");
				print_statistics_mid_try();
			} else if (term_char=='r') { //Force a random restart
				d2_printf1("\b");
				break;
			} else if (term_char=='q') { //Quit
				d2_printf1("\b");
				nCtrlC = 1;
				break;
			}
		}
		update_and_print_statistics_end_try();

		if(test_for_break()) break;
	}
	fEndTime = get_runtime() - fStartTime;
	print_statistics_final();
	freemem();
	if (numsuccesstry!=0) return SOLV_SAT;
	return SOLV_UNKNOWN;
}

void initprob(void)
{
	numvars = getNuminp();
	numBDDs = nmbrFunctions;
	
	atom = new int[numvars+1];
	
	occurance = new intlist[numvars+1];
	wlength = new int[numBDDs];
	wvariables = new intlist[numBDDs];
	int *tempmem = new int[numvars+1];
	int *tempint = NULL;
	int tempint_max = 0;
	
	true_var_weights = new double[numvars+1];
	previousState = new BDDState[numBDDs];
	wherefalse = new int[numBDDs];
	falseBDDs = new int[numBDDs];
	varstoflip = new int[numvars+1];
	best_to_flip = new int[numvars+1];
	second_to_flip = new int[numvars+1];

	for(int i = 1;i <= numvars; i++) {
		true_var_weights[i] = true_weight; //Set the taboo weights
	}
	
	for(int x = 0; x < taboo_length; x++)
	  true_weight_taboo = true_weight_taboo / true_weight_multi;
	for(int x = 0; x < taboo_max; x++)
	  true_weight_max = true_weight_max / true_weight_multi;
	
	for(int x = 0; x <= numvars; x++) {
		tempmem[x] = 0;
	}
	
	for(int x = 0; x < numBDDs; x++) {
		int y = 0;
		unravelBDD (&y, &tempint_max, &tempint, functions[x]);
		if (y != 0) qsort (tempint, y, sizeof (int), compfunc);
		
		wlength[x] = y;
		wvariables[x].num = new int[y]; //(int *)calloc(y, sizeof(int));
		wvariables[x].length = y;
		for (int i = 0; i < y; i++) {
			wvariables[x].num[i] = tempint[i];
			tempmem[tempint[i]]++;
		}
	}	

	//length is done
	//wvariables is done
	
	for (int x = 0; x <= numvars; x++) {
		occurance[x].num = new int[tempmem[x]];
		occurance[x].length = 0;
	}
	delete [] tempmem;
	
	for (int x = 0; x < numBDDs; x++) {
		for (int i = 0; i < wlength[x]; i++) {
			occurance[wvariables[x].num[i]].num[occurance[wvariables[x].num[i]].length] = x;
			occurance[wvariables[x].num[i]].length++;
		}
	}
	
	//occurance is done
	
	Fill_Weights(); //Each BDDNode now has the density of True's below it stored.

	ite_free((void**)&tempint); tempint_max = 0;
}

void freemem(void)
{
	delete [] atom;
	
	for (int x = 0; x <= numvars; x++)
	  delete [] occurance[x].num;
	delete [] occurance;
	
	for (int x = 0; x < numBDDs; x++)
	  delete [] wvariables[x].num;
	delete [] wvariables;
	
	delete [] wlength;
	
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
	d2_printf3("numvars = %i, numBDDs = %i\n",numvars,numBDDs);
   d2_printf1("wff read in\n\n");
	d2_printf1("    lowest     final       avg     noise     noise     total                 avg      mean      mean\n");
	d2_printf1("    #unsat    #unsat     noise   std dev     ratio     flips              length     flips     flips\n");
	d2_printf1("      this      this      this      this      this      this   success   success     until       std\n");
	d2_printf1("       try       try       try       try       try       try      rate     tries    assign       dev\n\n");

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

//Traverse the BDD and return if the current path (stored in atom[])
//satisfies or unsatisfies BDD f
inline bool traverseBDD(BDDNode *f) {
	assert(!IS_TRUE_FALSE(f)); //Assume every BDD is more complex than true or false
	while(1) {
		if(atom[f->variable] == TRUE) {
			f = f->thenCase;
		} else {
			f = f->elseCase;			  
		}
		if(f == false_ptr) return false;
		if(f == true_ptr)	return true;
	}
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
	
	if(arrVarTrueInfluences) {
		for(i = 1;i <= numvars; i++) {
			float random_num = (float)(random()%100/100.0);
			if((random_num*arrVarTrueInfluences[i]) >
				(((1.0-random_num)*(1.0-arrVarTrueInfluences[i]))))
			  atom[i] = 1;
			else
			  atom[i] = 0;						 
			//fprintf(stderr, "%d = %d (%4.2f), ", i, atom[i], random_num);
			true_var_weights[i] = true_weight; //Reset the taboo weights
			//Comment out for a small type of learning
		}
	} else {
		for(i = 1;i <= numvars; i++) {
			atom[i] = random()%2;  //This fills atom with a random truth assignment
			true_var_weights[i] = true_weight; //Reset the taboo weights
			//Comment out for a small type of learning
		}
	}
		
	/* Initialize previousState, wherefalse and falseBDDs in the following: */
	
	for(i = 0;i < numBDDs;i++) {
		previousState[i].visited = -1;
		if(!traverseBDD(functions[i])) { //BDD is falsified by the current assignment
			previousState[i].IsSAT = 0;
			wherefalse[i] = numfalse;
			falseBDDs[numfalse] = i;
			numfalse++;
		} else { //BDD is satisfied by the current assignment
			previousState[i].IsSAT = 1;
		}
	}
}

//This function finds a random path to True in BDD x and stores it
//in varstoflip[]. varstoflip[] consists of only variables in the path
//with opposite values of the current assignment.
inline int getRandomTruePath(int x) {
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
				//May be another way to do this taking into account
				//bdd->tbr_weight instead of true_var_weights.
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
			//  (1) We can set the variable randomly
			//  (2) We can leave the variable alone
			//  (3) We can set the variable based on it's taboo weight
			
			//Choose this variable's direction based on it's weight
			//(1)
			//if((random()%2) > 0) {

			//(2)
			if(1) { continue;

			//(3)
			//if((random()%100) < (true_var_weights[wvar]*100)) {
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
	makecount = 0; breakcount = 0;
	
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
					makecount++;
				}
			} else {
				//BDD is UNSAT
				if(previousState[cli].IsSAT == 1) {
					breakcount++;
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

int pickrandom(void)
{
	int tofix, BDDsize;
	int flippath = 0;
	
	tofix = falseBDDs[random()%numfalse]; //Maybe in the future not so random...
	                                      //Could be based on size of BDD (how many paths to true?)
	BDDsize = wlength[tofix];  

//	fprintf(stderr, "\ntofix=%d\n", tofix);

	int path_length = 0;
	
	flippath = 0;
	
	if(BDDWalkRandomOption == 1) //#1 Pick a random path to true in this BDD
	  path_length = getRandomTruePath(tofix);
	else if(BDDWalkRandomOption == 2) { //#2 Randomly flip variables in this BDD
		int i = 0;
		for(path_length = 0; path_length < wlength[tofix]; path_length++) {
			if((random()%2) > 0)
			  varstoflip[i++] = wvariables[tofix].num[path_length];
		}
		varstoflip[i] = 0;
	} else if(BDDWalkRandomOption == 3) {	//#3 Randomly flip one variable in the problem
		varstoflip[0] = random()%numvars+1;
		varstoflip[1] = 0;
		path_length = 1;
	} else if(BDDWalkRandomOption == 4) {	//#4 Randomly flip one variable in this BDD
		varstoflip[0] = wvariables[tofix].num[random()%BDDsize];
		varstoflip[1] = 0;
		path_length = 1;
	} else {
		fprintf(stderr, "pick-random given incorrect value - needs 1-4, given %d...exiting\n", BDDWalkRandomOption);
		exit(0);
	}
	return path_length;
}

//Adapted from "An Adaptive Noise Mechanism for WalkSAT" by Holger H. Hoos
void adaptNoveltyNoise() {
	if ((numflip - lastAdaptFlip) > (numBDDs / invTheta)) {
		noise += (int) ((100 - noise) / invPhi);
		lastAdaptFlip = numflip;
		lastAdaptNumFalse = numfalse;
	} else if (numfalse < lastAdaptNumFalse) {
		noise -= (int) (noise / invPhi / 2);
		lastAdaptFlip = numflip;
		lastAdaptNumFalse = numfalse;
	}	
}

int picknoveltyplus(void) {
	int best_make = 0, second_best_make = 0;
	double best_diff, second_best_diff;
	int youngest_birthdate, best=0, second_best=0;

	float percent_satisfied = (float)(numBDDs-numfalse)/(float)numBDDs;
	for(int z = 0; z < 2; z++) percent_satisfied *= percent_satisfied;
	float percent_unsatisfied = 1.0-percent_satisfied;
	path_factor = 11.0-(percent_unsatisfied*10.0);
	
	double diff = 0.0;
	int tofix, BDDsize;
	int flippath = 0;
	
	tofix = falseBDDs[random()%numfalse]; //Maybe in the future not so random...
	                                      //Could be based on size of BDD (how many paths to true?)
	BDDsize = wlength[tofix];  

//	fprintf(stderr, "\ntofix=%d\n", tofix);
	
	int path_length = 0;

	/* hh: inserted modified loop breaker: */
	if ((random()%wp_denominator < wp_numerator)) {
		if(BDDWalkRandomOption == 1) //#1 Pick a random path to true in this BDD
		  path_length = getRandomTruePath(tofix);
		else if(BDDWalkRandomOption == 2) { //#2 Randomly flip every variable in this BDD
			int i = 0;
			for(path_length = 0; path_length < wlength[tofix]; path_length++) {
				if((random()%2) > 0)
				  varstoflip[i++] = wvariables[tofix].num[path_length];
			}
			varstoflip[i] = 0;
		} else if(BDDWalkRandomOption == 3) {	//#3 Randomly flip one variable in the problem
			varstoflip[0] = random()%numvars+1;
			varstoflip[1] = 0;
			path_length = 1;
		} else if(BDDWalkRandomOption == 4) {	//#4 Randomly flip one variable in this BDD
			varstoflip[0] = wvariables[tofix].num[random()%BDDsize];
			varstoflip[1] = 0;
			path_length = 1;
		} else {
			fprintf(stderr, "random-option given incorrect value - needs 1-4, given %d...exiting\n", BDDWalkRandomOption);
			exit(0);
		}
	} else {
		youngest_birthdate = -1;
		best_diff = -1000000.0;
		second_best_diff = -1000000.0;

		unmark(functions[tofix]);
		weight_bdd(functions[tofix]);
		for(int i = 0; i < wlength[tofix]*path_factor; i++) {
			path_length = getRandomTruePath(tofix);
			
			getmbcountTruePath();
			//diff = makecount - breakcount;
			diff = -breakcount;
			//fprintf(stderr, "{mp=%d, bp=%d, diff=%f}", makecount, breakcount, diff);
			
			if (diff > best_diff || (diff == best_diff && makecount > best_make)) {
				/* found new best, demote best to 2nd best */
				if(best) {
					for(int z = 0; z <= best_to_flip_length; z++)
					  second_to_flip[z] = best_to_flip[z];
					second_to_flip_length = best_to_flip_length;
					second_best_diff = best_diff;
					second_best_make = best_make;
					second_best = 1;
				} else best = 1;
				
				for(int z = 0; z <= path_length; z++)
				  best_to_flip[z] = varstoflip[z];
				best_to_flip_length = path_length;
				best_diff = diff;
				best_make = makecount;
			} else if (diff > second_best_diff || (diff == second_best_diff && makecount > second_best_make)) {
				/* found new second best */
				for(int z = 0; z <= path_length; z++)
				  second_to_flip[z] = varstoflip[z];
				second_to_flip_length = path_length;
				second_best_diff = diff;
				second_best_make = makecount;
				second_best = 1;
			}
		}

		if(!second_best) flippath = 1;
		else if(best_diff == second_best_diff) flippath = 1;
		else if (random()%100 < noise) flippath = 2;
		else flippath = 1;
	}
	
	if(flippath == 1) { //Choose best path
		for(int z = 0; z <= best_to_flip_length; z++)
		  varstoflip[z] = best_to_flip[z];
		path_length = best_to_flip_length;
	} else if(flippath == 2) { //Choose second_best path
		for(int z = 0; z <= second_to_flip_length; z++)
		  varstoflip[z] = second_to_flip[z];
		path_length = second_to_flip_length;
	}
	
	return path_length;
}

void flipatoms() {
	numlook++;
	int toflip;
	int flipiter = 0;

	//Decrease the taboo value of each variable
	for(int i = 1; i <= numvars; i++) {
		if(true_var_weights[i] == true_weight) continue;
		if(true_var_weights[i] < true_weight) { true_var_weights[i] *= true_weight_multi; }
		if(true_var_weights[i] > true_weight) { true_var_weights[i] = 1-((1-true_var_weights[i])*true_weight_multi); }
		//fprintf(stderr, "%4.3f ", true_var_weights[i]);
	}
	//fprintf(stderr, "\n");
	
	while(varstoflip[flipiter]!=0) {
		toflip = varstoflip[flipiter];
		flipiter++;
		atom[toflip] = 1-atom[toflip];

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
				//BDD is UNSAT
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
	
	d2_printf2("Begin assign at flip = %ld\n", numflip);
	for (i=1; i<=numvars; i++){
		d2_printf2(" %d", atom[i]==0 ? -i : i);
		if (i % 10 == 0) d2_printf1("\n");
	}
	if ((i-1) % 10 != 0) d2_printf1("\n");
	d2_printf1("End assign\n");
}

void update_statistics_start_try(void) {
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

void update_statistics_end_flip(void) {
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

void print_statistics_mid_try(void) {
	if (sample_size > 0) {
		avgfalse = sumfalse/sample_size;
		second_moment_avgfalse = sumfalse_squared / sample_size;
		variance_avgfalse = second_moment_avgfalse - (avgfalse * avgfalse);
		if (sample_size > 1) { variance_avgfalse = (variance_avgfalse * sample_size)/(sample_size - 1); }
		std_dev_avgfalse = sqrt(variance_avgfalse);
		
		ratio_avgfalse = avgfalse / std_dev_avgfalse;
	} else {
		avgfalse = 0;
		variance_avgfalse = 0;
		std_dev_avgfalse = 0;
		ratio_avgfalse = 0;
	}
	
	d2_printf8("*%9li %9i %9.2f %9.2f %9.2f %9li %9i",
				  lowbad,numfalse,avgfalse, std_dev_avgfalse,ratio_avgfalse,numflip, (numtry>1?((numsuccesstry*100)/(numtry-1)):0));
	if (numsuccesstry > 1) {
		d2_printf2(" %9li", totalsuccessflip/numsuccesstry);
		d2_printf2(" %9.2f", mean_x);
		if (numsuccesstry > 1){
			d2_printf2(" %9.2f", std_dev_x);
		}
	}
//	d2_printf2("%d", numflip);
	d2_printf1("\r");
	
	fflush(stdout);
}

void update_and_print_statistics_end_try(void) {
	totalflip += numflip;
	flips_x += numflip;
	flips_r++;

	if (sample_size > 0) {
		avgfalse = sumfalse/sample_size;
		second_moment_avgfalse = sumfalse_squared / sample_size;
		variance_avgfalse = second_moment_avgfalse - (avgfalse * avgfalse);
		if (sample_size > 1) { variance_avgfalse = (variance_avgfalse * sample_size)/(sample_size - 1); }
		std_dev_avgfalse = sqrt(variance_avgfalse);
		
		ratio_avgfalse = avgfalse / std_dev_avgfalse;
		
		sum_avgfalse += avgfalse;
		sum_std_dev_avgfalse += std_dev_avgfalse;
		number_sampled_runs += 1;
		
		if (numfalse == 0) {
			suc_number_sampled_runs += 1;
			suc_sum_avgfalse += avgfalse;
			suc_sum_std_dev_avgfalse += std_dev_avgfalse;
		} else {
			nonsuc_number_sampled_runs += 1;
			nonsuc_sum_avgfalse += avgfalse;
			nonsuc_sum_std_dev_avgfalse += std_dev_avgfalse;
		}
	} else {
		avgfalse = 0;
		variance_avgfalse = 0;
		std_dev_avgfalse = 0;
		ratio_avgfalse = 0;
	}
	
	if(numfalse == 0) {
		save_solution();
		numsuccesstry++;
		
		totalsuccessflip += numflip;
		integer_sum_x += flips_x;
		sum_x = (double) integer_sum_x;
		sum_x_squared += ((double)flips_x)*((double)flips_x);
		mean_x = sum_x / numsuccesstry;
		if (numsuccesstry > 1){
			second_moment_x = sum_x_squared / numsuccesstry;
			variance_x = second_moment_x - (mean_x * mean_x);
			/* Adjustment for small small sample size */
			variance_x = (variance_x * numsuccesstry)/(numsuccesstry - 1);
			std_dev_x = sqrt(variance_x);
			std_error_mean_x = std_dev_x / sqrt((double)numsuccesstry);
		}
		sum_r += flips_r;
		mean_r = ((double)sum_r)/numsuccesstry;
		sum_r_squared += ((double)flips_r)*((double)flips_r);
		
		flips_x = 0;
		flips_r = 0;

		//This is a double check. Really this is unnecessary
		//if you uncomment these lines in init_CountFalses()
		for(int i = 1;i <= numvars; i++) {
			true_var_weights[i] = true_weight; //Reset the taboo weights
		}
		
		if(countunsat() != 0) {
			fprintf(stderr, "Program error, verification of solution fails!\n");
			exit(-1);
		}
	}

	d2_printf8(" %9li %9i %9.2f %9.2f %9.2f %9li %9i",
				  lowbad,numfalse,avgfalse, std_dev_avgfalse,ratio_avgfalse,numflip, (numsuccesstry*100)/numtry);
	if (numsuccesstry > 0) {
		d2_printf2(" %9li", totalsuccessflip/numsuccesstry);
	   d2_printf2(" %9.2f", mean_x);
		if (numsuccesstry > 1){
			d2_printf2(" %9.2f", std_dev_x);
		}
	}
	//d2_printf2("%d", numflip);
	d2_printf1("\n");
	
	fflush(stdout);
}

void print_statistics_final(void) {
	seconds_per_flip = fEndTime / totalflip;
	d2_printf2("\ntotal elapsed seconds = %f\n", fEndTime);
	d2_printf2("average flips per second = %ld\n", (long)(totalflip/fEndTime));
	d2_printf2("number solutions found = %d\n", numsuccesstry);
	d2_printf2("final success rate = %f\n", ((double)numsuccesstry * 100.0)/numtry);
	d2_printf2("average length successful tries = %li\n", numsuccesstry ? (totalsuccessflip/numsuccesstry) : 0);
	if (numsuccesstry > 0) {
		d2_printf2("mean flips until assign = %f\n", mean_x);
		if (numsuccesstry>1) {
			d2_printf2("  variance = %f\n", variance_x);
			d2_printf2("  standard deviation = %f\n", std_dev_x);
			d2_printf2("  standard error of mean = %f\n", std_error_mean_x);
		}
		d2_printf2("mean seconds until assign = %f\n", mean_x * seconds_per_flip);
		if (numsuccesstry>1) {
			d2_printf2("  variance = %f\n", variance_x * seconds_per_flip * seconds_per_flip);
			d2_printf2("  standard deviation = %f\n", std_dev_x * seconds_per_flip);
			d2_printf2("  standard error of mean = %f\n", std_error_mean_x * seconds_per_flip);
		}
		d2_printf2("mean restarts until assign = %f\n", mean_r);
		if (numsuccesstry>1) {
			variance_r = (sum_r_squared / numsuccesstry) - (mean_r * mean_r);
			if (numsuccesstry > 1) variance_r = (variance_r * numsuccesstry)/(numsuccesstry - 1);	   
			std_dev_r = sqrt(variance_r);
			std_error_mean_r = std_dev_r / sqrt((double)numsuccesstry);
			d2_printf2("  variance = %f\n", variance_r);
			d2_printf2("  standard deviation = %f\n", std_dev_r);
			d2_printf2("  standard error of mean = %f\n", std_error_mean_r);
		}
	}
	
	if (number_sampled_runs) {
      mean_avgfalse = sum_avgfalse / number_sampled_runs;
      mean_std_dev_avgfalse = sum_std_dev_avgfalse / number_sampled_runs;
      ratio_mean_avgfalse = mean_avgfalse / mean_std_dev_avgfalse;
		
      if (suc_number_sampled_runs) {
			suc_mean_avgfalse = suc_sum_avgfalse / suc_number_sampled_runs;
			suc_mean_std_dev_avgfalse = suc_sum_std_dev_avgfalse / suc_number_sampled_runs;
			suc_ratio_mean_avgfalse = suc_mean_avgfalse / suc_mean_std_dev_avgfalse;
      } else {
			suc_mean_avgfalse = 0;
			suc_mean_std_dev_avgfalse = 0;
			suc_ratio_mean_avgfalse = 0;
      }
      if (nonsuc_number_sampled_runs) {
			nonsuc_mean_avgfalse = nonsuc_sum_avgfalse / nonsuc_number_sampled_runs;
			nonsuc_mean_std_dev_avgfalse = nonsuc_sum_std_dev_avgfalse / nonsuc_number_sampled_runs;
			nonsuc_ratio_mean_avgfalse = nonsuc_mean_avgfalse / nonsuc_mean_std_dev_avgfalse;
      } else {
			nonsuc_mean_avgfalse = 0;
			nonsuc_mean_std_dev_avgfalse = 0;
			nonsuc_ratio_mean_avgfalse = 0;
      }
		
      d2_printf1("final noise level statistics\n");
      d2_printf1("    statistics over all runs:\n");
      d2_printf2("      overall mean average noise level = %f\n", mean_avgfalse);
      d2_printf2("      overall mean noise std deviation = %f\n", mean_std_dev_avgfalse);
      d2_printf2("      overall ratio mean noise to mean std dev = %f\n", ratio_mean_avgfalse);
      d2_printf1("    statistics on successful runs:\n");
      d2_printf2("      successful mean average noise level = %f\n", suc_mean_avgfalse);
      d2_printf2("      successful mean noise std deviation = %f\n", suc_mean_std_dev_avgfalse);
      d2_printf2("      successful ratio mean noise to mean std dev = %f\n", suc_ratio_mean_avgfalse);
      d2_printf1("    statistics on nonsuccessful runs:\n");
      d2_printf2("      nonsuccessful mean average noise level = %f\n", nonsuc_mean_avgfalse);
      d2_printf2("      nonsuccessful mean noise std deviation = %f\n", nonsuc_mean_std_dev_avgfalse);
      d2_printf2("      nonsuccessful ratio mean noise to mean std dev = %f\n", nonsuc_ratio_mean_avgfalse);
	}
	
	if (numsuccesstry > 0) { d2_printf1("ASSIGNMENT FOUND\n"); }
	else { d2_printf1("ASSIGNMENT NOT FOUND\n"); }
}

void save_solution(void) {
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
}

int countunsat(void) {
	int i, unsat = 0;
	for (i=0;i < numBDDs;i++) {
		if(!traverseBDD(functions[i])) {
			//printBDDerr(functions[i]);
			//fprintf(stderr, "\n");
			unsat++;
		}
	}
	return unsat;
}
