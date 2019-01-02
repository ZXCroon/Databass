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

#ifndef YY_YY_PARSER_PARSER_TAB_H_INCLUDED
# define YY_YY_PARSER_PARSER_TAB_H_INCLUDED
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
    VALUE_INT = 258,
    VALUE_FLOAT = 259,
    VALUE_STRING = 260,
    IDENTIFIER = 261,
    EXIT = 262,
    DATABASE = 263,
    DATABASES = 264,
    SHOW = 265,
    CREATE = 266,
    TABLE = 267,
    TABLES = 268,
    DROP = 269,
    USE = 270,
    DESC = 271,
    PRIMARY = 272,
    KEY = 273,
    NOT = 274,
    NUL = 275,
    IS = 276,
    FOREIGN = 277,
    REFERENCES = 278,
    INSERT = 279,
    INTO = 280,
    VALUES = 281,
    DELETE = 282,
    FROM = 283,
    WHERE = 284,
    UPDATE = 285,
    SET = 286,
    SELECT = 287,
    TYPE_INT = 288,
    TYPE_VARCHAR = 289,
    TYPE_FLOAT = 290,
    TYPE_DATE = 291,
    TYPE_CHAR = 292,
    AND = 293,
    LE = 294,
    GE = 295,
    NE = 296,
    LIKE = 297,
    INDEX = 298
  };
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef int YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_PARSER_PARSER_TAB_H_INCLUDED  */
