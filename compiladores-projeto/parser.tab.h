/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2021 Free Software Foundation,
   Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

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

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

#ifndef YY_YY_PARSER_TAB_H_INCLUDED
# define YY_YY_PARSER_TAB_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token kinds.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    YYEMPTY = -2,
    YYEOF = 0,                     /* "end of file"  */
    YYerror = 256,                 /* error  */
    YYUNDEF = 257,                 /* "invalid token"  */
    NUM_INT = 258,                 /* NUM_INT  */
    NUM_FLOAT = 259,               /* NUM_FLOAT  */
    STRING_LITERAL = 260,          /* STRING_LITERAL  */
    ID = 261,                      /* ID  */
    PLUS = 262,                    /* PLUS  */
    MINUS = 263,                   /* MINUS  */
    TIMES = 264,                   /* TIMES  */
    DIVIDE = 265,                  /* DIVIDE  */
    LPAREN = 266,                  /* LPAREN  */
    RPAREN = 267,                  /* RPAREN  */
    COLON = 268,                   /* COLON  */
    LCOLCH = 269,                  /* LCOLCH  */
    RCOLCH = 270,                  /* RCOLCH  */
    ASSIGN = 271,                  /* ASSIGN  */
    EQ = 272,                      /* EQ  */
    NEQ = 273,                     /* NEQ  */
    GTE = 274,                     /* GTE  */
    LTE = 275,                     /* LTE  */
    GT = 276,                      /* GT  */
    LT = 277,                      /* LT  */
    COMMA = 278,                   /* COMMA  */
    IF = 279,                      /* IF  */
    ELSE = 280,                    /* ELSE  */
    ELIF = 281,                    /* ELIF  */
    WHILE = 282,                   /* WHILE  */
    FOR = 283,                     /* FOR  */
    IN = 284,                      /* IN  */
    RANGE = 285,                   /* RANGE  */
    DEF = 286,                     /* DEF  */
    RETURN = 287,                  /* RETURN  */
    PRINT = 288,                   /* PRINT  */
    INPUT = 289,                   /* INPUT  */
    FALSE = 290,                   /* FALSE  */
    TRUE = 291,                    /* TRUE  */
    NOT = 292,                     /* NOT  */
    AND = 293,                     /* AND  */
    OR = 294,                      /* OR  */
    INDENT = 295,                  /* INDENT  */
    DEDENT = 296,                  /* DEDENT  */
    NEWLINE = 297,                 /* NEWLINE  */
    LOWER_THAN_ELSE = 298          /* LOWER_THAN_ELSE  */
  };
  typedef enum yytokentype yytoken_kind_t;
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 47 "parser/parser.y"

    int ival;
    double dval;
    char *sval;
    struct noAST *ast;

#line 114 "parser.tab.h"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;


int yyparse (void);


#endif /* !YY_YY_PARSER_TAB_H_INCLUDED  */
