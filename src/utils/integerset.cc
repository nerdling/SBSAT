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
/*********************************************************************
 *  integerset.cc (J. Ward) 1/3/2001
 *********************************************************************/

#include "ite.h"
#include "integerset.h"

void Display_ISAB(IntegerSet_ArrayBased &iset)
{
  for (int i = 0; i < iset.nNumElts; i++)
    {
      printf(" %d", iset.arrElts[i]);
    }
}

IntegerSet::IntegerSet()
{
  pList = NULL;
}

IntegerSet::~IntegerSet()
{
  return; 
  //IntNode *pToBeDeleted;

  //while (pList)
  //  {
  //    pToBeDeleted = pList;
  //    pList = pList->pNext;
  //    free(pToBeDeleted);
  //  }
}

IntNode *
AllocateIntNode(int n, IntNode *next);

void
IntegerSet::PushElement(int n)
  // Adds n to the IntegerSet.
  // For now, we assume that elts are entered in
  // decreasing order and stored in increasing order.
  // NEW
  // increasing order and stored in increasing order.
  // NEW
  // Later, it might be useful to drop this assumption,
  // in which case this routine will need to be expanded slightly.
{
  assert(pList==NULL || n > pList->nData);
  pList = AllocateIntNode(n, pList);
}

void
IntegerSet::StoreAsArrayBasedSet(IntegerSet_ArrayBased &iset_arr, int *ptr)
{
  int nNumElts = 0;
  IntNode *pNode = pList;
  while (pNode)
    {
      nNumElts++;
      pNode = pNode->pNext;
    }

  iset_arr.nNumElts = nNumElts;
  if (nNumElts > 0)
    {
       if (ptr != NULL) iset_arr.arrElts = ptr;
       else iset_arr.arrElts = (int *)calloc(nNumElts, sizeof(int));
    }
  else
    {
      iset_arr.arrElts = 0;
    }

  int i = 0;
  int *arr = iset_arr.arrElts;
  pNode = pList;
  while (pNode)
    {
      arr[i] = pNode->nData;
      pNode = pNode->pNext;
      i++;
    }
}

void
IntegerSet::StoreAsArrayBasedSet_OmitElt(IntegerSet_ArrayBased &iset_arr,
					 int nOmit)
{
  int nNumElts = 0;
  IntNode *pNode = pList;
  while (pNode)
    {
      if (pNode->nData != nOmit)
	{
	  nNumElts++;
	}
      pNode = pNode->pNext;
    }

  iset_arr.nNumElts = nNumElts;
  if (nNumElts > 0)
    {
      iset_arr.arrElts = (int *)calloc(nNumElts, sizeof(int));
    }
  else
    {
      iset_arr.arrElts = 0;
    }

  int i = 0;
  int *arr = iset_arr.arrElts;
  pNode = pList;
  while (pNode)
    {
      if (pNode->nData != nOmit)
	{
	  arr[i] = pNode->nData;
	  i++;
	}
      pNode = pNode->pNext;
    }
}

void
IntegerSet::Display()
{
  IntNode *pNode = pList;
  while (pNode)
    {
      printf(" %d", pNode->nData);
      pNode = pNode->pNext;
    }
}

int
IntegerSet::Size()
{
  int nCount = 0;
  IntNode *pNode = pList;
  while (pNode)
    {
      nCount++;
      pNode = pNode->pNext;
    }
  return nCount;
}

IntegerSetIterator::IntegerSetIterator(IntegerSet &intset)
{
  pCurrent = intset.pList;
}

bool
IntegerSetIterator::operator ()(int &n)
{
  if (!pCurrent)
    {
      return false;
    }

  n = pCurrent->nData;
  pCurrent = pCurrent->pNext;  
  return true;
}

void AppendElt(IntNode *&pResult, IntNode *&pCurrentResultNode, int nData)
{
  if (!pResult)
    {
      pCurrentResultNode = pResult = AllocateIntNode(nData, NULL);
    }
  else
    {
      pCurrentResultNode->pNext = AllocateIntNode(nData, NULL);
      pCurrentResultNode = pCurrentResultNode->pNext;
    }
  //pCurrentResultNode->nData = nData;
  //pCurrentResultNode->pNext = 0;
}

IntNode *
IntersectLists(IntNode *pList1, IntNode *pList2)
  // Utilizes the assumption that the two lists store
  // their elements in ascending order.
{
  IntNode *pResult = 0;
  IntNode *pCurrentResultNode;

  while (pList1 && pList2)
    {
      // Compare elements.
      if (pList1->nData > pList2->nData)
	{
	  pList1 = pList1->pNext;
	}
      else if (pList1->nData < pList2->nData)
	{
	  pList2 = pList2->pNext;
	}
      else
	{
	  // Elements match.
	  AppendElt(pResult, pCurrentResultNode, pList1->nData);
	  pList1 = pList1->pNext;
	  pList2 = pList2->pNext;
	}
    }

  return pResult;
}


IntNode *
UnionLists(IntNode *pList1, IntNode *pList2)
  // Utilizes the assumption that the two lists store
  // their elements in ascending order.
{
  IntNode *pResult = 0;
  IntNode *pCurrentResultNode;

  if (!pList1 && !pList2)
    {
      return 0;
    }

  if (!pList1)
    {
      return pList2;
    }

  if (!pList2)
    {
      return pList1;
    }

  while (pList1 && pList2)
    {
      // Compare elements.
      if (pList1->nData > pList2->nData)
	{
	  AppendElt(pResult, pCurrentResultNode, pList1->nData);
	  pList1 = pList1->pNext;
	}
      else if (pList1->nData < pList2->nData)
	{
	  AppendElt(pResult, pCurrentResultNode, pList2->nData);
	  pList2 = pList2->pNext;
	}
      else
	{
	  // Elements match.
	  AppendElt(pResult, pCurrentResultNode, pList1->nData);
	  pList1 = pList1->pNext;
	  pList2 = pList2->pNext;
	}
    }

  if (pList1)
    {
      pCurrentResultNode->pNext = pList1;
    }
  else
    {
      pCurrentResultNode->pNext = pList2;
    }

  return pResult;
}

IntNode *
ListDifference(IntNode *pList1, IntNode *pList2)
  // Utilizes the assumption that the two lists store
  // their elements in ascending order.
  // Returns a list of the elements that are in list1 but not in list2.
{
  IntNode *pResult = 0;
  IntNode *pCurrentResultNode;

  if (!pList1)
    {
      return 0;
    }

  if (!pList2)
    {
      return pList1;
    }

  while (pList1 && pList2)
    {
      // Compare elements.
      if (pList1->nData > pList2->nData)
	{
	  AppendElt(pResult, pCurrentResultNode, pList1->nData);
	  pList1 = pList1->pNext;
	}
      else if (pList1->nData < pList2->nData)
	{
	  pList2 = pList2->pNext;
	}
      else
	{
	  // Elements match.
	  pList1 = pList1->pNext;
	  pList2 = pList2->pNext;
	}
    }

  if (pResult)
    {
      pCurrentResultNode->pNext = pList1;
    }

  return pResult;
}


void ComputeIntersection(IntegerSet &iset1,
			 IntegerSet &iset2,
			 IntegerSet &isetIntersection)
{
  isetIntersection.pList = IntersectLists(iset1.pList, iset2.pList);
}

void ComputeUnion(IntegerSet &iset1,
			 IntegerSet &iset2,
			 IntegerSet &isetIntersection)
{
  isetIntersection.pList = UnionLists(iset1.pList, iset2.pList);
}

void ComputeDifference(IntegerSet &iset1,
		       IntegerSet &iset2,
		       IntegerSet &isetDifference)
{
  isetDifference.pList = ListDifference(iset1.pList, iset2.pList);
}
