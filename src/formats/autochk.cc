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
/********************************************************************
 *   autochk.c (S. Weaver)
 *   holds a single function getformat() which looks at an input file
 *   and determines what type it is (CNF, Trace, etc.)
 *   Returns a single char representing type.
 ********************************************************************/  
#include "ite.h"
#include "formats.h"

char getformat (FILE *finputfile) {
	char a, output;
	a = fgetc (finputfile);
	if (a == 'M') {
		char test[10];
		int x;
      test[0] = a;
      for (x=1; x < 6; x++) {
			a = fgetc (finputfile);
			test[x] = a;
		}
      test[6] = 0;
      if (!strcmp (test, "MODULE")) {
			for (x=0; x<6; x++)
			  ungetc (test[5-x], finputfile);
			return 't';
		}
		for (x=0; x<6; x++)
		  ungetc (test[5-x], finputfile);
		return 0;
	} else if (a == 'S') {
		char test[10];
		int x;
      test[0] = a;
      for (x=1; x < 5; x++) {
			a = fgetc (finputfile);
			test[x] = a;
		}
      if (!strncmp (test, "S0\n&\n", 5)) {
			for (x=0; x<5; x++)
			  ungetc (test[4-x], finputfile);
			return '3';
		}
		for (x=0; x<5; x++)
		  ungetc (test[4-x], finputfile);
		return 0;
	} else if (a == 'I') {
		char test[10];
		int x;
      test[0] = a;
      for (x=1; x < 5; x++) {
			a = fgetc (finputfile);
			test[x] = a;
		}
      test[5] = 0;
      if (!strcmp (test, "INPUT")) {
			for (x=0; x<5; x++)
			  ungetc (test[4-x], finputfile);
			return 'i';
		}
		for (x=0; x<5; x++)
		  ungetc (test[4-x], finputfile);
		return 0;
	} else if (a == 'i') {
		a = fgetc (finputfile);
		ungetc (a, finputfile);
		return 'b';
	} else if (a >= '1' && a <= '9') {
      ungetc (a, finputfile);
		return 'u';		//u for smUrf
	} else
	  while (a == 'c') {
		  while (a != '\n') {
			  a = fgetc (finputfile);
		  }
		  a = fgetc (finputfile);
	  }
	while (a != 'p') {
		a = fgetc (finputfile);
      
		//fputc(a, fg);
	}
	a = fgetc (finputfile);
	output = fgetc (finputfile);
	a = fgetc (finputfile);
	a = fgetc (finputfile);
	a = fgetc (finputfile);
	return output;		// c for CNF d for DNF s for SAT, 
	// b for BDD's, x for XOR
}
