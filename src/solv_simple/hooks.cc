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

bool CheckSimpleLimits(double fStartTime) {
	double fEndTime;
	fEndTime = get_runtime();
	if (nCtrlC) {
		return 1;
	}
	
	if (nTimeLimit && (fEndTime - fStartTime) > nTimeLimit) {
		d2_printf2("Bailling out because the Time limit of %lds ", nTimeLimit);
		d2_printf2("is smaller than elapsed time %.0fs\n", (fEndTime - fStartTime));
		return 1;
	}
	      
	if (nNumChoicePointLimit && ite_counters[NUM_CHOICE_POINTS]>nNumChoicePointLimit) {
		d2_printf2("Bailling out because the limit on the number of choicepoints %ld ",
					  nNumChoicePointLimit);
		d2_printf2("is smaller than the number of choicepoints %ld\n", (long)ite_counters[NUM_CHOICE_POINTS]);
		return 1;
	}

	return 0;
}

ITE_INLINE void SmurfStates_Push_Hooks(int current, int destination) {
	
	if(use_XORGElim==1) {
		pushXORGElimTable(SimpleSmurfProblemState->arrSmurfStack[current].XORGElimTable, SimpleSmurfProblemState->arrSmurfStack[destination].XORGElimTable);
	}

}

ITE_INLINE void SmurfStates_Pop_Hooks() {

	
}

ITE_INLINE void Alloc_SmurfStack_Hooks(int destination) {
	if(use_XORGElim==1) {
		SimpleSmurfProblemState->arrSmurfStack[destination].XORGElimTable = 
		  (XORGElimTableStruct *)ite_calloc(1, sizeof(XORGElimTableStruct), 9, "XORGElimTable");
		SimpleSmurfProblemState->arrSmurfStack[destination].XORGElimTable->frame = NULL;
	}
}

ITE_INLINE void Free_SmurfStack_Hooks(int destination) {
	if(use_XORGElim==1) {
      deleteXORGElimTable(SimpleSmurfProblemState->arrSmurfStack[destination].XORGElimTable);
      ite_free((void **)&SimpleSmurfProblemState->arrSmurfStack[destination].XORGElimTable);
	}
}

ITE_INLINE int ApplyInference_Hooks(int nBranchVar, bool bBVPolarity) {
	int ret = 1;

	if(print_search_dot) {

	}

	if(use_poor_mans_vsids) {
		arrPMVSIDS[nBranchVar]++;
	}
	
	if(use_XORGElim==1) { //SEAN!!! Would prefer not to do this if variable is not in the GE Table.
      ret = ApplyInferenceToXORGElimTable(SimpleSmurfProblemState->arrSmurfStack[SimpleSmurfProblemState->nCurrSearchTreeLevel].XORGElimTable, nBranchVar, bBVPolarity);
   }

	return ret;	
}

ITE_INLINE int ApplyInferenceToSmurf_Hooks(int nBranchVar, bool bBVPolarity, int nSmurfIndex, void **arrSmurfStates) {
	int ret = 1;

	if(((TypeStateEntry *)arrSmurfStates[nSmurfIndex])->cType == FN_XOR_GELIM) {
		ret = addRowXORGElimTable(SimpleSmurfProblemState->arrSmurfStack[SimpleSmurfProblemState->nCurrSearchTreeLevel].XORGElimTable,
										  ((XORGElimStateEntry *)arrSmurfStates[nSmurfIndex])->pVector,
										  ((XORGElimStateEntry *)arrSmurfStates[nSmurfIndex])->nSize,
										  ((XORGElimStateEntry *)arrSmurfStates[nSmurfIndex])->pnTransitionVars);
		if(ret == 1) {
			//To use the normal LSGB heuristic, comment out the line below.
			//arrSmurfStates[nSmurfIndex] = pTrueSimpleSmurfState; //addRowXORGElimTable returns -1 if the table is full.
         SimpleSmurfProblemState->arrSmurfStack[SimpleSmurfProblemState->nCurrSearchTreeLevel].nNumSmurfsSatisfied++;
		} else if (ret == -1) {
			ret = 1;			
		}
	}
	
	return ret;
}

ITE_INLINE void Calculate_Heuristic_Values_Hooks() {
	
	if(use_XORGElim==1) { //To use the normal LSGB heuristic, comment out the line below.
		//LSGBXORGElimTableGetHeurScore(SimpleSmurfProblemState->arrSmurfStack[SimpleSmurfProblemState->nCurrSearchTreeLevel].XORGElimTable);
	}
	
}

ITE_INLINE int Backtrack_Hooks() {
	int ret = SOLV_UNKNOWN;
	
	//Periodic Hooks
	if (ite_counters[NUM_BACKTRACKS] % BACKTRACKS_PER_STAT_REPORT == 0) {
		//Display hook
		DisplaySimpleSolverBacktrackInfo();

		//cvs trace file hook
		if (fd_csv_trace_file) {
			DisplaySimpleSolverBacktrackInfo_gnuplot();
		}
		//Solving limit hook
		if(CheckSimpleLimits(fSimpleSolverStartTime)==1) return SOLV_LIMIT;
	}

	return ret;	
}

ITE_INLINE void Init_Solver_PreSmurfs_Hooks() {
	
	if(use_XORGElim==1) {
		initXORGElimTable(SimpleSmurfProblemState->nNumVars);
      //SimpleSmurfProblemState->arrSmurfStack[SimpleSmurfProblemState->nCurrSearchTreeLevel].nNumSmurfsSatisfied--; //SEAN!!! TOTAL HACK!
	}
	
}


ITE_INLINE int Init_Solver_MidSmurfs_Hooks(int nSmurfIndex, void **arrSmurfStates) {
	int ret = SOLV_UNKNOWN;
	
   D_9(
       arrPrintStateEntry[((TypeStateEntry *)arrSmurfStates[nSmurfIndex])->cType](arrSmurfStates[nSmurfIndex]);
       );
   
	return ret;
}

size_t bytes_clause (unsigned size, unsigned learned) {
	size_t res;
	
	res = sizeof (Cls);
	res += (size-2) * sizeof (Lit *);
	//res -= 2 * sizeof (Lit *);
	
	return res;
}

ITE_INLINE int Init_Solver_PostSmurfs_Hooks(void **arrSmurfStates) {
	int ret = SOLV_UNKNOWN;

	//PicoSAT must go before the xor table, because we create all the lemmas
	//here and the xor table can cause inferences, which make use of the	lemmas.
	if(use_lemmas) {
		picosat_init_SBSAT(SimpleSmurfProblemState->nNumSmurfs);
		picosat_adjust(SimpleSmurfProblemState->nNumVars-1);
		picosat_set_seed(random_seed);
		
		SimpleSmurfProblemState->arrInferenceLemmas = (SimpleLemma *)ite_calloc(SimpleSmurfProblemState->nNumVars+1, sizeof(SimpleLemma), 9, "arrInferenceLemmas");
		for(int nVarIndex = 0; nVarIndex <= SimpleSmurfProblemState->nNumVars; nVarIndex++) {
			SimpleSmurfProblemState->arrInferenceLemmas[nVarIndex].clause = (Cls *)ite_calloc(1, bytes_clause(2, 0), 9, "arrInferenceLemmas.clause.lits");
			SimpleSmurfProblemState->arrInferenceLemmas[nVarIndex].max_size = 2;
		}
		SimpleSmurfProblemState->pConflictClause.clause = (Cls *)ite_calloc(1, bytes_clause(2, 0), 9, "pConflictClause.clause.lits");
		SimpleSmurfProblemState->pConflictClause.max_size = 2;

	}
	
	if(use_XORGElim==1) {
		initXORGElimTable(SimpleSmurfProblemState->nNumVars);
		allocXORGElimTable(SimpleSmurfProblemState->arrSmurfStack[SimpleSmurfProblemState->nCurrSearchTreeLevel].XORGElimTable, 0);
		for(int nSmurfIndex = 0; nSmurfIndex < SimpleSmurfProblemState->nNumSmurfs; nSmurfIndex++) {
			if(((TypeStateEntry *)arrSmurfStates[nSmurfIndex])->cType == FN_XOR_GELIM) {
				ret = addRowXORGElimTable(SimpleSmurfProblemState->arrSmurfStack[SimpleSmurfProblemState->nCurrSearchTreeLevel].XORGElimTable,
												  ((XORGElimStateEntry *)arrSmurfStates[nSmurfIndex])->pVector,
												  ((XORGElimStateEntry *)arrSmurfStates[nSmurfIndex])->nSize,
												  ((XORGElimStateEntry *)arrSmurfStates[nSmurfIndex])->pnTransitionVars);
				if(ret == 0) return SOLV_UNSAT;
				if(ret == 1) {
					//To use the normal LSGB heuristic, comment out the line below.
					//arrSmurfStates[nSmurfIndex] = pTrueSimpleSmurfState; //addRowXORGElimTable returns -1 if the table is full.
               SimpleSmurfProblemState->arrSmurfStack[SimpleSmurfProblemState->nCurrSearchTreeLevel].nNumSmurfsSatisfied++;
				} else if (ret == -1) {
					ret = 1;
				}
			}
		}
	}

	for(int nSmurfIndex = 0; nSmurfIndex < SimpleSmurfProblemState->nNumSmurfs; nSmurfIndex++) {
		if(((TypeStateEntry *)arrSmurfStates[nSmurfIndex])->cType == FN_INFERENCE) {
         ret = TransitionInference(nSmurfIndex, arrSmurfStates);			
		}
	}
	
	if(ret != 0)
	  return SOLV_UNKNOWN;
	
	return ret;
}

ITE_INLINE void Final_Solver_Hooks() {
//	if(use_XORGElim==1)
//	  deleteXORGElimTable(SimpleSmurfProblemState->arrSmurfStack[SimpleSmurfProblemState->nCurrSearchTreeLevel].XORGElimTable);
   
   if(use_lemmas) {
		for(int nVarIndex = 0; nVarIndex <= SimpleSmurfProblemState->nNumVars; nVarIndex++) {
			ite_free((void **)&SimpleSmurfProblemState->arrInferenceLemmas[nVarIndex].clause);
		}
      ite_free((void **)&SimpleSmurfProblemState->arrInferenceLemmas);
		ite_free((void **)&SimpleSmurfProblemState->pConflictClause.clause);
      picosat_reset();
   }
}
