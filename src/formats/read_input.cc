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
#include "sbsat_formats.h"

int parser_init();

int trace_parse();
extern FILE *trace_in;

int prover_parse();
extern FILE *prover_in;

int prover3_parse();
void prover3_free();
extern FILE *prover3_in;

int iscas_parse();
extern FILE *iscas_in;

int aig_parse();
extern FILE *aig_in;

int nle_parse();
extern FILE *nle_in;

int read_input_open();

int
read_input()
{
  d9_printf1("read_input\n");

  int ret = read_input_open();
  if (ret != NO_ERROR) return ret;

  /* autodetect the input format  */
  if (formatin == ' ') formatin = getformat ();

  switch  (formatin) {
  case 't':
    {
       parser_init();
       trace_in = finputfile;
       trace_parse();
       numinp = vars_max;
       numout = functions_max;
       sym_clear_all_flag(); 
    } break;

  case 'b': bddloop(); break;

  case 'x': xorloop(); break;
	  
  case 'u': Smurfs_to_BDD(); break;

  case 'c': CNF_to_BDD(); break;

  case 'd': DNF_to_BDD(); break;

  case 'B': Binary_to_BDD(); break;

  case 's': if (formatout == 'c') {
              SAT_to_CNF ();
              return 1;
            } else {

            } break;
  case 'p': {
     parser_init();
     prover_in = finputfile;
     prover_parse();
     numinp = vars_max;
     numout = functions_max;
  } break;
  case '3': {
     parser_init();
     prover3_in = finputfile;
     prover3_parse();
     //prover3_free();
     numinp = vars_max;
     numout = functions_max;
  } break;
  case 'i': {
     parser_init();
     iscas_in = finputfile;
     iscas_parse();
     numinp = vars_max;
     numout = functions_max;
  } break;
  case 'a': {
     parser_init();
     aig_in = finputfile;
     aig_parse();
     numinp = vars_max;
     numout = functions_max;
  } break;
  case 'l': {
     parser_init();
     nle_in = finputfile;
     nle_parse();
     numinp = vars_max;
     numout = functions_max;
  } break;
  default:
      fprintf (stderr, "Problem read_input: Unknown Input Format: %c\n", formatin);
      exit (1);
  }
  d9_printf1("read_input - done\n");
  return 0;
}

int
read_input_open()
{
   /* 
    * open the input file 
    */

   if (!strcmp(inputfile, "-")) { d2_printf2("Reading standard input %s....\n", comment); }
   else { d2_printf3("Reading File %s %s ....\n", inputfile, comment); }

   int zip=check_gzip(inputfile);
   if (zip) {
      d2_printf1("gzip file -- using zread\n");
      finputfile = zread(inputfile, zip);
   }
   else
      if (!strcmp(inputfile, "-")) {
         finputfile = stdin;
      }
      else {
         finputfile = fopen(inputfile, "r");
      }

   if (!finputfile) { 
      dE_printf2("Can't open the input file: %s\n", inputfile);
      return ERR_IO_INIT;
   } else d9_printf2("Input file opened: %s\n", inputfile);
   
   return NO_ERROR;
}

