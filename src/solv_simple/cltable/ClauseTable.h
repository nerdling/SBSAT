

#include <stdint.h>
//#include "sbsat.h"
//#include "sbsat_solver.h"
//#include "solver.h"

#include "Vec.h"
#include "Alg.h"

typedef int Var;
#define var_Undef (-1)

typedef int Lit;
inline  bool sign(Lit p) { return p & 1; }
inline  int  var(Lit p) { return p >> 1; }

const Lit lit_Undef = var_Undef + var_Undef;      // }- Useful special constants.
const Lit lit_Error = var_Undef + var_Undef + 1;  // }


//=================================================================================================
// Lifted booleans: (3 valued logic)

//typedef char lbool;

const char l_True  =  1;
const char l_False = -1;
const char l_Undef =  0;




//=================================================================================================
// Clause -- a simple class for representing a clause:

typedef vec<Lit> Clause;
int addClause(Clause* ps);  // Add a clause to the solver. 
char value (Lit p);       // The current value of a literal.
int ApplyInferenceToClauseTable(Lit p);




