#include "OrderPack.h"

void OrderPack::process() {
    switch (type) {

    case SHOW_DATABASES: {
        smm.showDbs();
        break;
    }

    case EXIT: {
        exit(0);
    }

    case CREATE_DATABASE: {
        printf("DEBUG: CREATE %s\n", dbname);
        smm.createDb(dbname);
        break;
    }

    case DROP_DATABASE: {
        printf("DEBUG: DROP %s\n", dbname);
        smm.dropDb(dbname);
        break;
    }

    case USE_DATABASE: {
        smm.openDb(dbname);
        break;
    }

    case SHOW_TABLES: {
        smm.showTables();
        break;
    }

    case CREATE_TABLE: {
        int size = attrList.attrList.size();
        int len;
        AttrInfo attrs[size];
        for (int i = 0; i < size; ++i) {
            len = strlen(attrList.attrList[i].id);
            memcpy(attrs[i].attrName, attrList.attrList[i].id, len);
            attrs[i].attrName[len] = '\0';
            attrs[i].attrType = attrList.attrList[i].attrType;
            attrs[i].attrLength = attrList.attrList[i].attrLength;
            if (attrList.attrList[i].isForeign) {
                len = strlen(attrList.attrList[i].refTbname);
                memcpy(attrs[i].refTbname, attrList.attrList[i].refTbname, len);
                attrs[i].refTbname[len] = '\0';
                len = strlen(attrList.attrList[i].refColname);
                memcpy(attrs[i].refColname, attrList.attrList[i].refColname, len);
                attrs[i].refColname[len] = '\0';
            }
            attrs[i].notNull = attrList.attrList[i].notNull;
            attrs[i].isPrimary = attrList.attrList[i].isPrimary;
            attrs[i].isForeign = attrList.attrList[i].isForeign;
        }
        smm.createTable(tbname, size, attrs);
        break;
    }

    case DROP_TABLE: {
        smm.dropTable(tbname);
        break;
    }

    case DESC_TABLE: {
        smm.showTable(tbname);
        break;
    }

    case INSERT_VALUES: {
        // todo
        break;
    }

    case DELETE_VALUES: {
        // todo
        break;
    }

    case UPDATE_VALUES: {
        // todo
        break;
    }

    case SELECT_VALUES: {
        // todo
        break;
    }

    case CREATE_INDEX: {
        smm.createIndex(tbname, colname);
        break;
    }

    case DROP_INDEX: {
        smm.dropIndex(tbname, colname);
        break;
    }

    }
}