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
# define	INTNUMBER	257
# define	MODULE	258
# define	ENDMODULE	259
# define	ID	260
# define	INPUT	261
# define	OUTPUT	262
# define	ARE_EQUAL	263
# define	OP	264
# define	OP_ITE	265
# define	U_OP	266
# define	U_OP_NOT	267
# define	STRUCTURE	268
# define	OP_NEW_INT_LEAF	269
# define	TPRINT	270
# define	STRING	271
# define	C_OP	272


extern YYSTYPE trace_lval;

#endif /* not BISON_Y_TAB_H */
