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
// literalset.cc
// Routines for manipulating sets of literals.
// Started 1/3/2001 - J. Ward

#include "ite.h"
#include "literalset.h"

LiteralSet::LiteralSet()
{
  nFlag = 0;
}

void
LiteralSet::PushElement(int nVbleIndex, bool bArity)
{
  if (nFlag == 1)
    {
      return;
    }
 
  nFlag = 2;

  if (bArity)
    {
      PosElts.PushElement(nVbleIndex);
    }
  else
    {
      NegElts.PushElement(nVbleIndex);
    }

}

void
LiteralSet::Display()
{
  printf(" PosElts: ");
  PosElts.Display();
  printf(" NegElts: ");
  NegElts.Display();
}

int
LiteralSet::Size()
{
  return PosElts.Size() + NegElts.Size();
}

LiteralSetIterator::LiteralSetIterator(LiteralSet &litset) :
  nextPos(litset.PosElts), nextNeg(litset.NegElts)
{
  nFlag = litset.nFlag;
  bStoredPos = bStoredNeg = bPosExhausted = bNegExhausted = false;
}

bool
LiteralSetIterator::operator () (int &n, bool &bArity)
  // Iterate through the literal set in ascending order
  // based on the indicies of the variables mentioned in
  // the literals.
{
  if (nFlag < 2)
    {
      return false;
    }

  if (!bPosExhausted && !bStoredPos)
    {
      bStoredPos = nextPos(nPosVble);
      bPosExhausted = !bStoredPos;
    }

  if (!bNegExhausted && !bStoredNeg)
    {
      bStoredNeg = nextNeg(nNegVble);
      bNegExhausted = !bStoredNeg;
    }

  if (bStoredPos && bStoredNeg)
    {
      if (nPosVble < nNegVble)
	{
	  n = nPosVble;
	  bStoredPos = false;
	  bArity = true;
	}
      else
	{
	  n = nNegVble;
	  bStoredNeg = false;
	  bArity = false;
	}
      return true;
    }
  else if (bStoredPos)
    {
      n = nPosVble;
      bStoredPos = false;
      bArity = true;
      return true;
    }
  else if (bStoredNeg)
    {
      n = nNegVble;
      bStoredNeg = false;
      bArity = false;
      return true;
    }
  else
    {
      return false;
    }
}


void
ComputeIntersection(LiteralSet &ls1,
		    LiteralSet &ls2,
		    LiteralSet &lsIntersection)
  // Precondition:  lsIntersection is empty.
{
  assert(lsIntersection.IsEmpty());

  if (ls1.IsEmpty() || ls2.IsEmpty())
    {
      // lsIntersection should stay empty.
      return;
    }

  if (ls1.IsInconsistent())
    {
      lsIntersection = ls2;  // NOTE:  structure sharing
      return;
    }

  if (ls2.IsInconsistent())
    {
      lsIntersection = ls1;  // NOTE:  structure sharing
      return;
    }

  ComputeIntersection(ls1.PosElts, ls2.PosElts, lsIntersection.PosElts);
  ComputeIntersection(ls1.NegElts, ls2.NegElts, lsIntersection.NegElts);
  if (!lsIntersection.PosElts.IsEmpty() || !lsIntersection.NegElts.IsEmpty())
    {
      lsIntersection.nFlag = 2;
    }
}

