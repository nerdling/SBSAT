
#include "ClauseTable.h"
#include "Sort.h"
#include <cmath>
#include <assert.h>
#include <stdio.h>

vec<char>           assigns;          // The current assignments 
vec<vec<Clause*> >  watches;          // 'watches[lit]' is a list of constraints watching 'lit' (will go there if literal becomes true).
vec<Clause*>        clauses;          // List of problem clauses.
vec<Clause*>        learnts;          // List of learnt clauses.
vec<double>         activity;         // A heuristic measurement of the activity of a variable.
double              var_inc;          // Amount to bump next variable with.
double    var_decay;                  // Inverse of the variable activity decay factor.                                            
//Heap<VarOrderLt>    order_heap(VarOrderLt(activity)); // A priority queue of variables ordered with respect to the variable activity.

int main(){
  //ClauseTable table;
  Clause c;
  Lit x = 2*1;
  Lit notx = x^1;
  Lit y = 2*2 + 1;
  Lit noty = y^1;
  Lit z = 2*3 + 1;
  Lit notz = z^1;
  c.push(x);
  c.push(y);
  c.push(z);
  addClause(&c);

  ApplyInferenceToClauseTable(notx);
  ApplyInferenceToClauseTable(noty);

  //ApplyInferenceToClauseTable(noty);
  //ApplyInferenceToClauseTable(notx);


}

void initializeTable(){
  var_inc = 1.0;
  var_decay = 1.0 / 0.95;
}

int addClause(Clause* ps)
{
 
  for(int i=0;i<ps->size();i++){
    Var v = var((*ps)[i]);
    while(v >= assigns.size()){   
      watches.push();          // (list for positive literal)
      watches.push();          // (list for negative literal)

      assigns.push(l_Undef);
      activity.push(0);

      //polarity    .push((char)sign);
      //if (!order_heap.inHeap(var) && decision_var[x]) order_heap.insert(x);       
    }
  }  

  if (ps->size() == 0)
    return 0; 
  else if (ps->size() == 1){
    //Inference
    //EnqueueInference(var(ps[0]),sign(ps[0])); 
    //return 1;
  }
  else{
    clauses.push(ps);
    Lit w1 = ((*ps)[0])^1;
    Lit w2 = ((*ps)[1])^1;

    watches[w1].push(ps); 
    watches[w2].push(ps); 
  }

  return 1;
}

// ApplyInference was  "propagate : [void]  ->  [Clause*]"

int ApplyInferenceToClauseTable(Lit p) {

  //Clause* confl     = NULL;

  assert(value(p) == l_Undef);
  assigns[var(p)] = sign(p)?l_True:l_False; 

  if(watches.size() >= p){
    vec<Clause*>&  ws  = watches[p]; //TODO: check size of watches 
    Clause         **i, **j, **end;

    for (i = j = (Clause**)ws, end = i + ws.size();  i != end;){
      Clause& c = **i++;

      // Make sure the false literal is c[1]
      Lit false_lit = p^1;
      if (c[0] == false_lit)
	c[0] = c[1], c[1] = false_lit;
      assert(c[1] == false_lit);

      // If 0th watch is true, then clause is already satisfied.
      Lit first = c[0];
      if (value(first) == l_True){
	*j++ = &c;
      }else{
	
	for (int k = 2; k < c.size(); k++){   	// Look for new watch:
	  if (value(c[k]) != l_False){
	    c[1] = c[k]; c[k] = false_lit;
	    Lit w = c[1]^1;
	    watches[w].push(&c);
	    goto FoundWatch; 
	  }
	}
	// Did not find watch -- clause is unit under assignment:
	*j++ = &c;
	if (value(first) == l_False){
	  //return 0;
	  //confl = &c;
	  // Copy the remaining watches:
	  while (i < end)
	    *j++ = *i++;
	}else{ //inference
	  printf("Inference!!!\n");
	  //EnqueueInference(var(first),sign(p)); 
	}
      }
    FoundWatch:;

    }
    ws.shrink(i - j);  //???

  //return confl;
  }
}

char value(Lit p) {
  if(assigns.size() < var(p)+1)
    return l_Undef;
  else
    return sign(p) ? assigns[var(p)] : -assigns[var(p)]; 
 
}

void varDecayActivity() { var_inc *= var_decay; }
void varBumpActivity(Var v) {  //how to increase size of activity vector (and all other vectors)
  int nVars = assigns.size();
  if ( (activity[v] += var_inc) > 1e100 ) {
    // Rescale:
    for (int i = 0; i < nVars; i++)
      activity[i] *= 1e-100;
    var_inc *= 1e-100; 
  }
  
  // Update order_heap with respect to new activity:
  //  if (order_heap.inHeap(v))
  //    order_heap.decrease(v); 
}


