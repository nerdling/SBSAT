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

/* pop the information */

#define MAX_SMURF_STATES_STACK_POOL 100
int nCurSmurfStatesVersion=1;
int nSmurfStatesStackIdx=0;
tSmurfStatesStack *arrSmurfStatesStack=NULL;
int *arrSmurfStatesFlags=NULL;
typedef struct _tSmurfStatesStackPool { 
   tSmurfStatesStack *stack; 
   int max; 
   _tSmurfStatesStackPool *next;
} tSmurfStatesStackPool;
tSmurfStatesStackPool *arrSmurfStatesStackPool=NULL;
tSmurfStatesStackPool *arrSmurfStatesStackPoolHead=NULL;

#define MAX_SPECIAL_FN_STACK_POOL 100
int nCurSpecialFnVersion=1;
int nSpecialFnStackIdx=0;
int nMaxSpecialFnStackIdx=0;
tSpecialFnStack *arrSpecialFnStack=NULL;
int *arrSpecialFnFlags=NULL;
typedef struct _tSpecialFnStackPool {
   tSpecialFnStack *stack; 
   int max; 
   _tSpecialFnStackPool *next;
} tSpecialFnStackPool;
tSpecialFnStackPool *arrSpecialFnStackPool=NULL;
tSpecialFnStackPool *arrSpecialFnStackPoolHead=NULL;

ITE_INLINE void Mark_arrSmurfStatesStack(int vx);
ITE_INLINE void Mark_arrNumRHSUnknowns(int vx);
ITE_INLINE void NextSpecialFnStack();
ITE_INLINE void NextSmurfStatesStack();
ITE_INLINE void InitializeSmurfStatesStack();
ITE_INLINE void InitializeSpecialFnStack();

#define LEVEL_START -1
#define POOL_START -2
#define POOL_END -3
#define LEVEL_MARK -4

ITE_INLINE
int
pop_mark_state_information()
{
   d9_printf1("pop_mark_state_information()\n");

   /* pop smurf state stack */
   assert(nSmurfStatesStackIdx>0);

   /* until LEVEL_START or LEVEL_MARK*/
   while (arrSmurfStatesStack[nSmurfStatesStackIdx].smurf != LEVEL_START &&
         arrSmurfStatesStack[nSmurfStatesStackIdx].smurf != LEVEL_MARK) {
      int smurf=arrSmurfStatesStack[nSmurfStatesStackIdx].smurf;

      if (smurf == POOL_START) 
      {
         nSmurfStatesStackIdx--;
         tSmurfStatesStack *new_arrSmurfStatesStack = (tSmurfStatesStack*)
            (arrSmurfStatesStack[nSmurfStatesStackIdx--].u.next_pool);
         nSmurfStatesStackIdx = 
            arrSmurfStatesStack[nSmurfStatesStackIdx].u.index_pool;
         assert(new_arrSmurfStatesStack[nSmurfStatesStackIdx].smurf==POOL_END);
         arrSmurfStatesStack = new_arrSmurfStatesStack;
      } else {
         if (smurf >= 0) 
         {
            d9_printf2("pop_mark_state_information: smurf(%d)\n", smurf);

            arrPrevStates[smurf]=
            arrCurrentStates[smurf]=
               arrSmurfStatesStack[nSmurfStatesStackIdx].u.state;
            arrSmurfPath[smurf].idx =
               arrSmurfStatesStack[nSmurfStatesStackIdx].path_idx;
            arrSmurfStatesFlags[smurf]=
               arrSmurfStatesStack[nSmurfStatesStackIdx].prev;
            arrChangedSmurfs[smurf]=0;
         }
      }
      nSmurfStatesStackIdx--;
   }
   /* keep the mark on the top of the stack */
   assert(nSmurfStatesStackIdx>0);

   /* Pop the special function information. */
   assert(nSpecialFnStackIdx > 0);

   /* until LEVEL_START or LEVEL_MARK */
   while (arrSpecialFnStack[nSpecialFnStackIdx].fn != LEVEL_START &&
         arrSpecialFnStack[nSpecialFnStackIdx].fn != LEVEL_MARK) {
      int fn=arrSpecialFnStack[nSpecialFnStackIdx].fn;

      if (fn == POOL_START) {
         nSpecialFnStackIdx--;
         tSpecialFnStack *new_arrSpecialFnStack = 
            (tSpecialFnStack*)(arrSpecialFnStack[nSpecialFnStackIdx--].u.next_pool);
         nSpecialFnStackIdx = arrSpecialFnStack[nSpecialFnStackIdx].u.index_pool;
         assert(new_arrSpecialFnStack[nSpecialFnStackIdx].fn==POOL_END);
         arrSpecialFnStack = new_arrSpecialFnStack;
      } else {
         if (fn >= 0) 
         {
            d9_printf6("pop_mark_state_information: specfn(%d), #RHSUnknowns: %d -> %d, #LHSUnknowns: %d -> %d\n", fn, 
                  arrNumRHSUnknowns[fn], arrSpecialFnStack[nSpecialFnStackIdx].u.value,
                  arrNumLHSUnknowns[fn], arrSpecialFnStack[nSpecialFnStackIdx].lhsvalue);

            arrNumRHSUnknownsNew[fn]=
            arrPrevNumRHSUnknowns[fn]=
            arrNumRHSUnknowns[fn]=arrSpecialFnStack[nSpecialFnStackIdx].u.value;

            arrNumLHSUnknownsNew[fn]=
            arrPrevNumLHSUnknowns[fn]=
            arrNumLHSUnknowns[fn]=arrSpecialFnStack[nSpecialFnStackIdx].lhsvalue;

            arrSumRHSUnknownsNew[fn]=
            arrPrevSumRHSUnknowns[fn]=
            arrSumRHSUnknowns[fn]=arrSpecialFnStack[nSpecialFnStackIdx].rhssum;

            arrSpecialFnFlags[fn]=arrSpecialFnStack[nSpecialFnStackIdx].prev;
            arrChangedSpecialFn[fn]=0;
         }
      }
      nSpecialFnStackIdx--;
   }

   /* keep the mark on the top of the stack */
   assert(nSpecialFnStackIdx>0);

   assert(arrSpecialFnStack[nSpecialFnStackIdx].fn == 
         arrSmurfStatesStack[nSmurfStatesStackIdx].smurf);

   if (arrSpecialFnStack[nSpecialFnStackIdx].fn == LEVEL_MARK)  {
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
      /* pop smurf state stack */
      assert(nSmurfStatesStackIdx>0);

      /* until LEVEL_START */
      while (arrSmurfStatesStack[nSmurfStatesStackIdx].smurf != LEVEL_START) {
         int smurf=arrSmurfStatesStack[nSmurfStatesStackIdx].smurf;

         if (smurf == POOL_START) {
            nSmurfStatesStackIdx--;
            tSmurfStatesStack *new_arrSmurfStatesStack = (tSmurfStatesStack*)
               (arrSmurfStatesStack[nSmurfStatesStackIdx--].u.next_pool);
            nSmurfStatesStackIdx = 
               arrSmurfStatesStack[nSmurfStatesStackIdx].u.index_pool;
            assert(new_arrSmurfStatesStack[nSmurfStatesStackIdx].smurf==POOL_END);
            arrSmurfStatesStack = new_arrSmurfStatesStack;
         } else 
            if (smurf >= 0) {
               d9_printf2("pop_state_information: smurf(%d)\n", smurf);

               arrPrevStates[smurf] =
               arrCurrentStates[smurf] =
                  arrSmurfStatesStack[nSmurfStatesStackIdx].u.state;
               arrSmurfPath[smurf].idx =
                  arrSmurfStatesStack[nSmurfStatesStackIdx].path_idx;
               arrSmurfStatesFlags[smurf]=
                  arrSmurfStatesStack[nSmurfStatesStackIdx].prev;
               arrChangedSmurfs[smurf]=0;
            }
         nSmurfStatesStackIdx--;
      }

      nSmurfStatesStackIdx--; /* skip the LEVEL_START */

      assert(nSmurfStatesStackIdx>0);
      //assert(nCurSmurfStatesVersion = arrSmurfStatesStack[nSmurfStatesStackIdx+1].u.version);
      //nCurSmurfStatesVersion--;

      nCurSmurfStatesVersion = arrSmurfStatesStack[nSmurfStatesStackIdx+1].u.version;

      /* Pop the special function information. */
      assert(nSpecialFnStackIdx > 0);

      /* until LEVEL_START */
      while (arrSpecialFnStack[nSpecialFnStackIdx].fn != LEVEL_START) {
         int fn=arrSpecialFnStack[nSpecialFnStackIdx].fn;

         if (fn == POOL_START) {
            nSpecialFnStackIdx--;
            tSpecialFnStack *new_arrSpecialFnStack = 
               (tSpecialFnStack*)(arrSpecialFnStack[nSpecialFnStackIdx--].u.next_pool);
            nSpecialFnStackIdx = arrSpecialFnStack[nSpecialFnStackIdx].u.index_pool;
            assert(new_arrSpecialFnStack[nSpecialFnStackIdx].fn==POOL_END);
            arrSpecialFnStack = new_arrSpecialFnStack;
         } else
            if (fn >= 0) {
               d9_printf6("pop_state_information: specfn(%d), #RHSUnknowns: %d -> %d, #LHSUnknowns: %d -> %d\n", fn, 
                     arrNumRHSUnknowns[fn], arrSpecialFnStack[nSpecialFnStackIdx].u.value,
                     arrNumLHSUnknowns[fn], arrSpecialFnStack[nSpecialFnStackIdx].lhsvalue);

               arrNumRHSUnknownsNew[fn]=
               arrPrevNumRHSUnknowns[fn]=
               arrNumRHSUnknowns[fn]=arrSpecialFnStack[nSpecialFnStackIdx].u.value;

               arrNumLHSUnknownsNew[fn]=
               arrPrevNumLHSUnknowns[fn]=
               arrNumLHSUnknowns[fn]=arrSpecialFnStack[nSpecialFnStackIdx].lhsvalue;

               arrSumRHSUnknownsNew[fn]=
               arrPrevSumRHSUnknowns[fn]=
               arrSumRHSUnknowns[fn]=arrSpecialFnStack[nSpecialFnStackIdx].rhssum;

               arrSpecialFnFlags[fn]=arrSpecialFnStack[nSpecialFnStackIdx].prev;
               arrChangedSpecialFn[fn]=0;
            }
         nSpecialFnStackIdx--;
      }

      nSpecialFnStackIdx--; /* skip the LEVEL_START */

      assert(nSpecialFnStackIdx>0);
      //assert(nCurSpecialFnVersion == arrSpecialFnStack[nSpecialFnStackIdx+1].u.version);
      //nCurSpecialFnVersion--;
      
      nCurSpecialFnVersion = arrSpecialFnStack[nSpecialFnStackIdx+1].u.version;
   }
}

ITE_INLINE
void
push_smurf_states_onto_stack()
{
   nCurSmurfStatesVersion++;
   Mark_arrSmurfStatesStack(LEVEL_START);
}

ITE_INLINE
void
push_special_fn_onto_stack()
{
   nCurSpecialFnVersion++;
   Mark_arrNumRHSUnknowns(LEVEL_START);
}

ITE_INLINE
void 
Mark_arrSmurfStatesStack(int vx) 
{ 
   nSmurfStatesStackIdx++; 
   if (arrSmurfStatesStack[nSmurfStatesStackIdx].smurf == POOL_END)
   {
      NextSmurfStatesStack();
      nSmurfStatesStackIdx++; 
   }
   arrSmurfStatesStack[nSmurfStatesStackIdx].smurf = vx;  
   arrSmurfStatesStack[nSmurfStatesStackIdx].u.version = nCurSmurfStatesVersion;  
}

ITE_INLINE
void
Mark_arrNumRHSUnknowns(int vx) 
{ 
   nSpecialFnStackIdx++; 
   if (arrSpecialFnStack[nSpecialFnStackIdx].fn == POOL_END)
   {
      NextSpecialFnStack();
      nSpecialFnStackIdx++; 
   }
   arrSpecialFnStack[nSpecialFnStackIdx].fn = vx;  
   arrSpecialFnStack[nSpecialFnStackIdx].u.version = nCurSpecialFnVersion;  
}

ITE_INLINE
void
Add_arrSmurfStatesStack(int vx) 
{ 
   nSmurfStatesStackIdx++; 
   if (arrSmurfStatesStack[nSmurfStatesStackIdx].smurf == POOL_END)
   {
      NextSmurfStatesStack();
      nSmurfStatesStackIdx++; 
   }
   arrSmurfStatesStack[nSmurfStatesStackIdx].smurf   = vx;  
   arrSmurfStatesStack[nSmurfStatesStackIdx].u.state = arrCurrentStates[vx]; 
   arrSmurfStatesStack[nSmurfStatesStackIdx].path_idx = arrSmurfPath[vx].idx;
   arrSmurfStatesStack[nSmurfStatesStackIdx].prev    = arrSmurfStatesFlags[vx]; 
   arrSmurfStatesFlags[vx]                           = nCurSmurfStatesVersion; 
}

ITE_INLINE
void
Add_arrNumRHSUnknowns(int vx) 
{ 
   nSpecialFnStackIdx++; 
   if (arrSpecialFnStack[nSpecialFnStackIdx].fn == POOL_END)
   {
      NextSpecialFnStack();
      nSpecialFnStackIdx++; 
   }
   arrSpecialFnStack[nSpecialFnStackIdx].fn      = vx;  
   arrSpecialFnStack[nSpecialFnStackIdx].u.value = arrNumRHSUnknowns[vx]; 
   arrSpecialFnStack[nSpecialFnStackIdx].lhsvalue = arrNumLHSUnknowns[vx]; 
   arrSpecialFnStack[nSpecialFnStackIdx].rhssum  = arrSumRHSUnknowns[vx]; 
   arrSpecialFnStack[nSpecialFnStackIdx].prev    = arrSpecialFnFlags[vx]; 
   arrSpecialFnFlags[vx]                         = nCurSpecialFnVersion; 
}

ITE_INLINE void
InitializeSmurfStatesStack()
{
   arrSmurfStatesFlags = (int*)ite_calloc(nNumRegSmurfs+1, sizeof(int), 
         9, "arrSmurfStatesFlags");
   AllocateSmurfStatesStack((nNumRegSmurfs+1)*SMURF_STATES_STACK_ALLOC_MULT);
}

ITE_INLINE void
NextSmurfStatesStack()
{
   assert(arrSmurfStatesStack[nSmurfStatesStackIdx].smurf == POOL_END);
   nSmurfStatesStackIdx++;

   if (arrSmurfStatesStack[nSmurfStatesStackIdx].u.next_pool == NULL) 
   {
      AllocateSmurfStatesStack((nNumRegSmurfs+1)*SMURF_STATES_STACK_ALLOC_MULT);
   } 
   else 
   {
      arrSmurfStatesStack = (tSmurfStatesStack*)arrSmurfStatesStack[nSmurfStatesStackIdx].u.next_pool;
      nSmurfStatesStackIdx    = 2;
   }
}

ITE_INLINE void
AllocateSmurfStatesStack(int newsize)
{
   assert(newsize > 0);

   newsize += 10000; /* it does not hurt to allocate a little bit more
                      * for backtracking stack */

   arrSmurfStatesStackPool = (tSmurfStatesStackPool*)ite_calloc(1, sizeof (tSmurfStatesStackPool),
         9, "arrSmurfStatesStackPool");

   arrSmurfStatesStackPool->max = newsize;

   arrSmurfStatesStackPool->stack =
      (tSmurfStatesStack*)ite_calloc(newsize, sizeof(tSmurfStatesStack),
                                     2, "smurf states stack");

   tSmurfStatesStack* prev_arrSmurfStatesStack = arrSmurfStatesStack;
   arrSmurfStatesStack = arrSmurfStatesStackPool->stack;

   arrSmurfStatesStackPool->next = arrSmurfStatesStackPoolHead;
   arrSmurfStatesStackPoolHead = arrSmurfStatesStackPool; 

   /* save the prev index*/
   arrSmurfStatesStack[0].u.index_pool  = nSmurfStatesStackIdx;
   arrSmurfStatesStack[1].u.next_pool   = (void *)prev_arrSmurfStatesStack;
   arrSmurfStatesStack[2].smurf         = POOL_START;
   arrSmurfStatesStack[newsize-2].smurf = POOL_END;
   arrSmurfStatesStack[newsize-1].u.next_pool   = (void *)NULL;

   nSmurfStatesStackIdx    = 2;
}

ITE_INLINE void
FreeSmurfStatesStack()
{
   while(arrSmurfStatesStackPoolHead != NULL) {
      free(arrSmurfStatesStackPoolHead->stack);
      tSmurfStatesStackPool *x = arrSmurfStatesStackPoolHead;
      arrSmurfStatesStackPoolHead = arrSmurfStatesStackPoolHead->next;
      free(x);
   }

   if (arrSmurfStatesFlags != NULL) {
      free(arrSmurfStatesFlags);
      arrSmurfStatesFlags = NULL;
   }
}

ITE_INLINE void
InitializeSpecialFnStack()
{
   arrSpecialFnFlags = (int*)ite_calloc(nNumSpecialFuncs+1, sizeof(int),
         9, "arrSpecialFnFlags");
   AllocateSpecialFnStack((nNumSpecialFuncs+1)*SPECIAL_FN_STACK_ALLOC_MULT);
}

ITE_INLINE void
NextSpecialFnStack()
{
   assert(arrSpecialFnStack[nSpecialFnStackIdx].fn == POOL_END);
   nSpecialFnStackIdx++;

   if (arrSpecialFnStack[nSpecialFnStackIdx].u.next_pool == NULL) 
   {
      AllocateSpecialFnStack((nNumSpecialFuncs+1)*SPECIAL_FN_STACK_ALLOC_MULT);
   } 
   else 
   {
      arrSpecialFnStack = (tSpecialFnStack*)arrSpecialFnStack[nSpecialFnStackIdx].u.next_pool;
      nSpecialFnStackIdx = 2;
   }
}

ITE_INLINE void
AllocateSpecialFnStack(int newsize)
{
   assert(newsize > 0);
   newsize += 10000; /* it does not hurt to allocate a little bit more
                      * for backtracking stack */

   arrSpecialFnStackPool = (tSpecialFnStackPool*)ite_calloc(1, sizeof (tSpecialFnStackPool),
         9, "arrSpecialFnStackPool");

   arrSpecialFnStackPool->max = newsize;
   arrSpecialFnStackPool->stack =
      (tSpecialFnStack*)ite_calloc(newsize, sizeof(tSpecialFnStack), 
                                   2, "special function stack");

   tSpecialFnStack* prev_arrSpecialFnStack = arrSpecialFnStack;
   arrSpecialFnStack = arrSpecialFnStackPool->stack;
   arrSpecialFnStackPool->next = arrSpecialFnStackPoolHead;
   arrSpecialFnStackPoolHead = arrSpecialFnStackPool;

   /* save the prev index*/
   arrSpecialFnStack[0].u.index_pool = nSpecialFnStackIdx; 
   arrSpecialFnStack[1].u.next_pool  = (void *)prev_arrSpecialFnStack;
   arrSpecialFnStack[2].fn = POOL_START;
   arrSpecialFnStack[newsize-2].fn = POOL_END;
   arrSpecialFnStack[newsize-1].u.next_pool  = (void *)NULL;

   nSpecialFnStackIdx      = 2;
}

ITE_INLINE void
FreeSpecialFnStack()
{
   while(arrSpecialFnStackPoolHead) {
      free(arrSpecialFnStackPoolHead->stack);
      tSpecialFnStackPool *x = arrSpecialFnStackPoolHead;
      arrSpecialFnStackPoolHead = arrSpecialFnStackPoolHead->next;
      free(x);
   }

   if (arrSpecialFnFlags != NULL) {
      free(arrSpecialFnFlags);
      arrSpecialFnFlags = NULL;
   }
}
