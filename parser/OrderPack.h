#ifndef ORDERPACK_H
#define ORDERPACK_H

#include "SemValue.h"
#include "unistd.h"
#include "cstdlib"
#include "../utils/defs.h"

extern RM_Manager rmm;
extern IX_Manager ixm;
extern SM_Manager smm;
extern QL_Manager qlm;

class OrderPack {

public:
    enum OrderType {
        SHOW_DATABASES,
        EXIT,
        CREATE_DATABASE,
        DROP_DATABASE,
        USE_DATABASE,
        SHOW_TABLES,
        CREATE_TABLE,
        DROP_TABLE,
        DESC_TABLE,
        INSERT_VALUES,
        DELETE_VALUES,
        UPDATE_VALUES,
        SELECT_VALUES,
        CREATE_INDEX,
        DROP_INDEX
    };

    OrderPack(OrderType type): type(type) {}

    void process();
    
    OrderType type;
    char *dbname, *tbname, *colname;
    AttrList attrList;
    std::vector<Condition> conditionList;
    SetList setList;
    int tupleSize;
    std::vector<void*> values;
    std::vector<AttrType> valueTypes;
    SelectList selectList;
    RelAttr updAttr, updRhsAttr;
    Value updValue;
    std::vector<char*> tableList;
    JoinType joinType;
    AggType aggType;
};

#endif
