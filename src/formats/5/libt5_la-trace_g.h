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




#if ! defined (YYSTYPE) && ! defined (YYSTYPE_IS_DECLARED)
#line 42 "trace_g.yy"
typedef union YYSTYPE {
    int         num;      /* For returning numbers.               */
    char        id[200];  /* For returning ids.                   */
    t_op2fn     op2fn;    /* For returning op2fn                  */
    BDDNode     *bdd;     /* For returning exp                    */
} YYSTYPE;
/* Line 1248 of yacc.c.  */
#line 79 "libt5_la-trace_g.h"
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE trace_lval;



