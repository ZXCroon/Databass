#ifndef SEMVALUE_H
#define SEMVALUE_H

#include "../utils/defs.h"
#include "../sm/sm.h"
#include <cstring>
#include <vector>


class AttrEntry {

public:
    enum FieldType {
        NORMAL,
        PRIMARY,
        FOREIGN
    };
    AttrEntry() {
        notNull = CAN_BE_NULL;
        isPrimary = NOT_PRIMARY;
        isForeign = NOT_FOREIGN;
    }
    AttrEntry(FieldType fieldType, char *id) : fieldType(fieldType), id(id) { }
    AttrEntry(FieldType fieldType, char *id, char *tbname, char *colname) : id(id) {
        refTbname = tbname;
        refColname = colname;
    }
    AttrEntry(FieldType fieldType, char *id, AttrType attrType, int attrLength) : fieldType(fieldType), id(id), attrType(attrType), attrLength(attrLength) {
        notNull = CAN_BE_NULL;
        isPrimary = NOT_PRIMARY;
        isForeign = NOT_FOREIGN;
    }
    char *id, *refTbname, *refColname;
    FieldType fieldType;
    AttrType attrType;
    int attrLength;
    NotNull notNull;
    IsPrimary isPrimary;
    IsForeign isForeign;
};


class AttrList {

public:
    void process(AttrEntry attrEntry) {
        switch (attrEntry.fieldType) {

        case AttrEntry::NORMAL: {
            attrList.push_back(attrEntry);
            break;
        }

        case AttrEntry::PRIMARY: {
            for (int i = 0; i < attrList.size(); ++i)
            if (strcmp(attrList[i].id, attrEntry.id) == 0) {
                attrList[i].isPrimary = IS_PRIMARY;
            }
            break;
        }

        case AttrEntry::FOREIGN: {
            for (int i = 0; i < attrList.size(); ++i)
            if (strcmp(attrList[i].id, attrEntry.id) == 0) {
                attrList[i].isForeign = IS_FOREIGN;
                attrList[i].refTbname = attrEntry.refTbname;
                attrList[i].refColname = attrEntry.refColname;
            }
            break;
        }

        }
    }

    std::vector<AttrEntry> attrList;
};


class CondEntry {

public:
    enum CalcOp {
        EQUAL,
        NOT_EQUAL,
        LESS_EQUAL,
        GREATR_EQUAL,
        LESS,
        GREATER,
        IS_NULL,
        IS_NOT_NULL,
        AND
    };
    CondEntry() {
        leftTbname = NULL;
        leftColname = NULL;
        rightTbname = NULL;
        rightColname = NULL;
        leftValue = NULL;
        rightValue = NULL;
    }
    CalcOp calcOp;
    char *leftTbname, *leftColname, *rightTbname, *rightColname;
    void *leftValue, *rightValue;
    CondEntry leftEntry, rightEntry;
};


class SetList {

public:
    void clear() {
        colnameList.clear();
        valueList.clear();
    }
    void add(char *colname, void *value) {
        colnameList.push_back(colname);
        valueList.push_back(value);
    }
    std::vector<char*> colnameList;
    std::vector<void*> valueList;
};


class SelectList {

public:
    enum SelectType {
        ALL,
        NORMAL
    };
    void clear() {
        selectType = NORMAL;
        tbnameList.clear();
        colnameList.clear();
    }
    void add(char *tbname, char *colname) {
        tbnameList.push_back(tbname);
        colnameList.push_back(colname);
    }
    SelectType selectType;
    std::vector<char*> tbnameList;
    std::vector<char*> colnameList;
};


class SemValue {

public:
    SemValue();
    int code;
    CompOp compOp;
    char *id; /* name of identifier */
    char *tbname, *colname;
    void *value;
    AttrType attrType;
    int attrLength;
    AttrEntry attrEntry;
    AttrList attrList;
    std::vector<void*> values;
    std::vector<std::vector<void*>> valuesList;
    CondEntry condEntry;
    SetList setList;
    SelectList selectList;
    std::vector<char*> tableList;
    static int keyword(int code);
};

#endif