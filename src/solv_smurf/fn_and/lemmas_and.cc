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

#include "sbsat.h"
#include "sbsat_solver.h"
#include "solver.h"

int nTotalBytesForLemmaInferences  = 0;
	
// ------------ special functions lemma construction -------------------------
ITE_INLINE
void
ConstructLemmasForAND(int nFnId)
{
   int nNumRHSVbles = arrSolverFunctions[nFnId].fn_and.rhsVbles.nNumElts;
   int *arrRHSVbles = arrSolverFunctions[nFnId].fn_and.rhsVbles.arrElts;
   int *arrRHSPolarities = arrSolverFunctions[nFnId].fn_and.arrRHSPolarities;
   int nLHSVble = arrSolverFunctions[nFnId].fn_and.nLHSVble;
   LemmaBlock *pFirstBlock;
   LemmaBlock *pLastBlock;
   int nNumBlocks;
   int nNumElts;  // Number of literals in the lemma.

   ///////////////////////////
   // Construct long lemma. //
   ///////////////////////////

   // nLHSVble would be 0 in a PLAINOR constraint.
   // We do not want to put vble 0 in our lemma.
   bool bSkipLHSLit = (nLHSVble == 0 ? true : false);

   // Store lemma length.
   nNumElts
      = (bSkipLHSLit ? nNumRHSVbles : nNumRHSVbles + 1);

   int *arrLits = (int*)ite_calloc(nNumElts, sizeof(int), 9, "arrLits");
   int nLitIndex = 0;

   // Store literals.
   if (!bSkipLHSLit)
   {
      arrLits[nLitIndex++]
         = ((arrSolverFunctions[nFnId].fn_and.nLHSPolarity == BOOL_TRUE) ? nLHSVble : -nLHSVble);
   }

   for (int i = 0; i < nNumRHSVbles; i++)
   {
      arrLits[nLitIndex++]
         = ((arrRHSPolarities[i] == BOOL_TRUE)
               ? -arrRHSVbles[i] : arrRHSVbles[i]);
   }

   EnterIntoLemmaSpace(nNumElts, arrLits,
         false, pFirstBlock, pLastBlock, nNumBlocks);
   arrSolverFunctions[nFnId].fn_and.pLongLemma = pFirstBlock;

   ////////////////////////////////////
   // Construct set of short lemmas. //
   ////////////////////////////////////
   arrSolverFunctions[nFnId].fn_and.arrShortLemmas 
      = (LemmaBlock **)ite_calloc(nNumRHSVbles, sizeof(LemmaBlock*),
            9, "pSpecialFunc->arrShortLemmas");
   nTotalBytesForLemmaInferences  += nNumRHSVbles*sizeof(LemmaBlock*);
   LemmaBlock **arrShortLemmas = arrSolverFunctions[nFnId].fn_and.arrShortLemmas;
   for (int i = 0; i < nNumRHSVbles; i++)
   {
      // Store literals.  Each short lemma has exactly two literals.
      arrLits[0]
         = ((arrSolverFunctions[nFnId].fn_and.nLHSPolarity == BOOL_TRUE) ? -nLHSVble : nLHSVble);
      arrLits[1]
         = ((arrRHSPolarities[i] == BOOL_TRUE)
               ? arrRHSVbles[i] : -arrRHSVbles[i]);

      EnterIntoLemmaSpace(2, arrLits, false,
            pFirstBlock, pLastBlock, nNumBlocks);
      arrShortLemmas[i] = pFirstBlock;
   }

   ite_free((void**)&arrLits);
}
