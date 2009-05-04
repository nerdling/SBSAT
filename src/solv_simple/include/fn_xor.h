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

#ifndef FN_XOR_S_H
#define FN_XOR_S_H

// XOR State

void initXORStateType();

void PrintXORStateEntry(void *pState);
void PrintXORStateEntry_dot(void *pState);
void PrintXORStateEntry_formatted(void *pState);

void LSGBXORStateSetHeurScore(void *pState);
double LSGBXORStateGetHeurScore(void *pState);
double LSGBXORGetHeurScoreTrans(XORStateEntry *pState);
void LSGBXORFree();

void CalculateXORLSGBHeuristic(void *pState, int nCurrInfLevel);

void *CreateXORState(int *arrElts, int nNumElts, BDDNode *pCurrentBDD, XORStateEntry *pStartState);

void SetVisitedXORState(void *pState, int value);
int ApplyInferenceToXOR(int nBranchVar, bool bBVPolarity, int nSmurfNumber, void **arrSmurfStates);

void FreeXORStateEntry(void *pState);

// XOR Counter State

void initXORCounterStateType();

void PrintXORCounterStateEntry(void *pState);
void PrintXORCounterStateEntry_dot(void *pState);

void LSGBXORCounterStateSetHeurScore(void *pState);
double LSGBXORCounterStateGetHeurScore(void *pState);
double LSGBXORCounterGetHeurScoreTrans(XORCounterStateEntry *pState);

void CalculateXORCounterLSGBHeuristic(void *pState, int nCurrInfLevel);

void SetVisitedXORCounterState(void *pState, int value);
int ApplyInferenceToXORCounter(int nBranchVar, bool bBVPolarity, int nSmurfNumber, void **arrSmurfStates);

void FreeXORCounterStateEntry(void *pState);

// Gaussian Elimination State

void initXORGElimStateType();

void PrintXORGElimStateEntry(void *pState);
void PrintXORGElimStateEntry_dot(void *pState);
void PrintXORGElimVector(void *pVector);

double LSGBarrXORWeight(void *pState);
double LSGBarrXORWeightTrans(int nSize);

void CalculateXORGElimLSGBHeuristic(void *pState, int nCurrInfLevel);

void *CreateXORGElimState(int *arrElts, int nNumElts, BDDNode *pCurrentBDD, XORGElimStateEntry *pStartState);

void SetVisitedXORGElimState(void *pState, int value);

void FreeXORGElimStateEntry(void *pState);

#endif
