/* A Bison parser, made by GNU Bison 2.3.  */

/* Skeleton interface for Bison's Yacc-like parsers in C

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




#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
#line 40 "trace_g.yy"
{
    int         num;      /* For returning numbers.               */
    char        id[200];  /* For returning ids.                   */
    t_op2fn     op2fn;    /* For returning op2fn                  */
    BDDNode     *bdd;     /* For returning exp                    */
}
/* Line 1489 of yacc.c.  */
#line 92 "libt5_a-trace_g.h"
	YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE trace_lval;

