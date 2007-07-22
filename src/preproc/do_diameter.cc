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

int diameter_vars;
int *var_depth;

int **dia_clause_list, *dia_clause_table;
int *clause_stamps, CCS;
int *dstack;

#define DIAMETER_FIX( __a ) \
{\
    if( var_depth[ __a ] == 0 ) {\
		 var_depth[ __a ] = current_depth + 1; \
		 *(dstackp++) = __a; \
	 }\
}

int compute_diameter( const int varnr );

int Do_Diameter() {
	int i, j, nrofliterals;
	int max_diameter = 0, nr_max = 0, count_diameter = 0;
	int min_diameter = numinp, nr_min = 0;
	double sum_diameter = 0.0;
	int n_min=0, n_max=0;
	CCS = 0;
	
	dstack  = (int*) calloc(numinp, sizeof(int));
	var_depth = (int*) calloc(numinp+1, sizeof(int));
	
	nrofliterals = 0;
	
	for(i = 0; i < nmbrFunctions; i++ )
		nrofliterals+=length[i];
	
	clause_stamps = (int*) calloc(nmbrFunctions, sizeof(int));

	dia_clause_table = (int*) calloc(nrofliterals + numinp, sizeof(int));
	for( i = 0; i < nrofliterals + numinp; i++ )
	  dia_clause_table[ i ] = -1;
	
	dia_clause_list = (int**) calloc(numinp+1, sizeof(int*));
	
	nrofliterals = 0;
	for(i = 1; i <= numinp; i++ ) {
		dia_clause_list[ i ] = &dia_clause_table[ nrofliterals ];
		nrofliterals += num_funcs_var_occurs[ i ] + 1;
		num_funcs_var_occurs[ i ] = 0;
	}
	
	for( i = 0; i < nmbrFunctions; i++ )
	  for( j = 0; j < length[ i ]; j++ )
		 dia_clause_list[ variables[i].num[j] ][ num_funcs_var_occurs[ variables[i].num[j] ]++ ] = i;
	
	for( i = 1; i <= numinp; i++ ) {
		int diameter = compute_diameter( i );
		
		if( diameter > max_diameter ) {
			  max_diameter = diameter; nr_max = 1; n_max = i;
		} else if( diameter == max_diameter ) nr_max++;
		
		if( diameter > 0 && diameter < min_diameter ) {
			min_diameter = diameter; nr_min = 1; n_min = i;
		} else if( diameter == min_diameter ) nr_min++;
		
		if( diameter > 0 ) {
			sum_diameter += diameter;
			count_diameter++;
		}
	}
	
	d2_printf8("\nc diameter():: MIN: %i (#%i, v%d) MAX: %i (#%i, v%d) AVG: %.3f\n", min_diameter, nr_min, n_min,
			 max_diameter, nr_max, n_max, sum_diameter / count_diameter );

	nVarChoiceLevelsNum++;
	if(nVarChoiceLevelsMax <= nVarChoiceLevelsNum) {
		arrVarChoiceLevels = (int **)ite_recalloc(arrVarChoiceLevels, nVarChoiceLevelsMax, nVarChoiceLevelsMax+10, sizeof(int *), 9, "arrVarChoiceLevels");
		nVarChoiceLevelsMax += 10;
	}
	
	for(int x = nVarChoiceLevelsNum-1; x > 0; x++)
	  arrVarChoiceLevels[x] = arrVarChoiceLevels[x-1];
	arrVarChoiceLevels[0] = (int *)ite_calloc(2, sizeof(int), 9, "arrVarChoiceLevels[0]");
	
	arrVarChoiceLevels[0][0] = n_max;
	arrVarChoiceLevels[0][1] = 0;
	
	free(dia_clause_table);
	free(dia_clause_list);
	free(dstack);
	free(var_depth);
	free(clause_stamps);	
	
	return PREP_NO_CHANGE;
}

int compute_diameter( const int varnr ) {
	int i, _varnr, current_depth = 0;
	int *_dstackp = dstack;
	int * dstackp = dstack;
	int *clauses, _clause;
	
	if( dia_clause_list[ varnr ][ 0 ] == -1 )
	  return 0;
	
	for( i = 1; i < numinp; i++ ) var_depth[ i ] = 0;
	
	CCS++;
	
	DIAMETER_FIX( varnr );
	
	while( _dstackp < dstackp ) {
		_varnr = *(_dstackp++);
		current_depth = var_depth[ _varnr ];
		
		clauses = dia_clause_list[ _varnr ];
		while( *clauses != -1 ) {
			_clause = *(clauses++);
			if( clause_stamps[ _clause ] == CCS ) continue;
			clause_stamps[ _clause ] = CCS;
			
			for( i = 0; i < length[ _clause ]; i++ ) {
				DIAMETER_FIX( variables[ _clause ].num[ i ] );
		   }
		}
	}
	

	/*for( i = 1; i <= current_depth; i++ )
	  {
		  int j;
		  printf("\ndepth %i :: ", i);
		  for( j = 1; j <= numinp; j++ )
	       if( var_depth[j] == i )
				printf("%i ", j);
	  }*/
	
	return current_depth;
}
