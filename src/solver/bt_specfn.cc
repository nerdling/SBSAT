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
#include "ite.h"
#include "solver.h"

/* param */
extern int *arrNumRHSUnknowns;

/* trace */
extern SpecialFunc *arrSpecialFuncs;
extern int *arrChangedSpecialFn;

ITE_INLINE int UpdateSpecialFunction_AND(IndexRoleStruct *pIRS);
ITE_INLINE int J_UpdateSpecialFunction_AND(IndexRoleStruct *pIRS);
ITE_INLINE int UpdateSpecialFunction_XOR(IndexRoleStruct *pIRS);
ITE_INLINE int J_UpdateSpecialFunction_XOR(IndexRoleStruct *pIRS);

ITE_INLINE
int
UpdateEachAffectedSpecialFunction (AffectedFuncsStruct *pAFS)
{
  int ret=NO_ERROR;

  // Determine the potentially affected special functions.
  int nNumSpecialFuncsAffected = pAFS->nNumSpecialFuncsAffected;
  IndexRoleStruct *pIRS = pAFS->arrSpecFuncsAffected;

  // Update each affected special function.
  for (int i = 0; i < nNumSpecialFuncsAffected; i++, pIRS++)
    {
      int nSpecFuncIndex = pIRS->nSpecFuncIndex;
      if ((arrChangedSpecialFn[nSpecFuncIndex]&1)==0) continue;

      assert(arrChangedSpecialFn[nSpecFuncIndex]==3);
      arrChangedSpecialFn[nSpecFuncIndex]=2;

      int nNumRHSUnknowns = arrNumRHSUnknowns[nSpecFuncIndex];
      if (nNumRHSUnknowns <= 0) continue;


	  // Check whether constraint has already been satisfied.
	  d9_printf2("Visiting special function %d ", nSpecFuncIndex);
	 // DisplaySpecialFunc(arrSpecialFuncs + nSpecFuncIndex, arrSolution);
	  d9_printf2("#RHS Unknowns: %d ", arrNumRHSUnknowns[nSpecFuncIndex]);

     switch (arrSpecialFuncs[nSpecFuncIndex].nFunctionType) {
      case AND: ret=UpdateSpecialFunction_AND(pIRS); break;
      case XOR: ret=UpdateSpecialFunction_XOR(pIRS); break;
      default: assert(0); break;
     }

     if (nHeuristic != JOHNSON_HEURISTIC) 
     {
        arrPrevNumRHSUnknowns[nSpecFuncIndex] = arrNumRHSUnknowns[nSpecFuncIndex];
        arrPrevNumLHSUnknowns[nSpecFuncIndex] = arrNumLHSUnknowns[nSpecFuncIndex];
        arrPrevSumRHSUnknowns[nSpecFuncIndex] = arrSumRHSUnknowns[nSpecFuncIndex];
     }
     if (ret != NO_ERROR) break;

    } // for each affected special function
 return ret;
}
