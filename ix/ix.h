#ifndef IX_H
#define IX_H

#include "../utils/defs.h"
#include "../rm/rm_rid.h"
#include "../fs/bufmanager/BufPageManager.h"


class IX_IndexHandle {

public:
    IX_IndexHandle();
    ~IX_IndexHandle();

    RC insertEntry(void *pData, const RID &rid);
    RC deleteEntry(void *pData, const RID &rid);
    RC forcePages();
};

class IX_IndexScan {

public:
    IX_IndexScan();
    ~IX_IndexScan();

    RC openScan(const IX_IndexHandle &indexHandle, CompOp compOp, void *value);
    RC getNextEntry(RID &rid);
    RC closeScan();
};

class IX_Manager {

public:
    IX_Manager(BufPageManager &bpm);
    ~IX_Manager();

    RC createIndex(const char *filename, int indexNo, AttrType attrType, int attrLength);
    RC destroyIndex(const char *filename, int indexNo);
    RC openIndex(const char *filename, int indexNo, IX_IndexHandle &indexHandle);
    RC closeIndex(IX_IndexHandle &indexHandle);

private:
    BufPageManager *bpm;
};


#endif