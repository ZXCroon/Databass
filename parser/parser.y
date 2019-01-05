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
    extern char *yytext;
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
%token TYPE_INT TYPE_VARCHAR TYPE_FLOAT TYPE_DATE TYPE_CHAR
%token AND LE GE NE LIKE
%token INDEX
%token JOIN INNER OUTER LEFT RIGHT FULL
%token AVG SUM MIN MAX COUNT

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
                            pack.tupleSize = $5.tupleSize;
                            pack.values = $5.values;
                            pack.valueTypes = $5.valueTypes;
                            pack.process();
                            prompt();
                        }
                    |   DELETE FROM IDENTIFIER WHERE WhereClause ';'
                        {
                            OrderPack pack(OrderPack::DELETE_VALUES);
                            pack.tbname = $3.id;
                            pack.conditionList = $5.conditionList;
                            pack.process();
                            prompt();
                        }
                    |   DELETE FROM IDENTIFIER ';'
                        {
                            OrderPack pack(OrderPack::DELETE_VALUES);
                            pack.tbname = $3.id;
                            pack.conditionList.clear();
                            pack.process();
                            prompt();
                        }
                    |   UPDATE IDENTIFIER SET IDENTIFIER '=' Expr WHERE WhereClause ';'
                        {
                            OrderPack pack(OrderPack::UPDATE_VALUES);
                            pack.tbname = $2.id;
                            pack.updAttr = {$2.id, $4.id};
                            pack.updRhsAttr = {$6.tbname, $6.colname};
                            pack.updValue = {$6.attrType, $6.value};
                            pack.conditionList = $8.conditionList;
                            pack.process();
                            prompt();
                        }
                    |   UPDATE IDENTIFIER SET IDENTIFIER '=' Expr';'
                        {
                            OrderPack pack(OrderPack::UPDATE_VALUES);
                            pack.tbname = $2.id;
                            pack.updAttr = {$2.id, $4.id};
                            pack.updRhsAttr = {$6.tbname, $6.colname};
                            pack.updValue = {$6.attrType, $6.value};
                            pack.conditionList.clear();
                            pack.process();
                            prompt();
                        }
                    |   SELECT Selector FROM SelectTableList WHERE WhereClause ';'
                        {
                            OrderPack pack(OrderPack::SELECT_VALUES);
                            pack.selectList = $2.selectList;
                            pack.aggType = NO_AGG;
                            pack.tableList = $4.tableList;
                            pack.joinType = $4.joinType;
                            pack.conditionList = $6.conditionList;
                            pack.process();
                            prompt();
                        }
                    |   SELECT AggSelector FROM SelectTableList WHERE WhereClause ';'
                        {
                            OrderPack pack(OrderPack::SELECT_VALUES);
                            pack.selectList = $2.selectList;
                            pack.selectList.selectType = SelectList::AGGREGATE;
                            pack.aggType = $2.aggType;
                            pack.tableList = $4.tableList;
                            pack.joinType = $4.joinType;
                            pack.conditionList = $6.conditionList;
                            pack.process();
                            prompt();
                        }
                    |   SELECT Selector FROM SelectTableList ';'
                        {
                            OrderPack pack(OrderPack::SELECT_VALUES);
                            pack.selectList = $2.selectList;
                            pack.aggType = NO_AGG;
                            pack.tableList = $4.tableList;
                            pack.joinType = $4.joinType;
                            pack.conditionList.clear();
                            pack.process();
                            prompt();
                        }
                    |   SELECT AggSelector FROM SelectTableList ';'
                        {
                            OrderPack pack(OrderPack::SELECT_VALUES);
                            pack.selectList = $2.selectList;
                            pack.selectList.selectType = SelectList::AGGREGATE;
                            pack.aggType = $2.aggType;
                            pack.tableList = $4.tableList;
                            pack.joinType = $4.joinType;
                            pack.conditionList.clear();
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
                    |   TYPE_CHAR '(' VALUE_INT ')'
                        {
                            $$.attrType = STRING;
                            $$.attrLength = *((int*)($3.value));
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
                            $$.tupleSize = $2.values.size();
                            $$.values = $2.values;
                            $$.valueTypes = $2.valueTypes;
                        }
                    |   ValueLists ',' '(' ValueList ')'
                        {
                            $$.values.insert($$.values.end(), $4.values.begin(), $4.values.end());
                            $$.valueTypes.insert($$.valueTypes.end(), $4.valueTypes.begin(), $4.valueTypes.end());
                        }
                    ;

ValueList           :   Value
                        {
                            $$.values.clear();
                            $$.values.push_back($1.value);
                            $$.valueTypes.clear();
                            $$.valueTypes.push_back($1.attrType);
                        }
                    |   ValueList ',' Value
                        {
                            $$.values.push_back($3.value);
                            $$.valueTypes.push_back($3.attrType);
                        }
                    ;

Value               :   VALUE_INT
                        {
                            $$.value = $1.value;
                            $$.attrType = INT;
                        }
                    |   VALUE_STRING
                        {
                            $$.value = $1.value;
                            $$.attrType = STRING;
                        }
                    |   VALUE_FLOAT
                        {
                            $$.value = $1.value;
                            $$.attrType = FLOAT;
                        }
                    |   NUL
                        {
                            $$.value = NULL;
                        }
                    ;

WhereClause         :   Col '=' Expr
                        {
                            $$.conditionList.clear();
                            Condition condition;
                            condition.lhsAttr = {$1.tbname, $1.colname};
                            condition.op = EQ_OP;
                            if ($3.value == NULL) {
                                condition.bRhsIsAttr = 1;
                                condition.rhsAttr = {$3.tbname, $3.colname};
                            }
                            else {
                                condition.bRhsIsAttr = 0;
                                condition.rhsValue = {$3.attrType, $3.value};
                            }
                            $$.conditionList.push_back(condition);
                        }

                    |   Col NE Expr
                        {
                            $$.conditionList.clear();
                            Condition condition;
                            condition.lhsAttr = {$1.tbname, $1.colname};
                            condition.op = NE_OP;
                            if ($3.value == NULL) {
                                condition.bRhsIsAttr = 1;
                                condition.rhsAttr = {$3.tbname, $3.colname};
                            }
                            else {
                                condition.bRhsIsAttr = 0;
                                condition.rhsValue = {$3.attrType, $3.value};
                            }
                            $$.conditionList.push_back(condition);
                        }
                    |   Col LE Expr
                        {
                            $$.conditionList.clear();
                            Condition condition;
                            condition.lhsAttr = {$1.tbname, $1.colname};
                            condition.op = LE_OP;
                            if ($3.value == NULL) {
                                condition.bRhsIsAttr = 1;
                                condition.rhsAttr = {$3.tbname, $3.colname};
                            }
                            else {
                                condition.bRhsIsAttr = 0;
                                condition.rhsValue = {$3.attrType, $3.value};
                            }
                            $$.conditionList.push_back(condition);
                        }
                    |   Col GE Expr
                        {
                            $$.conditionList.clear();
                            Condition condition;
                            condition.lhsAttr = {$1.tbname, $1.colname};
                            condition.op = GE_OP;
                            if ($3.value == NULL) {
                                condition.bRhsIsAttr = 1;
                                condition.rhsAttr = {$3.tbname, $3.colname};
                            }
                            else {
                                condition.bRhsIsAttr = 0;
                                condition.rhsValue = {$3.attrType, $3.value};
                            }
                            $$.conditionList.push_back(condition);
                        }
                    |   Col '<' Expr
                        {
                            $$.conditionList.clear();
                            Condition condition;
                            condition.lhsAttr = {$1.tbname, $1.colname};
                            condition.op = LT_OP;
                            if ($3.value == NULL) {
                                condition.bRhsIsAttr = 1;
                                condition.rhsAttr = {$3.tbname, $3.colname};
                            }
                            else {
                                condition.bRhsIsAttr = 0;
                                condition.rhsValue = {$3.attrType, $3.value};
                            }
                            $$.conditionList.push_back(condition);
                        }
                    |   Col '>' Expr
                        {
                            $$.conditionList.clear();
                            Condition condition;
                            condition.lhsAttr = {$1.tbname, $1.colname};
                            condition.op = GT_OP;
                            if ($3.value == NULL) {
                                condition.bRhsIsAttr = 1;
                                condition.rhsAttr = {$3.tbname, $3.colname};
                            }
                            else {
                                condition.bRhsIsAttr = 0;
                                condition.rhsValue = {$3.attrType, $3.value};
                            }
                            $$.conditionList.push_back(condition);
                        }
                    |   Col LIKE Expr
                        {
                            $$.conditionList.clear();
                            Condition condition;
                            condition.lhsAttr = {$1.tbname, $1.colname};
                            condition.op = LIKE_OP;
                            if ($3.value == NULL) {
                                condition.bRhsIsAttr = 1;
                                condition.rhsAttr = {$3.tbname, $3.colname};
                            }
                            else {
                                condition.bRhsIsAttr = 0;
                                condition.rhsValue = {$3.attrType, $3.value};
                            }
                            $$.conditionList.push_back(condition);
                        }
                    |   Col IS NUL
                        {
                            $$.conditionList.clear();
                            Condition condition;
                            condition.lhsAttr = {$1.tbname, $1.colname};
                            condition.op = ISNULL_OP;
                            if ($3.value == NULL) {
                                condition.bRhsIsAttr = 1;
                                condition.rhsAttr = {$3.tbname, $3.colname};
                            }
                            else {
                                condition.bRhsIsAttr = 0;
                                condition.rhsValue = {$3.attrType, $3.value};
                            }
                            $$.conditionList.push_back(condition);
                        }
                    |   Col IS NOT NUL
                        {
                            $$.conditionList.clear();
                            Condition condition;
                            condition.lhsAttr = {$1.tbname, $1.colname};
                            condition.op = NOTNULL_OP;
                            if ($3.value == NULL) {
                                condition.bRhsIsAttr = 1;
                                condition.rhsAttr = {$3.tbname, $3.colname};
                            }
                            else {
                                condition.bRhsIsAttr = 0;
                                condition.rhsValue = {$3.attrType, $3.value};
                            }
                            $$.conditionList.push_back(condition);
                        }
                    |   WhereClause AND WhereClause
                        {
                            $$.conditionList = $1.conditionList;
                            $$.conditionList.insert($$.conditionList.end(), $3.conditionList.begin(), $3.conditionList.end());
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
                            $$.attrType = $1.attrType;
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

AggSelector         :   AVG '(' Col ')'
                        {
                            $$.selectList.clear();
                            $$.selectList.add($3.tbname, $3.colname);
                            $$.aggType = AVG_AGG;
                        }
                    |   SUM '(' Col ')'
                        {
                            $$.selectList.clear();
                            $$.selectList.add($3.tbname, $3.colname);
                            $$.aggType = SUM_AGG;
                        }
                    |   MIN '(' Col ')'
                        {
                            $$.selectList.clear();
                            $$.selectList.add($3.tbname, $3.colname);
                            $$.aggType = MIN_AGG;
                        }
                    |   MAX '(' Col ')'
                        {
                            $$.selectList.clear();
                            $$.selectList.add($3.tbname, $3.colname);
                            $$.aggType = MAX_AGG;
                        }
                    |   COUNT '(' '*' ')'
                        {
                            $$.selectList.clear();
                            $$.aggType = COUNT_AGG;
                        }

SelectTableList     :   IDENTIFIER
                        {
                            $$.tableList.clear();
                            $$.tableList.push_back($1.id);
                            $$.joinType = NO_JOIN;
                        }
                    |   IDENTIFIER ',' IDENTIFIER
                        {
                            std::cout << "HEY" << std::endl;
                            $$.tableList.clear();
                            $$.tableList.push_back($1.id);
                            $$.tableList.push_back($3.id);
                            $$.joinType = INNER_JOIN;
                        }
                    |   IDENTIFIER INNER JOIN IDENTIFIER
                        {
                            std::cout << "CAO" << std::endl;
                            $$.tableList.clear();
                            $$.tableList.push_back($1.id);
                            $$.tableList.push_back($4.id);
                            $$.joinType = INNER_JOIN;
                        }
                    |   IDENTIFIER FULL JOIN IDENTIFIER
                        {
                            $$.tableList.clear();
                            $$.tableList.push_back($1.id);
                            $$.tableList.push_back($4.id);
                            $$.joinType = FULL_JOIN;
                        }
                    |   IDENTIFIER FULL OUTER JOIN IDENTIFIER
                        {
                            $$.tableList.clear();
                            $$.tableList.push_back($1.id);
                            $$.tableList.push_back($5.id);
                            $$.joinType = FULL_JOIN;
                        }
                    |   IDENTIFIER LEFT JOIN IDENTIFIER
                        {
                            $$.tableList.clear();
                            $$.tableList.push_back($1.id);
                            $$.tableList.push_back($4.id);
                            $$.joinType = LEFT_JOIN;
                        }
                    |   IDENTIFIER LEFT OUTER JOIN IDENTIFIER
                        {
                            $$.tableList.clear();
                            $$.tableList.push_back($1.id);
                            $$.tableList.push_back($5.id);
                            $$.joinType = LEFT_JOIN;
                        }
                    |   IDENTIFIER RIGHT JOIN IDENTIFIER
                        {
                            $$.tableList.clear();
                            $$.tableList.push_back($1.id);
                            $$.tableList.push_back($4.id);
                            $$.joinType = RIGHT_JOIN;
                        }
                    |   IDENTIFIER RIGHT OUTER JOIN IDENTIFIER
                        {
                            $$.tableList.clear();
                            $$.tableList.push_back($1.id);
                            $$.tableList.push_back($5.id);
                            $$.joinType = RIGHT_JOIN;
                        }
                    ;
                    

%%

void prompt() {
    if (isCmd) {
        printf("Databass>\n");
    }
}

void yyerror(const char *s) {
    printf("wrong %s %s\n", s, yytext);
}

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
