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

#ifndef BRANCHER_H
#define BRANCHER_H

#include "smurffactory.h"

#include "common.h"
#include "lemmainfo.h"

#define LEMMA_CONSTANT 10.0

void DisplayStatistics(int nNumChoicePts, int nNumBacktracks, int nNumBackjumps);


struct ChoicePointStruct
{
  int nBranchVble; // Index of branch variable
  int nNumUnresolved; // # of not-yet-satisfied functions
  // present in the problem before the branch variable was selected
};

struct IndexRoleStruct
{
  int nSpecFuncIndex;  // Index of a special function which mentions the
                      // current variable.
  int nPolarity; // Does it appear in a positive or negative literal?
  int nRHSIndex; // If it is mentioned in a RHS literal, what is the index
  // of that literal relative to the rest of the RHS.  E.g., is it
  // the 3rd literal in the RHS?  (In that case nRHSIndex would be 2 because
  // we start counting from 0.)
  // LHS literal has -1
};

//struct AffectedLemmasStruct 
//{
//  int nLemmaIndex;
//  AffectedLemmasStruct *pNext;
//};

struct AffectedFuncsStruct
// Structure which gives a list of the functions (constraints) mentioning
// a particular variable, and the role of the variable in each of those
// functions.
{
  int nNumRegSmurfsAffected; // Number of regular Smurfs mentioning the vble.
  int *arrRegSmurfsAffected;
  int nNumSpecialFuncsAffected; // Number of special funcs mentioning the vble.
  IndexRoleStruct *arrSpecFuncsAffected;
  // The following members are used to implement the Chaff-style lemmas.
  // For instance, pLemmasWherePos1 points to the list of lemmas
  // where the current variable is mentioned positively and where
  // the variable is the first of the two literals in the lemma
  // which are being watched by the brancher.
  LemmaInfoStruct LemmasWherePos[2];
  // pLemmasWherePos2 is the same thing, except that in each of these lemmas
  // the variable is the _second_ of the two literals which are being watched
  // by the brancher.
  LemmaInfoStruct LemmasWhereNeg[2];
};

struct BacktrackStackEntry
{
  int nBranchVble;
  bool bWasChoicePoint;
  LemmaBlock *pLemma;
  // pLemmaInfo will be NULL unless the lemma for this entry
  // was fired from the lemma cache when the inference was made.
  LemmaInfoStruct *pLemmaInfo;
  LemmaInfoStruct *pUnitLemmaList;
  LemmaInfoStruct *pUnitLemmaListTail;
};

int
Brancher();

void
DisplaySolution(int nMaxVbleIndex);

int
ConsistentPartialSolution(int nMaxVbleIndex);

AffectedFuncsStruct *
CreateAffectedFuncsStructures(int nMaxVbleIndex);

typedef struct _t_solution_info {
  int *arrElts;
  int nNumElts;
  struct _t_solution_info *next;
} t_solution_info;

#ifdef HAVE_IMAGE_JPEG
ITE_INLINE void
graph_record(double progress);

ITE_INLINE void
graph_generate(char *filename);
#endif

#endif // BRANCHER_H
