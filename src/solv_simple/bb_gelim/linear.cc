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

**/

#include "sbsat.h"

#define null -1
#define VecType unsigned long

typedef struct xorrecord {
	VecType *vector; // 0-1 vector showing vars in xor func and which type of xor func it is
	int vector_size; // Number of bytes in vector
	int *varlist;    // List of vars that are 1 in vector
	int nvars;       // Number of vars in the function
	int type;
	struct xorrecord *next;
} XORd;

typedef struct equiv_rec {
	int no_inp_vars; // The number of input variables                   
	int index;       // Number of vectors we have                       
	int vec_size;    // Number of bytes comprising each VecType vector
} EquivVars;


typedef struct {
	int left, rght;
} Result;


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

   
EquivVars *rec;
char *frame;
	
int no_inp_vars; // The number of input variables 
int no_funcs;    // The max number of functions (rows in matrix)
int vindex;      // Number of vectors we have 
int vec_size;    // Number of bytes comprising each VecType vector

int vecs_vlst_start;
int vecs_vsze_start;
int vecs_nvar_start;
int vecs_type_start;
int vecs_colm_start;
int vecs_rec_bytes;
	
VecType *mask;  // 0 bits are columns of the diagonalized submatrix or assigned variables
int *first_bit; // first_bit[i]: pointer to vector on diagonal with first 1 bit at column i
	
int frame_size;
int vecs_v_start;
unsigned long frame_start;

void initEquiv(int inp, int out) {
	no_funcs = out+inp; //The inp is room for equivalences.
	//This is the MAX number of rows allowed in the xor table
	
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
	first_bit_sze = sizeof(int)*inp;
	mask_sze      = sizeof(VecType)*(1+inp/(sizeof(VecType)*8));
		
	//LinearVars
	int vars_sze      = sizeof(EquivVars);
	int vars_ref      = 0;
	int first_bit_ref = vars_ref + vars_sze;
	int mask_ref      = first_bit_ref + first_bit_sze;
	int vecs_v_ref = mask_ref + mask_sze;
	vecs_v_start = vecs_v_ref;
	
	frame_size = vecs_v_ref + vecs_rec_bytes*no_funcs;
		
	frame = (char *)ite_calloc(1, frame_size, 9, "EquivClass memory frame");
	frame_start = (unsigned long)frame;
	
	rec = (EquivVars *)frame;
	no_inp_vars = rec->no_inp_vars = inp; // Number of input variables  
	vindex = rec->index = 0;
	
	vec_size = rec->vec_size = 1+no_inp_vars/(sizeof(VecType)*8);
	
	mask = (VecType *)(frame_start+mask_ref);
	for(int i=0; i < vec_size; i++) mask[i] = (VecType)(-1);
	mask[vec_size-1] -= (1 << sizeof(VecType)*8-1);
	
	first_bit = (int *)(frame_start + first_bit_ref);
	for(int i=0; i < inp; i++) first_bit[i] = null;
	
	for(VecType i=frame_start - frame_size; i< (VecType)vecs_rec_bytes*no_funcs; i++)
	  frame[i] = (char)(-1);
	
	
}

void  deleteEquiv () { 
	ite_free((void **) &frame);
}


// Push copy of this frame into frame of another level
char *copyFrame(char *next_frame) {
	memcpy(next_frame, frame, vecs_v_start+rec->index*vecs_rec_bytes);
	return next_frame;		
}

// Add row to the matrix
int addRow (XORd *xord) {
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
	//Modify this next block of code to also detect when only one 1 occurs.
	
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
	//if vec only has one 1 in it, it is an inference.
	//ret = EnqueueInference(nVariable, bPolarity);
	//if ret == 0? conflict
	
	// If k == -1 then no 1's were found in the new vector.
	if (k == -1) {
		if (vec[vec_size-1]&(1 << (sizeof(VecType)*8-1))) return -1; // Inconsistent. Is this check correct? Should it be on the last bit vs. last word?
		else return 0; // No change
	}
	// Open up a new diagonal column
	unsigned long vec_add = vecs_v_start+rec->index*vecs_rec_bytes+vecs_colm_start;
	*((short int *)(frame_start+vec_add)) = save_first_column;
	first_bit[save_first_column] = rec->index;
	int word = save_first_column/(sizeof(VecType)*8);
	int bit = save_first_column % (sizeof(VecType)*8);
	mask[word] &= (-1 ^ (1 << bit));
	
	// Cancel all 1's in the new column. Currently looks at *all* vectors!
	unsigned long vec_address = frame_start + vecs_v_start;
	for (int i=0 ; i < rec->index ; i++) {
		VecType *vn = (VecType *)vec_address;
		if (vn[word] & (1 << bit)) {
			for (int j=0; j < vec_size; j++) vn[j] ^= vec[j];
			//check vn for inference.
		}
		vec_address += vecs_rec_bytes;
	}
	
	//Check for inferences
	vec_address = frame_start + vecs_v_start;
	for (int i=0 ; i < rec->index ; i++) {
		VecType *vn = (VecType *)vec_address;
		int inference_column = 0;
		
		int nonzerok = -1; //Check to see if there is only 1 nonzero word in the vector
		for (k=vec_size-1 ; k >= 0 ; k--) {
			if((mask[k] & vec[k]) != 0){
		      if(nonzerok != -1){ //more than one nonzero --> no inference
					nonzerok = -1;
					break;
		      }
		      nonzerok = k;
			}
		}
		
		if(nonzerok == -1){ //try next vector		    
			continue;
		}
		
		// Check the nonzero word to see if there is only one bit set:
		VecType tmp;
		if ((tmp = (mask[nonzerok] & vn[nonzerok])) != 0) {
			int hgh = sizeof(VecType)*8-1;
			while (hgh > 0) { 
		      int mid = hgh/2;
		      if((tmp & (unsigned int) ~(-1 << mid+1)) > 0){ // bottom has a 1
					if((tmp & (unsigned int) (-1 << mid+1)) > 0){ // top also has a 1 --> fail
						inference_column = -1;
						break;
					}
					tmp &= (unsigned int) ~(-1 << mid+1);
		      }else { //top must have a 1 (because the equation is consistent)
					inference_column += mid+1;
					tmp >>= mid+1;
		      }
		      hgh /= 2;
			}
			inference_column += nonzerok*(sizeof(VecType)*8);
		}
		
		/* Call Sean's special function to store the inferences
		 if(inference_column != -1)
		 printf("inference column: %d\n",inference_column);		    
		 else
		 printf("no inference\n");		
		 */
	}
	// Insert the new row
	vindex++;
	rec->index++;
	
	
	return 1;
}

void printFrameSize () {
	cout << "Frame: " << frame_size << "\n";
}

   void printMask () {
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

   void printLinearN () {
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
   
   void printLinear () {
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

