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
#include "ite.h"

#define KEDAR 0

char tracer_tmp_filename[256];


int parser_init();
int trace_parse();
extern FILE *trace_in;

int
read_input(char formatin, char formatout, Tracer * &tracer)
{
  d9_printf1("read_input\n");

  switch  (formatin) {
  case 't':
    {
		 if (tracer5==0) {
			 char line[2048];
			 FILE *fg=NULL;
			 get_tempfile("tracer.tmp", tracer_tmp_filename, 255);
			 if ((fg = fopen (tracer_tmp_filename, "wb+"))==NULL) 
				{
					fprintf(stderr, "Can't open the temp file: %s\n", tracer_tmp_filename);
					exit(1);
				}
			 while (fgets (line, 2047, finputfile) != NULL) fputs (line, fg);
			 fclose (fg);
			 FlattenTrace * flatten = new FlattenTrace (tracer_tmp_filename);
			 flatten->normalizeInputTrace ();
			 flatten->insertModules ();
			 delete flatten;
				{
					char _tracer_tmp_filename[256];
					strcpy (_tracer_tmp_filename, tracer_tmp_filename);
					strcat (_tracer_tmp_filename, ".mac");
					tracer = new Tracer (_tracer_tmp_filename);
					if (tracer->parseInput ()) exit(1);
				}
		 } else {
          numinp=500000;
          numout=500000;
          bdd_circuit_init(numinp, numout);
          sym_init();
          parser_init();
          trace_in = finputfile;
          trace_parse();
          //exit(1);
       }
    } break;

  case 'b': bddloop (); break;

  case 'x': xorloop (); break;
	  
  case 'u': Smurfs_to_BDD (); break;

  case 'c': if (formatout == 'l') {
               //Do_Lemmas ();
               return 1; 
            } else CNF_to_BDD (1);
            break;

  case 'd': if (formatout == 'd') {
               if (KEDAR) fprintf (stdout, "S*****\n");
               DNF_to_CNF ();
               if (KEDAR) fprintf (stdout, "F*****\n");
               return 1; 
            } else CNF_to_BDD (0);
            break;

  case 's': if (formatout == 'c') {
              if (KEDAR) fprintf (stdout, "S*****\n");
              SAT_to_CNF ();
              if (KEDAR) fprintf (stdout, "F*****\n");
              return 1;
            } else {

            } break;

  default:
      fprintf (stderr, "Problem read_input: Unknown Input Format: %c\n", formatin);
      exit (1);
  }
  d9_printf1("read_input - done\n");
  return 0;
}

