/* =========FOR INTERNAL USE ONLY. NO DISTRIBUTION PLEASE ========== */

/*********************************************************************
 Copyright 1999-2003, University of Cincinnati.  All rights reserved.
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
/*********************************************************
 *  preprocess.c (S. Weaver)
 *********************************************************/

#include "ite.h"
#include "preprocess.h"

extern int Pos_replace;
extern int Neg_replace;
extern int Setting_Pos;
extern int Setting_Neg;

int Rebuild_BDDx (int x);
long bdd_tempint_max=0;
int *bdd_tempint=NULL;


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
	delete temp;

	void bdd_gc();
	if (enable_gc && (rand() % 1000 < 2)) bdd_gc();
	//I know it looks strange to have this here twice, but it's necessary
	//for preprocessing options that call this but don't have any inferences.
	
	while (inferlist != NULL) {
		//startover:;
		void bdd_gc();
		if (enable_gc && (rand() % 1000 < 4)) bdd_gc();
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
				d3_printf3 ("{%d=%d}", inferlist->nums[0], inferlist->nums[1]);
				str_length = 0;
				variablelist[inferlist->nums[1]].equalvars = inferlist->nums[0];
				for (llist * k = amount[inferlist->nums[1]].head; k != NULL;) {
					int j = k->num;
					k = k->next; //This must be done here because k could be deleted in Rebuild_BDDx()
					BDDNode *before = functions[j];
					functions[j] = num_replace (functions[j], inferlist->nums[1], inferlist->nums[0]);
					if (functions[j] == false_ptr)
					  return TRIV_UNSAT;
					if (before != functions[j]) {
						int changeFT = 0;
						if (functionType[j] == AND || functionType[j] == OR) {
							if (abs (equalityVble[j]) == inferlist->nums[0]) {
								for (int iter = 0; iter < length[j]; iter++) {
									if (variables[j].num[iter] >= inferlist->nums[1]) {
										if (variables[j].num[iter] == inferlist->nums[1])
										  changeFT = 1;
										break;
									}
								}
								if (changeFT == 1) {
									functionType[j] = UNSURE; //x v (a & b & c)
									//independantVars[equalityVble[j]] = 1;
									equalityVble[j] = 0;
								}
							} else if (abs (equalityVble[j]) == inferlist->nums[1]) {
								for (int iter = 0; iter < length[j]; iter++) {
									if (variables[j].num[iter] >= inferlist->nums[0]) {
										if (variables[j].num[iter] == inferlist->nums[0])
										  changeFT = 1;
										break;
									}
								}
								if (changeFT == 1) {
									functionType[j] = UNSURE; //x v (a & b & c)
									//independantVars[equalityVble[j]] = 1;
									equalityVble[j] = 0;
								} else if (equalityVble[j] > 0) {
									  equalityVble[j] = inferlist->nums[0];
								} else equalityVble[j] = -inferlist->nums[0];
							}
						}

						if (Rebuild_BDDx(j)==TRIV_UNSAT)
						  return TRIV_UNSAT;
					}
				}
//				amount[inferlist->nums[0]].tail->next = amount[inferlist->nums[1]].head;
//				amount[inferlist->nums[0]].tail = amount[inferlist->nums[1]].tail;
//				amount[inferlist->nums[1]].head = NULL;
//				amount[inferlist->nums[1]].tail = NULL;
				//verifyCircuit(inferlist->nums[1]);
			} else {
				Neg_replace++;
//            D_3(print_nonroller(););
				for(int iter = 0; iter<str_length; iter++)
				  d3_printf1("\b");
				str_length = 0;  
				d3_printf3 ("{%d=%d}", inferlist->nums[0], inferlist->nums[1]);
				variablelist[-inferlist->nums[1]].equalvars = -inferlist->nums[0];
				//Gotta keep that (-inferlist->nums[0]) negative...trust me
				for (llist * k = amount[-inferlist->nums[1]].head; k != NULL;) {
					int j = k->num;
					k = k->next; //This must be done here because k could be deleted in Rebuild_BDDx()
					BDDNode *before = functions[j];
					//printBDDerr(before);
					//fprintf(stderr, "\n");
					functions[j] = num_replace (functions[j], -inferlist->nums[1], -inferlist->nums[0]);
					if (functions[j] == false_ptr)
					  return TRIV_UNSAT;
					if (before != functions[j]) {
						int changeFT = 0;
						if (functionType[j] == AND || functionType[j] == OR) {
							if (abs (equalityVble[j]) == inferlist->nums[0]) {
								for (int iter = 0; iter < length[j]; iter++) {
									if (variables[j].num[iter] >= -inferlist->nums[1]) {
										if (variables[j].num[iter] == -inferlist->nums[1])
										  changeFT = 1;
										break;
									}
								}
								if (changeFT == 1) {
									//functionType[j] = ?????;	//-x -> (-a & -b & -c)
									  functionType[j] = UNSURE;
									//independantVars[equalityVble[j]] = 1;
									equalityVble[j] = 0;
								}
							} else if (abs (equalityVble[j]) == -inferlist->nums[1]) {
								for (int iter = 0; iter < length[j]; iter++) {
									if (variables[j].num[iter] >= inferlist->nums[0]) {
										if (variables[j].num[iter] == inferlist->nums[0])
										  changeFT = 1;
										break;
									}
								}
								if (changeFT == 1) {
									//functionType[j] = ?????;	//-x -> (-a & -b & -c)
									functionType[j] = UNSURE;
									//independantVars[equalityVble[j]] = 1;
									equalityVble[j] = 0;
								} else {
									if (equalityVble[j] > 0)
									  equalityVble[j] =  -inferlist->nums[0];
									else
									  equalityVble[j] = inferlist->nums[0];
								}
							}
						}
						if (Rebuild_BDDx(j)==TRIV_UNSAT)
						  return TRIV_UNSAT;
					}
				}
//				amount[inferlist->nums[0]].tail->next = amount[-inferlist->nums[1]].head;
//				amount[inferlist->nums[0]].tail = amount[-inferlist->nums[1]].tail;
//				amount[-inferlist->nums[1]].head = NULL;
//				amount[-inferlist->nums[1]].tail = NULL;
				//verifyCircuit(-inferlist->nums[1]);
			}
		} else {
			if (inferlist->nums[0] > 0) {
				Setting_Pos++;
//            D_3(print_nonroller(););
				for(int iter = 0; iter<str_length; iter++)
					d3_printf1("\b");
				str_length = 0;  
				d3_printf2 ("{%d=T}", abs (inferlist->nums[0]));
				variablelist[inferlist->nums[0]].true_false = 1;
				for (llist * k = amount[inferlist->nums[0]].head; k != NULL;) {
					int j = k->num;
					k = k->next; //This must be done here because k could be deleted in Rebuild_BDDx()
					BDDNode *before = functions[j];
					functions[j] = set_variable (functions[j], inferlist->nums[0], 1);
					if (functions[j] == false_ptr)
					  return TRIV_UNSAT;
					if (before != functions[j]) {
						if ((functionType[j] == AND && equalityVble[j] < 0)
							 ||(functionType[j] == OR  && equalityVble[j] > 0)) {
							if (abs (equalityVble[j]) == inferlist->nums[0]) {
								functionType[j] = PLAINOR;	//a v b v c
								//independantVars[equalityVble[j]] = 1;
								equalityVble[j] = 0;
							}
						}
						if (Rebuild_BDDx(j)==TRIV_UNSAT)
						  return TRIV_UNSAT;
					}
				}
				//verifyCircuit(inferlist->nums[0]);
			} else {
				Setting_Neg++;
//            D_3(print_nonroller(););
				for(int iter = 0; iter<str_length; iter++)
					d3_printf1("\b");
				str_length = 0;  
				d3_printf2 ("{%d=F}", abs (inferlist->nums[0]));
				variablelist[-inferlist->nums[0]].true_false = 0;
				for (llist * k = amount[-inferlist->nums[0]].head; k != NULL;) {
					int j = k->num;
					k = k->next; //This must be done here because k could be deleted in Rebuild_BDDx()
					BDDNode *before = functions[j];
					functions[j] = set_variable (functions[j], -inferlist->nums[0], 0);
					if (functions[j] == false_ptr)
					  return TRIV_UNSAT;
					if (before != functions[j]) {
						if ((functionType[j] == AND && equalityVble[j] > 0)
							 ||(functionType[j] == OR  && equalityVble[j] < 0)) {
							if (abs (equalityVble[j]) == abs(inferlist->nums[0])) {
								functionType[j] = PLAINOR;	//a v b v c
								//independantVars[equalityVble[j]] = 1;
								equalityVble[j] = 0;
							}
						}
						if (Rebuild_BDDx(j)==TRIV_UNSAT)
						  return TRIV_UNSAT;
					}
				}
				//verifyCircuit(-inferlist->nums[0]);
			}
		}
		temp = inferlist;
		inferlist = inferlist->next;
		delete temp;
		temp = NULL;
   }
	inferlist = new infer; //NEW
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
	delete temp;
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
		delete temp;
		temp = NULL;
	}
	inferlist = new infer;
	inferlist->next = NULL;
	lastinfer = inferlist;
	return ret;
}

void printBDDInfs(BDDNode *bdd) {
	for(infer *iterator = bdd->inferences; iterator != NULL; iterator = iterator->next) {
		d3_printf3 ("{%d, %d}", iterator->nums[0], iterator->nums[1]);
		//fprintf(stderr, "%d|%d, %d|", x, iterator->nums[0], iterator->nums[1]);
	}
}

int Rebuild_BDDx (int x) {
	SetRepeats(x);
	Result *result;
	
	if (functions[x] == false_ptr)
	  return TRIV_UNSAT;

//	countFalses(functions[x]);
//	countFalses(functions[x]);
	
	//Get Inferences
	
	//This still does more new statements than necessary, but it's definetly faster.

	infer *lastiter = NULL;
	infer *startiter = NULL;
	if(functions[x]->inferences != NULL) {
		lastiter = new infer;
		startiter = lastiter;
		for(infer *iterator = functions[x]->inferences; iterator != NULL; iterator = iterator->next) {
			lastiter->next = new infer;
			lastiter = lastiter->next;
			lastiter->nums[0] = iterator->nums[0];
			lastiter->nums[1] = iterator->nums[1];
			//fprintf(stderr, "%d|%d, %d|", x, iterator->nums[0], iterator->nums[1]);
		}
		lastiter->next = NULL;
	}
	
	if(startiter!=NULL) {
		lastinfer->next = startiter->next;
		delete startiter;
	} else lastinfer->next = NULL;
	
	infer *previous = lastinfer;
	
	//Remove duplicate inferences
	for (infer * iterator = lastinfer->next; iterator != NULL;
		  iterator = iterator->next) {
		//fprintf(stderr, "%d(%d, %d)", x, iterator->nums[0], iterator->nums[1]);
		if (iterator->nums[1] == 0) {
			if (iterator->nums[0] > 0) {
				result = l->insertEquiv (iterator->nums[0], T);
				if (result == NULL) {
					//fprintf(stderr, "(%d, %d)", iterator->nums[0], iterator->nums[1]);
					infer *temp = iterator;
					previous->next = iterator->next;	//Skip over inference already applied
					iterator = previous;
					delete temp;	//Delete the inference
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
					delete temp;	//Delete the inference
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
					delete temp;	//Delete the inference
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
					delete temp;	//Delete the inference
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
		//fprintf(stderr, "!%d, %d!\n", iterator->nums[0], iterator->nums[1]);
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

	assert(x!=nmbrFunctions+1);
	long y = 0;
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
						delete temp;
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
				llist *newllist = new llist;
				//fprintf(stderr, "{adding1 %d %d}\n", x, tempint[a]);
				newllist->num = x;
				newllist->next = NULL;
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
					delete temp;
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
			llist *newllist = new llist;
			//fprintf(stderr, "{adding2 %d %d}\n", x, tempint[a]);
			newllist->num = x;
			newllist->next = NULL;
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
		//Done adding BDD x into the inferences lists.
		// delete [] variables[x].num;
	}

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

   //A line like this would be better placed in smurffactory
	//where the smurfs are made...maybe...we like to be able to preprocess
	//things that have been made small...
	//if (length[x] < 3)
	//  functionType[x] = UNSURE;

	if (DO_INFERENCES) {
		return Do_Apply_Inferences();
	}
	
	return PREP_NO_CHANGE;
}

int
Rebuild_BDD (BDDNode *bdd, int *bdd_length, int *&bdd_vars)
{
	long y = 0;
	Result *result;
	
	if (bdd == false_ptr)
	  return TRIV_UNSAT;
	
	//Get Inferences
	
	//This still does more new statements than necessary, but it's definetly faster.
	
	infer *lastiter = NULL;
	infer *startiter = NULL;
	if(bdd->inferences != NULL) {
		lastiter = new infer;
		startiter = lastiter;
		for(infer *iterator = bdd->inferences; iterator != NULL; iterator = iterator->next) {
			lastiter->next = new infer;
			lastiter = lastiter->next;
			lastiter->nums[0] = iterator->nums[0];
			lastiter->nums[1] = iterator->nums[1];
		}
		lastiter->next = NULL;
	}
	
	if(startiter!=NULL) {
		lastinfer->next = startiter->next;
		delete startiter;
	}
	
	infer *previous = lastinfer;
	
	//Remove duplicate inferences
	for (infer * iterator = lastinfer->next; iterator != NULL;
		  iterator = iterator->next) {
		if (iterator->nums[1] == 0) {
			if (iterator->nums[0] > 0)	{
				result = l->insertEquiv (iterator->nums[0], T);
				if (result == NULL) {
					infer *temp = iterator;
					previous->next = iterator->next;	//Skip over inference already applied
					iterator = previous;
					delete temp;	//Delete the inference
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
					infer *temp = iterator;
					previous->next = iterator->next;	//Skip over inference already applied
					iterator = previous;
					delete temp;	//Delete the inference
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
				if (result == NULL) {
					infer *temp = iterator;
					previous->next = iterator->next;	//Skip over inference already applied
					iterator = previous;
					delete temp;	//Delete the inference
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
				if (result == NULL) {
					infer *temp = iterator;
					previous->next = iterator->next;	//Skip over inference already applied
					iterator = previous;
					delete temp;	//Delete the inference
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
		if (iterator->nums[1] != 0)
		  assert (iterator->nums[0] > 0);
		assert (iterator->nums[0] != 0);
		previous = iterator;
	}
	
	lastinfer = previous;
	lastinfer->next = NULL;
        
	unravelBDD(&y, &bdd_tempint_max, &bdd_tempint, bdd);
   int *tempint = bdd_tempint;;
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
