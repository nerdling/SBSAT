/* =========FOR INTERNAL USE ONLY. NO DISTRIBUTION PLEASE ========== */

/*********************************************************************
 Copyright 1999-2004, University of Cincinnati.  All rights reserved.
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

ITE_INLINE int BrancherPresetInt(int *ptr);
ITE_INLINE int InitBrancherX();

ITE_INLINE void
k_add2list(int **list, int *list_num, int *list_max, int *my_list, int my_list_len)
{
   BacktrackStackEntry *ptr = arrBacktrackStack;
   /*
   fprintf(stderr, "\nnono list: ");
   for(int i=0;i<my_list_len;i++)
      fprintf(stderr, "%d ", my_list[i]);
   fprintf(stderr, "\n");
*/
   for(;ptr < pBacktrackTop;ptr++) {
      assert(NO_LEMMAS == 0);
      int j;
      for(j=0;j<my_list_len;j++) {
         if (my_list[j] == (arrSolution[ptr->nBranchVble]==BOOL_TRUE?1:-1)*ptr->nBranchVble) break;
      }
      if (j<my_list_len) continue;
      if ((*list_num)+1 >= *list_max) {
         *list = (int*)ite_recalloc(*(void**)list, *list_max, (*list_max)+100, sizeof(int), 9, "add2list list");
         (*list_max) += 100;
      }
      d4_printf3(" added(%c%d)", arrSolution[ptr->nBranchVble]==BOOL_TRUE?'+':'-', ptr->nBranchVble);
      (*list)[(*list_num)] = (arrSolution[ptr->nBranchVble]==BOOL_TRUE?1:-1)*ptr->nBranchVble;
      (*list_num)++;
   }
}

ITE_INLINE void
k_inter2list(int **list, int *list_num, int *list_max)
{
   BacktrackStackEntry *ptr = arrBacktrackStack;
   int list_inter=0;
   while(ptr < pBacktrackTop) {
      d4_printf3(" inter(%c%d)", arrSolution[ptr->nBranchVble]==BOOL_TRUE?'+':'-', ptr->nBranchVble);
      for(int j=0;j<*list_num;j++) {
         if ((arrSolution[ptr->nBranchVble]==BOOL_TRUE?1:-1)*ptr->nBranchVble == (*list)[j]) {
            d4_printf1("*");
            int tmp = (*list)[j];
            (*list)[j] = (*list)[list_inter];
            (*list)[list_inter] = tmp;
            list_inter++;
            break;
         }
      }
      ptr++;
   }
   *list_num = list_inter;
}

ITE_INLINE int
kSolver(int k_top_vars)
{
   d3_printf2("Starting kSolver with %d top variables\n", k_top_vars);
   int i;
   int nInferredAtom=0, nInferredValue=0;
   int *infs=NULL, infs_num=0, infs_max=0, bt_start=0;
   int *top_vars = (int*)ite_calloc(k_top_vars+2, sizeof(int), 9, "kSolver top_vars");
   int *top_vars_vals = (int*)ite_calloc(k_top_vars+2, sizeof(int), 9, "kSolver top_vars_vals");
   d4_printf1("K-top variables are: ");
   for(i=0;i<k_top_vars;i++) {
      proc_call_heuristic(&nInferredAtom, &nInferredValue);
      if (nInferredAtom == 0) { i--; break; }
      top_vars[i] = nInferredAtom;
      arrSolution[nInferredAtom] = BOOL_TRUE; // just something so we don't get it again
      d4_printf2("%d ", nInferredAtom);
   }
   d4_printf1("\n");
   if (i==k_top_vars) {
ksolver_restart:
      for(int k=1;k<nNumVariables;k++) {
         fprintf(stderr, " kSolver %d/%d(%d) \r", k, nNumVariables, bt_start);
         int j;
         int counter=0;
         for(j=0;j<k_top_vars;j++) {
            if (top_vars[j] == k) break;
            top_vars_vals[j+bt_start] = -1*top_vars[j];
         }
         if (j != k_top_vars) continue; // don't use this variable
         top_vars_vals[k_top_vars+bt_start] = -1*k;
         top_vars[k_top_vars] = k;
         infs_num=0; // reset infs array
         int bt=k_top_vars; // there is k_top_vars+1 variables
         while (1) {
            /*
            for(int m=0;m<=k_top_vars+bt_start;m++)
               fprintf(stderr, "%d ", top_vars_vals[m]);
            fprintf(stderr, " (");
            for(int m=0;m<infs_num;m++)
               fprintf(stderr, "%d ", infs[m]);
            fprintf(stderr, ")\n");
            */

            int ret;
            ret = InitBrancherX();
            if (ret != SOLV_UNKNOWN) return ret; // should never happen

            ret = BrancherPresetInt(top_vars_vals);
            if (ret == SOLV_UNKNOWN) {
               if (counter == 0) {
                  // collect all inferenced literals 
                  k_add2list(&infs, &infs_num, &infs_max, top_vars_vals, k_top_vars+bt_start);
               } else {
                  // intersect inferenced literals
                  k_inter2list(&infs, &infs_num, &infs_max);
                  if (infs_num == 0) { 
     //                fprintf(stderr, "====\n"); 
                     break; 
                  }
               }
            } else {
      //         fprintf(stderr, "=-=-=-=\n");
               // it is unsat
               // could be sat?
            }

            while(bt >= 0 && top_vars_vals[bt+bt_start] > 0) bt--;
            if (bt < 0) break;
            top_vars_vals[bt+bt_start] = -top_vars_vals[bt+bt_start]; // was false turn it into true
            bt++;
            while(bt <= k_top_vars) { top_vars_vals[bt+bt_start] = -1*top_vars[bt]; bt++; }
            bt--;
            counter++;
         }
         // if there are any literals left in the inference set
         // use them ==> global
         if (infs_num) {
            d4_printf2(" %d infs ", infs_num);
            top_vars_vals = (int*)ite_recalloc((void*)top_vars_vals, bt_start+k_top_vars+2, bt_start+k_top_vars+2+infs_num,sizeof(int), 9, "kSolver top_vars_vals");
            for(i=0;i<infs_num;i++) {
               d4_printf2(" globalinfs(%d)", infs[i]);
               top_vars_vals[bt_start+i] = infs[i];
               //fprintf(stderr, "%d ", infs[i]);
            }
            bt_start += infs_num;
            //fprintf(stderr, "\n");
            goto ksolver_restart;
         } else {
            if (bt >= 0) {
               d4_printf2("Early bailout after %d\n", counter);
            } else {
               d4_printf2(" %d infs\n", infs_num);
            }
         }
      }
   }
   top_vars_vals[bt_start] = 0;
   ite_free((void **)&infs);
   ite_free((void **)&top_vars);
   d3_printf1("kSolver done\n");
   InitBrancherX();
   return BrancherPresetInt(top_vars_vals);
   //return SOLV_UNKNOWN;
}

