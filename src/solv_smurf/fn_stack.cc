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

#define MAX_SPECIAL_FN_STACK_POOL 100
#define FN_STACK_ALLOC_MULT 6 /* >= 2 */

int nCurFnVersion=1;
int nFnStackIdx=0;
int nMaxFnStackIdx=0;
FnStack *arrFnStack=NULL;
int *arrFnFlags=NULL;
typedef struct _tFnStackPool {
   FnStack *stack; 
   int max; 
   _tFnStackPool *next;
} tFnStackPool;
tFnStackPool *arrFnStackPool=NULL;
tFnStackPool *arrFnStackPoolHead=NULL;

ITE_INLINE void AllocateFnStack(int newsize);
ITE_INLINE void FnStackMark(int vx);
ITE_INLINE void FnStackNext();
ITE_INLINE void FnStackInit();

ITE_INLINE
int
pop_mark_state_information()
{
   d9_printf1("pop_mark_state_information()\n");

   /* Pop the special function information. */
   assert(nFnStackIdx > 0);

   /* until LEVEL_START or LEVEL_MARK */
   while (arrFnStack[nFnStackIdx].nFnId != LEVEL_START &&
         arrFnStack[nFnStackIdx].nFnId != LEVEL_MARK) {
      int fn=arrFnStack[nFnStackIdx].nFnId;

      if (fn == POOL_START) {
         nFnStackIdx--;
         FnStack *new_arrFnStack = 
            (FnStack*)(arrFnStack[nFnStackIdx--].fn_next_pool.next_pool);
         nFnStackIdx = arrFnStack[nFnStackIdx].fn_next_pool.index_pool;
         assert(new_arrFnStack[nFnStackIdx].nFnId==POOL_END);
         arrFnStack = new_arrFnStack;
      } else {
         if (fn >= 0)  {

            procRestoreFromStack[arrFnStack[nFnStackIdx].nType](&arrFnStack[nFnStackIdx]);
            arrChangedFn[fn] = 0;
            arrFnFlags[fn] = arrFnStack[nFnStackIdx].prev_version; 
            d9_printf2("pop function (%d)\n", fn);
         }
      }
      nFnStackIdx--;
   }

   /* keep the mark on the top of the stack */
   assert(nFnStackIdx>0);

   if (arrFnStack[nFnStackIdx].nFnId == LEVEL_MARK)  {
      d9_printf1("pop_mark_state_information() mark found\n");
      return 1;
   } else {
      d9_printf1("pop_mark_state_information() mark not found\n");
      return 0; 
   }
}


ITE_INLINE
void
pop_state_information(int n)
{
   d9_printf2("pop_state_information(%d)\n", n);

   for (int i=0;i<n;i++)
   {
      d9_printf2("pass %d\n", i);
      /* Pop the special function information. */
      assert(nFnStackIdx > 0);

      /* until LEVEL_START */
      while (arrFnStack[nFnStackIdx].nFnId != LEVEL_START) {
         int fn=arrFnStack[nFnStackIdx].nFnId;

         if (fn == POOL_START) {
            nFnStackIdx--;
            FnStack *new_arrFnStack = 
               (FnStack*)(arrFnStack[nFnStackIdx--].fn_next_pool.next_pool);
            assert(new_arrFnStack[/*nFnStackIdx=*/arrFnStack[nFnStackIdx].fn_next_pool.index_pool].nFnId==POOL_END);
            nFnStackIdx = arrFnStack[nFnStackIdx].fn_next_pool.index_pool;
            arrFnStack = new_arrFnStack;
         } else
            if (fn >= 0) {
               procRestoreFromStack[arrFnStack[nFnStackIdx].nType](&arrFnStack[nFnStackIdx]);
               arrChangedFn[fn] = 0;
               arrFnFlags[fn] = arrFnStack[nFnStackIdx].prev_version; 
               d9_printf2("pop function (%d)\n", fn);
               /*
               d9_printf6("pop_state_information: specfn(%d), #RHSUnknowns: %d -> %d, #LHSUnknowns: %d -> %d\n", fn, 
                     arrNumRHSUnknowns[fn], arrFnStack[nFnStackIdx].u.value,
                     arrNumLHSUnknowns[fn], arrFnStack[nFnStackIdx].lhsvalue);

               arrNumRHSUnknownsNew[fn]=
               arrPrevNumRHSUnknowns[fn]=
               arrNumRHSUnknowns[fn]=arrFnStack[nFnStackIdx].u.value;

               arrNumLHSUnknownsNew[fn]=
               arrPrevNumLHSUnknowns[fn]=
               arrNumLHSUnknowns[fn]=arrFnStack[nFnStackIdx].lhsvalue;

               arrSumRHSUnknownsNew[fn]=
               arrPrevSumRHSUnknowns[fn]=
               arrSumRHSUnknowns[fn]=arrFnStack[nFnStackIdx].rhssum;

               arrRHSCounterNew[fn]=
               arrPrevRHSCounter[fn]=
               arrRHSCounter[fn]=arrFnStack[nFnStackIdx].rhscounter;

               arrFnFlags[fn]=arrFnStack[nFnStackIdx].prev;
               */
            }
         nFnStackIdx--;
      }
      d9_printf1("Done\n");

      nFnStackIdx--; /* skip the LEVEL_START */

      assert(nFnStackIdx>0);
      //assert(nCurFnVersion == arrFnStack[nFnStackIdx+1].u.version);
      //nCurFnVersion--;
      
      nCurFnVersion = arrFnStack[nFnStackIdx+1].prev_version;
   }
}

ITE_INLINE void
FnStackPushFunctionLevel()
{
   d9_printf1("FnStackPushFunctionLevel\n");
   nCurFnVersion++;
   FnStackMark(LEVEL_START);
}

ITE_INLINE void
FnStackMark(int mark) 
{ 
   nFnStackIdx++; 
   if (arrFnStack[nFnStackIdx].nFnId == POOL_END)
   {
      FnStackNext();
      nFnStackIdx++; 
   }
   arrFnStack[nFnStackIdx].nFnId = mark;  
   arrFnStack[nFnStackIdx].prev_version = nCurFnVersion;  
}


ITE_INLINE void
FnStackPush(int nFnId, int nType) 
{ 
   d9_printf3("FnStackPush function %d type %d\n", nFnId, nType);

   nFnStackIdx++; 
   if (arrFnStack[nFnStackIdx].nFnId == POOL_END)
   {
      FnStackNext();
      nFnStackIdx++; 
   }
   arrFnStack[nFnStackIdx].nFnId  = nFnId;  
   arrFnStack[nFnStackIdx].nType  = nType;  
   arrFnStack[nFnStackIdx].prev_version = arrFnFlags[nFnId];
   arrFnFlags[nFnId]              = nCurFnVersion; 
  
   procSave2Stack[arrFnStack[nFnStackIdx].nType](nFnId, &arrFnStack[nFnStackIdx]);
}

ITE_INLINE void
FnStackInit()
{
   if (arrFnFlags == NULL)
      arrFnFlags = (int*)ite_calloc(nNumFuncs+1, sizeof(int),
            9, "arrFnFlags");
   if (arrFnStackPool == NULL)
      AllocateFnStack((nNumFuncs+1)*FN_STACK_ALLOC_MULT);
}

ITE_INLINE void
FnStackNext()
{
   assert(arrFnStack[nFnStackIdx].nFnId == POOL_END);
   nFnStackIdx++;

   if (arrFnStack[nFnStackIdx].fn_next_pool.next_pool == NULL) 
   {
      nFnStackIdx--;
      AllocateFnStack((nNumFuncs+1)*FN_STACK_ALLOC_MULT);
   } 
   else 
   {
      arrFnStack = (FnStack*)arrFnStack[nFnStackIdx].fn_next_pool.next_pool;
      nFnStackIdx = 2;
   }
}

ITE_INLINE void
AllocateFnStack(int newsize)
{
   int prevsize;
   assert(newsize > 0);
   newsize += 10000; /* it does not hurt to allocate a little bit more
                      * for backtracking stack */

   if (arrFnStackPool) prevsize = arrFnStackPool->max;
   arrFnStackPool = (tFnStackPool*)ite_calloc(1, sizeof (tFnStackPool),
         9, "arrFnStackPool");

   arrFnStackPool->max = newsize;
   arrFnStackPool->stack =
      (FnStack*)ite_calloc(newsize, sizeof(FnStack), 
                                   2, "special function stack");

   FnStack* prev_arrFnStack = arrFnStack;
   arrFnStack = arrFnStackPool->stack;
   arrFnStackPool->next = arrFnStackPoolHead;
   arrFnStackPoolHead = arrFnStackPool;

   /* save the prev index*/
   arrFnStack[0].fn_next_pool.index_pool = nFnStackIdx; 
   arrFnStack[1].fn_next_pool.next_pool  = prev_arrFnStack;
   arrFnStack[2].nFnId = POOL_START;
   arrFnStack[newsize-2].nFnId = POOL_END;
   arrFnStack[newsize-1].fn_next_pool.next_pool  = NULL;
   if (prev_arrFnStack != NULL) {
      prev_arrFnStack[prevsize-1].fn_next_pool.next_pool = arrFnStack;
   }

   nFnStackIdx      = 2;
}

ITE_INLINE void
FnStackFree()
{
   while(arrFnStackPoolHead) {
      free(arrFnStackPoolHead->stack);
      tFnStackPool *x = arrFnStackPoolHead;
      arrFnStackPoolHead = arrFnStackPoolHead->next;
      free(x);
   }

   arrFnStackPool = NULL;
   arrFnStack = NULL;
   ite_free((void**)&arrFnFlags);
}
