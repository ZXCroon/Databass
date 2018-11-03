#ifndef RM_H
#define RM_H

#include "utils/defs.h"
#include "rm_rid.h"
#include "fs/bufmanager/BufPageManager.h"


class RM_Manager {

public:
    RM_Manager(BufPageManager &bpm);
    ~RM_Manager();

    RC createFile(const char *fileName, int recordSize);
    RC openFile(const char *fileName, RM_FileHandle &fileHandle);
    RC closeFile(RM_FileHandle &fileHandle);

private:
    BufPageManager &bpm;

};


class RM_FileHandle {

public:
    RM_FileHandle(BufPageManager &bpm, unsigned int fileId);
    ~RM_FileHandle();

    RC getRec(const RID &rid, RM_Record &rec) const;
    RC insertRec(const char *pData, RID &rid);
    RC deleteRec(const RID &rid);
    RC updateRec(const RM_Record &rec);
    RC getFirstRid(RID &rid) const;
    RC getNextRid(const RID &lastRid, RID &rid) const;

private:
    inline int getSlotOffset(SlotNum slotNum) {
        return sizeof(PageHeader) + slotNum * recordSize;
    }
    BufPageManager &bpm;
    unsigned int fileId;
    int recordSize, maxRecordCnt, availPageCnt;

};


class RM_Record {

public:
    RM_Record();
    RM_Record(int size, const RID &rid);
    ~RM_Record();

    char *getData();
    RID getRid();

private:
    int size;
    char *pData;
    RID rid;

};


class RM_FileScan {

public:
    RM_FileScan();
    ~RM_FileScan();

    RC openScan(const RM_FileHandle &fileHandle, AttrType attrType,
                int attrLength, int attrOffset, CompOp compOp, void *value);
    RC getNextRec(RM_Record &rec);
    RC closeScan();

private:
    bool validate(char *pData, AttrType attrType, int attrLength,
                  CompOp compOp, void *value);
    RM_FileHandle *fileHandle;
    AttrType attrType;
    int attrLength, attrOffset;
    CompOp compOp;
    void *value;
    RID rid;
    bool open, start;

};


struct FileHeaderPage {
    unsigned int recordSize, availPageCnt;
    PageNum firstFree, lastFree;
};


struct PageHeader {
    PageNum prevFree, nextFree;
    Bits bitmap;
};


#endif
