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

void slider2_unsat(char *out_type, int n);
void slider2_sat(char *out_type, int n);

void slider2(char *out_type, int n, int sat) 
{
   if (sat == 0) slider2_unsat(out_type, n);
   else slider2_sat(out_type, n);
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
