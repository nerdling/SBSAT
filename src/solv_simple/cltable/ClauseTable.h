

#include <stdint.h>
//#include "sbsat.h"
//#include "sbsat_solver.h"
//#include "solver.h"

#include "Vec.h"
#include "Alg.h"

typedef int Var;
#define var_Undef (-1)

typedef int Lit;
inline  bool sign        (Lit p)           { return p & 1; }
inline  int  var         (Lit p)           { return p >> 1; }

const Lit lit_Undef = var_Undef + var_Undef + (int) false;  // }- Useful special constants.
const Lit lit_Error = var_Undef + var_Undef + (int) true;  // }


//=================================================================================================
// Lifted booleans: (3 valued logic)

typedef char lbool;
inline int   toInt  (lbool l) { return (int) l;} //l.toInt(); }

const lbool l_True  =  1;
const lbool l_False = -1;
const lbool l_Undef =  0;




//=================================================================================================
// Clause -- a simple class for representing a clause:

typedef vec<Lit> Clause;

class ClauseTable {
public:
  ClauseTable();
  bool    addClause (Clause* ps);       // Add a clause to the solver. NOTE! 'ps' may be shrunk by this method!
  //lbool   value      (Var x) const;       // The current value of a variable.
  lbool   value      (Lit p) const;       // The current value of a literal.
  int ApplyInferenceToClauseTable(Lit p);
  void attachClause(Clause& c);

  //private:
  //bool                ok;    // If FALSE, the constraints are already unsatisfiable. No part of the solver state may be used!

 protected:
    vec<char>           assigns;    // The current assignments (lbool:s stored as char:s).
    vec<vec<Clause*> >  watches;   // 'watches[lit]' is a list of constraints watching 'lit' (will go there if literal becomes true).
  vec<Clause*>        clauses;  // List of problem clauses.

};

//inline lbool    ClauseTable::value         (Var x) const   { return toLbool(assigns[x]); }
inline lbool    ClauseTable::value         (Lit p) const   { sign(p) ? -assigns[var(p)] : assigns[var(p)]; }

