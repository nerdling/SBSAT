#include "ite.h"
#include "symtable.h"


/* The symbol table: a chain of `struct symrec'.  */
symrec *sym_table = NULL;
symrec **sym_table_idx = NULL;
symrec *sym_table_tmp = NULL;
symrec *sym_hash_table=NULL;

int sym_table_index = 0;

#define SYM_HASH_SIZE 5000
#define SYM_TMP_SIZE 500000

void
sym_init()
{
  sym_hash_table = (symrec*)calloc(SYM_HASH_SIZE, sizeof(symrec));
  sym_table_tmp = (symrec*)calloc(SYM_TMP_SIZE, sizeof(symrec));
  sym_table_idx = (symrec**)calloc(3*SYM_TMP_SIZE, sizeof(symrec*));
  true_ptr->var_ptr=tputsym();
  false_ptr->var_ptr=tputsym();
  ((symrec*)(true_ptr->var_ptr))->id=0;
  ((symrec*)(false_ptr->var_ptr))->id=0;
}

symrec *
sym_hash(char *name)
{
  int i=0;
  int hash=0;
  while (name[i] && i<10) {
    hash += name[i];
    i++;
  }
  hash = hash % SYM_HASH_SIZE;
  return sym_hash_table+hash;
}

int
get_or_putsym(char *sym_name) 
{
  int i = i_getsym(sym_name);
  if (i) return i;
  putsym(sym_name);
  return i_getsym(sym_name);
}

void
fill_symrec(symrec *ptr)
{
  ptr->id   = ++sym_table_index;
  ptr->true_false.var_ptr = ptr;
  ptr->true_false.variable = ptr->id;
  ptr->true_false.thenCase = true_ptr;
  ptr->true_false.elseCase = false_ptr;
  ptr->false_true.var_ptr = ptr;
  ptr->false_true.variable = ptr->id;
  ptr->false_true.thenCase = false_ptr;
  ptr->false_true.elseCase = true_ptr;
  sym_table_idx[sym_table_index] = ptr;
}

symrec *
putsym (char *sym_name)
{
  symrec *ptr;
  symrec *tmp_sym_table = sym_hash(sym_name);
  ptr = (symrec *) calloc (1, sizeof (symrec));
  ptr->name = (char *) calloc (1, strlen (sym_name) + 1);
  strcpy (ptr->name,sym_name);
  ptr->next = (struct symrec *)tmp_sym_table->next;
  tmp_sym_table->next = ptr;
  fill_symrec(ptr);
  return ptr;
}

symrec *
getsym (char *sym_name)
{
  symrec *ptr;
  for (ptr = sym_hash(sym_name)->next; ptr != (symrec *) 0;
       ptr = (symrec *)ptr->next)
    if (strcmp (ptr->name,sym_name) == 0)
      return ptr;
  return 0;
}

symrec *
getsym_i(int id)
{
  if (sym_table_idx == NULL) return NULL;
  if (id > sym_table_index) return NULL;
  return sym_table_idx[id];
}

int
i_getsym (char *sym_name)
{
  symrec *ptr = getsym(sym_name);
  if (ptr) return ptr->id;
  return putsym(sym_name)->id;
}

symrec *
s_getsym (char *sym_name)
{
  symrec *ptr = getsym(sym_name);
  if (ptr) return ptr;
  return putsym(sym_name);
}

int t_sym_max=0;

symrec *
tputsym ()
{
  if (t_sym_max >= SYM_TMP_SIZE) {
     assert(0);
     dE_printf1("Too many tmp variables\n");
     exit(1);
  }
  symrec *ptr = &(sym_table_tmp[t_sym_max++]);
  fill_symrec(ptr);
  independantVars[ptr->id] = 2;
  return ptr;
}

void
print_symtable()
{
  for(int i=0;i<sym_table_index;i++) {
     symrec *ptr = getsym_i(i);
     if (ptr)
       printf("%d \"%s\" %d %d\n", i, (ptr->name?ptr->name:""), ptr->indep, independantVars[i]);
  } 

}
