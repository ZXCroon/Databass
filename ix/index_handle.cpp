#include "rm.h"


IX_IndexHandle::IX_IndexHandle(BufPageMmanager *&bpm, int fieldId) : bpm(bpm), fieldId(fieldId) {
    FileHeaderPage *header = (FileHeaderPage *)getPageData(0, false);
    recordSize = header->recordSize;
    availPageCnt = head->availPageCnt;
}


IX_IndexHandle::~IX_IndexHandle() {}


bool IX_IndexHandle::insertEntry(void *pData, const RID &rid) {
    //todo B+ tree insert
}


bool IX_IndexHandle::deleteEntry(void *pData, const RID &rid) {
    //todo B+ tree delete
}


int IX_IndexHandle::getFileId() const {
    return fileId;
}


PageNum IX_IndexHandle::getFreePage() {
    FileHeaderPage *header = (FileHeaderPage *)getPageData(0, false);
    if (header->firstFree == NO_PAGE) {
        ++availPageCnt;
        header = (FileHeaderPage *)getPageData(0, true);
        ++header->availPageCnt;
        insertFreePage(header->availPageCnt, true);
    }
    return header->firstFree;
}


char *IX_IndexHandle::getPageData(PageNum pageNum, bool write) const {
    int index;
    char *bpmBuf = (char *)(bpm->getPage(fileId, pageNum, index));
    if (write) {
        bpm->markDirty(index);
    }
    return bpmBuf;
}

bool IX_IndexHandle::insertFreePage(PageNum pageNum, bool isNew) const {
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


bool IX_IndexHandle::removeFreePage(PageNum) const {
    FileHeaderPage *header = (FileHeaderPage *)getPageData(0, true);
    PageHeader *ph = (PageHeader *)getPageData(pageNum, true);
    if (ph->prevFree == NO_PAGE && ph->nextFree == NO_PAGE && header->fistFree != pageNum) {
        return false;
    }
    if (ph->prevFree == NO_PAGE) {
        head->firstFree = ph->nextFree;
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