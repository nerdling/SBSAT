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
/**                              equivclass.h
 *                                                                            
 * Maintain sets of equivalent and opposite valued variables using inverted
 * trees and path compression when joining.  Equivalences can only be set,
 * not removed.  Assumes variables are numbered (that is, of type int).
 * 
 * class Linear API:
 *    Linear (int inp, int True, int False): Constructor, inp is the maximum
 *    numbered input variable, True is the number of the variable representing
 *    value true, False is the number of the variable representing value false.
 *
 *    Result *insertEquiv (int x, int y): Make variable x equivalent to 
 *    variable y.  Return a pair of int values: the left value is the 
 *    smallest numbered variable which is either equivalent to or opposite
 *    of x before the insertion; the right value is the smallest numbered 
 *    variable that is either equivalent to or opposite of y before the
 *    insertion.  The left (right) value takes a negative sign if the 
 *    smallest variable is in the same equivalence class of x (y).
 *
 *    Result *insertOppos (int x, int y): Make variable x opposite to 
 *    variable y.  Return a pair of int values: the left value is the 
 *    smallest numbered variable which is either equivalent to or opposite
 *    of x before the insertion; the right value is the smallest numbered 
 *    variable that is either equivalent to or opposite of y before the
 *    insertion.  The left (right) value takes a negative sign if the 
 *    smallest variable is in the opposite equivalence class of x (y).
 * 
 *    int isEquiv (int x, int y): Returns 0 if no equivalence or error, 
 *    1 if equivalent, -1 if opposite.
 * 
 *    int *equivalent (int x): Returns a -1 terminated list of variables
 *    equivalent to x or null on error.
 * 
 *    int *opposite (int x):  Returns a -1 terminated list of variables
 *    opposite to x or null on error.
 * 
 *    int *valueOfTrue (): Returns -1 terminated list of variables 
 *    equivalent to true.
 *
 *    int *valueOfFalse (): Returns -1 terminated list of variables 
 *    equivalent to false.
 * 
 *    int equivCount (int x): Returns the number of other variables
 *    equivalent to x.
 *
 *    int opposCount (int x): Returns the number of other variables 
 *    opposite to x.
 * 
 *    void printEquivalences (): Print lists of equivalence classes.  A line
 *    in the list such as:
 * 
 *      [F] 16 F 58 99 98 || [T] 73 116 128 T
 * 
 *    means 99, 98, and 16 are equivalent to false and opposite to 73, 116, 
 *    128 and true.  Another example line is:
 * 
 *      [16] 16 99 98 || [73] 73 116 128
 *  
 *    The number in brackets is the variable which represents the equivalent 
 *    class (the root of the inverted tree).
**/
#ifndef EQUIVCLASS_H
#define EQUIVCLASS_H

#define null -1
#define F (numinp+3)     //False
#define T (numinp+2)     //True            Head node must be smaller than T and F

typedef struct linear_rec {
    int no_inp_vars; // The number of input variables                   
    int index;       // Number of vectors we have                       
    int equiv_idx;   // how many such nodes are in use.                 
    int Tr,Fa;       // Symbols for true and false                      
} Vars;

typedef struct /*result*/ {
   int left, rght;
} Result;

//   frame - block of memory containing the number of input variables, number 
//           of output variables, the number of words in each 0-1 row (also   
//           called vector), the number of rows (index), the identities of    
//           T (true) and F (false), and "index" number of "row records".     
//           Each "row record" contains a consecutive "vec_size" number of    
//           words containing the 0-1 patterns, the "vec_size" value, the     
//           type of function (even/odd parity), in what column the bit of a  
//           row occupies in the diagonal component, and equivalence class    
//           variables.  Access to these values in the frame variable is      
//           accomplished as follows:                                         
//                                                                            
//           Equivalence Class variables (also see Figure 2):                 
//               equiv_fwd: ( int *)&frame[equiv_fwd_ref]                
//               equiv_cnt: ( int *)&frame[equiv_cnt_ref]                
//               equiv_bck: ( int *)&frame[equiv_bck_ref]                
//               equiv_end: ( int *)&frame[equiv_end_ref]                
//               equiv_rgt: ( int *)&frame[equiv_rgt_ref]                
//               equiv_lft: ( int *)&frame[equiv_lft_ref]                
//               equiv_res: ( int *)&frame[equiv_res_ref]                
//                                                                            

// Figure 1. 
// Equivalence classes are maintained using inverted trees.  These are        
// adjusted when truth values are assigned to variables.  The following       
// diagram illustrates the structure of the data.  An explanation follows.    
// Please note that not all links are shown in this example.                  
//                                                                            
// equiv_lft[4]=3 |=======| equiv_rgt[4]=8                                    
//      +---------*   4   *---------+                                         
//      |    +--->|=======|<---+    |                       +--> -1           
//      |    |  equiv_cnt[3]   |    |                       |                 
//      +->|=*=|    =6       |=*=|<-+  equiv_fwd[2]=8     |=*=|               
//         * 3 |             * 8 |<-----------+     +-----* 6 |<-------+      
//      +->|=*=|<-+          |=*=|<-+         |     |     |=*=|<-+     |      
//      |         |                 |         |     |       |    |     |      
//      |         |                 |         |     |   +---|----|-----|----+ 
//      |         |                 |         |     |   |   |    |     |    | 
//    |=*=|     |=*=|             |=*=|     |=*=|   | +-|---|->|=*=| |=*=|  | 
//    * 4 |     * 1 |<--------+   * 9 |     * 2 |   | | +---|--* 10| * 12|<-+ 
//    |=*=|<-+  |=*=|<-+      |   |=*=|<-+  |=*=|   | |   +-|->|=*=| |=*=|<-+ 
//           |         |      |          |          | |   |  \              | 
//           |         |      |          |          | |   |   \-------------+ 
//         |=*=|     |=*=|  |=*=|      |=*=|        | | |=*=|                 
//         * 13|     * 5 |  * 11|      * 0 |        | +-* 7 |                 
//         |=*=|     |=*=|  |=*=|      |=*=|        +-->|=*=|                 
//                                           equiv_bck[6]=7                   
//                                                                            
//  For this example: variables 8 and 3 have opposite value, variables 1 and  
//    4 have the same value, it is not known whether variables 10 and 13      
//    must have the same or opposite values, there are no known variables     
//    which must have value opposite to that of variable 7.                   
//                                                                            
//  LEGEND                                                                    
//                                                                            
//  *---->    These are data links.  The "*" is the point of origin (an index 
//            into some array) and the ">" is the target (the value of the    
//            array element.  Names are given to some of the links in the     
//            diagram for illustration purposes.  Explanations are given      
//            below.                                                          
//                                                                            
// |=======|  These are "super" nodes which connect two equivalence classes   
// * 1   2 *  with the interpretation that variables of one class have value  
// |=======|  opposite to the variables of the other class.  There are two    
//            pointers denoted by "*".  For super node "x":                   
//              1. equiv_lft[x] points to the root of one equivalence class   
//              2. equiv_rgt[x] points to the root of the other.              
//            There is no particular significance to the value of x since     
//            super nodes are taken from an available pool of super nodes,    
//            as needed.                                                      
//     1                                                                      
//   |=*=|    These are the variables arranged in inverted trees to identify  
// 2 *   |    sets or equivalence classes of variables that must have the     
//   |=*=|    same value.  There are three pointers denoted by "*".  Each is  
//     3      explained for variable "x":                                     
//              1. equiv_fwd[x] has a value which is that of another variable 
//                 in the equivalence class, or is a negative number.  If it  
//                 is a negative number then x is the root of the equivalence 
//                 class.  If it is a negative number less than -1 then it is 
//                 also pointing to a "super" node and there exists a class   
//                 variables of opposite value to x and its equivalence class.
//              2. equiv_bck[x] points to another variable in the class       
//                 containing x.  Such pointers form a linked list linking    
//                 all variables of an equivalence class starting at the root.
//                 This enables a quick response to a query such as "what     
//                 variables are equivalent to x?".                           
//              3. equiv_end[x] points to a variable in the equivalence class 
//                 which is at the end of the linked list formed by equiv_bck.
//                 This is used to merge two classes quickly when variables   
//                 of different classes have been discovered to be equivalent.
//             In addition to the pointers, there is a value equiv_cnt[x]     
//             which associates with variable x: this is valid only if x is a 
//             root of an equivalence class and is then the number of         
//             variables in the equivalence class.  This is used to perform   
//             path compression when merging and to answer queries about the  
//             size of an equivalence class.                                  
//                                                                            
// NOTE: Not all links are shown in the diagram                               

class Linear {
   Vars *rec;
   char *frame;
   Result result;
	
    int no_inp_vars; // The number of input variables 
    int index;       // Number of vectors we have 
    int equiv_idx;   // How many such nodes are in use.     
    int Tr,Fa;       // Symbols for true and false 

    int *equiv_fwd; // Structure for maintaining and finding equivalences 
    int *equiv_cnt; // Number of variables in an equiv tree - path comp. 
    int *equiv_min; // 
    int *equiv_bck; // Linked list downward from root of inverted tree    
    int *equiv_end; // Pointer to end of the bck linked list              
    int *equiv_rgt; // These three maintain nodes that point to trees of  
    int *equiv_lft; //   opposite equivalence.  The idx variable tells    
    int *equiv_res; // For returning the result of a query                
    int *xlist;     // Temporary space for equivalence operations         
    int *ylist;     // Temporary space for equivalence operations
   int frame_size;
   unsigned long frame_start;

 public:
   Linear (int inp, int True, int False) {
      if (inp >= True || inp >= False) {
	      cout << "Whoops! No. input vars must less than True and False\n";
	      cout << "var " << inp << " True " << True << " False " << False;
	      exit(0);
      }
      if (True == False) {
	      cout << "Whoops! True cannot be equal to False.\n";
	      exit(0);
      }
      int sze = (True > False) ? True : False; 
      sze++;
      int vars_sze      = sizeof(Vars);
      int equiv_fwd_sze = sizeof( int)*(sze);
      int equiv_bck_sze = sizeof( int)*(sze);
      int equiv_end_sze = sizeof( int)*(sze);
      int equiv_cnt_sze = sizeof( int)*(sze);
      int equiv_min_sze = sizeof( int)*(sze);
      int equiv_rgt_sze = sizeof( int)*(sze);
      int equiv_lft_sze = sizeof( int)*(sze);
      int equiv_res_sze = sizeof( int)*(sze);
      int xlist_sze     = sizeof( int)*inp;
      int ylist_sze     = sizeof( int)*inp;

      int vars_ref      = 0;
      int equiv_fwd_ref = vars_ref + vars_sze;
      int equiv_bck_ref = equiv_fwd_ref + equiv_fwd_sze;
      int equiv_end_ref = equiv_bck_ref + equiv_bck_sze;
      int equiv_cnt_ref = equiv_end_ref + equiv_end_sze;
      int equiv_min_ref = equiv_cnt_ref + equiv_cnt_sze;
      int equiv_rgt_ref = equiv_min_ref + equiv_min_sze;
      int equiv_lft_ref = equiv_rgt_ref + equiv_rgt_sze;
      int equiv_res_ref = equiv_lft_ref + equiv_lft_sze;
      int xlist_ref     = equiv_res_ref + equiv_res_sze;
      int ylist_ref     = xlist_ref     + xlist_sze;
      int vecs_v_ref    = ylist_ref     + ylist_sze;

      frame = (char *)calloc(1, vecs_v_ref);
      frame_start = (unsigned long)frame;

      rec = (Vars *)frame;
      no_inp_vars = rec->no_inp_vars = inp; // Number of input variables  
      index = rec->index = 0;
      equiv_idx = rec->equiv_idx = 3; // Can't use 0 since use minus to 
                                      // point to equiv node, can't use 1 
                                      // because -1 means not pointing to 
                                      // anything, can't use -2 since index 
                                      // 2 is taken for T and F
      Tr = rec->Tr = True;
      Fa = rec->Fa = False;
		
      // Use equiv for inverted trees with path compression 
      equiv_fwd = ( int *)(frame_start+equiv_fwd_ref);
      equiv_cnt = ( int *)(frame_start+equiv_cnt_ref);
      equiv_min = ( int *)(frame_start+equiv_min_ref);
      equiv_bck = ( int *)(frame_start+equiv_bck_ref); 
      equiv_end = ( int *)(frame_start+equiv_end_ref); 
      equiv_rgt = ( int *)(frame_start+equiv_rgt_ref);
      equiv_lft = ( int *)(frame_start+equiv_lft_ref); 
      equiv_res = ( int *)(frame_start+equiv_res_ref);

      for (int i=0 ; i < sze ; i++) {
	      equiv_fwd[i] = null;
	      equiv_cnt[i] = 1;
	      equiv_min[i] = i;
	      equiv_bck[i] = null;
	      equiv_end[i] = i;
	      equiv_rgt[i] = null;
	      equiv_lft[i] = null;
      }
      equiv_fwd[Tr] = -2;  // Create a super node for T and F immediately 
      equiv_fwd[Fa] = -2;
      equiv_rgt[2] = Tr;
      equiv_lft[2] = Fa;
      
      xlist = ( int *)(frame_start+xlist_ref); 
      ylist = ( int *)(frame_start+ylist_ref);

   }

   ~Linear () { free(frame); }

   // Make variable x equivalent to variable y.                            
   // 
   // Equivalences are stored as inverted trees.  All the arrays "equiv_*" 
   // have one entry per variable at least.  Array "equiv_fwd" points      
   // upward to a root "node" which is the identifier for an equivalence   
   // class.  Thus, for variable i, "equiv_fwd[i]" identifies the next     
   // variable in the path to the root from variable i's "node".  The root 
   // has the property that its value for "equiv_fwd" is negative.         
   //
   // To facilitate a query such as "which variables are variable x        
   // equivalent to" there is a linked list which threads through all      
   // "nodes" of the equivalence class.  This is done using "equiv_bck".   
   // Thus "equiv_bck[i]" is the next node on the thread through the class.
   // The last node in the thread is identified by a -1 value for          
   // "equiv_bck".  In order to quickly splice threads when merging due to 
   // discovery of equivalence, the link "equiv_end" is provided.  This    
   // points to the last "node" in the threaded list from the root node.   
   //
   // For efficiency, merging classes is done using path compression.      
   // Thus, a count of the number of variables in an equivalence class is  
   // maintained at the root.  This count is in "equiv_cnt".               
   //
   // We keep track of opposite valued equivalence classes by means of     
   // "equiv_rgt" and "equiv_lft".  These pairs may be thought of as       
   // "super" nodes pointing to two equivalence classes of opposite value. 
   // The original arrays are merely an available pool of "super" nodes    
   // and are used up as opposites are detected.  For example, suppose     
   // variable i is the root of one equivalence class and variable j is    
   // the root of another and it is discovered that these classes have     
   // opposite value (the first time for both classes).  Initially,        
   // "equiv_fwd[i]" and "equiv_fwd[j]" still have their original values   
   // of -1.  A number is picked based on the value of "equiv_idx".  Let   
   // this number be denoted by k.  Then "equiv_fwd[i]" and "equiv_fwd[j]" 
   // are both set to -k.  Also, either "equiv_rgt[k]" is set to i and     
   // "equiv_lft[k]" is set to j or the other way around.  A "equiv_lft"   
   // and "equiv_rgt" pair may be discarded when merging equivalence       
   // classes since their opposite classes must also be merged.  If this   
   // happens, that pair is never used again.                              
   //
	int get_equiv(int x) {
		int a;
		for (a=x ; equiv_fwd[a] >= 0 ; a=equiv_fwd[a]){}
		//d3_printf2("[%d]", equiv_lft[-equiv_fwd[a]]);
		return equiv_min[a];
	}
   
	Result *insertEquiv (int x, int y) {
      int xidx=0, yidx=0, a, b, i;
      int lft, rgt, tmp;
		
      for (a=x ; equiv_fwd[a] >= 0 ; a=equiv_fwd[a]) xlist[xidx++] = a;
      for (b=y ; equiv_fwd[b] >= 0 ; b=equiv_fwd[b]) ylist[yidx++] = b;
      if (a != b) { // x and y are in different equivalence classes   
			// if true: consistent 
			if (!(equiv_fwd[a] == equiv_fwd[b] && equiv_fwd[a] <= -2)) { 
				
				lft = equiv_min[a];
				rgt = equiv_min[b];
				// if a has a super node and a is not T or F then see if other
				// side of equiv class tree structure has a min smaller than a's
				if (equiv_fwd[a] <= -2 && lft < no_inp_vars) {
					if (equiv_lft[-equiv_fwd[a]] == a) {
						if ((tmp = equiv_min[equiv_rgt[-equiv_fwd[a]]]) < lft)
						  lft = -tmp;
					} else {
						if ((tmp = equiv_min[equiv_lft[-equiv_fwd[a]]]) < lft) 
						  lft = -tmp;
					}
				}
				// if b has a super node and b is not T or F then see if other
				// side of equiv class tree structure has a min smaller than b's
				if (equiv_fwd[b] <= -2 && rgt < no_inp_vars) {
					if (equiv_lft[-equiv_fwd[b]] == b) {
						if ((tmp = equiv_min[equiv_rgt[-equiv_fwd[b]]]) < rgt) 
						  rgt = -tmp;
					} else {
						if ((tmp = equiv_min[equiv_lft[-equiv_fwd[b]]]) < rgt) 
						  rgt = -tmp;
					}
				}
				
            if(lft < 0 && rgt < Tr) {
					lft = -lft;
					rgt = -rgt;
				}
            
            result.left = lft;
				result.rght = rgt;
				
				int root = (equiv_cnt[a] < equiv_cnt[b]) ? b : a;
				int auxx = (equiv_cnt[a] < equiv_cnt[b]) ? a : b;
				if (equiv_bck[root] == null) {
					equiv_bck[root] = auxx;
				} else {
					equiv_bck[equiv_end[root]] = auxx;
				}
				equiv_end[root] = equiv_end[auxx];    // due to self loop on "end" 
				equiv_cnt[root] += equiv_cnt[auxx];
				if ((equiv_min[root] < no_inp_vars && 
					  equiv_min[auxx] < equiv_min[root]) || 
					 equiv_min[auxx] >= no_inp_vars) 
				  equiv_min[root] = equiv_min[auxx];
				int auxx_super = -equiv_fwd[auxx]; // Should be a pos number 
				int root_super = -equiv_fwd[root]; // Should be a pos number 
				equiv_fwd[auxx] = root;
				
				// Straighten out the opposite classes     
				// Case 0: Auxx has no super - do nothing  
				if (auxx_super > 1) {
					// Case 1: Root has no super but auxx does 
					if (root_super == 1) {
						if (equiv_lft[auxx_super] == auxx) {
							equiv_lft[auxx_super] = root;
						} else {
							equiv_rgt[auxx_super] = root;
						}
						equiv_fwd[root] = -auxx_super;
					} else if (root_super > 1) {
						// Case 2: Root and auxx have a super, they must be different
						int auxx_opp = (equiv_lft[auxx_super] == auxx) ?
						  equiv_rgt[auxx_super] : equiv_lft[auxx_super];
						int root_opp = (equiv_lft[root_super] == root) ?
						  equiv_rgt[root_super] : equiv_lft[root_super];
						// Join opp trees and attach to opposite of new equiv class 
						if (equiv_cnt[auxx_opp] < equiv_cnt[root_opp]) {
							equiv_fwd[auxx_opp] = root_opp;
							if (equiv_bck[root_opp] == -1) {
								equiv_bck[root_opp] = auxx_opp;
							} else {
								equiv_bck[equiv_end[root_opp]] = auxx_opp;
							}
							equiv_end[root_opp] = equiv_end[auxx_opp];
							equiv_cnt[root_opp] += equiv_cnt[auxx_opp];
							if ((equiv_min[root_opp] < no_inp_vars && 
								  equiv_min[auxx_opp] < equiv_min[root_opp]) ||
								 equiv_min[auxx_opp] >= no_inp_vars) 
							  equiv_min[root_opp] = equiv_min[auxx_opp];
							equiv_fwd[root_opp] = -root_super;
						} else {
							equiv_fwd[root_opp] = auxx_opp;
							if (equiv_bck[auxx_opp] == -1) {
								equiv_bck[auxx_opp] = root_opp;
							} else {
								equiv_bck[equiv_end[auxx_opp]] = root_opp;
							}
							equiv_end[auxx_opp] = equiv_end[root_opp];
							equiv_cnt[auxx_opp] += equiv_cnt[root_opp];
							if ((equiv_min[auxx_opp] < no_inp_vars && 
								  equiv_min[root_opp] < equiv_min[auxx_opp]) ||
								 equiv_min[root_opp] >= no_inp_vars)
							  equiv_min[auxx_opp] = equiv_min[root_opp]; 
							equiv_fwd[auxx_opp] = -root_super;
							if (equiv_rgt[root_super] == root) {
								equiv_lft[root_super] = auxx_opp;
							} else {
								equiv_rgt[root_super] = auxx_opp;
							}
						}
					}
				}
				for (i=0 ; i < xidx ; i++) equiv_fwd[xlist[i]] = root;
				for (i=0 ; i < yidx ; i++) equiv_fwd[ylist[i]] = root;
			} else { // otherwise there is an inconsistency                  
				result.left = Tr;
				result.rght = Fa;
			}
      } else return NULL; // Already in same class - no change 
      return &result;
   }
   
   // Make x and y have opposite value.  For comments, see notes above 
   // code for "insertEquiv" above.                                    
   Result *insertOppos (int x, int y) {
      int xidx=0, yidx=0, a, b, lft, rgt, tmp, i;
      
      for (a=x ; equiv_fwd[a] >= 0 ; a=equiv_fwd[a]) xlist[xidx++] = a;
      for (b=y ; equiv_fwd[b] >= 0 ; b=equiv_fwd[b]) ylist[yidx++] = b;
      if (a == b) {  // Inconsistent 
			result.left = Tr;
			result.rght = Fa;
			return &result;
      }
      if (equiv_fwd[a] == equiv_fwd[b] && equiv_fwd[a] <= -2) { // Already OK 
			return NULL;
      }
      
      lft = -equiv_min[a];
      rgt = -equiv_min[b];

      // if a has a super node and a is not T or F then see if other
      // side of equiv class tree structure has a min smaller than a's
      if (equiv_fwd[a] <= -2 && -lft < no_inp_vars) {
			if (equiv_lft[-equiv_fwd[a]] == a) {
				if ((tmp = equiv_min[equiv_rgt[-equiv_fwd[a]]]) < -lft) lft = tmp;
			} else {
				if ((tmp = equiv_min[equiv_lft[-equiv_fwd[a]]]) < -lft) lft = tmp;
			}
      } else if (-lft >= no_inp_vars) {
			if (-lft == Fa) lft = Tr;
      }
      // if b has a super node and b is not T or F then see if other
      // side of equiv class tree structure has a min smaller than b's
		if (equiv_fwd[b] <= -2 && -rgt < no_inp_vars) {
			if (equiv_lft[-equiv_fwd[b]] == b) {
				if ((tmp = equiv_min[equiv_rgt[-equiv_fwd[b]]]) < -rgt) rgt = tmp;
			} else {
				if ((tmp = equiv_min[equiv_lft[-equiv_fwd[b]]]) < -rgt) rgt = tmp;
			}
		} else if (-rgt >= no_inp_vars) {
			if (-rgt == Tr) rgt = Fa;  //Iffy Here Sean!
			if (-rgt == Fa) rgt = Tr;
		}
      
      if(lft < 0 && rgt < Tr) {
			lft = -lft;
			rgt = -rgt;
      }
      result.left = lft;
      result.rght = rgt;		
		
      int a_super = -equiv_fwd[a]; // Should be a positive number 
      int b_super = -equiv_fwd[b]; // Should be a positive number 
      
      // If neither class a nor b has a super 
      if (a_super == 1 && b_super == 1) {
			equiv_lft[equiv_idx] = a;
			equiv_rgt[equiv_idx] = b;
			equiv_fwd[a] = -equiv_idx;
			equiv_fwd[b] = -equiv_idx;
			equiv_idx++;
			rec->equiv_idx++;
			for (i=0 ; i < xidx ; i++) equiv_fwd[xlist[i]] = a;
			for (i=0 ; i < yidx ; i++) equiv_fwd[ylist[i]] = b;
      } else if (a_super > 1 && b_super == 1) {
			// If b has no super but a does - merge b with a's opposite 
			int r = (equiv_rgt[a_super] == a) ?
			  equiv_lft[a_super] : equiv_rgt[a_super];
			int root = (equiv_cnt[r] < equiv_cnt[b]) ? b : r;
			int auxx = (equiv_cnt[r] < equiv_cnt[b]) ? r : b;
			if (equiv_bck[root] == -1) {
				equiv_bck[root] = auxx;
			} else {
				equiv_bck[equiv_end[root]] = auxx;
			}
			equiv_end[root] = equiv_end[auxx]; // because of self loop on "end" 
			equiv_cnt[root] += equiv_cnt[auxx];
			if ((equiv_min[root] < no_inp_vars && 
				  equiv_min[auxx] < equiv_min[root]) ||
				 equiv_min[auxx] >= no_inp_vars) 
			  equiv_min[root] = equiv_min[auxx];
			equiv_fwd[auxx] = root;
			equiv_fwd[root] = -a_super;
			if (equiv_rgt[a_super] == a) equiv_lft[a_super] = root;
         else equiv_rgt[a_super] = root;
			for (i=0 ; i < xidx ; i++) equiv_fwd[xlist[i]] = a;
			for (i=0 ; i < yidx ; i++) equiv_fwd[ylist[i]] = root;
      } else if (a_super == 1 && b_super > 1) {
			// If b has a super but a does not - merge a with b's opposite 
			int r = (equiv_rgt[b_super] == b) ?
			  equiv_lft[b_super] : equiv_rgt[b_super];
			int root = (equiv_cnt[r] < equiv_cnt[a]) ? a : r;
			int auxx = (equiv_cnt[r] < equiv_cnt[a]) ? r : a;
			if (equiv_bck[root] == -1) {
				equiv_bck[root] = auxx;
			} else {
				equiv_bck[equiv_end[root]] = auxx;
			}
			equiv_end[root] = equiv_end[auxx]; // because of self loop on "end" 
			equiv_cnt[root] += equiv_cnt[auxx];
			if ((equiv_min[root] < no_inp_vars && 
				  equiv_min[auxx] < equiv_min[root]) ||
				 equiv_min[auxx] >= no_inp_vars) 
			  equiv_min[root] = equiv_min[auxx];
			equiv_fwd[auxx] = root;
			equiv_fwd[root] = -b_super;
			if (equiv_rgt[b_super] == b) equiv_lft[b_super] = root;
         else equiv_rgt[b_super] = root;
			for (i=0 ; i < xidx ; i++) equiv_fwd[xlist[i]] = root;
			for (i=0 ; i < yidx ; i++) equiv_fwd[ylist[i]] = b;
      } else {
			// If a and b have a super 
			int rb = (equiv_rgt[b_super] == b) ?
			  equiv_lft[b_super] : equiv_rgt[b_super];
			int ra = (equiv_rgt[a_super] == a) ?
			  equiv_lft[a_super] : equiv_rgt[a_super];  // ra, rb are opposites
			
			int root1 = (equiv_cnt[rb] < equiv_cnt[a]) ? a : rb;
			int auxx1 = (equiv_cnt[rb] < equiv_cnt[a]) ? rb : a;
			if (equiv_bck[root1] == -1) {
				equiv_bck[root1] = auxx1;
			} else {
				equiv_bck[equiv_end[root1]] = auxx1;
			}
			equiv_end[root1] = equiv_end[auxx1]; // because of self loop on "end"
			equiv_cnt[root1] += equiv_cnt[auxx1];
			if ((equiv_min[root1] < no_inp_vars && 
				  equiv_min[auxx1] < equiv_min[root1]) ||
				 equiv_min[auxx1] >= no_inp_vars) 
			  equiv_min[root1] = equiv_min[auxx1];
			equiv_fwd[auxx1] = root1;
			
			int root2 = (equiv_cnt[ra] < equiv_cnt[b]) ? b : ra;
			int auxx2 = (equiv_cnt[ra] < equiv_cnt[b]) ? ra : b;
			if (equiv_bck[root2] == -1) {
				equiv_bck[root2] = auxx2;
			} else {
				equiv_bck[equiv_end[root2]] = auxx2;
			}
			equiv_end[root2] = equiv_end[auxx2]; // because of self loop on "end" 
			equiv_cnt[root2] += equiv_cnt[auxx2];
			if ((equiv_min[root2] < no_inp_vars && 
				  equiv_min[auxx2] < equiv_min[root2]) || 
				 equiv_min[auxx2] >= no_inp_vars) 
			  equiv_min[root2] = equiv_min[auxx2];
			equiv_fwd[auxx2] = root2;
			
			equiv_fwd[root1] = -a_super;
			equiv_fwd[root2] = -a_super;			
			equiv_rgt[a_super] = root1;
			equiv_lft[a_super] = root2;
			for (i=0 ; i < xidx ; i++) equiv_fwd[xlist[i]] = root1;
			for (i=0 ; i < yidx ; i++) equiv_fwd[ylist[i]] = root2;
      }
      return &result;
   }
   
   // Returns 0 if no equivalence or error, 1 if equivalent, -1 if opposite
   int isEquiv (int x, int y) {
      int a, b;
      
      if (x < 0 || y < 0) return 0;
      for (a=x ; equiv_fwd[a] >= 0 ; a=equiv_fwd[a]);
      for (b=y ; equiv_fwd[b] >= 0 ; b=equiv_fwd[b]);
      if (a == b) return 1;
      if (equiv_fwd[a] == equiv_fwd[b] && equiv_fwd[a] < -1) return null;
      return 0;
   }

   // Returns a -1 terminated list of vars equivalent to x or null on error
   int *equivalent (int x) {
      int a, i=0;
      
      if (x < 0) { equiv_res[0] = null; return equiv_res; }
      for (a=x ; equiv_fwd[a] >= 0 ; a=equiv_fwd[a]);
      while (a >= 0) {
			if (a != x) equiv_res[i++] = a;
			a = equiv_bck[a];
      }
      equiv_res[i] = null;
      return equiv_res;
   }
   
   // Returns a -1 terminated list of vars opposite to x or null on error
   int *opposite (int x) {
      int a, i=0;
      
      if (x < 0) { equiv_res[0] = null; return equiv_res; }
      for (a=x ; equiv_fwd[a] >= 0 ; a=equiv_fwd[a]);
      if (equiv_fwd[a] == null) {
			equiv_res[0] = null;
      } else {
			int root_super = -equiv_fwd[a];
			a = (equiv_rgt[root_super] == a) ?
			  equiv_lft[root_super] : equiv_rgt[root_super];
			while (a >= 0) {
				if (a != x) equiv_res[i++] = a;
				a = equiv_bck[a];
			}
			equiv_res[i] = null;
      }
      return equiv_res;
   }
   
   // Returns -1 terminated list of variables equivalent to T or F 
   int *valueOfTrue ()  { return equivalent(Tr); }
   int *valueOfFalse () { return equivalent(Fa); }
   
   // Returns the number of other variables equivalent to x 
   int equivCount (int x) {
      int a;
      for (a=x ; equiv_fwd[a] >= 0 ; a=equiv_fwd[a]);
      return equiv_cnt[a] - 1;
   }
   
   // Returns the number of other variables opposite to x 
   int opposCount (int x) {
      int a;
      for (a=x ; equiv_fwd[a] >= 0 ; a=equiv_fwd[a]);
      int s = -equiv_fwd[a];
      int other = (equiv_lft[s] == a) ? equiv_rgt[s] : equiv_lft[s];
      return equiv_cnt[other] - 1;
   }
   
   void printEquivalences () {
   int x;

      for (int i=0 ; i < no_inp_vars ; i++) {
			if (equiv_fwd[i] < 0) {
				if (equiv_min[i] == Tr)
				  cout << "[T] ";
				else if (equiv_min[i] == Fa)
				  cout << "[F] ";
				else
				  cout << "[" << equiv_min[i] << "] ";
				if (i == Tr) cout << "T "; else 
				  if (i == Fa) cout << "F "; else
				  cout << i << " ";
				for (x=equiv_bck[i] ; x != null ; x=equiv_bck[x]) {
					if (x == Tr) cout << "T "; else
					  if (x == Fa) cout << "F "; else
					  cout << x << " "; 
				}
				cout << "|| ";
				if (equiv_fwd[i] < -1) {
					int super = -equiv_fwd[i];
					int root = (equiv_lft[super] == i) ? 
					  equiv_rgt[super] : equiv_lft[super];
					if (equiv_min[root] == Tr)
					  cout << "[T] ";
					else if (equiv_min[root] == Fa)
					  cout << "[F] ";
					else
					  cout << "[" << equiv_min[root] << "] ";
					if (root == Tr) cout << "T "; else
					  if (root == Fa) cout << "F "; else
					  cout << root << " ";
					for (x=equiv_bck[root] ; x != null ; x=equiv_bck[x]) {
						if (x == Tr) cout << "T "; else
						  if (x == Fa) cout << "F "; else
						  cout << x << " ";
					}
				} 
				cout << "\n";
				flush(cout);
			}
      }
      if (equiv_fwd[Tr] == equiv_fwd[Fa] && equiv_fwd[Tr] != -1) {
			// Tr 
			cout << "T ";
			for (x=equiv_bck[Tr] ; x != null ; x=equiv_bck[x])
			  cout << x << " ";
			cout << "|| F ";
			// Fa 
			for (x=equiv_bck[Fa] ; x != null ; x=equiv_bck[x])
			  cout << x << " ";
			cout << "\n";
      }
      cout << "-------------------------\n";
   }
   
   void printEquivalence (int x) {
      int a;
      for (a=x ; equiv_fwd[a] >= 0 ; a=equiv_fwd[a]);
      for ( ; a != null ; a=equiv_bck[a]) cout << a << " ";
      cout << "\n";
   }
   
   void printOpposite (int x) {
      int a;
      for (a=x ; equiv_fwd[a] >= 0 ; a=equiv_fwd[a]);
      int super = -equiv_fwd[a];
      int root = (equiv_lft[super] == a) ? equiv_rgt[super] : equiv_lft[super];
      for (a=root ; a != null ; a=equiv_bck[a]) cout << a << " ";
      cout << "\n";
   }
   
   void printNoLinksToRoot () {
      cout << "Links: ";
      for (int i=0 ; i < no_inp_vars ; i++) {
			int j=0;
			for (int a=i ; equiv_fwd[a] >= 0 ; j++, a=equiv_fwd[a]);
			cout << j << " ";
      }
      cout << "\n";
   }
   
   void printEquivClassCounts () {
      cout << "EqCnt (Var:cnt): ";
      for (int i=0 ; i < no_inp_vars ; i++) {
			if (equiv_fwd[i] < 0) {
				cout << i << ":" << equiv_cnt[i] << " ";
			}
      }
      cout << "\n";		
   }
   
   void printEquivVarCount () {
      cout << "EqVarCnt (Var:cnt): ";
      for (int i=0 ; i < no_inp_vars ; i++) {
	 cout << i << ":" << equivCount(i) << " ";
      }
      cout << "\n";
   }
   
   void printOpposVarCount () {
      cout << "OpVarCount (Var:cnt): ";
      for (int i=0 ; i < no_inp_vars ; i++) {
			cout << i << ":" << opposCount(i) << " ";
      }
      cout << "\n";
   }
   
   void printWhetherEquiv () {
   int i;
      cout << "Equivalents:\n    ";
      for (i=0 ; i < no_inp_vars ; i++) printf("%3d",i);
      cout << "\n";
      cout << "     ---------------------------------------------------------------\n";
      for (i=0 ; i < no_inp_vars ; i++) {
			printf("%-3d:",i);
			for (int j=0 ; j < no_inp_vars ; j++) {
				if (i == j) printf("   "); else printf("%3d",isEquiv(i,j));
			}
			printf("\n");
      }
   }
};
#endif
