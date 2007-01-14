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

#ifndef FN_MINMAX_H
#define FN_MINMAX_H

// Function structure
typedef struct {
   int nNumRHSUnknowns;
   int nNumRHSUnknownsNew;
   int nNumRHSUnknownsPrev;

   int min;
   int max;

   IntegerSet_ArrayBased rhsVbles;
   int *arrRHSPolarities;

   int nRHSCounter;
   int nRHSCounterNew;
   int nRHSCounterPrev;
} Function_MinMax;

// AFS structure
typedef struct {
} AFS_MINMAX;

// Stack structure
typedef struct {
   int rhs_unknowns;
   int rhs_counter;
} Stack_MinMax;

extern int arrFnMinMaxTypes[];

int MinMaxCreateFunction(int nFnId, BDDNode *bdd, int nFnType, int eqVble);
void MinMaxAffectedVarList(int nFnId, int **arr1, int *num1, int **arr2, int *num2);
void MinMaxCreateAFS(int nFnId, int nVarId, int nAFSIndex);
int MinMaxUpdateAffectedFunction(int nFnId);
int MinMaxUpdateAffectedFunction_Infer(void *oneafs, int x);
int MinMaxSave2Stack(int nFnId, void *one_stack);
int MinMaxRestoreFromStack(void *one_stack);
void MinMaxUpdateFunctionInfEnd(int nFnId);
void LSGBMinMaxUpdateFunctionInfEnd(int nFnId);
void LSGBMinMaxGetHeurScores(int nFnId);
int FnMinMaxInit();
void HrLSGBFnMinMaxInit();
void HrLSGBWFnMinMaxInit();

#endif
