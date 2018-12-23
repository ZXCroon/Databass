%{
    #include <cstdio>
    #include <cstdlib>
    #include <iostream>
    #include "SemValue.h"
    #define YYSTYPE SemValue
    void yyerror(const char*);
    int yylex(void);
%}

%token VALUE_INT VALUE_FLOAT VALUE_STRING
%token IDENTIFIER OPERATOR
%token INT CHAR VARCHAR
%token EXIT
%token DATABASE DATABASES SHOW CREATE TABLE TABLES DROP USE DESC
%token PRIMARY KEY NOT NULL IS FOREIGN REFERENCES
%token INSERT INTO VALUES DELETE
%token FROM WHERE UPDATE SET SELECT
%token INT VARCHAR FLOAT DATE
%token AND
%token INDEX

%%
