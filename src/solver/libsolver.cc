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
//#define ITE_INLINE  __inline__

#include "smurffactory.cc"

/* init */
#include "solve.cc"
#include "init_solver.cc"

/* basic brancher */
#include "brancher.cc"
#include "select_bp.cc"
#include "update_heu.cc"

/* backtracking through ... */
#include "backtrack.cc"
#include "backtrack_nl.cc"
#include "backtrack_sbj.cc"
#include "bt_lemmas.cc"
#include "bt_smurfs.cc"
#include "bt_specfn.cc"
#include "bt_specfn_and.cc"
#include "bt_specfn_xor.cc"

/* null heuristic */
#include "heuristic.cc"

/* lemma heuristic */
#include "l_lemma.cc"
#include "l_heuristic.cc"

/* johnson heuristic */
#include "j_update_heu.cc"
#include "j_specfn.cc"
#include "j_heuristic.cc"

/* interactive heuristic */
#include "i_heuristic.cc"

#include "autarky.cc"

#include "bdd2smurf.cc"
#include "specfn2smurf.cc"
#include "smurfstates.cc"

#include "state_stacks.cc"

#include "lemmainfo.cc"
#include "lemmaspace.cc"
#include "verify.cc"
#include "display.cc"
#include "display_sf.cc"

#include "recordsol.cc"
#include "graphs.cc"

#include "transitions.cc"
#include "sf_addons.cc"

#include "bddwalk.cc"
#include "wvf.cc"

#include "crtwin.cc"

#include "load_lemmas.cc"
