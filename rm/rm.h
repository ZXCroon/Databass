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
    BufPageManager bpm;

};


class RM_FileHandle {

public:
    RM_FileHandle(unsigned int fileId);
    ~RM_FileHandle();

    RC getRec(const RID &rid, RM_Record &rec) const;
    RC insertRec(const char *pData, RID &rid);
    RC deleteRec(const RID &rid);
    RC updateRec(const RM_Record &rec);
    RC ForcePages(PageNum pageNum = ALL_PAGES) const;
    RC getFirstRid(RID &rid) const;
    RC getNextRid(const RID &lastRid, RID &rid) const;

    unsigned int fileId;

};


class RM_Record {

public:
    RM_Record();
    ~RM_Record();

    RC getData(char *&pData) const;
    RC getRid(RID &rid) const;

    friend RC RM_FileHandle::getRec(const RID &rid, RM_Record &rec);

private:
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


struct HeaderPage {
    unsigned int recordSize;
};


#endif
