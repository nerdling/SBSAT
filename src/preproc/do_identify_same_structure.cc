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
#include "sbsat_preproc.h"

int Do_Identify_Same_Structure() {
	d3_printf1("IDENTIFYING STRUCTURE - ");
	str_length = 0;

	int ret = PREP_NO_CHANGE;

	if(arrFunctionStructure == NULL)
	  arrFunctionStructure = (int *)ite_calloc(nmbrFunctions, sizeof(int), 9, "arrFunctionStructure");
	else return ret;
	
	int *arrTempFunctionStructure = (int *)ite_calloc(nmbrFunctions, sizeof(int), 9, "arrTempFunctionStructure");
	
	int nNumStructures = 0;

	for(int x = 0; x < nmbrFunctions; x++) {
		int found = 0;
		int y;
		for(y = 0; y < nNumStructures; y++) {
			if(length[x]!=length[arrTempFunctionStructure[y]]) continue;
			//printBDDerr(functions[arrTempFunctionStructure[y]]);
			//fprintf(stderr, "\n");
			//printBDDerr(functions[x]);
			//fprintf(stderr, "\n");
			//printBDDerr(shared_structure(functions[arrTempFunctionStructure[y]], arrTempFunctionStructure[y], x));
			//fprintf(stderr, "\n");

			if(shared_structure(functions[arrTempFunctionStructure[y]], arrTempFunctionStructure[y], x) == functions[x]) {
				found = 1; break;
			}
		}
		if(found == 0) { //Found a new function structure
			arrTempFunctionStructure[y] = x;
			nNumStructures++;
		}
		arrFunctionStructure[x] = y;

		//fprintf(stderr, "%d = %d\n", x, y);
	}

	ite_free((void **)&arrTempFunctionStructure);
	
	return ret;
}
