/* A Bison parser, made from /home/mkouril/tmp/sbsat-2.0b-5/src/formats/5/trace_g.yy
   by GNU bison 1.35.  */

#define YYBISON 1  /* Identify Bison output.  */

#define yyparse trace_parse
#define yylex trace_lex
#define yyerror trace_error
#define yylval trace_lval
#define yychar trace_char
#define yydebug trace_debug
#define yynerrs trace_nerrs
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

#line 2 "trace_g.yy"

#include "ite.h"
#include "bddnode.h"
#include "symtable.h"
#include "functions.h"

   int trace_lex();
   void trace_error(const char *);
   BDDNode *ite_op(proc_op2fn fn, int *);
   void     ite_op_id_equ(char *var, BDDNode *bdd);
   void     ite_op_equ(char *var, t_op2fn fn, BDDNode **);
   BDDNode *ite_op_exp(t_op2fn fn, BDDNode **);
   void     ite_op_are_equal(BDDNode **);
   void     ite_new_int_leaf(char *, char *);
   void     ite_flag_vars(symrec **, int);

   /* FIXME: make it more dynamic! */
   symrec *varlist[1000];
   int varindex;

   /* FIXME: make it more dynamic! */
   BDDNode *explist[10][1000];
   int expindex[10];
   int explevel;


   int lines=0;
   extern int normal_bdds;
   extern int spec_fn_bdds;
   extern int t_sym_max;

#ifndef __attribute__
#define __attribute__(x)
#endif


#line 39 "trace_g.yy"
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
#ifndef YYDEBUG
# define YYDEBUG 0
#endif



#define	YYFINAL		74
#define	YYFLAG		-32768
#define	YYNTBASE	24

/* YYTRANSLATE(YYLEX) -- Bison token number corresponding to YYLEX. */
#define YYTRANSLATE(x) ((unsigned)(x) <= 272 ? yytranslate[x] : 36)

/* YYTRANSLATE[YYLEX] -- Bison token number corresponding to YYLEX. */
static const char yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
      21,    22,     2,     2,    23,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,    19,
       2,    20,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     3,     4,     5,
       6,     7,     8,     9,    10,    11,    12,    13,    14,    15,
      16,    17,    18
};

#if YYDEBUG
static const short yyprhs[] =
{
       0,     0,     1,     8,    11,    13,    14,    18,    21,    22,
      26,    29,    32,    33,    37,    44,    48,    55,    60,    65,
      70,    72,    76,    78,    80,    84,    86,    95,   100,   105
};
static const short yyrhs[] =
{
      -1,    24,    25,    27,    28,    29,    26,     0,     4,     6,
       0,     5,     0,     0,     7,    32,    19,     0,     7,    19,
       0,     0,     8,    32,    19,     0,     8,    19,     0,    14,
      30,     0,     0,    30,    31,    19,     0,     6,    20,    10,
      21,    34,    22,     0,     6,    20,    33,     0,     6,    20,
      15,    21,     6,    22,     0,     9,    21,    34,    22,     0,
      18,    21,     3,    22,     0,    16,    21,    17,    22,     0,
       6,     0,    32,    23,     6,     0,    35,     0,    35,     0,
      34,    23,    35,     0,     6,     0,    11,    21,    33,    23,
      33,    23,    33,    22,     0,    10,    21,    34,    22,     0,
      13,    21,     6,    22,     0,    12,    21,     6,    22,     0
};

#endif

#if YYDEBUG
/* YYRLINE[YYN] -- source line where rule number YYN was defined. */
static const short yyrline[] =
{
       0,    56,    57,    60,    63,    67,    68,    70,    73,    74,
      76,    79,    82,    83,    87,    89,    91,    93,    95,    97,
     101,   103,   107,   111,   113,   116,   118,   120,   122,   124
};
#endif


#if (YYDEBUG) || defined YYERROR_VERBOSE

/* YYTNAME[TOKEN_NUM] -- String name of the token TOKEN_NUM. */
static const char *const yytname[] =
{
  "$", "error", "$undefined.", "INTNUMBER", "MODULE", "ENDMODULE", "ID", 
  "INPUT", "OUTPUT", "ARE_EQUAL", "OP", "OP_ITE", "U_OP", "U_OP_NOT", 
  "STRUCTURE", "OP_NEW_INT_LEAF", "TPRINT", "STRING", "C_OP", "';'", 
  "'='", "'('", "')'", "','", "module_input", "module", "endmodule", 
  "input", "output", "structure", "lines", "line", "varlist", "exp_start", 
  "exp_list", "exp", 0
};
#endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives. */
static const short yyr1[] =
{
       0,    24,    24,    25,    26,    27,    27,    27,    28,    28,
      28,    29,    30,    30,    31,    31,    31,    31,    31,    31,
      32,    32,    33,    34,    34,    35,    35,    35,    35,    35
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN. */
static const short yyr2[] =
{
       0,     0,     6,     2,     1,     0,     3,     2,     0,     3,
       2,     2,     0,     3,     6,     3,     6,     4,     4,     4,
       1,     3,     1,     1,     3,     1,     8,     4,     4,     4
};

/* YYDEFACT[S] -- default rule to reduce with in state S when YYTABLE
   doesn't specify something else to do.  Zero means the default is an
   error. */
static const short yydefact[] =
{
       1,     0,     0,     5,     3,     0,     8,    20,     7,     0,
       0,     0,     6,     0,    10,     0,    12,     0,    21,     9,
      11,     4,     2,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    13,    25,     0,     0,     0,     0,     0,    15,
      22,     0,     0,    23,     0,     0,     0,     0,     0,     0,
       0,     0,    17,     0,    19,    18,     0,     0,     0,     0,
       0,     0,    24,    14,     0,    29,    28,    16,    27,     0,
       0,     0,    26,     0,     0
};

static const short yydefgoto[] =
{
       1,     3,    22,     6,    11,    17,    20,    27,     9,    39,
      42,    40
};

static const short yypact[] =
{
  -32768,     6,    -1,     0,-32768,    -5,     7,-32768,-32768,    20,
      -3,    12,-32768,    15,-32768,    21,-32768,    40,-32768,-32768,
       2,-32768,-32768,    26,    27,    28,    29,    32,    17,    25,
      30,    49,-32768,-32768,    33,    34,    35,    36,    37,-32768,
  -32768,    38,   -10,-32768,    31,    39,    25,    25,    54,    56,
      57,    25,-32768,    25,-32768,-32768,    11,    41,    43,    44,
      45,    19,-32768,-32768,    25,-32768,-32768,-32768,-32768,    46,
      25,    48,-32768,    68,-32768
};

static const short yypgoto[] =
{
  -32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,    61,   -45,
     -42,   -29
};


#define	YYLAST		71


static const short yytable[] =
{
      43,     7,    57,     7,    56,     4,    73,     5,    23,    61,
       2,    24,    52,    53,     8,    10,    14,    43,    25,    69,
      26,    18,    43,    33,    62,    71,    16,    34,    35,    36,
      37,    33,    38,    63,    53,    41,    35,    36,    37,    12,
      19,    68,    53,    13,    13,    21,    28,    44,    29,    30,
      31,    32,    45,    54,    46,    47,    48,    49,    50,    51,
      58,    55,    59,    60,    64,    65,    66,    67,    74,    70,
      72,    15
};

static const short yycheck[] =
{
      29,     6,    47,     6,    46,     6,     0,     7,     6,    51,
       4,     9,    22,    23,    19,     8,    19,    46,    16,    64,
      18,     6,    51,     6,    53,    70,    14,    10,    11,    12,
      13,     6,    15,    22,    23,    10,    11,    12,    13,    19,
      19,    22,    23,    23,    23,     5,    20,    17,    21,    21,
      21,    19,     3,    22,    21,    21,    21,    21,    21,    21,
       6,    22,     6,     6,    23,    22,    22,    22,     0,    23,
      22,    10
};
/* -*-C-*-  Note some compilers choke on comments on `#line' lines.  */
#line 3 "/usr/share/bison/bison.simple"

/* Skeleton output parser for bison,

   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002 Free Software
   Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

/* As a special exception, when this file is copied by Bison into a
   Bison output file, you may use that output file without restriction.
   This special exception was added by the Free Software Foundation
   in version 1.24 of Bison.  */

/* This is the parser code that is written into each bison parser when
   the %semantic_parser declaration is not specified in the grammar.
   It was written by Richard Stallman by simplifying the hairy parser
   used when %semantic_parser is specified.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

#if ! defined (yyoverflow) || defined (YYERROR_VERBOSE)

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# if YYSTACK_USE_ALLOCA
#  define YYSTACK_ALLOC alloca
# else
#  ifndef YYSTACK_USE_ALLOCA
#   if defined (alloca) || defined (_ALLOCA_H)
#    define YYSTACK_ALLOC alloca
#   else
#    ifdef __GNUC__
#     define YYSTACK_ALLOC __builtin_alloca
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning. */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
# else
#  if defined (__STDC__) || defined (__cplusplus)
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   define YYSIZE_T size_t
#  endif
#  define YYSTACK_ALLOC malloc
#  define YYSTACK_FREE free
# endif
#endif /* ! defined (yyoverflow) || defined (YYERROR_VERBOSE) */


#if (! defined (yyoverflow) \
     && (! defined (__cplusplus) \
	 || (YYLTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  short yyss;
  YYSTYPE yyvs;
# if YYLSP_NEEDED
  YYLTYPE yyls;
# endif
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAX (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# if YYLSP_NEEDED
#  define YYSTACK_BYTES(N) \
     ((N) * (sizeof (short) + sizeof (YYSTYPE) + sizeof (YYLTYPE))	\
      + 2 * YYSTACK_GAP_MAX)
# else
#  define YYSTACK_BYTES(N) \
     ((N) * (sizeof (short) + sizeof (YYSTYPE))				\
      + YYSTACK_GAP_MAX)
# endif

/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if 1 < __GNUC__
#   define YYCOPY(To, From, Count) \
      __builtin_memcpy (To, From, (Count) * sizeof (*(From)))
#  else
#   define YYCOPY(To, From, Count)		\
      do					\
	{					\
	  register YYSIZE_T yyi;		\
	  for (yyi = 0; yyi < (Count); yyi++)	\
	    (To)[yyi] = (From)[yyi];		\
	}					\
      while (0)
#  endif
# endif

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack)					\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack, Stack, yysize);				\
	Stack = &yyptr->Stack;						\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAX;	\
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (0)

#endif


#if ! defined (YYSIZE_T) && defined (__SIZE_TYPE__)
# define YYSIZE_T __SIZE_TYPE__
#endif
#if ! defined (YYSIZE_T) && defined (size_t)
# define YYSIZE_T size_t
#endif
#if ! defined (YYSIZE_T)
# if defined (__STDC__) || defined (__cplusplus)
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# endif
#endif
#if ! defined (YYSIZE_T)
# define YYSIZE_T unsigned int
#endif

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		-2
#define YYEOF		0
#define YYACCEPT	goto yyacceptlab
#define YYABORT 	goto yyabortlab
#define YYERROR		goto yyerrlab1
/* Like YYERROR except do call yyerror.  This remains here temporarily
   to ease the transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */
#define YYFAIL		goto yyerrlab
#define YYRECOVERING()  (!!yyerrstatus)
#define YYBACKUP(Token, Value)					\
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    {								\
      yychar = (Token);						\
      yylval = (Value);						\
      yychar1 = YYTRANSLATE (yychar);				\
      YYPOPSTACK;						\
      goto yybackup;						\
    }								\
  else								\
    { 								\
      yyerror ("syntax error: cannot back up");			\
      YYERROR;							\
    }								\
while (0)

#define YYTERROR	1
#define YYERRCODE	256


/* YYLLOC_DEFAULT -- Compute the default location (before the actions
   are run).

   When YYLLOC_DEFAULT is run, CURRENT is set the location of the
   first token.  By default, to implement support for ranges, extend
   its range to the last symbol.  */

#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)       	\
   Current.last_line   = Rhs[N].last_line;	\
   Current.last_column = Rhs[N].last_column;
#endif


/* YYLEX -- calling `yylex' with the right arguments.  */

#if YYPURE
# if YYLSP_NEEDED
#  ifdef YYLEX_PARAM
#   define YYLEX		yylex (&yylval, &yylloc, YYLEX_PARAM)
#  else
#   define YYLEX		yylex (&yylval, &yylloc)
#  endif
# else /* !YYLSP_NEEDED */
#  ifdef YYLEX_PARAM
#   define YYLEX		yylex (&yylval, YYLEX_PARAM)
#  else
#   define YYLEX		yylex (&yylval)
#  endif
# endif /* !YYLSP_NEEDED */
#else /* !YYPURE */
# define YYLEX			yylex ()
#endif /* !YYPURE */


/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)			\
do {						\
  if (yydebug)					\
    YYFPRINTF Args;				\
} while (0)
/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
#endif /* !YYDEBUG */

/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef	YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   SIZE_MAX < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#if YYMAXDEPTH == 0
# undef YYMAXDEPTH
#endif

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif

#ifdef YYERROR_VERBOSE

# ifndef yystrlen
#  if defined (__GLIBC__) && defined (_STRING_H)
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
static YYSIZE_T
#   if defined (__STDC__) || defined (__cplusplus)
yystrlen (const char *yystr)
#   else
yystrlen (yystr)
     const char *yystr;
#   endif
{
  register const char *yys = yystr;

  while (*yys++ != '\0')
    continue;

  return yys - yystr - 1;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined (__GLIBC__) && defined (_STRING_H) && defined (_GNU_SOURCE)
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
static char *
#   if defined (__STDC__) || defined (__cplusplus)
yystpcpy (char *yydest, const char *yysrc)
#   else
yystpcpy (yydest, yysrc)
     char *yydest;
     const char *yysrc;
#   endif
{
  register char *yyd = yydest;
  register const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif
#endif

#line 315 "/usr/share/bison/bison.simple"


/* The user can define YYPARSE_PARAM as the name of an argument to be passed
   into yyparse.  The argument should have type void *.
   It should actually point to an object.
   Grammar actions can access the variable by casting it
   to the proper pointer type.  */

#ifdef YYPARSE_PARAM
# if defined (__STDC__) || defined (__cplusplus)
#  define YYPARSE_PARAM_ARG void *YYPARSE_PARAM
#  define YYPARSE_PARAM_DECL
# else
#  define YYPARSE_PARAM_ARG YYPARSE_PARAM
#  define YYPARSE_PARAM_DECL void *YYPARSE_PARAM;
# endif
#else /* !YYPARSE_PARAM */
# define YYPARSE_PARAM_ARG
# define YYPARSE_PARAM_DECL
#endif /* !YYPARSE_PARAM */

/* Prevent warning if -Wstrict-prototypes.  */
#ifdef __GNUC__
# ifdef YYPARSE_PARAM
int yyparse (void *);
# else
int yyparse (void);
# endif
#endif

/* YY_DECL_VARIABLES -- depending whether we use a pure parser,
   variables are global, or local to YYPARSE.  */

#define YY_DECL_NON_LSP_VARIABLES			\
/* The lookahead symbol.  */				\
int yychar;						\
							\
/* The semantic value of the lookahead symbol. */	\
YYSTYPE yylval;						\
							\
/* Number of parse errors so far.  */			\
int yynerrs;

#if YYLSP_NEEDED
# define YY_DECL_VARIABLES			\
YY_DECL_NON_LSP_VARIABLES			\
						\
/* Location data for the lookahead symbol.  */	\
YYLTYPE yylloc;
#else
# define YY_DECL_VARIABLES			\
YY_DECL_NON_LSP_VARIABLES
#endif


/* If nonreentrant, generate the variables here. */

#if !YYPURE
YY_DECL_VARIABLES
#endif  /* !YYPURE */

int
yyparse (YYPARSE_PARAM_ARG)
     YYPARSE_PARAM_DECL
{
  /* If reentrant, generate the variables here. */
#if YYPURE
  YY_DECL_VARIABLES
#endif  /* !YYPURE */

  register int yystate;
  register int yyn;
  int yyresult;
  /* Number of tokens to shift before error messages enabled.  */
  int yyerrstatus;
  /* Lookahead token as an internal (translated) token number.  */
  int yychar1 = 0;

  /* Three stacks and their tools:
     `yyss': related to states,
     `yyvs': related to semantic values,
     `yyls': related to locations.

     Refer to the stacks thru separate pointers, to allow yyoverflow
     to reallocate them elsewhere.  */

  /* The state stack. */
  short	yyssa[YYINITDEPTH];
  short *yyss = yyssa;
  register short *yyssp;

  /* The semantic value stack.  */
  YYSTYPE yyvsa[YYINITDEPTH];
  YYSTYPE *yyvs = yyvsa;
  register YYSTYPE *yyvsp;

#if YYLSP_NEEDED
  /* The location stack.  */
  YYLTYPE yylsa[YYINITDEPTH];
  YYLTYPE *yyls = yylsa;
  YYLTYPE *yylsp;
#endif

#if YYLSP_NEEDED
# define YYPOPSTACK   (yyvsp--, yyssp--, yylsp--)
#else
# define YYPOPSTACK   (yyvsp--, yyssp--)
#endif

  YYSIZE_T yystacksize = YYINITDEPTH;


  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;
#if YYLSP_NEEDED
  YYLTYPE yyloc;
#endif

  /* When reducing, the number of symbols on the RHS of the reduced
     rule. */
  int yylen;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY;		/* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */

  yyssp = yyss;
  yyvsp = yyvs;
#if YYLSP_NEEDED
  yylsp = yyls;
#endif
  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed. so pushing a state here evens the stacks.
     */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyssp >= yyss + yystacksize - 1)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
	/* Give user a chance to reallocate the stack. Use copies of
	   these so that the &'s don't force the real ones into
	   memory.  */
	YYSTYPE *yyvs1 = yyvs;
	short *yyss1 = yyss;

	/* Each stack pointer address is followed by the size of the
	   data in use in that stack, in bytes.  */
# if YYLSP_NEEDED
	YYLTYPE *yyls1 = yyls;
	/* This used to be a conditional around just the two extra args,
	   but that might be undefined if yyoverflow is a macro.  */
	yyoverflow ("parser stack overflow",
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),
		    &yyls1, yysize * sizeof (*yylsp),
		    &yystacksize);
	yyls = yyls1;
# else
	yyoverflow ("parser stack overflow",
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),
		    &yystacksize);
# endif
	yyss = yyss1;
	yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyoverflowlab;
# else
      /* Extend the stack our own way.  */
      if (yystacksize >= YYMAXDEPTH)
	goto yyoverflowlab;
      yystacksize *= 2;
      if (yystacksize > YYMAXDEPTH)
	yystacksize = YYMAXDEPTH;

      {
	short *yyss1 = yyss;
	union yyalloc *yyptr =
	  (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
	if (! yyptr)
	  goto yyoverflowlab;
	YYSTACK_RELOCATE (yyss);
	YYSTACK_RELOCATE (yyvs);
# if YYLSP_NEEDED
	YYSTACK_RELOCATE (yyls);
# endif
# undef YYSTACK_RELOCATE
	if (yyss1 != yyssa)
	  YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;
#if YYLSP_NEEDED
      yylsp = yyls + yysize - 1;
#endif

      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
		  (unsigned long int) yystacksize));

      if (yyssp >= yyss + yystacksize - 1)
	YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  goto yybackup;


/*-----------.
| yybackup.  |
`-----------*/
yybackup:

/* Do appropriate processing given the current state.  */
/* Read a lookahead token if we need one and don't already have one.  */
/* yyresume: */

  /* First try to decide what to do without reference to lookahead token.  */

  yyn = yypact[yystate];
  if (yyn == YYFLAG)
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* yychar is either YYEMPTY or YYEOF
     or a valid token in external form.  */

  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = YYLEX;
    }

  /* Convert token to internal form (in yychar1) for indexing tables with */

  if (yychar <= 0)		/* This means end of input. */
    {
      yychar1 = 0;
      yychar = YYEOF;		/* Don't call YYLEX any more */

      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yychar1 = YYTRANSLATE (yychar);

#if YYDEBUG
     /* We have to keep this `#if YYDEBUG', since we use variables
	which are defined only if `YYDEBUG' is set.  */
      if (yydebug)
	{
	  YYFPRINTF (stderr, "Next token is %d (%s",
		     yychar, yytname[yychar1]);
	  /* Give the individual parser a way to print the precise
	     meaning of a token, for further debugging info.  */
# ifdef YYPRINT
	  YYPRINT (stderr, yychar, yylval);
# endif
	  YYFPRINTF (stderr, ")\n");
	}
#endif
    }

  yyn += yychar1;
  if (yyn < 0 || yyn > YYLAST || yycheck[yyn] != yychar1)
    goto yydefault;

  yyn = yytable[yyn];

  /* yyn is what to do for this token type in this state.
     Negative => reduce, -yyn is rule number.
     Positive => shift, yyn is new state.
       New state is final state => don't bother to shift,
       just return success.
     0, or most negative number => error.  */

  if (yyn < 0)
    {
      if (yyn == YYFLAG)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }
  else if (yyn == 0)
    goto yyerrlab;

  if (yyn == YYFINAL)
    YYACCEPT;

  /* Shift the lookahead token.  */
  YYDPRINTF ((stderr, "Shifting token %d (%s), ",
	      yychar, yytname[yychar1]));

  /* Discard the token being shifted unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  *++yyvsp = yylval;
#if YYLSP_NEEDED
  *++yylsp = yylloc;
#endif

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  yystate = yyn;
  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     `$$ = $1'.

     Otherwise, the following line sets YYVAL to the semantic value of
     the lookahead token.  This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];

#if YYLSP_NEEDED
  /* Similarly for the default location.  Let the user run additional
     commands if for instance locations are ranges.  */
  yyloc = yylsp[1-yylen];
  YYLLOC_DEFAULT (yyloc, (yylsp - yylen), yylen);
#endif

#if YYDEBUG
  /* We have to keep this `#if YYDEBUG', since we use variables which
     are defined only if `YYDEBUG' is set.  */
  if (yydebug)
    {
      int yyi;

      YYFPRINTF (stderr, "Reducing via rule %d (line %d), ",
		 yyn, yyrline[yyn]);

      /* Print the symbols being reduced, and their result.  */
      for (yyi = yyprhs[yyn]; yyrhs[yyi] > 0; yyi++)
	YYFPRINTF (stderr, "%s ", yytname[yyrhs[yyi]]);
      YYFPRINTF (stderr, " -> %s\n", yytname[yyr1[yyn]]);
    }
#endif

  switch (yyn) {

case 4:
#line 64 "trace_g.yy"
{ /* print_symtable(); */ }
    break;
case 6:
#line 69 "trace_g.yy"
{ varlist[varindex] = NULL; ite_flag_vars(varlist, /*independent - 1*/1);  }
    break;
case 9:
#line 75 "trace_g.yy"
{ varlist[varindex] = NULL; ite_flag_vars(varlist, /*dependent - 0*/0);  }
    break;
case 13:
#line 84 "trace_g.yy"
{ if (((++lines) % 100) == 0) fprintf(stderr, "\r%d %d", lines, t_sym_max); }
    break;
case 14:
#line 88 "trace_g.yy"
{ explist[explevel][expindex[explevel]] = NULL; ite_op_equ(yyvsp[-5].id, yyvsp[-3].op2fn, explist[explevel]); explevel--; }
    break;
case 15:
#line 90 "trace_g.yy"
{ ite_op_id_equ(yyvsp[-2].id,yyvsp[0].bdd); }
    break;
case 16:
#line 92 "trace_g.yy"
{ ite_new_int_leaf(yyvsp[-5].id, yyvsp[-1].id); }
    break;
case 17:
#line 94 "trace_g.yy"
{ explist[explevel][expindex[explevel]] = NULL; ite_op_are_equal(explist[explevel]); explevel--; }
    break;
case 18:
#line 96 "trace_g.yy"
{ fprintf(stderr, "nonhandled are_c_op\n"); assert(0); }
    break;
case 19:
#line 98 "trace_g.yy"
{ printf("%s\n", yyvsp[-1].id); }
    break;
case 20:
#line 102 "trace_g.yy"
{ varindex=0; varlist[varindex++] = s_getsym(yyvsp[0].id); }
    break;
case 21:
#line 104 "trace_g.yy"
{ varlist[varindex++] = s_getsym(yyvsp[0].id); }
    break;
case 22:
#line 108 "trace_g.yy"
{ yyval.bdd = yyvsp[0].bdd; }
    break;
case 23:
#line 112 "trace_g.yy"
{ explevel++; expindex[explevel]=0; explist[explevel][expindex[explevel]++] = yyvsp[0].bdd; }
    break;
case 24:
#line 114 "trace_g.yy"
{ explist[explevel][expindex[explevel]++] = yyvsp[0].bdd; }
    break;
case 25:
#line 117 "trace_g.yy"
{ symrec *s=s_getsym(yyvsp[0].id); yyval.bdd = ite_vars(s); }
    break;
case 26:
#line 119 "trace_g.yy"
{ yyval.bdd = ite_s( yyvsp[-5].bdd, yyvsp[-3].bdd, yyvsp[-1].bdd); }
    break;
case 27:
#line 121 "trace_g.yy"
{ explist[explevel][expindex[explevel]] = NULL; yyval.bdd = ite_op_exp(yyvsp[-3].op2fn,explist[explevel]); explevel--; }
    break;
case 28:
#line 123 "trace_g.yy"
{ yyval.bdd = ite_not(ite_vars(s_getsym(yyvsp[-1].id))); }
    break;
case 29:
#line 125 "trace_g.yy"
{ fprintf(stderr, "nonhandled u_op\n"); yyval.bdd = NULL; }
    break;
}

#line 705 "/usr/share/bison/bison.simple"


  yyvsp -= yylen;
  yyssp -= yylen;
#if YYLSP_NEEDED
  yylsp -= yylen;
#endif

#if YYDEBUG
  if (yydebug)
    {
      short *yyssp1 = yyss - 1;
      YYFPRINTF (stderr, "state stack now");
      while (yyssp1 != yyssp)
	YYFPRINTF (stderr, " %d", *++yyssp1);
      YYFPRINTF (stderr, "\n");
    }
#endif

  *++yyvsp = yyval;
#if YYLSP_NEEDED
  *++yylsp = yyloc;
#endif

  /* Now `shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTBASE] + *yyssp;
  if (yystate >= 0 && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTBASE];

  goto yynewstate;


/*------------------------------------.
| yyerrlab -- here on detecting error |
`------------------------------------*/
yyerrlab:
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;

#ifdef YYERROR_VERBOSE
      yyn = yypact[yystate];

      if (yyn > YYFLAG && yyn < YYLAST)
	{
	  YYSIZE_T yysize = 0;
	  char *yymsg;
	  int yyx, yycount;

	  yycount = 0;
	  /* Start YYX at -YYN if negative to avoid negative indexes in
	     YYCHECK.  */
	  for (yyx = yyn < 0 ? -yyn : 0;
	       yyx < (int) (sizeof (yytname) / sizeof (char *)); yyx++)
	    if (yycheck[yyx + yyn] == yyx)
	      yysize += yystrlen (yytname[yyx]) + 15, yycount++;
	  yysize += yystrlen ("parse error, unexpected ") + 1;
	  yysize += yystrlen (yytname[YYTRANSLATE (yychar)]);
	  yymsg = (char *) YYSTACK_ALLOC (yysize);
	  if (yymsg != 0)
	    {
	      char *yyp = yystpcpy (yymsg, "parse error, unexpected ");
	      yyp = yystpcpy (yyp, yytname[YYTRANSLATE (yychar)]);

	      if (yycount < 5)
		{
		  yycount = 0;
		  for (yyx = yyn < 0 ? -yyn : 0;
		       yyx < (int) (sizeof (yytname) / sizeof (char *));
		       yyx++)
		    if (yycheck[yyx + yyn] == yyx)
		      {
			const char *yyq = ! yycount ? ", expecting " : " or ";
			yyp = yystpcpy (yyp, yyq);
			yyp = yystpcpy (yyp, yytname[yyx]);
			yycount++;
		      }
		}
	      yyerror (yymsg);
	      YYSTACK_FREE (yymsg);
	    }
	  else
	    yyerror ("parse error; also virtual memory exhausted");
	}
      else
#endif /* defined (YYERROR_VERBOSE) */
	yyerror ("parse error");
    }
  goto yyerrlab1;


/*--------------------------------------------------.
| yyerrlab1 -- error raised explicitly by an action |
`--------------------------------------------------*/
yyerrlab1:
  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
	 error, discard it.  */

      /* return failure if at end of input */
      if (yychar == YYEOF)
	YYABORT;
      YYDPRINTF ((stderr, "Discarding token %d (%s).\n",
		  yychar, yytname[yychar1]));
      yychar = YYEMPTY;
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */

  yyerrstatus = 3;		/* Each real token shifted decrements this */

  goto yyerrhandle;


/*-------------------------------------------------------------------.
| yyerrdefault -- current state does not do anything special for the |
| error token.                                                       |
`-------------------------------------------------------------------*/
yyerrdefault:
#if 0
  /* This is wrong; only states that explicitly want error tokens
     should shift them.  */

  /* If its default is to accept any token, ok.  Otherwise pop it.  */
  yyn = yydefact[yystate];
  if (yyn)
    goto yydefault;
#endif


/*---------------------------------------------------------------.
| yyerrpop -- pop the current state because it cannot handle the |
| error token                                                    |
`---------------------------------------------------------------*/
yyerrpop:
  if (yyssp == yyss)
    YYABORT;
  yyvsp--;
  yystate = *--yyssp;
#if YYLSP_NEEDED
  yylsp--;
#endif

#if YYDEBUG
  if (yydebug)
    {
      short *yyssp1 = yyss - 1;
      YYFPRINTF (stderr, "Error: state stack now");
      while (yyssp1 != yyssp)
	YYFPRINTF (stderr, " %d", *++yyssp1);
      YYFPRINTF (stderr, "\n");
    }
#endif

/*--------------.
| yyerrhandle.  |
`--------------*/
yyerrhandle:
  yyn = yypact[yystate];
  if (yyn == YYFLAG)
    goto yyerrdefault;

  yyn += YYTERROR;
  if (yyn < 0 || yyn > YYLAST || yycheck[yyn] != YYTERROR)
    goto yyerrdefault;

  yyn = yytable[yyn];
  if (yyn < 0)
    {
      if (yyn == YYFLAG)
	goto yyerrpop;
      yyn = -yyn;
      goto yyreduce;
    }
  else if (yyn == 0)
    goto yyerrpop;

  if (yyn == YYFINAL)
    YYACCEPT;

  YYDPRINTF ((stderr, "Shifting error token, "));

  *++yyvsp = yylval;
#if YYLSP_NEEDED
  *++yylsp = yylloc;
#endif

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

/*---------------------------------------------.
| yyoverflowab -- parser overflow comes here.  |
`---------------------------------------------*/
yyoverflowlab:
  yyerror ("parser stack overflow");
  yyresult = 2;
  /* Fall through.  */

yyreturn:
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
  return yyresult;
}
#line 128 "trace_g.yy"


void
ite_op_id_equ(char *var, BDDNode *bdd)
{
   symrec  *s_ptr = s_getsym(var);
   independantVars[s_ptr->id] = 0;
   BDDNode *ptr = ite_equ(ite_vars(s_ptr), bdd);
   functions_add(ptr, UNSURE, s_ptr->id);
}

int ite_last_fn=0;

void
ite_op_equ(char *var, t_op2fn fn, BDDNode **explist)
{
   fn.fn_type = MAKE_EQU(fn.fn_type);
   symrec  *s_ptr = s_getsym(var);
   independantVars[s_ptr->id] = 0;
   BDDNode *ptr = ite_op_exp(fn, explist);
   ptr = ite_equ(ite_vars(s_ptr), ptr);
   functions_add(ptr, ite_last_fn, s_ptr->id);
}

int ite_op_flag=0;

/* recursive counter of variables and union of variables */
void
ite_op_check(int *new_vars, int *total_vars, BDDNode *bdd)
{
   if (bdd == true_ptr || bdd == false_ptr) return;
   if (((symrec*)(bdd->var_ptr))->flag!=ite_op_flag) {
      ((symrec*)(bdd->var_ptr))->flag=ite_op_flag;
      (*new_vars)++;
   }
   (*total_vars)++;
   ite_op_check(new_vars, total_vars, bdd->thenCase);
   ite_op_check(new_vars, total_vars, bdd->elseCase);
}

int
explist_sort(const void *x, const void *y)
{
   int x_var = (*((BDDNode **)x))->variable;
   int y_var = (*((BDDNode **)y))->variable;
   //return (x_var - y_var);
   return (y_var - x_var);
}

BDDNode *ite_op_exp(t_op2fn fn, BDDNode **explist)
{
   int i=0, num_members=0;
   BDDNode *ptr=NULL;
   assert(explist[0] != NULL);
   /* first check the bdds:
    1. if they all have just one var => special fn
    2. if they combined have more than X distict var => split it
    */
   ite_op_flag++; /* FIX: make sure we don't overflow... */
   int spec_fn=0;
   int total_vars = 0;
   int new_vars = 0;

   for (num_members=0;explist[num_members]!=NULL;num_members++) {
      ite_op_check(&new_vars, &total_vars, explist[num_members]);
      if (total_vars <= 1) total_vars = 0;
   }

   /* check if each bdd in the function have just one variable */
   if (total_vars == 0 && num_members>=2 /*limit[fn.fn_type]*/) {
      /* we have a special function */ 
      spec_fn=1;
   } else
      /* LOOK: this does not happen -- why ??? */
      /* that's why untested!!!                */
      /* check if they have more than a limit to get broken up */
      if (new_vars >= 5 /*s_limit*/ || num_members>=2 /*limit[fn.fn_type]*/) {
         printf("x");
         /* we have a special function 
          but we have to break it apart
          */
         spec_fn=1;
         for (i=0;explist[i]!=NULL;i++) {
            /* take explist[i] and ite_equ to a new temp var */
            ite_op_flag++; /* make sure we don't overflow... */
            int new_vars=0;
            int total_vars=0;
            ite_op_check(&new_vars, &total_vars, explist[i]);
            if (total_vars > 1) {
               symrec *s_ptr = tputsym();
               /* save explist[i] into another function */
               BDDNode *new_ptr = ite_equ(ite_vars(s_ptr), explist[i]);
               functions_add(new_ptr, UNSURE, s_ptr->id);

               /* replace explist[i] with ite_var of the temp var */
               explist[i] = ite_vars(s_ptr);
            }
         }
      }

   /* FIX: if the fn type is not ??imp -- sort the bdds by top_variable */
   qsort (explist, num_members, sizeof (BDDNode*), explist_sort);

   /* apply the function */
   for (i=0;explist[i]!=NULL;i++) {
      assert(explist[i]->var_ptr);
      if (ptr==NULL) ptr = explist[i];
      else ptr = fn.fn(explist[i], ptr);
   }
   assert(ptr != NULL);
   assert(ptr->var_ptr);
   /* if it is a special function separate it */
   /* into a different function               */
   /* _EQU functions are handled using the grammar */
   if (spec_fn && !IS_EQU(fn.fn_type)) {
      symrec *s_ptr = tputsym();
      ptr = ite_equ(ite_vars(s_ptr),ptr);
      functions_add(ptr, MAKE_EQU(fn.fn_type), s_ptr->id);

      ptr = ite_vars(s_ptr);
   };
   ite_last_fn = fn.fn_type;
   return ptr;
}

void
ite_op_are_equal(BDDNode **explist)
{
   t_op2fn fn;

   /* LOOK: make sure ite_op_exp does not change the explist much */
   /* ite_and positive */
   fn.fn=op_equ;
   fn.fn_type=EQU_EQU;
   BDDNode *ptr = ite_op_exp(fn, explist);

   functions_add(ptr, EQU, 0); /* the only plain or? */
}

void
ite_new_int_leaf(char *id, char *zero_one)
{
   symrec *s_ptr = s_getsym(id);
   independantVars[s_ptr->id] = 0;
   BDDNode *ptr = ite_vars(s_ptr);
   if (*zero_one == '0') ptr = ite_not(ptr);
   else if (*zero_one != '1') {
      fprintf(stderr, "new_int_leaf with non 0/1 argument (%s) -- assuming 1\n", zero_one);
   }
   functions_add(ptr, UNSURE, s_ptr->id);
}

void
ite_flag_vars(symrec **varlist, int indep)
{
  for (int i=0; varlist[i]!=NULL; i++) {
     varlist[i]->indep = indep;
     independantVars[varlist[i]->id] = indep;
  }
}

void
printAllFunctions()
{
   for(int i=0;i<nmbrFunctions;i++) 
   { 
      printf("eq: %d\n", equalityVble[i]); 
      printBDD(functions[i]); 
      printf("\n"); 
   } 
}
