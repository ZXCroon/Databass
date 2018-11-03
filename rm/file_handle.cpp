#include "rm.h"


RM_FileHandle::RM_FileHandle(unsigned int fileId) : bpm(bpm), fileId(fileId) {
    FileHeaderPage *header = (FileHeaderPage *)getPageData(0, false);
    recordSize = header->recordSize;
    maxRecordCnt = (PAGE_SIZE - sizeof(PageHeader)) / recordSize;
    availPageCnt = header->availPageCnt;
}


RM_FileHandle::~RM_FileHandle() {}


RC RM_FileHandle::getRec(const RID &rid, RM_Record &rec) {
    PageNum pageNum = rid.getPageNum();
    SlotNum slotNum = rid.getSlotNum();
    char *page = getPageData(pageNum, false);
    rec = RM_Record(recordSize, rid);
    memcpy(rec.getData(), page + getSlotOffset(slotNum), recordSize);
    return 0;
}


RC RM_FileHandle::insertRec(const char *pData, RID &rid) {
    PageNum pageNum = getFreePage();
    PageHeader *ph = (PageHeader *)getPageData(pageNum, true);
    Bits x = ph->bitmap;
    SlotNum slotNum = 0;
    for (; x & 1; (x <<= 1), ++slotNum);
    rid = RID(pageNum, slotNum);
    memcpy(((char *)ph) + getSlotOffset(slotNum), pData, recordSize);
    ph->bitmap |= (1 << slotNum);
    if ((ph->bitmap & ((1 << maxRecordCnt) - 1)) == ((1 << maxRecordCnt) - 1)) {
        removeFreePage(pageNum);
    }
    return 0;
}


RC RM_FileHandle::deleteRec(const RID &rid) {
    PageNum pageNum = rid.getPageNum();
    PageHeader *ph = (PageHeader *)getPageData(pageNum, true);
    bool oriFull = ((ph->bitmap & ((1 << maxRecordCnt) - 1)) == ((1 << maxRecordCnt) - 1));
    ph->bitmap &= ~(1 << slotNum);
    if (oriFull) {
        insertFreePage(pageNum, false);
    }
    return 0;
}


RC RM_FileHandle::updateRec(const RM_Record &rec) {
    RID rid = rec.getRid();
    PageNum pageNum = rid.getPageNum();
    SlotNum slotNum = rid.getSlotNum();
    char *page = getPageData(pageNum, false);
    memcpy(page + getSlotOffset(slotNum), rec.getData(), recordSize);
    return 0;
}


RC RM_FileHandle::getFirstRid(RID &rid) {
    RID tmpRid(0, -1)
    return getNextRid(tmpRid, rid);
}


RC RM_FileHandle::getNextRid(const RID &lastRid, RID &rid) {
    PageNum pageNum = lastRid.getRid();
    SlotNum slotNum = lastRid.getSlotNum() + 1;
    for (; pageNum <= availPageCnt; ++pageNum) {
        PageHeader *ph = (PageHeader *)getPageData(pageNum, false);
        Bits bitmap = ph->bitmap;
        for (; slotNum < maxRecordCnt; ++slotNum) {
            if (bitmap & (1 << slotNum)) {
                rid = RID(pageNum, slotNum);
                return 0;
            }
        }
        slotNum = 0;
    }
    return RM_FILEHANDLE_NORID;
}


int RM_FileHandle::getFreePage() {
    FileHeaderPage *header = (FileHeaderPage *)getPageData(0, false);
    if (header->firstFree == NO_PAGE) {
        ++availPageCnt;
        ++header->availPageCnt;
        insertFreePage(header->availPageCnt, true);
    }
    return header->firstFree;
}


bool RM_FileHandle::write(PageNum pageNum, int offset, char *buf, int length) {
    char *bpmBuf = getPageData(pageNum, true);
    memcpy(bpmBuf + offset, buf, length);
    return true;
}


bool RM_FileHandle::read(PageNum pageNum, int offset, char *buf, int length) {
    char *bpmBuf = getPageData(pageNum, false);
    memcpy(buf, bpmBuf + offset, length);
    return true;
}


char *RM_FileHandle::getPageData(PageNum pageNum, write) {
    int index;
    char *bpmBuf = (char *)(bpm.getPage(fileId, pageNum, index));
    if (write) {
        bpm.markDirty(index);
    }
    return bpmBuf;
}


bool RM_FileHandle::insertFreePage(PageNum pageNum, isNew) {
    FileHeaderPage *header = (FileHeaderPage *)getPageData(0, true);
    PageHeader *ph = (PageHeader *)getPageData(pageNum, true);
    if (isNew) {
        ph->bitmap = 0;
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


bool RM_FileHandle::removeFreePage(PageNum pageNum) {
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
