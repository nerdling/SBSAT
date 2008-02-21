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
/**                              equivclass.h
 *                                                                            
 * Maintain sets of equivalent and opposite valued variables using inverted
 * trees and path compression when joining.  Equivalences can only be set,
 * not removed.  Assumes variables are numbered (that is, of type int).
 * 
 * class Equiv API:
 *    Equiv (int inp, int out, int True, int False): Constructor, inp is the
 *    maximum numbered input variable, True is the number of the variable
 *    representing value true, False is the number of the variable representing
 *    value false.
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

#include "sbsat.h"
#include "equivclass.h"

#define null -1
#define VecType unsigned long

/*
typedef struct xorrecord {
	VecType *vector; // 0-1 vector showing vars in xor func and which type of xor func it is
	int vector_size; // Number of bytes in vector
	int *varlist;    // List of vars that are 1 in vector
	int type;        // Number of vars in the function
	struct xorrecord *next;
} XORd;

typedef struct equiv_rec {
	int no_inp_vars; // The number of input variables
	int index;       // Number of vectors we have
	int vec_size;    // Number of bytes comprising each VecType vector
	int equiv_idx;   // how many such nodes are in use.
	int Tr,Fa;       // Symbols for true and false
} EquivVars;

typedef struct {
	int left, rght;
} Result;
*/

// Figure 1.
// 0-1 matrix representing a system of linear functions.  Vectors are packed
// into 32 bit words.  The example below shows a system of 46 variables.  It
// uses two words per function.  Columns are indexed on variables, lowest on
// the left, and rows on functions.  The first row in the example below
// represents the clause - operator "+" is the xor operator
//            (x[2] + x[4] + x[12] + x[19] + x[30] + x[45])
// because the "last column" (rightmost bit of rightmost word - most
// significant bit) is 0.  The second row in the example below represents
//            (x[3] + x[4] + x[5] + x[10] + x[21] + x[44] + 1)
// where the extra "1" is due to a "1" in the last column of the right word.
// The stored form of the matrix has been subjected to gaussian elimination
// and consists of a diagonal component plus an arbitrary component.
//
//            +--first_bit[6] = null         +--first_bit[36] = 9
//            |                              |
//      00101000000010000001000000000010 00000000000001000000000000000000
//      00011100001000000000010000000000 00000000000010000000000000000001
//      00000000000000000111000000000000 00000000000100000000000000000000
//      00101000000010000001000000000010 10000000001000000000000000000000
//      00001000000010000000000000000010 01000000010000000000000000000001
//      00100000000000000001000000000000 00000000100000000000000000000001
//      00001000000000000000000000000000 00000001000000000000000000000001
//      00000000010100000001000000100010 00000010000000000000000000000000
//      00000000010000000001000000000000 10000100000000000000000000000001
//      00000100000001000000001000000000 00001000000000000000000000000000
//      11000000010000000000000000000000 00010000000000000000000000000000
//      00000000010000000000001000000000 00100000000000000000000000000001
//      |<-------anything allowed-------->||<-------->||<---unused---->||
//                                           diagonal                   |
//                                           component     last column--+
//
// The following additional variables are used:
//   first_bit - If x is a column (variable) in the diagonal component then
//           first_bit[x] is the number of the row which has a "1" in that
//           column.  Otherwise first_bit[x] is null.
//   mask  - 0-1 masking pattern for columns of the matrix.  If mask[x] is 1
//           then variable x has not been assigned a value and is not a column
//           in the diagonal component.
//   order - list of pointers to vectors used for sorting by means of qsort
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
//           Matrix variables:
//               first_bit: (short int *)&frame[first_bit_ref]
//                   xlist: (short int *)&frame[xlist_ref]
//                   ylist: (short int *)&frame[ylist_ref]
//                  matrix: (char *)&frame[vecs_v_ref]
//                 ith row: (VecType *)&frame[vecs_v_start+i*vecs_rec_bytes]
//      ith row column ptr: *((short int *)&frame[offset+vecs_colm_start])
//              where offset = vecs_v_start + i*vecs_rec_bytes
//        ith row var list: (short int *)&frame[offset+vecs_vlst_start]
//        ith row vec_size: (short int *)&frame[offset+vecs_vsze_start]
//        ith row no. vars: (short int *)&frame[offset+vecs_nvar_start]
//       ith row func type: (short int *)&frame[offset+vecs_type_start]
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
// Matrix Operations:
//   1. addRow (XORd *xord)
//      xord is vector information including 0-1 row, list of variables which
//      have corresponding 1 columns, number of bytes the 0-1 part of the
//      vector occupies, the first word of the stored vector containing the
//      1 which is in the highest order bit (except for the last column),
//      the bit position in that word where the highest order 1 occurs, the
//      number of 1s in the vector (except the last column), the type of the
//      vector (even or odd parity), the highest order column with a 1.
//
//      The row "xord" is added to the matrix as follows:
//       a. direct memcpy of data occurs from xord to the correct frame row
//       b. existing rows of the matrix containing 1s on columns in the
//          diagonal which align with 1s of the "xord" row are added to it
//          so that all diagonal columns of "xord" are 0
//       c. the highest order 1 existing in "xord" is found
//          * if no 1s found and last column is 1 then inconsistency reported
//          * if no 1s found and last column 0 then matrix reverts
//       d. xord is added to all rows with a 1 in same column as xord's
//          highest order 1
//       e. first_bit[xord's highest order 1] is set to new row
//       f. mask takes a 0 in same position as xord's highest order 1
//       g. column ptr of the new row set to xord's highest order 1
//
//   2. makeAssign (int var, int val)
//      Adjust matrix data structures when variable "var" is set to value
//      "val".  Possible values for "val" are 0 (false) and 1 (true).
//
//      Structures are adjusted as follows:
//       a. if "val" is 0 or 1,
//          * if "var" is already set to opposite value return "inconsistent"
//          * if "var" is already set to same value simply return
//          * otherwise, update the equivalence classes accordingly
//       b. check mask bit for "var"
//          * if it is 0 and first_bit[var] is null, we have 0 column due to
//            prior assignment of the variable - hence return doing nothing
//       c. check whether "var" is a column of the diagonal component
//          * if so, 0 the column - while doing so, if "val" is 1 reverse the
//            last column values for all rows in which the column has value 1
//          * find first 1 of the row
//          * if no 1s found and last column has value 1 return "inconsistent"
//          * if no 1s found and last column 0 remove row from the matrix
//          * otherwise open new diagonal column by setting first_bit etc.
//          * set mask bit to 0
//          * cancel all 1s in new diagonal column by adding new row to exist
//            ing rows
//       d. otherwise
//          * set mask bit to 0
//          * 0 the column - while doing so, if "val" is 1 reverse the last
//            column values for all rows in which the column has value 1
//
//   3. findAndSaveEquivalences ()
//      walk through matrix, find equivalences and opposites, and add to DB
//
//      Equivalences are found as follows:
//       a. set pointers to all vectors from array "order"
//       b. sort array "order" by masked value of vector (non-diagonal
//          non-set variables)
//       c. for each block of rows having same value do the following
//          * attempt to add equivalence of the two diagonal variables to DB
//            if two "last columns" sum to 0 or add opposites if "last
//            columns" sum to 1.
//          * if attempted insertion returns with -1, report "inconsistent"
//          * if attempted insertion reports these are already there, skip
//          * otherwise, the insertion is completed
//

// Figure 2. 
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

int vsize; // Number of bytes comprising each VecType vector
VecType *msk; // 0 bits are columns of the diagonalized submatrix or assigned variables

// Compare function for the xor qsort
int cmp (const void *x, const void *y) {
	VecType **a = (VecType **)x;
	VecType **b = (VecType **)y;
	
	for(int j = vsize-1; j >= 0; j--) {
		if(((*a)[j] & msk[j]) < ((*b)[j] & msk[j])) return -1;
		if(((*a)[j] & msk[j]) > ((*b)[j] & msk[j])) return 1;
	}
	return 0;
}

/*
class Equiv {
   Vars *rec;
   char *frame;
   Result *result;
	
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
   */

   Equiv::Equiv(int inp, int out, int True, int False) {
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

      no_funcs = out+inp; //The inp is room for equivalences.
		                    //SEAN!!! This is a temp fix and
		                    //should be fixed in the future
		//if(no_funcs > 500) no_funcs = 500;
		//This is the MAX number of rows allowed in the xor table
		
		//EquivVars
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

		//LinearVars
		//Number of bytes in various fields, arrays, lists in a vector record
		int vecs_v_bytes = (1+inp/(sizeof(VecType)*8))*sizeof(VecType); //Actual vector
		int vecs_vlst_bytes = 0; //Variable list
		int vecs_vsze_bytes = sizeof(int); //Size of variable list
		int vecs_nvar_bytes = sizeof(int); //Number of rows
		int vecs_type_bytes = sizeof(int); //Vector type
		int vecs_colm_bytes = sizeof(int); //Column pointer
		
		//Starting point for various fields etc. in a vector record
		vecs_vlst_start = vecs_v_bytes;
		vecs_vsze_start = vecs_vlst_start + vecs_vlst_bytes;
		vecs_nvar_start = vecs_vsze_start + vecs_vsze_bytes;
		vecs_type_start = vecs_nvar_start + vecs_nvar_bytes;
		vecs_colm_start = vecs_type_start + vecs_type_bytes;
		vecs_rec_bytes  = vecs_colm_start + vecs_colm_bytes;

		int first_bit_sze;
		int mask_sze;
		if(ge_preproc == '1') {
			first_bit_sze = sizeof(int)*inp;
			mask_sze      = sizeof(VecType)*(1+inp/(sizeof(VecType)*8));
		} else {
			first_bit_sze = 0;
			mask_sze      = 0;
		}
		
		//LinearVars
		int vars_sze      = sizeof(EquivVars);
		int vars_ref      = 0;
		int first_bit_ref = vars_ref + vars_sze;
		int mask_ref      = first_bit_ref + first_bit_sze;
		//EquivVars		
      int equiv_fwd_ref = mask_ref + mask_sze;
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
		vecs_v_start      = vecs_v_ref;
		
		if(ge_preproc == '1') {
			frame_size = vecs_v_ref + vecs_rec_bytes*no_funcs;
			order = (VecType **)ite_calloc(1, sizeof(VecType *)*no_funcs, 9, "gaussian elimination order");
			result = (Result *)ite_calloc(1, sizeof(Result)*inp, 9, "gaussian elimination results");
		} else {
			frame_size = ylist_ref + ylist_sze;		
		}
		
      frame = (char *)ite_calloc(1, frame_size, 9, "EquivClass memory frame");
      frame_start = (unsigned long)frame;

      rec = (EquivVars *)frame;
      no_inp_vars = rec->no_inp_vars = inp; // Number of input variables  
      index = rec->index = 0;
      equiv_idx = rec->equiv_idx = 3; // Can't use 0 since use minus to 
                                      // point to equiv node, can't use 1 
                                      // because -1 means not pointing to 
                                      // anything, can't use -2 since index 
                                      // 2 is taken for T and F
      Tr = rec->Tr = True;
      Fa = rec->Fa = False;
		
		if(ge_preproc == '1') {
			vec_size = rec->vec_size = 1+no_inp_vars/(sizeof(VecType)*8);
			
			mask = (VecType *)(frame_start+mask_ref);
			for(int i=0; i < vec_size; i++) mask[i] = (VecType)(-1);
			mask[vec_size-1] -= (1 << sizeof(VecType)*8-1);
			
			first_bit = (int *)(frame_start + first_bit_ref);
			for(int i=0; i < inp; i++) first_bit[i] = null;
			
			for(VecType i=frame_start - frame_size; i< (VecType)vecs_rec_bytes*no_funcs; i++)
			  frame[i] = (char)(-1);
		}		
		
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

   Equiv::~Equiv () { 
		ite_free((void **) &frame);
		if(ge_preproc == '1') {
			ite_free((void **) &order);
			ite_free((void **) &result);
		}
	}

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
	int Equiv::get_equiv(int x) {
		int a;
		for (a=x ; equiv_fwd[a] >= 0 ; a=equiv_fwd[a]){}
		//d3_printf2("[%d]", equiv_lft[-equiv_fwd[a]]);
		return equiv_min[a];
	}
   
	Result *Equiv::insertEquiv (int x, int y) {
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
            
            tmp_result.left = lft;
				tmp_result.rght = rgt;
				
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
				tmp_result.left = Tr;
				tmp_result.rght = Fa;
			}
      } else return NULL; // Already in same class - no change 
      return &tmp_result;
   }
   
   // Make x and y have opposite value.  For comments, see notes above 
   // code for "insertEquiv" above.                                    
   Result *Equiv::insertOppos (int x, int y) {
      int xidx=0, yidx=0, a, b, lft, rgt, tmp, i;
      
      for (a=x ; equiv_fwd[a] >= 0 ; a=equiv_fwd[a]) xlist[xidx++] = a;
      for (b=y ; equiv_fwd[b] >= 0 ; b=equiv_fwd[b]) ylist[yidx++] = b;
      if (a == b) {  // Inconsistent 
			tmp_result.left = Tr;
			tmp_result.rght = Fa;
			return &tmp_result;
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
      tmp_result.left = lft;
      tmp_result.rght = rgt;		
		
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
      return &tmp_result;
   }
   
   // Returns 0 if no equivalence or error, 1 if equivalent, -1 if opposite
   int Equiv::isEquiv (int x, int y) {
      int a, b;
      
      if (x < 0 || y < 0) return 0;
      for (a=x ; equiv_fwd[a] >= 0 ; a=equiv_fwd[a]);
      for (b=y ; equiv_fwd[b] >= 0 ; b=equiv_fwd[b]);
      if (a == b) return 1;
      if (equiv_fwd[a] == equiv_fwd[b] && equiv_fwd[a] < -1) return null;
      return 0;
   }

   // Returns a -1 terminated list of vars equivalent to x or null on error
   int *Equiv::equivalent (int x) {
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
   int *Equiv::opposite (int x) {
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
   int *Equiv::valueOfTrue ()  { return equivalent(Tr); }
   int *Equiv::valueOfFalse () { return equivalent(Fa); }
   
   // Returns the number of other variables equivalent to x 
   int Equiv::equivCount (int x) {
      int a;
      for (a=x ; equiv_fwd[a] >= 0 ; a=equiv_fwd[a]);
      return equiv_cnt[a] - 1;
   }
   
   // Returns the number of other variables opposite to x 
   int Equiv::opposCount (int x) {
      int a;
      for (a=x ; equiv_fwd[a] >= 0 ; a=equiv_fwd[a]);
      int s = -equiv_fwd[a];
      int other = (equiv_lft[s] == a) ? equiv_rgt[s] : equiv_lft[s];
		int count = 0;
      for(a=other; a!=null; a=equiv_bck[a]) count++;
		return count;
		//return equiv_cnt[other] - 1;
   }

   void Equiv::printEquivalences () {
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
   
   void Equiv::printEquivalence (int x) {
      int a;
      for (a=x ; equiv_fwd[a] >= 0 ; a=equiv_fwd[a]);
      for ( ; a != null ; a=equiv_bck[a]) cout << a << " ";
      cout << "\n";
   }
   
   void Equiv::printOpposite (int x) {
      int a;
      for (a=x ; equiv_fwd[a] >= 0 ; a=equiv_fwd[a]);
      int super = -equiv_fwd[a];
      int root = (equiv_lft[super] == a) ? equiv_rgt[super] : equiv_lft[super];
      for (a=root ; a != null ; a=equiv_bck[a]) cout << a << " ";
      cout << "\n";
   }
   
   void Equiv::printNoLinksToRoot () {
      cout << "Links: ";
      for (int i=0 ; i < no_inp_vars ; i++) {
			int j=0;
			for (int a=i ; equiv_fwd[a] >= 0 ; j++, a=equiv_fwd[a]);
			cout << j << " ";
      }
      cout << "\n";
   }
   
   void Equiv::printEquivClassCounts () {
      cout << "EqCnt (Var:cnt): ";
      for (int i=0 ; i < no_inp_vars ; i++) {
			if (equiv_fwd[i] < 0) {
				cout << i << ":" << equiv_cnt[i] << " ";
			}
      }
      cout << "\n";		
   }
   
   void Equiv::printEquivVarCount () {
      cout << "EqVarCnt (Var:cnt): ";
      for (int i=0 ; i < no_inp_vars ; i++) {
	 cout << i << ":" << equivCount(i) << " ";
      }
      cout << "\n";
   }
   
   void Equiv::printOpposVarCount () {
      cout << "OpVarCount (Var:cnt): ";
      for (int i=0 ; i < no_inp_vars ; i++) {
			cout << i << ":" << opposCount(i) << " ";
      }
      cout << "\n";
   }
   
   void Equiv::printWhetherEquiv () {
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

   //var was the diagonal element of vector first_bit[var].
   //var has been removed from the vector.
   //This function will find a new diagonal element for vector first_bit[var]
   //and update all related data structures accordingly.
   int Equiv::rediagonalize(int var) {	
		unsigned long vec_add;
	   int v = first_bit[var];
		assert(v!=null);
		VecType *vec = (VecType *)(frame_start + vecs_v_start+v*vecs_rec_bytes);
		int word = var/(sizeof(VecType)*8);
		int bit = var%(sizeof(VecType)*8);
		
		// Find first 1 for new diagonal element
		int save_first_column = -1;
		int k;
		for(k = vec_size-1; k >= 0; k--) { // Maybe 10 of these loops
			VecType tmp;
			if((tmp = (mask[k] & vec[k])) != 0) {
				int hgh = sizeof(VecType)*8-1;
				save_first_column = 0;
				while (hgh > 0) { // Maybe 5 of these loops - binary search for leading 1
					int mid = hgh/2;
					if (tmp >= (unsigned long)(1 << mid+1)) {
						tmp >>=mid+1;
						save_first_column += mid+1;
					}
					hgh/=2;
				}
				save_first_column += k*(sizeof(VecType)*8);
				break;
			}
		}
		// If k == -1 then no 1's were found in modified vector called vec
		if(k == -1) {
			if(vec[vec_size-1]) return TRIV_UNSAT; // Inconsistency discovered
			else {
				vec_add = vecs_v_start+v*vecs_rec_bytes+vecs_colm_start;
				*((int *)(frame_start+vec_add)) = null;
				first_bit[var] = null; // Remove row from the matrix
				// mask[word] |= (1 << bit); // Leave this bit 0 so mask bits apply only to unassigned variables
				// But currently the row is still really in the matrix
				//fflush(stdout);		printLinear ();		fflush(stdout);
				return PREP_NO_CHANGE; // Return this fact
				// This row has been emptied
				// This is a fine thing to have happen
			}
		}
		// Open up a new diagonal column
		vec_add = vecs_v_start+v*vecs_rec_bytes+vecs_colm_start;
		*((int *)(frame_start+vec_add)) = save_first_column;
		first_bit[save_first_column] = v; //first_bit[var]
		first_bit[var] = null;
		
		// mask[word] |= (1 << bit); // Leave this bit 0 so mask bits apply only to unassigned variables
			
		word = save_first_column/(sizeof(VecType)*8);
		bit = save_first_column%(sizeof(VecType)*8);
		mask[word] &= (-1 ^ (1 << bit));
		
		// Cancel all 1's in the new column. Currently looks at *all* vectors!
		int vec_address = vecs_v_start;
		for(int i=0; i < v; i++) {
			VecType *vn = (VecType *)(frame_start+vec_address);
			if (vn[word] & (1 << bit)) {
				for(int j=0; j<vec_size; j++) vn[j] ^= vec[j];
			}
			vec_address += vecs_rec_bytes;
		}
		vec_address += vecs_rec_bytes; //Skip the vector holding the diagonal element
		for(int i=v+1; i< rec->index; i++) {
			VecType *vn = (VecType *)(frame_start+vec_address);
			if(vn[word] & (1 << bit)) {
				for(int j=0; j<vec_size; j++) vn[j] ^= vec[j];
			}
			vec_address += vecs_rec_bytes;
		}
		return PREP_CHANGED;
	}

   // When a variable is given an assignment, 0 out that column of all rows.
	// Check for inconsistency - if 0 row = 1 last column
	// One hitch - if column is in the diagonal submatrix, then look for 1st
	// column of that row intersecting the diagonal which is a 1. Rediagonal-
	// ize as though a new row has just been added.
   int Equiv::makeAssign (int var, int value) {
		int ret = PREP_NO_CHANGE;
		//fflush(stdout);		printLinear ();		fflush(stdout);
		int v;
		
		//Grab word and bit positions of mask and vector
		int word = var/(sizeof(VecType)*8);
		int bit = var%(sizeof(VecType)*8);
		// If we hit a 0 column, formerly on the diagonal, error?
		// I really don't think this should happen.
		if(!(mask[word] & (1 << bit)) && first_bit[var] == null) return ret;
		
		// Check whether column is in diagonal submatrix
		if((v=first_bit[var]) != null) {
			// If so,
			// Zero out the diagonal and if the value of the variable of the
			// column is 1, reverse the value of the last column
			VecType *vec = (VecType *)(frame_start + vecs_v_start+v*vecs_rec_bytes);
			if(value) {
				if(vec[vec_size-1]&(1<<sizeof(VecType)*8-1)) //Last bit is 1,
				  vec[vec_size-1] &= (-1 ^ (1 << sizeof(VecType)*8-1)); //Make it 0
				else //Last bit is 0,
				  vec[vec_size-1]|=(1 << sizeof(VecType)*8-1); //Make it 1
			}
			vec[word] ^= (1 << bit); //Remove var from the vector

			switch(ret = rediagonalize(var)) {
			 case TRIV_UNSAT:
			 case TRIV_SAT:
			 case PREP_ERROR: return ret;
			 default:break;
			}
		} else {
			// If the column is not in the diagonal submatrix
			// Zero out the column and set the mask bit to 0
			int vec_address = vecs_v_start;
			for(int i=0; i < rec->index; i++) {
				VecType *vn = (VecType *)(frame_start+vec_address);
				// If the row has a 1 in the zeroed column and the value of the
				// variable in the column is 1 then reverse the value of the last
				// column
				if(vn[word] & (1 << bit)) {
					if(value) {
						if(vn[vec_size-1] & (1 << sizeof(VecType)*8-1))
						  vn[vec_size-1] &= (-1 ^ (1 << sizeof(VecType)*8-1));
						else
						  vn[vec_size-1] |= (1 << sizeof(VecType)*8-1);
					}
					vn[word] &= (-1 ^ (1 << bit));
				}
				vec_address += vecs_rec_bytes;
			}
			mask[word] &= (-1 ^ (1 << bit));				  
			ret = PREP_CHANGED;
		}		
		//fflush(stdout);		printLinear ();		fflush(stdout);
		return ret; // Normal ending
	}

   int Equiv::applyEquiv (int var1, int var2) {
		int ret = PREP_NO_CHANGE;
		//fflush(stdout);		printLinear ();		fflush(stdout);
		int v1,v2;
		assert(var1>0 && var1<abs(var2));
		int value = var2>0;
		var2 = abs(var2);
		
		//Grab word and bit positions of mask and vector
		int word1 = var1/(sizeof(VecType)*8);
		int bit1 = var1%(sizeof(VecType)*8);
		int word2 = var2/(sizeof(VecType)*8);
		int bit2 = var2%(sizeof(VecType)*8);
		
		// Check whether both columns are in diagonal submatrix
		if((v1=first_bit[var1]) != null && (v2=first_bit[var2]) != null) {
			// If so,
			// replace var2 by var1 in var2's vector (handle 'value' correctly)
			// XOR var1's vector and var2's vector
			// choose new diagonal element for var2's vector
			VecType *vec1 = (VecType *)(frame_start + vecs_v_start+v1*vecs_rec_bytes);
			VecType *vec2 = (VecType *)(frame_start + vecs_v_start+v2*vecs_rec_bytes);

			if(!value) { //flip when opposite, e.g. 1+0=1, 0+1=1
				if(vec2[vec_size-1]&(1<<sizeof(VecType)*8-1)) //Last bit is 1,
				  vec2[vec_size-1] &= (-1 ^ (1 << sizeof(VecType)*8-1)); //Make it 0
				else //Last bit is 0,
				  vec2[vec_size-1]|=(1 << sizeof(VecType)*8-1); //Make it 1
			}

			vec2[word2]^=(1 << bit2); //Remove var2 from the vector
			vec2[word1]^=(1 << bit1); //Add var1 into the vector
			for(int j=0; j<vec_size; j++) vec2[j] ^= vec1[j]; //XOR the vectors together

			switch(ret = rediagonalize(var2)) {
			 case TRIV_UNSAT:
			 case TRIV_SAT:
			 case PREP_ERROR: return ret;
			 default:break;
			}
		} else if((v2=first_bit[var2]) != null) {
			// If only var2 is a diagonal element,
			VecType *vec2 = (VecType *)(frame_start + vecs_v_start+v2*vecs_rec_bytes);
			// remove var2 (and var1 if it occurs) from var2's vector (handle 'value' correctly)
			// choose new diagonal element for var2's vector
			if(!value) { //flip when opposite, e.g. 1+0=1, 0+1=1
				if(vec2[vec_size-1]&(1<<sizeof(VecType)*8-1)) //Last bit is 1,
				  vec2[vec_size-1] &= (-1 ^ (1 << sizeof(VecType)*8-1)); //Make it 0
				else //Last bit is 0,
				  vec2[vec_size-1]|=(1 << sizeof(VecType)*8-1); //Make it 1
			}
			
			vec2[word2]^=(1 << bit2); //Remove var2 from the vector
			// Check if var1 is also in the vector
			if(vec2[word1] & (1 << bit1))
			  vec2[word1]^=(1 << bit1); //Remove var1 from the vector
			
			switch(ret = rediagonalize(var2)) {
			 case TRIV_UNSAT:
			 case TRIV_SAT:
			 case PREP_ERROR: return ret;
			 default:break;
			}
		} else if((v1=first_bit[var1]) != null){
			// If only var1 is a diagonal element,
			VecType *vec1 = (VecType *)(frame_start + vecs_v_start+v1*vecs_rec_bytes);
			// If var2 is also in the vector,
			if(vec1[word2] & (1 << bit2)) {
				// remove var1 and var2 from var1's vector (handle 'value' correctly)
				// choose new diagonal element for var1's vector
				if(!value) { //flip when opposite, e.g. 1+0=1, 0+1=1
					if(vec1[vec_size-1]&(1<<sizeof(VecType)*8-1)) //Last bit is 1,
					  vec1[vec_size-1] &= (-1 ^ (1 << sizeof(VecType)*8-1)); //Make it 0
					else //Last bit is 0,
					  vec1[vec_size-1]|=(1 << sizeof(VecType)*8-1); //Make it 1
				}
				
				vec1[word1]^=(1 << bit1); //Remove var1 from the vector
				vec1[word2]^=(1 << bit2); //Remove var2 from the vector
				
				switch(ret = rediagonalize(var1)) {
				 case TRIV_UNSAT:
				 case TRIV_SAT:
				 case PREP_ERROR: return ret;
				 default:break;
				}
				mask[word1] |= (1 << bit1); // Set the mask bit to 1
				                            // var1 is no longer in the diagonal
				
				// Go through the whole table, replacing v2's with v1's
				// correctly handling 'value'
				int vec_address = vecs_v_start;
				for(int i=0; i < rec->index; i++) {
					VecType *vn = (VecType *)(frame_start+vec_address);
					if(vn[word2] & (1 << bit2)) {
						if(!value) { //flip when opposite, e.g. 1+0=1, 0+1=1
							if(vn[vec_size-1]&(1<<sizeof(VecType)*8-1)) //Last bit is 1,
							  vn[vec_size-1] &= (-1 ^ (1 << sizeof(VecType)*8-1)); //Make it 0
							else //Last bit is 0,
							  vn[vec_size-1]|=(1 << sizeof(VecType)*8-1); //Make it 1
						}
						vn[word2]^=(1 << bit2); //Remove var2 from the vector
						vn[word1]^=(1 << bit1); //Add var1 into the vector
						
					}
					vec_address += vecs_rec_bytes;
				}
				mask[word2] &= (-1 ^ (1 << bit2));
				ret = PREP_CHANGED;
			}
		} else {
			// Neither var1 or var2 are an element in the diagonal

			// Go through the whole table, replacing v2's with v1's
			// correctly handling 'value'
			int vec_address = vecs_v_start;
			for(int i=0; i < rec->index; i++) {
				VecType *vn = (VecType *)(frame_start+vec_address);
				if(vn[word2] & (1 << bit2)) {
					if(!value) { //flip when opposite, e.g. 1+0=1, 0+1=1
						if(vn[vec_size-1]&(1<<sizeof(VecType)*8-1)) //Last bit is 1,
						  vn[vec_size-1] &= (-1 ^ (1 << sizeof(VecType)*8-1)); //Make it 0
						else //Last bit is 0,
						  vn[vec_size-1]|=(1 << sizeof(VecType)*8-1); //Make it 1
					}
					vn[word2]^=(1 << bit2); //Remove var2 from the vector
					vn[word1]^=(1 << bit1); //Add/Remove var1 into/from the vector (doesn't matter)
				}
				vec_address += vecs_rec_bytes;
			}
			mask[word2] &= (-1 ^ (1 << bit2));
			ret = PREP_CHANGED;
		}
		//fflush(stdout);		printLinear ();		fflush(stdout);
		return ret; // Normal ending
	}

	char *Equiv::pw (VecType word) {
		char *out = (char *)calloc(1,sizeof(char)*sizeof(VecType)*8);
		for (unsigned int i=0 ; i < sizeof(VecType)*8 ; i++) {
			if (word & (1 << i)) out[i] = '1'; else out[i] = '0';
		}
		out[sizeof(VecType)*8] = 0;
		return out;
	}
		
   // Go through matrix to find and record equivalences.  Additional data
   // structures are needed to prevent attempting to add the same equiv.
   // to the data base more than once.
   // Returns: array of following form:
   //    5 6 -4 = inconsistency found variables 5 and 6 can't be same
   //    5 6 -5 = inconsistency found variables 5 and 6 can't be opposite
   //    3 4 -2 2 6 -3 1 2 -2 -1 = vars 3,4 equiv, 1,2 equiv, 2,6 oppos
   //    (-2 separator means equivalent, -3 opposite, -1 terminates list)
   //    NULL = no vectors are in the matrix
   // Operation:
   //    Set pointers to all rows (by means of array "order")
   //    Sort the "order" array where "mask" is applied to each row so that
   //       only non-assigned, non-diagonal columns are used in weights
   //    From lowest to highest, scan for blocks of same values
   //       In each block check for equivalence or opposite wrt first block
   //       variable found
   //    Add equivalences and opposites to internal data base and "result"
   //    Return "result"
   Result *Equiv::findAndSaveEquivalences () {
		//fflush(stdout);		printLinear ();		fflush(stdout);
		int idx = 0; // index into return array
		unsigned long vec_add, vec_f_add;
		if (rec->index < 1) return NULL; // Return NULL if no vectors in matrix
		// Sort all rows by masked value (non-diagonal submatrix columns)
		// Ultimately we should prevent having to do this over and over again
		int vec_address = vecs_v_start;
		for (int i=0 ; i < rec->index ; i++) {
			order[i] = (VecType *)(frame_start+vec_address);
			vec_address += vecs_rec_bytes;
		}
		
		msk = mask;
		vsize = vec_size;
		qsort((void *)order, (size_t)rec->index, sizeof(VecType *), cmp);

		//Add equivalences when rows have same value
		int p;
		//Start looking at all 0 rows first to avoid the check later
		for(p=0; p < rec->index; p++) {
			VecType *vec = order[p];
			for(int j=0; j < vec_size; j++) if(mask[j] & vec[j]) goto dd;
			// If it is an all 0 row and if there is a diagonal 1 bit, then
			// set the value of the diagonal variable to the value given by
			// the "last bit".
			vec_add = (unsigned long)vec+vecs_colm_start;
			int v = *((int *)vec_add);
			if(v == null) { // No diagonal bit and all 0 row
				if (vec[vec_size-1] & (1 << (sizeof(VecType)*8-1))) { // inconsistend since last column is 1
					result[0].left = Tr; result[0].rght = Fa;
					return result;
				} else { // Not inconsistent but useless, so skip
					continue;
				}
			} else if(vec[vec_size-1] & (1 << (sizeof(VecType)*8-1))) {
				//fflush(stdout);fprintf(stdout, "|%d=T|", v);fflush(stdout);
				Result *fase_result = insertEquiv(v, Tr);
				if(fase_result == NULL) continue; //No change - already in the database
				result[idx].left = fase_result->left; // Attempt to make v = Tr
				result[idx].rght = fase_result->rght; // Attempt to make v = Tr
				//fprintf(stdout, "|%d, %d|", result[idx].left, result[idx].rght);
				if(fase_result->left == Tr && fase_result->rght == Fa) return result; // Inconsistency
				idx++; // Equivalence is valid
			} else {
				//fflush(stdout);fprintf(stdout, "|%d=F|", v);fflush(stdout);
				Result *fase_result = insertEquiv(v, Fa);
				if(fase_result == NULL) continue; // No change - already in database
				result[idx].left = fase_result->left; // Attempt to make v = Tr
				result[idx].rght = fase_result->rght; // Attempt to make v = Tr
				//fprintf(stdout, "|%d, %d|", result[idx].left, result[idx].rght);
				if(fase_result->left == Tr && fase_result->rght == Fa) return result; // Inconsistency
				idx++; // Equivalence is valid
			}
		}
		// Check the remaining rows for equivalence
dd:;

		while (p < index) {
			VecType *vec = order[p];
			vec_add = (unsigned long)vec+vecs_colm_start;
			int vpc = *((short int *)vec_add);
			
			// Find block of vectors from p to h with same masked value
			// and either make those variables equivalent or opposite
			// depending on the value of the "last column".
			int h;
			for (h=p+1 ; h < index ; h++) {
				VecType *vef = order[h];
				for (int j=0 ; j < vec_size ; j++)
				  if ((mask[j] & vec[j]) != (mask[j] & vef[j])) goto d1;
				vec_f_add = (unsigned long)vef+vecs_colm_start;
				int vph = *((short int *)vec_f_add);
				if ((vec[vec_size-1] & (1 << (sizeof(VecType)*8-1))) ==
					 (vef[vec_size-1] & (1 << (sizeof(VecType)*8-1)))) {
					// The value of the row is 0
					//cout << "Attempt " << vart[0] << " equiv " << vart[1] << "\n";
					Result *fase_result;
					//fflush(stdout);fprintf(stdout, "|%d=%d|", vph, vpc);fflush(stdout);
					if(vph > vpc) fase_result = insertEquiv(vph, vpc);
					else fase_result = insertEquiv(vpc, vph);
					if(fase_result == NULL) continue; // No change - already in database
					result[idx].left = fase_result->left;
					result[idx].rght = fase_result->rght;
					//fprintf(stdout, "|%d, %d|", result[idx].left, result[idx].rght);
					if(fase_result->left == Tr && fase_result->rght == Fa) return result; // Inconsistency
					idx++; // Equivalence is valid
				} else {
					// The value of the row is 1 -
					//cout << "Attempt " << vph << " oppos " << vpc << "\n";
					Result *fase_result;
					//fflush(stdout);fprintf(stdout, "|%d=-%d|", vph, vpc);fflush(stdout);
					if(vph > vpc) fase_result = insertOppos(vph, vpc);
					else fase_result = insertOppos(vpc, vph);
					if(fase_result == NULL) continue; // No change - already in database
					result[idx].left = fase_result->left;
					result[idx].rght = -fase_result->rght;
					//fprintf(stdout, "|%d, %d|", result[idx].left, result[idx].rght);
					if(fase_result->left == Tr && fase_result->rght == Fa) {
						result[idx].rght = -result[idx].rght;
						return result; // Inconsistency
					}
					idx++; // Equivalence is valid
				}
			}
d1:;
			p = h;
		}
		
		// Look for all rows with 2 1s, one is on the diagonal
		for (p=0 ; p < rec->index ; p++) {
			VecType *vec = order[p];
			vec_add = (unsigned long)vec+vecs_colm_start;
			int v = *((short int *)vec_add); // Diagonal, if it exists
			int cnt = 0;
			int vart[2];

			//char *vec_char = pw(*vec);
			//for (int j=0 ; j < no_inp_vars ; j++) {
			//	fprintf(stderr, "%c", vec_char[j]);
			//}
			//fprintf(stderr, "\n");
			//free(vec_char);
			
			for (int j=0 ; j < vec_size ; j++) { //Can speed this up using binary search.
				//This word better either be zero, or have only one 1.
				if(mask[j] & vec[j] == 0) continue;
				if(cnt>=1) {cnt++; break;}
				for(unsigned int bit = 0; bit < sizeof(VecType)*8; bit++)
				  if(mask[j] & vec[j] & (1 << bit)) {
					  if(cnt==1) {cnt++; break;}
					  vart[cnt++] = bit+j*sizeof(VecType)*8;
				  }
			}
			if (cnt == 1) {
				if (v != null) vart[cnt++] = v; else continue;
			} else continue;
			
			if (vec[vec_size-1] & (1 << (sizeof(VecType)*8-1))) {
				// The value of the row is 1 -
				//cout << "Attempt " << vart[0] << " oppos " << vart[1] << "\n";
				Result *fase_result;
				//fflush(stdout);fprintf(stdout, "|%d=-%d|", vart[0], vart[1]);fflush(stdout);
				if(vart[0] > vart[1]) fase_result = insertOppos(vart[0], vart[1]);
				else fase_result = insertOppos(vart[1], vart[0]);
				if(fase_result == NULL) continue; // No change - already in database
				result[idx].left = fase_result->left;
				result[idx].rght = -fase_result->rght;
				//fprintf(stdout, "|%d, %d|", result[idx].left, result[idx].rght);
				if(fase_result->left == Tr && fase_result->rght == Fa) {
					result[idx].rght = -result[idx].rght;
					return result; // Inconsistency
				}
				idx++; // Equivalence is valid
			} else {
				// The value of the row is 0
				//cout << "Attempt " << vart[0] << " equiv " << vart[1] << "\n";
				Result *fase_result;
				//fflush(stdout);fprintf(stdout, "|%d=%d|", vart[0], vart[1]);fflush(stdout);
				if(vart[0] > vart[1]) fase_result = insertEquiv(vart[0], vart[1]);
				else fase_result = insertEquiv(vart[1], vart[0]);
				if(fase_result == NULL) continue; // No change - already in database
				result[idx].left = fase_result->left;
				result[idx].rght = fase_result->rght;
				//fprintf(stdout, "|%d, %d|", result[idx].left, result[idx].rght);
				if(fase_result->left == Tr && fase_result->rght == Fa) return result; // Inconsistency
				idx++; // Equivalence is valid
			}
		}
		
		//vec_char = pw(*vef);
		//for (int j=0 ; j < no_inp_vars ; j++) {
		//	fprintf(stderr, "%c", vec_char[j]);
		//}
		//fprintf(stderr, "\n");
		//free(vec_char);
		
		result[idx].left = Fa; result[idx].rght = Fa; // End of result
		return result;
	}

   // Install copy of this frame into frame of another level
   char *Equiv::copyFrame(char *next_frame) {
		memcpy(next_frame, frame, vecs_v_start+rec->index*vecs_rec_bytes);
		return next_frame;		
	}

   // Add row to the matrix
   int Equiv:: addRow (XORd *xord) {
		// Return inconsistency (-1) if the row has no 1's but equals 1
		if(!xord->nvars && xord->type) return -1;
		
		// Return no change (0) if row is all 0
		if(!xord->nvars) return 0;
		
		if(rec->index >= no_funcs) {
			return 0; // Cannot add anymore vectors to the matrix
		}
		
		// Grab a new Vector and copy xord info to it
		unsigned long offset = frame_start + vecs_v_start + rec->index*vecs_rec_bytes;
		VecType *vec = (VecType*)offset;
		int *vsz = (int *)(offset+vecs_vsze_start);
		int *vnr = (int *)(offset+vecs_nvar_start);
		int *vty = (int *)(offset+vecs_type_start);

		*vsz = xord->vector_size;
		*vnr = xord->nvars;
		*vty = xord->type;
		memcpy(vec, xord->vector, sizeof(VecType)*vec_size);
		
		// The first 1 bit of the new vector is in a column which intersects
		// the diagonal.  Add the existing such row to the new vector.  For
		// all 1's of new vector in columns which intersect the diagonal, in
		// decreasing order, add rows to new vector.  While doing this,
		// locate the first 1 in a column intersecting the diagonal.  Open
		// this column to the diagonal.
		// 
		// Eliminate all 1's of the new vector in the current diagonal matrix
		for (int i=0 ; i < xord->nvars; i++) {
			short int v;
			if ((v = first_bit[(int)xord->varlist[i]]) != null) {
				VecType *vn = (VecType *)(frame_start+vecs_v_start+v*vecs_rec_bytes);
				for (int j=0 ; j < vec_size ; j++) vec[j] ^= vn[j];
			}
		}
		// Now that all the 1's in the diagonal submatrix are taken care of,
		// scan the vector to find the first 1 (MSB).  The variable (column)
		// which is found is stored in "save_first_column".
		int save_first_column = 0;
		int k;
		for (k=vec_size-1 ; k >= 0 ; k--) {
			// Maybe 10 of these loops
			VecType tmp;
			if ((tmp = (mask[k] & vec[k])) != 0) {
				int hgh = sizeof(VecType)*8-1;
				while (hgh > 0) { // Maybe 5 of these loops - binary search for leading 1
					int mid = hgh/2;
					if (tmp >= (unsigned int)(1 << mid+1)) {
						tmp >>= mid+1;
						save_first_column += mid+1;
					}
					hgh /= 2;
				}
				save_first_column += k*(sizeof(VecType)*8);
				break;
			}
		}
		// If k == -1 then no 1's were found in the new vector.
		if (k == -1) {
			if (vec[vec_size-1] & (1 << (sizeof(VecType)*8-1))) return -1; // Inconsistent
			else return 0; // No change
		}
		// Open up a new diagonal column
		unsigned long vec_add = vecs_v_start+rec->index*vecs_rec_bytes+vecs_colm_start;
		*((short int *)(frame_start+vec_add)) = save_first_column;
		first_bit[save_first_column] = rec->index;
		int word = save_first_column/(sizeof(VecType)*8);
		int bit = save_first_column % (sizeof(VecType)*8);
		mask[word] &= (-1 ^ (1 << bit));
		
		// Cancel all 1's in the new column.  Currently looks at *all* vectors!
		unsigned long vec_address = frame_start + vecs_v_start;
		for (int i=0 ; i < rec->index ; i++) {
			VecType *vn = (VecType *)vec_address;
			if (vn[word] & (1 << bit)) {
				for (int j=0 ; j < vec_size ; j++) vn[j] ^= vec[j];
			}
			vec_address += vecs_rec_bytes;
		}
		// Insert the new row
		index++;
      rec->index++;
		
		//SEAN!!! TO DO!!!
		//Mark the new column owner as a dependent variable.
		
		return 1;
	}
	
   void Equiv::printFrameSize () {
		cout << "Frame: " << frame_size << "\n";
	}

   void Equiv::printMask () {
		cout << "Mask (" << vec_size*sizeof(VecType)*8 << " bits):\n     ";
		for (int i=0 ; i < vec_size ; i++) {
			VecType tmp = mask[i];
			for (unsigned int j=0 ; j < sizeof(VecType)*8 ; j++) {
				if (tmp % 2) cout << "1"; else cout << "0";
				tmp /= 2;
			}
		}
		cout << "\n";
	}

   void Equiv::printLinearN () {
		printMask ();
		cout << "Vectors:\n";
		for (int i=0 ; i < rec->index ; i++) {
			cout << "     ";
			VecType *vn = (VecType *)(frame_start+vecs_v_start+i*vecs_rec_bytes);
			for (int j=0 ; j < vec_size ; j++) {
				VecType tmp = vn[j];
				for (unsigned int k=0 ; k < sizeof(VecType)*8 ; k++) {
					if (tmp % 2) cout << "1"; else cout << "0";
					tmp /= 2;
				}
			}
			cout << "     ";
			for (int j=0 ; j < vec_size ; j++) {
				VecType tmp = vn[j] & mask[j];
				for (unsigned int k=0 ; k < sizeof(VecType)*8 ; k++) {
					if (tmp % 2) cout << "1"; else cout << "0";
					tmp /= 2;
				}
			}
			cout << "\n";
		}
		cout << " +-----+     +-----+     +-----+     +-----+     +-----+\n";
	}
   
   void Equiv::printLinear () {
		int xlate[512];
		int rows[512];
		printLinearN ();
		for (int i=0 ; i < 512 ; i++) xlate[i] = i;
		int j=0;
		for (int i=no_inp_vars-1 ; i >= 0 ; i--)
		  if (first_bit[i] != null) {
			  rows[j] = first_bit[i]; xlate[j++] = i;
		  }
		
		rows[j] = -1;
		for (int i=no_inp_vars-1 ; i >= 0 ; i--)
		  if (first_bit[i] == null) xlate[j++] = i;

		cout << "Mask (" << vec_size*sizeof(VecType)*8 << " bits):\n     ";
		for (int i=0 ; i < no_inp_vars ; i++) {
			int word = xlate[i]/(sizeof(VecType)*8);
			int bit  = xlate[i] % (sizeof(VecType)*8);
			if (mask[word] & (1 << bit)) cout << "1"; else cout << "0";
		}
		cout << "\n";
		
		cout << "Vectors:\n";
		for (int i=0 ; rows[i] >= 0 ; i++) {
			cout << "     ";
			VecType *vn = (VecType *)(frame_start+vecs_v_start+rows[i]*vecs_rec_bytes);
			for (int j=0 ; j < no_inp_vars ; j++) {
				int word = xlate[j]/(sizeof(VecType)*8);
				int bit  = xlate[j] % (sizeof(VecType)*8);
				if (vn[word] & (1 << bit)) cout << "1"; else cout << "0";
			}
			cout << ".";
			if (vn[vec_size-1] & (1 << (sizeof(VecType)*8-1)))
			  cout << "1"; else cout << "0";
			cout << "     ";
			for (int j=0 ; j < no_inp_vars ; j++) {
				int word = xlate[j]/(sizeof(VecType)*8);
				int bit  = xlate[j] % (sizeof(VecType)*8);
				if (vn[word] & mask[word] & (1 << bit)) cout << "1"; else cout << "0";
			}
			cout << ".";
			if (vn[vec_size-1] & (1 << (sizeof(VecType)*8-1)))
			  cout << "1"; else cout << "0";
			cout << "\n";
		}
		cout << "========================================================\n";
	}
//};
