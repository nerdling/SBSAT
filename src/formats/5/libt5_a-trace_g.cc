/* A Bison parser, made by GNU Bison 2.3.  */

/* Skeleton implementation for Bison's Yacc-like parsers in C

   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006
   Free Software Foundation, Inc.

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
   Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "2.3"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Using locations.  */
#define YYLSP_NEEDED 0

/* Substitute the variable and function names.  */
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
/* Tokens.  */
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

#include "sbsat.h"
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
   void trace_reallocate_varlist();
   void trace_reallocate_exp();
   void trace_free();

   symrec **trace_varlist = NULL;
   int trace_varmax = 0;
   int trace_varindex;

   BDDNode ***explist = NULL;
   int *expindex = NULL;
   int *expmax = NULL;
   int explevel_max;
   int explevel;


   int lines=0;

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

/* Enabling the token table.  */
#ifndef YYTOKEN_TABLE
# define YYTOKEN_TABLE 0
#endif

#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
#line 40 "trace_g.yy"
{
    int         num;      /* For returning numbers.               */
    char        id[200];  /* For returning ids.                   */
    t_op2fn     op2fn;    /* For returning op2fn                  */
    BDDNode     *bdd;     /* For returning exp                    */
}
/* Line 187 of yacc.c.  */
#line 185 "libt5_a-trace_g.cc"
	YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif



/* Copy the second part of user declarations.  */


/* Line 216 of yacc.c.  */
#line 198 "libt5_a-trace_g.cc"

#ifdef short
# undef short
#endif

#ifdef YYTYPE_UINT8
typedef YYTYPE_UINT8 yytype_uint8;
#else
typedef unsigned char yytype_uint8;
#endif

#ifdef YYTYPE_INT8
typedef YYTYPE_INT8 yytype_int8;
#elif (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
typedef signed char yytype_int8;
#else
typedef short int yytype_int8;
#endif

#ifdef YYTYPE_UINT16
typedef YYTYPE_UINT16 yytype_uint16;
#else
typedef unsigned short int yytype_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef short int yytype_int16;
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif ! defined YYSIZE_T && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned int
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

#ifndef YY_
# if YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(msgid) dgettext ("bison-runtime", msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(msgid) msgid
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(e) ((void) (e))
#else
# define YYUSE(e) /* empty */
#endif

/* Identity function, used to suppress warnings about constant conditions.  */
#ifndef lint
# define YYID(n) (n)
#else
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static int
YYID (int i)
#else
static int
YYID (i)
    int i;
#endif
{
  return i;
}
#endif

#if ! defined yyoverflow || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#     ifndef _STDLIB_H
#      define _STDLIB_H 1
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (YYID (0))
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined _STDLIB_H \
       && ! ((defined YYMALLOC || defined malloc) \
	     && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef _STDLIB_H
#    define _STDLIB_H 1
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


#if (! defined yyoverflow \
     && (! defined __cplusplus \
	 || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yytype_int16 yyss;
  YYSTYPE yyvs;
  };

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(To, From, Count) \
      __builtin_memcpy (To, From, (Count) * sizeof (*(From)))
#  else
#   define YYCOPY(To, From, Count)		\
      do					\
	{					\
	  YYSIZE_T yyi;				\
	  for (yyi = 0; yyi < (Count); yyi++)	\
	    (To)[yyi] = (From)[yyi];		\
	}					\
      while (YYID (0))
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
    while (YYID (0))

#endif

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  2
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   71

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  24
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  13
/* YYNRULES -- Number of rules.  */
#define YYNRULES  30
/* YYNRULES -- Number of states.  */
#define YYNSTATES  74

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   273

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
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
static const yytype_uint8 yyprhs[] =
{
       0,     0,     3,     4,    11,    14,    16,    17,    21,    24,
      25,    29,    32,    35,    36,    40,    47,    51,    58,    63,
      68,    73,    75,    79,    81,    83,    87,    89,    98,   103,
     108
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int8 yyrhs[] =
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
static const yytype_uint8 yyrline[] =
{
       0,    57,    57,    58,    61,    64,    68,    69,    71,    74,
      75,    77,    80,    83,    84,    88,    90,    92,    94,    96,
      98,   102,   104,   108,   112,   114,   118,   120,   122,   124,
     126
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "INTNUMBER", "MODULE", "ENDMODULE", "ID",
  "INPUT", "OUTPUT", "ARE_EQUAL", "OP", "OP_ITE", "U_OP", "U_OP_NOT",
  "STRUCTURE", "OP_NEW_INT_LEAF", "TPRINT", "STRING", "C_OP", "';'", "'='",
  "'('", "')'", "','", "$accept", "module_input", "module", "endmodule",
  "input", "output", "structure", "lines", "line", "varlist", "exp_start",
  "exp_list", "exp", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,    59,
      61,    40,    41,    44
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    24,    25,    25,    26,    27,    28,    28,    28,    29,
      29,    29,    30,    31,    31,    32,    32,    32,    32,    32,
      32,    33,    33,    34,    35,    35,    36,    36,    36,    36,
      36
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     0,     6,     2,     1,     0,     3,     2,     0,
       3,     2,     2,     0,     3,     6,     3,     6,     4,     4,
       4,     1,     3,     1,     1,     3,     1,     8,     4,     4,
       4
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
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

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int8 yydefgoto[] =
{
      -1,     1,     4,    23,     7,    12,    18,    21,    28,    10,
      40,    43,    41
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -47
static const yytype_int8 yypact[] =
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
static const yytype_int8 yypgoto[] =
{
     -47,   -47,   -47,   -47,   -47,   -47,   -47,   -47,   -47,    60,
     -46,   -43,   -30
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -1
static const yytype_uint8 yytable[] =
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

static const yytype_int8 yycheck[] =
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
static const yytype_uint8 yystos[] =
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

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		(-2)
#define YYEOF		0

#define YYACCEPT	goto yyacceptlab
#define YYABORT		goto yyabortlab
#define YYERROR		goto yyerrorlab


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
      YYPOPSTACK (1);						\
      goto yybackup;						\
    }								\
  else								\
    {								\
      yyerror (YY_("syntax error: cannot back up")); \
      YYERROR;							\
    }								\
while (YYID (0))


#define YYTERROR	1
#define YYERRCODE	256


/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#define YYRHSLOC(Rhs, K) ((Rhs)[K])
#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)				\
    do									\
      if (YYID (N))                                                    \
	{								\
	  (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;	\
	  (Current).first_column = YYRHSLOC (Rhs, 1).first_column;	\
	  (Current).last_line    = YYRHSLOC (Rhs, N).last_line;		\
	  (Current).last_column  = YYRHSLOC (Rhs, N).last_column;	\
	}								\
      else								\
	{								\
	  (Current).first_line   = (Current).last_line   =		\
	    YYRHSLOC (Rhs, 0).last_line;				\
	  (Current).first_column = (Current).last_column =		\
	    YYRHSLOC (Rhs, 0).last_column;				\
	}								\
    while (YYID (0))
#endif


/* YY_LOCATION_PRINT -- Print the location on the stream.
   This macro was not mandated originally: define only if we know
   we won't break user code: when these are the locations we know.  */

#ifndef YY_LOCATION_PRINT
# if YYLTYPE_IS_TRIVIAL
#  define YY_LOCATION_PRINT(File, Loc)			\
     fprintf (File, "%d.%d-%d.%d",			\
	      (Loc).first_line, (Loc).first_column,	\
	      (Loc).last_line,  (Loc).last_column)
# else
#  define YY_LOCATION_PRINT(File, Loc) ((void) 0)
# endif
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
} while (YYID (0))

# define YY_SYMBOL_PRINT(Title, Type, Value, Location)			  \
do {									  \
  if (yydebug)								  \
    {									  \
      YYFPRINTF (stderr, "%s ", Title);					  \
      yy_symbol_print (stderr,						  \
		  Type, Value); \
      YYFPRINTF (stderr, "\n");						  \
    }									  \
} while (YYID (0))


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_value_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# else
  YYUSE (yyoutput);
# endif
  switch (yytype)
    {
      default:
	break;
    }
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  if (yytype < YYNTOKENS)
    YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);

  yy_symbol_value_print (yyoutput, yytype, yyvaluep);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_stack_print (yytype_int16 *bottom, yytype_int16 *top)
#else
static void
yy_stack_print (bottom, top)
    yytype_int16 *bottom;
    yytype_int16 *top;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (; bottom <= top; ++bottom)
    YYFPRINTF (stderr, " %d", *bottom);
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)				\
do {								\
  if (yydebug)							\
    yy_stack_print ((Bottom), (Top));				\
} while (YYID (0))


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_reduce_print (YYSTYPE *yyvsp, int yyrule)
#else
static void
yy_reduce_print (yyvsp, yyrule)
    YYSTYPE *yyvsp;
    int yyrule;
#endif
{
  int yynrhs = yyr2[yyrule];
  int yyi;
  unsigned long int yylno = yyrline[yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
	     yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      fprintf (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr, yyrhs[yyprhs[yyrule] + yyi],
		       &(yyvsp[(yyi + 1) - (yynrhs)])
		       		       );
      fprintf (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug)				\
    yy_reduce_print (yyvsp, Rule); \
} while (YYID (0))

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
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
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif



#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined __GLIBC__ && defined _STRING_H
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static YYSIZE_T
yystrlen (const char *yystr)
#else
static YYSIZE_T
yystrlen (yystr)
    const char *yystr;
#endif
{
  YYSIZE_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static char *
yystpcpy (char *yydest, const char *yysrc)
#else
static char *
yystpcpy (yydest, yysrc)
    char *yydest;
    const char *yysrc;
#endif
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYSIZE_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYSIZE_T yyn = 0;
      char const *yyp = yystr;

      for (;;)
	switch (*++yyp)
	  {
	  case '\'':
	  case ',':
	    goto do_not_strip_quotes;

	  case '\\':
	    if (*++yyp != '\\')
	      goto do_not_strip_quotes;
	    /* Fall through.  */
	  default:
	    if (yyres)
	      yyres[yyn] = *yyp;
	    yyn++;
	    break;

	  case '"':
	    if (yyres)
	      yyres[yyn] = '\0';
	    return yyn;
	  }
    do_not_strip_quotes: ;
    }

  if (! yyres)
    return yystrlen (yystr);

  return yystpcpy (yyres, yystr) - yyres;
}
# endif

/* Copy into YYRESULT an error message about the unexpected token
   YYCHAR while in state YYSTATE.  Return the number of bytes copied,
   including the terminating null byte.  If YYRESULT is null, do not
   copy anything; just return the number of bytes that would be
   copied.  As a special case, return 0 if an ordinary "syntax error"
   message will do.  Return YYSIZE_MAXIMUM if overflow occurs during
   size calculation.  */
static YYSIZE_T
yysyntax_error (char *yyresult, int yystate, int yychar)
{
  int yyn = yypact[yystate];

  if (! (YYPACT_NINF < yyn && yyn <= YYLAST))
    return 0;
  else
    {
      int yytype = YYTRANSLATE (yychar);
      YYSIZE_T yysize0 = yytnamerr (0, yytname[yytype]);
      YYSIZE_T yysize = yysize0;
      YYSIZE_T yysize1;
      int yysize_overflow = 0;
      enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
      char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
      int yyx;

# if 0
      /* This is so xgettext sees the translatable formats that are
	 constructed on the fly.  */
      YY_("syntax error, unexpected %s");
      YY_("syntax error, unexpected %s, expecting %s");
      YY_("syntax error, unexpected %s, expecting %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s");
# endif
      char *yyfmt;
      char const *yyf;
      static char const yyunexpected[] = "syntax error, unexpected %s";
      static char const yyexpecting[] = ", expecting %s";
      static char const yyor[] = " or %s";
      char yyformat[sizeof yyunexpected
		    + sizeof yyexpecting - 1
		    + ((YYERROR_VERBOSE_ARGS_MAXIMUM - 2)
		       * (sizeof yyor - 1))];
      char const *yyprefix = yyexpecting;

      /* Start YYX at -YYN if negative to avoid negative indexes in
	 YYCHECK.  */
      int yyxbegin = yyn < 0 ? -yyn : 0;

      /* Stay within bounds of both yycheck and yytname.  */
      int yychecklim = YYLAST - yyn + 1;
      int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
      int yycount = 1;

      yyarg[0] = yytname[yytype];
      yyfmt = yystpcpy (yyformat, yyunexpected);

      for (yyx = yyxbegin; yyx < yyxend; ++yyx)
	if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
	  {
	    if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
	      {
		yycount = 1;
		yysize = yysize0;
		yyformat[sizeof yyunexpected - 1] = '\0';
		break;
	      }
	    yyarg[yycount++] = yytname[yyx];
	    yysize1 = yysize + yytnamerr (0, yytname[yyx]);
	    yysize_overflow |= (yysize1 < yysize);
	    yysize = yysize1;
	    yyfmt = yystpcpy (yyfmt, yyprefix);
	    yyprefix = yyor;
	  }

      yyf = YY_(yyformat);
      yysize1 = yysize + yystrlen (yyf);
      yysize_overflow |= (yysize1 < yysize);
      yysize = yysize1;

      if (yysize_overflow)
	return YYSIZE_MAXIMUM;

      if (yyresult)
	{
	  /* Avoid sprintf, as that infringes on the user's name space.
	     Don't have undefined behavior even if the translation
	     produced a string with the wrong number of "%s"s.  */
	  char *yyp = yyresult;
	  int yyi = 0;
	  while ((*yyp = *yyf) != '\0')
	    {
	      if (*yyp == '%' && yyf[1] == 's' && yyi < yycount)
		{
		  yyp += yytnamerr (yyp, yyarg[yyi++]);
		  yyf += 2;
		}
	      else
		{
		  yyp++;
		  yyf++;
		}
	    }
	}
      return yysize;
    }
}
#endif /* YYERROR_VERBOSE */


/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep)
#else
static void
yydestruct (yymsg, yytype, yyvaluep)
    const char *yymsg;
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  YYUSE (yyvaluep);

  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  switch (yytype)
    {

      default:
	break;
    }
}


/* Prevent warnings from -Wmissing-prototypes.  */

#ifdef YYPARSE_PARAM
#if defined __STDC__ || defined __cplusplus
int yyparse (void *YYPARSE_PARAM);
#else
int yyparse ();
#endif
#else /* ! YYPARSE_PARAM */
#if defined __STDC__ || defined __cplusplus
int yyparse (void);
#else
int yyparse ();
#endif
#endif /* ! YYPARSE_PARAM */



/* The look-ahead symbol.  */
int yychar;

/* The semantic value of the look-ahead symbol.  */
YYSTYPE yylval;

/* Number of syntax errors so far.  */
int yynerrs;



/*----------.
| yyparse.  |
`----------*/

#ifdef YYPARSE_PARAM
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void *YYPARSE_PARAM)
#else
int
yyparse (YYPARSE_PARAM)
    void *YYPARSE_PARAM;
#endif
#else /* ! YYPARSE_PARAM */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void)
#else
int
yyparse ()

#endif
#endif
{
  
  int yystate;
  int yyn;
  int yyresult;
  /* Number of tokens to shift before error messages enabled.  */
  int yyerrstatus;
  /* Look-ahead token as an internal (translated) token number.  */
  int yytoken = 0;
#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

  /* Three stacks and their tools:
     `yyss': related to states,
     `yyvs': related to semantic values,
     `yyls': related to locations.

     Refer to the stacks thru separate pointers, to allow yyoverflow
     to reallocate them elsewhere.  */

  /* The state stack.  */
  yytype_int16 yyssa[YYINITDEPTH];
  yytype_int16 *yyss = yyssa;
  yytype_int16 *yyssp;

  /* The semantic value stack.  */
  YYSTYPE yyvsa[YYINITDEPTH];
  YYSTYPE *yyvs = yyvsa;
  YYSTYPE *yyvsp;



#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  YYSIZE_T yystacksize = YYINITDEPTH;

  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;


  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

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
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
	/* Give user a chance to reallocate the stack.  Use copies of
	   these so that the &'s don't force the real ones into
	   memory.  */
	YYSTYPE *yyvs1 = yyvs;
	yytype_int16 *yyss1 = yyss;


	/* Each stack pointer address is followed by the size of the
	   data in use in that stack, in bytes.  This used to be a
	   conditional around just the two extra args, but that might
	   be undefined if yyoverflow is a macro.  */
	yyoverflow (YY_("memory exhausted"),
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),

		    &yystacksize);

	yyss = yyss1;
	yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyexhaustedlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
	goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
	yystacksize = YYMAXDEPTH;

      {
	yytype_int16 *yyss1 = yyss;
	union yyalloc *yyptr =
	  (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
	if (! yyptr)
	  goto yyexhaustedlab;
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

  /* Do appropriate processing given the current state.  Read a
     look-ahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to look-ahead token.  */
  yyn = yypact[yystate];
  if (yyn == YYPACT_NINF)
    goto yydefault;

  /* Not known => get a look-ahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid look-ahead symbol.  */
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
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
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

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the look-ahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the shifted token unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  yystate = yyn;
  *++yyvsp = yylval;

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
#line 65 "trace_g.yy"
    { /* print_symtable(); */ trace_free(); }
    break;

  case 7:
#line 70 "trace_g.yy"
    { trace_varlist[trace_varindex] = NULL; trace_reallocate_varlist(); ite_flag_vars(trace_varlist, /*independent - 1*/1);  }
    break;

  case 10:
#line 76 "trace_g.yy"
    { trace_varlist[trace_varindex] = NULL; trace_reallocate_varlist(); ite_flag_vars(trace_varlist, /*dependent - 0*/0);  }
    break;

  case 14:
#line 85 "trace_g.yy"
    { if (((++lines) % 100) == 0) d2_printf2("\r%d", lines); }
    break;

  case 15:
#line 89 "trace_g.yy"
    { trace_reallocate_exp(); explist[explevel][expindex[explevel]] = NULL; ite_op_equ((yyvsp[(1) - (6)].id), (yyvsp[(3) - (6)].op2fn), explist[explevel]); explevel--; }
    break;

  case 16:
#line 91 "trace_g.yy"
    { ite_op_id_equ((yyvsp[(1) - (3)].id),(yyvsp[(3) - (3)].bdd)); }
    break;

  case 17:
#line 93 "trace_g.yy"
    { ite_new_int_leaf((yyvsp[(1) - (6)].id), (yyvsp[(5) - (6)].id)); }
    break;

  case 18:
#line 95 "trace_g.yy"
    { trace_reallocate_exp(); explist[explevel][expindex[explevel]] = NULL; ite_op_are_equal(explist[explevel]); explevel--; }
    break;

  case 19:
#line 97 "trace_g.yy"
    { fprintf(stderr, "nonhandled are_c_op\n"); assert(0); }
    break;

  case 20:
#line 99 "trace_g.yy"
    { printf("%s\n", (yyvsp[(3) - (4)].id)); }
    break;

  case 21:
#line 103 "trace_g.yy"
    { trace_varindex=0; trace_reallocate_varlist(); trace_varlist[trace_varindex++] = s_getsym((yyvsp[(1) - (1)].id), SYM_VAR); assert(trace_varlist[trace_varindex-1]); }
    break;

  case 22:
#line 105 "trace_g.yy"
    { trace_reallocate_varlist(); trace_varlist[trace_varindex++] = s_getsym((yyvsp[(3) - (3)].id), SYM_VAR);  assert(trace_varlist[trace_varindex-1]);}
    break;

  case 23:
#line 109 "trace_g.yy"
    { (yyval.bdd) = (yyvsp[(1) - (1)].bdd); }
    break;

  case 24:
#line 113 "trace_g.yy"
    { explevel++; trace_reallocate_exp(); expindex[explevel]=0; explist[explevel][expindex[explevel]++] = (yyvsp[(1) - (1)].bdd); }
    break;

  case 25:
#line 115 "trace_g.yy"
    { trace_reallocate_exp(); explist[explevel][expindex[explevel]++] = (yyvsp[(3) - (3)].bdd); }
    break;

  case 26:
#line 119 "trace_g.yy"
    { symrec *s=s_getsym((yyvsp[(1) - (1)].id), SYM_VAR); assert(s); (yyval.bdd) = ite_vars(s); }
    break;

  case 27:
#line 121 "trace_g.yy"
    { (yyval.bdd) = ite_s( (yyvsp[(3) - (8)].bdd), (yyvsp[(5) - (8)].bdd), (yyvsp[(7) - (8)].bdd)); }
    break;

  case 28:
#line 123 "trace_g.yy"
    { trace_reallocate_exp(); explist[explevel][expindex[explevel]] = NULL; (yyval.bdd) = ite_op_exp((yyvsp[(1) - (4)].op2fn),explist[explevel]); explevel--; }
    break;

  case 29:
#line 125 "trace_g.yy"
    { symrec *s=s_getsym((yyvsp[(3) - (4)].id), SYM_VAR); assert(s); (yyval.bdd) = ite_not_s(ite_vars(s)); }
    break;

  case 30:
#line 127 "trace_g.yy"
    { fprintf(stderr, "nonhandled u_op\n"); (yyval.bdd) = NULL; }
    break;


/* Line 1267 of yacc.c.  */
#line 1545 "libt5_a-trace_g.cc"
      default: break;
    }
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
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
#if ! YYERROR_VERBOSE
      yyerror (YY_("syntax error"));
#else
      {
	YYSIZE_T yysize = yysyntax_error (0, yystate, yychar);
	if (yymsg_alloc < yysize && yymsg_alloc < YYSTACK_ALLOC_MAXIMUM)
	  {
	    YYSIZE_T yyalloc = 2 * yysize;
	    if (! (yysize <= yyalloc && yyalloc <= YYSTACK_ALLOC_MAXIMUM))
	      yyalloc = YYSTACK_ALLOC_MAXIMUM;
	    if (yymsg != yymsgbuf)
	      YYSTACK_FREE (yymsg);
	    yymsg = (char *) YYSTACK_ALLOC (yyalloc);
	    if (yymsg)
	      yymsg_alloc = yyalloc;
	    else
	      {
		yymsg = yymsgbuf;
		yymsg_alloc = sizeof yymsgbuf;
	      }
	  }

	if (0 < yysize && yysize <= yymsg_alloc)
	  {
	    (void) yysyntax_error (yymsg, yystate, yychar);
	    yyerror (yymsg);
	  }
	else
	  {
	    yyerror (YY_("syntax error"));
	    if (yysize != 0)
	      goto yyexhaustedlab;
	  }
      }
#endif
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse look-ahead token after an
	 error, discard it.  */

      if (yychar <= YYEOF)
	{
	  /* Return failure if at end of input.  */
	  if (yychar == YYEOF)
	    YYABORT;
	}
      else
	{
	  yydestruct ("Error: discarding",
		      yytoken, &yylval);
	  yychar = YYEMPTY;
	}
    }

  /* Else will try to reuse look-ahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:

  /* Pacify compilers like GCC when the user code never invokes
     YYERROR and the label yyerrorlab therefore never appears in user
     code.  */
  if (/*CONSTCOND*/ 0)
     goto yyerrorlab;

  /* Do not reclaim the symbols of the rule which action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
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


      yydestruct ("Error: popping",
		  yystos[yystate], yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  *++yyvsp = yylval;


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

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
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEOF && yychar != YYEMPTY)
     yydestruct ("Cleanup: discarding lookahead",
		 yytoken, &yylval);
  /* Do not reclaim the symbols of the rule which action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
		  yystos[*yyssp], yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif
  /* Make sure YYID is used.  */
  return YYID (yyresult);
}


#line 130 "trace_g.yy"


void
trace_reallocate_exp()
{
   if (explevel >= explevel_max)
   {
      explist = (BDDNode ***)ite_recalloc((void*)explist, explevel_max, explevel_max+10, sizeof(BDDNode **), 9, "explist");
      expindex = (int *)ite_recalloc((void*)expindex, explevel_max, explevel_max+10, sizeof(int), 9, "expindex");
      expmax = (int *)ite_recalloc((void*)expmax, explevel_max, explevel_max+10, sizeof(int), 9, "expmax");
      explevel_max += 10;
   }
   if (expindex[explevel] >= expmax[explevel])
   {
      explist[explevel] = (BDDNode **)ite_recalloc((void*)explist[explevel], expmax[explevel], expmax[explevel]+100, sizeof(BDDNode *), 9, "explist array");
      expmax[explevel] += 100;
   }
}

void
trace_reallocate_varlist()
{
   if (trace_varindex >= trace_varmax)
   {
      trace_varlist = (symrec **)ite_recalloc((void*)trace_varlist, trace_varmax, trace_varmax+100, sizeof(symrec *), 9, "trace_varlist");
      trace_varmax += 100;
   }
}

void
trace_free()
{
   ite_free((void**)&trace_varlist);
   trace_varmax = 0;

   for(int i=0;i<explevel_max;i++)
      ite_free((void**)&explist[i]);
   ite_free((void**)&explist);
   ite_free((void**)&expindex);
   ite_free((void**)&expmax);
   explevel_max = 0;
}

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
ite_op_equ(char *var, t_op2fn fn, BDDNode **_explist)
{
   fn.fn_type = MAKE_EQU(fn.fn_type);
   symrec  *s_ptr = s_getsym(var, SYM_VAR);
   assert(s_ptr);
   s_set_indep(s_ptr, 0);
   //independantVars[s_ptr->id] = 0;
   BDDNode *ptr = ite_op_exp(fn, _explist);
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
   int x_var = (*(*(BDDNode ***)&x))->variable;
   int y_var = (*(*(BDDNode ***)&y))->variable;
   //return (x_var - y_var);
   return (y_var - x_var);
}

BDDNode *ite_op_exp(t_op2fn fn, BDDNode **_explist)
{
   int i=0, num_members=0;
   BDDNode *ptr=NULL;
   assert(_explist[0] != NULL);
   /* first check the bdds:
    1. if they all have just one var => special fn
    2. if they combined have more than X distict var => split it
    */
   ite_op_flag++; 
   if (ite_op_flag > (1<<30)) { sym_clear_all_flag(); ite_op_flag = 1; }
   int spec_fn=0;
   int total_vars = 0;
   int new_vars = 0;

   for (num_members=0;_explist[num_members]!=NULL;num_members++) {
      ite_op_check(&new_vars, &total_vars, _explist[num_members]);
      if (total_vars <= 1) total_vars = 0;
   }

   /* check if each bdd in the function have just one variable */
   if (total_vars == 0 && num_members>=2 /*limit[fn.fn_type]*/) {
      /* we have a special function */ 
      spec_fn=1;
   } else {
      /* this does not happen in the current setup */
      /* if you decide to keep some special functions in a bigger BDD */
      /* you can use this to break bigger BDDs appart */
      /* check if they have more than a limit to get broken up */
      if (new_vars >= 5 /*s_limit*/ || num_members>=2 /*limit[fn.fn_type]*/) {
         printf("x");
         /* we have a special function 
          but we have to break it apart
          */
         spec_fn=1;
         for (i=0;_explist[i]!=NULL;i++) {
            /* take _explist[i] and ite_equ to a new temp var */
            ite_op_flag++; /* make sure we don't overflow... */
            int _new_vars=0;
            int _total_vars=0;
            ite_op_check(&_new_vars, &_total_vars, _explist[i]);
            if (total_vars > 1) {
               symrec *s_ptr = tputsym(SYM_VAR);
               /* save _explist[i] into another function */
               BDDNode *new_ptr = ite_equ(ite_vars(s_ptr), _explist[i]);
               functions_add(new_ptr, UNSURE, s_ptr->id);

               /* replace _explist[i] with ite_var of the temp var */
               _explist[i] = ite_vars(s_ptr);
            }
         }
      }
   }

   if (fn.as_type == AS_FULL) 
      qsort (_explist, num_members, sizeof (BDDNode*), explist_sort);

   /* apply the function */
   if (fn.as_type != AS_LEFT || fn.as_type == AS_RIGHT)
   {
      for (i=0;_explist[i]!=NULL;i++) {
         assert(_explist[i]->var_ptr);
         if (ptr==NULL) ptr = _explist[i];
         else ptr = fn.fn(ptr, _explist[i]);
      }
   } else {
      for(i=0;_explist[i]!=NULL;i++) { }
      while(i>0) {
         i--;
         assert(_explist[i]->var_ptr);
         if (ptr==NULL) ptr = _explist[i];
         else ptr = fn.fn(_explist[i], ptr);
      }
   }
   if (fn.neg_all) ptr = ite_not_s(ptr);
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
ite_op_are_equal(BDDNode **_explist)
{
   t_op2fn fn;

   /* LOOK: make sure ite_op_exp does not change the _explist much */
   /* ite_and positive */
   fn.fn=op_equ;
   fn.fn_type=EQU_EQU;
   fn.as_type=AS_FULL;
   fn.neg_all=0;
   BDDNode *ptr = ite_op_exp(fn, _explist);

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

