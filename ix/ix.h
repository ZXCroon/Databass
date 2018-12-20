#ifndef IX_H
#define IX_H

#include <assert.h>

#include "../utils/defs.h"
#include "../rm/rm_rid.h"
#include "../fs/bufmanager/BufPageManager.h"


struct IX_FileHeaderPage {
    int recordSize, availPageCnt;
    RID root;
    AttrType attrType;
    int attrLength;
    PageNum firstFree, lastFree;
};


struct IX_PageHeader {
    PageNum prevFree, nextFree;
    Bits bitmap, nullmap;
};


class IX_Record {

public:
    IX_Record();
    IX_Record(int size, int attrLength, const RID &rid);
    ~IX_Record();

    int *getIsLeaf() const;
    int *getSize() const;
    RID *getIndexRID(int i) const;
    RID *getChild(int i) const;
    RID *getFather() const;
    RID *getPrev() const;
    RID *getNext() const;
    void *getIndexValue(int i) const;

    char *getData() const;
    RID getRid() const;
    void nullify();

private:
    int size, attrLength;
    char *pData;
    RID rid;
};

// the alignment of pData: bool isLeaf; int size(of indexes); RID indexRID[4], child[5], father, prev, next; AttrType indexValue
struct IX_Bnode {
    int isLeaf;
    int size;
    RID indexRID[4], child[5], father, prev, next;
    void *indexValue[4];
};


class IX_IndexHandle {

public:
    IX_IndexHandle(BufPageManager *bpm, int fileId);
    ~IX_IndexHandle();

    bool insertEntry(void *pData, const RID &rid);
    bool deleteEntry(void *pData, const RID &rid);

    bool getRec(const RID &rid, IX_Record &rec) const;
    bool insertRec(const char *pData, RID &rid);
    bool deleteRec(const RID &rid);
    bool updateRec(const IX_Record &rec);

    int getFileId() const;
    RID getRoot() const;
    AttrType getAttrType() const;
    int getAttrLength() const;

    bool findInsertPos(RID u, void *pData, const RID &rid, RID &res, int &pos) const;
    void searchFirst(RID u, RID &res, int &pos) const;
    void searchGE(RID u, void *pData, const RID &rid, RID &res, int &pos) const;
    void searchLT(RID u, void *pData, const RID &rid, RID &res, int &pos) const;
    void searchNext(RID &rid, int &pos, bool direct) const;
    void deleteNode(RID &u, RID & v);

    bool indexEQ(void *data1, RID rid1, void *data2, RID rid2) const;
    bool indexGE(void *data1, RID rid1, void *data2, RID rid2) const;
    bool indexGT(void *data1, RID rid1, void *data2, RID rid2) const;
    bool indexLE(void *data1, RID rid1, void *data2, RID rid2) const;
    bool indexLT(void *data1, RID rid1, void *data2, RID rid2) const;

private:
    PageNum getFreePage();
    char *getPageData(PageNum pageNum, bool write) const;
    bool insertFreePage(PageNum pageNum, bool isNew) const;
    bool removeFreePage(PageNum pageNum) const;

    inline int getSlotOffset(SlotNum slotNum) const {
        return sizeof(IX_PageHeader) + slotNum * recordSize;
    }

    BufPageManager *bpm;
    const int fileId;
    int recordSize, maxRecordCnt, availPageCnt;
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
    CompOp compOp;
    void *value;
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
    bool deleteIndex(const char *filename, int indexNo);
    bool openIndex(const char *filename, int indexNo, IX_IndexHandle *&indexHandle);
    bool closeIndex(IX_IndexHandle &indexHandle);

private:
    const char* getIndexFilename(const char* filename, int indexNo);

    BufPageManager *bpm;
};


#endif