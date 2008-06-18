/* =========FOR INTERNAL USE ONLY. NO DISTRIBUTION PLEASE ========== */

/*********************************************************************
 Copyright 1999-2007, University of Cincinnati.  All rights reserved.
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
 any trademark, service mark, or the name of University of Cincinnati.


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

#include "sbsat.h"
#include "symtable.h"


/* The symbol table: a chain of `struct symrec'.  */
symrec **sym_table = NULL;
symrec *symtmp_table = NULL;
symrec *sym_hash_table=NULL;

#define SYM_HASH_SIZE 5000
#define SYMTMP_TABLE_SIZE 5000
#define SYM_TABLE_SIZE 5000

int sym_table_idx = 0;
int sym_table_max = 0;
int symtmp_table_idx = 0;
int symtmp_table_max = 0;
int sym_all_int_flag = 1;

symrec * tputsym_truefalse(int sym_type);
void fill_symrec_with_id(symrec *ptr, int sym_type, int id);
void free_symrec_name_pool_chain();
void free_symrec_rec_pool_chain();
void free_symtmp_table();
void symrec_rec_pool_alloc();

void
sym_init()
{
  sym_hash_table = (symrec*)ite_calloc(SYM_HASH_SIZE, sizeof(symrec), 9, "sym_hash_table");
  sym_table_max = SYM_TABLE_SIZE;
  sym_table = (symrec**)ite_calloc(SYM_TABLE_SIZE, sizeof(symrec*), 9, "sym_table");
  true_ptr->var_ptr=tputsym_truefalse(SYM_VAR);
  false_ptr->var_ptr=tputsym_truefalse(SYM_VAR);
  ((symrec*)(true_ptr->var_ptr))->id=0;
  ((symrec*)(false_ptr->var_ptr))->id=0;
  sym_table_idx = 0;
  symrec_rec_pool_alloc();
}

void
sym_free()
{
   ite_free((void**)&sym_hash_table);
   ite_free((void**)&sym_table);
   free_symrec_name_pool_chain();
   free_symrec_rec_pool_chain();
   free_symtmp_table();
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

/* ------------------- name rec pool ------------------- */

char *symrec_name_pool = NULL;
int symrec_name_pool_max = 0;
int symrec_name_pool_index = 0;
typedef struct _symrec_name_pool_chain_type {
   char *pool;
   struct _symrec_name_pool_chain_type * next;
} symrec_name_pool_chain_type;
symrec_name_pool_chain_type * symrec_name_pool_chain_head=NULL;
symrec_name_pool_chain_type * symrec_name_pool_chain=NULL;

void
free_symrec_name_pool_chain()
{
   symrec_name_pool_chain_type * next = symrec_name_pool_chain_head;
   while(next) {
      symrec_name_pool_chain_type *current = next;
      next = next->next;
      free(current->pool);
      free(current);
   }
   symrec_name_pool_chain_head = NULL;
   symrec_name_pool_chain = NULL;
   symrec_name_pool_max = 0;
   symrec_name_pool_index = 0;
}

void
symrec_name_pool_alloc(int min)
{
   symrec_name_pool_chain_type *tmp_symrec_name_pool_chain = 
      (symrec_name_pool_chain_type*)ite_calloc(1, sizeof(symrec_name_pool_chain_type), 9, "symrec_name_pool_chain");
   if (symrec_name_pool_chain == NULL) {
      symrec_name_pool_chain_head = symrec_name_pool_chain = tmp_symrec_name_pool_chain;
   } else {
      symrec_name_pool_chain->next = tmp_symrec_name_pool_chain;
      symrec_name_pool_chain = tmp_symrec_name_pool_chain;
   }
   symrec_name_pool_max = 100000;
   if (symrec_name_pool_max < min) symrec_name_pool_max = min;
   symrec_name_pool_chain->pool = (char*)ite_calloc(symrec_name_pool_max, sizeof(char), 9, "symrec_name_pool");
   symrec_name_pool = symrec_name_pool_chain->pool;
   symrec_name_pool_index = 0;
}

char *
symrec_name_alloc(char *name)
{
   int len = strlen(name)+1;
   if (symrec_name_pool_index + len >= symrec_name_pool_max) {
      symrec_name_pool_alloc(len);
   }
   strcpy(symrec_name_pool + symrec_name_pool_index, name);
   symrec_name_pool_index += len;
   return (symrec_name_pool + symrec_name_pool_index - len);
}

/* ------------------- sym rec pool ------------------ */

symrec *symrec_rec_pool = NULL;
int symrec_rec_pool_max = 0;
int symrec_rec_pool_index = 0;
typedef struct _symrec_rec_pool_chain_type {
   symrec *pool;
   struct _symrec_rec_pool_chain_type * next;
} symrec_rec_pool_chain_type;
symrec_rec_pool_chain_type * symrec_rec_pool_chain_head=NULL;
symrec_rec_pool_chain_type * symrec_rec_pool_chain=NULL;

void
free_symrec_rec_pool_chain()
{
   symrec_rec_pool_chain_type * next = symrec_rec_pool_chain_head;
   while(next) {
      symrec_rec_pool_chain_type *current = next;
      next = next->next;
      free(current->pool);
      free(current);
   }
   symrec_rec_pool_chain_head = NULL;
   symrec_rec_pool_chain = NULL;
   symrec_rec_pool_max = 0;
   symrec_rec_pool_index = 0;
}

void
symrec_rec_pool_alloc()
{
   symrec_rec_pool_chain_type *tmp_symrec_rec_pool_chain = 
      (symrec_rec_pool_chain_type*)ite_calloc(1, sizeof(symrec_rec_pool_chain_type), 9, "symrec_rec_pool_chain");
   if (symrec_rec_pool_chain == NULL) {
      symrec_rec_pool_chain_head = symrec_rec_pool_chain = tmp_symrec_rec_pool_chain;
   } else {
      symrec_rec_pool_chain->next = tmp_symrec_rec_pool_chain;
      symrec_rec_pool_chain = tmp_symrec_rec_pool_chain;
   }
   symrec_rec_pool_max = 100000;
   symrec_rec_pool_chain->pool = (symrec*)ite_calloc(symrec_rec_pool_max, sizeof(symrec), 9, "symrec_rec_pool");
   symrec_rec_pool = symrec_rec_pool_chain->pool;
   symrec_rec_pool_index = 0;
}

symrec *
symrec_rec_alloc()
{
   if (symrec_rec_pool_index + 1 >= symrec_rec_pool_max) {
      symrec_rec_pool_alloc();
   }
   symrec_rec_pool_index += 1;
   return (symrec_rec_pool + symrec_rec_pool_index - 1);
}

/* ----------------------------------------------------------- */

symrec *
putsym_with_id(char *sym_name, int sym_type, int id)
{
  symrec *ptr;
  symrec *tmp_sym_table = sym_hash(sym_name);
  ptr = symrec_rec_alloc();
  ptr->name = symrec_name_alloc(sym_name);
  ptr->next = (struct symrec *)tmp_sym_table->next;
  tmp_sym_table->next = ptr;
  fill_symrec_with_id(ptr, sym_type, id);
  return ptr;
}


symrec *
putsym(char *sym_name, int sym_type)
{
  symrec *ptr;
  //{int id; if (sscanf(sym_name, "%d", &id)==1 && id!=0) return putsym_with_id(sym_name, sym_type, id); }
  symrec *tmp_sym_table = sym_hash(sym_name);
  ptr = symrec_rec_alloc();
  ptr->name = symrec_name_alloc(sym_name);
  if (sym_all_int_flag && sscanf(sym_name, "%d", &(ptr->name_int)) == 0) sym_all_int_flag = 0;
  ptr->next = (struct symrec *)tmp_sym_table->next;
  tmp_sym_table->next = ptr;
  fill_symrec(ptr, sym_type);
  return ptr;
}

void
create_all_syms(int sym_max_id)
{
   for(int i=1;i<=sym_max_id;i++)
   {
      if (i>sym_table_idx ||
            sym_table[i] == NULL) {
         char num[10];
         sprintf(num, "%d", i);
         putsym_with_id(num, SYM_VAR, i);
      }
   }
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
  assert(ptr->id != 0);
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

typedef struct _symtmp_table_pools_type {
   symrec *pool;
   struct _symtmp_table_pools_type * next;
} symtmp_table_pools_type;

symtmp_table_pools_type * symtmp_table_pools_head=NULL;
symtmp_table_pools_type * symtmp_table_pools=NULL;

void
symtmp_table_alloc()
{
   symtmp_table_pools_type * tmp_symtmp_table_pools = (symtmp_table_pools_type*)ite_calloc(1, sizeof(symtmp_table_pools_type), 9, "symtmp_table_pools");
   if (symtmp_table_pools_head == NULL) {
      symtmp_table_pools_head = 
         symtmp_table_pools = tmp_symtmp_table_pools;
   } else {
      symtmp_table_pools->next = tmp_symtmp_table_pools;
      symtmp_table_pools = symtmp_table_pools->next;
   }
   /* cannot reallocate  -- pointers exist to this pool */
   symtmp_table_max = SYMTMP_TABLE_SIZE;
   symtmp_table = (symrec*)ite_calloc(symtmp_table_max, sizeof(symrec), 9, "symtmp_table");
   symtmp_table_idx = 0;
   symtmp_table_pools->pool = symtmp_table;
}

void
free_symtmp_table()
{
   symtmp_table_pools_type * next = symtmp_table_pools_head;
   while (next) {
      symtmp_table_pools_type * current = next;
      next = next->next;
      ite_free((void**)&current->pool);
      ite_free((void**)&current);
   }
   symtmp_table_pools_head = NULL;
   symtmp_table_pools = NULL;
   symtmp_table_max = 0;
   symtmp_table_idx = 0;
}

symrec *
tputsym_truefalse(int sym_type)
{
   /* special temp var -- don't set their independance */
   /* e.g. for cnf that takes absolute ids the id 1 would be set to be temp indep */

  if (symtmp_table_idx >= symtmp_table_max) {
     symtmp_table_alloc();
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
     symtmp_table_alloc();
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
sym_regex_init(t_myregex *rg, char *sym_exp)
{
   assert(rg != NULL);
   memset(rg, 0, sizeof(t_myregex));
   rg->last_id = 1;
   return regcomp(&(rg->rg), sym_exp, REG_NOSUB+REG_EXTENDED+REG_ICASE);
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
_sym_is_flag(int id) 
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
   int i, max=0;
   for(i=1;i <= sym_table_idx;i++) {
      int num;
      if (sym_table[i] == NULL) continue;
      if (sym_table[i]->name == NULL) return 0;
      if (sscanf(sym_table[i]->name, "%d", &num)==0) return 0;
		if (max < atoi(sym_table[i]->name))
		  max = atoi(sym_table[i]->name);
   }
   return max;
}
