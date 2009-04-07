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

int Rebuild_BDDx(int x);
int checkGaussianElimTableforInfs();
int addEquivalencetoGETable(int v1, int v2);

int
Do_Apply_Inferences ()
{
	int ret = PREP_NO_CHANGE;

	/* Since I got up here DO_INFERENCES is set to 1 
	 * It gets restored upon the exit from this fn
	 */
	DO_INFERENCES = 0; 
	
	//fprintf(stderr, "APPLYING INFERENCES - ");
	infer *temp = inferlist;
	inferlist = inferlist->next;	//NEW Must increment past the empty start node
   DeallocateOneInference(temp); 

//	if (enable_gc && (rand() % 100 < 1)) 
      bdd_gc();
	//I know it looks strange to have this here twice, but it's necessary
	//for preprocessing options that call this but don't have any inferences.
	
	while (inferlist != NULL) {
		//startover:;
      bdd_gc();
      if (inferlist->nums[1] != 0) {
			if (inferlist->nums[1] > 0) {
/*				
				//d3_printf2("|%d ", inferlist->nums[0]);
				//d3_printf2("%d ", inferlist->nums[1]);
				inferlist->nums[0] = l->get_equiv(inferlist->nums[0]);
				if (inferlist->nums[0] == T) {
					inferlist->nums[0] = inferlist->nums[1];
					inferlist->nums[1] = 0;
					goto startover;
				} else if (inferlist->nums[0] == F) {
					inferlist->nums[0] = -inferlist->nums[1];
					inferlist->nums[1] = 0;
					goto startover;
				} else if (inferlist->nums[0] < 0) {
					inferlist->nums[0] = -inferlist->nums[0];
					inferlist->nums[1] = -inferlist->nums[1];					
				}
				//d3_printf2("%d ", inferlist->nums[0]);
				//d3_printf2("%d|", inferlist->nums[1]);
*/
				//            D_3(print_nonroller();)
				Pos_replace++;
				for(int iter = 0; iter<str_length; iter++)
				  d3_printf1("\b");
				d3e_printf3 ("{%d=%d}", inferlist->nums[0], inferlist->nums[1]);
				d4_printf5("{%s(%d)=%s(%d)}", s_name(inferlist->nums[0]), inferlist->nums[0], s_name(inferlist->nums[1]), inferlist->nums[1]);
				str_length = 0;

				if(variablelist[inferlist->nums[0]].true_false == 2) {
					d7_printf1("Inferring a quantified variable...\n");
				} else if(variablelist[inferlist->nums[0]].true_false != -1 || variablelist[inferlist->nums[0]].equalvars != 0) {
					fprintf(stderr, "Error: Inferring a variable twice, exiting...\n");
					assert(0);
					exit(1);
				}

				if(variablelist[inferlist->nums[1]].true_false == 2) {
					d7_printf1("Inferring a quantified variable...\n");
				} else if(variablelist[inferlist->nums[1]].true_false != -1 || variablelist[inferlist->nums[1]].equalvars != 0) {
					fprintf(stderr, "Error: Inferring a variable twice, exiting...\n");
					assert(0);
					exit(1);
				}
					 
				//assert(variablelist[inferlist->nums[1]].true_false == -1);
				variablelist[inferlist->nums[1]].equalvars = inferlist->nums[0];
				int count1 = 0;
				num_replace_all(amount[inferlist->nums[1]].head, inferlist->nums[1], inferlist->nums[0]);
				for (llist * k = amount[inferlist->nums[1]].head; k != NULL;) {
					count1++;
					int j = k->num;
					k = k->next; //This must be done here because k could be deleted in Rebuild_BDDx()
               if (Rebuild_BDDx(j)==TRIV_UNSAT)
                  return TRIV_UNSAT;
            }

				int count0 = 0;
				for (llist *k = amount[inferlist->nums[0]].head; k != NULL; k=k->next)
				  count0++;
				
				if(independantVars[inferlist->nums[1]] == 2 && count1 >= count0)
				  independantVars[inferlist->nums[0]] = 2;
				else if(independantVars[inferlist->nums[0]] == 2 && count1 > count0)
				  independantVars[inferlist->nums[0]] = independantVars[inferlist->nums[1]];
				else if(independantVars[inferlist->nums[1]] == 1 || independantVars[inferlist->nums[0]] == 1) {
					independantVars[inferlist->nums[1]] = 1;
					independantVars[inferlist->nums[0]] = 1;
				}
				
				
				//verifyCircuit(inferlist->nums[1]);
				
				if(ge_preproc == '1') {
					int r1; //Apply the equivalence to the Gaussian Elimination Table
					//switch (int r1 = addEquivalencetoGETable(inferlist->nums[0], inferlist->nums[1])) { //Test
					switch (r1 = l->applyEquiv(inferlist->nums[0], inferlist->nums[1])) {
					 case TRIV_UNSAT:
					 case TRIV_SAT:
					 case PREP_ERROR: return r1;
					 default:break;
					}
					if(r1 == PREP_CHANGED) {
						switch(r1=checkGaussianElimTableforInfs()) {
						 case TRIV_UNSAT:
						 case TRIV_SAT:
						 case PREP_ERROR: return r1;
						 default: break;
						}
					}
					//r1 == 0, no change
				}
			} else {
				Neg_replace++;
//            D_3(print_nonroller(););
				for(int iter = 0; iter<str_length; iter++)
				  d3_printf1("\b");
				str_length = 0;  
				d3e_printf3 ("{%d=%d}", inferlist->nums[0], inferlist->nums[1]);
				d4_printf5 ("{%s(%d)!=%s(%d)}", s_name(inferlist->nums[0]), inferlist->nums[0], s_name(-inferlist->nums[1]), -inferlist->nums[1]);

				if(variablelist[inferlist->nums[0]].true_false == 2) {
					d7_printf1("Inferring a quantified variable...\n");
				} else if(variablelist[inferlist->nums[0]].true_false != -1 || variablelist[inferlist->nums[0]].equalvars != 0) {
					fprintf(stderr, "Error: Inferring a variable twice, exiting...\n");
					assert(0);
					exit(1);
				}
				
				if(variablelist[-inferlist->nums[1]].true_false == 2) {
					d7_printf1("Inferring a quantified variable...\n");
				} else if(variablelist[-inferlist->nums[1]].true_false != -1 || variablelist[-inferlist->nums[1]].equalvars != 0) {
					fprintf(stderr, "Error: Inferring a variable twice, exiting...\n");
					assert(0);
					exit(1);
				}
				
				//assert(variablelist[-inferlist->nums[1]].true_false = -1);
				variablelist[-inferlist->nums[1]].equalvars = -inferlist->nums[0];
				//Gotta keep that (-inferlist->nums[0]) negative...trust me
				int count1 = 0;
				num_replace_all(amount[-inferlist->nums[1]].head, -inferlist->nums[1], -inferlist->nums[0]);
				for (llist * k = amount[-inferlist->nums[1]].head; k != NULL;) {
					count1++;
					int j = k->num;
					k = k->next; //This must be done here because k could be deleted in Rebuild_BDDx()
					if (functions[j] == false_ptr)
					  return TRIV_UNSAT;
               if (Rebuild_BDDx(j)==TRIV_UNSAT)
                  return TRIV_UNSAT;
            }
            int count0 = 0;
            for (llist *k = amount[inferlist->nums[0]].head; k != NULL; k=k->next)
				  count0++;
				
				if(independantVars[-inferlist->nums[1]] == 2 && count1 >= count0)
				  independantVars[inferlist->nums[0]] = 2;
				else if(independantVars[inferlist->nums[0]] == 2 && count1 > count0)
				  independantVars[inferlist->nums[0]] = independantVars[-inferlist->nums[1]];
				else if(independantVars[-inferlist->nums[1]] == 1 || independantVars[inferlist->nums[0]] == 1) {
					independantVars[-inferlist->nums[1]] = 1;
					independantVars[inferlist->nums[0]] = 1;
				}
				//verifyCircuit(-inferlist->nums[1]);
				
				if(ge_preproc == '1') {
					//switch (int r1 = addEquivalencetoGETable(inferlist->nums[0], inferlist->nums[1])) { //Test
					int r1; //Apply the equivalence to the Gaussian Elimination Table
					switch (r1 = l->applyEquiv(inferlist->nums[0], inferlist->nums[1])) {
					 case TRIV_UNSAT:
					 case TRIV_SAT:
					 case PREP_ERROR: return r1;
					 default:break;
					}
					if(r1 == PREP_CHANGED) {
						switch(r1=checkGaussianElimTableforInfs()) {
						 case TRIV_UNSAT:
						 case TRIV_SAT:
						 case PREP_ERROR: return r1;
						 default: break;
						}
					}
					//r1 == 0, no change
				}
			}
		} else {
			if (inferlist->nums[0] > 0) {
				Setting_Pos++;
//            D_3(print_nonroller(););
				for(int iter = 0; iter<str_length; iter++)
				  d3_printf1("\b");
				str_length = 0;  
				d3e_printf2 ("{%d=T}", abs (inferlist->nums[0]));
				d4_printf3 ("{%s(%d)=T}", s_name(abs(inferlist->nums[0])), abs (inferlist->nums[0]));

				if(variablelist[inferlist->nums[0]].true_false == 2) {
					d7_printf1("Inferring a quantified variable...\n");
				} else if(variablelist[inferlist->nums[0]].true_false != -1 || variablelist[inferlist->nums[0]].equalvars != 0) {
					fprintf(stderr, "Inferring a variable (%d) twice, exiting...\n", inferlist->nums[0]);
					assert(0);
					exit(1);
				}
				
				variablelist[inferlist->nums[0]].true_false = 1;
				set_variable_all(amount[inferlist->nums[0]].head, inferlist->nums[0], 1);
				for (llist * k = amount[inferlist->nums[0]].head; k != NULL;) {
					int j = k->num;
					k = k->next; //This must be done here because k could be deleted in Rebuild_BDDx()
               if (Rebuild_BDDx(j)==TRIV_UNSAT)
                  return TRIV_UNSAT;
            }
				//verifyCircuit(inferlist->nums[0]);
				if(ge_preproc == '1') {
					int r1; //Apply the inference to the Gaussian Elimination Table
					switch (r1 = l->makeAssign(inferlist->nums[0], 1)) {
					 case TRIV_UNSAT:
					 case TRIV_SAT:
					 case PREP_ERROR: return r1;
					 default:break;
					}
					if(r1 == PREP_CHANGED) {
						switch(r1=checkGaussianElimTableforInfs()) {
						 case TRIV_UNSAT:
						 case TRIV_SAT:
						 case PREP_ERROR: return r1;
						 default: break;
						}
					}
					//r1 == 0, no change
				}
			} else {
				Setting_Neg++;
//            D_3(print_nonroller(););
				for(int iter = 0; iter<str_length; iter++)
					d3_printf1("\b");
				str_length = 0;  
				d3e_printf2 ("{%d=F}", -inferlist->nums[0]);
				d4_printf3 ("{%s(%d)=F}", s_name(-inferlist->nums[0]), -inferlist->nums[0]);

				if(variablelist[-inferlist->nums[0]].true_false == 2) {
					d7_printf1("Inferring a quantified variable...\n");
				} else if(variablelist[-inferlist->nums[0]].true_false != -1 || variablelist[-inferlist->nums[0]].equalvars != 0) {
					fprintf(stderr, "Inferring a variable (%d) twice, exiting...\n", inferlist->nums[0]);
					assert(0);
					exit(1);
				}
				
				variablelist[-inferlist->nums[0]].true_false = 0;
				set_variable_all(amount[-inferlist->nums[0]].head, -inferlist->nums[0], 0);
				for (llist * k = amount[-inferlist->nums[0]].head; k != NULL;) {
					int j = k->num;
					k = k->next; //This must be done here because k could be deleted in Rebuild_BDDx()
               if (Rebuild_BDDx(j)==TRIV_UNSAT)
                  return TRIV_UNSAT;
				}
				//verifyCircuit(-inferlist->nums[0]);
				if(ge_preproc == '1') {
					int r1; //Apply the inference to the Gaussian Elimination Table
					switch (r1 = l->makeAssign(-inferlist->nums[0], 0)) {
					 case TRIV_UNSAT:
					 case TRIV_SAT:
					 case PREP_ERROR: return r1;
					 default:break;
					}
					if(r1 == PREP_CHANGED) {
						switch(r1=checkGaussianElimTableforInfs()) {
						 case TRIV_UNSAT:
						 case TRIV_SAT:
						 case PREP_ERROR: return r1;
						 default: break;
						}
					}
					//r1 == 0, no change
				}
			}
		}
		temp = inferlist;
		inferlist = inferlist->next;
      DeallocateOneInference(temp); 
      temp = NULL;
   }
	inferlist = AllocateInference(0, 0, NULL); //NEW
	inferlist->next = NULL;
	lastinfer = inferlist;
	DO_INFERENCES = 1;
	
	return ret;
}

int setALLequiv(int nums0, int nums1, int torf) {
//   D_3(print_nonroller(););
	for(int iter = 0; iter<str_length; iter++)
	  d3_printf1("\b");
	str_length = 0;  
   d3_printf3 ("{%d=%d}", torf*nums0, torf*nums1);
	variablelist[nums1].equalvars = nums0;
	for (llist * k = amount[nums1].head; k != NULL;) {
		int j = k->num;
		k = k->next; //This must be done here because k could be deleted in Rebuild_BDDx()
		BDDNode *before = functions[j];
		functions[j] = num_replace (functions[j], nums1, nums0);
		if (before != functions[j]) {
			if (Rebuild_BDDx(j)==TRIV_UNSAT)
			  return TRIV_UNSAT;
		}
	}

	BDDNode *before = functions[0];
	functions[0] = num_replace (functions[0], nums1, nums0);
	if (before != functions[0]) {
      if (Rebuild_BDDx(0)==TRIV_UNSAT)
		  return TRIV_UNSAT;
	}

//	amount[torf*nums0].tail->next = amount[nums1].head;
//	amount[torf*nums0].tail = amount[nums1].tail;
//	amount[nums1].head = NULL;
//	amount[nums1].tail = NULL;
	//verifyCircuit(nums1);
	return PREP_NO_CHANGE;
}

int setALLinfer(int nums0, int torf) {
	if(torf) {
//      D_3(print_nonroller(););
		for(int iter = 0; iter<str_length; iter++)
		  d3_printf1("\b");
		str_length = 0;  
      d3_printf2 ("{%d=T}", nums0);
		variablelist[nums0].true_false = 1;
	} else {
//      D_3(print_nonroller(););
		for(int iter = 0; iter<str_length; iter++)
		  d3_printf1("\b");
		str_length = 0;  
      d3_printf2 ("{%d=F}", nums0);
		variablelist[nums0].true_false = 0;
	}
	for (llist * k = amount[nums0].head; k != NULL;) {
		int j = k->num;
		k = k->next; //This must be done here because k could be deleted in Rebuild_BDDx()
		BDDNode *before = functions[j];
		functions[j] = set_variable (functions[j], nums0, torf);
		if (before != functions[j]) {
         if (Rebuild_BDDx(j)==TRIV_UNSAT)
			  return TRIV_UNSAT;
		}
	}

	BDDNode *before = functions[0];
	functions[0] = set_variable (functions[0], nums0, torf);
	if (before != functions[0]) {
		if (Rebuild_BDDx(0)==TRIV_UNSAT)
		  return TRIV_UNSAT;
	}
	
	//verifyCircuit(nums0);
	return PREP_NO_CHANGE;
}

int Do_Apply_Inferences_backend () {
	int ret = PREP_NO_CHANGE;
	//  fprintf(stderr, "APPLYING INFERENCES - ");
	infer *temp = inferlist;
	inferlist = inferlist->next;	//NEW Must increment past the empty start node
   DeallocateOneInference(temp); 
   while (inferlist != NULL) {
		if (inferlist->nums[1] != 0) {
			if (inferlist->nums[1] > 0) {
				if(setALLequiv(inferlist->nums[0], inferlist->nums[1], 1) == TRIV_UNSAT) return TRIV_UNSAT;
			}	else {
				if(setALLequiv(-inferlist->nums[0], -inferlist->nums[1], -1) == TRIV_UNSAT) return TRIV_UNSAT;
			}
		} else {
			if (inferlist->nums[0] > 0) {
				if(setALLinfer(inferlist->nums[0], 1) == TRIV_UNSAT) return TRIV_UNSAT;
			} else {
				if(setALLinfer(-inferlist->nums[0], 0) == TRIV_UNSAT) return TRIV_UNSAT;
			}
		}
		temp = inferlist;
		inferlist = inferlist->next;
      DeallocateOneInference(temp); 
      temp = NULL;
	}
	inferlist = AllocateInference(0, 0, NULL);
	lastinfer = inferlist;
	return ret;
}

void printBDDInfs(BDDNode *bdd) {
	for(infer *iterator = bdd->inferences; iterator != NULL; iterator = iterator->next) {
		d3_printf3 ("{%d, %d}", iterator->nums[0], iterator->nums[1]);
		//fprintf(stderr, "%d|%d, %d|", x, iterator->nums[0], iterator->nums[1]);
	}
}

VecType *addXORVector (int x) {
	VecType *vector = (VecType *)ite_calloc(1, sizeof(VecType)*(1+numinp/sizeof(VecType)*8), 9, "xor vector");
	for(int j=0; j < length[x]; j++) {
		vector[variables[x].num[j]/(sizeof(VecType)*8)] |= (1 << (variables[x].num[j] % (sizeof(VecType)*8)));
	}
	
	BDDNode *ptr;
	for(ptr = functions[x]; !IS_TRUE_FALSE(ptr); ptr = ptr->thenCase);
	if((length[x]%2) == 0) { //If xor has an even number of variables
		if(ptr == false_ptr) //and True most leaf node is false, xor function equals True
		  vector[numinp/(sizeof(VecType)*8)] |= (VecType)(1 << ((sizeof(VecType)*8)-1));
	} else { //If xor has an odd number of variables
		if(ptr == true_ptr) //and True most leaf is true, xor function equals True
		  vector[numinp/(sizeof(VecType)*8)] |= (VecType)(1 << ((sizeof(VecType)*8)-1));
	} //xor function equals False otherwise
	return vector;
}

int *getVarList (VecType *vector, int size, int *nvars) {
	int count = 0;
	for(int i=0; i<size; i++) {
		VecType w = vector[i];
		for(int j=0; j<(int)sizeof(VecType)*8; j++) {
			if(!(i == size-1 && j == sizeof(VecType)*8-1))
			  if(w % 2) count++;
			w/=2;
		}
	}
	*nvars = count;
	int *varlist = (int *)ite_calloc(1, sizeof(int)*(count+1),9, "xor varlist");
	count = 0;
	for(int i=size-1; i>=0; i--) {
		VecType w=vector[i];
		for(int j=sizeof(VecType)*8-1; j>=0; j--) {
			if(i == size-1 && j == sizeof(VecType)*8-1) continue;
			if(w & (1 << j)) varlist[count++] = i*sizeof(VecType)*8 + j;
		}
	}
	varlist[count] = -1;
	return varlist;
}

int addEquivalencetoGETable(int v1, int v2) {
	XORd *xor_func = new XORd;
	// 0-1 vector showing vars in xor func and which type of xor func it is

	// List of vars that are 1 in vector
	xor_func->varlist = (int *)ite_calloc(1, sizeof(int)*3,9, "xor varlist");
	xor_func->varlist[0] = v1;
	xor_func->varlist[2] = -1;
	
	VecType *vector = (VecType *)ite_calloc(1, sizeof(VecType)*(1+numinp/sizeof(VecType)*8), 9, "xor vector");
	vector[v1/(sizeof(VecType)*8)] |= (1 << (v1 % (sizeof(VecType)*8)));
	if(v2 > 0) {
		vector[v2/(sizeof(VecType)*8)] |= (1 << (v2 % (sizeof(VecType)*8)));
		xor_func->varlist[1] = v2;
	} else {
		vector[-v2/(sizeof(VecType)*8)] |= (1 << (-v2 % (sizeof(VecType)*8)));
		vector[numinp/(sizeof(VecType)*8)] |= (VecType)(1 << ((sizeof(VecType)*8)-1));
		xor_func->varlist[1] = -v2;
	}

	xor_func->vector = vector;
	
	// Number of bytes in xor_vector
	xor_func->vector_size = 1 + numinp/(sizeof(VecType)*8);
	// Number of vars in the function
	xor_func->nvars = 2;
	xor_func->type = (xor_func->vector[xor_func->vector_size-1] & (1 << (sizeof(VecType)*8-1))) ? 1 : 0;
	xor_func->next = NULL;
	int r=l->addRow(xor_func);
	ite_free((void **)&xor_func->varlist);
	ite_free((void **)&xor_func->vector);
	if(r == 1) {
		switch(int r1=checkGaussianElimTableforInfs()) {
		 case TRIV_UNSAT:
		 case TRIV_SAT:
		 case PREP_ERROR: return r1;
		 default: break;
		}
	} else if(r == -1) return TRIV_UNSAT;
	//r == 0, no change
	return PREP_NO_CHANGE;
}

int checkGaussianElimTableforInfs() {
	Result *result = l->findAndSaveEquivalences();
	if(result != NULL) {
		for(int x = 0; result[x].left != F || result[x].rght != F; x++) {
			if(result[x].left == T || result[x].left == -F) {
				if(result[x].rght == F) return TRIV_UNSAT;
				lastinfer->next = AllocateInference(result[x].rght, 0, NULL);
				lastinfer = lastinfer->next;
			} else if(result[x].rght == T || result[x].rght == -F) {
				lastinfer->next = AllocateInference(result[x].left, 0, NULL);
				lastinfer = lastinfer->next;
			} else if(result[x].left == F || result[x].left == -T) {
				lastinfer->next = AllocateInference(-result[x].rght, 0, NULL);
				lastinfer = lastinfer->next;
			} else if(result[x].rght == F || result[x].rght == -T) {			
				lastinfer->next = AllocateInference(-result[x].left, 0, NULL);
				lastinfer = lastinfer->next;
			} else if(abs(result[x].left) < abs(result[x].rght)) {
				if(result[x].left < 0) {
					lastinfer->next = AllocateInference(-result[x].left, -result[x].rght, NULL);
					lastinfer = lastinfer->next;
				} else {
					lastinfer->next = AllocateInference(result[x].left, result[x].rght, NULL);
					lastinfer = lastinfer->next;
				}
			} else {
				if(result[x].rght < 0) {
					lastinfer->next = AllocateInference(-result[x].rght, -result[x].left, NULL);
					lastinfer = lastinfer->next;
				} else {
					lastinfer->next = AllocateInference(result[x].rght, result[x].left, NULL);
					lastinfer = lastinfer->next;
				}
			}
			lastinfer->next = NULL;
		}
	}
	return PREP_NO_CHANGE;	
}

int ReduceInferences() {
	Result *result;
	infer *previous = lastinfer;
	
	//Remove duplicate inferences
	for (infer * iterator = lastinfer->next; iterator != NULL;
		  iterator = iterator->next) {
		//fprintf(stderr, "(%d, %d)", iterator->nums[0], iterator->nums[1]);
		if (iterator->nums[1] == 0) {
			if (iterator->nums[0] > 0) {
				result = l->insertEquiv (iterator->nums[0], T);
				if (result == NULL) {
					//fprintf(stderr, "(%d, %d)", iterator->nums[0], iterator->nums[1]);
					infer *temp = iterator;
					previous->next = iterator->next;	//Skip over inference already applied
					iterator = previous;
               DeallocateOneInference(temp); //Delete the inference
					continue;	//Must continue to skip over 'previous = iterator;'
				} else if (result->left == T && result->rght == F)
					  return TRIV_UNSAT;
				else if (result->left < T) {		//Probably unnecessary...
					iterator->nums[0] = result->left;	//i don't think result->left will ever be T or F
				} else {
					iterator->nums[0] = result->rght;
				}
			} else {
				result = l->insertEquiv (-iterator->nums[0], F);
				if (result == NULL) {
					//fprintf(stderr, "(%d, %d)", iterator->nums[0], iterator->nums[1]);
					infer *temp = iterator;
					previous->next = iterator->next;	//Skip over inference already applied
					iterator = previous;
               DeallocateOneInference(temp); //Delete the inference
					continue;	//Must continue to skip over 'previous = iterator;'
				} else if (result->left == T && result->rght == F)
					  return TRIV_UNSAT;
				else if (result->left < T) {		//Probably unnecessary...
					iterator->nums[0] = -result->left;	//i don't think result->left will ever be T or F
				} else {
					iterator->nums[0] = -result->rght;
				}
			}
		} else {
			if (iterator->nums[1] > 0) {
				result = l->insertEquiv (iterator->nums[0], iterator->nums[1]);
				//if(result != NULL) fprintf(stderr, "<%d, %d>", result->left, result->rght);
				if (result == NULL) {
					//fprintf(stderr, "(%d, %d)", iterator->nums[0], iterator->nums[1]);
					infer *temp = iterator;
					previous->next = iterator->next;	//Skip over inference already applied
					iterator = previous;
               DeallocateOneInference(temp); //Delete the inference
					continue;	//Must continue to skip over 'previous = iterator;'
				} else if (result->left == T && result->rght == F)
					  return TRIV_UNSAT;
				else if (result->left == T) {
					iterator->nums[0] = result->rght;
					iterator->nums[1] = 0;
				} else if (result->rght == T) {
					iterator->nums[0] = result->left;
					iterator->nums[1] = 0;
				} else if (result->left == F) {
					iterator->nums[0] = -result->rght;
					iterator->nums[1] = 0;
				} else if (result->rght == F) {
					iterator->nums[0] = -result->left;
					iterator->nums[1] = 0;
				} else if (abs (result->left) < abs (result->rght)) {
					if (result->left < 0) {
						iterator->nums[0] = -result->left;
						iterator->nums[1] = -result->rght;
					} else {
						iterator->nums[0] = result->left;
						iterator->nums[1] = result->rght;
					}
				} else {
					if (result->rght < 0) {
						iterator->nums[0] = -result->rght;
						iterator->nums[1] = -result->left;
					} else {
						iterator->nums[0] = result->rght;
						iterator->nums[1] = result->left;
					}
				}
			} else {
				result = l->insertOppos (iterator->nums[0], -iterator->nums[1]);
				//if(result != NULL) fprintf(stderr, ":%d, %d:", result->left, result->rght);
				if (result == NULL) {
					//fprintf(stderr, "(%d, %d)", iterator->nums[0], iterator->nums[1]);
					infer *temp = iterator;
					previous->next = iterator->next;	//Skip over inference already applied
					iterator = previous;
               DeallocateOneInference(temp); //Delete the inference
					continue;	//Must continue to skip over 'previous = iterator;'
				} else if (result->left == T && result->rght == F)
					  return TRIV_UNSAT;
				else if (result->left == T) {
					iterator->nums[0] = -result->rght;
					iterator->nums[1] = 0;
				} else if (result->rght == T) {
					iterator->nums[0] = -result->left;
					iterator->nums[1] = 0;
				} else if (result->left == F) {
					iterator->nums[0] = result->rght;
					iterator->nums[1] = 0;
				} else if (result->rght == F) {
					iterator->nums[0] = result->left;
					iterator->nums[1] = 0;
				} else if (abs (result->left) < abs (result->rght)) {
					if (result->left < 0) {
						iterator->nums[0] = -result->left;
						iterator->nums[1] = result->rght;
					} else {
						iterator->nums[0] = result->left;
						iterator->nums[1] = -result->rght;
					}
				} else {
					if (result->rght < 0) {
						iterator->nums[0] = -result->rght;
						iterator->nums[1] = result->left;
					} else {
						iterator->nums[0] = result->rght;
						iterator->nums[1] = -result->left;
					}
				}
			}
		}
		//fprintf(stderr, "!%d, %d!", iterator->nums[0], iterator->nums[1]);
		//if(iterator->nums[1] > 100000) { 
		//l->printEquivalences();
		//	exit(1);
		//}
		if (iterator->nums[1] != 0)
		  assert (iterator->nums[0] > 0);
		assert (iterator->nums[0] != 0);
		previous = iterator;
	}
	
	lastinfer = previous;
	lastinfer->next = NULL;
	return PREP_NO_CHANGE;
}

int Rebuild_BDDx (int x) {
	SetRepeats(x);
	
	if (functions[x] == false_ptr)
	  return TRIV_UNSAT;

	//Get Inferences	
	infer *lastiter = NULL;
	infer *startiter = NULL;

	if(functions[x]->inferences != NULL) {
		lastiter = AllocateInference(0, 0, NULL);
		startiter = lastiter;
		for(infer *iterator = functions[x]->inferences; iterator != NULL; iterator = iterator->next) {
			lastiter->next = AllocateInference(iterator->nums[0], iterator->nums[1], NULL);
			lastiter = lastiter->next;
		}
		lastiter->next = NULL;
		if(startiter!=NULL) {
			lastinfer->next = startiter->next;
			DeallocateOneInference(startiter); 
		} else lastinfer->next = NULL;
	}

	switch(int r1 = ReduceInferences()) {
	  case TRIV_UNSAT:
 	  case TRIV_SAT:
	  case PREP_ERROR: return r1;
	  default: break;
	}

	assert(x<nmbrFunctions+1);
	int y = 0;
	unravelBDD(&y, &bdd_tempint_max, &bdd_tempint, functions[x]);
   int *tempint=bdd_tempint;
//	fprintf(stderr, "%d|", y);
	if (y != 0) qsort (tempint, y, sizeof (int), compfunc);
//	fprintf(stderr, "%d|", y);
//	printBDDerr(functions[x]);
//	fprintf(stderr, "\n");
//	for(int j = 0; j < y; j++)
//	  fprintf(stderr, "%d ", tempint[j]);
//	fprintf(stderr, "\n");
						 
	
	//Checking to see if any variables were added to this BDD
	//Dependent clustering can add variables, so can Ex_AND, so can Steal

	//Would really like to incorporate existential quantification into this.
	//Any variable which is removed from this BDD should be checked for Ex.
	//What I really need to do is make a "functions to update" queue, and
	//use that instead of relying on recursion.
	
	//Need to add BDD x into the inference list of any variable that wasn't
	//originally in x.
	if (variables[x].num != NULL) {
		int a = 0, b = 0;
		while(a < y && b < length[x]) {
			if(tempint[a] > variables[x].num[b]) {
				//could remove bdd x from variables[x].num[b]'s inference list.
				llist *k = amount[variables[x].num[b]].head;
				llist *follow = NULL;
				while (k != NULL) {
					if(k->num == x) {
						//fprintf(stderr, "{deleting1 %d %d}\n", x, variables[x].num[b]);
						num_funcs_var_occurs[variables[x].num[b]]--;
						llist *temp = k;
						k = k->next;
                  DeallocateOneLList(temp);
                  //delete temp;
                  if(follow != NULL) follow->next = k;
						else amount[variables[x].num[b]].head = k;
						if(k == NULL) amount[variables[x].num[b]].tail = follow;
						k = NULL; //to break out now that we've deleted the entry.
					} else {
						follow = k;
						k = k->next;
					}
				}
//				if(follow == NULL) {
//					amount[variables[x].num[b]].head = NULL;
//					amount[variables[x].num[b]].tail = NULL;
//				}
				b++;
			} else if(tempint[a] < variables[x].num[b]) {
            llist *newllist = AllocateLList(x, NULL);
				//llist *newllist = new llist;
				//fprintf(stderr, "{adding1 %d %d}\n", x, tempint[a]);
				//newllist->num = x;
				//newllist->next = NULL;
				if (amount[tempint[a]].head == NULL) {
					num_funcs_var_occurs[tempint[a]] = 1;
					amount[tempint[a]].head = newllist;
					amount[tempint[a]].tail = newllist;
				} else {
					num_funcs_var_occurs[tempint[a]]++;
					amount[tempint[a]].tail->next = newllist;
					amount[tempint[a]].tail = newllist;
				}
				a++;
			} else { a++; b++; }
		}
		
		while(b < length[x]) {
			//could remove bdd x from variables[x].num[b]'s inference list.
			llist *k = amount[variables[x].num[b]].head;
			llist *follow = NULL;
			while (k != NULL) {
				if(k->num == x) {
					//fprintf(stderr, "{deleting2 %d %d}\n", x, variables[x].num[b]);
					num_funcs_var_occurs[variables[x].num[b]]--;
					llist *temp = k;
					k = k->next;
               DeallocateOneLList(temp);
               //delete temp;
               if(follow != NULL) follow->next = k;
					else amount[variables[x].num[b]].head = k;
					if(k == NULL) amount[variables[x].num[b]].tail = follow;
					k = NULL; //to break out now that we've deleted the entry.
				} else {
					follow = k;
					k = k->next;
				}
			}
//			if(follow == NULL) {
//				amount[variables[x].num[b]].head = NULL;
//				amount[variables[x].num[b]].tail = NULL;
//			}
			b++;			  
		}
		
		while(a < y) {
         llist *newllist = AllocateLList(x, NULL);
         //llist *newllist = new llist;
			//fprintf(stderr, "{adding2 %d %d}\n", x, tempint[a]);
			//newllist->num = x;
			//newllist->next = NULL;
			if (amount[tempint[a]].head == NULL) {
				num_funcs_var_occurs[tempint[a]] = 1;
				amount[tempint[a]].head = newllist;
				amount[tempint[a]].tail = newllist;
			} else {
				num_funcs_var_occurs[tempint[a]]++;
				amount[tempint[a]].tail->next = newllist;
				amount[tempint[a]].tail = newllist;
			}
			a++;
		}
	} else { //A NEW BDD
		for (int b = 0; b < y; b++) {
			llist *newllist = AllocateLList(x, NULL);
			//fprintf(stderr, "f=%d v=%d\n", x, tempint[b]);
			if (amount[tempint[b]].head == NULL) {
				num_funcs_var_occurs[tempint[b]] = 1;
				amount[tempint[b]].head = newllist;
				amount[tempint[b]].tail = newllist;
			} else {
				num_funcs_var_occurs[tempint[b]]++;
				amount[tempint[b]].tail->next = newllist;
				amount[tempint[b]].tail = newllist;
			}
		}
	}
	//Done adding BDD x into the inferences lists.

	length[x] = y;
	
   if (variables[x].num_alloc < y+1) {
      if (variables[x].num_alloc > 0) delete [] variables[x].num;
      variables[x].num = new int[y + 1]; //(int *)calloc(y+1, sizeof(int));
      variables[x].num_alloc = y+1;
   }

	for (int i = 0; i < y; i++)
	  variables[x].num[i] = tempint[i];
   if (y==0) {
      variables[x].min = 0;
      variables[x].max = 0;
   } else {
      variables[x].min = tempint[0];
      variables[x].max = tempint[y-1];
   }

	if(functionType[x] != XDD) {
		functionType[x] = UNSURE;
		equalityVble[x] = 0;
		findandset_fnType(x);
	}
	
	if(ge_preproc == '1' && DO_INFERENCES) {
//	if(ge_preproc == '1') {
		//Handle the XOR functions by using gaussian Elimination
		//Can make this better by splitting the xor parts out of each BDD
		if(functionType[x] == PLAINXOR && l->rec->index <= l->no_funcs) { 
			XORd *xor_func = new XORd;
			// 0-1 vector showing vars in xor func and which type of xor func it is
			xor_func->vector = addXORVector(x);
			// Number of bytes in xor_vector
			xor_func->vector_size = 1 + numinp/(sizeof(VecType)*8);
			// List of vars that are 1 in vector
			int nvars = length[x];
			xor_func->varlist = getVarList(xor_func->vector, xor_func->vector_size, &nvars);
			// Number of vars in the function
			xor_func->nvars = nvars;
			xor_func->type = (xor_func->vector[xor_func->vector_size-1] & (1 << (sizeof(VecType)*8-1))) ? 1 : 0;
			xor_func->next = NULL;
			int r=l->addRow(xor_func);
			ite_free((void **)&xor_func->varlist);
			ite_free((void **)&xor_func->vector);
			if(r == 1) {
				switch(int r1=checkGaussianElimTableforInfs()) {
 				  case TRIV_UNSAT:
				  case TRIV_SAT:
				  case PREP_ERROR: return r1;
				  default: break;
				}
			} else if(r == -1) return TRIV_UNSAT;
			//r == 0, no change
		}
	}

	if (DO_INFERENCES) {
		return Do_Apply_Inferences();
	}
	
	return PREP_NO_CHANGE;
}

int
Rebuild_BDD (BDDNode *bdd, int *bdd_length, int *&bdd_vars)
{
	if (bdd == false_ptr)
	  return TRIV_UNSAT;
	
	//Get Inferences
	
	//This still does more new statements than necessary, but it's definetly faster.
	
	infer *lastiter = NULL;
	infer *startiter = NULL;
	if(bdd->inferences != NULL) {
		lastiter = AllocateInference(0, 0, NULL);
		startiter = lastiter;
		for(infer *iterator = bdd->inferences; iterator != NULL; iterator = iterator->next) {
			lastiter->next = AllocateInference(iterator->nums[0], iterator->nums[1], NULL);
			lastiter = lastiter->next;
			//lastiter->nums[0] = iterator->nums[0];
			//lastiter->nums[1] = iterator->nums[1];
		}
		lastiter->next = NULL;
	}
	
	if(startiter!=NULL) {
		lastinfer->next = startiter->next;
      DeallocateOneInference(startiter); 
   }

	switch(int r1 = ReduceInferences()) {
	  case TRIV_UNSAT:
 	  case TRIV_SAT:
	  case PREP_ERROR: return r1;
	  default: break;
	}

	int y = 0;	
	unravelBDD(&y, &bdd_tempint_max, &bdd_tempint, bdd);
   int *tempint = bdd_tempint;
	if (y != 0) qsort (tempint, y, sizeof (int), compfunc);
	
	(*bdd_length) = y;
	if (bdd_vars != NULL)
	  delete [] bdd_vars;
	bdd_vars = new int[y + 1];	//(int *)calloc(y+1, sizeof(int));
	for (int i = 0; i < y; i++)
	  bdd_vars[i] = tempint[i];
	
	//if (DO_INFERENCES)
	//  return Do_Apply_Inferences();

	return PREP_NO_CHANGE;
}
