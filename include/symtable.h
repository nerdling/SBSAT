
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
  int sym_type;
  BDDNode true_false;
  BDDNode false_true;
};

typedef struct symrec symrec;

#define SYM_ANY   0
#define SYM_VAR   1
#define SYM_FN    2
#define SYM_OTHER 3

// flags
#define SYM_FLAG_UNRAVEL 0x10

/* The symbol table: a chain of `struct symrec'.     */
//extern symrec **sym_table;

void    sym_init();
symrec *putsym(char *, int);
symrec *getsym(char *);
int     i_getsym(char *, int);
int     i_getsym_int(int var_int, int sym_type);
int     i_putsym(char *, int);
symrec *s_getsym(char *, int);
void    s_set_indep(symrec *, int);
char   *s_name(int);
symrec *tputsym(int);
symrec *getsym_i(int id);
void    print_symtable();
int get_or_putsym_check(char *sym_name, int sym_type, int id);

int sym_is_flag(int id);
void sym_set_flag(int id);
void sym_reset_flag(int id); 


/* reg expressions */
typedef struct {
   int last_id;
   regex_t rg;
} t_myregex;
int sym_regex_init(t_myregex *rg, char *exp);
int sym_regex(t_myregex *rg);
int sym_regex_free(t_myregex *rg);

#endif
