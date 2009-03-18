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

/********************************************************************
 *   holds a single function getformat() which looks at an input file
 *   and determines what type it is (CNF, Trace, etc.)
 *   Returns a single char representing type.
 ********************************************************************/  

#include "sbsat.h"
#include "sbsat_formats.h"

char getformat () {
	char a, output;
	a = fgetc (finputfile);
  d9_printf3("getformat(FILE): first character: %c(0x%x)\n", a, a);
   
   // check for trace format
	if (a == 'M') {
		char test[10];
		int x;
      test[0] = a;
      for (x=1; x < 6; x++) {
			a = fgetc (finputfile);
         if (feof(finputfile)) return 0;
			test[x] = a;
		}
      for (x=0; x<6; x++)
         ungetc (test[5-x], finputfile);

      if (!strncmp (test, "MODULE", 6)) 
         return 't';
      else
         return 0;

   // check for prover3 format
   } else if (a == 'S') {
		char test[10];
		int x;
      test[0] = a;
      for (x=1; x < 5; x++) {
			a = fgetc (finputfile);
         if (feof(finputfile)) return 0;
			test[x] = a;
		}
      for (x=0; x<5; x++)
         ungetc (test[4-x], finputfile);
      if (!strncmp (test, "S0\n&\n", 5)) 
			return '3';
      else
         return 0;

/*
   } else if (a == 'I') {
		char test[10];
		int x;
      test[0] = a;
      for (x=1; x < 5; x++) {
			a = fgetc (finputfile);
         if (feof(finputfile)) return 0;
			test[x] = a;
		}
		for (x=0; x<5; x++)
			  ungetc (test[4-x], finputfile);
      if (!strncmp (test, "INPUT", 5)) 
			return 'i';
      else
		   return 0;
 */
   // check for bdd input format ???
	} else if (a == 'i') {
		//a = fgetc (finputfile);
		ungetc (a, finputfile);
		return 'b';

   // check for smurf input format
	} else if (a == 'a') {
		a = fgetc (finputfile);
		if(a == 'i') {
			finputfile = aigread(inputfile);
			//decode AIG format;
			return 'a';
		} else if(a != 'a')
			  return 0;

		ungetc(a, finputfile);
		ungetc('a', finputfile);
		return 'a';
	} else if (a >= '1' && a <= '9') {
      ungetc (a, finputfile);
		return 'u';		//u for smUrf
	}

   // check for DIMACS
   if (feof(finputfile)) return 0;
   while (a == 'c' || a == '\n') {
      while (a != '\n') {
         a = fgetc (finputfile);
         if (feof(finputfile)) return 0;
      }
      a = fgetc (finputfile);
      if (feof(finputfile)) return 0;
   }
   if (a != 'p') return 0; // unknown format

   a = fgetc (finputfile); // space
   if (feof(finputfile) || a != ' ') return 0;
	output = fgetc (finputfile); // format 
   if (feof(finputfile)) return 0;
	a = fgetc (finputfile); // format
   if (feof(finputfile)) return 0;
	a = fgetc (finputfile); // format
   if (feof(finputfile)) return 0;
	a = fgetc (finputfile); // space
   if (feof(finputfile) || a != ' ') return 0;
	return output;		// c for CNF d for DNF s for SAT, 
	// b for BDD's, x for XOR
}
