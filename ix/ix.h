#ifndef IX_H
#define IX_H

#include "../utils/defs.h"
#include "../rm/rm_rid.h"
#include "../fs/bufmanager/BufPageManager.h"


struct FileHeaderPage {
    int recordSize, availPageCnt;
    RID root;
    PageNum firstFree, lastFree;
};


struct PageHeader {
    PageNum prevFree, nextFree;
    Bits bitmap, nullmap;
};


class IX_Record {

public:
    IX_Record(int size, const RID &rid);
    ~IX_Record();

    char *getData() const;
    RID getRid() const;
    void nullify();

private:
    int size;
    char *pData;
    RID rid;

};


struct IX_Bnode {
    bool isLeaf;
    void *indexValue[4];
    RID indexRID[4], child[5], father, prev, next;
    int size;

    IX_Bnode() {
        isLeaf = true;
        for (int i = 0; i < 4; ++i) {
            indexValue[i] = NULL;
            indexRID[i] = RID(-1, -1);
        }
        for (int i = 0; i < 5; ++i) {
            child[i] = RID(-1, -1);
        }
        father = RID(-1, -1);
        prev = RID(-1, -1);
        next = RID(-1, -1);
        size = 0;
    }
};


class IX_IndexHandle {

public:
    IX_IndexHandle(BufPageManager *bpm, int fileId);
    ~IX_IndexHandle();

    bool insertEntry(void *pData, const RID &rid);
    bool deleteEntry(void *pData, const RID &rid);
    int getFileId() const;
    RID getRoot() const;
    AttrType getAttrType() const;
    int getAttrLength() const;

    bool findInsertPos(RID u, void *pData, const RID &rid, RID &res, int &pos) const;
    void searchFirst(RID u, RID &res, int &pos) const;
    void searchGE(RID u, void *pData, const RID &rid, RID &res, int &pos) const;
    void searchLT(RID u, void *pData, const RID &rid, RID &res, int &pos) const;
    void searchNext(RID &rid, int &pos, bool direct) const;

private:
    PageNum getFreePage();
    char *getPageData(PageNum pageNum, bool write) const;
    bool insertFreePage(PageNum pageNum, bool isNew) const;
    bool removeFreePage(PageNum pageNum) const;

    BufPageManager *bpm;
    const int fileId;
    int recordSize, availPageCnt;
    AttrType attrType;
    int attrLength;
    RID root;
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
    RID res;
    int pos;
    int direct;
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