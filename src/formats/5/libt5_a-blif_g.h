#ifndef BISON_Y_TAB_H
# define BISON_Y_TAB_H

#ifndef YYSTYPE
typedef union {
    int         num;      /* For returning numbers.               */
    char        id[200];  /* For returning ids.                   */
    t_op2fn     op2fn;    /* For returning op2fn                  */
    BDDNode     *bdd;     /* For returning exp                    */
} yystype;
# define YYSTYPE yystype
# define YYSTYPE_IS_TRIVIAL 1
#endif
# define	MODEL	257
# define	INPUTS	258
# define	OUTPUTS	259
# define	LATCH	260
# define	NAMES	261
# define	END	262
# define	ID	263
# define	ZEROONE	264
# define	TT	265
# define	TTONE	266


extern YYSTYPE blif_lval;

#endif /* not BISON_Y_TAB_H */
