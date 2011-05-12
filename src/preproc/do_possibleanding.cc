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


int PossibleAnding();

int Do_PossibleAnding() {
	MAX_EXQUANTIFY_CLAUSES += 1005;
	MAX_EXQUANTIFY_VARLENGTH += 5;
	dX_printf(3, "POSSIBLE ANDING - ");
	int cofs = PREP_CHANGED;
	int ret = PREP_NO_CHANGE;
	affected = 0;
	char p[100];
    sprintf(p, "{0:0/%ld}", numinp);
    str_length = dX_printf(3, p);
	while (cofs!=PREP_NO_CHANGE) {
		cofs = PossibleAnding ();
		if(cofs == PREP_CHANGED) ret = PREP_CHANGED;
		else if(cofs == TRIV_UNSAT) {
			return TRIV_UNSAT;
		}
		cofs = ret = PREP_NO_CHANGE; //SEAN
	}
	dX_printf(3, "\n");
	d2e_printf1 ("\r                                      ");
	return ret;
}

int PossibleAnding () {
	int ret = PREP_NO_CHANGE;
	
	BDDNode *Quantify;
	
	if (enable_gc) bdd_gc(); //Hit it!
	
	for (int i = 1; i < numinp + 1; i++) {
		char p[100];

        if (i % ((numinp/100)+1) == 0) {
            for(int iter = 0; iter<str_length; iter++)
                dX_printf(3, "\b");
            sprintf(p, "{%ld:%d/%ld}", affected, i, numinp);
            str_length = dX_printf(3, p);
        }
		if (nCtrlC) {
			dX_printf(3, "Breaking out of Possible Anding\n");
			ret = PREP_NO_CHANGE;
			nCtrlC = 0;
			goto ea_bailout;
		}
		
		if (i % ((numinp/100)+1) == 0) {
			d2e_printf3("\rPreprocessing Pa %d/%ld ", i, numinp);
		}

		if(variablelist[i].true_false != -1 || variablelist[i].equalvars != 0)
		  continue;
		//fprintf(stderr, "%d\n", i);

		int amount_count = 0;		
		for (llist * k = amount[i].head; k != NULL; k = k->next)
		  amount_count++;

		if (1) {//amount_count < MAX_EXQUANTIFY_CLAUSES){
			if(amount[i].head == NULL) continue; //Collected that variable
			int j = amount[i].head->num;
			Quantify = strip_x(j, i);
			//Quantify = functions[j];
			int out = 0;
			int cheat[1000];
			cheat[0] = j;
			
			int bdd_length = 0;
			int *bdd_vars = NULL;
			switch (int r=Rebuild_BDD(Quantify, &bdd_length, bdd_vars)) {
			 case TRIV_UNSAT:
			 case TRIV_SAT:
			 case PREP_ERROR: return r;
			 default: break;
			}
			delete [] bdd_vars;
			bdd_vars = NULL;
			if(bdd_length>MAX_EXQUANTIFY_VARLENGTH) out = 1;
			
			DO_INFERENCES = 0;
			int count1 = 0;
			affected++;
			for (llist * k = amount[i].head; k != NULL && out==0;) {
				int z = k->num;
				count1++;
				cheat[count1] = z;
				k = k->next;

                for(int iter = 0; iter<str_length; iter++)
                dX_printf(3, "\b");
                sprintf(p, "(%d:%d/%d[%d])",i, count1, amount_count, countBDDs());
                str_length = dX_printf(3, p);
				if (nCtrlC) {
					out = 1;
					break;
				}

				int bdd_length = 0;
				int *bdd_vars = NULL;
				switch (int r=Rebuild_BDD(Quantify, &bdd_length, bdd_vars)) {
				 case TRIV_UNSAT:
				 case TRIV_SAT:
				 case PREP_ERROR: return r;
				 default: break;
				}
				delete [] bdd_vars;
				bdd_vars = NULL;
				if(bdd_length>MAX_EXQUANTIFY_VARLENGTH) {
					out = 1;
					affected--;
				   break;
				}

				BDDNode *striped_z = strip_x(z, i);
				UnSetRepeats(z);
				equalityVble[z] = 0;
				functionType[z] = UNSURE;

				Quantify = ite_and(Quantify, striped_z);
				//Quantify = ite_and(Quantify, functions[z]);
				//affected++;
				ret = PREP_CHANGED;
			}
			if(amount_count != 1) {
                for(int iter = 0; iter<str_length; iter++)
                    dX_printf(3, "\b");
                sprintf(p, "(%d:%d/%d[%d])",i, count1, amount_count, countBDDs());
                str_length = dX_printf(3, p);
			}
			if(out == 0 && xquantify(Quantify, i) == true_ptr) {
				DO_INFERENCES = 1;
				functions[j] = ite_and(functions[j], xquantify(Quantify, i));
				switch (int r=Rebuild_BDDx(j)) {
				 case TRIV_UNSAT: 
				 case TRIV_SAT: 
				 case PREP_ERROR:
					ret = r; goto ea_bailout;
				 default: break;
				}
				
				for(int a = 0; a <= count1; a++) {
					original_functions[cheat[a]] = functions[cheat[a]];
					//fprintf(stderr, "[%d] ", cheat[a]);
					str_length = 0;
					for (int b = 0; b < numinp; b++) {
						for (llist * k = amount[b].head; k != NULL; k = k->next) {
							if(k->num == cheat[a]) {
								fprintf(stderr, "%d EEK!", b);
								exit(0);
							}
						}
					}
				}

				
				if(amount[i].head!=NULL) {
					for (llist * k = amount[i].head; k != NULL;) {
						int z = k->num;
						k = k->next;
						fprintf(stderr, "\n%d:", z);
						printBDD(functions[z]);						
					}
					fprintf(stderr, "OH NO!");
					exit(0);
				}				
				variablelist[i].true_false = 2;
				str_length = 0;
				dX_printf(3, "&");
			} else {
				DO_INFERENCES = 1;
				
				//functions[j] = ite_and(functions[j], Quantify);
				for(int a = 0; a <= count1; a++) {
					functions[cheat[a]] = original_functions[cheat[a]];
					switch (int r=Rebuild_BDDx(cheat[a])) {
					 case TRIV_UNSAT: 
					 case TRIV_SAT: 
					 case PREP_ERROR:
						ret = r; goto ea_bailout; /* as much as I hate gotos */
					 default: break;
					}
				}
			}
		}
	}
	ea_bailout:

    for(int iter = 0; iter<str_length; iter++)
    dX_printf(3, "\b");
    char p[100];
    sprintf(p, "{%ld:%ld/%ld}", affected, numinp, numinp);
    str_length = dX_printf(3, p);
	
	for(int x = 0; x < nmbrFunctions; x++)
	  Ea_repeat[x] = 1;
	return ret;
}
