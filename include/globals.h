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
/*********************************************************************
 *  globals.h (S. Weaver, J. Franco)
 *********************************************************************/
#ifndef GLOBALS_H
#define GLOBALS_H

void ite_free(void **ptr);
void *ite_calloc(int x, int y, int dbg_lvl, const char *for_what);
void *ite_recalloc(void *ptr, int oldx, int x, int y, int dbg_lvl, const char *for_what);

char *ite_basename(char *filename);


#define ITE_NEW_CATCH(x, desc) \
 try { x; } catch(...) { \
  cerr << "ERROR: Unable to allocate memory for " << desc; \
  cerr << "IN FILE " << __FILE__ << " AT " << __LINE__ << endl; \
  exit(1); };

extern long numinp, numout;
extern BDDNode *false_ptr, *true_ptr;

//Begin preprocessing globals

extern int DO_CLUSTER;  //CNF clustering
extern int DO_COFACTOR;
extern int DO_PRUNING;
extern int DO_STRENGTH;
extern int DO_INFERENCES;
extern int DO_EXIST_QUANTIFY;
extern int DO_EXIST_QUANTIFY_AND;
extern int DO_DEP_CLUSTER;
extern int MAX_EXQUANTIFY_CLAUSES;     //Number of clauses a variable appears in
                                     //to quantify that variable away.
extern int COF_MAX;
extern int MAX_EXQUANTIFY_VARLENGTH;   //Limits size of number of vars in 
                                     //constraints created by ExQuantify
//End preprocessing globals
#endif
