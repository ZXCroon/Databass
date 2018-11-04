#include <cstring>
#include "rm.h"


RM_FileHandle::RM_FileHandle(BufPageManager *&bpm, int fileId) : bpm(bpm), fileId(fileId) {
    FileHeaderPage *header = (FileHeaderPage *)getPageData(0, false);
    recordSize = header->recordSize;
    maxRecordCnt = (PAGE_SIZE - sizeof(PageHeader)) / recordSize;
    availPageCnt = header->availPageCnt;
}


RM_FileHandle::~RM_FileHandle() {}


bool RM_FileHandle::getRec(const RID &rid, RM_Record &rec) const {
    PageNum pageNum = rid.getPageNum();
    SlotNum slotNum = rid.getSlotNum();
    char *page = getPageData(pageNum, false);
    rec = RM_Record(recordSize, rid);
    if (((PageHeader *)page)->bitmap & (1 << slotNum)) {
        if (((PageHeader *)page)->nullmap & (1 << slotNum)) {
            rec.nullify();
        } else {
            memcpy(rec.getData(), page + getSlotOffset(slotNum), recordSize);
        }
        return true;
    } else {
        return false;
    }
}


bool RM_FileHandle::insertRec(const char *pData, RID &rid) {
    PageNum pageNum = getFreePage();
    PageHeader *ph = (PageHeader *)getPageData(pageNum, false);
    SlotNum slotNum = 0;
    for (Bits x = ph->bitmap; x & 1; (x <<= 1), ++slotNum);
    ph = (PageHeader *)getPageData(pageNum, true);
    rid = RID(pageNum, slotNum);
    if (pData != NULL) {
        memcpy(((char *)ph) + getSlotOffset(slotNum), pData, recordSize);
        ph->nullmap &= ~(1 << slotNum);
    } else {
        memset(((char *)ph) + getSlotOffset(slotNum), 0, recordSize);
        ph->nullmap |= (1 << slotNum);
    }
    ph->bitmap |= (1 << slotNum);
    if ((ph->bitmap & ((1 << maxRecordCnt) - 1)) == ((1 << maxRecordCnt) - 1)) {
        return removeFreePage(pageNum);
    }
}


bool RM_FileHandle::deleteRec(const RID &rid) {
    PageNum pageNum = rid.getPageNum();
    SlotNum slotNum = rid.getSlotNum();
    PageHeader *ph = (PageHeader *)getPageData(pageNum, false);
    bool oriFull = ((ph->bitmap & ((1 << maxRecordCnt) - 1)) == ((1 << maxRecordCnt) - 1));
    if ((ph->bitmap | ~(1 << slotNum)) == 0) {
        return false;
    }
    ph = (PageHeader *)getPageData(pageNum, true);
    ph->bitmap &= ~(1 << slotNum);
    ph->nullmap &= ~(1 << slotNum);
    if (oriFull) {
        return insertFreePage(pageNum, false);
    }
}


bool RM_FileHandle::updateRec(const RM_Record &rec) {
    RID rid = rec.getRid();
    PageNum pageNum = rid.getPageNum();
    SlotNum slotNum = rid.getSlotNum();
    PageHeader *ph = (PageHeader *)getPageData(pageNum, false);
    if ((ph->bitmap | ~(1 << slotNum)) == 0) {
        return false;
    }
    char *page = getPageData(pageNum, true);
    if (rec.getData() != NULL) {
        memcpy(page + getSlotOffset(slotNum), rec.getData(), recordSize);
        ((PageHeader *)page)->nullmap &= ~(1 << slotNum);
    } else {
        memset(page + getSlotOffset(slotNum), 0, recordSize);
        ((PageHeader *)page)->nullmap |= (1 << slotNum);
    }
    return true;
}


bool RM_FileHandle::getFirstRid(RID &rid) const {
    RID tmpRid(1, -1);
    return getNextRid(tmpRid, rid);
}


bool RM_FileHandle::getNextRid(const RID &lastRid, RID &rid) const {
    PageNum pageNum = lastRid.getPageNum();
    SlotNum slotNum = lastRid.getSlotNum() + 1;
    for (; pageNum <= availPageCnt; ++pageNum) {
        PageHeader *ph = (PageHeader *)getPageData(pageNum, false);
        Bits bitmap = ph->bitmap;
        for (; slotNum < maxRecordCnt; ++slotNum) {
            if (bitmap & (1 << slotNum)) {
                rid = RID(pageNum, slotNum);
                return true;
            }
        }
        slotNum = 0;
    }
    return false;
}


int RM_FileHandle::getFileId() const {
    return fileId;
}


PageNum RM_FileHandle::getFreePage() {
    FileHeaderPage *header = (FileHeaderPage *)getPageData(0, false);
    if (header->firstFree == NO_PAGE) {
        ++availPageCnt;
        header = (FileHeaderPage *)getPageData(0, true);
        ++header->availPageCnt;
        insertFreePage(header->availPageCnt, true);
    }
    return header->firstFree;
}


char *RM_FileHandle::getPageData(PageNum pageNum, bool write) const {
    int index;
    char *bpmBuf = (char *)(bpm->getPage(fileId, pageNum, index));
    if (write) {
        bpm->markDirty(index);
    }
    return bpmBuf;
}


bool RM_FileHandle::insertFreePage(PageNum pageNum, bool isNew) const {
    FileHeaderPage *header = (FileHeaderPage *)getPageData(0, true);
    PageHeader *ph = (PageHeader *)getPageData(pageNum, true);
    if (isNew) {
        ph->bitmap = 0;
        ph->prevFree = ph->nextFree = NO_PAGE;
    }
    if (header->firstFree == NO_PAGE) {
        header->firstFree = header->lastFree = pageNum;
        ph->prevFree = ph->nextFree = NO_PAGE;
    } else {
        if (header->firstFree == pageNum || ph->prevFree != NO_PAGE || ph->nextFree != NO_PAGE) {
            return false;
        }
        PageHeader *fph = (PageHeader *)getPageData(header->firstFree, true);
        fph->prevFree = pageNum;
        ph->nextFree = header->firstFree;
        ph->prevFree = NO_PAGE;
        header->firstFree = pageNum;
    }
    return true;
}


bool RM_FileHandle::removeFreePage(PageNum pageNum) const {
    FileHeaderPage *header = (FileHeaderPage *)getPageData(0, true);
    PageHeader *ph = (PageHeader *)getPageData(pageNum, true);
    if (ph->prevFree == NO_PAGE && ph->nextFree == NO_PAGE && header->firstFree != pageNum) {
        return false;
    }
    if (ph->prevFree == NO_PAGE) {
        header->firstFree = ph->nextFree;
    } else {
        PageHeader *pph = (PageHeader *)getPageData(ph->prevFree, true);
        pph->nextFree = ph->nextFree;
        ph->prevFree = NO_PAGE;
    }
    if (ph->nextFree == NO_PAGE) {
        header->lastFree = ph->prevFree;
    } else {
        PageHeader *nph = (PageHeader *)getPageData(ph->nextFree, true);
        nph->prevFree = ph->prevFree;
        ph->nextFree = NO_PAGE;
    }
    return true;
}
