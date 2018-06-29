/* A Bison parser, made by GNU Bison 3.0.4.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

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

#ifndef YY_YY_YACC_HH_INCLUDED
# define YY_YY_YACC_HH_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    TOK_ATOM = 258,
    TOK_FEAT = 259,
    TOK_VAR = 260,
    TOK_STARTP = 261,
    TOK_MODULENAME = 262,
    TOK_INTEGER = 263,
    TOK_STRING = 264,
    TOK_DOUBLE = 265,
    TOK_INVALID = 266,
    TOK_IS = 267,
    TOK_MOD = 268,
    TOK_EOF = 269,
    SUBSUMED_BY = 270,
    ATTR_DEF = 271,
    ATOMP = 272,
    LISTEND = 273,
    MEANS = 274,
    MACRO_DEF = 275,
    QUERY = 276,
    UMEANS = 277,
    IF_THEN = 278,
    NOT = 279,
    CMP_L = 280,
    CMP_LE = 281,
    CMP_G = 282,
    CMP_GE = 283,
    CMP_EQ = 284,
    CMP_NE = 285,
    NOT_UNIFIABLE = 286,
    BIT_OR = 287,
    BIT_AND = 288,
    INTDIV = 289,
    USIGN = 290,
    FEATURE = 291
  };
#endif
/* Tokens.  */
#define TOK_ATOM 258
#define TOK_FEAT 259
#define TOK_VAR 260
#define TOK_STARTP 261
#define TOK_MODULENAME 262
#define TOK_INTEGER 263
#define TOK_STRING 264
#define TOK_DOUBLE 265
#define TOK_INVALID 266
#define TOK_IS 267
#define TOK_MOD 268
#define TOK_EOF 269
#define SUBSUMED_BY 270
#define ATTR_DEF 271
#define ATOMP 272
#define LISTEND 273
#define MEANS 274
#define MACRO_DEF 275
#define QUERY 276
#define UMEANS 277
#define IF_THEN 278
#define NOT 279
#define CMP_L 280
#define CMP_LE 281
#define CMP_G 282
#define CMP_GE 283
#define CMP_EQ 284
#define CMP_NE 285
#define NOT_UNIFIABLE 286
#define BIT_OR 287
#define BIT_AND 288
#define INTDIV 289
#define USIGN 290
#define FEATURE 291

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED

union YYSTYPE
{
#line 195 "yacc.yy" /* yacc.c:1909  */

  int      ival;
  double   dval;
  char*    str;
  lilfes::type*    typeval;
  const lilfes::type *ctypeval;
  lilfes::feature* featval;
  const lilfes::feature* cfeatval;
  lilfes::module *modval;
  struct {
	  lilfes::feature *feat;
	  const lilfes::type *app;
  } appfeatval;
  lilfes::core_p   pval;

#line 142 "yacc.hh" /* yacc.c:1909  */
};

typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif



int yyparse (void *parm);

#endif /* !YY_YY_YACC_HH_INCLUDED  */
