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
#ifndef VARS_H
#define VARS_H

#define LONG64 long long

/* counter and return values */
#define NO_ERROR 0

#define ERR_BT_SMURF 1
#define ERR_BT_SPEC_FN_AND 2
#define ERR_BT_SPEC_FN_XOR 3
#define ERR_BT_LEMMA 4
#define ERR_BT_SOL_LEMMA 5
#define ERR_LIMITS 6
#define ERR_BT_SPEC_FN_MINMAX 7

#define INF_SMURF        10
#define INF_SPEC_FN_AND  11 
#define INF_SPEC_FN_XOR  12 
#define INF_LEMMA        13
#define INF_SPEC_FN_MINMAX  14 

#define SMURF_STATES	    15
#define NUM_SOLUTIONS	 16

#define BDD_NODE_FIND    17
#define BDD_NODE_NEW     18
#define BDD_NODE_STEPS   19

#define SMURF_NODE_FIND    20
#define SMURF_NODE_NEW     21

#define NUM_BACKTRACKS	 22
#define NUM_BACKJUMPS	 23
#define NUM_TOTAL_BACKJUMPS	 24
#define NUM_AUTARKIES	 25
#define NUM_TOTAL_AUTARKIES	 26
#define NUM_CHOICE_POINTS     27
#define HEU_DEP_VAR      28

#define NUM_LEMMA_INTO_CACHE    29

#define MAX_COUNTER      30

/* non counter errors */
#define ERR_IO_INIT	101

/* ite_counters_f */
#define RUNNING_TIME      1
#define BUILD_SMURFS      2
#define PREPROC_TIME      3
#define BRANCHER_TIME     4
#define CURRENT_TIME      5
#define PROGRESS          6 
#define MAX_COUNTER_F    10

enum {
	CNF_NOQM,
	CNF_QM,
	CNF_3SAT
};

void params_consistency_check ();
int params_parse_cmd_line(int argc, char *argv[]);
void params_dump();

extern float STATE_TIME;
extern int PRINT_TREE_WIDTH;

extern int reverse_independant_dependant;
extern int clear_dependance;
extern int backjumping;
extern char formatin;
extern char formatout;
extern int n_cnfformat;
extern int print_tree;

extern char inputfile[128];
extern char outputfile[128];
extern FILE * finputfile;
extern FILE * foutputfile;

extern int DO_CLUSTER;
extern int DO_COFACTOR;

extern int DO_PRUNING;
extern int DO_STRENGTH;
extern int DO_STEAL;
extern int DO_INFERENCES;
extern int DO_EXIST_QUANTIFY;
extern int DO_EXIST_QUANTIFY_AND;
extern int DO_DEP_CLUSTER;
extern int DO_SPLIT;
extern int DO_REWIND;
extern int DO_CLEAR_FUNCTION_TYPE;
extern int DO_PROVER3;

extern int PARAMS_DUMP;

extern int nHeuristic;
extern int result_display_type;
extern char module_root[128];
extern int numBuckets;
extern int sizeBuckets;

extern int TRACE_START;

extern LONG64 ite_counters[MAX_COUNTER];
extern double ite_counters_f[MAX_COUNTER_F];

//extern char png_filename[128];
extern char jpg_filename[128];

//extern int AND_EQU_LIMIT;
//extern int OR_EQU_LIMIT;
//extern int PLAINXOR_LIMIT;
//extern int PLAINOR_LIMIT;

extern int verify_solution;

extern char s_expected_result[128];

extern int  max_solutions;

extern char sHeuristic[10];

extern int BREAK_XORS;
extern int SMURFS_SHARE_PATHS;

extern int max_preproc_time;

extern int NO_LEMMAS;

extern int sbj;

extern int reports;

extern char temp_dir[128];

extern char lemma_out_file[128];
extern char lemma_in_file[128];
extern char csv_trace_file[128];
extern FILE *fd_csv_trace_file;

extern char var_stat_file[128];

extern int _bdd_pool_size;
extern int autarky;

typedef struct {
   int pos;
   int last_count_pos;
   int neg;
   int last_count_neg;
} t_arrVarScores;

extern t_arrVarScores *arrVarScores;
extern char brancher_presets[4096];
extern char comment[1024];

#define _BACKTRACKS_PER_STAT_REPORT 10000
extern int BACKTRACKS_PER_STAT_REPORT;
#define BACKTRACKS_MAX            100000

extern int PRINT_TREE_WIDTH;
extern float STATE_TIME;

#define MAX_MAX_VBLES_PER_SMURF 17
#define _MAX_VBLES_PER_SMURF 8
extern int MAX_VBLES_PER_SMURF;

// MAX_NUM_NON_CHACHED_LEMMAS -- soft limit -- lemmas are added as needed
#define MAX_NUM_NON_CACHED_LEMMAS 1000
#define MAX_NUM_LEMMAS (MAX_NUM_CACHED_LEMMAS + MAX_NUM_NON_CACHED_LEMMAS)

#define _MAX_NUM_CACHED_LEMMAS 5000 /* default */
extern int MAX_NUM_CACHED_LEMMAS;

#define _JHEURISTIC_K 3.0 // Parameter to the Johnson Village heuristic.
extern float JHEURISTIC_K; // Parameter to the Johnson Village heuristic.
extern float JHEURISTIC_K_TRUE; 
extern float JHEURISTIC_K_INF; 

extern char preset_variables_string[2048];
extern int prover3_max_vars;
extern int do_split_max_vars;
extern int functionTypeLimits[MAX_FUNC];
extern int enable_gc;
extern int ex_infer;
extern int competition_enable;
extern int sat_timeout;
extern int sat_ram;
#endif
