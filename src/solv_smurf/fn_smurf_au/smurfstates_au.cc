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

#define SIZE_SMURF_AU_STATE_POOL 20000 // Size of initial pool of SmurfAu states.

typedef struct _t_smurf_austate_pool {
   SmurfAuState *memory;
   int   max;
   struct _t_smurf_austate_pool *next;
} t_smurf_austate_pool;

t_smurf_austate_pool *smurf_austate_pool = NULL;
t_smurf_austate_pool *smurf_austate_pool_head = NULL;
int smurf_austate_pool_index=0;


ITE_INLINE void
InitializeSmurfAuStatePool()
{
   t_smurf_austate_pool *tmp_smurf_austate_pool;
   tmp_smurf_austate_pool = (t_smurf_austate_pool*)ite_calloc(1, sizeof(t_smurf_austate_pool),
         9, "tmp_smurf_austate_pool");
   tmp_smurf_austate_pool->max = SIZE_SMURF_AU_STATE_POOL;
   tmp_smurf_austate_pool->memory = (SmurfAuState *)ite_calloc(tmp_smurf_austate_pool->max, sizeof(SmurfAuState),
        9, "tmp_smurf_austate_pool->memory");

   if (smurf_austate_pool_head == NULL) {
      smurf_austate_pool = tmp_smurf_austate_pool;
      smurf_austate_pool_head = smurf_austate_pool;
   } else {
      smurf_austate_pool->next = (struct _t_smurf_austate_pool*)tmp_smurf_austate_pool;
      smurf_austate_pool = (t_smurf_austate_pool*)(smurf_austate_pool->next);
   }
   smurf_austate_pool_index = 0;
}

ITE_INLINE SmurfAuState *
AllocateSmurfAuState()
{
   SmurfAuState *smurf_austate;

   if (smurf_austate_pool == NULL || smurf_austate_pool_index == smurf_austate_pool->max)
   {
      InitializeSmurfAuStatePool();
   }
   smurf_austate = smurf_austate_pool->memory+smurf_austate_pool_index;
   smurf_austate_pool_index++;
   ite_counters[SMURF_AU_STATES]++;
   return smurf_austate;
}

ITE_INLINE void
FreeSmurfAuStatePool()
{
   t_smurf_austate_pool *tmp_smurf_austate_pool = NULL;
   while (smurf_austate_pool_head != NULL)
   {
      SmurfAuState *arrSmurfAuStatePool = smurf_austate_pool_head->memory;

      for (int i=0;i<smurf_austate_pool_head->max;i++)
      {
         if (arrSmurfAuStatePool[i].arrTransitionAus) {
            for (int k=0;k<arrSmurfAuStatePool[i].vbles.nNumElts;k++) {
               
               if (arrSmurfAuStatePool[i].arrTransitionAus[2*k+BOOL_TRUE].positiveInferences.arrElts)
                  free(arrSmurfAuStatePool[i].arrTransitionAus[2*k+BOOL_TRUE].positiveInferences.arrElts);
	       else 
               if (arrSmurfAuStatePool[i].arrTransitionAus[2*k+BOOL_TRUE].negativeInferences.arrElts)
                  free(arrSmurfAuStatePool[i].arrTransitionAus[2*k+BOOL_TRUE].negativeInferences.arrElts);
	      
               if (arrSmurfAuStatePool[i].arrTransitionAus[2*k+BOOL_FALSE].positiveInferences.arrElts)
                  free(arrSmurfAuStatePool[i].arrTransitionAus[2*k+BOOL_FALSE].positiveInferences.arrElts);
          else
               if (arrSmurfAuStatePool[i].arrTransitionAus[2*k+BOOL_FALSE].negativeInferences.arrElts)
                  free(arrSmurfAuStatePool[i].arrTransitionAus[2*k+BOOL_FALSE].negativeInferences.arrElts);

            }
            free(arrSmurfAuStatePool[i].arrTransitionAus);
         }
         if (arrSmurfAuStatePool[i].vbles.arrElts != NULL)
            free(arrSmurfAuStatePool[i].vbles.arrElts);
      }
      free(smurf_austate_pool_head->memory);
      tmp_smurf_austate_pool = (t_smurf_austate_pool*)(smurf_austate_pool_head->next);
      free(smurf_austate_pool_head);
      smurf_austate_pool_head = tmp_smurf_austate_pool;
   }
}

ITE_INLINE void
SmurfAuStatesDisplayInfo()
{
   d3_printf2("Number of autarky Smurf states: %ld\n", (long)(ite_counters[SMURF_AU_STATES]));
}

/* don't use -- bad one */
ITE_INLINE SmurfAuState *
GetSmurfAuState(int i) 
{
   if (i >= smurf_austate_pool_index) return NULL;
   return smurf_austate_pool->memory+i;
}
