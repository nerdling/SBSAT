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
#include "solver.h"

int *arrSimpleSolver2IteVarMap=NULL;
int *arrIte2SimpleSolverVarMap=NULL;

double *arrVarTrueInfluencesPlaceHolder;

ITE_INLINE
int InitSimpleVarMap() {
   int total_vars = numinp = getNuminp();

	int nIndepVars=0;
	int nDepVars=0;
	
	//reversing dependency if requested
   if (reverse_independant_dependant) {
      for (int x = 0; x < numinp + 1; x++) {
         if (independantVars[x] == 1)
			  independantVars[x] = 0;
         else if (independantVars[x] == 0)
			  independantVars[x] = 1;
      }
   }
	
   if (clear_dependance) {
      for (int x = 0; x < numinp + 1; x++)
		  if (independantVars[x] == 0)
			 independantVars[x] = 1;
   }
	
	int nNumVars = 1; /* 0 => true, false */
	
	for (int x = 0; x <= numinp; x++) {
		if (variablelist[x].true_false == -1) {
			nNumVars++;
			if (independantVars[x] == 1) nIndepVars++;
			else if (independantVars[x] == 0) nDepVars++;
		}
	}
	/* temporary variables (xor) */
	nNumVars += (total_vars - numinp);
	
	d3_printf4("Solver vars: %d/%d (not used: %d)\n", nNumVars-1, total_vars, total_vars-nNumVars+1);
	d3_printf4("Indep/Dep vars: %d/%d (other vars: %d)\n", nIndepVars, nDepVars, nNumVars-1-nIndepVars-nDepVars);
	
	/* init mapping arrays */
	arrSimpleSolver2IteVarMap = (int *)ite_calloc(nNumVars, sizeof(int), 2, "solver mapping(s2i)");
	arrIte2SimpleSolverVarMap = (int *)ite_calloc(total_vars+1, sizeof(int), 2, "solver mapping(i2s)");
	
	int nSolutionIndep = 1;
	int nSolutionDep = nIndepVars+1;
	int nSolutionOther = nIndepVars+nDepVars+1;
	int index;
	
	for (int x=0;x<total_vars+1;x++) {
		index = -1;
		if (x > numinp) index = nSolutionOther++;
		else if (variablelist[x].true_false == -1) {
			if (independantVars[x] == 1) index = nSolutionIndep++;
			else if (independantVars[x] == 0) index = nSolutionDep++;
			else index = nSolutionOther++;
		}
		
		if (index != -1) {
			arrSimpleSolver2IteVarMap[index] = x;
			arrIte2SimpleSolverVarMap[x] = index;
			if (x <= numinp && var_score) 
           var_score[index] = var_score[x];
		}
	}
	
	if(arrVarChoiceLevels) {
		for(int level = 0; level < nVarChoiceLevelsNum; level++) {
			int j=0;
			while(arrVarChoiceLevels[level][j]!=0) {
				arrVarChoiceLevels[level][j] = arrIte2SimpleSolverVarMap[arrVarChoiceLevels[level][j]];
				j++;
			}
		}
	}
	if(arrVarTrueInfluences) {
		double *arrVarTrueInfluencesTmp = (double *)ite_calloc(nNumVars, sizeof(double), 9, "arrVarTrueInfluencesTmp");
		for(int x=0; x < nNumVars; x++) {
			if(arrSimpleSolver2IteVarMap[x] == 0)
			  arrVarTrueInfluencesTmp[x] = 0.5;
			else arrVarTrueInfluencesTmp[x] = arrVarTrueInfluences[arrSimpleSolver2IteVarMap[x]];
		}
		arrVarTrueInfluencesPlaceHolder = arrVarTrueInfluences;
		arrVarTrueInfluences = arrVarTrueInfluencesTmp;
	}

	assert (nmbrFunctions > 0);
	assert (nSolutionIndep == nIndepVars+1);
	assert (nSolutionDep-nIndepVars-1 == nDepVars);

	return nNumVars;
}

ITE_INLINE
void FreeSimpleVarMap() {
	if(arrVarChoiceLevels) {
		for(int level = 0; level < nVarChoiceLevelsNum; level++) {
			int j=0;
			while(arrVarChoiceLevels[level][j]!=0) {
				arrVarChoiceLevels[level][j] = arrSimpleSolver2IteVarMap[arrVarChoiceLevels[level][j]];
				j++;
			}
		}
	}

	if(arrVarTrueInfluences) {
		ite_free((void**)&(arrVarTrueInfluences));
		arrVarTrueInfluences = arrVarTrueInfluencesPlaceHolder;
	}
	
	ite_free((void**)&arrSimpleSolver2IteVarMap);
   ite_free((void**)&arrIte2SimpleSolverVarMap);
}
