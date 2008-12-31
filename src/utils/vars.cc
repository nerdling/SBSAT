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
#include "sbsat_solver.h"

//float STATE_TIME = 5000.0;
int PRINT_TREE_WIDTH = 64;
int reverse_independant_dependant = 0;
int clear_dependance = 0;
int print_independent_vars;
int backjumping = 1;
int nForceBackjumpLevel = -1;
int ge_preproc = 0;
int use_XORGElim = 0;
int use_SmurfWatchedLists = 0;
int numSmurfWatchedVars = 2;
int precompute_smurfs = 1;
int smurfs_share_states = 1;
int use_lemmas = 0;
char formatin = 'f', formatout = 'b'; //Solving the problem is the Default
int print_tree = 0;
char inputfile[256] = "-";
char outputfile[256] = "-";
int protect_outputfile = 1;
//char progname[65] = "ite";
FILE * finputfile = NULL;
FILE * foutputfile = NULL;
char temp_dir[256] = "";
char ini_filename[256] = "";
int  result_display_type = 0;
char module_root[256] = "./Modules";
char input_result_filename[256] = "";

int DO_CLUSTER = 1;   //CNF clustering
int DO_COFACTOR = 1;  //Cofactoring doesn't help for any benchmark i've tried
                      //Except small xor-chain files. Even then Ea works better.
int DO_PRUNING = 1;
int DO_STRENGTH = 1;
int DO_SIMPLEAND = 1;
int DO_SAFE_ASSIGN = 1;
int DO_SAFE_SEARCH = 1;
int DO_INFERENCES = 1;
int DO_EQUIVALENCES = 1;
int DO_EXIST_QUANTIFY = 1;
int DO_EXIST_QUANTIFY_AND = 1;
int DO_EX_SAFE_CLUSTER = 1;
int DO_POSSIBLE_ANDING = 1;
int DO_DEP_CLUSTER = 1;
int DO_SPLIT = 1;
int DO_REWIND = 1;
int DO_CLEAR_FUNCTION_TYPE = 1;
int DO_FIND_FUNCTION_TYPE = 1;
int DO_PROVER3 = 1;
int DO_IDENTIFY_SAME_STRUCTURE = 1;
int DO_EXTEND_RES = 1;
int DO_DIAMETER = 1;
int DO_MESSAGE_PASSING = 1;

int NO_LEMMAS = 0;
int NO_AU_LEMMAS = 0;

int PARAMS_DUMP = 0;

int MAX_NUM_CACHED_LEMMAS = _MAX_NUM_CACHED_LEMMAS;
int BACKTRACKS_PER_STAT_REPORT = _BACKTRACKS_PER_STAT_REPORT;
int nHeuristic = JOHNSON_HEURISTIC;
int MAX_VBLES_PER_SMURF=_MAX_VBLES_PER_SMURF;
long MAX_NUM_PATH_EQUIV_SETS; /* 3^MAX_VBLES_PER_SMURF */
float JHEURISTIC_K=_JHEURISTIC_K;
float JHEURISTIC_K_TRUE=0;
float JHEURISTIC_K_INF=1;
float JHEURISTIC_K_UNKNOWN=0;

int mp_vars_to_set_for_each_survey = 1;
int mp_surveys = 0;
float mp_epsilon = 0.001;

int random_seed = 0;

long nTimeLimit = 0;
long nNumChoicePointLimit = 0;

/* former CircuitStruct */
int nmbrFunctions=0;
struct BDDNodeStruct **functions=NULL;
int original_numout=0;
BDDNodeStruct **original_functions = NULL;
int *functionType=NULL;
int *equalityVble=NULL; ///< Variable on the LHS of an ite=, and=, or or= BDD.
int *independantVars=NULL;
char **labels=NULL;
FNProps *functionProps = NULL;
int *arrFunctionStructure = NULL;
int **arrSolverVarsInFunction = NULL;

int numBuckets=16;
int sizeBuckets=5;
char preproc_string[256]="";

int TRACE_START=0;
int BDDWalkCutoff=100000;
int BDDWalktaboo_max=6;
float BDDWalktaboo_multi=1.5;
char BDDWalkHeur;
int BDDWalkRandomOption;
float BDDWalk_wp_prob;
float BDDWalk_prob;

//char png_filename[256]="";
char jpg_filename[256]="";

int AND_EQU_LIMIT=0;
int OR_EQU_LIMIT=0;
int PLAINXOR_LIMIT=0;
int PLAINOR_LIMIT=0;

int cluster_step_increase=5;

int verify_solution=1;

char s_expected_result[256]="";

LONG64 max_solutions=0;

int BREAK_XORS=0;
int SMURFS_SHARE_PATHS=1;

int max_preproc_time=0;

int USE_AUTARKY_SMURFS=0;

int sbj = 0;

int reports = 0;

char lemma_out_file[256] = "";
char lemma_in_file[256] = "";
char csv_trace_file[256] = "";
FILE *fd_csv_trace_file = NULL;

char var_stat_file[256] = "";
char csv_depth_breadth_file[256]="";

int _bdd_pool_size=1000000;

varinfo *variablelist = NULL;
llistStruct *amount = NULL;
int *num_funcs_var_occurs = NULL;

int solver_reset_level = -1;

char solver_presets[4096]="";
char comment[1024]="";

char preset_variables_string[4096]="";
char solver_polarity_presets[4096]="";
int solver_polarity_presets_length=0;
int prover3_max_vars=10;
int do_split_max_vars=10;

int functionTypeLimits[MAX_FUNC];

int enable_gc = 1;
int ex_infer = 0;
int competition_enable = 0;

int sat_timeout = 0;
int sat_ram = 0;

store *variables;
int *length = NULL;

int K_TOP_VARIABLES=0;
