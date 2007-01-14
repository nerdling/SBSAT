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

HeurScores *arrHeurScores=NULL;

#define HEUR_SCORES_STACK_ALLOC_MULT 6 /* >= 2 */
#define MAX_HEUR_SCORES_STACK_POOL 100
int nCurHeurScoresVersion=1;
int nHeurScoresStackIdx=0;
int nMaxHeurScoresStackIdx=0;
tHeurScoresStack *arrHeurScoresStack=NULL;
int *arrHeurScoresFlags=NULL;
typedef struct { tHeurScoresStack *stack; int max; } tHeurScoresStackPool;
tHeurScoresStackPool *arrHeurScoresStackPool=NULL;
int nHeurScoresStackPool=0;    
int nHeurScoresStackPoolMax=MAX_HEUR_SCORES_STACK_POOL;

ITE_INLINE void
AllocateHeurScoresStack(int newsize)
{
   assert(newsize > 0);

   if (arrHeurScoresFlags==NULL) {
      arrHeurScoresFlags = (int*)ite_calloc(nNumVariables, sizeof(int),
            9, "arrHeurScoresFlags");
      arrHeurScoresStackPool = (tHeurScoresStackPool*)ite_calloc(
            nHeurScoresStackPoolMax, sizeof (tHeurScoresStackPool),
            9, "arrHeurScoresStackPool");
   }

   if (nHeurScoresStackPool >= nHeurScoresStackPoolMax) {
      arrHeurScoresStackPool = (tHeurScoresStackPool*)ite_recalloc(
            arrHeurScoresStackPool, nHeurScoresStackPoolMax,
            nHeurScoresStackPoolMax+MAX_HEUR_SCORES_STACK_POOL, 
            sizeof (tHeurScoresStackPool), 9, "arrHeurScoresStackPool");
      nHeurScoresStackPoolMax += MAX_HEUR_SCORES_STACK_POOL;
   }

   if (arrHeurScoresStackPool[nHeurScoresStackPool].stack == NULL) {
      newsize += INIT_STACK_BACKTRACKS_ALLOC*4; 

      arrHeurScoresStackPool[nHeurScoresStackPool].max = newsize;

      arrHeurScoresStackPool[nHeurScoresStackPool].stack =
         (tHeurScoresStack*)ite_calloc(newsize, sizeof(tHeurScoresStack), 2,
                                   "j heuristic states stack");

      tHeurScoresStack* prev_arrHeurScoresStack = arrHeurScoresStack;
      arrHeurScoresStack = arrHeurScoresStackPool[nHeurScoresStackPool].stack;

      /* save the prev index*/
      arrHeurScoresStack[0].u.index_pool  = nHeurScoresStackIdx;
      arrHeurScoresStack[1].u.next_pool   = (void *)prev_arrHeurScoresStack;
      arrHeurScoresStack[2].v             = POOL_START;
      arrHeurScoresStack[newsize-1].v     = POOL_END;

   } else {
      arrHeurScoresStack = arrHeurScoresStackPool[nHeurScoresStackPool].stack;
   }

   nHeurScoresStackIdx    = 2;
}

ITE_INLINE void
FreeHeurScoresStack ()
{
   if (arrHeurScoresStackPool) {
      for (int i=0;i<=nHeurScoresStackPoolMax && arrHeurScoresStackPool[i].stack;i++)
         ite_free((void**)&arrHeurScoresStackPool[i].stack);
      ite_free((void**)&arrHeurScoresStackPool);
      ite_free((void**)&arrHeurScoresFlags);
   }
   ite_free((void**)&arrHeurScores);
}

ITE_INLINE void
InitHeurScoresStack()
{
   arrHeurScores = (HeurScores *)ite_calloc(nNumVariables, sizeof(HeurScores), 2,
         "heuristic scores");

   AllocateHeurScoresStack (nNumVariables * HEUR_SCORES_STACK_ALLOC_MULT);
}

ITE_INLINE void
Mark_arrHeurScoresStack(int vx);

ITE_INLINE void
PushHeuristicScores()
{
   nCurHeurScoresVersion++;
   Mark_arrHeurScoresStack(LEVEL_START);
}

ITE_INLINE void
Mark_arrHeurScoresStack(int vx)
{
   nHeurScoresStackIdx++;
   if (arrHeurScoresStack[nHeurScoresStackIdx].v == POOL_END)
   {
      nHeurScoresStackPool++;
      AllocateHeurScoresStack (nNumVariables * HEUR_SCORES_STACK_ALLOC_MULT);
      nHeurScoresStackIdx++;
   }
   arrHeurScoresStack[nHeurScoresStackIdx].v = vx;
}

ITE_INLINE
void
Add_arrHeurScoresStack(int vx)
{
   nHeurScoresStackIdx++;
   if (arrHeurScoresStack[nHeurScoresStackIdx].v == POOL_END)
   {
      nHeurScoresStackPool++;
      AllocateHeurScoresStack (nNumVariables * HEUR_SCORES_STACK_ALLOC_MULT);
      nHeurScoresStackIdx++;
   }

   arrHeurScoresStack[nHeurScoresStackIdx].v = vx;  
   arrHeurScoresStack[nHeurScoresStackIdx].h = arrHeurScores[vx];
   arrHeurScoresStack[nHeurScoresStackIdx].prev  = arrHeurScoresFlags[vx]; 
   arrHeurScoresFlags[vx]=nCurHeurScoresVersion;
}

ITE_INLINE
void
PopHeuristicScores(int n)
{
   for(;n>0;n--)
   {
      /* pop heur scores stack */
      assert(nHeurScoresStackIdx>0);

      /* until LEVEL_START */
      while (arrHeurScoresStack[nHeurScoresStackIdx].v != LEVEL_START) 
      {
         int v=arrHeurScoresStack[nHeurScoresStackIdx].v;
         if (v == POOL_START) {
            nHeurScoresStackPool--;
            nHeurScoresStackIdx--;
            tHeurScoresStack *new_arrHeurScoresStack = 
               (tHeurScoresStack*)
               (arrHeurScoresStack[nHeurScoresStackIdx--].u.next_pool);
            nHeurScoresStackIdx =
               arrHeurScoresStack[nHeurScoresStackIdx].u.index_pool;
            assert(new_arrHeurScoresStack[nHeurScoresStackIdx].v==POOL_END);
            arrHeurScoresStack = new_arrHeurScoresStack;
         } else
            if (v >= 0) {
               arrHeurScores[v] =arrHeurScoresStack[nHeurScoresStackIdx].h;
               arrHeurScoresFlags[v]=arrHeurScoresStack[nHeurScoresStackIdx].prev;
            }
         nHeurScoresStackIdx--;
      }
      nHeurScoresStackIdx--; /* skip the LEVEL_START */
      assert(nHeurScoresStackIdx>0);
      nCurHeurScoresVersion--;
   }
}

