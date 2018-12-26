#ifndef SM_H
#define SM_H

#include "../utils/defs.h"
#include "../rm/rm.h"
#include "../ix/ix.h"


struct AttrInfo {
    char attrName[MAXNAME + 1];
    AttrType attrType;
    int attrLength;
    char refTbname[MAXNAME + 1], refColname[MAXNAME + 1];
    bool notNull, isPrimary, isForeign;
};


struct DataAttrInfo {
    char relName[MAXNAME + 1];
    char attrName[MAXNAME + 1];
    int offset;
    AttrType attrType;
    int attrLength;
    int indexNo;
};


struct RelcatLayout {
    char relName[MAXNAME + 1];
    unsigned char attrCount;
    int tupleLength;
    int nextIndex;
};


struct AttrcatLayout {
    char relName[MAXNAME + 1];
    char attrName[MAXNAME + 1];
    char refRelName[MAXNAME + 1];
    char refAttrName[MAXNAME + 1];
    int offset;
    AttrType attrType;
    int attrLength;
    int indexNo;
    int constrFlag;  // bit 0: notNull; bit 1: isPrimary; bit 2: isForeign
};


class QL_Manager;

class SM_Manager {

public:
    SM_Manager(IX_Manager *ixm, RM_Manager *rmm);
    ~SM_Manager();

    bool createDb(const char *dbName);
    bool dropDb(const char *dbName);
    bool openDb(const char *dbName);
    bool closeDb();
    void showCurrentDb();
    bool showDb(const char *dbName);
    void showDbs();
    bool createTable(const char *relName, int attrCount, AttrInfo *attributes);
    bool dropTable(const char *relName);
    bool showTable(const char *relName);
    bool showTables();
    bool createIndex(const char *relName, const char *attrName);
    bool dropIndex(const char *relName, const char *attrName);

    friend class QL_Manager;

private:
    char *getPath(const char *dbName, const char *relName);
    void padName(char name[MAXNAME + 1], char padding = ' ');
    bool getRelcatRid(const char *relName, RID &rid);
    bool getAttrcatRids(const char *relName, const char *attrName, RID *rids, int &ridCount);
    void getRelcatFromRid(const RID &rid, RelcatLayout &relcat);
    void getAttrcatFromRid(const RID &rid, AttrcatLayout &attrcat);

    IX_Manager *ixm;
    RM_Manager *rmm;
    RM_FileHandle *relcatHandle, *attrcatHandle;
    char dbName[MAXNAME + 1], pathBuf[MAXNAME * 2 + 10];
};


#endif
