/* =========FOR INTERNAL USE ONLY. NO DISTRIBUTION PLEASE ========== */

/****************************************************************************************[Solver.C]
 * SBSAT -- Copyright (c) 2008, Sean Weaver.
 * Built upon VarSat code by Eric Hsu.
 * Their original copyright notice is repeated below.
 * SBSAT's standard copyright notice is also stated below.
 *
 * VarSat -- Copyright (c) 2008, Eric Hsu.
 * Built upon MiniSat code by Niklas Een and Niklas Sorensson.
 * Their original copyright notice is repeated below.
 * 
 * MiniSat -- Copyright (c) 2003-2006, Niklas Een, Niklas Sorensson
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
 * associated documentation files (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge, publish, distribute,
 * sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT
 * NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT
 * OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
**************************************************************************************************/

/*********************************************************************
 Copyright 1999-2008, University of Cincinnati.  All rights reserved.
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
 any trademark, service mark, or the name of University of Cincinnati.


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

int MAXITERATIONS = 100;
double EPSILON = 0.001;
int MP_VARS_TO_SET = 0;

#define heuristic_BP 0
#define heuristic_SP 1
#define heuristic_EMBPL 2
#define heuristic_EMBPG 3
#define heuristic_EMSPL 4
#define heuristic_EMSPG 5
#define heuristic_EMSPGV2 6
#define heuristic_EMBPGV2 7

extern int random_seed;
double mp_seed;
void clear_all_bdd_density();

int surveys_attempted = 0;
int surveys_converged = 0;

//Returns a random float 0<=x<1. Seed must never be 0.
inline double drand() {
	mp_seed *= 1389796;
	int q = (int)(mp_seed / 2147483647);
	mp_seed -= (double)q * 2147483647;
	return mp_seed / 2147483647;
}

int ComputeSurvey(int);

double *pos_biases, *neg_biases, *star_biases;

//SCOREVARSKEW: Score a survey variable by the difference between its positive and negative readings.
double scoreVarSkew(int v) {
	return fabs(pos_biases[v] - neg_biases[v]);
}

// REPORTBIASES: Report biases for all the variables.
void reportBiases() {
	int v;
	d4_printf1("==============================[ Survey Report ]================================\n");
	d4_printf1("     Variable       Pos       Neg       Star\n");
	for (v = 1; v <= numinp; v++) {
		if(variablelist[v].true_false != -1 || variablelist[v].equalvars != 0) {			  
				d4_printf6(" %8d Fix: %2d (%1.3f)   (%1.3f)   (%1.3f)\n",
							  v, variablelist[v].true_false, pos_biases[v], neg_biases[v], star_biases[v]);
		} else {
			d4_printf5(" %8d  %1.3f     %1.3f     %1.3f\n",
						  v, pos_biases[v], neg_biases[v], star_biases[v]);
		}
	}
}

//INITIALIZEBIASES: Seed biases with random values.
void initializeBiases(int heuristic_mode) {
	int i;
	double pos, neg, star, sum;

	for(i = 1; i <= numinp; i++) {
		if(variablelist[i].true_false != -1 || variablelist[i].equalvars != 0)
		  continue;
		
		//Initialize pos and neg biases if we are using a BP-type heuristic.
		if ((heuristic_mode == heuristic_BP) ||
			 (heuristic_mode == heuristic_EMBPL) ||
			 (heuristic_mode == heuristic_EMBPG) ||
			 (heuristic_mode == heuristic_EMBPGV2)) {
			pos = drand(); neg = drand();
			sum = pos + neg;
			pos_biases[i] = pos / sum;
			neg_biases[i] = 1.0 - pos_biases[i];
			star_biases[i] = 0.0; //Not used
		} else if ((heuristic_mode == heuristic_SP) ||
					  (heuristic_mode == heuristic_EMSPL) ||
					  (heuristic_mode == heuristic_EMSPG) ||
					  (heuristic_mode == heuristic_EMSPGV2)) {
			pos = drand(); neg = drand(); star = drand();
			sum = pos + neg + star;
			pos_biases[i] = pos / sum;
			neg_biases[i] = neg / sum;
			star_biases[i] = 1.0 - pos_biases[i] - neg_biases[i];			
		}
	}
}

int Do_Message_Passing() {
	d3_printf1 ("MESSAGE PASSING - ");
	int ret = PREP_NO_CHANGE;
	char p[100];
	int v;
	int next = 1;
	int sign = 0;

	D_3(
		 sprintf(p, "{0:0/%ld}", numinp);
		 str_length = strlen(p);
		 d3_printf1(p);
	);

	mp_seed = (double)random_seed;
	int heuristic_mode = heuristic_SP; //SEAN!!! Make this a command line option

	MP_VARS_TO_SET = numinp; //SEAN!!! Make this a command line option
	
	surveys_attempted = 0;
	surveys_converged = 0;
	
	pos_biases = (double *)ite_calloc(numinp+1, sizeof(double), 9, "pos_biases");
	neg_biases = (double *)ite_calloc(numinp+1, sizeof(double), 9, "neg_biases");
	star_biases = (double *)ite_calloc(numinp+1, sizeof(double), 9, "star_biases");
	
	int surveys = 0;

	int converged = 1;
	while (converged == 1 && surveys_converged < MP_VARS_TO_SET) {
		initializeBiases(heuristic_mode);
		converged = ComputeSurvey(heuristic_mode);
		surveys++;
		if(converged == 1)
		  ret = PREP_CHANGED;
		
		double maxscore = 0.0;
		int var_found = 0;
		
		for(v = numinp; v > 0; v--) {
			//skip vars already assigned or quantified away
			if(variablelist[v].true_false != -1 || variablelist[v].equalvars != 0)
			  continue;
			var_found = 1;
			if (scoreVarSkew(v) >= maxscore) {
				next = v;
				maxscore = scoreVarSkew(v);
			}
		}
		if(var_found == 0) break;
		
		sign = (pos_biases[next] > neg_biases[next]);
		
		if(converged == 1) {
			BDDNode *inferBDD = sign?ite_var(next):ite_var(-next);
			int bdd_length = 0;
			int *bdd_vars = NULL;
			switch (int r=Rebuild_BDD(inferBDD, &bdd_length, bdd_vars)) {
			  case TRIV_UNSAT:
			  case TRIV_SAT:
			  case PREP_ERROR: return r;
			  default: break;				
			}

			switch (int r=Do_Apply_Inferences()) {
			  case TRIV_UNSAT:
			  case TRIV_SAT:
			  case PREP_ERROR: return r;
			  default: break;
			}
			
			delete [] bdd_vars;
			bdd_vars = NULL;
		}
		
		D_3(
			 sprintf(p, "{0:%d/%ld}", surveys, numinp);
			 str_length = strlen(p);
			 d3_printf1(p);
			 );
		if(nCtrlC) {
			d3_printf1("Breaking out of Message Passing\n");
			ret = PREP_NO_CHANGE;
			nCtrlC = 0;
			break;
		}
	}

	ite_free((void **)&pos_biases);
	ite_free((void **)&neg_biases);
	ite_free((void **)&star_biases);
	
	d3_printf1 ("\n");
	d2e_printf1 ("\r                  ");
	return ret;
}

double _calculateNeed(BDDNode *f) {
	if(f == true_ptr) return 0.0;
	if(f == false_ptr) return 1.0;
	double r = _calculateNeed(f->thenCase);
	double e = _calculateNeed(f->elseCase);
	return f->tbr_weight = (r*pos_biases[f->variable]) + (e*neg_biases[f->variable]);
}

double calculateNeed(BDDNode *f, int v, int pos) {
	BDDNode *vBDD = set_variable (f, v, pos);
	return _calculateNeed(vBDD);
}

// CALCULATEFREEDOM: Calculate probability (by _product_) that no BDD in the given list needs variable v;
// as a SIDE EFFECT, store the number of active BDDs in the list, in given address.  This corresponds to the
// product over the clauses of 1 - sigma.
double calculateFreedom(int v, int pos, int &count) {
	double retval = 1.0;
	int retcount = 0;
	
	int i;
	BDDNode *f;
	
	// Loop through the BDDs.
	for(llist *k = amount[v].head; k != NULL; k = k->next) {
		f = functions[k->num];
		  
		double need = calculateNeed(f, v, pos);
		if(need != 0.0) {
			retcount++;
			// Update product of probabilities.  Remember that we want the prob that the BDD
			// _doesn't_ need the var's help, so we subtract the running product from one.
			retval *= 1.0 - need;
		}
	}
	
	count = retcount;  // SIDE EFFECT
	return retval;
}

// CALCULATERESTRICTION: Calculate _sum_ of probabilities that the BDDs in the given list need variable v;
// as a SIDE EFFECT, store the number of active BDDs in the list, in given address.  This corresponds to the
// sum of sigmas over all the BDDs.
double calculateRestriction(int v, int pos, int &count) {
	double retval = 0.0;
	int retcount = 0;

	int i;
	BDDNode *f;
	
	// Loop through the BDDs.
	for(llist *k = amount[v].head; k != NULL; k = k->next) {
		f = functions[k->num];
		  
		double need = calculateNeed(f, v, pos);
		if(need != 0.0) {
			retcount++;
			// Update product of probabilities.  Remember that we want the prob that the BDD
			// _doesn't_ need the var's help, so we subtract the running sum from one.
			retval += 1.0 - need;
		}
	}
	
	count = retcount;  // SIDE EFFECT
	return retval;
}

// DETERMINERATIOBP: Update a two-state variable's bias by given weights; return delta to test for convergence.
// (Check boundary conditions via nneg and npos.)
double determineRatioBP(int v, int nneg, int npos, double wplus, double wminus) {
	double old_pos; // for figuring out how much the bias changed
	double new_pos; // caches a temporary value to avoid recalculation
	
	// Actually with the new code, I think boundary conditions can't happen.
	new_pos = wplus / (wplus + wminus);
	
	bool lacks_pos_BDD = (npos == 0) ? true : false;   // for handling boundary conditions
	bool lacks_neg_BDD = (nneg == 0) ? true : false;
	
	// Handle boundary conditions, or else actually determine the bias by ratio in their absence.
	if (wplus + wminus == 0.0) {
		// Prevents divide-by-zero's; this usually would also invoke lacks_pos_BDD and lacks_neg_BDD,
		// but might also result from pure neighbor literals that themselves lack certain polarities of BDDs.
		wplus = 1.0; wminus = 1.0;
		//d4_printf1("Hey there was a divide by zero problem with EM.\nExiting.\n"); exit(-1);
	}
	
	if (lacks_pos_BDD && lacks_neg_BDD) {
		// Technically this case is not handled by BP, but we know that half the solutions have v set
		// positively and the other half have it set negatively.
		new_pos = 0.5;
	} else if (lacks_pos_BDD) {
		// By BP's assumption that every var is a sole support, we must conclude v is constrained to be neg.
		new_pos = 0.0;
	} else if (lacks_neg_BDD) {
		// By BP's assumption that every var is a sole support, we must conclude v is constrained to be pos.
		new_pos = 1.0;
	} else {
		// In the absence of a boundary condition, the variable must balance its responsibilities to both
		// positive and negative BDDs.  So, to do a regular update, the variable's new positive bias is:
		// wplus / (wplus + wminus).  That is, the probability that it must be positive is estimated as the
		// probability that it doesn't have to be negative, normalized.  And bcs we are just doing the
		// two-state, BP version of things, the chance of being negative is just 1 minus that.
		new_pos = wplus / (wplus + wminus);
	}

	// Store the resulting values.
	old_pos = pos_biases[v];
	pos_biases[v] = new_pos;
	neg_biases[v] = 1.0 - new_pos;
	
	// Finally, return a float measuring how much this variable actually moved.
	return fabs(old_pos - new_pos);
}

// DETERMINERATIOSP: Update a three-state variable's bias by given weights; return delta to test for convergence.
// (Check boundary conditions via nneg and npos.)
double determineRatioSP(int v, int nneg, int npos, double wplus, double wminus, double wstar) {
	double old_pos; // for figuring out how much the bias changed
	double old_neg;
	double new_pos; // caches a temporary value to avoid recalculation
	double new_neg;
	
	//d4_printf5("DETERMINERATIOSP(%d): %f %f %f \n", v + 1, wplus, wminus, wstar);
	
	// Actually with the new code, I think boundary conditions can't happen.
	new_pos = wplus / (wplus + wminus + wstar);
	new_neg = wminus / (wplus + wminus + wstar);

	
	bool lacks_pos_BDD = (npos == 0) ? true : false;   // for handling boundary conditions
	bool lacks_neg_BDD = (nneg == 0) ? true : false;
	
	// Handle boundary conditions, or else actually determine the bias by ratio in their absence.
	if (wplus + wminus + wstar == 0.0) {
		// Prevents divide-by-zero's; this usually would also invoke lacks_pos_BDD and lacks_neg_BDD,
		// but might also result from pure neighbor literals that themselves lack certain polarities of BDDs.
		wplus = 1.0; wminus = 1.0; wstar = 1.0;
		//d4_printf1("Hey there was a divide by zero problem with EM.\nExiting.\n"); exit(-1);
	}
	
	if (lacks_pos_BDD && lacks_neg_BDD) {
		//d4_printf1("case1");
		// Variable is unconstrained, put all its weight on star.
		new_pos = 0.0;
		new_neg = 0.0;
	} else if (lacks_pos_BDD) {
		//d4_printf1("case2");
		// Variable is either constrained negative, or it's star.  FIXTHIS: can divide-by-zero happen?
		new_pos = 0.0;
		new_neg = wminus / (wminus + wstar);
	} else if (lacks_neg_BDD) {
		// Variable is either constrained positive, or it's star.  FIXTHIS: can divide-by-zero happen?
		//d4_printf1("case3");
		new_pos = wplus / (wplus + wstar);
		new_neg = 0.0;
	} else {
		//d4_printf1("case4");
		// In the absence of a boundary condition, the variable must balance its three responsibilities
		// by normalizing over its weights.  Because we are doing three states for SP, we can just
		// determine the chances of being positive and negative; the chances of being star are just 1 minus
		// these two quantities.
		new_pos = wplus / (wplus + wminus + wstar);
		new_neg = wminus / (wplus + wminus + wstar);
	}

	//d4_printf4(":%f %f %f\n", new_pos, new_neg, 1.0 - new_pos - new_neg);
	// Store the resulting values.
	old_pos = pos_biases[v];
	old_neg = neg_biases[v];
	pos_biases[v] = new_pos;
	neg_biases[v] = new_neg;
	star_biases[v] = 1.0 - new_pos - new_neg;
	
	// Finally, return a float measuring how much this variable actually moved.
	return fabs(old_pos - new_pos) + fabs(old_neg - new_neg);
}

// UPDATEBP: Update a variable according to BP.
double updateBP(int v) {
	int nneg, npos;  //number of active negative and positive BDDs for v.
	  double alpha = calculateFreedom(v, 1, nneg);
	double beta = calculateFreedom(v, 0, npos);
	
	return determineRatioBP(v, nneg, npos, alpha, beta);
}

// UPDATEEMBPL: Update a variable accoring to EMBP-L.
double updateEMBPL(int v) {
	int nneg, npos;  // number of active negative and positive BDDs for v.
	double sum_alpha = calculateRestriction(v, 1, nneg);
	double sum_beta = calculateRestriction(v, 0, npos);
	
	double c = double(nneg + npos);
	return determineRatioBP(v, nneg, npos, c - sum_alpha, c - sum_beta);
}

// UPDATEEMBPG: Update a variable accoring to EMBP-G.
double updateEMBPG(int v) {
	int nneg, npos;  // number of active negative and positive BDDs for v.
	double alpha = calculateFreedom(v, 1, nneg);
	double beta = calculateFreedom(v, 0, npos);
	
	return determineRatioBP(v, nneg, npos, (double)nneg * alpha + (double)npos, (double)npos * beta + (double)nneg);
}

// UPDATEEMBPGV2: Update a variable accoring to EMBP-G-V2.
double updateEMBPGV2(int v) {
	int nneg, npos;  // number of active negative and positive BDDs for v.
	double alpha = calculateFreedom(v, 1, nneg);
	double beta = calculateFreedom(v, 0, npos);
	
	double c = double(nneg + npos);
	return determineRatioBP(v, nneg, npos, c * alpha, c * beta);
}

// UPDATESP: Update a variable according to SP.
double updateSP(int v) {
	int nneg, npos;    // number of active negative and positive BDDs for v.
	double rho = 0;//0.95; // important to prevent the trivial core, as in for instance, Maneva '06
	
	double alpha = calculateFreedom(v, 1, nneg);
	double beta = calculateFreedom(v, 0, npos);
	
	double prod = alpha * beta;
	return determineRatioSP(v, nneg, npos, alpha - rho * prod, beta - rho * prod, prod);
}

// UPDATEEMSPL: Update a variable accoring to EMSP-L.
double updateEMSPL(int v) {
	int nneg, npos;  // number of active negative and positive BDDs for v.
	double sum_alpha = calculateRestriction(v, 1, nneg);
	double sum_beta = calculateRestriction(v, 0, npos);
	
	double c = double(nneg + npos);
	return determineRatioSP(v, nneg, npos, c - sum_alpha, c - sum_beta, c - sum_alpha - sum_beta);
}

// UPDATEEMSPG: Update a variable accoring to EMSP-G.
double updateEMSPG(int v) {
	int nneg, npos;  // number of active negative and positive BDDs for v.
	double alpha = calculateFreedom(v, 1, nneg);
	double beta = calculateFreedom(v, 0, npos);
	
	return determineRatioSP(v, nneg, npos, (double)nneg * alpha + (double)npos * (1.0 - beta),
									(double)npos * beta + (double)nneg * (1.0 - alpha),
									(double)(npos + nneg) * alpha * beta);
}

// UPDATEEMSPGV2: Update a variable accoring to EMSP-G-V2.
double updateEMSPGV2(int v) {
	int nneg, npos;      // number of active negative and positive BDDs for v.
	
	double alpha = calculateFreedom(v, 1, nneg);
	double beta = calculateFreedom(v, 0, npos);
	
	double prod = alpha * beta;
	double c = double(nneg + npos);
	return determineRatioSP(v, nneg, npos, c * (alpha - prod), c * (beta - prod), c * prod);
}

int ComputeSurvey (int heuristic_mode) {
	double max_change = 0.0;
	double change = 0.0;
	int iter = 0;
	int v = 0;
	
	do {
		d3_printf1(".");
		max_change = 0.0;
		reportBiases();
		for (v = 1; v <= numinp; v++) {
			// clear_all_bdd_density();
			if(variablelist[v].true_false != -1 || variablelist[v].equalvars != 0)
			  continue;
			
			//Update each unfixed variable according to the appropriate rule.
			switch (heuristic_mode) {
			  case heuristic_BP: change = updateBP(v); break;
			  case heuristic_EMBPL: change = updateEMBPL(v); break;
			  case heuristic_EMBPG: change = updateEMBPG(v); break;
			  case heuristic_EMBPGV2: change = updateEMBPGV2(v); break;
			  case heuristic_SP: change = updateSP(v); break;
			  case heuristic_EMSPL: change = updateEMSPL(v); break;
			  case heuristic_EMSPG: change = updateEMSPG(v); break;
			  case heuristic_EMSPGV2: change = updateEMSPGV2(v); break;
			  default: assert(0);
			}
			
			//Keep track of maximal change in order to test for convergence.
			if (change > max_change) max_change = change;
		}
	} while (max_change > EPSILON && iter++ < MAXITERATIONS);
	
	//Update stats, print log messages, and return flag according to convergence or timeout.
	surveys_attempted++;
	if(max_change <= EPSILON) {
		surveys_converged++;
		d3_printf1(":-)\n");
		D_4(reportBiases(););
		return 1;
	} else {
		d3_printf2("[%f]:-(\n", max_change);
		D_4(reportBiases(););
		return 0;		
	}	
}
