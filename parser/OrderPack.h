#ifndef ORDERPACK_H
#define ORDERPACK_H

#include "SemValue.h"

class OrderPack {

public:
    enum OrderType {
        SHOW_DATABASES,
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
    
    OrderType type;
    char *dbname, *tbname, *colname;
    AttrList attrList;
    CondEntry condEntry;
    SetList setList;
    std::vector<std::vector<void*>> valuesList;
    SelectList selectList;
    std::vector<char*> tableList;
}

#endif