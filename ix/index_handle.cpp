#include "rm.h"


IX_IndexHandle::IX_IndexHandle(BufPageMmanager *&bpm, int fieldId) : bpm(bpm), fieldId(fieldId) {
    FileHeaderPage *header = (FileHeaderPage *)getPageData(0, false);
    recordSize = header->recordSize;
    availPageCnt = header->availPageCnt;
    root = header->root;
}


IX_IndexHandle::~IX_IndexHandle() {}


bool IX_IndexHandle::insertEntry(void *pData, const RID &rid) {
    //todo check if not an open index
    if (root.getPageNum() == -1 && root.getSlotNum() == -1) {
        IX_Bnode rootNode();
        //todo insert record rootNode and return a RID tempRID
        root = tempRID;
    }
    RID res;
    int pos;
    if (findInsertPos(root, pData, rid, res, pos) == false) {
        return false;
    }
    //todo B+ tree insert
}


bool IX_IndexHandle::deleteEntry(void *pData, const RID &rid) {
    //todo check if not an open index
    //todo B+ tree delete
    RID res;
    int pos;
    assert(findInsertPos(root, pData, rid, res, pos) == true);
    
    // todo get record of res in node
    for (int i = pos; i < node.size - 1; ++i) {
        node.indexValue[i] = node.indexValue[i + 1];
        node.indexRID[i] = node.indexRID[i + 1];
    }
    node.indexValue[node.size] = NULL;
    node.indexRID[node.size] = RID(-1, -1);
    --node.size;
    // todo write node to RID: res

    // update indexValue in ancestor node
    if (pos == 0 && node.size > 0 && res != root) {
        next_value = node.indexValue[0];
        while (node.father != root) {
            res = node.father;
            // todo get record of res in node
            for (int i = 0; i < node.size; ++i)
            if (node.indexValue[i] == pData) {
                node.indexValue[i] = next_value;
                // todo write node to RID: res
                return true;
            }
        }
        return true;
    }

    if (node.size == 0) {
        RID prev = node.prev;
        RID next = node.next;
        if (prev != RID(-1, -1)) {
            // todo get record of prev in node
            node.next = next;
            // todo write node to RID: prev
        }
        if (next != RID(-1, -1)) {
            // todo get record of next in node
            node.prev = prev;
            // todo write node to RID: next
        }

        // todo delete RID: res

        deleteNode(node.father, res);
    }

    return true;
}


int IX_IndexHandle::getFileId() const {
    return fileId;
}


RID IX_IndexHandle::getRoot() const {
    return root;
}


AttrType IX_IndexHandle::getAttrType() const {
    return attrType;
}


int IX_IndexHandle::getAttrLength() const {
    return attrLength;
}


bool IX_IndexHandle::findInsertPos(RID u, void *pData, const RID &rid, RID &res, int &pos) const {
    // todo get the Record of RID u in node
    if (node.isLeaf == true) {
        res = u;
        for (int i = 0; i < node.size; ++i) {
            // todo implement indexEqual
            if (indexEQ(pData, rid, indexValue[i], indexRID[i])) {
                pos = i;
                return false;
            }
            if (indexLT(pData, rid, indexValue[i], indexRID[i])) {
                pos = i;
                return true;
            }
        }
        pos = node.size;
        return true;
    }
    for (int i = 0; i < node.size; ++i)
    // implement indexLess
    if (indexLT(pData, rid, indexValue[i], indexRID[i])) {
        return findInsertPos(node.indexRID[i], pData, rid, res, pos);
    }
    return findInsertPos(node.indexRID[node.size], pData, rid, res, pos);
}


void IX_IndexHandle::searchFirst(RID u, RID &res, int &pos) const {
    // todo get the Record of RID u in node
    if (node.isLeaf = true) {
        res = u;
        pos = 0;
        return;
    }
    searchFirst(node.indexRID[0], res, pos);
}


void IX_IndexHandle::searchGE(RID u, void *pData, const RID &rid, RID &res, int &pos) const {
    // todo get the Record of RID u in node
    if (node.isLeaf == true) {
        res = u;
        for (int i = 0; i < node.size; ++i)
        if (indexGE(indexValue[i], indexRID[i], pData, rid)) {
            pos = i;
            return;
        }
        res = RID(-1, -1);
        pos = -1;
        return;
    }
    for (int i = 0; i < node.size; ++i)
    if (indexGT(indexValue[i], indexRID[i], pData, rid)) {
        searchGE(node.indexRID[i], pData, rid, res, pos);
        return;
    }
    searchGE(node.indexRID[node.size], pData, rid, res, pos);
}

void IX_IndexHandle::searchLT(RID u, void *pData, const RID &rid, RID &res, int &pos) const {
    // todo get Record of RID u in node
    if (node.isLeaf == true) {
        res = u;
        for (int i = node.size - 1; i >= 0; --i)
        if (indexLT(indexValue[i], indexRID[i], pData, rid)) {
            pos = i;
            return;
        }
        res = RID(-1, -1);
        pos = -1;
        return;
    }
    for (int i = node.size - 1; i >= 0; --i)
    if (indexLT(indexValue[i], indexRID[i], pData, rid)) {
        searchLT(node.indexRID[i + 1], pData, rid, res, pos);
        return;
    }
    searchLT(node.indexRID[0], pData, rid, res, pos);
}


void IX_IndexHandle::searchNext(RID &rid, int &pos, bool direct) const {
    if (pos == -1) {
        return;
    }
    if (direct == 0) {
        if (pos == 0) {
            rid = node.prev;
            // todo get the Record of rid in node
            pos = node.size;
        }
        else {
            --pos;
        }
    }
    else {
        // todo get the Record of rid in node
        if (pos == node.size) {
            rid = node.next;
            pos = 0;
        }
        else {
            ++pos;
        }
    }
}


void IX_IndexHandle::deleteNode(RID &u, RID & v) {
    // todo get Record of RID u in node
    for (int pos = 0; pos <= node.size; ++pos)
    if (node.indexRID[pos] == v) {
        for (int i = pos; i < node.size; ++i) {
            node.indexRID[i] = node.indexRID[i + 1];
        }
        node.indexRID[node.size] = RID(-1, -1);

        for (int i = max(pos - 1, 0); i < node.size - 1; ++i) {
            node.indexValue[i] = node.indexValue[i + 1];
        }
        node.indexValue[node.size - 1] = NULL;

        node.size--;
        // todo write node to RID: u
        break;
    }

    if (node.size == 0) {
        // todo delete record RID: u
        deleteNode(node.father, u);
    }
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