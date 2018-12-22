#ifndef QL_H
#define QL_H

#include "../utils/defs.h"
#include "../rm/rm.h"
#include "../ix/ix.h"
#include "../sm/sm.h"


struct RelAttr {
    const char *relName;
    const char *attrName;
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
    const AttrcatLayout *attrcat;  // NULL means no fast-searching strategy
    CompOp compOp;
    Value value;  // NULL means not decided until value of the other relation's "auxAttrName" is decided (cross-relation attribute)
    const AttrcatLayout *auxAttrcat;
};


struct SelectStrategy {
    SearchStrategy strat1, strat2;
    int outer;  // 0/1/2; which relation to put in outer loop; if 0, adopt post-join strategy
};


class QL_Manager {

public:
    QL_Manager(SM_Manager *&smm, IX_Manager *&ixm, RM_Manager *&rmm);
    ~QL_Manager();

    RC select(int nSelAttrs, const RelAttr selAttrs[],
              const char* relation1, const char *relation2, const JoinType &joinType,
              int nRelations, const char * const relations[],
              int nConditions,
              const Condition conditions[]);
    void insert(const char *relName, int nValues, const Value values[]);
    void del(const char *relName, int nConditions, const Condition conditions[]);
    void update(const char *relName, const RelAttr &updAttr, const int bIsValue,
                const RelAttr &rhsRelAttr, const Value &rhsValue,
                int nConditions, const Condition conditions[]);

private:
    bool getCatalog(const char *relName, Catalog &cat);
    const AttrcatLayout *locateAttrcat(const char *relName, const Catalog &cat, const RelAttr &ra);
    char *getPath(const char *dbName, const char *relName);
    void *padValue(void *value, AttrType attrType, int attrLength);
    bool decideStrategy(const char *relation1, const char *relation2,
                        const Catalog &cat1, const Catalog &cat2, const JoinType &joinType,
                        int nConditions, const Condition conditions[], SelectStrategy &strat);
    bool singleValidate(const char *relation, const Catalog &cat, int nConditions, const Condition *conditions, const RM_Record &rec);
    bool pairValidate(const char *relation1, const char *relation2,
                      const Catalog &cat1, const Catalog &cat2,
                      int nConditions, const Condition *conditions,
                      const RM_Record &rec1, const RM_Record &rec2);

    SM_Manager *smm;
    IX_Manager *ixm;
    RM_Manager *rmm;
    char *dbName;
    char valBuf[MAXSTRINGLEN];
    char pathBuf[MAXNAME * 2 + 10];
};


#endif