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

void
fprintVars(FILE *fout, int nNumVariables)
{
#define VAR_PER_LINE 30 /* make it a multiple of 10 */
int i,j;
for (i=0; i<nNumVariables; i++)
{
  if ((i%VAR_PER_LINE) == 0)
  {
    if (i) fprintf(fout, "\n");
    fprintf(fout, "% 6d: ", i);
    if ((i%(10*VAR_PER_LINE)) == 0) 
    {
     for (j=0;j<VAR_PER_LINE;j++)
      fprintf(fout, "%d ", j%10);
     fprintf(fout, "\n% 6d: ", i);
    }
  }
  fprintf(fout, "%c ", arrSolution[i]==BOOL_TRUE?'T':
                       arrSolution[i]==BOOL_FALSE?'F':'*');
   
}
fprintf(fout, "\n");
}

ITE_INLINE
void
I_OptimizedHeuristic(int *pnBranchAtom, int *pnBranchValue)
{
  int lit=0;

  D_3(fprintVars(stdout, nNumVariables);)
  do
  {
     fprintf(stdout, "Please enter the next variable to branch on: ");
     if (scanf("%d", &lit)!=1) {
        char c;
        scanf("%c", &c);  
        lit=0;
     }
     if (lit != 0) {
        if (abs(lit) >= nNumVariables) {
           fprintf(stdout, "This variable does not exist\n");
           lit=0;
        } else
           if (arrSolution[abs(lit)] != BOOL_UNKNOWN) {
              fprintf(stdout, "This variable is already set to %c\n",
                    arrSolution[abs(lit)]==BOOL_TRUE?'T':'F');
              lit=0;
           } else {
              if (lit > 0) {
                 *pnBranchValue = BOOL_TRUE;
                 *pnBranchAtom = lit;
              } else {
                 *pnBranchValue = BOOL_FALSE;
                 *pnBranchAtom = -lit;
              }
           }
     } 
  }
  while (lit==0);
  fprintf(stdout, "Branching on %d\n", lit);
}

