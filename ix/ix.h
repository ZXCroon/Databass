#ifndef IX_H
#define IX_H

#include "../utils/defs.h"
#include "../rm/rm_rid.h"
#include "../fs/bufmanager/BufPageManager.h"


struct FileHeaderPage {
    int recordSize, availPageCnt;
    PageNum firstFree, lastFree;
};


struct PageHeader {
    PageNum prevFree, nextFree;
    Bits bitmap, nullmap;
};


class IX_IndexHandle {

public:
    IX_IndexHandle(BufPageManager *bpm, int fileId);
    ~IX_IndexHandle();

    bool insertEntry(void *pData, const RID &rid);
    bool deleteEntry(void *pData, const RID &rid);
    int getFileId() const;

private:
    PageNum getFreePage();
    char *getPageData(PageNum pageNum, bool write) const;
    bool insertFreePage(PageNum pageNum, bool isNew) const;
    bool removeFreePage(PageNum pageNum) const;
    BufPageManager *bpm;
    const int fileId;
    int recordSize, availPageCnt;
};

class IX_IndexScan {

public:
    IX_IndexScan();
    ~IX_IndexScan();

    RC openScan(const IX_IndexHandle &indexHandle, CompOp compOp, void *value);
    RC getNextEntry(RID &rid);
    RC closeScan();

private:
    const IX_IndexHandle *indexHandle;
    AttrType attrType;
    int attrLength;
    CompOp compOp;
    void *value;
    RID rid;
    bool open, start;
};

class IX_Manager {

public:
    IX_Manager(BufPageManager &bpm);
    ~IX_Manager();

    RC createIndex(const char *filename, int indexNo, AttrType attrType, int attrLength);
    bool openIndex(const char *filename, int indexNo, IX_IndexHandle *&indexHandle);
    bool closeIndex(IX_IndexHandle &indexHandle);

private:
    const char* getIndexFilename(const char* filename, int indexNo);

    BufPageManager *bpm;
};


#endif