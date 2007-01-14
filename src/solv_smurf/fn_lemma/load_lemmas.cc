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

#define MAX_LEMMA_LEN 1024
int lemma[MAX_LEMMA_LEN];

void
LoadLemmas(char *filename)
{
   int idx = 0;
   int lemma_no = 0;
   FILE *fin = fopen(filename, "r");
   LemmaInfoStruct *p;
   if (!fin) return;

   d2_printf2("Reading lemmas from %s\n", filename);
   do {
      idx = 0;
      while (!feof(fin)) {
         if (1 != fscanf(fin, "%d", &(lemma[idx]))) break;
         if (lemma[idx] == 0) break;
         idx++;
         if (idx >= MAX_LEMMA_LEN) break;
      }
      if (lemma[idx] != 0) break;
      p = AddLemma(idx, lemma, true, NULL, NULL);
      assert(p);
      AddLemmaIntoCache(p);
      d2_printf2("%d\r", lemma_no++);
   } while (!feof(fin));

   fclose(fin);
}
