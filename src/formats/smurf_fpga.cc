#include "sbsat.h"
#include "sbsat_formats.h"
#include "sbsat_solver.h"

extern SmurfStateEntry *TrueSimpleSmurfState;
extern ProblemState *SimpleSmurfProblemState;
extern int smurfs_share_paths;

// convert a heuristic value from a 'double'
// (assumed to be in the range 0.0 thru MAX_HEURISTIC_VALUE)
// to a binary value with NUM_BINARY_DIGITS bits,
// and print that binary value out in big-endian ASCII notation.
static void print_heuristic_value(FILE * foutputfile, double heuristic_weight) {
	const double MAX_HEURISTIC_VALUE = 1000;
	const double NUM_BINARY_DIGITS = 18;
	fputc('\"', foutputfile);
	for (int i = 0; i < NUM_BINARY_DIGITS; i++) {
		heuristic_weight *= 2;
		if (heuristic_weight > MAX_HEURISTIC_VALUE) {
			putc('1', foutputfile);
			heuristic_weight -= MAX_HEURISTIC_VALUE;
		} else {
			putc('0', foutputfile);
		}
	}
	fputc('"', foutputfile);
}

int MaxStatesPerSmurf() {
	int *arrSmurfStates = SimpleSmurfProblemState->arrSmurfStack[0].arrSmurfStates;

	int numStates = 0;
	int maxStates = SimpleSmurfProblemState->nNumSmurfStateEntries - arrSmurfStates[SimpleSmurfProblemState->nNumSmurfs-1];
	for(int y = 0; y < SimpleSmurfProblemState->nNumSmurfs-1; y++) {
		numStates = arrSmurfStates[y+1] - arrSmurfStates[y];
		if(numStates > maxStates) maxStates = numStates;
	}
	return maxStates;
}

void MakeStatesLoop() {
	for(int y = 0; y < SimpleSmurfProblemState->nNumSmurfStateEntries; y++) {
		if(SimpleSmurfProblemState->arrSmurfStatesTable[y].nVarIsAnInference>0) {
			SimpleSmurfProblemState->arrSmurfStatesTable[y].nNextVarInThisState = 
			  SimpleSmurfProblemState->arrSmurfStatesTable[y].nVarIsTrueTransition;
			continue;
		} else if(SimpleSmurfProblemState->arrSmurfStatesTable[y].nVarIsAnInference<0) {
			SimpleSmurfProblemState->arrSmurfStatesTable[y].nNextVarInThisState = 
			  SimpleSmurfProblemState->arrSmurfStatesTable[y].nVarIsFalseTransition;
			continue;
		}
		SmurfStateEntry *SmurfState;
		int nStateNum = y;
		for(SmurfState = &(SimpleSmurfProblemState->arrSmurfStatesTable[y]); SmurfState->nNextVarInThisState > nStateNum; SmurfState = &(SimpleSmurfProblemState->arrSmurfStatesTable[nStateNum])) {
			assert(SmurfState->nVarIsAnInference==0);
			nStateNum = SmurfState->nNextVarInThisState;
		}
		if(SmurfState->nNextVarInThisState == 0)
		  SmurfState->nNextVarInThisState = y;
	}
}

void Smurf_FPGA() {
	int ssp_old = smurfs_share_paths;
	smurfs_share_paths = 0;
	int ret = Init_SimpleSmurfSolver();
	if(ret != SOLV_UNKNOWN) return;
	
	MakeStatesLoop();
	
	char *p = (char *)calloc(8192, sizeof(char));
	
	fprintf(foutputfile, "-- Note: the SAT_problem_constraints and SAT_problem_tables packages\n");
	fprintf(foutputfile, "-- are problem-specific and were automatically generated by SBSAT.\n");
	fprintf(foutputfile, "\n");
	fprintf(foutputfile, "library ieee;\n");
	fprintf(foutputfile, "use ieee.std_logic_1164.all;\n");
	//fprintf(foutputfile, "use work.SAT_problem_infrastructure.all;\n");
	fprintf(foutputfile, "\n");
	
	fprintf(foutputfile, "package SAT_problem_constants is\n");
	fprintf(foutputfile, "  constant num_vars : natural := %d;\n", SimpleSmurfProblemState->nNumVars);
	fprintf(foutputfile, "  constant num_SMURFS : natural := %d;\n", SimpleSmurfProblemState->nNumSmurfs);
	fprintf(foutputfile, "  constant num_states : natural := %d;  -- maximum states per SMURF\n", MaxStatesPerSmurf());
	fprintf(foutputfile, "end package SAT_problem_constants;\n");
	fprintf(foutputfile, "\n");

	fprintf(foutputfile, "library ieee;\n");
	fprintf(foutputfile, "use ieee.std_logic_1164.all;\n");
	fprintf(foutputfile, "use work.SAT_problem_infrastructure.all;\n");
	fprintf(foutputfile, "package SAT_problem_tables is\n");
	fprintf(foutputfile, "\n");
	fprintf(foutputfile, "  constant ST : SMURF_state_num := TrueState;\n");
	fprintf(foutputfile, "\n");
	fprintf(foutputfile, "  constant SMURF_input_vars : input_vars_sets :=\n");
	fprintf(foutputfile, "   (");

	for(int nSmurfIndex = 0; nSmurfIndex < SimpleSmurfProblemState->nNumSmurfs; nSmurfIndex++) {
		if(nSmurfIndex == 0)
		  fprintf(foutputfile, "%d => (", nSmurfIndex);
		else 
		  fprintf(foutputfile, "    %d => (", nSmurfIndex);
	
		int *arrSmurfStates = SimpleSmurfProblemState->arrSmurfStack[0].arrSmurfStates;
		int next_transition = 1;
		SmurfStateEntry *SmurfState;
		for(SmurfState = &(SimpleSmurfProblemState->arrSmurfStatesTable[arrSmurfStates[nSmurfIndex]]); SmurfState->nNextVarInThisState > next_transition; SmurfState = &(SimpleSmurfProblemState->arrSmurfStatesTable[next_transition])) {
			next_transition = SmurfState->nNextVarInThisState;
			fprintf(foutputfile, "%d => '1', ", SmurfState->nTransitionVar);
		}
		fprintf(foutputfile, "%d => '1', ", SmurfState->nTransitionVar);
		if(nSmurfIndex < SimpleSmurfProblemState->nNumSmurfs-1)
		  fprintf(foutputfile, "others => '0'),\n");
		else
		  fprintf(foutputfile, "others => '0')\n");
	}
	fprintf(foutputfile, "   );\n");
	fprintf(foutputfile, "\n");
	fprintf(foutputfile, "  constant SMURF_transition_tables : transition_tables :=\n");
	fprintf(foutputfile, "   -- The fields are as follows:\n");
	fprintf(foutputfile, "   -- transition_var : var_range;\n");
	fprintf(foutputfile, "   -- pos_transition : SMURF_state_num;\n");
	fprintf(foutputfile, "   -- neg_transition : SMURF_state_num;\n");
	fprintf(foutputfile, "   -- is_pos_inference : std_logic;\n");
	fprintf(foutputfile, "   -- is_neg_inference : std_logic;\n");
	fprintf(foutputfile, "   -- pos_heur_weight : heur_weight;\n");
	fprintf(foutputfile, "   -- neg_heur_weight : heur_weight;\n");
	fprintf(foutputfile, "   -- next_var_state : SMURF_state_num;\n");
	fprintf(foutputfile, "\n");
	
	for(int nSmurfIndex = 0; nSmurfIndex < SimpleSmurfProblemState->nNumSmurfs; nSmurfIndex++) {
		if(nSmurfIndex == 0)
		  fprintf(foutputfile, "   (%d =>\n    (", nSmurfIndex);
		else
		  fprintf(foutputfile, "   %d =>\n    (", nSmurfIndex);
		int *arrSmurfStates = SimpleSmurfProblemState->arrSmurfStack[0].arrSmurfStates;
		int start_state = arrSmurfStates[nSmurfIndex];
		int stop_at;
		if(nSmurfIndex+1 == SimpleSmurfProblemState->nNumSmurfs)
		  stop_at = SimpleSmurfProblemState->nNumSmurfStateEntries;
		else stop_at = arrSmurfStates[nSmurfIndex+1];
		for(int y = start_state; y < stop_at; y++) {
			SmurfStateEntry *SmurfState = &(SimpleSmurfProblemState->arrSmurfStatesTable[y]);
			//transition variable
			fprintf(foutputfile, "%d => (%d, ", y-start_state, SmurfState->nTransitionVar);
			if(SmurfState->nVarIsAnInference < 0)
			  fprintf(foutputfile, "0, 0, '0', '1', ");
			else if(SmurfState->nVarIsAnInference > 0)
			  fprintf(foutputfile, "0, 0, '1', '0', ");
			else {
				//positive transition state
				if(SmurfState->nVarIsTrueTransition == 1)
				  fprintf(foutputfile, "ST, ");
				else fprintf(foutputfile, "%d, ", SmurfState->nVarIsTrueTransition-start_state);
				//negative transition state
				if(SmurfState->nVarIsFalseTransition == 1)
				  fprintf(foutputfile, "ST, ");
				else fprintf(foutputfile, "%d, ", SmurfState->nVarIsFalseTransition-start_state);
				fprintf(foutputfile, "'0', '0', ");
			}
			//Positive Heuristic
			print_heuristic_value(foutputfile, SmurfState->nHeurWghtofTrueTransition);
			fprintf(foutputfile, ", ");
			//Negative Heuristic
			print_heuristic_value(foutputfile, SmurfState->nHeurWghtofFalseTransition);
			fprintf(foutputfile, ", ");
			//next var state
			if(SmurfState->nNextVarInThisState == 1)
			  fprintf(foutputfile, "ST)");
			else fprintf(foutputfile, "%d)", SmurfState->nNextVarInThisState-start_state);
			//if(y != stop_at-1)
			fprintf(foutputfile, ",\n     ");
		}
		fprintf(foutputfile, "others => (0, 0, 0, '0', '0', (others => '0'), (others => '0'), 0)\n");
		fprintf(foutputfile, "    ");
		if(nSmurfIndex+1 == SimpleSmurfProblemState->nNumSmurfs)
		  fprintf(foutputfile, ")\n");
		else fprintf(foutputfile, "),\n\n");
	}
	fprintf(foutputfile, "   );\n");
	fprintf(foutputfile, "\n");
	fprintf(foutputfile, "end SAT_problem_tables;\n");	

	free(p);
	smurfs_share_paths = ssp_old;
}
