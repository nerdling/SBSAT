#include <stdio.h>
#include "bddnode.h"

int *length;
BDDNode *xorFunctions;
int nmbrFunctions=0;
int numBuckets;
int sizeBuckets;
int memorySize;
BDDNode * memory=NULL;
BDDNode * false_ptr=NULL;
BDDNode * true_ptr =NULL;
BDDNode **functions;
int *functionType;
int *equalityVble;
int  DEBUG_LVL;
long int numinp, numout;

void bdd_init() {
  memorySize = (numBuckets + sizeBuckets);
  memory = (BDDNode*)calloc((1 << memorySize), sizeof(BDDNode));
  if (!memory) exit(1);

    if (false_ptr == NULL && true_ptr == NULL)
    {
      BDDNode * next = memory;
      false_ptr = next;
      false_ptr->variable = 0;
      false_ptr->thenCase = false_ptr;
      false_ptr->elseCase = false_ptr;
      next++;
      true_ptr = next;
      true_ptr->variable = 0;
      true_ptr->thenCase = true_ptr;
      true_ptr->elseCase = true_ptr;
    }
}

int
bdd_circuit_init(int n_vars, int n_out)
{
  bdd_init();
  functions = (BDDNode**)calloc(n_out, sizeof(BDDNode*));
  functionType = (int *)calloc(n_out, sizeof(int));
  equalityVble = (int *)calloc(n_out, sizeof(int));
  return 0;
}

void
printBDD (BDDNode * bdd)
{
  return;
  if (!bdd) return;
  if (bdd == true_ptr)
    {
      fprintf (stdout, "T");
      return;
    }
  if (bdd == false_ptr)
    {
      fprintf (stdout, "F");
      return;
    }
  fprintf (stdout, "(");
  printBDD (bdd->thenCase);
  fprintf (stdout, "[%d]", bdd->variable);
  printBDD (bdd->elseCase);
  fprintf (stdout, ")");
  fprintf (stdout, "\n");
}


BDDNode * find_or_add_node (int v, BDDNode * r, BDDNode * e)
{
  long startx =
    ((v + (int) r + (int) e) & ((1 << numBuckets) -
                                1)) << sizeBuckets;
  BDDNode * curr = &memory[startx];
  BDDNode * last = &memory[(1 << memorySize) - 1];
  BDDNode * start = curr;
  //assert (curr >= circuit->memory);
  //assert (curr <= last);
  for (; curr <= last; curr++)
    {
      if (curr->variable == 0)
        {
          curr->variable = v;
          curr->thenCase = r;
          curr->elseCase = e;
          return curr;
        }
      if ((curr->variable == v) && (curr->thenCase == r)
           && (curr->elseCase == e))
        return curr;
    }
  for (curr = &memory[0]; curr < start; curr++)
    {
      if (curr->variable == 0)
        {
          curr->variable = v;
          curr->thenCase = r;
          curr->elseCase = e;
          return curr;
        }
      if ((curr->variable == v) && (curr->thenCase == r)
           && (curr->elseCase == e))
        return curr;
    }
  fprintf (stderr, "\nPlease allocate more memory for BDD hash table\n");
  fprintf (stderr, "memorySize: %d\n", memorySize);
  exit (1);
}

BDDNode * ite (BDDNode * x, BDDNode * y, BDDNode * z)
{
  int v;
  symrec* s;
  BDDNode *r, *e; 
  if (x == true_ptr)  return y;
  if (x == false_ptr) return z;
  s = (symrec*)top_variable_s(x, y, z);
  v = s->id;
  r = ite (reduce_t(v, x), reduce_t(v, y), reduce_t(v, z));
  e = ite (reduce_f(v, x), reduce_f(v, y), reduce_f(v, z));
  return r == e ? r : find_or_add_node_s (v, s, r, e);
}

// c is not allowed to be false.
BDDNode * gcf (BDDNode * f, BDDNode * c)
{
  int v;
  BDDNode *r,*e;

  if (f == c)
    return true_ptr;
  if ((c == true_ptr) || (f == true_ptr) || (f == false_ptr))
    return f;

    // We know that f & c are both BDD's with top variables.
  v = c->variable;
  if ((f->variable) < v) v = f->variable;

    //v is the top variable of f & c.
    if (reduce_f (v, c) == false_ptr)
    return gcf (reduce_t (v, f), reduce_t (v, c));
  if (reduce_t (v, c) == false_ptr)
    return gcf (reduce_f (v, f), reduce_f (v, c));
  r = gcf (reduce_t (v, f), reduce_t (v, c));
  e = gcf (reduce_f (v, f), reduce_f (v, c));
  if (r == e)
    return r;
  return find_or_add_node (v, r, e);
}
