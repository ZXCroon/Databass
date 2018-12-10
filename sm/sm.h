#ifndef SM_H
#define SM_H

#include "../utils/defs.h"
#include "../rm/rm.h"
#include "../ix/ix.h"


struct AttrInfo {
    char attrName[MAXNAME + 1];
    AttrType attrType;
    int attrLength;
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
    int offset;
    AttrType attrType;
    int attrLength;
    int indexNo;
};


class SM_Manager {

public:
    SM_Manager(const IX_Manager *&ixm, const RM_Manager *&rmm);
    ~SM_Manager();

    bool createDb(const char *dbName);
    // bool dropDb(const char *dbName);
    bool openDb(const char *dbName);
    bool closeDb();
    bool createTable(const char *relName, int attrCount, AttrInfo *attributes);
    bool dropTable(const char *relName);
    bool createIndex(const char *relName, const char *attrName);
    bool dropIndex(const char *relName, const char *attrName);

private:
    char *getPath(const char *dbName, const char *relName);
    void padName(char name[MAXNAME + 1], char padding = ' ') {

    IX_Manager *ixm;
    RM_Manager *rmm;
    RM_FileHandle *relcatHandle, attrcatHandle;
    char *dbNameBuf, *pathBuf;
    char dbNameBuf[MAXNAME + 1], pathBuf[MAXNAME * 2 + 10];
};


#endif
