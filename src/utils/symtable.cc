#include "ite.h"
#include "symtable.h"


/* The symbol table: a chain of `struct symrec'.  */
symrec **sym_table = NULL;
symrec *symtmp_table = NULL;
symrec *sym_hash_table=NULL;

#define SYM_HASH_SIZE 5000
#define SYMTMP_TABLE_SIZE 5000
#define SYM_TABLE_SIZE 5000

int sym_table_idx = 0;
int sym_table_max = SYM_TABLE_SIZE;
int symtmp_table_idx = 0;
int symtmp_table_max = SYMTMP_TABLE_SIZE;

void
sym_init()
{
  sym_hash_table = (symrec*)ite_calloc(SYM_HASH_SIZE, sizeof(symrec), 9, "sym_hash_table");
  symtmp_table = (symrec*)ite_calloc(SYMTMP_TABLE_SIZE, sizeof(symrec), 9, "symtmp_table");
  sym_table = (symrec**)ite_calloc(SYM_TABLE_SIZE, sizeof(symrec*), 9, "sym_table");
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
  if (sym_table_idx >= vars_max) 
      vars_alloc(sym_table_idx+100);
   
  ptr->id   = ++sym_table_idx;
  //ptr->indep = 1;
  ptr->true_false.var_ptr = ptr;
  ptr->true_false.variable = ptr->id;
  ptr->true_false.thenCase = true_ptr;
  ptr->true_false.elseCase = false_ptr;
  ptr->false_true.var_ptr = ptr;
  ptr->false_true.variable = ptr->id;
  ptr->false_true.thenCase = false_ptr;
  ptr->false_true.elseCase = true_ptr;
  if (sym_table_idx >= sym_table_max) {
     sym_table = (symrec**)ite_recalloc((void*)sym_table, sym_table_max, 
           sym_table_max+SYM_TABLE_SIZE, sizeof(symrec*), 9, "sym_table_idx");
     sym_table_max += SYM_TABLE_SIZE;
  }
  sym_table[sym_table_idx] = ptr;
}

symrec *
putsym (char *sym_name)
{
  symrec *ptr;
  symrec *tmp_sym_table = sym_hash(sym_name);
  ptr = (symrec *)ite_calloc (1, sizeof (symrec), 9, "symrec");
  ptr->name = (char *)ite_calloc (1, strlen (sym_name) + 1, 9, "symrec name");
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
  if (sym_table == NULL) return NULL;
  if (id > sym_table_idx) return NULL;
  return sym_table[id];
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

void   
s_set_indep(symrec *ptr, int i)
{
   independantVars[ptr->id] = i;
   //ptr->indep = i;
}

int   
s_is_indep(symrec *ptr)
{
   return independantVars[ptr->id];
   //ptr->indep = i;
}


symrec *
tputsym ()
{
  if (symtmp_table_idx >= symtmp_table_max) {
     /* cannot reallocate  -- pointers exist to this pool */
     symtmp_table = (symrec*)ite_calloc(SYMTMP_TABLE_SIZE, sizeof(symrec), 9, "symtmp_table");
     symtmp_table_idx = 0;
     /*
     assert(0);
     symtmp_table = (symrec*)ite_recalloc((void*)symtmp_table, symtmp_table_max, 
           symtmp_table_max+SYMTMP_TABLE_SIZE, sizeof(symrec), 9, "symtmp_table_idx");
     symtmp_table_max += SYMTMP_TABLE_SIZE;
     */
  }
  symrec *ptr = &(symtmp_table[symtmp_table_idx]);
  fill_symrec(ptr);
  s_set_indep(ptr, 2);
  symtmp_table_idx++;
  return ptr;
}

void
print_symtable()
{
  for(int i=0;i<sym_table_idx;i++) {
     symrec *ptr = getsym_i(i);
     if (ptr)
       printf("%d \"%s\" %d %d\n", i, (ptr->name?ptr->name:""), s_is_indep(ptr), independantVars[i]);
  } 

}
