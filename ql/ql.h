#ifndef QL_H
#define QL_H

#include "../utils/defs.h"
#include "../rm/rm.h"
#include "../ix/ix.h"
#include "../sm/sm.h"


struct RelAttr {
    char *relName;
    char *attrName;
};


struct Value {
    AttrType type;
    void *data;
};


struct Condition {
    RelAttr lhsAttr;
    CompOp op;
    int bRhsIsAttr;

    RelAttr rhsAttr;
    Value rhsValue;
};


struct Catalog {
    RelcatLayout relcat;
    AttrcatLayout attrcats[MAXATTRS];
};


struct SearchStrategy {
    Attrcat *attrcat;  // NULL means no fast-searching strategy
    CompOp compOp;
    Value value;  // NULL means not decided until value of the other relation's "auxAttrName" is decided (cross-relation attribute)
    Attrcat *auxAttrcat;
};


struct SelectStrategy {
    SearchStrategy strat1, strat2;
    int outer;  // 0/1/2; which relation to put in outer loop; if 0, adopt post-join strategy
};


class QL_Manager {

public:
    QL_Manager(SM_Manager &smm, IX_Manager &ixm, RM_Manager &rmm);
    ~QL_Manager();

    RC select(int nSelAttrs, const RelAttr selAttrs[],
                const char* relation1, const char *relation2, const JoinType joinType,
                int nRelations, const char * const relations[],
                int nConditions,
                const Condition conditions[]);
    void insert(const char *relName, int nValues, const Value values[]);
    void del(const char *relName, int nConditions, const Condition conditions[]);
    void update(const char *relName, const RelAttr &updAttr, const int bIsValue,
                const RelAttr &rhsRelAttr, const Value &rhsValue,
                int nConditions, const Condition conditions[]);

private:
    char *getPath(const char *dbName, const char *relName);
    void padName(char name[MAXNAME + 1], char padding = ' ');

    char *dbName;
    char pathBuf[MAXNAME * 2 + 10];
};


#endif
