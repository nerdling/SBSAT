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
/* walksat version 35hw4 */
/* version 1 by Bram Cohen 7/93 */
/* versions 2 - 35  by Henry Kautz */
/* versions 35h by Holger H Hoos -- all modifications marked "hh" */
/* version 35hw* by Sean Weaver -- modified to solve on BDDs instead of CNF */

#include "ite.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <sys/types.h>
#include <limits.h>
#include <signal.h>
#include <sys/times.h>
#include <sys/time.h>

#ifndef CLK_TCK
#define CLK_TCK 60
#endif

#define STOREBLOCK 2000000	/* size of block to malloc each time */
#define BIG 100000000

#define Var(CLAUSE, POSITION) (ABS(clause[CLAUSE][POSITION]))

#define NOVALUE -1
#define INIT_PARTIAL 1
#define HISTMAX 101		/* length of histogram of tail */

#define TRUE 1
#define FALSE 0

//#define USE_BRITTLE
//#define USE_FALSE_PATHS
//#define USE_SIMPLE_PATH_RES
#define USE_PATHS_TO_TRUE

#define MAX_NODES_PER_BDD 5000

static int scratch;
#define ABS(x) ((scratch=(x))>0?(scratch):(-scratch))

int numvars; //number of variables
int numBDDs; //number of BDDs (numout | nmbrFunctions)

int numliterals;

float *brittlecount;   /* records the brittleness a BDD changes to by flipping a var */

floatlist *previousBrittle;
dualintlist *previousState; /* records which BDDs are true, and which are false */
                            /* along with which variables changed the makecount(-) */
                            /* or breakcount(+) */
pathStruct *BDDTruePaths;   /* Holds the paths in a BDD which lead to True */
int pathmake, pathbreak;

int *falseBDDs; 		/* clauses which are false */
int *wherefalse;  	/* where each clause is listed in false */
int *wlength;			/* number of variables in each BDD */
int *lowfalse;
int *numtruelit;		/* number of true literals in each clause */

intlist *wvariables;  /* a list of variables in each BDD, could be used to */
                      /* makes 'wlength' an unnecessary variable */
int *wvisited;

intlist *occurence;	/* where each variable occurs */
                     /* indexed as occurence[variable].num[occurence_num] */
                     /* numoccurence is now occurence[variable].length */
							/* which is the number of times each variable occurs */

int *atom;		      /* value of each atom */ 
int *lowatom;
int *solution;

int *changed;		/* step at which atom was last flipped */

int *breakcount;	/* number of clauses that become unsat if var if flipped */
int *makecount;	/* number of clauses that become sat if var if flipped */

int numfalse;		/* number of false clauses */

int *varstoflip;
/************************************/
/* Global flags and parameters      */
/************************************/

int abort_flag;

int numerator = 50;//NOVALUE;	/* make random flip with numerator/denominator frequency */

int denominator = 100;

int wp_numerator = 1;//NOVALUE;	/* walk probability numerator/denominator */
int wp_denominator = 100;

long int numflip;		/* number of changes so far */
long int numnullflip;		/*  number of times a clause was picked, but no  */
				/*  variable from it was flipped  */
long int numlook;
int numrun = BIG;
int cutoff = BIG;//1000000;
int base_cutoff = 1000000;
int target = 0;
int numtry = 0;			/* total attempts at solutions */
int numsol = NOVALUE;	        /* stop after this many tries succeeds */

int makeflag = TRUE;		/* set to true by heuristics that require the make values to be calculated */

/* Histogram of tail */

long int tailhist[HISTMAX];	/* histogram of num unsat in tail of run */
long histtotal;
int tail = 3;
int tail_start_flip;

/* Initialization options */

char initfile[100] = { 0 };
int initoptions = FALSE;

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

/* Hamming calcualations */

char hamming_target_file[512] = { 0 };
char hamming_data_file[512] = { 0 };
int hamming_sample_freq;
int hamming_flag = FALSE;
int hamming_distance;
int *hamming_target;
void read_hamming_file(char initfile[]);
void open_hamming_data(char initfile[]);
int calc_hamming_dist(int atom[], int hamming_target[]);
FILE * hamming_fp;

/* Noise level */
int samplefreq = 1;

int picknoveltyplus(void);

int countunsat(void);
int countunsatfalses(void);
void verifymbcount();
void scanone(int argc, char *argv[], int i, int *varptr);
//void init(char initfile[], int initoptions);
void init_1false(char initfile[], int initoptions);
void init_CountFalses(char initfile[], int initoptions);
void initprob(void);                 /* create a new problem */
void freemem(void);
void flipatoms_true_paths(void);
void flipatoms(void);       /* changes the assignment of the literals */
                            /* specified in varstoflip[] */

void save_low_assign(void);
void save_solution(void);

void print_current_assign(void);

void print_statistics_header(void);
void initialize_statistics(void);
void update_statistics_start_try(void);
void print_statistics_start_flip(void);
void update_and_print_statistics_end_try(void);
void update_statistics_end_flip(void);
void print_statistics_final(void);
void print_sol_cnf(void);

//Both these in recordsol.cc
extern t_solution_info *solution_info;
extern t_solution_info *solution_info_head;

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
	initialize_statistics();
	print_statistics_header();
	abort_flag = FALSE;
	expertime = get_runtime();
	while ((!abort_flag) && (numsuccesstry < numsol) && (numtry < numrun)) {
		numtry++;
		init_CountFalses(initfile, initoptions);
		update_statistics_start_try();
		numflip = 0;
		numlook = 0;
		while((numfalse > target) && (numflip < cutoff)) {
			print_statistics_start_flip();
			numflip+=picknoveltyplus();
#ifdef USE_PATHS_TO_TRUE
			flipatoms_true_paths();
#else
			flipatoms();
#endif
			update_statistics_end_flip();
			if (nCtrlC) {
				d3_printf1("Breaking out of BDD WalkSAT\n");
				break;
			}
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
	if (numsol!=NOVALUE) return SOLV_SAT;
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
	
	occurence = new intlist[numvars+1];
	wlength = new int[numBDDs+1];
	wvariables = new intlist[numBDDs+1];
	wvisited = new int[numBDDs+1];
	int *tempmem = new int[numvars+1];
	int *tempint = new int[MAX_NODES_PER_BDD];
	
	changed = new int[numvars+1];
	breakcount = new int[numvars+1];
	makecount = new int[numvars+1];	
	previousState = new dualintlist[numBDDs+1]; //intlist[numBDDs+1];
	brittlecount = new float[numvars+1];
	previousBrittle = new floatlist[numBDDs+1];
	wherefalse = new int[numBDDs+1];
	falseBDDs = new int[numBDDs+1];
	varstoflip = new int[MAX_NODES_PER_BDD];

#ifdef USE_PATHS_TO_TRUE
	BDDTruePaths = new pathStruct[numBDDs+1];
#endif

	for(int x = 0; x < numvars+1; x++) {
		brittlecount[x] = 0;
		tempmem[x] = 0;
	}
	
	for(int x = 0; x < numBDDs; x++) {
		long y = 0;
		unravelBDD (&y, tempint, functions[x]);
		qsort (tempint, y, sizeof (int), compfunc);
		if (y != 0) {
			int v = 0;
			for (int i = 1; i < y; i++) {
				v++;
				if (tempint[i] == tempint[i-1])
				  v--;
				tempint[v] = tempint[i];
			}
			y = v+1;
		}
		
		wlength[x] = y;
		//previousState[x].num = NULL;
		//previousState[x].count = NULL;
		previousState[x].num = new int[wlength[x]+2];
		previousState[x].count = new int[wlength[x]+2];
		previousBrittle[x].num = new int[wlength[x]+2];
		previousBrittle[x].count = new float[wlength[x]+2];
		wvariables[x].num = new int[y+1];	//(int *)calloc(y+1, sizeof(int));
		for (int i = 0; i < y; i++) {
			wvariables[x].num[i] = tempint[i];
			tempmem[tempint[i]]++;
		}
	}	
	
	for (int x = 0; x < numvars+1; x++)
	  {
		  occurence[x].num = new int[tempmem[x]+1];
		  occurence[x].length = 0;
	  }
	delete [] tempmem;
	
	for (int x = 0; x < numBDDs; x++)
	  {
		  for (int i = 0; i < wlength[x]; i++)
			 {
				 occurence[wvariables[x].num[i]].num[occurence[wvariables[x].num[i]].length] = x;
				 occurence[wvariables[x].num[i]].length++;
			 }
	  }
	
	//Need "variables" for later! Don't delete!
	
	//occurence is done.
	//length is done.
	
	Fill_Density(); //Each BDDNode now has the density of True's below it stored.

#ifdef USE_PATHS_TO_TRUE
   for (int x = 0; x < numBDDs; x++) {
      fprintf(stderr, "[%d]     \r", x);
      int numTrues = countTrues(functions[x]);
      BDDTruePaths[x].paths = new intlist[numTrues];
      BDDTruePaths[x].numpaths = 0;
      findPathsToTrue (functions[x], tempint, BDDTruePaths[x].paths, &(BDDTruePaths[x].numpaths));
	}
	//The paths to true in each BDD have been recorded.

	/*
	for (int x = 0; x < numBDDs; x++) {
		fprintf(foutputfile, "\n%d: ", x);
	   printBDD(functions[x]);
		fprintf(foutputfile, "\n");
		for (int y = 0; y < BDDTruePaths[x].numpaths; y++) {
			for (int z = 0; z < BDDTruePaths[x].paths[y].length; z++)
			  fprintf (foutputfile, "%d ", BDDTruePaths[x].paths[y].num[z]);
			fprintf (foutputfile, "0\n");
		}
	}
	*/
	
#endif
	delete [] tempint;
	
}

void freemem(void)
{
	delete [] atom;
	delete [] lowatom;
	delete [] solution;
	
	for (int x = 0; x < numvars+1; x++)
	  delete [] occurence[x].num;
	delete [] occurence;
	
	for (int x = 0; x < numBDDs; x++)
	  delete [] wvariables[x].num;
	delete [] wvariables;
	
	delete [] wlength;
	delete [] wvisited;
	
	delete [] changed;
	delete [] breakcount;
	delete [] makecount;
	delete [] brittlecount;
	
	for (int x = 0; x < numBDDs; x++) {
		delete [] previousState[x].num;
		delete [] previousState[x].count;
		delete [] previousBrittle[x].num;
		delete [] previousBrittle[x].count;
	}

#ifdef USE_PATHS_TO_TRUE
	for (int x = 0; x < numBDDs; x++) {
		for (int y = 0; y < BDDTruePaths[x].numpaths; y++) {
			delete [] BDDTruePaths[x].paths[y].num;
		}
		delete [] BDDTruePaths[x].paths;
	}
	delete [] BDDTruePaths;
#endif
	
	delete [] previousState;
	delete [] previousBrittle;
		
	delete [] wherefalse;
	delete [] falseBDDs;
	delete [] varstoflip;
}

void initialize_statistics(void)
{  //it doesn't seem that this does too much...ask holger if he wants
	//to have this functionality (reading in hamming files)
	x = 0; r = 0;
	if (hamming_flag) {
		read_hamming_file(hamming_target_file);
		open_hamming_data(hamming_data_file);
	}
	tail_start_flip = tail * numvars;
	printf("tail starts after flip = %i\n", tail_start_flip);
	numnullflip = 0;
}

void read_hamming_file(char initfile[])
{
	int i;			/* loop counter */
	FILE * infile;
	int lit;    
	
	printf("loading hamming target file %s ...", initfile);
	
	if ((infile = fopen(initfile, "r")) == NULL){
		fprintf(stderr, "Cannot open %s\n", initfile);
		exit(1);
	}
	i=0;
	for(i = 1;i < numvars+1;i++)
	  hamming_target[i] = 0;
	
	while (fscanf(infile, " %d", &lit)==1){
		if (ABS(lit)>numvars){
			fprintf(stderr, "Bad hamming file %s\n", initfile);
			exit(1);
		}
		if (lit>0) hamming_target[lit]=1;
	}
	printf("done\n");
}

void open_hamming_data(char initfile[])
{
	if ((hamming_fp = fopen(initfile, "w")) == NULL){
		fprintf(stderr, "Cannot open %s for output\n", initfile);
		exit(1);
	}
}

int calc_hamming_dist(int atom[], int hamming_target[])
{
	int i;
	int dist = 0;
	
	for (i=1; i<=numvars; i++){
		if (atom[i] != hamming_target[i]) dist++;
	}
	return dist;
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

void print_statistics_start_flip(void)
{
//	if (printtrace && (numflip % printtrace == 0)){
//		printf(" %9i %9i                     %9li\n", lowbad,numfalse,numflip);
//		if (trace_assign)
//		  print_current_assign();
//		fflush(stdout);
//	}
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

int CountFalseBDD(BDDNode *f) {
	int count = 0;
	if(f == false_ptr) return 1;
	while(!IS_TRUE_FALSE(f)) {
		if(atom[f->variable] == TRUE) {
			if(f->thenCase == false_ptr) {
				f = f->elseCase;
				count++;
			} else f = f->thenCase;
		} else {
			if(f->elseCase == false_ptr) {
				f = f->thenCase;
				count++;
			} else f = f->elseCase;			  
		}
	}
	//fprintf(stderr, "{%d}", count);
	return count;
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

void updateBDDCounts(int i) {
	BDDNode *f = functions[i];
	int j = 0, k = 0;
	float top_density = f->density;
	int count = 0;

#ifdef USE_FALSE_PATHS
# ifdef USE_SIMPLE_PATH_RES
	count = CountFalseBDD(f);
# else
	if(!traverseBDD(f)) count = 1;
# endif
#else
	if(!traverseBDD(f)) count = 1;
#endif
	if(count==0) { //BDD is SAT
#ifndef USE_FALSE_PATHS
		previousState[i].num[0] = f->variable;
#endif
		while(!IS_TRUE_FALSE(f)) {
			top_density = f->density;
			if(atom[f->variable] == 1) {
#ifdef USE_FALSE_PATHS
# ifdef USE_SIMPLE_PATH_RES				
				int false_count = CountFalseBDD(f->elseCase);
# else
				int false_count = 0;
				if(!traverseBDD(f->elseCase)) false_count = 1;
# endif
				if(false_count > 0) { 
					breakcount[f->variable]+=false_count;
					previousState[i].count[j] = false_count;
					previousState[i].num[j++] = f->variable;
				}
#endif
#ifdef USE_BRITTLE
				previousBrittle[i].count[k] = (f->elseCase->density - f->thenCase->density)/top_density;
				previousBrittle[i].num[k++] = f->variable;
				brittlecount[f->variable]+=(f->elseCase->density - f->thenCase->density)/top_density;
#endif
				f = f->thenCase;
			} else {
#ifdef USE_FALSE_PATHS
# ifdef USE_SIMPLE_PATH_RES				
				int false_count = CountFalseBDD(f->thenCase);
# else
				int false_count = 0;
				if(!traverseBDD(f->thenCase)) false_count = 1;
# endif
				if(false_count > 0) {
					breakcount[f->variable]+=false_count;
					previousState[i].count[j] = false_count;
					previousState[i].num[j++] = f->variable;
				}
#endif
#ifdef USE_BRITTLE
				previousBrittle[i].count[k] = (f->thenCase->density - f->elseCase->density)/top_density;
				previousBrittle[i].num[k++] = f->variable;
				brittlecount[f->variable]+=(f->thenCase->density - f->elseCase->density)/top_density;
#endif
				f = f->elseCase;
			}
		}
#ifdef USE_FALSE_PATHS
		previousState[i].num[j] = 0;
#endif
#ifdef USE_BRITTLE
		previousBrittle[i].num[k] = 0;
#endif
	} else { //BDD is UNSAT
#ifndef USE_FALSE_PATHS
		previousState[i].num[0] = -(f->variable);
#endif
		while(!IS_TRUE_FALSE(f)) {
			top_density = f->density;
			if(atom[f->variable] == 1) {
#ifdef USE_FALSE_PATHS
# ifdef USE_SIMPLE_PATH_RES				
				int false_count = CountFalseBDD(f->elseCase);
# else
				int false_count = 0;
				if(!traverseBDD(f->elseCase)) false_count = 1;
# endif
				if(false_count != count) {
					makecount[f->variable]+=count-false_count;
					previousState[i].count[j] = count-false_count;
					previousState[i].num[j++] = -(f->variable);
				}
#endif
#ifdef USE_BRITTLE
				previousBrittle[i].count[k] = (f->elseCase->density - f->thenCase->density)/top_density;
				previousBrittle[i].num[k++] = f->variable;
				brittlecount[f->variable]+=(f->elseCase->density - f->thenCase->density)/top_density;
#endif
				f = f->thenCase;
			} else {
#ifdef USE_FALSE_PATHS
# ifdef USE_SIMPLE_PATH_RES				
				int false_count = CountFalseBDD(f->thenCase);
# else
				int false_count = 0;
				if(!traverseBDD(f->thenCase)) false_count = 1;
# endif
				if(false_count != count) {
					makecount[f->variable]+=count-false_count;
					previousState[i].count[j] = count-false_count;
					previousState[i].num[j++] = -(f->variable);
				}
#endif
#ifdef USE_BRITTLE
				previousBrittle[i].count[k] = (f->thenCase->density - f->elseCase->density)/top_density;
				previousBrittle[i].num[k++] = f->variable;
				brittlecount[f->variable]+=(f->thenCase->density - f->elseCase->density)/top_density;
#endif
				f = f->elseCase;
			}
		}
#ifdef USE_FALSE_PATHS
		previousState[i].num[j] = 0;
#endif
#ifdef USE_BRITTLE
		previousBrittle[i].num[k] = 0;
#endif
	}
}

void init_CountFalses(char initfile[], int initoptions)
{
	int i;
	
	numfalse = 0;
	
	for(i = 0;i < numvars+1;i++)
	  {
		  changed[i] = -BIG;
		  breakcount[i] = 0;
		  makecount[i] = 0;
		  brittlecount[i] = 0;
	  }
	
	for(i = 1;i < numvars+1;i++)
	  atom[i] = random()%2;  //This makes a random truth assignment
	
	/* Initialize breakcount, makecount, brittlecount, previousBrittle, and previousState in the following: */
	
	for(i = 0;i < numBDDs;i++) {
		wvisited[i] = -1;
#ifndef USE_PATHS_TO_TRUE
		updateBDDCounts(i);
#endif
		if(!traverseBDD(functions[i])) {
#ifdef USE_PATHS_TO_TRUE
			previousState[i].num[0] = -functions[i]->variable;
			previousState[i].num[1] = 0;
#endif
			wherefalse[i] = numfalse;
			falseBDDs[numfalse] = i;
			numfalse++;
		} else {
#ifdef USE_PATHS_TO_TRUE
			previousState[i].num[0] = functions[i]->variable;
			previousState[i].num[1] = 0;
#endif
		}
	}
	
	//	for(int x = 1; x < numvars+1; x++)
	//	  {
	//		  fprintf(stderr, "\n%d: bc=%d, mc=%d, diff=%d", x, breakcount[x], makecount[x], (makecount[x] - breakcount[x]));
	//	  }
	//	fprintf(stderr, "\n");
	if (hamming_flag) { //Doesn't do this, hamming_flag == false
		hamming_distance = calc_hamming_dist(atom, hamming_target);
		fprintf(hamming_fp, "0 %i\n", hamming_distance);
	}
}

void getmbcountTruePath(intlist *path) {

	pathmake = 0; pathbreak = 0;
	
	//Temporarily set all variables in the global solution
	for(int i = 0; i < path->length; i++) {
		if((path->num[i] < 0 && atom[abs(path->num[i])] == 1) || (path->num[i] > 0 && atom[abs(path->num[i])] == 0)) {
			path->num[i] = -path->num[i]; //Flipping all variables that don't match the path
			atom[abs(path->num[i])] = 1-atom[abs(path->num[i])];
		}
	}
	for(int x = 0; x < path->length; x++) {
		
		int numocc = occurence[abs(path->num[x])].length;
		int *occptr = occurence[abs(path->num[x])].num;
		
		for(int i = 0; i < numocc ;i++) {
			int cli = *(occptr++);
			//cli = occurence[toflip].num[i];
			if(wvisited[cli] == numlook) continue;
			wvisited[cli] = numlook;
			//Removing the influence every variable in this BDD has on this BDD.
			if(traverseBDD(functions[cli])) {
				//BDD is SAT
				if(previousState[cli].num[0] < 0) {
					pathmake++;
				}
			} else {
				if(previousState[cli].num[0] > 0) {
					pathbreak++;
				}
			}
		}
	}
	
	//Reset all variables in the global solution
	for(int i = 0; i < path->length; i++) {
		if((path->num[i] < 0 && atom[abs(path->num[i])] == 1) || (path->num[i] > 0 && atom[abs(path->num[i])] == 0)) {
			path->num[i] = -path->num[i]; //Unflipping all variables that didn't match the path
			atom[abs(path->num[i])] = 1-atom[abs(path->num[i])];
		}
	}
	
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

int picknoveltyplus(void)
{
	int var, birthdate;
	float diff;
	int youngest=0, youngest_birthdate, best=0, second_best=0;
	float best_diff, second_best_diff;
	int tofix, BDDsize, i;
	
	tofix = falseBDDs[random()%numfalse]; //Maybe in the future not so random...
	BDDsize = wlength[tofix];  

//	fprintf(stderr, "\ntofix=%d\n", tofix);
	
	//This will never happen
	//if (BDDsize == 1) {
	//	varstoflip[0] = wvariables[tofix].num[0];
	//	varstoflip[1] = 0;
	//	return 1;
	//}

	int flipvar = 0;
	int flippath = 0;
	int path;
	int y;
	//SEAN!!! LOOK HERE!!!
	//Maybe random variable is good, !should try random true_path as well!.
	/* hh: inserted modified loop breaker: */
	if ((random()%wp_denominator < wp_numerator)) flipvar = wvariables[tofix].num[random()%BDDsize];
	else {
		//	youngest = -1;
		//	best = -1;
		//	second_best = -1;
		youngest_birthdate = -1;
		best_diff = -BIG;
		second_best_diff = -BIG;

#ifdef USE_PATHS_TO_TRUE
		for(i = 0; i < BDDTruePaths[tofix].numpaths; i++) {
			numlook++;
			getmbcountTruePath(&BDDTruePaths[tofix].paths[i]);
			path = i;
			diff = (float)pathmake - (float)pathbreak;
			//fprintf(stderr, "{mp=%d, bp=%d, diff=%f}", pathmake, pathbreak, diff);
			//SEAN!!! LOOK HERE!!! BIRTHDAYS!!!
			//Ugg...birthdays for multiple variables...ugg! No birthdays right now...
			//birthdate = changed[path];
			//if (birthdate > youngest_birthdate){
			//	youngest_birthdate = birthdate;
			//	youngest = var;
			//}
			if (diff > best_diff || (diff == best_diff)) {// && changed[path] < changed[best])) {
				/* found new best, demote best to 2nd best */
				second_best = best;
				second_best_diff = best_diff;
				best = path;
				best_diff = diff;
			}
			else if (diff > second_best_diff || (diff == second_best_diff)) {// && changed[var] < changed[second_best])){
				/* found new second best */
				second_best = path;
				second_best_diff = diff;
			}
		}
		//if (best != youngest) flipvar = best;
		//else
		if ((random()%denominator < numerator)) flippath = second_best;
		else flippath = best;
	}
	
	y = 0;
	for(i = 0; i < BDDTruePaths[tofix].paths[flippath].length; i++) {
		if((BDDTruePaths[tofix].paths[flippath].num[i] > 0 && atom[abs(BDDTruePaths[tofix].paths[flippath].num[i])] == 0) ||
			(BDDTruePaths[tofix].paths[flippath].num[i] < 0 && atom[abs(BDDTruePaths[tofix].paths[flippath].num[i])] == 1)) {
			varstoflip[y] = abs(BDDTruePaths[tofix].paths[flippath].num[i]);
			y++;
		}
	}	
	varstoflip[y] = 0;
#else
		for(i = 0; i < BDDsize; i++){
			var = wvariables[tofix].num[i];
			diff = (float)makecount[var] - (float)breakcount[var];
			//diff = brittlecount[var]/(float)occurence[var].length;
			//diff = brittlecount[var];
		   //diff = ((float)makecount[var] - (float)breakcount[var]) + (brittlecount[var]/(float)occurence[var].length);
			//diff = ((float)makecount[var] - (float)breakcount[var]) + brittlecount[var];
			//fprintf(stderr, "{%f4.3:", diff);
			//fprintf(stderr, "%f4.3}", brittlecount[var]);
			birthdate = changed[var];
			if (birthdate > youngest_birthdate){
				youngest_birthdate = birthdate;
				youngest = var;
			}
			if (diff > best_diff || (diff == best_diff && changed[var] < changed[best])) {
				/* found new best, demote best to 2nd best */
				second_best = best;
				second_best_diff = best_diff;
				best = var;
				best_diff = diff;
			}
			else if (diff > second_best_diff || (diff == second_best_diff && changed[var] < changed[second_best])){
				/* found new second best */
				second_best = var;
				second_best_diff = diff;
			}
		}
		if (best != youngest) flipvar = best;
		else if ((random()%denominator < numerator)) flipvar = second_best;
		else flipvar = best;
	}
	varstoflip[0] = flipvar;

	y = 1;
   //atom[flipvar] = 1-atom[flipvar];  //temporarily flip variable
   //int y = findTrue(functions[tofix]);
   //atom[flipvar] = 1-atom[flipvar];  //temporarily flip variable
	varstoflip[y] = 0;
#endif
	
	return y;
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

void flipatoms_true_paths() {
	numlook++;
	//Temporarily set all variables in the global solution
	int toflip;
	int flipiter = 0;
	while(varstoflip[flipiter]!=0) {
		toflip = varstoflip[flipiter];
		flipiter++;
		atom[toflip] = 1-atom[toflip];
		changed[toflip] = numflip;
	}
	
	flipiter = 0;
	while(varstoflip[flipiter]!=0) {
		toflip = varstoflip[flipiter];
		flipiter++;
		int numocc = occurence[toflip].length;
		int *occptr = occurence[toflip].num;
		
		for(int i = 0; i < numocc ;i++) {
			int cli = *(occptr++);
			//cli = occurence[toflip].num[i];
			if(wvisited[cli] == numlook) continue;
			wvisited[cli] = numlook;
			//Removing the influence every variable in this BDD has on this BDD.
			if(traverseBDD(functions[cli])) {
				//BDD is SAT
				if(previousState[cli].num[0] < 0) {
					previousState[cli].num[0] = functions[cli]->variable;
					previousState[cli].num[1] = 0;
					numfalse--;
					//fprintf(stderr, "removing fB=%d, moving fB=%d to spot %d\n", falseBDDs[wherefalse[cli]], falseBDDs[numfalse], wherefalse[cli]);
					falseBDDs[wherefalse[cli]] = falseBDDs[numfalse];
					wherefalse[falseBDDs[numfalse]] = wherefalse[cli];
				}
			} else {
				if(previousState[cli].num[0] > 0) {
					previousState[cli].num[0] = -functions[cli]->variable;
					previousState[cli].num[1] = 0;
					//fprintf(stderr, "adding fB=%d to spot %d\n", cli, numfalse);
					falseBDDs[numfalse] = cli;
					wherefalse[cli] = numfalse;
					numfalse++;
				}
			}
		}
	}
}

void flipatoms()
{
	int i, j, k;			/* loop counter */
	//int toenforce;		/* literal to enforce */
	register int cli;
	int numocc;
	int * occptr;
	int toflip;
	int flipiter = 0;
	
	//	fprintf(stderr, "flipping %i\n", toflip);
	//	fprintf(stderr, "bc=%d, mc=%d, diff=%d\n", breakcount[toflip], makecount[toflip], (makecount[toflip] - breakcount[toflip]));
	while(varstoflip[flipiter]!=0) {
		toflip = varstoflip[flipiter];
		if (toflip == NOVALUE){
			numnullflip++;
			return;
		}
		flipiter++;

		changed[toflip] = numflip;
		//if(atom[toflip] > 0)
		//  toenforce = -toflip;
		//else
		//  toenforce = toflip;
		atom[toflip] = 1-atom[toflip];  //flipped variable
		
		if (hamming_flag){ //not turned on
			if (atom[toflip] == hamming_target[toflip])
			  hamming_distance--;
			else
			  hamming_distance++;
			if ((numflip % hamming_sample_freq) == 0)
			  fprintf(hamming_fp, "%ld %i\n", numflip, hamming_distance);
		}
		
		numocc = occurence[toflip].length;
		occptr = occurence[toflip].num;
		
		for(i = 0; i < numocc ;i++) {
			//cli = occurence[toflip].num[i];
			cli = *(occptr++);
			
			//Removing the influence every variable in this BDD has on this BDD.
#ifndef USE_PATHS_TO_TRUE
# ifdef USE_FALSE_PATHS
			for(j = 0; previousState[cli].num[j]!=0; j++) {
				if(previousState[cli].num[j] > 0) {
					breakcount[previousState[cli].num[j]]-=previousState[cli].count[j];
				} else {
					makecount[-(previousState[cli].num[j])]-=previousState[cli].count[j];
				}
			}
# endif
# ifdef USE_BRITTLE
			for(k = 0; previousBrittle[cli].num[k]!=0; k++) {
				brittlecount[previousBrittle[cli].num[k]]-=previousBrittle[cli].count[k];				  
			}
# endif
#endif
			if(traverseBDD(functions[cli])) {
				//BDD is SAT
				if(previousState[cli].num[0] < 0) {
#ifdef USE_PATHS_TO_TRUE
					previousState[cli].num[0] = functions[cli]->variable;
					previousState[cli].num[1] = 0;
#endif
					numfalse--;
					//fprintf(stderr, "removing fB=%d, moving fB=%d to spot %d\n", falseBDDs[wherefalse[cli]], falseBDDs[numfalse], wherefalse[cli]);
					falseBDDs[wherefalse[cli]] = falseBDDs[numfalse];
					wherefalse[falseBDDs[numfalse]] = wherefalse[cli];
				}
			} else {
				if(previousState[cli].num[0] > 0) 
				  {
#ifdef USE_PATHS_TO_TRUE
					  previousState[cli].num[0] = -functions[cli]->variable;
					  previousState[cli].num[1] = 0;
#endif
					  //fprintf(stderr, "adding fB=%d to spot %d\n", cli, numfalse);
					  falseBDDs[numfalse] = cli;
					  wherefalse[cli] = numfalse;
					  numfalse++;
				  }
			}
#ifndef USE_PATHS_TO_TRUE
			updateBDDCounts(cli);
#endif
		}
		//if(countunsat()!=numfalse) assert(0);
		//	verifymbcount();
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
	
	if (hamming_flag){
		fclose(hamming_fp);
		printf("Final distance to hamming target = %i\n", calc_hamming_dist(atom, hamming_target));
		printf("Hamming distance data stored in %s\n", hamming_data_file);
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

int countunsatfalses(void) 
{
	int i, unsat;
	
	unsat = 0;
	for (i=0;i < numBDDs;i++) {
		int count = CountFalseBDD(functions[i]);
		if(count > 0) {
			//printBDDerr(functions[i]);
			//fprintf(stderr, "\n");
			unsat+=count;
		}
	}
	return unsat;
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
