/* A Bison parser, made from /usr/home/mkouril/tmp/sbsat/src/formats/5/prover_g.yy, by GNU bison 1.75.  */

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

#ifndef BISON_LIBT__LA_PROVER_G_H
# define BISON_LIBT__LA_PROVER_G_H

/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     ID = 258,
     P_IMP = 259,
     P_EQUIV = 260
   };
#endif
#define ID 258
#define P_IMP 259
#define P_EQUIV 260




#ifndef YYSTYPE
#line 29 "prover_g.yy"
typedef union {
    int         num;      /* For returning numbers.               */
    char        id[200];  /* For returning ids.                   */
    t_op2fn     op2fn;    /* For returning op2fn                  */
    BDDNode     *bdd;     /* For returning exp                    */
} yystype;
/* Line 1281 of /usr/local/share/bison/yacc.c.  */
#line 57 "libt5_la-prover_g.h"
# define YYSTYPE yystype
#endif

extern YYSTYPE prover_lval;


#endif /* not BISON_LIBT__LA_PROVER_G_H */

