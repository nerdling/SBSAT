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
extern SmurfState **arrCurrentStates;
extern int *arrNumRHSUnknowns;

#define YES_AUTARKY 1
#define NO_AUTARKY 0

tSmurfStatesStack* AU_arrSmurfStatesStack;
int AU_nSmurfStatesStackIdx;

ITE_INLINE void check_pop_information_init();
ITE_INLINE int check_pop_information_specfn();
ITE_INLINE int check_pop_information_smurfs();
ITE_INLINE int is_autarky();

ITE_INLINE
int
check_pop_information_smurfs()
{
   if (arrCurrentStates == NULL) return YES_AUTARKY;

   while (AU_arrSmurfStatesStack[AU_nSmurfStatesStackIdx].smurf != LEVEL_START) 
   {
      int smurf=AU_arrSmurfStatesStack[AU_nSmurfStatesStackIdx].smurf;

      if (smurf == POOL_START)
      {
         AU_nSmurfStatesStackIdx--;
         tSmurfStatesStack *new_arrSmurfStatesStack = (tSmurfStatesStack*)
            (AU_arrSmurfStatesStack[AU_nSmurfStatesStackIdx--].smurf);
         AU_nSmurfStatesStackIdx =
            AU_arrSmurfStatesStack[AU_nSmurfStatesStackIdx].smurf;
         if (new_arrSmurfStatesStack == NULL) break;
         assert(new_arrSmurfStatesStack[AU_nSmurfStatesStackIdx].smurf==POOL_END);
         AU_arrSmurfStatesStack = new_arrSmurfStatesStack;
      } else {
         if (smurf >= 0)
         {
            d9_printf3("autarky: check_pop_information_smurfs(%d), state: %s\n", smurf, arrCurrentStates[smurf] == pTrueSmurfState?"True":"non-true");
            if (arrCurrentStates[smurf] != pTrueSmurfState) return NO_AUTARKY;
         }
      }
      AU_nSmurfStatesStackIdx--;
   }

   AU_nSmurfStatesStackIdx--;
   return YES_AUTARKY;
}

tSpecialFnStack* AU_arrSpecialFnStack;
int AU_nSpecialFnStackIdx;

ITE_INLINE
int
check_pop_information_specfn()
{
   if (arrNumRHSUnknowns == NULL) return YES_AUTARKY;

   while (AU_arrSpecialFnStack[AU_nSpecialFnStackIdx].fn != LEVEL_START) 
   {
      int fn=AU_arrSpecialFnStack[AU_nSpecialFnStackIdx].fn;

      if (fn == POOL_START) {
         AU_nSpecialFnStackIdx--;
         tSpecialFnStack *new_arrSpecialFnStack =
            (tSpecialFnStack*)(AU_arrSpecialFnStack[AU_nSpecialFnStackIdx--].fn);
         AU_nSpecialFnStackIdx = AU_arrSpecialFnStack[AU_nSpecialFnStackIdx].fn;
         if (new_arrSpecialFnStack == NULL) break;
         assert(new_arrSpecialFnStack[AU_nSpecialFnStackIdx].fn==POOL_END);
         AU_arrSpecialFnStack = new_arrSpecialFnStack;
      } else {
         if (fn >= 0)
         {
            d9_printf3("autarky: check_pop_information_specfn(%d), #RHSUnknowns: %d\n", fn, arrNumRHSUnknowns[fn]);
            if (arrNumRHSUnknowns[fn] != 0) return NO_AUTARKY;
         }
      }
      AU_nSpecialFnStackIdx--;
   }

   AU_nSpecialFnStackIdx--;
   return YES_AUTARKY;
}

ITE_INLINE
void
check_pop_information_init()
{
   AU_arrSmurfStatesStack   = arrSmurfStatesStack;
   AU_nSmurfStatesStackIdx  = nSmurfStatesStackIdx;

   AU_arrSpecialFnStack   = arrSpecialFnStack;
   AU_nSpecialFnStackIdx  = nSpecialFnStackIdx;
}


/* in the end of backjump loop */

extern bool *arrLemmaFlag;

ITE_INLINE
int
is_autarky()
{
   d9_printf1("Autarky check\n");

   if (AU_nSpecialFnStackIdx <= 0) return NO_AUTARKY;
   if (AU_nSmurfStatesStackIdx <= 0) return NO_AUTARKY;

   /* check each affected smurf -- 
    if not satisfied => not autarky */
   if (check_pop_information_smurfs()==YES_AUTARKY) {

      /* check each affected special function -- 
       if not satisfied => not autarky */
      if (check_pop_information_specfn()==YES_AUTARKY)
      {
         return YES_AUTARKY;
      }
   }
   return NO_AUTARKY;
}

