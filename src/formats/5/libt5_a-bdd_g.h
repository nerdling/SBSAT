#ifndef BISON_Y_TAB_H
# define BISON_Y_TAB_H

#ifndef YYSTYPE
typedef union {
    int  num;      /* For returning numbers.               */
    char id[200];  /* For returning ids.                   */
} yystype;
# define YYSTYPE yystype
# define YYSTYPE_IS_TRIVIAL 1
#endif
# define	INTNUMBER	257
# define	ID	258
# define	S_OP	259
# define	OP	260
# define	OP_ITE	261
# define	U_OP	262
# define	BDDID	263
# define	P_BDD	264
# define	ADD_STATE	265
# define	STRING	266
# define	C_OP	267


extern YYSTYPE bdd_lval;

#endif /* not BISON_Y_TAB_H */
