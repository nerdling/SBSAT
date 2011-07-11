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
//            +--first_bit[6] = -1         +--first_bit[36] = 9
//            |                              |
//      INT32 00101000000010000001000000000010 00000000000001000000000000000000 ...
//      INT32 00011100001000000000010000000000 00000000000010000000000000000000 ...
//      INT32 00000000000000000111000000000000 00000000000100000000000000000000 ...
//      INT32 00101000000010000001000000000010 10000000001000000000000000000000 ...
//      INT32 00001000000010000000000000000010 01000000010000000000000000000000 ...
//      INT32 00100000000000000001000000000000 00000000100000000000000000000000 ...
//      INT32 00001000000000000000000000000000 00000001000000000000000000000000 ...
//      INT32 00000000010100000001000000100010 00000010000000000000000000000000 ...
//      INT32 00000000010000000001000000000000 10000100000000000000000000000000 ...
//      INT32 00000100000001000000001000000000 00001000000000000000000000000000 ...
//      INT32 11000000010000000000000000000000 00010000000000000000000000000000 ...
//      INT32 00000000010000000000001000000000 00100000000000000000000000000000 ...
//      INT32 ||<-------anything allowed------->||<-------->||<---unused----->| ...
//      INT32 |                                    diagonal                    
//      INT32 +--equality bit                      component                   
//
// The following additional variables are used:
//   first_bit - If x is a column (variable) in the diagonal component then
//           first_bit[x] is the number of the row which has a "1" in that
//           column.  Otherwise first_bit[x] is -1.
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

// #define VecType uint32_t
#define BITS_PER_BYTE 8

int frame_size;
int vecs_rec_bytes;
int no_inp_vars; // The number of input variables 
int vec_size;    // Number of bytes comprising each VecType vector
int mask_ref;
int column_ref;
int vecs_v_ref;
int counters_size;
int mask_size;
int column_size;


void printLinear(XORGElimTableStruct *x);
void printLinearN (XORGElimTableStruct *x);
void PrintXORGElimVector(void *pVector);

//unsigned char
static int bits_in_16bits [0x1u << 16] ;

/* Iterated bitcount iterates over each bit. The while condition sometimes helps
*    terminates the loop earlier */
int iterated_bitcount (unsigned int n) {
	int count=0;    
	while (n) {
		count += n & 0x1u;
		n >>= 1 ;
	}
	return count;
}

void compute_bits_in_16bits () {
	VecType i;
	for(i = 0; i < (VecType)(0x1u<<16); i++)
	  bits_in_16bits[i] = iterated_bitcount(i) ;
	return ;
}

#ifdef BITS_64
ITE_INLINE int precomputed16_bitcount (VecType n){
	// works only for 32-bits
	// fprintf(stderr, " n = %llx\n", n);
	return bits_in_16bits [n & 0xffffu]
	  +  bits_in_16bits [(n >> 16) & 0xffffu] 
	  +  bits_in_16bits [(n >> 32) & 0xffffu] 
	  +  bits_in_16bits [(n >> 48) & 0xffffu] ;
}
#else
ITE_INLINE int precomputed16_bitcount (VecType n){
	// works only for 32-bits
	// fprintf(stderr, " n = %lx\n", n);
	return bits_in_16bits [n & 0xffffu]
	  +  bits_in_16bits [(n >> 16) & 0xffffu];
}
#endif


void LSGBXORGElimTableGetHeurScore(XORGElimTableStruct *x) {
	d7_printf1("    Checking the LSGBXORGElimTableGetHeurScore\n");
	//printLinearN(x);
	VecType *vn = (VecType*)(&(((unsigned char*)(x->frame))[vecs_v_ref]));
	int32_t *last_block = (int32_t*)(&(((unsigned char*)(x->frame))[column_ref]));

	for(int i=0 ; i < x->num_vectors; i++) {

		int32_t local_last_block = *last_block / (sizeof(VecType)*BITS_PER_BYTE);
		int32_t j=0;
		int count = 0;
		for(; j <= local_last_block; j++) count += precomputed16_bitcount(vn[j]);
		
		if (count == 0) {
			vn=(VecType*)&(((unsigned char*)vn)[vecs_rec_bytes]);
			last_block=(int32_t*)&(((unsigned char*)last_block)[vecs_rec_bytes]);
			continue;
		}
		
		// fprintf(stderr, " i = %d count = %d\n", i, count);

		assert ((count-(vn[0]&1))<=(VecType)no_inp_vars);	
		assert ((count-(vn[0]&1))>=0);	
		
		double fScore = LSGBarrXORWeightTrans(count-(vn[0]&1));

		int k;
		for (k=local_last_block ; k >= 0; k--) {
			VecType tmp;
			VecType mask = ~0;
			while ((tmp = vn[k]&mask) != 0) {
				int bit = 0;
				int hgh = sizeof(VecType)*BITS_PER_BYTE-1;
				while (hgh > 0) { //Binary search for leading 1
					int mid = hgh/2;
					VecType tmp_tmp = (VecType (1))<<(mid+1);
					if (tmp >= tmp_tmp) {
						tmp >>= mid+1;
						bit += mid+1;
					}
					hgh /= 2;
				}
				
				int nVar = bit + k*(sizeof(VecType)*BITS_PER_BYTE);
				
//				d2_printf4("%d|%d|%d ", i, nVar, count-(vn[0]&1));
				// fprintf(stderr, "fScore = %f k = %d nVar = %d\n", fScore, k, nVar);

				SimpleSmurfProblemState->arrPosVarHeurWghts[nVar]+=fScore;
				SimpleSmurfProblemState->arrNegVarHeurWghts[nVar]+=fScore;
				//k=0; break;
				VecType tmp_mask = 1;
				tmp_mask <<= bit;
				mask &= ((~(VecType)0) ^ tmp_mask);
				// mask &= ((~0) ^ (1 << bit));
			}
		}
		vn=(VecType*)&(((unsigned char*)vn)[vecs_rec_bytes]);
		last_block=(int32_t*)&(((unsigned char*)last_block)[vecs_rec_bytes]);
	}
}

void initXORGElimTable(int nVars){
	//EquivVars* rec = (EquivVars *)frame;
	no_inp_vars = nVars; // Number of input variables
	vec_size = 1+no_inp_vars/(sizeof(VecType)*BITS_PER_BYTE);

	compute_bits_in_16bits();
	
	/* counters: INT32 INT32 .. no_inp_vars+1 times */
	/* mask:     VecType VecType ..  vec_size times */ 
	/* rows:     INT32 VecType VecType .. vec_size times */
	
	counters_size = sizeof(int32_t)*(no_inp_vars+1);
	mask_ref = counters_size; // sizeof(int32_t)*(no_inp_vars+1); // for every variable it holds diagonal row offset 
	mask_size = sizeof(VecType)*vec_size;
	column_ref = mask_ref + mask_size; // sizeof(VecType)*vec_size;
	column_size = sizeof(int32_t) + vec_size*sizeof(VecType);

	vecs_rec_bytes = column_size; // sizeof(int32_t) + vec_size*sizeof(VecType); // counter + bitvectors //mask_size = vecs_rec_bytes
	vecs_v_ref = column_ref + sizeof(int32_t); // counter
}

void allocXORGElimTable(XORGElimTableStruct *x, int no_funcs){
	if(x->frame == NULL) {
		frame_size = counters_size + mask_size + column_size*no_funcs;
		x->frame = (unsigned char *)ite_calloc(1, frame_size, 9, "Gaussian elimination table memory frame");
		
		x->no_funcs = no_funcs;
		
		x->first_bit = (int32_t *)(x->frame);
		for(int i=0; i <= no_inp_vars; i++) x->first_bit[i] = -1;
		
		x->mask = (void *)(&(((unsigned char*)(x->frame))[mask_ref]));
		for(int i=0; i < vec_size; i++) ((VecType*)(x->mask))[i] = (VecType)(~0);
		((VecType*)(x->mask))[0] -= 1;
		
		//Zero out new rows
		int32_t *vv = (int32_t *)&(((unsigned char*)(x->frame))[column_ref]);
		if(no_funcs > 0) *vv = -1;
		for (int i=1 ; i < x->no_funcs; i++) {
			vv=(int32_t*)&(((unsigned char*)vv)[vecs_rec_bytes]);
			*vv = -1;
		}

		x->num_vectors = 0;
	} else if(x->no_funcs < no_funcs) {
		int old_frame_size = counters_size + mask_size + column_size*x->no_funcs;
		frame_size = counters_size + mask_size + column_size*no_funcs;
		x->frame = (unsigned char *)ite_recalloc(x->frame, old_frame_size, frame_size, 1, 9, "Gaussian elimination table memory frame");
		
		x->first_bit = (int32_t *)(x->frame);

		x->mask = (void *)(&(((unsigned char*)(x->frame))[mask_ref]));
		
		//Zero out new rows
		int32_t *vv = (int32_t *)&(((unsigned char*)(x->frame))[column_ref+((x->no_funcs-1)*vecs_rec_bytes)]);
		for (int i=x->no_funcs; i < no_funcs; i++) {
			vv=(int32_t*)&(((unsigned char*)vv)[vecs_rec_bytes]);
			*vv = -1;
		}
		x->no_funcs = no_funcs;
	}
}

void deleteXORGElimTable (XORGElimTableStruct *x) { 
   if(x!=NULL)
     ite_free((void **) &(x->frame));
}

// Push a copy of this frame into the frame of another level
void pushXORGElimTable(XORGElimTableStruct *curr, XORGElimTableStruct *dest) {
//	d2_printf3("Pushing Table - %d (%d)\n", curr->num_vectors, SimpleSmurfProblemState->nCurrSearchTreeLevel+1);
	// printLinearN(curr);

	allocXORGElimTable(dest, curr->no_funcs);

	memcpy_ite(dest->frame, curr->frame, column_ref + curr->num_vectors*vecs_rec_bytes);
	dest->num_vectors = curr->num_vectors;

	// printLinearN(dest);
}

void *createXORGElimTableVector(int nvars, int *varlist, bool bParity) {
	VecType *vector = (VecType *)ite_calloc(vec_size, sizeof(VecType), 9, "VecType *vector");
	for (int i=0 ; i < nvars; i++) {
		int word = varlist[i]/(sizeof(VecType)*BITS_PER_BYTE);
		int bit = varlist[i]%(sizeof(VecType)*BITS_PER_BYTE);		
		vector[word] += ((VecType)1 << bit);
	}
	if(bParity) vector[0]|=(VecType)1;
	 
	// fprintf(stderr, "  nvars = %d iblahsdf0 = %llx\n", nvars, vector[0]);
	return (void *)vector;
}

ITE_INLINE int rediagonalizeXORGElimTable(XORGElimTableStruct *x, VecType *vec, int loc) {
	// Now that all the 1's in the diagonal submatrix are taken care of,
	// scan the vector to find the first 1 (MSB).  The variable (column)
	// which is found is stored in "save_first_column".
	int save_first_column = 0;
	int k;
	for (k=vec_size-1 ; k >= 0 ; k--) {
		// Maybe 10 of these loops
		VecType tmp;
		if ((tmp = (((VecType*)(x->mask))[k] & vec[k])) != 0) {
			int hgh = sizeof(VecType)*BITS_PER_BYTE-1;
			while (hgh > 0) { // Maybe 5 of these loops - binary search for leading 1
				int mid = hgh/2;
				VecType tmp_tmp = (VecType (1))<<(mid+1);
				if (tmp >= tmp_tmp) {
					tmp >>= mid+1;
					save_first_column += mid+1;
				}
				hgh /= 2;
			}

			save_first_column += k*(sizeof(VecType)*BITS_PER_BYTE);
			break;
		}
	}

	// If k == -1 then no 1's were found in the new vector.
	if (k == -1) {
		if (vec[0]&1) {
         //SEAN!!! Here one would need to add sort of a strange lemma
         return 0; // Inconsistent.
      }
		else return 2; // No change
	}
	
	// Open up a new diagonal column

	int word = k;//save_first_column/(sizeof(VecType)*BITS_PER_BYTE);
	int bit = save_first_column % (sizeof(VecType)*BITS_PER_BYTE);

	((VecType*)(x->mask))[word] &= ((~0) ^ ((VecType)1 << bit));
	x->first_bit[save_first_column] = loc;

	*(int32_t*)(&(((unsigned char*)(x->frame))[column_ref+loc*vecs_rec_bytes])) = save_first_column;
	
	//Look for second 1. If doesn't exist --> vec gives inference.
	for(; k>=0 ; k--)
		if ((((VecType*)(x->mask))[k] & vec[k]) != 0) break;

	// If k == -1 then we have an inference
	if (k == -1) {
		if(EnqueueInference(save_first_column, vec[0]&1, INF_BB_GELIM) == 0)
		  return 0;
		//return 2;
	}

	// Cancel all 1's in the new column. Currently looks at *all* vectors!
	//VecType *vec_address = (VecType*)(&(((unsigned char*)(x->frame))[vecs_v_ref]));
	// fprintf(stderr, " vstart = %llx\n", vec_address[0]);
	// VecType *vn = (VecType*)(&(((unsigned char*)(x->frame))[vecs_v_ref+i*vecs_rec_bytes]));
	VecType *vn = (VecType*)(&(((unsigned char*)(x->frame))[vecs_v_ref]));
	for (int i=0 ; i < x->num_vectors; i++) {
		if(i == loc) { vn=(VecType*)&(((unsigned char*)vn)[vecs_rec_bytes]); continue; }
		if (vn[word] & ((VecType)1 << bit)) {
			bool nonzero = 0;
			int j=0;
			for (; j <= word; j++) nonzero |= (((VecType*)(x->mask))[j] & (vn[j] ^= vec[j]))!=0;
			for (; j < vec_size && !nonzero; j++) nonzero |= (((VecType*)(x->mask))[j] & vn[j])!=0;
//			d2_printf1("b    ");PrintXORGElimVector(vn);d2_printf2(" %d\n", i);
			
			if(!nonzero){
				//Inference
				int32_t inf = *(int32_t*)(&(((unsigned char*)(x->frame))[column_ref+i*vecs_rec_bytes]));
				assert(inf != -1);
				
				int inf_word = inf/(sizeof(VecType)*BITS_PER_BYTE);
				int inf_bit = inf%(sizeof(VecType)*BITS_PER_BYTE);
				
				//printLinearN(x);
				if(vn[inf_word]&((VecType)1 << inf_bit))
				  if(EnqueueInference(inf, vn[0]&(VecType)1, INF_BB_GELIM) == 0)
					 return 0;
			}
		}
		vn=(VecType*)&(((unsigned char*)vn)[vecs_rec_bytes]);
	}
	// fprintf(stderr, " v = %llx\n", vec_address[0]);
	return 1;
}

// Add row to the matrix
int addRowXORGElimTable (XORGElimTableStruct *x, void *pVector, int nVars, int *pnVarlist) {
	assert(nVars > 1);

	d7_printf1("    Checking the addRowXORGElimTable\n");
	
	if(x->num_vectors >= x->no_funcs) {
		allocXORGElimTable(x, x->num_vectors+1);
	   //assert(0); // Cannot add anymore vectors to the matrix
	}
	
	// Grab a new Vector and copy vector info to it
	VecType *vec = (VecType*)(&(((unsigned char*)(x->frame))[vecs_v_ref+x->num_vectors*vecs_rec_bytes]));

//	d2_printf1("i    ");PrintXORGElimVector(pVector);d2_printf1("\n");
	
	memcpy_ite(vec, pVector, sizeof(VecType)*vec_size);
	
	// The first 1 bit of the new vector is in a column which intersects
	// the diagonal.  Add the existing such row to the new vector.  For
	// all 1's of new vector in columns which intersect the diagonal, in
	// decreasing order, add rows to new vector.  While doing this,
	// locate the first 1 in a column intersecting the diagonal.  Open
	// this column to the diagonal.
	// 
	// Eliminate all 1's of the new vector in the current diagonal matrix
	for (int i=0 ; i < nVars; i++) {
		int v;
		if ((v = x->first_bit[pnVarlist[i]]) != -1) {
			VecType *vn = (VecType*)(&(((unsigned char*)(x->frame))[vecs_v_ref+v*vecs_rec_bytes]));
			for (int j=0 ; j <= (int)(pnVarlist[i]/(sizeof(VecType)*BITS_PER_BYTE)); j++) vec[j] ^= vn[j];
//			d2_printf1("a    ");PrintXORGElimVector(vec);d2_printf2(" %d\n", v);
		}
	}

	// Insert the new row
	
	int ret = rediagonalizeXORGElimTable(x, vec, x->num_vectors);
	if(ret == 1) x->num_vectors++;
	if(ret == 0) return 0;
	return 1;
}

// When a variable is given an assignment, 0 out that column of all rows.
// Check for inconsistency - if 0 row = 1 last column
// One hitch - if column is in the diagonal submatrix, then look for 1st
// column of that row intersecting the diagonal which is a 1. Rediagonal-
// ize as though a new row has just been added.
int ApplyInferenceToXORGElimTable (XORGElimTableStruct *x, int nVar, bool bValue) {
	int v;

	d7_printf1("    Checking the XORGETable\n");

	//printLinearN(x);
	
	//Grab word and bit positions of x->mask and vector
	int word = nVar/(sizeof(VecType)*BITS_PER_BYTE);
	int bit = nVar%(sizeof(VecType)*BITS_PER_BYTE);

	// Check whether column is in diagonal submatrix
	if((v=x->first_bit[nVar]) != -1) {
		// If so,
		// Zero out the diagonal and if the value of the variable of the
		// column is 1, reverse the value of the last column

		assert((((VecType*)(x->mask))[word] & ((VecType)1 << bit)) == 0);
		VecType *vec = (VecType*)(&(((unsigned char*)(x->frame))[vecs_v_ref+v*vecs_rec_bytes]));
		if(bValue) {
			vec[0]=vec[0]^(VecType)1;
		}
		assert((vec[word]&(((VecType)1) << bit))>0);
		vec[word] ^= ((VecType)1 << bit); //Remove var from the vector
		//x->first_bit[nVar] = -1;
		
		//Rediagonalize the vector.
		return (rediagonalizeXORGElimTable(x, vec, v)>0);
	} else {
		// If the column is not in the diagonal submatrix
		// Zero out the column and set the x->mask bit to 0
		((VecType*)(x->mask))[word] &= ((~(VecType)0) ^ ((VecType)1 << bit));

		VecType *vn = (VecType*)(&(((unsigned char*)(x->frame))[vecs_v_ref]));
		for (int i=0 ; i < x->num_vectors; i++) {
			// VecType *vn = (VecType *)vec_address;
			// If the row has a 1 in the zeroed column and the value of the
			// variable in the column is 1 then reverse the value of the last
			// column
			if(vn[word] & ((VecType)1 << bit)) {
				vn[word] ^= ((VecType)1 << bit); //Remove var from the vector
				if(bValue) {
					vn[0]=vn[0]^(VecType)1;			
				}

				//Check to see if this vector has become an inference i.e. only has 1 one (the diagonal column bit).
				bool nonzero = 0;

				for (int j=0; j < vec_size && !nonzero; j++) nonzero |= (((VecType*)(x->mask))[j] & vn[j])!=0;
				
				if(!nonzero){
					//Inference
					
					int32_t inf = *(int32_t*)(&(((unsigned char*)(x->frame))[column_ref+i*vecs_rec_bytes]));
					assert(inf != -1);
	
					int inf_word = inf/(sizeof(VecType)*BITS_PER_BYTE);
					int inf_bit = inf%(sizeof(VecType)*BITS_PER_BYTE);
					
//					d2_printf5("Attemping to Infer %d %d %d %d\n", inf*((vn[0]&1)?1:-1), inf_word, inf_bit, vn[inf_word]&(1 << inf_bit));
					//printLinearN(x);
					if(vn[inf_word]&((VecType)1 << inf_bit))
					  if(EnqueueInference(inf, vn[0]&1, INF_BB_GELIM) == 0)
						 return 0;
				}
			}
         vn=(VecType*)&(((unsigned char*)vn)[vecs_rec_bytes]);
      }
	}
   
	return 1; // Normal ending
}

void printframeSize () {
	cout << "frame: " << frame_size << "\n";
}

int isMaskZero(XORGElimTableStruct *x) {
   for (int i=0 ; i < vec_size ; i++) {
      if(((VecType*)(x->mask))[i]!=0) return 0;
   }
   return 1;
}

int isVectorZero(XORGElimTableStruct *x, void *pVector) {
	for (int word=0 ; word < vec_size; word++) {
		if (((VecType *)pVector)[word] & ((VecType *)(x->mask))[word]) return 0;
	}
	return 1;
}

int nNumActiveXORGElimVectors(XORGElimTableStruct *x) {
	int nNumActiveVectors = 0;
	for (int i=0 ; i < x->num_vectors; i++) {
		if(!isVectorZero(x, (&(((unsigned char*)(x->frame))[vecs_v_ref+i*vecs_rec_bytes]))))
		  nNumActiveVectors++;
	}
	return nNumActiveVectors;
}		  

void printMask (XORGElimTableStruct *x) {
	d2_printf2("mask (%lx", vec_size*sizeof(VecType)*BITS_PER_BYTE);
	d2_printf1(" bits):\n     ");
	for (int i=0 ; i < vec_size ; i++) {
		VecType tmp = ((VecType*)(x->mask))[i];
		for (unsigned int j=i==0?1:0 ; j < sizeof(VecType)*BITS_PER_BYTE ; j++) {
			if (tmp % 2) {d2_printf1("1");} else {d2_printf1("0");}
			tmp /= 2;
		}
	}

	d2_printf1(".");
	if (((VecType*)(x->mask))[0]&1) {
		d2_printf1("1");
	} else { d2_printf1("0"); }
	
	d2_printf1("\n");
}

void PrintXORGElimVector(void *pVector) {
	for (int word=0 ; word < vec_size; word++) {
		for(int bit = word==0?1:0; bit < (int)(sizeof(VecType)*BITS_PER_BYTE); bit++) {
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

void PrintMaskXORGElimVector(XORGElimTableStruct *x, void *pVector) {
	for (int word=0 ; word < vec_size; word++) {
		for(int bit = word==0?1:0; bit < (int)(sizeof(VecType)*BITS_PER_BYTE); bit++) {
			if (((VecType *)pVector)[word] & ((VecType)1 << bit) & ((VecType*)(x->mask))[word]) {
				d2_printf1("1");
			} else { d2_printf1("0"); }
		}
	}
	
	d2_printf1(".");
	if (((VecType *)pVector)[0]&1) {
		d2_printf1("1");
	} else { d2_printf1("0"); }
}

void printLinearN (XORGElimTableStruct *x) {
	printMask(x);
	d2_printf1("Vectors:\n");
	for (int i=0 ; i < x->num_vectors; i++) {
		d2_printf1("     ");
		VecType *vn = (VecType*)(&(((unsigned char*)(x->frame))[vecs_v_ref+i*vecs_rec_bytes]));
		PrintXORGElimVector((void *)vn);
		d2_printf1("     ");
		PrintMaskXORGElimVector(x, (void *)vn);
		// d2_printf2(" %d ",((int *)(((VecType*)x->frame) + column_ref + i*vecs_rec_bytes))[0]);
		d2_printf1("\n");		
	}
	d2_printf1(" +-----+     +-----+     +-----+     +-----+     +-----+\n");
}

void printLinear (XORGElimTableStruct *x) {
	int xlate[512];
	int rows[512];
	//printLinearN(x);
	for (int i=0 ; i < 512 ; i++) xlate[i] = i;
	int j=0;
	for (int i=no_inp_vars-1 ; i >= 0 ; i--)
	  if (x->first_bit[i] != -1) {
		  rows[j] = x->first_bit[i]; xlate[j++] = i;
	  }
	
	rows[j] = -1;
	for (int i=no_inp_vars-1 ; i >= 0 ; i--)
	  if (x->first_bit[i] == -1) xlate[j++] = i;
	
	cout << "x->mask (" << vec_size*sizeof(VecType)*BITS_PER_BYTE << " bits):\n     ";
	for (int i=0 ; i < no_inp_vars ; i++) {
		int word = xlate[i]/(sizeof(VecType)*BITS_PER_BYTE);
		int bit  = xlate[i] % (sizeof(VecType)*BITS_PER_BYTE);
		if (((VecType*)(x->mask))[word] & (1 << bit)) cout << "1"; else cout << "0";
	}
	cout << "\n";
	
	cout << "Vectors:\n";
	for (int i=0 ; rows[i] >= 0 ; i++) {
		cout << "     ";
		VecType *vn = (VecType *)(((VecType*)x->frame)+vecs_v_ref+rows[i]*vecs_rec_bytes);
		for (j=0 ; j < no_inp_vars ; j++) {
			int word = xlate[j]/(sizeof(VecType)*BITS_PER_BYTE);
			int bit  = xlate[j] % (sizeof(VecType)*BITS_PER_BYTE);
			if (vn[word] & (1 << bit)) cout << "1"; else cout << "0";
		}
		cout << ".";
		if (vn[0]&1)
		  cout << "1"; else cout << "0";
		cout << "     ";
		for (j=0 ; j < no_inp_vars ; j++) {
			int word = xlate[j]/(sizeof(VecType)*BITS_PER_BYTE);
			int bit  = xlate[j] % (sizeof(VecType)*BITS_PER_BYTE);
			if (vn[word] & ((VecType*)(x->mask))[word] & (1 << bit)) cout << "1"; else cout << "0";
		}
		cout << ".";
		if (vn[0]&1)
		  cout << "1"; else cout << "0";
		cout << "\n";
	}
	cout << "========================================================\n";
}
