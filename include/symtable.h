
#ifndef SYMTABLE_H
#define SYMTABLE_H
/* Data type for links in the chain of symbols.      */
struct symrec
{
  char *name;             /* name of symbol          */
  struct symrec *next;    /* link field              */
  int id;
  int flag;
  int indep;
  BDDNode true_false;
  BDDNode false_true;
};

typedef struct symrec symrec;

/* The symbol table: a chain of `struct symrec'.     */
extern symrec *sym_table;

void    sym_init();
symrec *putsym(char *);
symrec *getsym(char *);
int    i_getsym(char *);
symrec *s_getsym(char *);
symrec *tputsym();
symrec *getsym_i(int id);
void   print_symtable();
#endif
