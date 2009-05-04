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

#ifndef SBSAT_VARS_H
#define SBSAT_VARS_H

#define LONG64 long long

/* counter and return values */
#define NO_ERROR 0

//ERR_BT must follow INF for each function type
#define INF_SMURF              1
#define ERR_BT_SMURF           2

#define INF_SMURF_AU           3
#define ERR_BT_SMURF_AU        4

#define INF_SPEC_FN_AND        5 
#define ERR_BT_SPEC_FN_AND     6

#define INF_SPEC_FN_XOR        7
#define ERR_BT_SPEC_FN_XOR     8

#define INF_LEMMA              9
#define ERR_BT_LEMMA          10

#define ERR_BT_SOL_LEMMA      11

#define ERR_LIMITS            12

#define INF_SPEC_FN_MINMAX    13
#define ERR_BT_SPEC_FN_MINMAX 14

#define INF_SPEC_FN_NEGMINMAX    15
#define ERR_BT_SPEC_FN_NEGMINMAX 16

#define INF_SPEC_FN_OR        17
#define ERR_BT_SPEC_FN_OR     18

#define INF_BB_GELIM          19
#define ERR_BT_BB_GELIM       20

#define SMURF_STATES	    30
#define SMURF_AU_STATES  31

#define NUM_SOLUTIONS	 32

#define BDD_NODE_FIND    33
#define BDD_NODE_NEW     34
#define BDD_NODE_STEPS   35

#define SMURF_NODE_FIND    36
#define SMURF_NODE_NEW     37

#define SMURF_AU_NODE_FIND 38
#define SMURF_AU_NODE_NEW  39

#define NUM_INFERENCES	 40
#define NUM_BACKTRACKS	 41
#define NUM_BACKJUMPS	 42
#define NUM_TOTAL_BACKJUMPS	 43
#define NUM_AUTARKIES	 44
#define NUM_TOTAL_AUTARKIES	 45
#define NUM_CHOICE_POINTS     46
#define HEU_DEP_VAR      47

#define NUM_LEMMA_INTO_CACHE    48

#define MAX_COUNTER      49

#define SILENT 50

/* non counter errors */
#define ERR_IO_INIT	101
#define ERR_IO_READ 102
#define ERR_IO_UNEXPECTED_CHAR 103
#define IO_CNF_HEADER 104

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

enum {
	heuristic_BP,
	heuristic_EMBPL,
   heuristic_EMBPG,
   heuristic_EMBPGV2,
   heuristic_SP,
   heuristic_EMSPL,
   heuristic_EMSPG,
   heuristic_EMSPGV2
};

extern int random_seed;

void params_consistency_check ();
int params_parse_cmd_line(int argc, char *argv[]);
void params_dump();

extern float STATE_TIME;
extern int PRINT_TREE_WIDTH;

extern int reverse_independant_dependant;
extern int print_independent_vars;
extern int clear_dependance;
extern int backjumping;
extern int nForceBackjumpLevel;
extern char formatin;
extern char formatout;
extern int n_cnfformat;
extern int n_mp_heuristic;
extern int mp_vars_to_set_for_each_survey;
extern int mp_surveys;
extern float mp_epsilon;
extern int print_tree;

extern char inputfile[256];
extern char outputfile[256];
extern FILE * finputfile;
extern FILE * foutputfile;
extern int protect_outputfile;

extern int DO_CLUSTER; //CNF clustering
extern int DO_COFACTOR;
extern int DO_PRUNING;
extern int DO_STRENGTH;
extern int DO_SIMPLEAND;
extern int DO_SAFE_ASSIGN;
extern int DO_SAFE_SEARCH;
extern int DO_STEAL;
extern int DO_INFERENCES;
extern int DO_EQUIVALENCES;
extern int DO_EXIST_QUANTIFY;
extern int DO_EXIST_QUANTIFY_AND;
extern int DO_POSSIBLE_ANDING;
extern int DO_DEP_CLUSTER;
extern int DO_SPLIT;
extern int DO_REWIND;
extern int DO_CLEAR_FUNCTION_TYPE;
extern int DO_FIND_FUNCTION_TYPE;
extern int DO_PROVER3;
extern int DO_IDENTIFY_SAME_STRUCTURE;
extern int DO_EXTEND_RES;
extern int DO_DIAMETER;
extern int DO_MESSAGE_PASSING;

extern int PARAMS_DUMP;

extern int nHeuristic;
extern int result_display_type;
extern char module_root[256];
extern int numBuckets;
extern int sizeBuckets;

extern int *arrFunctionStructure;
extern int **arrSolverVarsInFunction;

extern double *arrVarTrueInfluences;
extern int **arrVarChoiceLevels;
extern int nVarChoiceLevelsMax;
extern int nVarChoiceLevelsNum;

extern int TRACE_START;

extern LONG64 ite_counters[MAX_COUNTER];
extern double ite_counters_f[MAX_COUNTER_F];

//extern char png_filename[256];
extern char jpg_filename[256];

//extern int AND_EQU_LIMIT;
//extern int OR_EQU_LIMIT;
//extern int PLAINXOR_LIMIT;
//extern int PLAINOR_LIMIT;

extern int verify_solution;

extern char s_expected_result[256];

extern LONG64  max_solutions;

extern char sHeuristic[10];
extern char sRestartHeuristic[10];

extern int BREAK_XORS;
extern int SMURFS_SHARE_PATHS;

extern int max_preproc_time;

extern int NO_LEMMAS;
extern int NO_AU_LEMMAS;

extern int slide_lemmas;
extern int print_search_dot;

extern int sbj;

extern int ge_preproc;
extern int use_XORGElim;
extern int use_SmurfWatchedLists;
extern int numSmurfWatchedVars;
extern int precompute_smurfs;
extern int smurfs_share_states;
extern int use_lemmas;

extern int USE_AUTARKY_SMURFS;

extern int reports;

extern char temp_dir[256];

extern char lemma_out_file[256];
extern char lemma_in_file[256];
extern char csv_trace_file[256];
extern FILE *fd_csv_trace_file;

extern char var_stat_file[256];
extern char csv_depth_breadth_file[256];

extern int _bdd_pool_size;
extern int autarky;

extern int solver_reset_level;

extern char solver_presets[4096];
extern char solver_polarity_presets[4096];
extern int solver_polarity_presets_length;
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
extern float JHEURISTIC_K_UNKNOWN;

extern char preset_variables_string[4096];
extern int prover3_max_vars;
extern int do_split_max_vars;
extern int functionTypeLimits[MAX_FUNC];
extern int enable_gc;
extern int ex_infer;
extern int competition_enable;
extern int sat_timeout;
extern int sat_ram;
extern int K_TOP_VARIABLES;

extern long nTimeLimit;
extern long nNumChoicePointLimit;
extern char temp_dir[256];
extern char ini_filename[256];
extern char input_result_filename[256];
extern char preproc_string[256];

extern int BDDWalkCutoff;
extern int BDDWalktaboo_max;
extern float BDDWalktaboo_multi;
extern char BDDWalkHeur;
extern int BDDWalkRandomOption;
extern float BDDWalk_wp_prob;
extern float BDDWalk_prob;

//Begin preprocessing globals

extern int T;
extern int F;
extern int DO_CLUSTER;  //CNF clustering
extern int DO_COFACTOR;
extern int DO_PRUNING;
extern int DO_STRENGTH;
extern int DO_SIMPLEAND;
extern int DO_SAFE_ASSIGN;
extern int DO_SAFE_SEARCH;
extern int DO_STEAL;
extern int DO_MESSAGE_PASSING;
extern int DO_DIAMETER;
extern int DO_INFERENCES;
extern int DO_EXIST_QUANTIFY;
extern int DO_EXIST_QUANTIFY_AND;
extern int DO_EX_SAFE_CLUSTER;
extern int DO_POSSIBLE_ANDING;
extern int DO_DEP_CLUSTER;
extern int DO_SPLIT;
extern int DO_REWIND;
extern int DO_CLEAR_FUNCTION_TYPE;
extern int DO_FIND_FUNCTION_TYPE;
extern int DO_PROVER3;
extern int MAX_EXQUANTIFY_CLAUSES;     //Number of clauses a variable appears in
                                     //to quantify that variable away.
extern int COF_MAX;
extern int MAX_EXQUANTIFY_VARLENGTH;   //Limits size of number of vars in 
                                     //constraints created by ExQuantify
extern int cluster_step_increase;
//End preprocessing globals

#endif
