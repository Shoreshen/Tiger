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

#ifndef YY_YY_CH5_TIGER_TAB_H_INCLUDED
# define YY_YY_CH5_TIGER_TAB_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif
/* "%code requires" blocks.  */
#line 8 "ch5/tiger.y"

    #include "ast.h" // tiger.tab.h need it for type definition, thus should 
                     // be included in tiger.tab.h
    extern char *yyfilename;

    typedef struct YYLTYPE {
        int first_line;
        int first_column;
        int last_line;
        int last_column;
        char *filename;
    } YYLTYPE;
    # define YYLTYPE_IS_DECLARED 1
    # define YYLLOC_DEFAULT(Current, Rhs, N)                                \
        do {                                                                \
            if (N) {                                                        \
                (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;      \
                (Current).first_column = YYRHSLOC (Rhs, 1).first_column;    \
                (Current).last_line    = YYRHSLOC (Rhs, N).last_line;       \
                (Current).last_column  = YYRHSLOC (Rhs, N).last_column;     \
                (Current).filename     = YYRHSLOC (Rhs, N).filename;        \
            } else {                                                        \
                (Current).first_line   = (Current).last_line   =            \
                    YYRHSLOC (Rhs, 0).last_line;                            \
                (Current).first_column = (Current).last_column =            \
                    YYRHSLOC (Rhs, 0).last_column;                          \
                (Current).filename     = NULL;                              \
            }                                                               \
        } while (0)

#line 80 "./ch5/tiger.tab.h"

/* Token kinds.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    YYEMPTY = -2,
    YYEOF = 0,                     /* "end of file"  */
    YYerror = 256,                 /* error  */
    YYUNDEF = 257,                 /* "invalid token"  */
    ID = 258,                      /* ID  */
    STRING = 259,                  /* STRING  */
    INT = 260,                     /* INT  */
    NEQ = 261,                     /* NEQ  */
    LE = 262,                      /* LE  */
    GE = 263,                      /* GE  */
    ASSIGN = 264,                  /* ASSIGN  */
    ARRAY = 265,                   /* ARRAY  */
    IF = 266,                      /* IF  */
    THEN = 267,                    /* THEN  */
    ELSE = 268,                    /* ELSE  */
    WHILE = 269,                   /* WHILE  */
    FOR = 270,                     /* FOR  */
    TO = 271,                      /* TO  */
    DO = 272,                      /* DO  */
    LET = 273,                     /* LET  */
    IN = 274,                      /* IN  */
    END = 275,                     /* END  */
    OF = 276,                      /* OF  */
    BREAK = 277,                   /* BREAK  */
    NIL = 278,                     /* NIL  */
    FUNCTION = 279,                /* FUNCTION  */
    VAR = 280,                     /* VAR  */
    TYPE = 281,                    /* TYPE  */
    UMINUS = 282                   /* UMINUS  */
  };
  typedef enum yytokentype yytoken_kind_t;
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 39 "ch5/tiger.y"

    int pos;
    int ival;
    char *sval;
    A_var var;
    A_exp exp;
    A_dec dec;
    A_ty ty;
    A_decList decList;
    A_expList expList;
    A_field field;
    A_fieldList fieldList;
    A_fundec fundec;
    A_namety namety;
    A_efield efield;
    A_efieldList efieldList;

#line 142 "./ch5/tiger.tab.h"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif

/* Location type.  */
#if ! defined YYLTYPE && ! defined YYLTYPE_IS_DECLARED
typedef struct YYLTYPE YYLTYPE;
struct YYLTYPE
{
  int first_line;
  int first_column;
  int last_line;
  int last_column;
};
# define YYLTYPE_IS_DECLARED 1
# define YYLTYPE_IS_TRIVIAL 1
#endif


extern YYSTYPE yylval;
extern YYLTYPE yylloc;

int yyparse (void);


#endif /* !YY_YY_CH5_TIGER_TAB_H_INCLUDED  */
