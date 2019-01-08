#ifndef RM_H
#define RM_H

#include <map>
#include "../utils/defs.h"
#include "../utils/utils.h"
#include "rm_rid.h"
#include "../fs/bufmanager/BufPageManager.h"


struct FileHeaderPage {
    int recordSize, availPageCnt;
    PageNum firstFree, lastFree;
};


struct PageHeader {
    PageNum prevFree, nextFree;
    BitMap bitmap;
};


class RM_Record {

public:
    RM_Record();
    RM_Record(int size, const RID &rid);
    ~RM_Record();
    RM_Record &operator=(const RM_Record &rec);

    char *getData() const;
    RID getRid() const;
    void setRid(const RID &rid);

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
    // bool getFirstRid(RID &rid) const;
    void startVisiting();
    bool getNextRid(RID &rid);
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
    BitMap bmForScan;
    int pnForScan;

};


class RM_Manager {

public:
    RM_Manager(BufPageManager *bpm);
    ~RM_Manager();

    RC createFile(const char *fileName, int recordSize);
    bool createDir(const char *dirName);
    bool deleteFile(const char *fileName);
    bool deleteDir(const char *dirName);
    bool openFile(const char *fileName, RM_FileHandle *&fileHandle);
    bool closeFile(RM_FileHandle &fileHandle);
    std::vector<std::string> listDir(const char *dirName);

private:
    void clearPool();

    BufPageManager *bpm;
    static const int MAX_POOL_SIZE;
    std::map<std::string, RM_FileHandle *> handlePool;

};


class RM_FileScan {

public:
    RM_FileScan();
    ~RM_FileScan();

    void openScan(RM_FileHandle &fileHandle, AttrType attrType,
                int attrLength, int attrOffset, CompOp compOp, const void *value);
    RC getNextRec(RM_Record &rec);
    RC closeScan();

private:
    RM_FileHandle *fileHandle;
    AttrType attrType;
    int attrLength, attrOffset;
    CompOp compOp;
    const void *value;
    bool open;

};


#endif
