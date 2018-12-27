%{
    #include <cstdio>
    #include <cstdlib>
    #include <iostream>
    #include "SemValue.h"
    #include "OrderPack.h"
    #include "../fs/bufmanager/BufPageManager.h"
    #include "../rm/rm.h"
    #include "../rm/rm_rid.h"
    #include "../ix/ix.h"
    #include "../sm/sm.h"
    #include "../ql/ql.h"
    #include "../utils/defs.h"
    #include "../utils/utils.h"
    #define YYSTYPE SemValue
    void yyerror(const char*);
    void prompt();
    bool isCmd = true;
    int yylex(void);
    extern FILE *yyin;
    extern int yyparse();
    FileManager fm("test_dbfiles");
    BufPageManager bpm(&fm);
    RM_Manager rmm(&bpm);
    IX_Manager ixm(bpm);
    SM_Manager smm(&ixm, &rmm);
    QL_Manager qlm(&smm, &ixm, &rmm);
%}

%token VALUE_INT VALUE_FLOAT VALUE_STRING
%token IDENTIFIER
%token EXIT
%token DATABASE DATABASES SHOW CREATE TABLE TABLES DROP USE DESC
%token PRIMARY KEY NOT NUL IS FOREIGN REFERENCES
%token INSERT INTO VALUES DELETE
%token FROM WHERE UPDATE SET SELECT
%token TYPE_INT TYPE_VARCHAR TYPE_FLOAT TYPE_DATE
%token AND LE GE NE
%token INDEX

%left AND

%%

Program             :   Program Stmt
                    |   %empty
                    ;

Stmt                :   SysStmt
                    |   DbStmt
                    |   TbStmt
                    |   IdxStmt
                    ;

SysStmt             :   SHOW DATABASES ';'
                        {
                            OrderPack pack(OrderPack::SHOW_DATABASES);
                            pack.process();
                            prompt();
                        }
                    |   EXIT ';'
                        {
                            OrderPack pack(OrderPack::EXIT);
                            pack.process();
                        }
                    ;

DbStmt              :   CREATE DATABASE IDENTIFIER ';'
                        {
                            OrderPack pack(OrderPack::CREATE_DATABASE);
                            pack.dbname = $3.id;
                            pack.process();
                            prompt();
                        }
                    |   DROP DATABASE IDENTIFIER ';'
                        {
                            OrderPack pack(OrderPack::DROP_DATABASE);
                            pack.dbname = $3.id;
                            pack.process();
                            prompt();
                        }
                    |   USE IDENTIFIER ';'
                        {
                            OrderPack pack(OrderPack::USE_DATABASE);
                            pack.dbname = $2.id;
                            pack.process();
                            prompt();
                        }
                    |   SHOW TABLES ';'
                        {
                            OrderPack pack(OrderPack::SHOW_TABLES);
                            pack.process();
                            prompt();
                        }
                    ;

TbStmt              :   CREATE TABLE IDENTIFIER '(' FieldList ')' ';'
                        {
                            OrderPack pack(OrderPack::CREATE_TABLE);
                            pack.tbname = $3.id;
                            pack.attrList = $5.attrList;
                            pack.process();
                            prompt();
                        }
                    |   DROP TABLE IDENTIFIER ';'
                        {
                            OrderPack pack(OrderPack::DROP_TABLE);
                            pack.tbname = $3.id;
                            pack.process();
                            prompt();
                        }
                    |   DESC IDENTIFIER ';'
                        {
                            OrderPack pack(OrderPack::DESC_TABLE);
                            pack.tbname = $2.id;
                            pack.process();
                            prompt();
                        }
                    |   INSERT INTO IDENTIFIER VALUES ValueLists ';'
                        {
                            OrderPack pack(OrderPack::INSERT_VALUES);
                            pack.tbname = $3.id;
                            pack.valuesList = $5.valuesList;
                            pack.process();
                            prompt();
                        }
                    |   DELETE FROM IDENTIFIER WHERE WhereClause ';'
                        {
                            OrderPack pack(OrderPack::DELETE_VALUES);
                            pack.tbname = $3.id;
                            pack.condEntry = $5.condEntry;
                            pack.process();
                            prompt();
                        }
                    |   UPDATE IDENTIFIER SET SetClause WHERE WhereClause ';'
                        {
                            OrderPack pack(OrderPack::UPDATE_VALUES);
                            pack.tbname = $2.id;
                            pack.setList = $4.setList;
                            pack.condEntry = $6.condEntry;
                            pack.process();
                            prompt();
                        }
                    |   SELECT Selector FROM TableList WHERE WhereClause ';'
                        {
                            OrderPack pack(OrderPack::SELECT_VALUES);
                            pack.selectList = $2.selectList;
                            pack.tableList = $4.tableList;
                            pack.condEntry = $6.condEntry;
                            pack.process();
                            prompt();
                        }
                    ;

IdxStmt             :   CREATE INDEX IDENTIFIER  '(' IDENTIFIER ')' ';'
                        {
                            OrderPack pack(OrderPack::CREATE_INDEX);
                            pack.tbname = $3.id;
                            pack.colname = $5.id;
                            pack.process();
                            prompt();
                        }
                    |   DROP INDEX IDENTIFIER '(' IDENTIFIER ')' ';'
                        {
                            OrderPack pack(OrderPack::DROP_INDEX);
                            pack.tbname = $3.id;
                            pack.colname = $5.id;
                            pack.process();
                            prompt();
                        }
                    ;

FieldList           :   Field
                        {
                            $$.attrList.clear();
                            $$.attrList.process($1.attrEntry);
                        }
                    |   FieldList ',' Field
                        {
                            $$.attrList = $1.attrList;
                            $$.attrList.process($3.attrEntry);
                        }
                    ;

Field               :   IDENTIFIER Type
                        {
                            $$.attrEntry = AttrEntry(AttrEntry::NORMAL, $1.id, $2.attrType, $2.attrLength);
                        }
                    |   IDENTIFIER Type NOT NUL
                        {
                            $$.attrEntry = AttrEntry(AttrEntry::NORMAL, $1.id, $2.attrType, $2.attrLength);
                            $$.attrEntry.notNull = true;
                        }
                    |   PRIMARY KEY '(' IDENTIFIER ')'
                        {
                            $$.attrEntry = AttrEntry(AttrEntry::PRIMARY, $4.id);
                        }
                    |   FOREIGN KEY '(' IDENTIFIER ')' REFERENCES IDENTIFIER '(' IDENTIFIER ')'
                        {
                            $$.attrEntry = AttrEntry(AttrEntry::FOREIGN, $4.id, $7.id, $9.id);
                        }
                    ;

Type                :   TYPE_INT '(' VALUE_INT ')'
                        {
                            $$.attrType = INT;
                            $$.attrLength = 4;
                        }
                    |   TYPE_VARCHAR '(' VALUE_INT ')'
                        {
                            $$.attrType = VARSTRING;
                            $$.attrLength = *((int*)($3.value)) + 1;
                        }
                    |   TYPE_DATE
                        {
                            $$.attrType = DATE;
                            $$.attrLength = 4;
                        }
                    |   TYPE_FLOAT
                        {
                            $$.attrType = FLOAT;
                            $$.attrLength = 4;
                        }
                    ;

ValueLists          :   '(' ValueList ')'
                        {
                            $$.valuesList.clear();
                            $$.valuesList.push_back($2.values);
                        }
                    |   ValueLists ',' '(' ValueList ')'
                        {
                            $$.valuesList = $1.valuesList;
                            $$.valuesList.push_back($4.values);
                        }
                    ;

ValueList           :   Value
                        {
                            $$.values.clear();
                            $$.values.push_back($1.value);
                        }
                    |   ValueList ',' Value
                        {
                            $$.values = $1.values;
                            $$.values.push_back($3.value);
                        }
                    ;

Value               :   VALUE_INT
                        {
                            $$.value = $1.value;
                        }
                    |   VALUE_STRING
                        {
                            $$.value = $1.value;
                        }
                    |   VALUE_FLOAT
                        {
                            $$.value = $1.value;
                        }
                    |   NUL
                        {
                            $$.value = NULL;
                        }
                    ;

WhereClause         :   Col '=' Expr
                        {
                            $$.condEntry.calcOp = CondEntry::EQUAL;
                            $$.condEntry.leftTbname = $1.tbname;
                            $$.condEntry.leftColname = $1.colname;
                            $$.condEntry.leftValue = $1.value;
                            $$.condEntry.rightTbname = $3.tbname;
                            $$.condEntry.rightColname = $3.colname;
                            $$.condEntry.rightValue = $3.value;
                        }

                    |   Col NE Expr
                        {
                            $$.condEntry.calcOp = CondEntry::NOT_EQUAL;
                            $$.condEntry.leftTbname = $1.tbname;
                            $$.condEntry.leftColname = $1.colname;
                            $$.condEntry.leftValue = $1.value;
                            $$.condEntry.rightTbname = $3.tbname;
                            $$.condEntry.rightColname = $3.colname;
                            $$.condEntry.rightValue = $3.value;
                        }
                    |   Col LE Expr
                        {
                            $$.condEntry.calcOp = CondEntry::LESS_EQUAL;
                            $$.condEntry.leftTbname = $1.tbname;
                            $$.condEntry.leftColname = $1.colname;
                            $$.condEntry.leftValue = $1.value;
                            $$.condEntry.rightTbname = $3.tbname;
                            $$.condEntry.rightColname = $3.colname;
                            $$.condEntry.rightValue = $3.value;
                        }
                    |   Col GE Expr
                        {
                            $$.condEntry.calcOp = CondEntry::GREATER_EQUAL;
                            $$.condEntry.leftTbname = $1.tbname;
                            $$.condEntry.leftColname = $1.colname;
                            $$.condEntry.leftValue = $1.value;
                            $$.condEntry.rightTbname = $3.tbname;
                            $$.condEntry.rightColname = $3.colname;
                            $$.condEntry.rightValue = $3.value;
                        }
                    |   Col '<' Expr
                        {
                            $$.condEntry.calcOp = CondEntry::LESS;
                            $$.condEntry.leftTbname = $1.tbname;
                            $$.condEntry.leftColname = $1.colname;
                            $$.condEntry.leftValue = $1.value;
                            $$.condEntry.rightTbname = $3.tbname;
                            $$.condEntry.rightColname = $3.colname;
                            $$.condEntry.rightValue = $3.value;
                        }
                    |   Col '>' Expr
                        {
                            $$.condEntry.calcOp = CondEntry::GREATER;
                            $$.condEntry.leftTbname = $1.tbname;
                            $$.condEntry.leftColname = $1.colname;
                            $$.condEntry.leftValue = $1.value;
                            $$.condEntry.rightTbname = $3.tbname;
                            $$.condEntry.rightColname = $3.colname;
                            $$.condEntry.rightValue = $3.value;
                        }
                    |   Col IS NUL
                        {
                            $$.condEntry.calcOp = CondEntry::IS_NULL;
                            $$.condEntry.leftTbname = $1.tbname;
                            $$.condEntry.leftColname = $1.colname;
                            $$.condEntry.leftValue = $1.value;
                        }
                    |   Col IS NOT NUL
                        {
                            $$.condEntry.calcOp = CondEntry::IS_NOT_NULL;
                            $$.condEntry.leftTbname = $1.tbname;
                            $$.condEntry.leftColname = $1.colname;
                            $$.condEntry.leftValue = $1.value;
                        }
                    |   WhereClause AND WhereClause
                        {
                            $$.condEntry.calcOp = CondEntry::AND;
                            $$.condEntry.leftEntry = &($1.condEntry);
                            $$.condEntry.rightEntry = &($3.condEntry);
                        }
                    ;

Col                 :   IDENTIFIER '.' IDENTIFIER
                        {
                            $$.tbname = $1.id;
                            $$.colname = $3.id;
                        }
                    |   IDENTIFIER
                        {
                            $$.tbname = NULL;
                            $$.colname = $1.id;
                        }
                    ;

Expr                :   Value
                        {
                            $$.value = $1.value;
                            $$.tbname = NULL;
                            $$.colname = NULL;
                        }
                    |   Col
                        {
                            $$.value = NULL;
                            $$.tbname = $1.tbname;
                            $$.colname = $1.colname;
                        }
                    ;

SetClause           :   IDENTIFIER '=' Value
                        {
                            $$.setList.clear();
                            $$.setList.add($1.id, $3.value);
                        }
                    |   SetClause ',' IDENTIFIER '=' Value
                        {
                            $$.setList = $1.setList;
                            $$.setList.add($3.id, $5.value);
                        }
                    ;

Selector            :   '*'
                        {
                            $$.selectList.clear();
                            $$.selectList.selectType = SelectList::ALL;
                        }
                    |   Col
                        {
                            $$.selectList.clear();
                            $$.selectList.add($1.tbname, $1.colname);
                        }
                    |   Selector ',' Col
                        {
                            $$.selectList = $1.selectList;
                            $$.selectList.add($3.tbname, $3.colname);
                        }
                    ;

TableList           :   IDENTIFIER
                        {
                            $$.tableList.clear();
                            $$.tableList.push_back($1.id);
                        }
                    |   TableList ',' IDENTIFIER
                        {
                            $$.tableList = $1.tableList;
                            $$.tableList.push_back($3.id);
                        }
                    ;

%%

void prompt() {
    if (isCmd) {
        printf("Databass>\n");
    }
}

void yyerror(const char *s) {}

int main(int argc, char **argv) {
    if (argc > 2) {
        std::cout << "Usage: main [filename]" << std::endl;
        return -1;
    }

    FILE *pFile = NULL;
    if (argc == 2) {
        pFile = fopen(argv[1], "r");
        if (!pFile) {
            std::cout << "Input File not Exist!" << std::endl;
            return -1;
        }
        isCmd = false;
        yyin = pFile;
        yyparse();
    }
    isCmd = true;
    prompt();
    yyparse();

    return 0;
}