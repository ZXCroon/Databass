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
    if (queryBit(((PageHeader *)page)->bitmap, slotNum) == 1) {
        memcpy(rec.getData(), page + getSlotOffset(slotNum), recordSize);
        return true;
    } else {
        return false;
    }
}


bool RM_FileHandle::insertRec(const char *pData, RID &rid) {
    PageNum pageNum = getFreePage();
    PageHeader *ph = (PageHeader *)getPageData(pageNum, false);
    SlotNum slotNum = findRightMost(ph->bitmap, 0);
    ph = (PageHeader *)getPageData(pageNum, true);
    rid = RID(pageNum, slotNum);
    memcpy(((char *)ph) + getSlotOffset(slotNum), pData, recordSize);
    setBit(ph->bitmap, slotNum, 1);
    int tmp = findRightMost(ph->bitmap, 0);
    if (tmp == -1 || tmp >= maxRecordCnt) {
        return removeFreePage(pageNum);
    }
    return true;
}


bool RM_FileHandle::deleteRec(const RID &rid) {
    PageNum pageNum = rid.getPageNum();
    SlotNum slotNum = rid.getSlotNum();
    PageHeader *ph = (PageHeader *)getPageData(pageNum, false);
    // bool oriFull = ((ph->bitmap & ((1 << maxRecordCnt) - 1)) == ((1 << maxRecordCnt) - 1));
    bool oriFull = findRightMost(ph->bitmap, 0) == -1;
    if (queryBit(ph->bitmap, slotNum) == 0) {
        return false;
    }
    ph = (PageHeader *)getPageData(pageNum, true);
    setBit(ph->bitmap, slotNum, 0);
    if (oriFull) {
        return insertFreePage(pageNum, false);
    }
    return true;
}


bool RM_FileHandle::updateRec(const RM_Record &rec) {
    RID rid = rec.getRid();
    PageNum pageNum = rid.getPageNum();
    SlotNum slotNum = rid.getSlotNum();
    PageHeader *ph = (PageHeader *)getPageData(pageNum, false);
    if (queryBit(ph->bitmap, slotNum) == 0) {
        return false;
    }
    char *page = getPageData(pageNum, true);
    if (rec.getData() != NULL) {
        memcpy(page + getSlotOffset(slotNum), rec.getData(), recordSize);
    } else {
        return false;
    }
    return true;
}


void RM_FileHandle::startVisiting() {
    pnForScan = 1;
    BitMap bm = ((PageHeader *)getPageData(1, false))->bitmap;
    memcpy((char *)(&bmForScan), (char *)(&bm), sizeof(bmForScan));
}


bool RM_FileHandle::getNextRid(RID &rid) {
    if (pnForScan > availPageCnt) {
        return false;
    }
    while (true) {
        SlotNum slotNum = findRightMost(bmForScan, 1);
        if (slotNum == -1 || slotNum >= maxRecordCnt) {
            ++pnForScan;
            if (pnForScan > availPageCnt) {
                return false;
            }
            BitMap bm = ((PageHeader *)getPageData(pnForScan, false))->bitmap;
            memcpy((char *)(&bmForScan), (char *)(&bm), sizeof(bmForScan));
        } else {
            setBit(bmForScan, slotNum, 0);
            rid = RID(pnForScan, slotNum);
            return true;
        }
    }
}


// bool RM_FileHandle::getFirstRid(RID &rid) const {
    // RID tmpRid(1, -1);
    // return getNextRid(tmpRid, rid);
// }

// bool RM_FileHandle::getNextRid(const RID &lastRid, RID &rid) const {
    // PageNum pageNum = lastRid.getPageNum();
    // SlotNum slotNum = lastRid.getSlotNum() + 1;
    // for (; pageNum <= availPageCnt; ++pageNum) {
        // PageHeader *ph = (PageHeader *)getPageData(pageNum, false);
        // BitMap bitmap = ph->bitmap;
        // for (; slotNum < maxRecordCnt; ++slotNum) {
            // if (bitmap & (1 << slotNum)) {
                // rid = RID(pageNum, slotNum);
                // return true;
            // }
        // }
        // slotNum = 0;
    // }
    // return false;
// }


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
        initBitMap(ph->bitmap);
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
