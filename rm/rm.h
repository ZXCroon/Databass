#ifndef RM_H
#define RM_H

#include "../utils/defs.h"
#include "rm_rid.h"
#include "../fs/bufmanager/BufPageManager.h"


struct FileHeaderPage {
    int recordSize, availPageCnt;
    PageNum firstFree, lastFree;
};


struct PageHeader {
    PageNum prevFree, nextFree;
    Bits bitmap, nullmap;
};


class RM_Record {

public:
    RM_Record(int size, const RID &rid);
    ~RM_Record();

    char *getData() const;
    RID getRid() const;
    void nullify();

private:
    int size;
    char *pData;
    RID rid;

};


class RM_FileHandle {

public:
    RM_FileHandle(BufPageManager *&bpm, int fileId);
    ~RM_FileHandle();

    bool getRec(const RID &rid, RM_Record &rec) const;
    bool insertRec(const char *pData, RID &rid);
    bool deleteRec(const RID &rid);
    bool updateRec(const RM_Record &rec);
    bool getFirstRid(RID &rid) const;
    bool getNextRid(const RID &lastRid, RID &rid) const;
    int getFileId() const;

private:
    PageNum getFreePage();
    char *getPageData(PageNum pageNum, bool write) const;
    bool insertFreePage(PageNum pageNum, bool isNew) const;
    bool removeFreePage(PageNum pageNum) const;

    inline int getSlotOffset(SlotNum slotNum) const {
        return sizeof(PageHeader) + slotNum * recordSize;
    }

    BufPageManager *bpm;
    const int fileId;
    int recordSize, maxRecordCnt, availPageCnt;

};


class RM_Manager {

public:
    RM_Manager(BufPageManager &bpm);
    ~RM_Manager();

    RC createFile(const char *fileName, int recordSize);
    bool openFile(const char *fileName, RM_FileHandle *&fileHandle);
    bool closeFile(RM_FileHandle &fileHandle);

private:
    BufPageManager *bpm;

};


class RM_FileScan {

public:
    RM_FileScan();
    ~RM_FileScan();

    void openScan(const RM_FileHandle &fileHandle, AttrType attrType,
                int attrLength, int attrOffset, CompOp compOp, void *value);
    RC getNextRec(RM_Record &rec);
    RC closeScan();

private:
    bool validate(char *pData, AttrType attrType, int attrLength,
                  CompOp compOp, void *value);
    const RM_FileHandle *fileHandle;
    AttrType attrType;
    int attrLength, attrOffset;
    CompOp compOp;
    void *value;
    RID rid;
    bool open, start;

};


#endif
