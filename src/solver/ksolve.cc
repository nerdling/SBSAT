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

ITE_INLINE void
add2list(int **list, int *list_num, int *list_max, int *addlist, int addlist_num, int polarity)
{
   if ((*list_num)+addlist_num >= *list_max) {
      *list = (int*)ite_recalloc(*(void**)list, *list_max, (*list_max)+100, sizeof(int), 9, "add2list list");
      (*list_max) += 100;
   }
   for(int i=0;i<addlist_num;i++) {
      d4_printf2(" added(%d)", addlist[i]*polarity);
      (*list)[(*list_num)+i] = addlist[i]*polarity;
   }
   (*list_num) += addlist_num;
}

ITE_INLINE void
inter2list(int **list, int *list_num, int *list_max, int *list_inter, int *interlist, int interlist_num, int polarity)
{
   for(int i=0;i<interlist_num;i++) {
      d4_printf2(" inter(%d)", interlist[i]*polarity);
      for(int j=0;j<*list_num;j++) {
         if (interlist[i]*polarity == (*list)[j]) {
            d4_printf1("*");
            int tmp = (*list)[j];
            (*list)[j] = (*list)[*list_inter];
            (*list)[*list_inter] = tmp;
            (*list_inter)++;
            break;
         }
      }
   }
}

ITE_INLINE void
kSmurf(SmurfState *pStartState, int *top_vars, int k_top_vars)
{
   SmurfState *pState;
   int i, bt=k_top_vars-1, *infs=NULL, infs_num=0, infs_max=0, counter=0;
   for(i=0; i<k_top_vars; i++) arrSolution[top_vars[i]] = BOOL_FALSE;

   while(1) 
   {
      int list_inter=0;
      pState = pStartState;
/*
      for(int z=0;z<k_top_vars;z++) 
         fprintf(stderr, "%c%d ", arrSolution[top_vars[z]]==BOOL_TRUE?'+':'-', top_vars[z]);
      fprintf(stderr, "\n");
      */

      while(1) 
      {
         if (pState == pTrueSmurfState) break;

         int vble = 0, k;
         int nNumElts = pState->vbles.nNumElts;
         int *arrElts = pState->vbles.arrElts;
         for (k = 0; k < nNumElts; k++) 
         {
            vble = arrElts[k];
            if (arrSolution[vble]!=BOOL_UNKNOWN)  break;
         }

         /* if no more instantiated variables */
         if (k == nNumElts) break;

         // Get the transition.
         Transition *pTransition = FindTransition(pState, k, vble, arrSolution[vble]);
         if (pTransition->pNextState == NULL) pTransition = CreateTransition(pState, k, vble, arrSolution[vble]);
         assert(pTransition->pNextState != NULL);
         pState = pTransition->pNextState;

         if (counter==0) {
            // gather all the inferences
            add2list(&infs, &infs_num, &infs_max, pTransition->positiveInferences.arrElts, pTransition->positiveInferences.nNumElts, 1);
            add2list(&infs, &infs_num, &infs_max, pTransition->negativeInferences.arrElts, pTransition->negativeInferences.nNumElts, -1);
         } else {
            // intersection all the inferences
            inter2list(&infs, &infs_num, &infs_max, &list_inter, pTransition->positiveInferences.arrElts, pTransition->positiveInferences.nNumElts, 1);
            inter2list(&infs, &infs_num, &infs_max, &list_inter, pTransition->negativeInferences.arrElts, pTransition->negativeInferences.nNumElts, -1);
         }
      }
      // check all transitions? -- not necessary
      if (counter != 0) infs_num = list_inter;
      if (infs_num == 0) break;

      while(bt >= 0 && arrSolution[top_vars[bt]] == BOOL_TRUE) bt--;
      if (bt < 0) break;
      arrSolution[top_vars[bt]] = BOOL_TRUE;
      bt++;
      while(bt < k_top_vars) { arrSolution[top_vars[bt]] = BOOL_FALSE; bt++; }
      bt--;
      counter++;
   }
   // infs, infs_num => inferences
   // if you find it finding any inferences
   // add the code to apply them
   if (infs_num) {
      d4_printf2(" %d infs ", infs_num);
      for(i=0;i<infs_num;i++) 
         fprintf(stderr, "%d ", infs[i]);
      fprintf(stderr, "\n");
   } else {
      if (bt >= 0) {
         d4_printf2("Early bailout after %d", counter);
      } else {
         d4_printf2(" %d infs ", infs_num);
      }
   }
   if (infs) free(infs);
}

ITE_INLINE int
kSolver(int k_top_vars)
{
   d3_printf2("Starting kSolver with %d top variables\n", k_top_vars);
   int i;
   int nInferredAtom=0, nInferredValue=0;
   int *top_vars = (int*)ite_calloc(k_top_vars, sizeof(int), 9, "kSolver top_vars");
   d4_printf1("K-top variables are: ");
   for(i=0;i<k_top_vars;i++) {
      proc_call_heuristic(&nInferredAtom, &nInferredValue);
      if (nInferredAtom == 0) { i--; break; }
      top_vars[i] = nInferredAtom;
      arrSolution[nInferredAtom] = nInferredValue;
      d4_printf2("%d ", nInferredAtom);
   }
   d4_printf1("\n");
   if (i==k_top_vars) {
      int j;
      for(j=0;j<nNumRegSmurfs;j++)
      {
         d4_printf2("Checking %d smurf: ", j);
         kSmurf(arrRegSmurfInitialStates[j], top_vars, k_top_vars);
         d4_printf1("\n");
      }
   }
   for(i--;i>=0;i--) {
      arrSolution[top_vars[i]] = BOOL_UNKNOWN;
   }
   ite_free((void **)&top_vars);
   d3_printf1("kSolver done\n");
   return SOLV_UNKNOWN;
}

