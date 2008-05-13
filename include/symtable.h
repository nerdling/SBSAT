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

#ifndef SYMTABLE_H
#define SYMTABLE_H
/* Data type for links in the chain of symbols.      */
struct symrec
{
   char *name;  /* name of symbol          */
   int   name_int;
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
void    sym_free();
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
symrec *putsym_with_id(char *sym_name, int sym_type, int id);

int _sym_is_flag(int id);
extern symrec **sym_table;
#define sym_is_flag(id) (sym_table[id]->flag & SYM_FLAG_UNRAVEL)
void sym_set_flag(int id);
void sym_reset_flag(int id); 
void sym_clear_all_flag();
int sym_all_int();


/* reg expressions */
typedef struct {
   int last_id;
   regex_t rg;
} t_myregex;
int sym_regex_init(t_myregex *rg, char *exp);
int sym_regex(t_myregex *rg);
int sym_regex_free(t_myregex *rg);
void create_all_syms(int sym_max_id);

#endif
