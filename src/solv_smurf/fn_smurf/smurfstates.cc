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

#define SIZE_SMURF_STATE_POOL 20000 // Size of initial pool of Smurf states.

typedef struct _t_smurfstate_pool {
   SmurfState *memory;
   int   max;
   struct _t_smurfstate_pool *next;
} t_smurfstate_pool;

t_smurfstate_pool *smurfstate_pool = NULL;
t_smurfstate_pool *smurfstate_pool_head = NULL;
int smurfstate_pool_index=0;


ITE_INLINE void
InitializeSmurfStatePool()
{
   t_smurfstate_pool *tmp_smurfstate_pool;
   tmp_smurfstate_pool = (t_smurfstate_pool*)ite_calloc(1, sizeof(t_smurfstate_pool),
         9, "tmp_smurfstate_pool");
   tmp_smurfstate_pool->max = SIZE_SMURF_STATE_POOL;
   tmp_smurfstate_pool->memory = (SmurfState *)ite_calloc(tmp_smurfstate_pool->max, sizeof(SmurfState),
        9, "tmp_smurfstate_pool->memory");

   if (smurfstate_pool_head == NULL) {
      smurfstate_pool = tmp_smurfstate_pool;
      smurfstate_pool_head = smurfstate_pool;
   } else {
      smurfstate_pool->next = (struct _t_smurfstate_pool*)tmp_smurfstate_pool;
      smurfstate_pool = (t_smurfstate_pool*)(smurfstate_pool->next);
   }
   smurfstate_pool_index = 0;
}

ITE_INLINE SmurfState *
AllocateSmurfState()
{
   SmurfState *smurfstate;

   if (smurfstate_pool == NULL || smurfstate_pool_index == smurfstate_pool->max)
   {
      InitializeSmurfStatePool();
   }
   smurfstate = smurfstate_pool->memory+smurfstate_pool_index;
   smurfstate_pool_index++;
   ite_counters[SMURF_STATES]++;
   return smurfstate;
}

ITE_INLINE void
FreeSmurfStatePool()
{
   t_smurfstate_pool *tmp_smurfstate_pool = NULL;
   while (smurfstate_pool_head != NULL)
   {
      SmurfState *arrSmurfStatePool = smurfstate_pool_head->memory;

      for (int i=0;i<smurfstate_pool_head->max;i++)
      {
         if (arrSmurfStatePool[i].arrTransitions) {
            for (int k=0;k<arrSmurfStatePool[i].vbles.nNumElts;k++) {
               
               if (arrSmurfStatePool[i].arrTransitions[2*k+BOOL_TRUE].positiveInferences.arrElts)
                  free(arrSmurfStatePool[i].arrTransitions[2*k+BOOL_TRUE].positiveInferences.arrElts);
	       else 
               if (arrSmurfStatePool[i].arrTransitions[2*k+BOOL_TRUE].negativeInferences.arrElts)
                  free(arrSmurfStatePool[i].arrTransitions[2*k+BOOL_TRUE].negativeInferences.arrElts);
	      
               if (arrSmurfStatePool[i].arrTransitions[2*k+BOOL_FALSE].positiveInferences.arrElts)
                  free(arrSmurfStatePool[i].arrTransitions[2*k+BOOL_FALSE].positiveInferences.arrElts);
          else
               if (arrSmurfStatePool[i].arrTransitions[2*k+BOOL_FALSE].negativeInferences.arrElts)
                  free(arrSmurfStatePool[i].arrTransitions[2*k+BOOL_FALSE].negativeInferences.arrElts);

            }
            free(arrSmurfStatePool[i].arrTransitions);
         }
         if (arrSmurfStatePool[i].vbles.arrElts != NULL)
            free(arrSmurfStatePool[i].vbles.arrElts);
         if (arrSmurfStatePool[i].arrHeuristicXors != NULL) 
            free(arrSmurfStatePool[i].arrHeuristicXors);
         
      }
      free(smurfstate_pool_head->memory);
      tmp_smurfstate_pool = (t_smurfstate_pool*)(smurfstate_pool_head->next);
      free(smurfstate_pool_head);
      smurfstate_pool_head = tmp_smurfstate_pool;
   }
}

ITE_INLINE void
SmurfStatesDisplayInfo()
{
   d3_printf2("Number of Smurf states: %ld\n", (long)(ite_counters[SMURF_STATES]));
}

/* don't use -- bad one */
ITE_INLINE SmurfState *
GetSmurfState(int i) 
{
   if (i >= smurfstate_pool_index) return NULL;
   return smurfstate_pool->memory+i;
}
