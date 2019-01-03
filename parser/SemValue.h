#ifndef SEMVALUE_H
#define SEMVALUE_H

#include "../utils/defs.h"
#include "../sm/sm.h"
#include "../ql/ql.h"
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
        notNull = false;
        isPrimary = false;
        isForeign = false;
    }
    AttrEntry(FieldType fieldType, char *id) : fieldType(fieldType), id(id) { }
    AttrEntry(FieldType fieldType, char *id, char *tbname, char *colname) : id(id) {
        refTbname = tbname;
        refColname = colname;
    }
    AttrEntry(FieldType fieldType, char *id, AttrType attrType, int attrLength) : fieldType(fieldType), id(id), attrType(attrType), attrLength(attrLength) {
        notNull = false;
        isPrimary = false;
        isForeign = false;
    }
    char *id, *refTbname, *refColname;
    FieldType fieldType;
    AttrType attrType;
    int attrLength;
    bool notNull, isPrimary, isForeign;
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
                attrList[i].isPrimary = true;
            }
            break;
        }

        case AttrEntry::FOREIGN: {
            for (int i = 0; i < attrList.size(); ++i)
            if (strcmp(attrList[i].id, attrEntry.id) == 0) {
                attrList[i].isForeign = true;
                attrList[i].refTbname = attrEntry.refTbname;
                attrList[i].refColname = attrEntry.refColname;
            }
            break;
        }

        }
    }

    void clear() {
        attrList.clear();
    }

    std::vector<AttrEntry> attrList;
};

/*
class CondEntry {

public:
    enum CalcOp {
        EQUAL,
        NOT_EQUAL,
        LESS_EQUAL,
        GREATER_EQUAL,
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
    };
    CompOp compOp;
    char *leftTbname, *leftColname, *rightTbname, *rightColname;
    void *leftValue, *rightValue;
    CondEntry *leftEntry, *rightEntry;
};*/


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
        attrList.clear();
    }
    void add(char *tbname, char *colname) {
        RelAttr relAttr = {tbname, colname};
        attrList.push_back(relAttr);
    }
    SelectType selectType;
    std::vector<RelAttr> attrList;
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
    int tupleSize;
    std::vector<void*> values;
    std::vector<AttrType> valueTypes;
    std::vector<Condition> conditionList;
    SetList setList;
    SelectList selectList;
    std::vector<char*> tableList;
    JoinType joinType;
    static int keyword(int code);
};

#endif
