/* A Bison parser, made by GNU Bison 1.875.  */

/* Skeleton parser for Yacc-like parsing with Bison,
   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002 Free Software Foundation, Inc.

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

/* Written by Richard Stallman by simplifying the original so called
   ``semantic'' parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Using locations.  */
#define YYLSP_NEEDED 0

/* If NAME_PREFIX is specified substitute the variables and functions
   names.  */
#define yyparse trace_parse
#define yylex   trace_lex
#define yyerror trace_error
#define yylval  trace_lval
#define yychar  trace_char
#define yydebug trace_debug
#define yynerrs trace_nerrs


/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     INTNUMBER = 258,
     MODULE = 259,
     ENDMODULE = 260,
     ID = 261,
     INPUT = 262,
     OUTPUT = 263,
     ARE_EQUAL = 264,
     OP = 265,
     OP_ITE = 266,
     U_OP = 267,
     U_OP_NOT = 268,
     STRUCTURE = 269,
     OP_NEW_INT_LEAF = 270,
     TPRINT = 271,
     STRING = 272,
     C_OP = 273
   };
#endif
#define INTNUMBER 258
#define MODULE 259
#define ENDMODULE 260
#define ID 261
#define INPUT 262
#define OUTPUT 263
#define ARE_EQUAL 264
#define OP 265
#define OP_ITE 266
#define U_OP 267
#define U_OP_NOT 268
#define STRUCTURE 269
#define OP_NEW_INT_LEAF 270
#define TPRINT 271
#define STRING 272
#define C_OP 273




/* Copy the first part of user declarations.  */
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

#ifndef __attribute__
#define __attribute__(x)
#endif



/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

#if ! defined (YYSTYPE) && ! defined (YYSTYPE_IS_DECLARED)
#line 38 "trace_g.yy"
typedef union YYSTYPE {
    int         num;      /* For returning numbers.               */
    char        id[200];  /* For returning ids.                   */
    t_op2fn     op2fn;    /* For returning op2fn                  */
    BDDNode     *bdd;     /* For returning exp                    */
} YYSTYPE;
/* Line 191 of yacc.c.  */
#line 163 "libt5_la-trace_g.cc"
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif



/* Copy the second part of user declarations.  */


/* Line 214 of yacc.c.  */
#line 175 "libt5_la-trace_g.cc"

#if ! defined (yyoverflow) || YYERROR_VERBOSE

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
#endif /* ! defined (yyoverflow) || YYERROR_VERBOSE */


#if (! defined (yyoverflow) \
     && (! defined (__cplusplus) \
	 || (YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  short yyss;
  YYSTYPE yyvs;
  };

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (short) + sizeof (YYSTYPE))				\
      + YYSTACK_GAP_MAXIMUM)

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
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (0)

#endif

#if defined (__STDC__) || defined (__cplusplus)
   typedef signed char yysigned_char;
#else
   typedef short yysigned_char;
#endif

/* YYFINAL -- State number of the termination state. */
#define YYFINAL  2
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   71

/* YYNTOKENS -- Number of terminals. */
#define YYNTOKENS  24
/* YYNNTS -- Number of nonterminals. */
#define YYNNTS  13
/* YYNRULES -- Number of rules. */
#define YYNRULES  30
/* YYNRULES -- Number of states. */
#define YYNSTATES  74

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   273

#define YYTRANSLATE(YYX) 						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const unsigned char yytranslate[] =
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
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const unsigned char yyprhs[] =
{
       0,     0,     3,     4,    11,    14,    16,    17,    21,    24,
      25,    29,    32,    35,    36,    40,    47,    51,    58,    63,
      68,    73,    75,    79,    81,    83,    87,    89,    98,   103,
     108
};

/* YYRHS -- A `-1'-separated list of the rules' RHS. */
static const yysigned_char yyrhs[] =
{
      25,     0,    -1,    -1,    25,    26,    28,    29,    30,    27,
      -1,     4,     6,    -1,     5,    -1,    -1,     7,    33,    19,
      -1,     7,    19,    -1,    -1,     8,    33,    19,    -1,     8,
      19,    -1,    14,    31,    -1,    -1,    31,    32,    19,    -1,
       6,    20,    10,    21,    35,    22,    -1,     6,    20,    34,
      -1,     6,    20,    15,    21,     6,    22,    -1,     9,    21,
      35,    22,    -1,    18,    21,     3,    22,    -1,    16,    21,
      17,    22,    -1,     6,    -1,    33,    23,     6,    -1,    36,
      -1,    36,    -1,    35,    23,    36,    -1,     6,    -1,    11,
      21,    34,    23,    34,    23,    34,    22,    -1,    10,    21,
      35,    22,    -1,    13,    21,     6,    22,    -1,    12,    21,
       6,    22,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const unsigned char yyrline[] =
{
       0,    55,    55,    56,    59,    62,    66,    67,    69,    72,
      73,    75,    78,    81,    82,    86,    88,    90,    92,    94,
      96,   100,   102,   106,   110,   112,   116,   118,   120,   122,
     124
};
#endif

#if YYDEBUG || YYERROR_VERBOSE
/* YYTNME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals. */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "INTNUMBER", "MODULE", "ENDMODULE", "ID", 
  "INPUT", "OUTPUT", "ARE_EQUAL", "OP", "OP_ITE", "U_OP", "U_OP_NOT", 
  "STRUCTURE", "OP_NEW_INT_LEAF", "TPRINT", "STRING", "C_OP", "';'", 
  "'='", "'('", "')'", "','", "$accept", "module_input", "module", 
  "endmodule", "input", "output", "structure", "lines", "line", "varlist", 
  "exp_start", "exp_list", "exp", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const unsigned short yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,    59,
      61,    40,    41,    44
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const unsigned char yyr1[] =
{
       0,    24,    25,    25,    26,    27,    28,    28,    28,    29,
      29,    29,    30,    31,    31,    32,    32,    32,    32,    32,
      32,    33,    33,    34,    35,    35,    36,    36,    36,    36,
      36
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const unsigned char yyr2[] =
{
       0,     2,     0,     6,     2,     1,     0,     3,     2,     0,
       3,     2,     2,     0,     3,     6,     3,     6,     4,     4,
       4,     1,     3,     1,     1,     3,     1,     8,     4,     4,
       4
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const unsigned char yydefact[] =
{
       2,     0,     1,     0,     6,     4,     0,     9,    21,     8,
       0,     0,     0,     7,     0,    11,     0,    13,     0,    22,
      10,    12,     5,     3,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    14,    26,     0,     0,     0,     0,     0,
      16,    23,     0,     0,    24,     0,     0,     0,     0,     0,
       0,     0,     0,    18,     0,    20,    19,     0,     0,     0,
       0,     0,     0,    25,    15,     0,    30,    29,    17,    28,
       0,     0,     0,    27
};

/* YYDEFGOTO[NTERM-NUM]. */
static const yysigned_char yydefgoto[] =
{
      -1,     1,     4,    23,     7,    12,    18,    21,    28,    10,
      40,    43,    41
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -47
static const yysigned_char yypact[] =
{
     -47,     6,   -47,    -1,     0,   -47,    -5,     7,   -47,   -47,
      20,    -3,    12,   -47,    15,   -47,    21,   -47,    40,   -47,
     -47,     2,   -47,   -47,    26,    27,    28,    29,    32,    17,
      25,    30,    49,   -47,   -47,    33,    34,    35,    36,    37,
     -47,   -47,    38,   -10,   -47,    31,    39,    25,    25,    54,
      56,    57,    25,   -47,    25,   -47,   -47,    11,    41,    43,
      44,    45,    19,   -47,   -47,    25,   -47,   -47,   -47,   -47,
      46,    25,    48,   -47
};

/* YYPGOTO[NTERM-NUM].  */
static const yysigned_char yypgoto[] =
{
     -47,   -47,   -47,   -47,   -47,   -47,   -47,   -47,   -47,    60,
     -46,   -43,   -30
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -1
static const unsigned char yytable[] =
{
      44,     8,    58,     8,    57,     5,     2,     6,    24,    62,
       3,    25,    53,    54,     9,    11,    15,    44,    26,    70,
      27,    19,    44,    34,    63,    72,    17,    35,    36,    37,
      38,    34,    39,    64,    54,    42,    36,    37,    38,    13,
      20,    69,    54,    14,    14,    22,    29,    45,    30,    31,
      32,    33,    46,    55,    47,    48,    49,    50,    51,    52,
      59,    56,    60,    61,    65,    66,    67,    68,     0,    71,
      73,    16
};

static const yysigned_char yycheck[] =
{
      30,     6,    48,     6,    47,     6,     0,     7,     6,    52,
       4,     9,    22,    23,    19,     8,    19,    47,    16,    65,
      18,     6,    52,     6,    54,    71,    14,    10,    11,    12,
      13,     6,    15,    22,    23,    10,    11,    12,    13,    19,
      19,    22,    23,    23,    23,     5,    20,    17,    21,    21,
      21,    19,     3,    22,    21,    21,    21,    21,    21,    21,
       6,    22,     6,     6,    23,    22,    22,    22,    -1,    23,
      22,    11
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const unsigned char yystos[] =
{
       0,    25,     0,     4,    26,     6,     7,    28,     6,    19,
      33,     8,    29,    19,    23,    19,    33,    14,    30,     6,
      19,    31,     5,    27,     6,     9,    16,    18,    32,    20,
      21,    21,    21,    19,     6,    10,    11,    12,    13,    15,
      34,    36,    10,    35,    36,    17,     3,    21,    21,    21,
      21,    21,    21,    22,    23,    22,    22,    35,    34,     6,
       6,     6,    35,    36,    22,    23,    22,    22,    22,    22,
      34,    23,    34,    22
};

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
#define YYEMPTY		(-2)
#define YYEOF		0

#define YYACCEPT	goto yyacceptlab
#define YYABORT		goto yyabortlab
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
      yytoken = YYTRANSLATE (yychar);				\
      YYPOPSTACK;						\
      goto yybackup;						\
    }								\
  else								\
    { 								\
      yyerror ("syntax error: cannot back up");\
      YYERROR;							\
    }								\
while (0)

#define YYTERROR	1
#define YYERRCODE	256

/* YYLLOC_DEFAULT -- Compute the default location (before the actions
   are run).  */

#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)         \
  Current.first_line   = Rhs[1].first_line;      \
  Current.first_column = Rhs[1].first_column;    \
  Current.last_line    = Rhs[N].last_line;       \
  Current.last_column  = Rhs[N].last_column;
#endif

/* YYLEX -- calling `yylex' with the right arguments.  */

#ifdef YYLEX_PARAM
# define YYLEX yylex (YYLEX_PARAM)
#else
# define YYLEX yylex ()
#endif

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

# define YYDSYMPRINT(Args)			\
do {						\
  if (yydebug)					\
    yysymprint Args;				\
} while (0)

# define YYDSYMPRINTF(Title, Token, Value, Location)		\
do {								\
  if (yydebug)							\
    {								\
      YYFPRINTF (stderr, "%s ", Title);				\
      yysymprint (stderr, 					\
                  Token, Value);	\
      YYFPRINTF (stderr, "\n");					\
    }								\
} while (0)

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (cinluded).                                                   |
`------------------------------------------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yy_stack_print (short *bottom, short *top)
#else
static void
yy_stack_print (bottom, top)
    short *bottom;
    short *top;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (/* Nothing. */; bottom <= top; ++bottom)
    YYFPRINTF (stderr, " %d", *bottom);
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)				\
do {								\
  if (yydebug)							\
    yy_stack_print ((Bottom), (Top));				\
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yy_reduce_print (int yyrule)
#else
static void
yy_reduce_print (yyrule)
    int yyrule;
#endif
{
  int yyi;
  unsigned int yylineno = yyrline[yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %u), ",
             yyrule - 1, yylineno);
  /* Print the symbols being reduced, and their result.  */
  for (yyi = yyprhs[yyrule]; 0 <= yyrhs[yyi]; yyi++)
    YYFPRINTF (stderr, "%s ", yytname [yyrhs[yyi]]);
  YYFPRINTF (stderr, "-> %s\n", yytname [yyr1[yyrule]]);
}

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug)				\
    yy_reduce_print (Rule);		\
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YYDSYMPRINT(Args)
# define YYDSYMPRINTF(Title, Token, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
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



#if YYERROR_VERBOSE

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

#endif /* !YYERROR_VERBOSE */



#if YYDEBUG
/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yysymprint (FILE *yyoutput, int yytype, YYSTYPE *yyvaluep)
#else
static void
yysymprint (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  /* Pacify ``unused variable'' warnings.  */
  (void) yyvaluep;

  if (yytype < YYNTOKENS)
    {
      YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
# ifdef YYPRINT
      YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# endif
    }
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);

  switch (yytype)
    {
      default:
        break;
    }
  YYFPRINTF (yyoutput, ")");
}

#endif /* ! YYDEBUG */
/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yydestruct (int yytype, YYSTYPE *yyvaluep)
#else
static void
yydestruct (yytype, yyvaluep)
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  /* Pacify ``unused variable'' warnings.  */
  (void) yyvaluep;

  switch (yytype)
    {

      default:
        break;
    }
}


/* Prevent warnings from -Wmissing-prototypes.  */

#ifdef YYPARSE_PARAM
# if defined (__STDC__) || defined (__cplusplus)
int yyparse (void *YYPARSE_PARAM);
# else
int yyparse ();
# endif
#else /* ! YYPARSE_PARAM */
#if defined (__STDC__) || defined (__cplusplus)
int yyparse (void);
#else
int yyparse ();
#endif
#endif /* ! YYPARSE_PARAM */



/* The lookahead symbol.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;

/* Number of syntax errors so far.  */
int yynerrs;



/*----------.
| yyparse.  |
`----------*/

#ifdef YYPARSE_PARAM
# if defined (__STDC__) || defined (__cplusplus)
int yyparse (void *YYPARSE_PARAM)
# else
int yyparse (YYPARSE_PARAM)
  void *YYPARSE_PARAM;
# endif
#else /* ! YYPARSE_PARAM */
#if defined (__STDC__) || defined (__cplusplus)
int
yyparse (void)
#else
int
yyparse ()

#endif
#endif
{
  
  register int yystate;
  register int yyn;
  int yyresult;
  /* Number of tokens to shift before error messages enabled.  */
  int yyerrstatus;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken = 0;

  /* Three stacks and their tools:
     `yyss': related to states,
     `yyvs': related to semantic values,
     `yyls': related to locations.

     Refer to the stacks thru separate pointers, to allow yyoverflow
     to reallocate them elsewhere.  */

  /* The state stack.  */
  short	yyssa[YYINITDEPTH];
  short *yyss = yyssa;
  register short *yyssp;

  /* The semantic value stack.  */
  YYSTYPE yyvsa[YYINITDEPTH];
  YYSTYPE *yyvs = yyvsa;
  register YYSTYPE *yyvsp;



#define YYPOPSTACK   (yyvsp--, yyssp--)

  YYSIZE_T yystacksize = YYINITDEPTH;

  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;


  /* When reducing, the number of symbols on the RHS of the reduced
     rule.  */
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

  if (yyss + yystacksize - 1 <= yyssp)
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
	   data in use in that stack, in bytes.  This used to be a
	   conditional around just the two extra args, but that might
	   be undefined if yyoverflow is a macro.  */
	yyoverflow ("parser stack overflow",
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),

		    &yystacksize);

	yyss = yyss1;
	yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyoverflowlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
	goto yyoverflowlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
	yystacksize = YYMAXDEPTH;

      {
	short *yyss1 = yyss;
	union yyalloc *yyptr =
	  (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
	if (! yyptr)
	  goto yyoverflowlab;
	YYSTACK_RELOCATE (yyss);
	YYSTACK_RELOCATE (yyvs);

#  undef YYSTACK_RELOCATE
	if (yyss1 != yyssa)
	  YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;


      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
		  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
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
  if (yyn == YYPACT_NINF)
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = YYLEX;
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YYDSYMPRINTF ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yyn == 0 || yyn == YYTABLE_NINF)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  /* Shift the lookahead token.  */
  YYDPRINTF ((stderr, "Shifting token %s, ", yytname[yytoken]));

  /* Discard the token being shifted unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  *++yyvsp = yylval;


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

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 5:
#line 63 "trace_g.yy"
    { /* print_symtable(); */ }
    break;

  case 7:
#line 68 "trace_g.yy"
    { varlist[varindex] = NULL; ite_flag_vars(varlist, /*independent - 1*/1);  }
    break;

  case 10:
#line 74 "trace_g.yy"
    { varlist[varindex] = NULL; ite_flag_vars(varlist, /*dependent - 0*/0);  }
    break;

  case 14:
#line 83 "trace_g.yy"
    { if (((++lines) % 100) == 0) fprintf(stderr, "\r%d", lines); }
    break;

  case 15:
#line 87 "trace_g.yy"
    { explist[explevel][expindex[explevel]] = NULL; ite_op_equ(yyvsp[-5].id, yyvsp[-3].op2fn, explist[explevel]); explevel--; }
    break;

  case 16:
#line 89 "trace_g.yy"
    { ite_op_id_equ(yyvsp[-2].id,yyvsp[0].bdd); }
    break;

  case 17:
#line 91 "trace_g.yy"
    { ite_new_int_leaf(yyvsp[-5].id, yyvsp[-1].id); }
    break;

  case 18:
#line 93 "trace_g.yy"
    { explist[explevel][expindex[explevel]] = NULL; ite_op_are_equal(explist[explevel]); explevel--; }
    break;

  case 19:
#line 95 "trace_g.yy"
    { fprintf(stderr, "nonhandled are_c_op\n"); assert(0); }
    break;

  case 20:
#line 97 "trace_g.yy"
    { printf("%s\n", yyvsp[-1].id); }
    break;

  case 21:
#line 101 "trace_g.yy"
    { varindex=0; varlist[varindex++] = s_getsym(yyvsp[0].id, SYM_VAR); assert(varlist[varindex-1]); }
    break;

  case 22:
#line 103 "trace_g.yy"
    { varlist[varindex++] = s_getsym(yyvsp[0].id, SYM_VAR);  assert(varlist[varindex-1]);}
    break;

  case 23:
#line 107 "trace_g.yy"
    { yyval.bdd = yyvsp[0].bdd; }
    break;

  case 24:
#line 111 "trace_g.yy"
    { explevel++; expindex[explevel]=0; explist[explevel][expindex[explevel]++] = yyvsp[0].bdd; }
    break;

  case 25:
#line 113 "trace_g.yy"
    { explist[explevel][expindex[explevel]++] = yyvsp[0].bdd; }
    break;

  case 26:
#line 117 "trace_g.yy"
    { symrec *s=s_getsym(yyvsp[0].id, SYM_VAR); assert(s); yyval.bdd = ite_vars(s); }
    break;

  case 27:
#line 119 "trace_g.yy"
    { yyval.bdd = ite_s( yyvsp[-5].bdd, yyvsp[-3].bdd, yyvsp[-1].bdd); }
    break;

  case 28:
#line 121 "trace_g.yy"
    { explist[explevel][expindex[explevel]] = NULL; yyval.bdd = ite_op_exp(yyvsp[-3].op2fn,explist[explevel]); explevel--; }
    break;

  case 29:
#line 123 "trace_g.yy"
    { symrec *s=s_getsym(yyvsp[-1].id, SYM_VAR); assert(s); yyval.bdd = ite_not_s(ite_vars(s)); }
    break;

  case 30:
#line 125 "trace_g.yy"
    { fprintf(stderr, "nonhandled u_op\n"); yyval.bdd = NULL; }
    break;


    }

/* Line 991 of yacc.c.  */
#line 1203 "libt5_la-trace_g.cc"

  yyvsp -= yylen;
  yyssp -= yylen;


  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;


  /* Now `shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*------------------------------------.
| yyerrlab -- here on detecting error |
`------------------------------------*/
yyerrlab:
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if YYERROR_VERBOSE
      yyn = yypact[yystate];

      if (YYPACT_NINF < yyn && yyn < YYLAST)
	{
	  YYSIZE_T yysize = 0;
	  int yytype = YYTRANSLATE (yychar);
	  char *yymsg;
	  int yyx, yycount;

	  yycount = 0;
	  /* Start YYX at -YYN if negative to avoid negative indexes in
	     YYCHECK.  */
	  for (yyx = yyn < 0 ? -yyn : 0;
	       yyx < (int) (sizeof (yytname) / sizeof (char *)); yyx++)
	    if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
	      yysize += yystrlen (yytname[yyx]) + 15, yycount++;
	  yysize += yystrlen ("syntax error, unexpected ") + 1;
	  yysize += yystrlen (yytname[yytype]);
	  yymsg = (char *) YYSTACK_ALLOC (yysize);
	  if (yymsg != 0)
	    {
	      char *yyp = yystpcpy (yymsg, "syntax error, unexpected ");
	      yyp = yystpcpy (yyp, yytname[yytype]);

	      if (yycount < 5)
		{
		  yycount = 0;
		  for (yyx = yyn < 0 ? -yyn : 0;
		       yyx < (int) (sizeof (yytname) / sizeof (char *));
		       yyx++)
		    if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
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
	    yyerror ("syntax error; also virtual memory exhausted");
	}
      else
#endif /* YYERROR_VERBOSE */
	yyerror ("syntax error");
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
	 error, discard it.  */

      /* Return failure if at end of input.  */
      if (yychar == YYEOF)
        {
	  /* Pop the error token.  */
          YYPOPSTACK;
	  /* Pop the rest of the stack.  */
	  while (yyss < yyssp)
	    {
	      YYDSYMPRINTF ("Error: popping", yystos[*yyssp], yyvsp, yylsp);
	      yydestruct (yystos[*yyssp], yyvsp);
	      YYPOPSTACK;
	    }
	  YYABORT;
        }

      YYDSYMPRINTF ("Error: discarding", yytoken, &yylval, &yylloc);
      yydestruct (yytoken, &yylval);
      yychar = YYEMPTY;

    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab2;


/*----------------------------------------------------.
| yyerrlab1 -- error raised explicitly by an action.  |
`----------------------------------------------------*/
yyerrlab1:

  /* Suppress GCC warning that yyerrlab1 is unused when no action
     invokes YYERROR.  */
#if defined (__GNUC_MINOR__) && 2093 <= (__GNUC__ * 1000 + __GNUC_MINOR__)
  __attribute__ ((__unused__))
#endif


  goto yyerrlab2;


/*---------------------------------------------------------------.
| yyerrlab2 -- pop states until the error token can be shifted.  |
`---------------------------------------------------------------*/
yyerrlab2:
  yyerrstatus = 3;	/* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (yyn != YYPACT_NINF)
	{
	  yyn += YYTERROR;
	  if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
	    {
	      yyn = yytable[yyn];
	      if (0 < yyn)
		break;
	    }
	}

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
	YYABORT;

      YYDSYMPRINTF ("Error: popping", yystos[*yyssp], yyvsp, yylsp);
      yydestruct (yystos[yystate], yyvsp);
      yyvsp--;
      yystate = *--yyssp;

      YY_STACK_PRINT (yyss, yyssp);
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  YYDPRINTF ((stderr, "Shifting error token, "));

  *++yyvsp = yylval;


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

#ifndef yyoverflow
/*----------------------------------------------.
| yyoverflowlab -- parser overflow comes here.  |
`----------------------------------------------*/
yyoverflowlab:
  yyerror ("parser stack overflow");
  yyresult = 2;
  /* Fall through.  */
#endif

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
   symrec  *s_ptr = s_getsym(var, SYM_VAR);
   assert(s_ptr);
   s_set_indep(s_ptr, 0);
   //independantVars[s_ptr->id] = 0;
   BDDNode *ptr = ite_equ(ite_vars(s_ptr), bdd);
   functions_add(ptr, UNSURE, s_ptr->id);
}

int ite_last_fn=0;

void
ite_op_equ(char *var, t_op2fn fn, BDDNode **explist)
{
   fn.fn_type = MAKE_EQU(fn.fn_type);
   symrec  *s_ptr = s_getsym(var, SYM_VAR);
   assert(s_ptr);
   s_set_indep(s_ptr, 0);
   //independantVars[s_ptr->id] = 0;
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
               symrec *s_ptr = tputsym(SYM_VAR);
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
      symrec *s_ptr = tputsym(SYM_VAR);
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
   symrec *s_ptr = s_getsym(id, SYM_VAR);
   assert(s_ptr);
   s_set_indep(s_ptr, 0);
   //independantVars[s_ptr->id] = 0;
   BDDNode *ptr = ite_vars(s_ptr);
   if (*zero_one == '0') ptr = ite_not_s(ptr);
   else if (*zero_one != '1') {
      fprintf(stderr, "new_int_leaf with non 0/1 argument (%s) -- assuming 1\n", zero_one);
   }
   functions_add(ptr, UNSURE, s_ptr->id);
}

void
ite_flag_vars(symrec **varlist, int indep)
{
  for (int i=0; varlist[i]!=NULL; i++) {
     s_set_indep(varlist[i], indep);
     //independantVars[varlist[i]->id] = indep;
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

