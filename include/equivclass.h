/* =========FOR INTERNAL USE ONLY. NO DISTRIBUTION PLEASE ========== */

/*********************************************************************
 Copyright 1999-2004, University of Cincinnati.  All rights reserved.
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

#ifndef EQUIVCLASS_H
#define EQUIVCLASS_H

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

#define null -1

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
   Linear (int inp, int True, int False);
   ~Linear ();

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
	int get_equiv(int x);
	Result *insertEquiv (int x, int y);
   
   // Make x and y have opposite value.  For comments, see notes above 
   // code for "insertEquiv" above.                                    
   Result *insertOppos (int x, int y);
   
   // Returns 0 if no equivalence or error, 1 if equivalent, -1 if opposite
   int isEquiv (int x, int y);

   // Returns a -1 terminated list of vars equivalent to x or null on error
   int *equivalent (int x);
   
   // Returns a -1 terminated list of vars opposite to x or null on error
   int *opposite (int x);
   
   // Returns -1 terminated list of variables equivalent to T or F 
   int *valueOfTrue ();
   int *valueOfFalse ();
   
   // Returns the number of other variables equivalent to x 
   int equivCount (int x);
   
   // Returns the number of other variables opposite to x 
   int opposCount (int x);
   
   void printEquivalences ();
   
   void printEquivalence (int x);
   
   void printOpposite (int x);
   
   void printNoLinksToRoot ();
   
   void printEquivClassCounts ();
   
   void printEquivVarCount ();
   
   void printOpposVarCount ();
   
   void printWhetherEquiv ();
};
#endif
