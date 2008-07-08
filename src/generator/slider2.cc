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

#include <stdio.h>
#include <sys/time.h>
#include "sbsat.h"

void slider2_sat(char *out_type, int n);
void slider2_unsat(char *out_type, int n);
void slider2_unsat_iscas(int n);
void slider2_unsat_trace(int n);

void slider2(char *out_type, int n, int sat) 
{
   if (sat == 0) slider2_unsat(out_type, n);
   else if (sat == 1) slider2_sat(out_type, n);
	else if (sat == 2) slider2_unsat_iscas(n);
	else if (sat == 3) slider2_unsat_trace(n);
	else {
		fprintf(stderr, "Error: Unknown out_type\n");
		exit(0);
	}
}

/*
 * sliders
 *
 * unsat
 * (first function -- notice 1,2,3,4,5,6)
   #define add_state1(1, 2, 3, 4, 5, 6)
   #equ(xor(1, and(-3, 5), nand(6, 4)), ite(2, or(5, -6), -5)))
 *
 * (second/third function)
   #define add_state2(1, 2, 3, 4, 5, 6, 7)
   #equ(6, xor(-1, xor(3, and(-4, 5), 4), equ(7, 2)))
   #define add_state3(1, 2, 3, 4, 5, 6, 7)
   #not(equ(6, xor(-1, xor(3, and(-4, 5), 4), equ(7, 2))))
 *
 * sat
 * (first function -- notice 1,2,3,5,4,6 -- 5,4 switched)
   #define add_state1(1, 2, 3, 5, 4, 6)\n");
   #equ(xor(1, and(-3, 5), nand(6, 4)), ite(2, or(5, -6), -5)))
 *
 * (second/third functions - notice -- they are the same)
   #define add_state2(1, 2, 3, 4, 5, 6)
   #xor(-1, xor(3, and(-4, 5), 4), equ(6, 2))
   #define add_state3(1, 2, 3, 4, 5, 6)
   #xor(-1, xor(3, and(-4, 5), 4), equ(6, 2))
 *
 * numbers are generated the same way for both sat and unsat
   int start1[6] = {1, 2*s+3, 2*s+1, n/2-1-3*s, n/2-1, n/2};
   int start2[7] = {1, 2*s-1, 2*s+2, n/2-1-4*s, n/2-1-2*s, n/2-1-s, n/2};
 *
 * Please note that UNSAT version seems to be harder than SAT.
 *
 * Disclaimer: no formal analysis was done to verify SAT and UNSAT
 * This means that for some n SAT might return UNSAT and UNSAT might return SAT problem.
 * Please use the solver to verify UN/SAT.
 *
 * I have tested n for mulple of 10 starting 20.
 * SAT instances seem to have at most 2 solutions.
 */

void slider2_sat(char *out_type, int n)
{
   int s=n/20;
   int start1[6] = {1, 2*s+3, 2*s+1, n/2-1-3*s, n/2-1, n/2};
   int start2[7] = {1, 2*s-1, 2*s+2, n/2-1-4*s, n/2-1-2*s, n/2-1-s, n/2};
/*
   fprintf(stderr, "{%d, %d, %d, %d, %d, %d}\n", 
         start1[0], start1[1], start1[2], start1[3], start1[4], start1[5]);
   fprintf(stderr, "{%d, %d, %d, %d, %d, %d, %d}\n", 
         start2[0], start2[1], start2[2], start2[3], start2[4], start2[5], start2[6]);
*/
   printf("p bdd %d %d\n", n, n);
   printf("; automatically generated SAT slider2 with n=%d \n", n);
   printf("; Disclaimer: no formal analysis was done to verify SAT and UNSAT\n");

   // first function
   printf("#define add_state1(1, 2, 3, 5, 4, 6)\n");
   printf("#equ(xor(1, and(-3, 5), nand(6, 4)), ite(2, or(5, -6), -5)))\n");
   for(int i=0;i<n/2;i++) {
      printf("add_state1(%d, %d, %d, %d, %d, %d)\n",
            start1[0], start1[1], start1[2], start1[3], start1[4], start1[5]);
      start1[0]++;
      start1[1]++;
      start1[2]++;
      start1[3]++;
      start1[4]++;
      start1[5]++;
   }
   // second and third function
   printf("#define add_state2(1, 2, 3, 4, 5, 6)\n");
   printf("#xor(-1, xor(3, and(-4, 5), 4), equ(6, 2))\n");
   printf("#define add_state3(1, 2, 3, 4, 5, 6)\n");
   printf("#xor(-1, xor(3, and(-4, 5), 4), equ(6, 2))\n");
   for(int i=0;i<n-n/2;i++) {
      printf("add_state%d(%d, %d, %d, %d, %d, %d)\n", (i%2)+2,
            start2[0], start2[1], start2[2], start2[3], start2[4], start2[6]);
      start2[0]++;
      start2[1]++;
      start2[2]++;
      start2[3]++;
      start2[4]++;
      start2[5]++;
      start2[6]++;
   }
}

void slider2_unsat_iscas(int n)
{
   int s=n/20;
   int start1[6] = {1, 2*s+3, 2*s+1, n/2-1-3*s, n/2-1, n/2};
   int start2[7] = {n, 2*s-1, 2*s+2, n/2-1-4*s, n/2-1-2*s, n/2-1-s, n/2};
	int tmpnum = 0;

   for(int i=(n/2)+1;i<=n;i++)
	  printf("INPUT(v_%d)\n", i);

	printf("OUTPUT(MITER)\n");

   // first function
   for(int i=0;i<n/2;i++) {
	   printf("g_%d  = NOT(v_%d)\n", tmpnum+1, start1[1]);
      printf("g_%d  = NOT(v_%d)\n", tmpnum+2, start1[2]);
      printf("g_%d  = OR(g_%d, g_%d)\n", tmpnum+3, tmpnum+1, tmpnum+2);
      printf("g_%d  = OR(v_%d, v_%d)\n", tmpnum+4, start1[1], start1[2]);
      printf("g_%d  = AND(g_%d, g_%d)\n", tmpnum+5, tmpnum+3, tmpnum+4);
      printf("g_%d  = NOT(g_%d)\n", tmpnum+6, tmpnum+5);
      printf("g_%d  = AND(v_%d, g_%d)\n", tmpnum+7, start1[4], tmpnum+5);
      printf("g_%d  = NOT(v_%d)\n", tmpnum+8, start1[4]);
      printf("g_%d  = AND(v_%d, g_%d)\n", tmpnum+9, start1[4], tmpnum+6);
      printf("g_%d = AND(g_%d, g_%d)\n", tmpnum+10, tmpnum+8, tmpnum+1);
      printf("g_%d = OR(g_%d, g_%d)\n", tmpnum+11, tmpnum+9, tmpnum+10);
      printf("g_%d = AND(v_%d, g_%d)\n", tmpnum+12, start1[4], tmpnum+5);
      printf("g_%d = AND(g_%d, v_%d)\n", tmpnum+13, tmpnum+8, start1[1]);
      printf("g_%d = OR(g_%d, g_%d)\n", tmpnum+14, tmpnum+12, tmpnum+13);
      printf("g_%d = AND(v_%d, g_%d)\n", tmpnum+15, start1[3], tmpnum+11);
      printf("g_%d = NOT(v_%d)\n", tmpnum+16, start1[3]);
      printf("g_%d = AND(g_%d, g_%d)\n", tmpnum+17, tmpnum+16, tmpnum+14);
      printf("g_%d = OR(g_%d, g_%d)\n", tmpnum+18, tmpnum+15, tmpnum+17);
      printf("g_%d = AND(v_%d, g_%d)\n", tmpnum+19, start1[5], tmpnum+18);
      printf("g_%d = NOT(v_%d)\n", tmpnum+20, start1[5]);
      printf("g_%d = AND(g_%d, g_%d)\n", tmpnum+21, tmpnum+20, tmpnum+7);
      printf("v_%d = OR(g_%d, g_%d)\n", start1[0], tmpnum+19, tmpnum+21);

		tmpnum+=21;
      start1[0]++;
      start1[1]++;
      start1[2]++;
      start1[3]++;
      start1[4]++;
      start1[5]++;
   }
   // second and third function

//	xor(T, 1, 2, 3, 6, 7, or(4, 5))

   for(int i=0;i<n-n/2;i++) {
		if(i%2) {
			printf("g_%d = OR(v_%d, v_%d)\n", tmpnum+1, start2[3], start2[4]);
			printf("g_%d = XOR(v_%d, v_%d, v_%d, v_%d, g_%d)\n", tmpnum+2, start2[1], start2[2], start2[5], start2[6], tmpnum+1);
			printf("v_%d = NOT(g_%d)\n", start2[0], tmpnum+2);
			tmpnum+=2;
		} else {
			printf("g_%d = OR(v_%d, v_%d)\n", tmpnum+1, start2[3], start2[4]);
			printf("v_%d = XOR(v_%d, v_%d, v_%d, v_%d, g_%d)\n", start2[0], start2[1], start2[2], start2[5], start2[6], tmpnum+1);
			tmpnum+=1;
		}
		
		start2[0]++;
		start2[1]++;
		start2[2]++;
		start2[3]++;
		start2[4]++;
		start2[5]++;
		start2[6]++;
   }

	for(int i=1;i<=n/2;i++) {
		start1[0]--;
		start2[0]--;
		printf("g_%d = XOR(v_%d, v_%d)\n", tmpnum+i, start1[0], start2[0]);
	}
	printf("MITER = OR(g_%d", tmpnum+1);
	for(int i=2;i<=n/2;i++)
	  printf(", g_%d", tmpnum+i);
	printf(")\n");
}

void slider2_unsat_trace(int n)
{
   int s=n/20;
   int start1[6] = {1, 2*s+3, 2*s+1, n/2-1-3*s, n/2-1, n/2};
   int start2[7] = {n, 2*s-1, 2*s+2, n/2-1-4*s, n/2-1-2*s, n/2-1-s, n/2};
	int tmpnum = 0;
	
	printf("MODULE slider2_%d_unsat\nINPUT v_%d", n, (n/2)+1);

   for(int i=(n/2)+2;i<=n;i++)
	  printf(", v_%d", i);
	printf(";\n");

	printf("OUTPUT MITER;\nSTRUCTURE\n");

   // first function
   for(int i=0;i<n/2;i++) {
	   printf("g_%d  = not(v_%d);\n", tmpnum+1, start1[1]);
      printf("g_%d  = not(v_%d);\n", tmpnum+2, start1[2]);
      printf("g_%d  = or(g_%d, g_%d);\n", tmpnum+3, tmpnum+1, tmpnum+2);
      printf("g_%d  = or(v_%d, v_%d);\n", tmpnum+4, start1[1], start1[2]);
      printf("g_%d  = and(g_%d, g_%d);\n", tmpnum+5, tmpnum+3, tmpnum+4);
      printf("g_%d  = not(g_%d);\n", tmpnum+6, tmpnum+5);
      printf("g_%d  = and(v_%d, g_%d);\n", tmpnum+7, start1[4], tmpnum+5);
      printf("g_%d  = not(v_%d);\n", tmpnum+8, start1[4]);
      printf("g_%d  = and(v_%d, g_%d);\n", tmpnum+9, start1[4], tmpnum+6);
      printf("g_%d = and(g_%d, g_%d);\n", tmpnum+10, tmpnum+8, tmpnum+1);
      printf("g_%d = or(g_%d, g_%d);\n", tmpnum+11, tmpnum+9, tmpnum+10);
      printf("g_%d = and(v_%d, g_%d);\n", tmpnum+12, start1[4], tmpnum+5);
      printf("g_%d = and(g_%d, v_%d);\n", tmpnum+13, tmpnum+8, start1[1]);
      printf("g_%d = or(g_%d, g_%d);\n", tmpnum+14, tmpnum+12, tmpnum+13);
      printf("g_%d = and(v_%d, g_%d);\n", tmpnum+15, start1[3], tmpnum+11);
      printf("g_%d = not(v_%d);\n", tmpnum+16, start1[3]);
      printf("g_%d = and(g_%d, g_%d);\n", tmpnum+17, tmpnum+16, tmpnum+14);
      printf("g_%d = or(g_%d, g_%d);\n", tmpnum+18, tmpnum+15, tmpnum+17);
      printf("g_%d = and(v_%d, g_%d);\n", tmpnum+19, start1[5], tmpnum+18);
      printf("g_%d = not(v_%d);\n", tmpnum+20, start1[5]);
      printf("g_%d = and(g_%d, g_%d);\n", tmpnum+21, tmpnum+20, tmpnum+7);
      printf("v_%d = or(g_%d, g_%d);\n", start1[0], tmpnum+19, tmpnum+21);

		tmpnum+=21;
      start1[0]++;
      start1[1]++;
      start1[2]++;
      start1[3]++;
      start1[4]++;
      start1[5]++;
   }
   // second and third function

//	xor(T, 1, 2, 3, 6, 7, or(4, 5))

   for(int i=0;i<n-n/2;i++) {
		if(i%2) {
			printf("g_%d = or(v_%d, v_%d);\n", tmpnum+1, start2[3], start2[4]);
			printf("g_%d = xor(v_%d, v_%d, v_%d, v_%d, g_%d);\n", tmpnum+2, start2[1], start2[2], start2[5], start2[6], tmpnum+1);
			printf("v_%d = not(g_%d);\n", start2[0], tmpnum+2);
			tmpnum+=2;
		} else {
			printf("g_%d = or(v_%d, v_%d);\n", tmpnum+1, start2[3], start2[4]);
			printf("v_%d = xor(v_%d, v_%d, v_%d, v_%d, g_%d);\n", start2[0], start2[1], start2[2], start2[5], start2[6], tmpnum+1);
			tmpnum+=1;
		}
		
		start2[0]++;
		start2[1]++;
		start2[2]++;
		start2[3]++;
		start2[4]++;
		start2[5]++;
		start2[6]++;
   }

	for(int i=1;i<=n/2;i++) {
		start1[0]--;
		start2[0]--;
		printf("g_%d = xor(v_%d, v_%d);\n", tmpnum+i, start1[0], start2[0]);
	}
	printf("MITER = or(g_%d", tmpnum+1);
	for(int i=2;i<=n/2;i++)
	  printf(", g_%d", tmpnum+i);
	printf(");\nfalse_value = new_int_leaf(0);\nare_equal(false_value, MITER);\nENDMODULE\n");
}

void slider2_unsat(char *out_type, int n)
{
   // 80: 1675229 124.090s 
   // {1, 11,  9, 27, 39, 40 }
   // {1,  7, 10, 21, 33, 35, 40}
   //int start1[6] = {1, 17-6, 15-6, 24+3, 33+6, n/2};
   //int start2[7] = {1, 12-5, 16-6, 18+3, 27+6, 29+6, n/2};

   // {1, 11,  9, 27, 39, 40 }
   // {1,  7, 10, 23, 31, 35, 40}
   //int start1[6] = {1, 17-6, 15-6, 24+3, 33+6, n/2};
   //int start2[7] = {1, 12-5, 16-6, 18+5, 27+4, 29+6, n/2};
   //
   //good one for 80 and more unsat
   //int s=n/20;
   //int start1[6] = {1, 2*s+3, 2*s+1, n/2-1-3*s, n/2-1, n/2};
   //int start2[7] = {1, 2*s-1, 2*s+2, n/2-1-4*s, n/2-1-2*s, n/2-1-s, n/2};
   //
    //
    //UNSAT
   int s=n/20;
   int start1[6] = {1, 2*s+3, 2*s+1, n/2-1-3*s, n/2-1, n/2};
   int start2[7] = {1, 2*s-1, 2*s+2, n/2-1-4*s, n/2-1-2*s, n/2-1-s, n/2};
   //UNSAT
   /*
   fprintf(stderr, "{%d, %d, %d, %d, %d, %d}\n", 
         start1[0], start1[1], start1[2], start1[3], start1[4], start1[5]);
   fprintf(stderr, "{%d, %d, %d, %d, %d, %d, %d}\n", 
         start2[0], start2[1], start2[2], start2[3], start2[4], start2[5], start2[6]);
         */
   
   printf("p bdd %d %d\n", n, n);
   printf("; automatically generated UNSAT slider2 with n=%d \n", n);
   printf("; Disclaimer: no formal analysis was done to verify SAT and UNSAT\n");

   // first function
   printf("#define add_state1(1, 2, 3, 4, 5, 6)\n");
   printf("#equ(xor(1, and(-3, 5), nand(6, 4)), ite(2, or(5, -6), -5)))\n");
   for(int i=0;i<n/2;i++) {
      printf("add_state1(%d, %d, %d, %d, %d, %d)\n",
            start1[0], start1[1], start1[2], start1[3], start1[4], start1[5]);
      start1[0]++;
      start1[1]++;
      start1[2]++;
      start1[3]++;
      start1[4]++;
      start1[5]++;
   }
   // second and third function
   printf("#define add_state2(1, 2, 3, 4, 5, 6, 7)\n");
   printf("#equ(6, xor(-1, xor(3, and(-4, 5), 4), equ(7, 2)))\n");
   printf("#define add_state3(1, 2, 3, 4, 5, 6, 7)\n");
   printf("#not(equ(6, xor(-1, xor(3, and(-4, 5), 4), equ(7, 2))))\n");
   for(int i=0;i<n-n/2;i++) {
      printf("add_state%d(%d, %d, %d, %d, %d, %d, %d)\n", (i%2)+2,
            start2[0], start2[1], start2[2], start2[3], start2[4], start2[5], start2[6]);
      start2[0]++;
      start2[1]++;
      start2[2]++;
      start2[3]++;
      start2[4]++;
      start2[5]++;
      start2[6]++;
   }
}
