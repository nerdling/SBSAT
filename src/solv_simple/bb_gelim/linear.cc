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
#include "sbsat_solver.h"
#include "solver.h"

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
//      00011100001000000000010000000000 00000000000010000000000000000000
//      00000000000000000111000000000000 00000000000100000000000000000000
//      00101000000010000001000000000010 10000000001000000000000000000000
//      00001000000010000000000000000010 01000000010000000000000000000000
//      00100000000000000001000000000000 00000000100000000000000000000000
//      00001000000000000000000000000000 00000001000000000000000000000000
//      00000000010100000001000000100010 00000010000000000000000000000000
//      00000000010000000001000000000000 10000100000000000000000000000000
//      00000100000001000000001000000000 00001000000000000000000000000000
//      11000000010000000000000000000000 00010000000000000000000000000000
//      00000000010000000000001000000000 00100000000000000000000000000000
//      ||<-------anything allowed------->||<-------->||<---unused----->|
//      |                                    diagonal                    
//      +--equality bit                      component                   
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

/*
struct XORGElimTableStruct {
	char *frame;
	int *first_bit; // first_bit[i]: pointer to vector on diagonal with first 1 bit at column i
   unsigned long *mask; // 0 bits are columns of the diagonalized submatrix or assigned variables
   int num_vectors;
 };
*/

#define VecType unsigned long

char *frame;
unsigned long *mask; // 0 bits are columns of the diagonalized submatrix or assigned variables
int *first_bit; // first_bit[i]: pointer to vector on diagonal with first 1 bit at column i
int num_vectors;

int frame_size;
int vecs_rec_bytes;
int no_inp_vars; // The number of input variables 
int no_funcs;    // The max number of functions (rows in matrix)
int vec_size;    // Number of bytes comprising each VecType vector
int first_bit_ref;
int mask_ref;
int vecs_v_ref;

void initXORGElimTable(int nFuncs, int nVars){
	//EquivVars* rec = (EquivVars *)frame;
	no_funcs = nFuncs; //The inp is room for equivalences.
	no_inp_vars = nVars; // Number of input variables
	num_vectors = 0;
	vec_size = 1+no_inp_vars/(sizeof(VecType)*8);
	
	vecs_rec_bytes = vec_size*sizeof(VecType); //mask_size = vecs_rec_bytes
	
	first_bit_ref = 0;
	mask_ref = first_bit_ref + sizeof(int)*(no_inp_vars+1);
	vecs_v_ref = mask_ref + vecs_rec_bytes;
	frame_size = vecs_v_ref + vecs_rec_bytes*no_funcs;	
}

void allocXORGElimTable(XORGElimTableStruct *x){
	x->frame = (char *)ite_calloc(1, frame_size, 9, "Gaussian elimination table memory frame");

	x->first_bit = (int *)(x->frame + first_bit_ref);
	for(int i=0; i <= no_inp_vars; i++) x->first_bit[i] = -1;

	x->mask = (VecType *)(x->frame + mask_ref);
	for(int i=0; i < vec_size; i++) x->mask[i] = (VecType)(-1);
	x->mask[0] -= 1;
	
	char *vv = (char *)(x->frame + vecs_v_ref);
	for (int i=0 ; i < (VecType)vecs_rec_bytes*no_funcs; i++)
	  vv[i] = (char)(-1);
}

void deleteXORGElimTable () { 
  ite_free((void **) &(frame));
}

// Push a copy of this frame into the frame of another level
void pushXORGElimTable(XORGElimTableStruct *x) {
	memcpy_ite(x->frame, frame, vecs_v_ref + num_vectors*vecs_rec_bytes);
	x->first_bit = first_bit;
	x->mask = mask;
	x->num_vectors = num_vectors;
}

//Pop this frame off the stack and revert to a previously saved frame
void popXORGElimTable(XORGElimTableStruct *x) {
	frame = x->frame;
	first_bit = x->first_bit;
	mask = x->mask;
	num_vectors = x->num_vectors;
}

void *createXORGElimTableVector(int nvars, int *varlist, bool bParity) {
	VecType *vector = (VecType *)ite_calloc(vec_size, sizeof(VecType), 9, "VecType *vector");
	for (int i=0 ; i < nvars; i++) {
		int word = varlist[i]/(sizeof(VecType)*8);
		int bit = varlist[i]%(sizeof(VecType)*8);		
		vector[word] += (1 << bit);
	}
	if(bParity) vector[0]&1;
	
	return (void *)vector;
}

// Add row to the matrix
int addRowXORGElimTable (void *pVector, int nVars, int *pnVarlist) {
	assert(nVars > 0);
	
	if(num_vectors >= no_funcs) {
		return 0; // Cannot add anymore vectors to the matrix
	}
	
	// Grab a new Vector and copy vector info to it
	unsigned long offset = ((unsigned long)frame) + vecs_v_ref + num_vectors*vecs_rec_bytes;
	VecType *vec = (VecType*)offset;

	memcpy(vec, pVector, sizeof(VecType)*vec_size);
	
	// The first 1 bit of the new vector is in a column which intersects
	// the diagonal.  Add the existing such row to the new vector.  For
	// all 1's of new vector in columns which intersect the diagonal, in
	// decreasing order, add rows to new vector.  While doing this,
	// locate the first 1 in a column intersecting the diagonal.  Open
	// this column to the diagonal.
	// 
	// Eliminate all 1's of the new vector in the current diagonal matrix
	for (int i=0 ; i < nVars; i++) {
		short int v;
		if ((v = first_bit[pnVarlist[i]]) != -1) {
			VecType *vn = (VecType *)(((unsigned long)frame)+vecs_v_ref+v*vecs_rec_bytes);
			for (int j=0 ; j <= v/(sizeof(VecType)*8); j++) vec[j] ^= vn[j];
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
		if (vec[0]&1) return -1; // Inconsistent. 
		else return 0; // No change
	}
	
	// Open up a new diagonal column

	first_bit[save_first_column] = num_vectors;
	int word = k;//save_first_column/(sizeof(VecType)*8);
	int bit = save_first_column % (sizeof(VecType)*8);
	mask[word] &= (-1 ^ (1 << bit));

	//Look for second 1. If doesn't exist --> vec gives inference.
	int save_second_column = 0;
	for(; k>=0 ; k--){
		// Maybe 10 of these loops
		VecType tmp;
		if ((tmp = (mask[k] & vec[k])) != 0) {
			int hgh = sizeof(VecType)*8-1;
			while (hgh > 0) { // Maybe 5 of these loops - binary search for leading 1
				int mid = hgh/2;
				if (tmp >= (unsigned int)(1 << mid+1)) {
					tmp >>= mid+1;
					save_second_column += mid+1;
				}
				hgh /= 2;
			}
			
			save_second_column += k*(sizeof(VecType)*8);
			break;
		}
	}

	// If k == -1 then we have an inference
	if (k == -1) {
	  int ret = EnqueueInference(save_first_column, vec[0]&1);
	  if(ret == 0)
	    return -1; //Is this the return value I want?			    
	}

	// Cancel all 1's in the new column. Currently looks at *all* vectors!
	unsigned long vec_address = ((unsigned long)frame) + vecs_v_ref;
	for (int i=0 ; i < num_vectors ; i++) {
		VecType *vn = (VecType *)vec_address;
		if (vn[word] & (1 << bit)) {
			bool nonzero = 0;
			int j=0;
			for (; j <= word; j++) nonzero &= mask[j] & (vn[j] ^= vec[j]);
			for (; j < vec_size && !nonzero; j++) nonzero &= mask[j] & vn[j];
			
			if(!nonzero){
				//Inference
				int inference_column = 0;
				
				for (k=vec_size-1 ; k >= 0 ; k--) {
					// Maybe 10 of these loops
					VecType tmp;
					if ((tmp = vn[k]) != 0) {
						int hgh = sizeof(VecType)*8-1;
						while (hgh > 0) { // Maybe 5 of these loops - binary search for leading 1
							int mid = hgh/2;
							if (tmp >= (unsigned int)(1 << mid+1)) {
								tmp >>= mid+1;
								inference_column += mid+1;
							}
							hgh /= 2;
						}
						
						inference_column += k*(sizeof(VecType)*8);
						break;
					}
				}
				assert(inference_column);
				
				int ret = EnqueueInference(inference_column, vn[0]&1);				
				if(ret == 0)
				  return -1; //Is this the return value I want?			    
			}
		}
		vec_address += vecs_rec_bytes;
	}
	
	// Insert the new row
	num_vectors++;
	
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
	for (int i=0 ; i < num_vectors ; i++) {
		cout << "     ";
		VecType *vn = (VecType *)(((unsigned long)frame)+vecs_v_ref+i*vecs_rec_bytes);
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

void printXORGElimVector(void *pVector) {
	for (int word=0 ; word < vec_size; word++) {
		for(int bit = word==0?1:0; bit < (sizeof(VecType)*8); bit++) {
			if (((VecType *)pVector)[word] & (1 << bit)) {
				d2_printf1("1");
			} else { d2_printf1("0"); }
		}
	}
	
	d2_printf1(".");
	if (((VecType *)pVector)[0]&1) {
		d2_printf1("1");
	} else { d2_printf1("0"); }
}

void printLinear () {
	int xlate[512];
	int rows[512];
	printLinearN ();
	for (int i=0 ; i < 512 ; i++) xlate[i] = i;
	int j=0;
	for (int i=no_inp_vars-1 ; i >= 0 ; i--)
	  if (first_bit[i] != -1) {
		  rows[j] = first_bit[i]; xlate[j++] = i;
	  }
	
	rows[j] = -1;
	for (int i=no_inp_vars-1 ; i >= 0 ; i--)
	  if (first_bit[i] == -1) xlate[j++] = i;
	
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
		VecType *vn = (VecType *)(((unsigned long)frame)+vecs_v_ref+rows[i]*vecs_rec_bytes);
		for (int j=0 ; j < no_inp_vars ; j++) {
			int word = xlate[j]/(sizeof(VecType)*8);
			int bit  = xlate[j] % (sizeof(VecType)*8);
			if (vn[word] & (1 << bit)) cout << "1"; else cout << "0";
		}
		cout << ".";
		if (vn[0]&1)
		  cout << "1"; else cout << "0";
		cout << "     ";
		for (int j=0 ; j < no_inp_vars ; j++) {
			int word = xlate[j]/(sizeof(VecType)*8);
			int bit  = xlate[j] % (sizeof(VecType)*8);
			if (vn[word] & mask[word] & (1 << bit)) cout << "1"; else cout << "0";
		}
		cout << ".";
		if (vn[0]&1)
		  cout << "1"; else cout << "0";
		cout << "\n";
	}
	cout << "========================================================\n";
}
