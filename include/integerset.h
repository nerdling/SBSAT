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
/************************************************************
 *  integerset.h (J. Ward) 1/3/2001
 ************************************************************/

#ifndef INTEGERSET_H
#define INTEGERSET_H

struct IntegerSet_ArrayBased
{
  int nNumElts;
  int *arrElts;
};

void Display_ISAB(IntegerSet_ArrayBased &iset);

struct IntNode
{
  int nData;
  IntNode *pNext;
};

class IntegerSet
{
 public:
  IntegerSet();
  ~IntegerSet();
  void PushElement(int n);
  bool IsEmpty() { return pList == 0; }
  void StoreAsArrayBasedSet(IntegerSet_ArrayBased &iset_arr, int *ptr);
  void StoreAsArrayBasedSet_Append(IntegerSet_ArrayBased &iset_arr);
  void StoreAsArrayBasedSet_OmitElt(IntegerSet_ArrayBased &iset_arr,
				    int nOmit);
  void Display();
  int Size();

  friend void ComputeIntersection(IntegerSet &iset1,
				  IntegerSet &iset2,
				  IntegerSet &isetIntersection);
  friend void ComputeUnion(IntegerSet &iset1,
			   IntegerSet &iset2,
			   IntegerSet &isetUnion);
  friend void ComputeDifference(IntegerSet &iset1,
				IntegerSet &iset2,
				IntegerSet &isetDifference);
  friend class IntegerSetIterator;

 private:
  IntNode *pList; // A singly-linked list, storing the elements
  // in ascending order.
};

class IntegerSetIterator
{
 public:
  IntegerSetIterator(IntegerSet &intset);
  bool operator ()(int &n);
 private:
  IntNode *pCurrent;
};

void ComputeIntersection(IntegerSet &iset1,
			 IntegerSet &iset2,
			 IntegerSet &isetIntersection);

void ComputeUnion(IntegerSet &iset1,
		  IntegerSet &iset2,
		  IntegerSet &isetUnion);

void ComputeDifference(IntegerSet &iset1,
		       IntegerSet &iset2,
		       IntegerSet &isetDifference);



#endif
