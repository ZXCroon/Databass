#include "OrderPack.h"

extern bool isDebug;

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
        if (isDebug) {
            printf("DEBUG: CREATE %s\n", dbname);
        }
        smm.createDb(dbname);
        break;
    }

    case DROP_DATABASE: {
        if (isDebug) {
            printf("DEBUG: DROP %s\n", dbname);
        }
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
        int size = values.size() / tupleSize;
        if (isDebug) {
            printf("DEBUG: INSERT %d %d\n", size, tupleSize);
        }
        for (int i = 0; i < size; ++i) {
            Value tempValues[tupleSize];
            for (int j = 0; j < tupleSize; ++j) {
                tempValues[j] = (Value) {valueTypes[i * tupleSize + j], values[i * tupleSize + j]};
            }
            qlm.insert(tbname, tupleSize, tempValues);
        }
        break;
    }

    case DELETE_VALUES: {
        if (isDebug) {
            printf("DEBUG: DELETE %s %d\n", tbname, conditionList.size());
        }
        int size = conditionList.size();
        Condition conditions[size];
        for (int i = 0; i < size; ++i) {
            conditions[i] = conditionList[i];
            if (conditions[i].lhsAttr.attrName != NULL && conditions[i].lhsAttr.relName == NULL) {
                conditions[i].lhsAttr.relName = tbname;
            }
            if (conditions[i].rhsAttr.attrName != NULL && conditions[i].rhsAttr.relName == NULL) {
                conditions[i].rhsAttr.relName = tbname;
            }
        }
        if (isDebug) {
            printf("DEBUG: start deleting\n");
        }
        qlm.del(tbname, size, conditions);
        if (isDebug) {
            printf("DEBUG: finish deleting\n");
        }
        break;
    }

    case UPDATE_VALUES: {
        int size = conditionList.size();
        Condition conditions[size];
        for (int i = 0; i < size; ++i) {
            conditions[i] = conditionList[i];
            if (conditions[i].lhsAttr.attrName != NULL && conditions[i].lhsAttr.relName == NULL) {
                conditions[i].lhsAttr.relName = tbname;
            }
            if (conditions[i].rhsAttr.attrName != NULL && conditions[i].rhsAttr.relName == NULL) {
                conditions[i].rhsAttr.relName = tbname;
            }
        }
        if (isDebug) {
            printf("DEBUG: start updating\n");
        }
        qlm.update(tbname, updAttr, (updValue.data != NULL || updValue.type == NULL_TYPE), updRhsAttr, updValue, size, conditions);
        if (isDebug) {
            printf("DEBUG: finish updating\n");
        }
        break;
    }

    case SELECT_VALUES: {
        char *relation1, *relation2;
        JoinType joinType;
        if (tableList.size() == 1) {
            relation1 = tableList[0];
            relation2 = NULL;
            joinType = NO_JOIN;
        }
        else if (tableList.size() == 2) {
            relation1 = tableList[0];
            relation2 = tableList[1];
            joinType = this->joinType;
        }
        else {
            printf("TOO MANY RELATIONS!\n");
            return;
        }
        int size = conditionList.size();
        Condition conditions[size];
        for (int i = 0; i < size; ++i) {
            conditions[i] = conditionList[i];
        }
        if (selectList.selectType == SelectList::ALL) {
            qlm.select(-1, NULL, relation1, relation2, joinType, size, conditions);
        }
        else {
            int len = selectList.attrList.size();
            RelAttr selAttrs[len];
            for (int i = 0; i < len; ++i) {
                selAttrs[i] = selectList.attrList[i];
            }
            if (selectList.selectType == SelectList::AGGREGATE) {
                len = -2 - aggType;
            }
            if (isDebug) {
                printf("DEBUG: start selecting\n");
            }
            qlm.select(len, selAttrs, relation1, relation2, joinType, size, conditions);
            if (isDebug) {
                printf("DEBUG: finish selecting\n");
            }
        }
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
