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
int sym_all_int_flag = 1;

symrec * tputsym_truefalse(int sym_type);
void fill_symrec_with_id(symrec *ptr, int sym_type, int id);

void
sym_init()
{
  sym_hash_table = (symrec*)ite_calloc(SYM_HASH_SIZE, sizeof(symrec), 9, "sym_hash_table");
  symtmp_table = (symrec*)ite_calloc(SYMTMP_TABLE_SIZE, sizeof(symrec), 9, "symtmp_table");
  sym_table = (symrec**)ite_calloc(SYM_TABLE_SIZE, sizeof(symrec*), 9, "sym_table");
  true_ptr->var_ptr=tputsym_truefalse(SYM_VAR);
  false_ptr->var_ptr=tputsym_truefalse(SYM_VAR);
  ((symrec*)(true_ptr->var_ptr))->id=0;
  ((symrec*)(false_ptr->var_ptr))->id=0;
  sym_table_idx = 0;
}

void
sym_free()
{
   if (sym_table != NULL) {
      for (int i=0; i<= sym_table_idx; i++) {
         if (sym_table[i] && sym_table[i]->name) {
            ite_free((void**)&sym_table[i]->name);
            ite_free((void**)&sym_table[i]);
         }
      }
   }
   ite_free((void**)&sym_hash_table);
   ite_free((void**)&symtmp_table);
   ite_free((void**)&sym_table);
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
i_putsym(char *sym_name, int sym_type) 
{
  int i = i_getsym(sym_name, sym_type);
  if (i!=0) return i;
  putsym(sym_name, sym_type);
  return i_getsym(sym_name, sym_type);
}

int
get_or_putsym_check(char *sym_name, int sym_type, int id) 
{
   int i = i_putsym(sym_name, sym_type);
   assert(i==id);
   return i;
}

void
fill_symrec_with_id(symrec *ptr, int sym_type, int id)
{
   if (id >= vars_max) 
      vars_alloc(id+100);

   if (id >= sym_table_max) {
      sym_table = (symrec**)ite_recalloc((void*)sym_table, sym_table_max, 
            sym_table_max+SYM_TABLE_SIZE, sizeof(symrec*), 9, "sym_table_idx");
      sym_table_max += SYM_TABLE_SIZE;
   }
   if (id > sym_table_idx) sym_table_idx = id;

   ptr->id   = id;
   ptr->sym_type   = sym_type;
   sym_table[id] = ptr;

   //ptr->indep = 1;
   /* this is not necessary -- part of testing*/
   ptr->true_false.var_ptr = ptr;
   ptr->true_false.variable = ptr->id;
   ptr->true_false.thenCase = true_ptr;
   ptr->true_false.elseCase = false_ptr;
   ptr->false_true.var_ptr = ptr;
   ptr->false_true.variable = ptr->id;
   ptr->false_true.thenCase = false_ptr;
   ptr->false_true.elseCase = true_ptr;
   /* end of this... */
}

void
fill_symrec(symrec *ptr, int sym_type)
{
   ++sym_table_idx;
   fill_symrec_with_id(ptr, sym_type, sym_table_idx);
}


symrec *
putsym_with_id(char *sym_name, int sym_type, int id)
{
  symrec *ptr;
  symrec *tmp_sym_table = sym_hash(sym_name);
  ptr = (symrec *)ite_calloc (1, sizeof (symrec), 9, "symrec");
  ptr->name = (char *)ite_calloc (1, strlen (sym_name) + 1, 9, "symrec name");
  strcpy (ptr->name,sym_name);
  ptr->next = (struct symrec *)tmp_sym_table->next;
  tmp_sym_table->next = ptr;
  fill_symrec_with_id(ptr, sym_type, id);
  return ptr;
}


symrec *
putsym(char *sym_name, int sym_type)
{
  symrec *ptr;
  //{int id; if (sscanf(sym_name, "%d", &id)==1) return putsym_with_id(sym_name, sym_type, id); }
  symrec *tmp_sym_table = sym_hash(sym_name);
  ptr = (symrec *)ite_calloc (1, sizeof (symrec), 9, "symrec");
  ptr->name = (char *)ite_calloc (1, strlen (sym_name) + 1, 9, "symrec name");
  strcpy (ptr->name,sym_name);
  if (sym_all_int_flag && sscanf(sym_name, "%d", &(ptr->name_int)) == 0) sym_all_int_flag = 0;
  ptr->next = (struct symrec *)tmp_sym_table->next;
  tmp_sym_table->next = ptr;
  fill_symrec(ptr, sym_type);
  return ptr;
}

symrec *
getsym(char *sym_name)
{
  symrec *ptr;
  //int sym_name_int=0; if (sscanf(sym_name, "%d", &sym_name_int) == 0) sym_name_int = 0;
  for (ptr = sym_hash(sym_name)->next; ptr != (symrec *) 0;
       ptr = (symrec *)ptr->next)
  {
     /*
     if (sym_name_int > 0 && ptr->name_int > 0)
        if (sym_name_int == ptr->name_int) 
           return ptr;
        else
           continue;
           */
     if (strcasecmp(ptr->name, sym_name) == 0) {
        return ptr;
     }
  }
  return 0;
}

symrec *
getsym_i(int id)
{
  if (sym_table == NULL) return NULL;
  if (id > sym_table_idx) return NULL;
  /*  -- enable if vars are missing
   if (sym_table[id] == NULL) {
     char sym_name[32];
     sprintf(sym_name, "%d", id);
     if (getsym(sym_name) != NULL) { // bummer
         exit();
     }
     putsym_id(sym_name, SYM_VAR, id);
     assert(sym_table[id] != NULL);
  }
  */
  return sym_table[id];
}

char *
s_name(int id)
{
  if (sym_table == NULL) return NULL;
  if (id > sym_table_idx) return NULL;
  symrec *ptr = sym_table[id];
  if (ptr == NULL) return NULL;
  return ptr->name;
}


int
i_getsym (char *sym_name, int sym_type)
{
  symrec *ptr = getsym(sym_name);
  if (ptr) {
     if (ptr->sym_type != sym_type && sym_type != 0)
        return -ptr->id;
     else
        return ptr->id;
  }
  ptr = putsym(sym_name, sym_type);
  return ptr->id;
}

int
i_getsym_int(int var_int, int sym_type)
{
   char sym_name[64];
   int sign=var_int<0?-1:1;
   var_int *= sign; // abs value
   assert(var_int > 0);
   sprintf(sym_name, "%d", var_int);
   return sign*i_getsym(sym_name, sym_type);
}

symrec *
s_getsym(char *sym_name, int sym_type)
{
  symrec *ptr = getsym(sym_name);
  if (ptr) {
     if (ptr->sym_type != sym_type && sym_type != 0) 
        return NULL;
     else
        return ptr;
  } 
  ptr = putsym(sym_name, sym_type);
  return ptr;
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
tputsym_truefalse(int sym_type)
{
   /* special temp var -- don't set their independance */
   /* e.g. for cnf that takes absolute ids the id 1 would be set to be temp indep */

  if (symtmp_table_idx >= symtmp_table_max) {
     /* cannot reallocate  -- pointers exist to this pool */
     symtmp_table = (symrec*)ite_calloc(SYMTMP_TABLE_SIZE, sizeof(symrec), 9, "symtmp_table");
     symtmp_table_idx = 0;
  }
  symrec *ptr = &(symtmp_table[symtmp_table_idx]);
  fill_symrec(ptr, sym_type);
  symtmp_table_idx++;
  return ptr;
}

symrec *
tputsym(int sym_type)
{
  if (symtmp_table_idx >= symtmp_table_max) {
     /* cannot reallocate  -- pointers exist to this pool */
     symtmp_table = (symrec*)ite_calloc(SYMTMP_TABLE_SIZE, sizeof(symrec), 9, "symtmp_table");
     symtmp_table_idx = 0;
  }
  symrec *ptr = &(symtmp_table[symtmp_table_idx]);
  fill_symrec(ptr, sym_type);
  s_set_indep(ptr, 2);
  symtmp_table_idx++;
  return ptr;
}

void
print_symtable()
{
  for(int i=0;i <= sym_table_idx;i++) {
     symrec *ptr = getsym_i(i);
     if (ptr)
       printf("%d \"%s\" %d %d\n", i, (ptr->name?ptr->name:""), s_is_indep(ptr), independantVars[i]);
  } 
}

/* set myregex = NULL */
/* call sym_regex_init(&myregex, "expression") */
/* call sym_regex(&myregex) returns the first id or 0 */
/* call sym_regex_free(&myregex) */

int
sym_regex_init(t_myregex *rg, char *exp)
{
   assert(rg != NULL);
   memset(rg, 0, sizeof(t_myregex));
   rg->last_id = 1;
   return regcomp(&(rg->rg), exp, REG_NOSUB+REG_EXTENDED+REG_ICASE);
   /* REG_EXTENDED REG_ICASE REG_NEWLINE */
}

int
sym_regex(t_myregex *rg)
{
   assert(rg != NULL);
   for(;rg->last_id <= sym_table_idx; rg->last_id++)
   {
      symrec *ptr = getsym_i(rg->last_id);
      if (ptr && ptr->name &&
            regexec(&(rg->rg), ptr->name, 0, NULL, 0) == 0)
      {
         /* found one */
         rg->last_id++;
         return rg->last_id-1;
      }
   }
   return 0;
}

int
sym_regex_free(t_myregex *rg)
{
   assert(rg != NULL);
   regfree(&(rg->rg));
   memset(rg, 0, sizeof(t_myregex));
   return 0;
}

/* here is the test -- to use it call this from e.g. read_input */
void
sym_regex_test(char *reg)
{
   t_myregex myrg;
   sym_regex_init(&myrg, reg);
   int id = sym_regex(&myrg);
   while (id) {
      /* found variable and the variable id is id */
      fprintf(stderr, "%d %s\n", id, getsym_i(id)->name);
      id = sym_regex(&myrg);
   }
   sym_regex_free(&myrg);
   exit(1);
}

void
sym_reset_flag(int id) 
{
   assert(getsym_i(id)->flag & SYM_FLAG_UNRAVEL);
   getsym_i(id)->flag &= ~SYM_FLAG_UNRAVEL;
}

void
sym_set_flag(int id) 
{
   assert((getsym_i(id)->flag & SYM_FLAG_UNRAVEL) == 0);
   getsym_i(id)->flag |= SYM_FLAG_UNRAVEL;
}

int
sym_is_flag(int id) 
{
   return getsym_i(id)->flag & SYM_FLAG_UNRAVEL;
}

void
sym_clear_all_flag()
{
   int i;
   for(i=1;i <= sym_table_idx;i++)
      sym_table[i]->flag = 0;
}

int
sym_all_int()
{
   int i;
   for(i=1;i <= sym_table_idx;i++) {
      int num;
      if (sym_table[i] == NULL) continue;
      if (sym_table[i]->name == NULL) return 0;
      if (sscanf(sym_table[i]->name, "%d", &num)==0) return 0;
   }
   return 1;
}
