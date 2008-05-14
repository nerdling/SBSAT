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

int aag_use_symtable = 1;

void aag_print_independent_vars() {
	for(int i = 1; i <= numinp; i++) {
		if(independantVars[i] == 1-reverse_independant_dependant)
		  fprintf (foutputfile, "%d\n ", aag_use_symtable?atoi(getsym_i(i)->name):i);
	}
}

void print_aag_symtable() {
	for(int i = 1; i <= numinp; i++)
	  fprintf(foutputfile, "%d = %s\n", i, getsym_i(i)->name);
}

void printBDDToAAG () {	

   //numinp (global) is the current number of BDD variables;
   numinp = getNuminp();

   //Print the header:
   //fprintf (foutputfile, "aag %d %d %d %d %d\n", ?, ?, ?, ?, ?);

	if(print_independent_vars) {
		aag_print_independent_vars();
	}

   //BDDs are stored here: BDDNode **functions[0..nmbrFunctions];
   //We can print them all to the outputfile:
   for(int i = 0; i < nmbrFunctions; i++) {
      d2_printf3("%d/%d \r", i, nmbrFunctions); //Print progress to the screen

		fprintf(foutputfile, "Constraint #%d ", i);
		
		//We can print out the function type
		//(supported function types are shown in bdd.cc:findandset_fnType())
		if(functionType[i] == AND)
		  fprintf(foutputfile, "AND= ");
		else fprintf(foutputfile, "UNSURE ");

		//If SBSAT auto detected an equality variable, we can print it.
		if(equalityVble[i] != 0) fprintf(foutputfile, "eq:%d ", aag_use_symtable?atoi(getsym_i(equalityVble[i])->name):equalityVble[i]);
				
		//We can print the BDD
		fprintf(foutputfile, "\nBDD: ");
		if(aag_use_symtable)
		  printBDDfile_sym(functions[i], foutputfile);
		else printBDDfile(functions[i], foutputfile);
		fprintf(foutputfile, "\n");
		
	}

   //We can print the symbol table
   if(aag_use_symtable) {
		print_aag_symtable();
	}

}
