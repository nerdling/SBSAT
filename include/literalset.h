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
// literalset.h
// Started 1/3/2001 - J. Ward

#ifndef LITERALSET_H
#define LITERALSET_H

#include "integerset.h"

class LiteralSet
{
 public:
  LiteralSet();
  // ~LiteralSet();  Using default destructor.
  void PushElement(int nVbleIndex, bool bArity);
  IntegerSet *GetPtrPositiveInferences() { return &PosElts; }
  IntegerSet *GetPtrNegativeInferences() { return &NegElts; }
  bool IsInconsistent() { return nFlag == 1; }
  bool IsEmpty() { return nFlag == 0; }
  void SetToInconsistent() { nFlag = 1; }
  void StoreAsArrayBasedSets(IntegerSet_ArrayBased &s1,
			     IntegerSet_ArrayBased &s2);
  //void SetToEmpty() { nFlag = 0; }
  void Display();
  int Size();

  friend class LiteralSetIterator;

  friend void ComputeIntersection(LiteralSet &ls1,
			 LiteralSet &ls2,
			 LiteralSet &lsIntersection);

 private:
  int nFlag; // 0: emptyset
  // 1: inconsistent set
  // 2: neither of the above cases --
  // check following members for included
  // elements.

  IntegerSet PosElts;
  IntegerSet NegElts;
};

class LiteralSetIterator
{
 public:
  LiteralSetIterator(LiteralSet &litset);
  bool operator ()(int &n, bool &bArity);
 private:
  int nFlag; // Flag copied from the LiteralSet.
  IntegerSetIterator nextPos;
  IntegerSetIterator nextNeg;
  bool bStoredPos;
  bool bStoredNeg;
  int nPosVble;
  int nNegVble;
  bool bPosExhausted;
  bool bNegExhausted;
};

void ComputeIntersection(LiteralSet &ls1,
			 LiteralSet &ls2,
			 LiteralSet &lsIntersection);

#endif
