
#include "ClauseTable.h"
#include "Sort.h"
#include <cmath>
#include <assert.h>


ClauseTable::ClauseTable(){
  //ok = true;
}

bool ClauseTable::addClause(vec<Lit>& ps)
{
 
  //if (!ok)
  //    return false;
    /* //I don't think we want to do this if we are adding clauses not only at decision level 0 -Sarah
    else{
        // Check if clause is satisfied and remove false/duplicate literals:
        sort(ps);
        Lit p; int i, j;
        for (i = j = 0, p = lit_Undef; i < ps.size(); i++)
            if (value(ps[i]) == l_True || ps[i] == ~p)
                return true;
            else if (value(ps[i]) != l_False && ps[i] != p)
                ps[j++] = p = ps[i];
        ps.shrink(i - j);
    }
    */

  if (ps.size() == 0)
    return 0; //ok = false;
  else if (ps.size() == 1){
    //Inference
    //EnqueueInference(var(ps[0]),toInt(lbool(!sign(ps[0])))); //?? !sign???
    //return 1;
  }
  else{
    Clause* c = Clause_new(ps, false);
    clauses.push(c);
    watches[toInt(~(*c)[0])].push(c);
    watches[toInt(~(*c)[1])].push(c);
  }

  return true;
}

// ApplyInference was  "propagate : [void]  ->  [Clause*]"

int ClauseTable::ApplyInferenceToClauseTable(Lit p) {

  //Clause* confl     = NULL;

  assert(value(p) == l_Undef);
  assigns [var(p)] = toInt(lbool(!sign(p)));  // !sign???

  vec<Clause*>&  ws  = watches[toInt(p)];
  Clause         **i, **j, **end;

  for (i = j = (Clause**)ws, end = i + ws.size();  i != end;){
    Clause& c = **i++;

    // Make sure the false literal is data[1]:
    Lit false_lit = ~p;
    if (c[0] == false_lit)
      c[0] = c[1], c[1] = false_lit;

    assert(c[1] == false_lit);

    // If 0th watch is true, then clause is already satisfied.
    Lit first = c[0];
    if (value(first) == l_True){
      *j++ = &c;
    }else{
      // Look for new watch:
      for (int k = 2; k < c.size(); k++)
	if (value(c[k]) != l_False){
	  c[1] = c[k]; c[k] = false_lit;
	  watches[toInt(~c[1])].push(&c);
	  goto FoundWatch; }
      
      // Did not find watch -- clause is unit under assignment:
      *j++ = &c;
      if (value(first) == l_False){
	return 0;
	//confl = &c;
	// Copy the remaining watches:
	while (i < end)
	  *j++ = *i++;
      }else{ //inference
	//EnqueueInference(var(first),toInt(lbool(!sign(p)))); //?? !sign???
      }
    }
  FoundWatch:;
  }
  ws.shrink(i - j);
  
  //return confl;
}



