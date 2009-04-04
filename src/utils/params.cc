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
#include "params.h"

extern int params_current_src;

extern int print_independent_vars;

void ctrl_c_proc(int x);

char sHeuristic[10]="j";
char sRestartHeuristic[10]="0";
char cnfformat[256]="noqm";
int  n_cnfformat=CNF_NOQM;
char mp_heuristic[10]="SP";
int  n_mp_heuristic=heuristic_SP;
char dependence='c';
char sResult[4]="n";
int  ctrl_c=0;
int  nCtrlC=0; /* ctrl c pressed */
LONG64 ite_counters[MAX_COUNTER];
double ite_counters_f[MAX_COUNTER_F];
int  autarky=0; /* autarkies enabled */
char debug_dev[256]="stderr";
int USE_AUTARKY_LEMMAS;
int slide_lemmas;
int print_search_dot;

void DO_ALL(int);
void DO_ALL_default(int value);
void fn_parse_filename(char *filename);

t_opt options[] = { 
// p_target, l_opt, w_opt, p_type, p_value, p_defa, var_type, p_src=0, desc_opt

/* 
 * General SBSAT options 
 */
{ (void*)show_help, "h",   "help", P_FN, V(i:0,"0"), V(i:0,"0"), VAR_CMD, 0,
              "Show all program options"},
{ (void*)show_version, "", "version", P_FN, V(i:0,"0"), V(i:0,"0"), VAR_CMD, 0,
              "Show program version"},
{ (void*)show_ini, "", "create-ini", P_FN, V(i:0,"0"), V(i:0,"0"), VAR_CMD, 0,
              "Create ini file"},
{ &ini_filename, "", "ini", P_STRING, 
	       V(i:255,"255"), {"~/sbsat.ini"}, VAR_CMD+VAR_DUMP, 0,
              "Set the ini file"},
{ &DEBUG_LVL, "",   "debug", P_INT, V(i:0,"0"), V(i:2,"2"), VAR_NORMAL, 0,
              "debugging level (0-none, 9-max)"},
{ &debug_dev, "",   "debug-dev", P_STRING, V(i:255,"255"), {"stderr"}, VAR_NORMAL, 0,
              "debugging device"},
{ &PARAMS_DUMP, "D",   "params-dump",   P_PRE_INT,  V(i:1,"1"), V(i:0,"0"), VAR_CMD, 0,
               "dump all internal parameters before processing"},
{ inputfile, "", "input-file", P_STRING, V(i:255,"255"), {"-"}, VAR_NORMAL, 0, 
                "input filename"},
{ outputfile, "o", "output-file", P_STRING, V(i:255,"255"), {"-"}, VAR_NORMAL, 0,
                "output filename"},
{ &protect_outputfile, "pof",   "protect-output-file", P_INT, V(i:0,"0"), V(i:1,"1"), VAR_NORMAL, 0,
              "If set to 1, SBSAT will not to overwrite an existing output file"},
{ &random_seed, "seed", "random-seed", P_INT, V(i:0,"0"), V(i:0,"0"), VAR_NORMAL, 0,
              "Random seed to use (0 generates a random seed)."},
{ temp_dir, "", "temp-dir", P_STRING, 
		V(i:255,"255"), {"$TEMP"}, VAR_NORMAL, 0, 
                "directory for temporary files"},
{ sResult,   "R",   "show-result",   P_STRING,  V(i:2,"2"), {"n"}, VAR_NORMAL, 0,
               "Show result (n=no result, r=raw, f=fancy, b=binary)"},
{ &verify_solution,   "",  "verify-solution",   P_INT,  V(i:0,"0"), V(i:1,"1"), VAR_NORMAL, 0,
               "Verify solution"},
{ s_expected_result, "",  "expected-result", P_STRING, V(i:255,"255"), {""}, VAR_NORMAL, 0,
               "Report error if the result is not as specified. Options are SAT, UNSAT, TRIV_SAT, TRIV_UNSAT, SOLV_SAT and SOLV_UNSAT"},
{ comment, "",  "comment", P_STRING, V(i:1023,"1023"), {""}, VAR_NORMAL, 0,
               "Comment to appear next to the filename"},
/*
{ &input_result_filename, "", "read-result-filename", P_STRING, 
	       V(i:255,"255"), {""}, VAR_NORMAL, 0,
              "Filename with the partial assignment"},
*/
{ &ctrl_c,  "",   "ctrl-c",   P_INT,  V(i:1,"1"), V(i:0,"0"), VAR_NORMAL, 0,
               "Enable/Disable Ctrl-c handler to end preproc/brancher"},
{ &reports,  "",   "reports",   P_INT,  V(i:1,"1"), V(i:0,"0"), VAR_NORMAL, 0,
               "Reporting style during branching (0 - standard, 1 - crtwin)"},
{ &competition_enable,  "",   "competition",   P_INT,  V(i:1,"1"), V(i:0,"0"), VAR_NORMAL, 0,
               "Competition reporting style"},
{ &sat_timeout,  "",   "sattimeout",   P_INT,  V(i:1,"1"), V(i:0,"0"), VAR_NORMAL, 0,
               "For SAT Competition SATTIMEOUT"},
{ &sat_ram,  "",   "satram",   P_INT,  V(i:1,"1"), V(i:0,"0"), VAR_NORMAL, 0,
               "For SAT Competition SATRAM"},
{ (void*)fn_parse_filename, "",  "parse-filename", P_FN_STRING, V(i:0,"0"), V(i:0,"0"), VAR_CMDLINE+VAR_DUMP, 0, 
	       "For testing purposes"},

/* 
 * BDD options
 */
{ NULL, "", "", P_NONE, {"0"}, {"0"}, VAR_NORMAL, 0, 
	        "\nBDD options:"},
{ &numBuckets, "", "num-buckets", P_INT, V(i:0,"0"), V(i:16,"16"), VAR_NORMAL, 0, 
                "Set the number of buckets in power of 2"},
{ &sizeBuckets, "", "size-buckets", P_INT, V(i:0,"0"), V(i:5,"5"), VAR_NORMAL, 0,
                "Set the size of a bucket in power of 2"},
{ &_bdd_pool_size, "", "bdd-pool-size", P_INT, V(i:0,"0"), V(i:1000000,"1000000"), VAR_NORMAL, 0, 
                "The size of the bdd pool increment"},
{ &enable_gc, "", "gc", P_INT, V(i:1,"1"), V(i:1,"1"), VAR_NORMAL, 0, 
                "Use garbage collection"},

/* 
 * Input options
 */
{ NULL, "", "", P_NONE, {"0"}, {"0"}, VAR_NORMAL, 0, 
	        "\nInput options:"},
{ functionTypeLimits+AND, "", "limit-and-equ", P_INT, V(i:0,"0"), V(i:2,"2"), VAR_NORMAL, 0, 
                "The minimum # of literals to flag sp. function and_equ"},
{ functionTypeLimits+OR, "", "limit-or-equ", P_INT, V(i:0,"0"), V(i:2,"2"), VAR_NORMAL, 0, 
                "The minimum # of literals to flag sp. function or_equ"},
{ functionTypeLimits+PLAINOR, "", "limit-or", P_INT, V(i:0,"0"), V(i:2,"2"), VAR_NORMAL, 0,
                "The minimum # of literals to flag sp. function or"},
{ functionTypeLimits+PLAINAND, "", "limit-and", P_INT, V(i:0,"0"), V(i:2,"2"), VAR_NORMAL, 0,
                "The minimum # of literals to flag sp. function or"},
{ functionTypeLimits+PLAINXOR, "", "limit-xor", P_INT, V(i:0,"0"), V(i:2,"2"), VAR_NORMAL, 0,
                "The minimum # of literals to flag sp. function xor"},
{ functionTypeLimits+MINMAX, "", "limit-minmax", P_INT, V(i:0,"0"), V(i:3,"3"), VAR_NORMAL, 0,
                "The minimum # of literals to flag sp. function minmax"},
{ functionTypeLimits+NEG_MINMAX, "", "limit-neg-minmax", P_INT, V(i:0,"0"), V(i:3,"3"), VAR_NORMAL, 0,
                "The minimum # of literals to flag sp. function minmax"},
/*{ &AND_EQU_LIMIT, "", "limit-and-equ", P_INT, V(i:0,"0"), V(i:2,"2"), VAR_NORMAL, 0, 
                "The minimum # of literals to flag sp. function and_equ"},
{ &OR_EQU_LIMIT, "", "limit-or-equ", P_INT, V(i:0,"0"), V(i:2,"2"), VAR_NORMAL, 0, 
                "The minimum # of literals to flag sp. function or_equ"},
{ &PLAINOR_LIMIT, "", "limit-or", P_INT, V(i:0,"0"), V(i:8,"8"), VAR_NORMAL, 0, 
                "The minimum # of literals to flag sp. function plainor"},
{ &PLAINXOR_LIMIT, "", "limit-xor", P_INT, V(i:0,"0"), V(i:5,"5"), VAR_NORMAL, 0,
                "The minimum # of literals to flag sp. function plainxor"},*/
{ &BREAK_XORS, "", "break-xors", P_INT, V(i:0,"0"), V(i:1,"1"), VAR_NORMAL, 0, 
                "Break XORS into linear and non-linear functions during search"},


/* 
 * Output options
 */
{ NULL, "", "", P_NONE, {"0"}, {"0"}, VAR_NORMAL, 0, 
	        "\nOutput options:"},
{ &formatout, "b", "", P_PRE_CHAR, V(c:'b',"b"), V(c:'b',"b"), VAR_NORMAL, 0, 
                "Start SMURF solver"},
{ &formatout, "w", "", P_PRE_CHAR, V(c:'w',"w"), V(c:'b',"b"), VAR_NORMAL, 0, 
                "Start BDD WalkSAT solver"},
{ &formatout, "m", "", P_PRE_CHAR, V(c:'m',"m"), V(c:'b',"b"), VAR_NORMAL, 0, 
                "Start WVF solver"},
{ &formatout, "t", "", P_PRE_CHAR, V(c:'t',"t"), V(c:'b',"b"), VAR_NORMAL, 0,       
	             "Start a stripped down version of the SMURF solver"},
{ &formatout, "n", "", P_PRE_CHAR, V(c:'n',"n"), V(c:'b',"b"), VAR_NORMAL, 0, 
                "Don't start any brancher or conversion"},
{ &formatout, "s", "", P_PRE_CHAR, V(c:'s',"s"), V(c:'b',"b"), VAR_NORMAL, 0, 
                "Output in SMURF format"},
{ &formatout, "c", "", P_PRE_CHAR, V(c:'c',"c"), V(c:'b',"b"), VAR_NORMAL, 0, 
                "Output in CNF format"},
{ &formatout, "i", "", P_PRE_CHAR, V(c:'i',"i"), V(c:'b',"b"), VAR_NORMAL, 0,
                "Output in ITE(bdd) format"},
{ &formatout, "a", "", P_PRE_CHAR, V(c:'a',"a"), V(c:'b',"b"), VAR_NORMAL, 0, 
                "Output in AAG format"},
{ &formatout, "x", "", P_PRE_CHAR, V(c:'x',"x"), V(c:'b',"b"), VAR_NORMAL, 0, 
                "Output in XOR format"},
{ &formatout, "v", "", P_PRE_CHAR, V(c:'v',"v"), V(c:'b',"b"), VAR_NORMAL, 0,
                "Output in VHDL/FPGA format"},
{ &formatout, "p", "", P_PRE_CHAR, V(c:'p',"p"), V(c:'b',"b"), VAR_NORMAL, 0, 
                "Output in tree like format"},
{ &formatout,  "", "formatout", P_CHAR, V(i:0,"0"), V(c:'b',"b"), VAR_NORMAL, 0, 
                "Output format"},
{ cnfformat,  "", "cnf", P_STRING, V(i:255,"255"), {"noqm"}, VAR_NORMAL, 0, 
                "Format of CNF output (3sat, qm, noqm)"},
{ &print_independent_vars, "",  "print-indep", P_INT, V(i:1,"1"), V(i:0,"0"), VAR_NORMAL, 0, 
	       "Print independent variables, CNF format only (1/0)"},

	
/* b s c w   - left ldxpi  */

{ &formatin,  "", "formatin", P_CHAR, V(i:0,"0"), V(c:' '," "), VAR_NORMAL, 0, 
                "Input format"},
{ &print_tree, "", "tree", P_PRE_INT, V(i:1,"1"), V(i:0,"0"), VAR_CMD, 0, 
                "Output BDDs in tree representation (used in conjunction with -p)"},
{ &PRINT_TREE_WIDTH, "", "tree-width", P_INT, V(i:0,"0"), V(i:64,"64"), VAR_NORMAL, 0,
                "Set BDD tree printing width"},
{ &prover3_max_vars, "", "prover3-max-vars", P_INT, V(i:0,"0"), V(i:10,"10"), VAR_NORMAL, 0, 
                "Max vars per BDD when reading 3 address code (intput format 3)"},

/* 
 * Preprocessing options
 */
{ NULL, "", "", P_NONE, {"0"}, {"0"}, VAR_NORMAL, 0, 
	        "\nPreprocessing options:"},
{ preset_variables_string, "", "preset-variables", P_STRING, 
		V(i:2048,"2048"), {""}, VAR_NORMAL, 0, 
                "Variables forced during preprocessing"},
{ preproc_string, "P", "preprocess-sequence", P_STRING, 
		V(i:255,"255"), {"{ExDc}{ExSt}{ExPr}{ExSp}{Ff}"}, VAR_NORMAL, 0,
                "The preprocessing sequence"},
{ (void*)DO_ALL, "All",  "All", P_FN_INT, V(i:0,"0"), V(i:2,"2"), VAR_CMDLINE+VAR_DUMP, 0, 
	       "Enable/Disable All Preprocessing Options (1/0)"},
{ &DO_CLUSTER, "Cl",  "Cl", P_INT, V(i:0,"0"), V(i:1,"1"), VAR_NORMAL, 0, 
	       "Enable/Disable Clustering (1/0)"},
{ &DO_COFACTOR, "Co",  "Co", P_INT, V(i:0,"0"), V(i:0,"1"), VAR_NORMAL, 0, 
		"Enable/Disable Cofactoring (1/0)"},
{ &DO_PRUNING, "Pr",  "Pr",   P_INT, V(i:0,"0"), V(i:1,"1"), VAR_NORMAL, 0, 
		"Enable/Disable Pruning (1/0)"},
{ &DO_STRENGTH, "St",  "St",  P_INT, V(i:0,"0"),  V(i:1,"1"), VAR_NORMAL, 0, 
		"Enable/Disable Strengthening (1/0)"},
{ &DO_INFERENCES, "In",  "In", P_INT, V(i:0,"0"),  V(i:1,"1"), VAR_NORMAL, 0, 
		"Enable/Disable Inferences (1/0)"},
{ &DO_EQUIVALENCES, "Eq",  "Eq", P_INT, V(i:0,"0"),  V(i:1,"1"), VAR_NORMAL, 0, 
		"Enable/Disable Equivalences (1/0)"},
{ &DO_EXIST_QUANTIFY, "Ex",  "Ex", P_INT, V(i:0,"0"),  V(i:1,"1"), VAR_NORMAL, 0,
		"Enable/Disable Existential Quantification (1/0)"},
{ &DO_EXIST_QUANTIFY_AND, "Ea",  "Ea",  P_INT,  V(i:0,"0"),  V(i:1,"1"), VAR_NORMAL, 0, 
		"Enable/Disable AND-Existential Quantification (1/0)"},
{ &DO_EX_SAFE_CLUSTER, "Es",  "Es",  P_INT,  V(i:0,"0"),  V(i:1,"1"), VAR_NORMAL, 0, 
		"Enable/Disable AND-Safe Assign + Existential Quantification (1/0)"},
{ &DO_SAFE_ASSIGN, "Sa",  "Sa",  P_INT,  V(i:0,"0"),  V(i:1,"1"), VAR_CHECK+VAR_DUMP, 0, 
		"Enable/Disable Searching for Safe Assignments (1/0)"},
{ &DO_SAFE_SEARCH, "Ss",  "Ss",  P_INT,  V(i:0,"0"),  V(i:1,"1"), VAR_CHECK+VAR_DUMP, 0, 
		"Enable/Disable SafeSearch (1/0)"},
{ &DO_POSSIBLE_ANDING, "Pa",  "Pa",  P_INT,  V(i:0,"0"),  V(i:1,"1"), VAR_CHECK+VAR_DUMP, 0,
		"Enable/Disable clustering to find possible values to variables (1/0)"},
{ &DO_DEP_CLUSTER, "Dc",  "Dc",  P_INT, V(i:0,"0"),  V(i:1,"1"), VAR_NORMAL, 0,
	  "Enable/Disable Dependent Variable Clustering (1/0)"},
{ &DO_SPLIT, "Sp",  "Sp",  P_INT, V(i:0,"0"),  V(i:1,"1"), VAR_NORMAL, 0,
	  "Enable/Disable Large Function Splitting (1/0)"},
{ &DO_REWIND, "Rw",  "Rw",  P_INT, V(i:0,"0"),  V(i:1,"1"), VAR_NORMAL, 0,
	  "Enable/Disable Rewinding of BDDs back to their initial state (1/0)"},
{ &DO_CLEAR_FUNCTION_TYPE, "Cf",  "Cf",  P_INT, V(i:0,"0"),  V(i:1,"1"), VAR_NORMAL, 0,
	  "Enable/Disable Clearing the Function Type of BDDs (1/0)"},
{ &DO_FIND_FUNCTION_TYPE, "Ff",  "Ff",  P_INT, V(i:0,"0"),  V(i:1,"1"), VAR_NORMAL, 0,
	  "Enable/Disable Searching for the Function Type of BDDs (1/0)"},
{ &DO_PROVER3, "P3",  "P3",  P_INT, V(i:0,"0"),  V(i:1,"1"), VAR_NORMAL, 0,
	  "Enable/Disable Recreating a new set of prover3 BDDs (1/0)"},
{ &DO_IDENTIFY_SAME_STRUCTURE, "Is",  "Is",  P_INT, V(i:0,"0"),  V(i:1,"1"), VAR_CHECK+VAR_DUMP, 0,
	  "Enable/Disable Identifying BDDs with the same structure/function type (1/0)"},
{ &DO_EXTEND_RES, "Er",  "Er",  P_INT, V(i:0,"0"),  V(i:1,"1"), VAR_NORMAL, 0,
	  "Enable/Disable Creating Extended Resolvents for all pairs of variables (1/0)"},
{ &DO_DIAMETER, "Di",  "Di",  P_INT, V(i:0,"0"),  V(i:1,"1"), VAR_CHECK+VAR_DUMP, 0,
	  "Enable/Disable Computing the diameter of variable interaction graph (1/0)"},
{ &DO_MESSAGE_PASSING, "Mp",  "Mp",  P_INT, V(i:0,"0"),  V(i:1,"1"), VAR_NORMAL, 0,
	  "Enable/Disable Use of message passing routines like BP/SP etc. (1/0)"},
{ &max_preproc_time, "",  "max-preproc-time", P_INT, V(i:0,"0"),  V(i:0,"0"), VAR_NORMAL, 0,
		"set the time limit in seconds (0=no limit)"},
{ &do_split_max_vars, "",  "do-split-max-vars", P_INT, V(i:0,"0"),  V(i:10,"10"), VAR_NORMAL, 0,
		"Threashold above which the Sp splits BDDs"},
{ mp_heuristic,  "", "mp-heuristic", P_STRING, V(i:"8","8"), {"SP"}, VAR_NORMAL, 0,
      "Message Passing (Mp) heuristic (BP, EMBPL, EMBPG, EMBPGV2, SP, EMSPL, EMSPG, EMSPGV2)"},
{ &mp_surveys, "",  "mp-surveys",  P_INT, V(i:0,"0"),  V(i:0,"0"), VAR_NORMAL, 0,
	   "Number of surveys to take before exiting the Mp routine."},
{ &mp_vars_to_set_for_each_survey, "",  "mp-vars-to-set",  P_INT, V(i:0,"0"),  V(i:1,"1"), VAR_NORMAL, 0,
	   "Number of variables to assign after each convergent Mp survey."},
{ &mp_epsilon, "",  "mp-epsilon",  P_FLOAT, V(i:0,"0"),  V(i:1,"0.001"), VAR_NORMAL, 0,
	   "Change needed for an Mp survey to be considered non-convergent."},
{ &cluster_step_increase, "",  "cluster-step-increase", P_INT, V(i:0,"0"),  V(i:5,"5"), VAR_NORMAL, 0,
		"Step size used to increment the threshold under which BDDs are clustered"},
{ &ex_infer, "",  "ex-infer", P_INT, V(i:0,"0"),  V(i:1,"1"), VAR_NORMAL, 0,
		"Enable/Disable Ex Quantification trying to safely assign variables before they are quantified away (1/0)"},
{ &ge_preproc, "gauss", "gaussian-elimination", P_INT, V(i:0, "0"),  V(i:'0', "0"), VAR_NORMAL, 0,
	   "Enable/Disable Gaussian Elimination in the preprocessor (1/0)"},
/*
 * General Solver options
 */
{ NULL, "", "", P_NONE, {"0"}, {"0"}, VAR_NORMAL, 0, 
	   "\nGeneral solver options:"},
{ &dependence,  "", "dependence", P_CHAR, V(i:0,"0"), V(c:'c',"c"), VAR_NORMAL, 0,
	   "Modify Independent/Dependent Variables (n=no change, r=reverse, c=clear)"},
{ &max_solutions, "", "max-solutions", P_LONG, V(i:0,"0"), V(i:1,"1"), VAR_NORMAL, 0,
		"Set the maximum number of solutions to search for. 0 will cause the solver to find all solutions."},
{ &autarky, "", "autarky", P_INT, V(i:0,"0"), V(i:0,"0"), VAR_CHECK+VAR_DUMP, 0,
		"Enable/Disable autarkies (1/0)"},
{ &nTimeLimit, "", "max-solver-time", P_INT,
		V(i:0,"0"), V(i:0,"0"), VAR_NORMAL, 0,
		"set the time limit in seconds (0=no limit)"},

/* 
 * SMURF Solver options
 */
{ NULL, "", "", P_NONE, {"0"}, {"0"}, VAR_NORMAL, 0, 
	        "\nSMURF Solver options:"},
{ lemma_out_file, "", "lemma-out-file", P_STRING, 
		V(i:255,"255"), {""}, VAR_NORMAL, 0, 
                "File to dump lemmas to"},
{ lemma_in_file, "", "lemma-in-file", P_STRING, 
		V(i:255,"255"), {""}, VAR_NORMAL, 0, 
                "File to read lemmas from"},
{ csv_trace_file, "", "csv-trace-file", P_STRING, 
		V(i:255,"255"), {""}, VAR_NORMAL, 0, 
                "File to save execution trace in CSV format"},
{ var_stat_file, "", "var-stat-file", P_STRING, 
		V(i:255,"255"), {""}, VAR_NORMAL, 0, 
                "File to save var stats"},
{ csv_depth_breadth_file, "", "csv-depth-breadth-file", P_STRING, 
		V(i:255,"255"), {""}, VAR_NORMAL, 0, 
                "Save depth/breadth statistic"},

{ &backjumping, "", "backjumping", P_INT, V(i:0,"0"), V(i:1,"1"), VAR_NORMAL, 0,
		"Enable/Disable backjumping (1/0)"},
{ &nForceBackjumpLevel, "", "force-backjump-level", P_INT, V(i:0,"0"), V(i:-1, "-1"), VAR_NORMAL, 0,
	   "Set the search tree level (given by initial_branch) to backjump to once a solution is found"},
{ solver_presets, "", "solver-presets", P_STRING,	V(i:4095,"4095"), {""}, VAR_NORMAL, 0, 
	   "Variables that are preset before the solver is called. Options are ([[=|!|#|+var|-var] ]*)"},
{ solver_polarity_presets, "", "solver-polarity-presets", P_STRING,	V(i:4095,"4095"), {""}, VAR_NORMAL, 0, 
	   "Force the polarity of the top K choice points. Options are ([+|-]*)"},
{ &solver_reset_level, "", "solver-reset-level", P_INT, V(i:0,"0"),  V(i:0,"0"), VAR_NORMAL, 0,
			      "Force the solver to explore only the subtree under the first 'solver-reset-level' choice points"},
{ &precompute_smurfs, "", "precompute-smurfs", P_INT, V(i:0,"0"),  V(i:1,"1"), VAR_NORMAL, 0,
			      "Smurf states can either be precomputed before search (default), or built during search (may use less memory, but some heuristics are weaker)"},
{ &smurfs_share_states, "", "smurfs-share-states", P_INT, V(i:0,"0"),  V(i:1,"1"), VAR_NORMAL, 0,
			      "Share Smurf states between Smurfs. May reduce memory usage if enabled (1/0)"},
{ &use_lemmas, "", "lemmas", P_INT, V(i:0,"0"),  V(i:0,"0"), VAR_NORMAL, 0,
			      "Turn lemmas on or off (1/0)"},
{ &MAX_NUM_CACHED_LEMMAS, "L", "max-cached-lemmas", P_INT,
		V(i:0,"0"), V(i:5000, "5000"), VAR_NORMAL, 0,
                "set the maximum # of lemmas"}, 
{ &print_search_dot, "", "print-search-tree", P_INT, V(i:0,"0"),  V(i:0,"0"), VAR_NORMAL, 0,
			      "Enable/Disable printing the search tree in .dot format (1/0)"},
{ &slide_lemmas, "", "slide-lemmas", P_INT, V(i:0,"0"),  V(i:0,"0"), VAR_CHECK+VAR_DUMP, 0,
			      "Enable/Disable attempting to slide lemmas during search (1/0)"},
{ &use_XORGElim, "", "gelim-smurfs", P_INT, V(i:0, "0"),  V(i:'0', "0"), VAR_NORMAL, 0,
	   "Use Gaussian Elimination during search (1/0)"},
{ &use_SmurfWatchedLists, "", "watched-smurfs", P_INT, V(i:0, "0"),  V(i:'0', "0"), VAR_NORMAL, 0,
	   "Allow dynamic Smurf association lists during search (1/0)"},
{ sRestartHeuristic, "", "rapid-restarts", P_STRING, V(i:8,"8"), {"0"}, VAR_NORMAL, 0,
		"Choose heuristic 0=None, m=MiniSAT, p=PicoSAT, l=Luby"},
{ &numSmurfWatchedVars, "", "num-watched-variables", P_INT, V(i:0, "0"),  V(i:'2', "2"), VAR_NORMAL, 0,
	   "Number of variables per Smurf to watch if watched-smurfs is enabled"},
{ &USE_AUTARKY_SMURFS, "", "autarky-smurfs", P_INT, V(i:0,"0"), V(i:0,"0"), VAR_CHECK+VAR_DUMP, 0,
		"Use Autarky Smurfs in the solver (1/0)"},
{ &USE_AUTARKY_LEMMAS, "", "autarky-lemmas", P_INT, V(i:0,"0"), V(i:0,"0"), VAR_CHECK+VAR_DUMP, 0,
		"Use Autarky Lemmas in the solver (Currently Unavailiable)"},
{ &K_TOP_VARIABLES, "", "K-top-variables", P_INT, V(i:0,"0"), V(i:0,"0"), VAR_NORMAL, 0,
		"Try to set top K variables and collect common inferences"},
	  { &sbj, "", "sbj", P_INT, V(i:0,"0"), V(i:0,"0"), VAR_CHECK+VAR_DUMP, 0,
		"Super backjumping"},
/*
{ NULL, "", "lemma-space-size", P_LONG, 
		V(i:0,"0"), V(l:LEMMA_SPACE_SIZE,"0"), VAR_DUMP, 0,
                "size of the increments to lemma space"}, 
*/
{ &MAX_VBLES_PER_SMURF, "S", "max-vbles-per-smurf", P_INT, 
		V(i:0,"0"), V(i:8,"8"), VAR_NORMAL, 0,
		"set the maximum number variables per smurf"},
{ &BACKTRACKS_PER_STAT_REPORT, "", "backtracks-per-report", P_INT, 
		V(i:0,"0"), V(i:10000,"10000"), VAR_NORMAL, 0,
		"set the number of backtracks per report"},
{ &nNumChoicePointLimit, "", "max-brancher-cp", P_INT, 
		V(i:0,"0"), V(i:0,"0"), VAR_NORMAL, 0,
		"set the choice point limit (0=no limit)"},
{ &TRACE_START, "", "brancher-trace-start", P_INT, 
		V(i:0,"0"), V(i:0,"0"), VAR_NORMAL, 0,
		"number of backtracks to start the trace (when debug=9)"},
{ sHeuristic, "H", "heuristic", P_STRING, V(i:8,"8"), {"j"}, VAR_NORMAL, 0,
		"Choose heuristic j=LSGB, l=lemma baswed, v=VSIDS, b=BerkMin, m=MiniSAT"},/* a=Anne"},*/
{ &JHEURISTIC_K, "K", "jheuristic-k", P_FLOAT, 
		V(i:0,"0"), V(f:3.0, "3.0"), VAR_NORMAL, 0,
		"set the value of K"},
{ &JHEURISTIC_K_TRUE, "", "jheuristic-k-true", P_FLOAT, 
		V(i:0,"0"), V(f:0.0, "0.0"), VAR_NORMAL, 0,
		"set the value of True state"},
{ &JHEURISTIC_K_INF, "", "jheuristic-k-inf", P_FLOAT, 
		V(i:0,"0"), V(f:1.0, "1.0"), VAR_NORMAL, 0,
		"set the value of the inference multiplier"},
{ &JHEURISTIC_K_UNKNOWN, "", "jheuristic-k-unk", P_FLOAT,
		V(i:0,"0"), V(f:0.0, "0.0"), VAR_NORMAL, 0,
		"set the value of unknown states (used with --precompute-states 0)"},

/* 
 * BDDWalkSAT Solver options
 */
{ NULL, "", "", P_NONE, {"0"}, {"0"}, VAR_NORMAL, 0, 
	        "\nBDD WalkSAT solver options:"},
{ &BDDWalkCutoff, "", "cutoff", P_INT, V(i:0,"0"), V(i:100000,"100000"), VAR_NORMAL, 0,
		"BDD WalkSAT number of flips per random restart"},
{ &BDDWalkRandomOption, "", "random-option", P_INT, V(i:0,"0"), V(i:1,"1"), VAR_NORMAL, 0,
		"BDD WalkSAT option for random walk (1=Pick a random path to true in current BDD, 2=Randomly flip every variable in current BDD, 3=Randomly flip one variable, 4=Randomly flip one variable in current BDD)"},
{ &BDDWalkHeur,  "", "bddwalk-heur", P_CHAR, V(i:0,"0"), V(c:'a',"a"), VAR_NORMAL, 0,
	   "BDD WalkSAT Heuristic (a=adaptive novelty+, n=novelty+, r=random)"},
{ &BDDWalktaboo_max, "", "tabu-max", P_INT, V(i:0,"0"), V(i:6,"6"), VAR_NORMAL, 0,
		"BDD WalkSAT length of tabu list (used in conjunction with novelty+ heuristic)"},
{ &BDDWalktaboo_multi, "", "tabu-multi", P_FLOAT, V(i:0,"0"), V(f:1.5,"1.5"), VAR_NORMAL, 0,
		"BDD WalkSAT multiplier for the probablity of picking variables with tabu (used in conjunction with novelty+ heuristic)"},
{ &BDDWalk_wp_prob, "", "bddwalk-wp-prob", P_FLOAT, V(i:0,"0"), V(f:0.1,"0.1"), VAR_NORMAL, 0,
		"BDD WalkSAT probablity of making a random walk (used in conjunction with novelty+ heuristic)"},
{ &BDDWalk_prob, "", "bddwalk-prob", P_FLOAT, V(i:0,"0"), V(f:0.1,"0.1"), VAR_NORMAL, 0,
		"BDD WalkSAT probablity of picking second best path (used in conjunction with novelty+ heuristic)"},

	
/* 
 * the end
 */
{ NULL, "", "", P_NONE, {"0"}, {"0"}, 0, 0, ""}
};

/*
"MAX_NUM_PRIME_IMPLICATS: %ld\n",(long)MAX_NUM_PRIME_IMPLICANTS);
"MAX_NUM_PATH_EQUIV_SETS: %ld\n",(long)MAX_NUM_PATH_EQUIV_SETS);
"MAX_EXQUANTIFY_CLAUSES: %d\n", MAX_EXQUANTIFY_CLAUSES);
"COF_MAX: %d\n",                COF_MAX);
"MAX_EXQUANTIFY_VARLENGTH: %d\n", MAX_EXQUANTIFY_VARLENGTH);
*/
// "HEURISTIC: %d\n",               nHeuristic);
// "STATE_TIME: %f\n",        STATE_TIME);

// 
// l_opt,  w_opt, function
// { "c",  "create_ini_file",  create_ini_file }
// { "h",  "help",             show_help }
//

void
init_params()
{
   for(int i=0;i<MAX_FUNC;i++)
      functionTypeLimits[i] = 1000000000; /* huge number max_int? */
	functionTypeLimits[XDD] = 0; //Don't want to mess with XDDs
}

/* To be used with "SATRAM" or "SATTIMEOUT" */
/* Return the limit or:
 *    -1 if the variable is not set
 *       0 if there was a problem in atoi (variable isn't a number) */
int getSATlimit(const char * name) {
    char * value;
     
     value = getenv(name);
      if (value == NULL) return(-1);
       return atoi(value);
}

void
finish_params()
{
   if (competition_enable) {
      if (DEBUG_LVL <= 2) DEBUG_LVL = 0;
      if (sResult[0] == 'n') sResult[0] = 'c';
      if (sat_timeout == 0) sat_timeout = getSATlimit("SATTIMEOUT");
      if (sat_ram == 0) sat_ram = getSATlimit("SATRAM");

      if (sat_timeout > 0) max_preproc_time = sat_timeout / 2;
   }

//	if(functionTypeLimits[MINMAX]<3) functionTypeLimits[MINMAX] = 3;
//	if(functionTypeLimits[NEG_MINMAX]<3) functionTypeLimits[NEG_MINMAX] = 3;
	
	if(dependence == 'r') reverse_independant_dependant = 1;
	if(dependence == 'c') clear_dependance = 1;
	
   if (MAX_NUM_CACHED_LEMMAS <= 0) {
		NO_LEMMAS=1;
		MAX_NUM_CACHED_LEMMAS = 0;
      backjumping = 0;
   }

   /* special files */
   if (!strcmp(debug_dev, "stdout")) stddbg = stdout;
   else
   if (!strcmp(debug_dev, "stderr")) stddbg = stderr;
   else {
   }


   /* temp-dir needs to have the ~ expanded to $HOME */
   /* and $TEMP expanded to  $TEMP or $TMPDIR or /tmp whichever comes first */
   t_opt *p_opt;
   char a_opt[][255] = { "temp-dir", /*"-any-other-par-",*/ "" };


   for (int i=0; a_opt[i][0]!=0; i++)
   { 
      p_opt = lookup_keyword(a_opt[i]);
      if (!p_opt) continue;
      if (((char*)(p_opt->p_target))[0] == '~') {
         char *env = getenv("HOME");
         if (env) 
         {
            char temp_str[256];
            sprintf(temp_str, "%s%s", 
                  env, ((char*)(p_opt->p_target))+1);
            ite_strncpy((char*)(p_opt->p_target), temp_str,
                  p_opt->p_value.i-1);
            ((char*)(p_opt->p_target))[p_opt->p_value.i]=0;
         }
      }
      if (!strncmp((char*)(p_opt->p_target), "$TEMP", 5)) {
         char *env = getenv("TEMP");
         char tmp_dir[] = "/tmp";
         if (!env) env = getenv("TMPDIR");
         if (!env) {
            struct stat buf;
            if (stat(tmp_dir, &buf) == 0) env = tmp_dir;
         }
         if (env)
         {
            char temp_str[256];
            sprintf(temp_str, "%s%s", 
                  env, ((char*)(p_opt->p_target))+5);
            ite_strncpy((char*)(p_opt->p_target), temp_str,
                  p_opt->p_value.i-1);
            ((char*)(p_opt->p_target))[p_opt->p_value.i]=0;
         } else {
            d1_printf1 ("TEMP or TMPDIR variable not found.\n");
         }
      }
   }

   /* decode result type -- none, raw, fancy */
   switch (sResult[0]) {
    case 'i': result_display_type = -1; break;
    case 'n': result_display_type = 0; break;
    case 'r': result_display_type = 1; break;
    case 'f': result_display_type = 2; break;
    case 'm': result_display_type = 3; break;
    case 'c': result_display_type = 4; break;
	 case 'b': result_display_type = 5; break;
    default: 
              dE_printf2("error: Unknown result type '%c'\n", sResult[0]); 
              exit(1);
              break;
   }

	solver_polarity_presets_length = strlen(solver_polarity_presets);
	for(int x = 0; x < solver_polarity_presets_length; x++) {
		if(!(solver_polarity_presets[x] == '+' || solver_polarity_presets[x] == '-')) {
			dE_printf3("error: --solver-polarity-presets is malformed at character %d (%c)\n", x, solver_polarity_presets[x]);
			exit(1);
		}
	}

   /* limit max vbles per smurf flag -S */
   if (MAX_VBLES_PER_SMURF > MAX_MAX_VBLES_PER_SMURF)
   {
      dE_printf2("error: limit for MAX_VBLES_PER_SMURF -S is %d\n",
            MAX_MAX_VBLES_PER_SMURF);
      exit(1);
   }

	if(USE_AUTARKY_LEMMAS == 1) NO_AU_LEMMAS = 0;
	else if(USE_AUTARKY_LEMMAS == 0) NO_AU_LEMMAS = 1;
	
   /* check validity of the cnf output format */
   if (!strcmp(cnfformat, "noqm")) n_cnfformat = CNF_NOQM; else
      if (!strcmp(cnfformat, "qm"))   n_cnfformat = CNF_QM; else
         if (!strcmp(cnfformat, "3sat")) n_cnfformat = CNF_3SAT; else
         {
            dE_printf2("error: Unrecognized CNF Format: %s\n", cnfformat); 
            exit(1);
         }

	/* check validity of the mp-heuristic option */
	if (!strcmp(mp_heuristic, "BP")) n_mp_heuristic = heuristic_BP; else
	   if (!strcmp(mp_heuristic, "EMBPL"))   n_mp_heuristic = heuristic_EMBPL; else
	      if (!strcmp(mp_heuristic, "EMBPG")) n_mp_heuristic = heuristic_EMBPG; else
	  	      if (!strcmp(mp_heuristic, "EMBPGV2")) n_mp_heuristic = heuristic_EMBPGV2; else
	  	  	      if (!strcmp(mp_heuristic, "SP")) n_mp_heuristic = heuristic_SP; else
	  	  	  	      if (!strcmp(mp_heuristic, "EMSPL")) n_mp_heuristic = heuristic_EMSPL; else
	  	  	  	  	      if (!strcmp(mp_heuristic, "EMSPG")) n_mp_heuristic = heuristic_EMSPG; else
	  	  	  	  	  	      if (!strcmp(mp_heuristic, "EMSPGV2")) n_mp_heuristic = heuristic_EMSPGV2; else
	      {
				dE_printf2("error: Unrecognized Mp heuristic: %s\n", mp_heuristic); 
				exit(1);
			}
	
   /* install ctrl-c function if ctrl_c enabled */
   if (ctrl_c == 1) {
      signal (SIGINT, ctrl_c_proc);
   }
}

//#define SLIDER_DEFAULTS

int
init_slider_options()
{
   char tmp_str[5];
   change_defa_param_int(strcpy(tmp_str, "max-cached-lemmas"), 0);
   change_defa_param_int(strcpy(tmp_str, "backjumping"), 0);
   DO_ALL_default(0);
   return 0;
}

int
params_parse_cmd_line(int argc, char *argv[])
{
   init_params();
   init_options();
#ifdef SLIDER_DEFAULTS
   init_slider_options();
#endif
   read_cmd(argc, argv);
   fix_ini_filename();
   read_ini(ini_filename);
   finish_params();

   if (PARAMS_DUMP) dump_params();
   return 0;
}

void 
ctrl_c_proc(int x)
{
   fprintf(stderr, "Please hold on for phase interruption\n");
   nCtrlC++;
   if (nCtrlC > 10) {
      fprintf(stderr, "Shutting down...\n");
      free_terminal_in();
      free_terminal_out();
      exit(1);
   }
}

void
DO_ALL(int value)
{
   char tmp_str[5];
   set_param_int(strcpy(tmp_str, "Cl"), value);
   set_param_int(strcpy(tmp_str, "Co"), value);
   set_param_int(strcpy(tmp_str, "Pr"), value);
   set_param_int(strcpy(tmp_str, "St"), value);
	set_param_int(strcpy(tmp_str, "Sa"), value);
	set_param_int(strcpy(tmp_str, "Ss"), value);
   set_param_int(strcpy(tmp_str, "Ex"), value);
   set_param_int(strcpy(tmp_str, "Ea"), value);
	set_param_int(strcpy(tmp_str, "Es"), value);
	set_param_int(strcpy(tmp_str, "Pa"), value);
   set_param_int(strcpy(tmp_str, "Dc"), value);
	set_param_int(strcpy(tmp_str, "Sp"), value);
	set_param_int(strcpy(tmp_str, "Rw"), value);
	set_param_int(strcpy(tmp_str, "Cf"), value);
	set_param_int(strcpy(tmp_str, "Ff"), value);
	set_param_int(strcpy(tmp_str, "P3"), value);
	set_param_int(strcpy(tmp_str, "Is"), value);
	set_param_int(strcpy(tmp_str, "Er"), value);
	set_param_int(strcpy(tmp_str, "Di"), value);
	set_param_int(strcpy(tmp_str, "Mp"), value);
   // also add a line to the following function
}

void
DO_ALL_default(int value)
{
   char tmp_str[5];
   change_defa_param_int(strcpy(tmp_str, "Cl"), value);
   change_defa_param_int(strcpy(tmp_str, "Co"), value);
   change_defa_param_int(strcpy(tmp_str, "Pr"), value);
   change_defa_param_int(strcpy(tmp_str, "St"), value);
   change_defa_param_int(strcpy(tmp_str, "Sa"), value);
	change_defa_param_int(strcpy(tmp_str, "Ss"), value);	
   change_defa_param_int(strcpy(tmp_str, "Ex"), value);
   change_defa_param_int(strcpy(tmp_str, "Ea"), value);
	change_defa_param_int(strcpy(tmp_str, "Es"), value);
   change_defa_param_int(strcpy(tmp_str, "Pa"), value);
   change_defa_param_int(strcpy(tmp_str, "Dc"), value);
   change_defa_param_int(strcpy(tmp_str, "Sp"), value);
   change_defa_param_int(strcpy(tmp_str, "Rw"), value);
   change_defa_param_int(strcpy(tmp_str, "Cf"), value);
   change_defa_param_int(strcpy(tmp_str, "Ff"), value);
   change_defa_param_int(strcpy(tmp_str, "P3"), value);
	change_defa_param_int(strcpy(tmp_str, "Is"), value);
	change_defa_param_int(strcpy(tmp_str, "Er"), value);
	change_defa_param_int(strcpy(tmp_str, "Di"), value);
	change_defa_param_int(strcpy(tmp_str, "Mp"), value);
}

void
fn_parse_filename(char *filename)
{
   // filename in the format
   // _ are separators (replaced with 0)
   // name _ (un)sat _ arguments . ext
   //
   int argc=4;
   char *filename_copy = strdup(filename);
   char *ptr=filename_copy;
   if (ptr == NULL) { perror("strdup"); exit(1); }
   while(*ptr != 0) {
      if (*ptr == '_') argc++;
      ptr++;
   }
   char **argv = (char**)ite_calloc(argc+1, sizeof(char*), 9, "argv");
   argc=0;
   argv[argc++] = NULL; // skipped
   argv[argc++] = filename;
   // strip the extension
   if ((ptr = strrchr(filename_copy, '.'))!=NULL) *ptr = 0;
   ptr = filename_copy;
   while(*ptr != 0 && *ptr!='_') ptr++;
   if (*ptr != 0) {
      ptr++;
      // set the expected result
      char expected_result[] = "--expected-result";
      argv[argc++] = expected_result;
      argv[argc++] = ptr;
      while(1) {
         while(*ptr != 0 && *ptr!='_') ptr++;
         if (*ptr == 0) break;
         *ptr = 0;
         ptr++;
         if (*ptr == 0) break;
         argv[argc++] = ptr;
         ptr++;
      }
   }
   read_cmd(argc, argv);
   free(filename_copy);
   free(argv);
}
