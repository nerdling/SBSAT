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

int
CreateInferences () {
	
	Result *equivresult;

   //Compress the inverse tree that is variablelist.	
	for (int i = 1; i <= numinp; i++) {		  
		if (variablelist[i].equalvars != 0 && variablelist[abs(variablelist[i].equalvars)].equalvars!=0) {
			if(variablelist[i].equalvars>0) variablelist[i].equalvars = variablelist[variablelist[i].equalvars].equalvars;
			else variablelist[i].equalvars = -variablelist[-variablelist[i].equalvars].equalvars;
		}
	}
	
	//Create Inferences from variablelist
	
	infer *lastiter = AllocateInference(0, 0, NULL);
	infer *startiter = lastiter;
	for(int x = 1; x < numinp + 1; x++) {
		//fprintf(stderr, "%d = %d/%d ", x, variablelist[x].true_false, variablelist[x].equalvars);
		if(variablelist[x].true_false == 1) {
			lastiter->next = AllocateInference(x, 0, NULL);
			lastiter = lastiter->next;
		} else if(variablelist[x].true_false == 0) {
			lastiter->next = AllocateInference(-x, 0, NULL);
			lastiter = lastiter->next;
		} else if(variablelist[x].equalvars != 0) {
         lastiter->next = AllocateInference(x, variablelist[x].equalvars, NULL);
         /*
         if(variablelist[x].equalvars>0) {
            lastiter->next = AllocateInference(x, variablelist[x].equalvars, NULL);
			} else {
            lastiter->next = AllocateInference(-x, -variablelist[x].equalvars, NULL);
			}
         */
			lastiter = lastiter->next;
		}
	}
	lastiter->next = NULL;
	
	if(startiter == lastiter) {
      DeallocateOneInference(lastiter);
		lastiter = NULL;
		startiter = NULL;
		return 0;
	} else {	
		lastinfer->next = startiter->next;
      DeallocateOneInference(startiter);
	}
	
	infer *previous = lastinfer;
	
	//Remove duplicate inferences
	for (infer * iterator = lastinfer->next; iterator != NULL; iterator = iterator->next) {
		//fprintf(stderr, "(%d, %d)", iterator->nums[0], iterator->nums[1]);
		if (iterator->nums[1] == 0) {
			if (iterator->nums[0] > 0) {
				equivresult = l->insertEquiv (iterator->nums[0], T);
				if (equivresult == NULL) {
					//fprintf(stderr, "(%d, %d)", iterator->nums[0], iterator->nums[1]);
					infer *temp = iterator;
					previous->next = iterator->next;	
					//Skip over inference already applied
					iterator = previous;
               DeallocateOneInference(temp); //Delete the inference
               continue;	//Must continue to skip over 'previous = iterator;'
				} else if (equivresult->left == T && equivresult->rght == F)
					  return TRIV_UNSAT;
				else if (equivresult->left < T) {
					//Probably unnecessary...
					iterator->nums[0] = equivresult->left;	
					//i don't think equivresult->left will ever be T or F
				} else {
					iterator->nums[0] = equivresult->rght;
				}
			} else {
				equivresult = l->insertEquiv (-iterator->nums[0], F);
				if (equivresult == NULL) {
					//fprintf(stderr, "(%d, %d)", iterator->nums[0], iterator->nums[1]);
					infer *temp = iterator;
					previous->next = iterator->next;	
					//Skip over inference already applied
					iterator = previous;
               DeallocateOneInference(temp); //Delete the inference
					continue;	//Must continue to skip over 'previous = iterator;'
				} else if (equivresult->left == T && equivresult->rght == F)
					  return TRIV_UNSAT;
				else if (equivresult->left < T) {
					//Probably unnecessary...
					iterator->nums[0] = -equivresult->left;	//i don't think equivresult->left will ever be T or F
				} else {
					iterator->nums[0] = -equivresult->rght;
				}
			}
		} else {
			if (iterator->nums[1] > 0) {
				equivresult = l->insertEquiv (iterator->nums[0], iterator->nums[1]);
				//if(equivresult != NULL) fprintf(stderr, "<%d, %d>", equivresult->left, equivresult->rght);
				if (equivresult == NULL) {
					//fprintf(stderr, "(%d, %d)", iterator->nums[0], iterator->nums[1]);
					infer *temp = iterator;
					previous->next = iterator->next;	//Skip over inference already applied
					iterator = previous;
               DeallocateOneInference(temp); //Delete the inference
					continue;	//Must continue to skip over 'previous = iterator;'
				} else if (equivresult->left == T && equivresult->rght == F)
					  return TRIV_UNSAT;
				else if (equivresult->left == T) {
					iterator->nums[0] = equivresult->rght;
					iterator->nums[1] = 0;
				} else if (equivresult->rght == T) {
					iterator->nums[0] = equivresult->left;
					iterator->nums[1] = 0;
				} else if (equivresult->left == F) {
					iterator->nums[0] = -equivresult->rght;
					iterator->nums[1] = 0;
				} else if (equivresult->rght == F) {
					iterator->nums[0] = -equivresult->left;
					iterator->nums[1] = 0;
				} else if (abs (equivresult->left) < abs (equivresult->rght)) {
					if (equivresult->left < 0) {
						iterator->nums[0] = -equivresult->left;
						iterator->nums[1] = -equivresult->rght;
					} else {
						iterator->nums[0] = equivresult->left;
						iterator->nums[1] = equivresult->rght;
					}
				} else {
					if (equivresult->rght < 0) {
						iterator->nums[0] = -equivresult->rght;
						iterator->nums[1] = -equivresult->left;
					} else {
						iterator->nums[0] = equivresult->rght;
						iterator->nums[1] = equivresult->left;
					}
				}
			} else {
				equivresult = l->insertOppos (iterator->nums[0], -iterator->nums[1]);
				//if(equivresult != NULL) fprintf(stderr, ":%d, %d:", equivresult->left, equivresult->rght);
				if (equivresult == NULL) {
					//fprintf(stderr, "(%d, %d)", iterator->nums[0], iterator->nums[1]);
					infer *temp = iterator;
					previous->next = iterator->next;	//Skip over inference already applied
					iterator = previous;
               DeallocateOneInference(temp); //Delete the inference
					continue;	//Must continue to skip over 'previous = iterator;'
				} else if (equivresult->left == T && equivresult->rght == F)
					  return TRIV_UNSAT;
				else if (equivresult->left == T)	{
					iterator->nums[0] = -equivresult->rght;
					iterator->nums[1] = 0;
				} else if (equivresult->rght == T) {
					iterator->nums[0] = -equivresult->left;
					iterator->nums[1] = 0;
				} else if (equivresult->left == F) {
					iterator->nums[0] = equivresult->rght;
					iterator->nums[1] = 0;
				} else if (equivresult->rght == F) {
					iterator->nums[0] = equivresult->left;
					iterator->nums[1] = 0;
				} else if (abs (equivresult->left) < abs (equivresult->rght)) {
					if (equivresult->left < 0) {
						iterator->nums[0] = -equivresult->left;
						iterator->nums[1] = equivresult->rght;
					} else {
						iterator->nums[0] = equivresult->left;
						iterator->nums[1] = -equivresult->rght;
					}
				} else {
					if (equivresult->rght < 0) {
						iterator->nums[0] = -equivresult->rght;
						iterator->nums[1] = equivresult->left;
					} else {
						iterator->nums[0] = equivresult->rght;
						iterator->nums[1] = -equivresult->left;
					}
				}
			}
		}
      //fprintf(stderr, "!%d, %d!", iterator->nums[0], iterator->nums[1]);
      //              if(iterator->nums[1] > 100000) { 
      //                      l->printEquivalences();
      //                      exit(1);
      //              }
		if (iterator->nums[1] != 0)
		  assert (iterator->nums[0] > 0);
		assert (iterator->nums[0] != 0);
		previous = iterator;
	}
	
	lastinfer = previous;
	lastinfer->next = NULL;
	
	return PREP_NO_CHANGE;
}
