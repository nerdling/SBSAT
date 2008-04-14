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

void DisplaySimpleStatistics(int nNumChoicePts, int nNumBacktracks, int nNumBackjumps) {
	d2_printf2("Choice Points: %d", nNumChoicePts);
	d2_printf3(", Backtracks: %d, Backjumps: %d \n", 
				  nNumBacktracks, nNumBackjumps);
}

void PrintAllSmurfStateEntries() {
	SmurfStatesTableStruct *arrSmurfStatesTable = SimpleSmurfProblemState->arrSmurfStatesTableHead;
	int state_num = 0;
	while(arrSmurfStatesTable != NULL) {
		void *arrSmurfStates = arrSmurfStatesTable->arrStatesTable;
		int size = 0;
		for(int x = 0; x < arrSmurfStatesTable->curr_size; x+=size) {
			state_num++;
			d9_printf3("State %d(%x), ", state_num, arrSmurfStates);
			if(((TypeStateEntry *)arrSmurfStates)->cType == FN_SMURF) {
				PrintSmurfStateEntry((SmurfStateEntry *)arrSmurfStates);
				arrSmurfStates = (void *)(((SmurfStateEntry *)arrSmurfStates) + 1);
				size = sizeof(SmurfStateEntry);
			} else if(((TypeStateEntry *)arrSmurfStates)->cType == FN_INFERENCE) {
				PrintInferenceStateEntry((InferenceStateEntry *)arrSmurfStates);
				arrSmurfStates = (void *)(((InferenceStateEntry *)arrSmurfStates) + 1);
				size = sizeof(InferenceStateEntry);
			} else if(((TypeStateEntry *)arrSmurfStates)->cType == FN_OR) {
				PrintORStateEntry((ORStateEntry *)arrSmurfStates);
				arrSmurfStates = (void *)(((ORStateEntry *)arrSmurfStates) + 1);
				size = sizeof(ORStateEntry);
			} else if(((TypeStateEntry *)arrSmurfStates)->cType == FN_OR_COUNTER) {
				PrintORCounterStateEntry((ORCounterStateEntry *)arrSmurfStates);
				arrSmurfStates = (void *)(((ORCounterStateEntry *)arrSmurfStates) + 1);
				size = sizeof(ORCounterStateEntry);
			} else if(((TypeStateEntry *)arrSmurfStates)->cType == FN_XOR) {
				PrintXORStateEntry((XORStateEntry *)arrSmurfStates);
				arrSmurfStates = (void *)(((XORStateEntry *)arrSmurfStates) + 1);
				size = sizeof(XORStateEntry);
			} else if(((TypeStateEntry *)arrSmurfStates)->cType == FN_XOR_COUNTER) {
				PrintXORCounterStateEntry((XORCounterStateEntry *)arrSmurfStates);
				arrSmurfStates = (void *)(((XORCounterStateEntry *)arrSmurfStates) + 1);
				size = sizeof(XORCounterStateEntry);
			} else if(((TypeStateEntry *)arrSmurfStates)->cType == FN_XOR_GELIM) {
				PrintXORGElimStateEntry((XORGElimStateEntry *)arrSmurfStates);
				arrSmurfStates = (void *)(((XORGElimStateEntry *)arrSmurfStates) + 1);
				size = sizeof(XORGElimStateEntry);
			}
		}
		arrSmurfStatesTable = arrSmurfStatesTable->pNext;
	}
}

void PrintSmurf_dot(void *ssEntry) {
	if(((TypeStateEntry *)ssEntry)->visited) return;
	((TypeStateEntry *)ssEntry)->visited = 1;
	if(((SmurfStateEntry *)ssEntry) == pTrueSimpleSmurfState) {
	
	} else if(((TypeStateEntry *)ssEntry)->cType == FN_SMURF) {
		PrintSmurfStateEntry_dot((SmurfStateEntry *)ssEntry);
		SmurfStateEntry *pState = (SmurfStateEntry *)ssEntry;
		while(pState!=NULL) {
			PrintSmurf_dot(pState->pVarIsTrueTransition);
			PrintSmurf_dot(pState->pVarIsFalseTransition);
			pState = (SmurfStateEntry *)pState->pNextVarInThisState;
		}
	} else if(((TypeStateEntry *)ssEntry)->cType == FN_INFERENCE) {
		PrintInferenceStateEntry_dot((InferenceStateEntry *)ssEntry);
		PrintSmurf_dot(((InferenceStateEntry *)ssEntry)->pVarTransition);
	} else if(((TypeStateEntry *)ssEntry)->cType == FN_OR) {
		PrintORStateEntry_dot((ORStateEntry *)ssEntry);
	} else if(((TypeStateEntry *)ssEntry)->cType == FN_OR_COUNTER) {
		PrintORCounterStateEntry_dot((ORCounterStateEntry *)ssEntry);
	} else if(((TypeStateEntry *)ssEntry)->cType == FN_XOR) {
		PrintXORStateEntry_dot((XORStateEntry *)ssEntry);
	} else if(((TypeStateEntry *)ssEntry)->cType == FN_XOR_COUNTER) {
		PrintXORCounterStateEntry_dot((XORCounterStateEntry *)ssEntry);
	} else if(((TypeStateEntry *)ssEntry)->cType == FN_XOR_GELIM) {
		PrintXORGElimStateEntry_dot((XORGElimStateEntry *)ssEntry);
	}
}

void PrintAllXORSmurfStateEntries() {
	void **arrSmurfStates = SimpleSmurfProblemState->arrSmurfStack[SimpleSmurfProblemState->nCurrSearchTreeLevel].arrSmurfStates;

	for(int i = 0; i < SimpleSmurfProblemState->nNumSmurfs; i++) {
		if(arrSmurfStates[i] == pTrueSimpleSmurfState) continue;
		void *pState = arrSmurfStates[i];
		if(((TypeStateEntry *)pState)->cType == FN_XOR_COUNTER) {
			PrintXORStateEntry_formatted(((XORCounterStateEntry *)pState)->pXORState);
		} else if(((TypeStateEntry *)pState)->cType == FN_XOR) {
			PrintXORStateEntry_formatted((XORStateEntry *)pState);
		}
	}
	d2_printf1("\n");
}

ITE_INLINE
void CalculateSimpleSolverProgress(int *_whereAmI, int *_total) {
	int whereAmI=0;
	int total=0;
	int soft_count=14;
	int hard_count=28;
	int count=0;
	int nInfQueueTail = 0;
	
	if(SimpleSmurfProblemState->nCurrSearchTreeLevel >= 0)
	  nInfQueueTail = SimpleSmurfProblemState->arrSmurfStack[SimpleSmurfProblemState->nCurrSearchTreeLevel].nNumFreeVars;
	
	int nCurrSearchTreeLevel = 0;
	for(int i = nInfQueueStart; i < nInfQueueTail && (count<soft_count || (count < hard_count && whereAmI==0)); i++) {
		int nBranchLit = SimpleSmurfProblemState->arrInferenceQueue[i];
		if(SimpleSmurfProblemState->arrInferenceDeclaredAtLevel[abs(nBranchLit)] < 0 ||
			(SimpleSmurfProblemState->arrInferenceDeclaredAtLevel[abs(nBranchLit)] == 0 && add_one_display==1)) {
			//This variable is an old choicepoint
			whereAmI *= 2;
			whereAmI += 1;
			total *= 2;
			total += 1;
			count++;
		} else if(nBranchLit == SimpleSmurfProblemState->arrInferenceQueue[SimpleSmurfProblemState->arrSmurfStack[nCurrSearchTreeLevel].nNumFreeVars]) {
			//This variable is a current choicepoint
			whereAmI *= 2;
			total *= 2;
			total += 1;
			nCurrSearchTreeLevel++;
			count++;
		}		
	}
	*_whereAmI = whereAmI;
	*_total    = total;
}

void DisplaySimpleSolverBacktrackInfo(double &fSimpleSolverPrevEndTime, double &fSimpleSolverStartTime) {
	double fSimpleSolverEndTime = ite_counters_f[CURRENT_TIME] = get_runtime();
	double fTotalDurationInSecs = fSimpleSolverEndTime - fSimpleSolverStartTime;
	double fDurationInSecs = fSimpleSolverEndTime - fSimpleSolverPrevEndTime;
	double fBacktracksPerSec = BACKTRACKS_PER_STAT_REPORT / (fDurationInSecs>0?fDurationInSecs:0.001);
	fSimpleSolverPrevEndTime = fSimpleSolverEndTime;
	
	d2_printf2("Time: %4.3fs. ", fTotalDurationInSecs);
	d2_printf3("Backtracks: %ld (%4.3f per sec) ",
				  (long)ite_counters[NUM_BACKTRACKS], fBacktracksPerSec);
	
	int whereAmI = 0;
	int total = 0;
	double progress = 0.0;
	CalculateSimpleSolverProgress(&whereAmI, &total);
	if (total == 0) total=1;
	progress = ite_counters_f[PROGRESS] = (float)whereAmI*100/total;
	//d2_printf3("Progress: %x/%x        ", whereAmI, total);
	d2_printf1("Progress: ");
	char number[10];
	char back[10] = "\b\b\b\b\b\b\b\b\b";
	sprintf(number, "% 3.2f%%", progress);
	//sprintf(number, " ?");
	back[strlen(number)]=0;
	if ((DEBUG_LVL&15) == 1) {
		fprintf(stderr, "%s%s", number, back);
	} else {
		D_1(
			 d0_printf3("%s%s", number, back);
			 fflush(stddbg);
			 )
	}
	
	d2_printf1("\n Choices (total, dependent" );
	dC_printf4("c %4.3fs. Progress %s Choices: %lld\n", fTotalDurationInSecs, number, ite_counters[NO_ERROR]);
	if (backjumping) d2_printf1(", backjumped");
	d2_printf3("): (%lld, %lld", ite_counters[NUM_CHOICE_POINTS], ite_counters[HEU_DEP_VAR]);
	if (backjumping) d2_printf2(", %lld", ite_counters[NUM_TOTAL_BACKJUMPS]);
	d2_printf1(")");
	
	d2_printf1("\n");
	d2_printf1(" Inferences by ");
	d2_printf2("smurfs: %lld; ", ite_counters[INF_SMURF]);
	d2_printf2("ANDs: %lld; ", ite_counters[INF_SPEC_FN_AND]);
	d2_printf2("XORs: %lld; ", ite_counters[INF_SPEC_FN_XOR]);
	d2_printf2("MINMAXs: %lld; ", ite_counters[INF_SPEC_FN_MINMAX]);
	if (NO_LEMMAS == 0) d2_printf2("lemmas: %lld; ", ite_counters[INF_LEMMA]);
	d2_printf1("\n");
	
	d2_printf1(" Backtracks by ");
	d2_printf2("smurfs: %lld; ", ite_counters[ERR_BT_SMURF]);
	d2_printf2("ANDs: %lld; ", ite_counters[ERR_BT_SPEC_FN_AND]);
	d2_printf2("XORs: %lld; ", ite_counters[ERR_BT_SPEC_FN_XOR]);
	d2_printf2("MINMAXs: %lld; ", ite_counters[ERR_BT_SPEC_FN_MINMAX]);
	if (NO_LEMMAS == 0) d2_printf2("lemmas: %lld; ", ite_counters[ERR_BT_LEMMA]);
	d2_printf1("\n");
	if (backjumping) d2_printf3(" Backjumps: %ld (avg bj len: %.1f)\n",
										 (long)ite_counters[NUM_TOTAL_BACKJUMPS],
										 (float)ite_counters[NUM_TOTAL_BACKJUMPS]/(1+ite_counters[NUM_BACKJUMPS]));
	if (autarky) d2_printf3(" Autarkies: %ld (avg au len: %.1f)\n",
									(long)ite_counters[NUM_TOTAL_AUTARKIES],
									(float)ite_counters[NUM_TOTAL_AUTARKIES]/(1+ite_counters[NUM_AUTARKIES]));
	if (max_solutions != 1) d2_printf3(" Solutions found: %ld/%ld\n",
												  (long)ite_counters[NUM_SOLUTIONS], (long)max_solutions);
	
	d2_printf1("\n");
}

void DisplaySimpleSolverBacktrackInfo_gnuplot(double &fSimpleSolverPrevEndTime, double &fSimpleSolverStartTime) {
	double fSimpleSolverEndTime = ite_counters_f[CURRENT_TIME];
	double fTotalDurationInSecs = fSimpleSolverEndTime - fSimpleSolverStartTime;
	double fDurationInSecs = fSimpleSolverEndTime - fSimpleSolverPrevEndTime;
	double fBacktracksPerSec = BACKTRACKS_PER_STAT_REPORT / (fDurationInSecs>0?fDurationInSecs:0.001);
	
	fprintf(fd_csv_trace_file, "%4.3f ", fTotalDurationInSecs);
	fprintf(fd_csv_trace_file, "%ld %4.3f ",
			  (long)ite_counters[NUM_BACKTRACKS], fBacktracksPerSec);
	
	int whereAmI = 0;
	int total = 0;
	double progress = 0.0;
	CalculateSimpleSolverProgress(&whereAmI, &total);
	if (total == 0) total=1;
	progress = ite_counters_f[PROGRESS] = (float)whereAmI*100/total;
	fprintf(fd_csv_trace_file, "%4.3f ", progress);
	
	fprintf(fd_csv_trace_file, "%lld %lld ", ite_counters[NUM_CHOICE_POINTS], ite_counters[HEU_DEP_VAR]);
	fprintf(fd_csv_trace_file, "%lld ", ite_counters[INF_SMURF]);
	fprintf(fd_csv_trace_file, "%lld ", ite_counters[ERR_BT_SMURF]);
	fprintf(fd_csv_trace_file, "%ld %ld\n", (long)ite_counters[NUM_SOLUTIONS], (long)max_solutions);
}
