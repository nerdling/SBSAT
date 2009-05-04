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

extern int solutions_overflow;

void DisplaySimpleStatistics(long long int nNumChoicePts, long long int nNumBacktracks, long long int nNumBackjumps) {
	d2_printf2("Choice Points: %lld", nNumChoicePts);
	d2_printf3(", Backtracks: %lld, Backjumps: %lld \n",
				  nNumBacktracks, nNumBackjumps);
}

void PrintAllSmurfStateEntries() {
   int state_num = 0;
   for(SmurfStatesTableStruct *pIter = SimpleSmurfProblemState->arrSmurfStatesTableHead; pIter != NULL;) {
      int x=0;
      while(1) {
         int size = arrStatesTypeSize[(unsigned char)((TypeStateEntry *)(((char *)pIter->arrStatesTable) + x))->cType];
         if(((TypeStateEntry *)(((char *)pIter->arrStatesTable) + x))->cType!=FN_FREE_STATE) {
            d9_printf3("State %d(%p), ", state_num++, (void *)(((char *)pIter->arrStatesTable) + x));
            arrPrintStateEntry[(unsigned char)((TypeStateEntry *)(((char *)pIter->arrStatesTable) + x))->cType]((void *)(((char *)pIter->arrStatesTable) + x));
         }
         x+=size;
         if(x>=SimpleSmurfProblemState->arrCurrSmurfStates->max_size) break;
      }
      pIter = pIter->pNext;
   }
}

void PrintSmurf_dot(void *pState) {
	if(((TypeStateEntry *)pState)->visited) return;
	((TypeStateEntry *)pState)->visited = 1;
	if(((SmurfStateEntry *)pState) == pTrueSimpleSmurfState) {
		
	} else if(((TypeStateEntry *)pState)->cType == FN_SMURF) {
		PrintSmurfStateEntry_dot(pState);
		fprintf(stdout, " b%p [shape=\"ellipse\", label=\"S\"]\n", (void *)pState);
		SmurfStateEntry *pSmurfState = (SmurfStateEntry *)pState;
		while(pSmurfState!=NULL) {
			PrintSmurf_dot(pSmurfState->pVarIsTrueTransition); //Recurse
			PrintSmurf_dot(pSmurfState->pVarIsFalseTransition); //Recurse
			pSmurfState = (SmurfStateEntry *)pSmurfState->pNextVarInThisState;
		}
	} else if(((TypeStateEntry *)pState)->cType == FN_INFERENCE) {
		PrintInferenceStateEntry_dot(pState);
		PrintSmurf_dot(((InferenceStateEntry *)pState)->pVarTransition); //Recurse
	} else {
		arrPrintStateEntry_dot[(int)((TypeStateEntry *)pState)->cType](pState);
	}
}

void PrintAllXORSmurfStateEntries() {
	void **arrSmurfStates = SimpleSmurfProblemState->arrSmurfStack[SimpleSmurfProblemState->nCurrSearchTreeLevel].arrSmurfStates;

	for(int i = 0; i < SimpleSmurfProblemState->nNumSmurfs; i++) {
		if(arrSmurfStates[i] == pTrueSimpleSmurfState) continue;
		void *pState = arrSmurfStates[i];
		if(((TypeStateEntry *)pState)->cType == FN_XOR_COUNTER) {
			PrintXORStateEntry_formatted((void *)((XORCounterStateEntry *)pState)->pXORState);
		} else if(((TypeStateEntry *)pState)->cType == FN_XOR) {
			PrintXORStateEntry_formatted(pState);
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

void DisplaySimpleSolverBacktrackInfo() {
	fSimpleSolverEndTime = ite_counters_f[CURRENT_TIME] = get_runtime();
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
	//d2_printf3("Progress: %p/%p        ", whereAmI, total);
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
	if(ite_counters[INF_SMURF]>0) d2_printf2("smurfs: %lld; ", ite_counters[INF_SMURF]);
	if(ite_counters[INF_SPEC_FN_OR]>0) d2_printf2("ORs: %lld; ", ite_counters[INF_SPEC_FN_OR]);
   if(ite_counters[INF_SPEC_FN_XOR]>0)	d2_printf2("XORs: %lld; ", ite_counters[INF_SPEC_FN_XOR]);
   if(ite_counters[INF_BB_GELIM]>0) d2_printf2("GELIM: %lld; ", ite_counters[INF_BB_GELIM]);
   if(ite_counters[INF_SPEC_FN_MINMAX]>0)	d2_printf2("MINMAXs: %lld; ", ite_counters[INF_SPEC_FN_MINMAX]);
   if(ite_counters[INF_SPEC_FN_NEGMINMAX]>0)	d2_printf2("NEGMINMAXs: %lld; ", ite_counters[INF_SPEC_FN_NEGMINMAX]);
	if(ite_counters[INF_LEMMA]>0) d2_printf2("lemmas: %lld; ", ite_counters[INF_LEMMA]);
	d2_printf1("\n");
	
	d2_printf1(" Backtracks by ");
	if(ite_counters[ERR_BT_SMURF]>0) d2_printf2("smurfs: %lld; ", ite_counters[ERR_BT_SMURF]);
	if(ite_counters[ERR_BT_SPEC_FN_OR]>0) d2_printf2("ORs: %lld; ", ite_counters[ERR_BT_SPEC_FN_OR]);
	if(ite_counters[ERR_BT_SPEC_FN_XOR]>0) d2_printf2("XORs: %lld; ", ite_counters[ERR_BT_SPEC_FN_XOR]);
   if(ite_counters[ERR_BT_BB_GELIM]>0) d2_printf2("GELIM: %lld; ", ite_counters[ERR_BT_BB_GELIM]);
	if(ite_counters[ERR_BT_SPEC_FN_MINMAX]>0) d2_printf2("MINMAXs: %lld; ", ite_counters[ERR_BT_SPEC_FN_MINMAX]);
   if(ite_counters[ERR_BT_SPEC_FN_NEGMINMAX]>0) d2_printf2("NEGMINMAXs: %lld; ", ite_counters[ERR_BT_SPEC_FN_NEGMINMAX]);
	if(ite_counters[ERR_BT_LEMMA]>0) d2_printf2("lemmas: %lld; ", ite_counters[ERR_BT_LEMMA]);
	d2_printf1("\n");
	if (backjumping) d2_printf3(" Backjumps: %ld (avg bj len: %.1f)\n",
										 (long)ite_counters[NUM_TOTAL_BACKJUMPS],
										 (float)ite_counters[NUM_TOTAL_BACKJUMPS]/(1+ite_counters[NUM_BACKJUMPS]));
	if (autarky) d2_printf3(" Autarkies: %ld (avg au len: %.1f)\n",
									(long)ite_counters[NUM_TOTAL_AUTARKIES],
									(float)ite_counters[NUM_TOTAL_AUTARKIES]/(1+ite_counters[NUM_AUTARKIES]));
	if (max_solutions != 1 && solutions_overflow==0) { d2_printf3(" Solutions found: %lld/%lld\n", ite_counters[NUM_SOLUTIONS], max_solutions); }
	else if(solutions_overflow) d2_printf2(" Solutions found: > %lld\n", ~(((long long)1)<<(long long)63));
	
	d2_printf1("\n");
}

void DisplaySimpleSolverBacktrackInfo_gnuplot() {
	fSimpleSolverEndTime = ite_counters_f[CURRENT_TIME];
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
	fprintf(fd_csv_trace_file, "%lld ", ite_counters[NUM_INFERENCES]);
	fprintf(fd_csv_trace_file, "%lld ", ite_counters[NUM_BACKTRACKS]);
	fprintf(fd_csv_trace_file, "%ld %ld\n", (long)ite_counters[NUM_SOLUTIONS], (long)max_solutions);
}
