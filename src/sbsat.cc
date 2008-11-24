/* =========FOR INTERNAL USE ONLY. NO DISTRIBUTION PLEASE ========== */ 

/*********************************************************************
 Copyright 1999-2007, University of Cincinnati.  All rights reserved.
 By using this software the USER indicates that he or she has read,
 understood and will comply with the following:
 * 
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
 * 
 * 
 --- This software and any associated documentation is provided "as is"
 * 
 UNIVERSITY OF CINCINNATI MAKES NO REPRESENTATIONS OR WARRANTIES, EXPRESS
 OR IMPLIED, INCLUDING THOSE OF MERCHANTABILITY OR FITNESS FOR A
 PARTICULAR PURPOSE, OR THAT  USE OF THE SOFTWARE, MODIFICATIONS, OR
 ASSOCIATED DOCUMENTATION WILL NOT INFRINGE ANY PATENTS, COPYRIGHTS,
 TRADEMARKS OR OTHER INTELLECTUAL PROPERTY RIGHTS OF A THIRD PARTY.
 * 
 University of Cincinnati shall not be liable under any circumstances for
 any direct, indirect, special, incidental, or consequential damages
 with respect to any claim by USER or any third party on account of
 or arising from the use, or inability to use, this software or its
 associated documentation, even if University of Cincinnati has been advised
 of the possibility of those damages.
 *********************************************************************/

#include "sbsat.h"
#include "sbsat_formats.h"
#include "sbsat_preproc.h"
#include "sbsat_postproc.h"
#include "sbsat_solver.h"
#include "sbsat_utils.h"

extern BDDNodeStruct **original_functions;

void bddtable_free_pools();
void bdd_circuit_free();
void wvfSolve();
void Verify_NoSolver();
void Verify_Solver();

int ite_pre_init();
void ite_main_free();
int ite_io_init();
void ite_io_free();
int ite_preprocessing();
int ite_final(int ret);

int check_expected_result(int result);
int ite_main_init(int argc, char *argv[]);
int ite_main_load();
int ite_main();
int ite_main_preproc();
void itetable_init();
void itetable_free_pools();

struct timeval tv1;
struct timezone tzp1;

int 
main(int argc, char *argv[])
{
   int ret = NO_ERROR;
   ret = ite_main_init(argc, argv);

	gettimeofday(&tv1,&tzp1);
	if(random_seed <= 0)
	  random_seed = ((tv1.tv_sec & 0177 ) * 1000000) + tv1.tv_usec;
	d3_printf2("Random seed = %d\n", random_seed);
	srandom(random_seed);	
	srand(random_seed);

   if (ret == NO_ERROR) {
      ret = ite_main_load();
      if (ret == NO_ERROR) {
         ret = ite_main_preproc();
         if (ret == NO_ERROR) ret = ite_main();
      }
   }

   return ite_final(ret);
}

int
ite_main_preproc()
{
   int ret = ite_preprocessing();
   if (ret == TRIV_SAT || ret == TRIV_UNSAT) return ret;

   return NO_ERROR;
}

int 
ite_main_init(int argc, char *argv[])
{
   int ret = NO_ERROR;
	
   ret = ite_pre_init();
   if (ret != NO_ERROR) return ret;
	
   ret = params_parse_cmd_line(argc, argv);
   if (ret != NO_ERROR) return ret;
	
   if (DEBUG_LVL == 1) d1_printf3("%s %s ", ite_basename(inputfile), comment);

   if (competition_enable == 1) {
      show_competition_version();
      printf("c %s %s\n", ite_basename(inputfile), comment);
      printf("c SATTIMEOUT = %d\n", sat_timeout);
      printf("c SATRAM = %d\n", sat_ram);
   }

   bdd_init();
   itetable_init();
   sym_init();

   return NO_ERROR;
}

int
ite_main_load()
{
   int ret = NO_ERROR;
	
   ret = ite_io_init();
   if (ret != NO_ERROR) return ret;
	
   /* read input file */

	start_prep = get_runtime (); //Start the preprocessing clock.
   ret = read_input();
   if (ret != NO_ERROR) return ret;

   return ret;
}

int
ite_main()
{
   int ret = NO_ERROR;
   dC_printf1("c Starting solver\n");

	switch (formatout) {
    case 'n': ret = SILENT; break;
    case 'b': ret = solve(); break;
    case 'w': ret = walkSolve(); break;
    case 'm': wvfSolve(); break;
	 case 't': ret = simpleSolve(); break;
    default: write_output(formatout);   
		ret = CONV_OUTPUT;
   }

   return ret;
}

int
ite_final(int ret)
{
   int competition_exit_code=0; // UNKNOWN
   char competition_output[32]="UNKNOWN";

   if (s_expected_result[0]) {
      ret = check_expected_result(ret);
   }

   if (ite_counters[BDD_NODE_FIND]==0) ite_counters[BDD_NODE_FIND] = 1;
   d4_printf4("BDD Lookup Statistic: %ld/%ld (%f hit rate)\n", 
         (long)(ite_counters[BDD_NODE_FIND] - ite_counters[BDD_NODE_NEW]), 
         (long)(ite_counters[BDD_NODE_FIND]),
         1.0 * (ite_counters[BDD_NODE_FIND] - ite_counters[BDD_NODE_NEW]) / ite_counters[BDD_NODE_FIND]);
   d4_printf4("BDD Hash Table Lookup Statistic: %ld/%ld (%f steps)\n", 
         (long)(ite_counters[BDD_NODE_STEPS]),
         (long)(ite_counters[BDD_NODE_FIND]),
         1.0 * ite_counters[BDD_NODE_STEPS] / ite_counters[BDD_NODE_FIND]);
   d4_printf4("BDD Hash Table Storage Statistic: %ld/%ld (%f nodes taken)\n", 
         (long)(ite_counters[BDD_NODE_NEW]),
         (long)((1<<(numBuckets+sizeBuckets))-1),
         1.0 * ite_counters[BDD_NODE_NEW] / (long)((1<<(numBuckets+sizeBuckets))-1));

   char result_string[64];
   switch(ret) {
    case TRIV_SAT: {
		 strcpy(result_string, "Satisfiable");
		 Verify_NoSolver();

		 strcpy(competition_output, "SATISFIABLE");
       competition_exit_code=10;
	 }
		break;
    case SOLV_SAT: {
       if (ite_counters[NUM_SOLUTIONS] > 1) {
          sprintf(result_string, "Satisfiable(%lld)", ite_counters[NUM_SOLUTIONS]);
       } else {
          strcpy(result_string, "Satisfiable");
       }
       Verify_Solver();

		 strcpy(competition_output, "SATISFIABLE");
       competition_exit_code=10;
    } 
		break;
    case TRIV_UNSAT: 
    case SOLV_UNSAT: 
      strcpy(competition_output, "UNSATISFIABLE");
      competition_exit_code=20;
      strcpy(result_string, "Unsatisfiable"); break;
    case SOLV_UNKNOWN: strcpy(result_string, "Unknown Result"); break;
    case CONV_OUTPUT: strcpy(result_string, "Conversion"); break;
    case SOLV_ERROR: strcpy(result_string, "SOLVER ERROR: Result is not as expected"); break;
	 case SILENT:break;
    default: sprintf(result_string, "Error(%d)", ret); break;
   }

   ite_io_free();
   ite_main_free();
   sym_free();
   ite_free((void**)&length);

   ite_counters_f[RUNNING_TIME] = get_runtime();
   if (DEBUG_LVL == 1) {
      d1_printf3("%s %lld ", result_string, ite_counters[NUM_CHOICE_POINTS]);//ite_counters[NO_ERROR]);
      d1_printf4("%.3fs %.3fs %.3fs\n", 
            ite_counters_f[RUNNING_TIME], 
            ite_counters_f[PREPROC_TIME], 
            ite_counters_f[BRANCHER_TIME]);
   } else {
      if (DEBUG_LVL > 0) {
         d0_printf2("%s\n", result_string);
         d2_printf2("Total Time: %4.3f \n", get_runtime());
      } else {
         if (competition_enable == 0) {
            printf("%s\n", result_string);
         }
      }
   }

   free_terminal_in();
   free_terminal_out();

   if (competition_enable) {
      printf("c Time (total, preproc, brancher): %.3fs %.3fs %.3fs\n", 
            ite_counters_f[RUNNING_TIME], 
            ite_counters_f[PREPROC_TIME], 
            ite_counters_f[BRANCHER_TIME]);
      printf("s %s\n", competition_output);
      exit(competition_exit_code);
   }
   if (ret == SOLV_ERROR) return 1;
   return 0;
}

int 
ite_pre_init()
{
   int i;
   for (i=0;i<MAX_COUNTER;i++) ite_counters[i] = 0;
   for (i=0;i<MAX_COUNTER_F;i++) ite_counters_f[i] = 0;

   init_terminal_in();
   init_terminal_out();

   return NO_ERROR;
}

void 
ite_main_free()
{
   d9_printf1("ite_main_free\n");
   ite_free((void **)&variablelist);
   ite_free((void **)&original_functions);
	ite_free((void **)&var_score);
	
   bddtable_free_pools();
   itetable_free_pools();
   bdd_circuit_free();
}

void 
ite_io_free()
{
   d9_printf1("ite_io_free\n");
   if (finputfile && finputfile != stdin) fclose(finputfile);
   if (foutputfile && foutputfile != stdout && foutputfile != stderr) fclose(foutputfile);
   //if (fresultfile != stdin && fresultfile) fclose(fresultfile);
}

int 
ite_io_init()
{
   d9_printf1("ite_io_init\n");

   /* 
    * open the output file 
    */

   if (!strcmp(outputfile, "-")) foutputfile = stdout;
   else {
      struct stat buf;
      if (stat(outputfile, &buf) == 0 && (protect_outputfile == 1)) {
         dE_printf2("Error: File %s exists\n", outputfile);
         return ERR_IO_INIT;
      }
      foutputfile = fopen(outputfile, "w");
   }

   if (!foutputfile) { 
      dE_printf2("Can't open the output file: %s\n", outputfile);
      return ERR_IO_INIT;
   } else d9_printf2("Output file opened: %s\n", outputfile);

   /* 
    * open the result file
    */
   /*
    if (input_result_filename[0]) {
    fresultfile = fopen(input_result_filename, "r");

    if (!fresultfile) {
    dE_printf2("Can't open the result filename: %s\n", input_result_filename);
    return ERR_IO_INIT;
    } else
    d9_printf2("Result file opened: %s\n", input_result_filename);
    }
    */
   return NO_ERROR;
}

int 
ite_preprocessing()
{
   d9_printf1("ite_preprocessing\n");
   dC_printf1("c Starting preprocessing\n");
   int ret=0, prep_ret=0;
   /*
    * Preprocessing - start
    */
   prep_ret = Init_Preprocessing();

   if (prep_ret == PREP_NO_CHANGE || prep_ret == PREP_CHANGED) {
      prep_ret = Preprocessor();
   }

   ret = Finish_Preprocessing();

   if (prep_ret == TRIV_SAT || prep_ret == TRIV_UNSAT) {
      ret = prep_ret;
   }
   /*
    * Preprocessing - end
    */
   d9_printf2("ite_preprocessing done: %d\n", ret);
   return ret;
}

int 
check_expected_result(int result)
{
   switch (result) {
    case TRIV_SAT:
       if (strcasecmp(s_expected_result, "TRIV_SAT") && 
             strcasecmp(s_expected_result, "SAT")) return SOLV_ERROR;
       break;
    case TRIV_UNSAT: 
       if (strcasecmp(s_expected_result, "TRIV_UNSAT") && 
             strcasecmp(s_expected_result, "UNSAT")) return SOLV_ERROR;
       break;
    case SOLV_SAT:  
       if (strcasecmp(s_expected_result, "SOLV_SAT") &&
			  strcasecmp(s_expected_result, "SAT")) return SOLV_ERROR;
       break;
    case SOLV_UNSAT: 
		if (strcasecmp(s_expected_result, "SOLV_UNSAT") && 
			 strcasecmp(s_expected_result, "UNSAT")) return SOLV_ERROR;
       break;
    default: /* can't verify the result */
       dE_printf1("Can't check the result against the expected result\n");
       break;
   }	
   return result;
}

int
ITE_Final(int ret)
{
   return ite_final(ret); 
}

