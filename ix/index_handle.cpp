#include "rm.h"


IX_IndexHandle::IX_IndexHandle(BufPageMmanager *&bpm, int fieldId) : bpm(bpm), fieldId(fieldId) {
    IX_FileHeaderPage *header = (IX_FileHeaderPage *)getPageData(0, false);
    recordSize = header->recordSize;
    maxRecordCnt = (PAGE_SIZE - sizeof(IX_PageHeader)) / recordSize;
    availPageCnt = header->availPageCnt;
    root = header->root;
    attrType = header->attrType;
    attrLength = header->attrLength;
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
    
    // todo get the Record of res in node
    if (node.size < 4) {
        for (int i = node.size; i > pos; --i) {
            node.indexValue[i] = node.indexValue[i - 1];
            node.indexRID[i] = node.indexRID[i - 1];
        }
        node.indexValue[pos] = pData;
        node.indexRID[pos] = rid;
        // todo write node in RID: res

        // update index
        if (pos == 0 && res != root) {
            void *replaced = node.indexValue[1];
            while (node.father != root) {
                res = node.father;
                // todo get record of res in node
                for (int i = 0; i < node.size; ++i)
                    if (node.indexValue[i] == replaced) {
                node.indexValue[i] = next_value;
                // todo write node to RID: res
                return true;
            }
            }
        }
        return true;
    }

    return true;
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
        void *next_value = node.indexValue[0];
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


bool IX_IndexHandle::getRec(const RID &rid, IX_Record &rec) const {
    PageNum pageNum = rid.getPageNum();
    SlotNum slotNum = rid.getSlotNum();
    char *page = getPageData(pageNum, false);
    rec = IX_Record(recordSize, rid);
    if (((IX_PageHeader *)page)->bitmap & (1 << slotNum)) {
        memcpy(rec.getData(), page + getSlotOffset(slotNum), recordSize);
        return true;
    } else {
        return false;
    }
}


bool IX_IndexHandle::insertRec(const char *pData, RID &rid) {
    PageNum pageNum = getFreePage();
    IX_PageHeader *ph = (IX_PageHeader *)getPageData(pageNum, false);
    SlotNum slotNum = 0;
    for (Bits x = ph->bitmap; x & 1; (x <<= 1), ++slotNum);
    ph = (PageHeader *)getPageData(pageNum, true);
    rid = RID(pageNum, slotNum);
    memcpy(((char *)ph) + getSlotOffset(slotNum), pData, recordSize);
    ph->bitmap |= (1 << slotNum);
    if ((ph->bitmap & ((1 << maxRecordCnt) - 1)) == ((1 << maxRecordCnt) - 1)) {
        return removeFreePage(pageNum);
    }
    return true;
}


bool IX_IndexHandle::deleteRec(const RID &rid) {
    PageNum pageNum = rid.getPageNum();
    SlotNum slotNum = rid.getSlotNum();
    IX_PageHeader *ph = (IX_PageHeader *)getPageData(pageNum, false);
    bool oriFull = ((ph->bitmap & ((1 << maxRecordCnt) - 1)) == ((1 << maxRecordCnt) - 1));
    if ((ph->bitmap | ~(1 << slotNum)) == 0) {
        return false;
    }
    ph = (IX_PageHeader *)getPageData(pageNum, true);
    ph->bitmap &= ~(1 << slotNum);
    if (oriFull) {
        return insertFreePage(pageNum, false);
    }
    return true;
}


bool IX_IndexHandle::updateRec(const IX_Record &rec) {
    RID rid = rec.getRid();
    PageNum pageNum = rid.getPageNum();
    SlotNum slotNum = rid.getSlotNum();
    IX_PageHeader *ph = (IX_PageHeader *)getPageData(pageNum, false);
    if ((ph->bitmap | ~(1 << slotNum)) == 0) {
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
    IX_Record rec;
    if (!getRec(u, rec)) {
        return false;
    }
    
    if (*(rec.getIsLeaf()) == true) {
        res = u;
        for (int i = 0; i < *(rec.getSize()); ++i) {
            if (indexEQ(pData, rid, rec.getIndexValue(i), *(rec.getIndexRID(i)))) {
                pos = i;
                return false;
            }
            if (indexLT(pData, rid, rec.getIndexValue(i), *(rec.getIndexRID(i)))) {
                pos = i;
                return true;
            }
        }
        pos = *(rec.getSize());
        return true;
    }

    for (int i = 0; i < *(rec.getSize()); ++i)
    if (indexLT(pData, rid, rec.getIndexValue(i), *(rec.getIndexRID(i)))) {
        return findInsertPos(*(rec.getChild(i)), pData, rid, res, pos);
    }
    return findInsertPos(*(rec.getChild(*(rec.getSize()))), pData, rid, res, pos);
}


void IX_IndexHandle::searchFirst(RID u, RID &res, int &pos) const {
    IX_Record rec;
    if (!getRec(u, rec)) {
        res = RID(-1, -1);
        pos = -1;
        return;
    }

    if (*(rec.getIsLeaf()) == true) {
        res = u;
        pos = 0;
        return;
    }
    searchFirst(*(rec.getChild(0)), res, pos);
}


void IX_IndexHandle::searchGE(RID u, void *pData, const RID &rid, RID &res, int &pos) const {
    IX_Record rec;
    if (!getRec(u, rec)) {
        res = RID(-1, -1);
        pos = -1;
        return;
    }

    if (*(rec.getIsLeaf()) == true) {
        res = u;
        for (int i = 0; i < *(rec.getSize()); ++i)
        if (indexGE(rec.getIndexValue(i), *(rec.getIndexRID(i)), pData, rid)) {
            pos = i;
            return;
        }
        res = RID(-1, -1);
        pos = -1;
        return;
    }

    for (int i = 0; i < *(rec.getSize()); ++i)
    if (indexGT(rec.getIndexValue(i), *(rec.getIndexRID(i)), pData, rid)) {
        searchGE(*(rec.getChild(i)), pData, rid, res, pos);
        if (pos != -1) {
            return;
        }
    }
    searchGE(*(rec.getChild(*(rec.getChild(i)))), pData, rid, res, pos);
}


void IX_IndexHandle::searchLT(RID u, void *pData, const RID &rid, RID &res, int &pos) const {
    IX_Record rec;
    if (!getRec(u, rec)) {
        res = RID(-1, -1);
        pos = -1;
        return;
    }
    
    if (*(rec.getIsLeaf()) == true) {
        res = u;
        for (int i = *(rec.getSize()) - 1; i >= 0; --i)
        if (indexLT(rec.getIndexValue(i), *(rec.getIndexRID(i)), pData, rid)) {
            pos = i;
            return;
        }
        res = RID(-1, -1);
        pos = -1;
        return;
    }

    for (int i = *(rec.getSize()) - 1; i >= 0; --i)
    if (indexLT(rec.getIndexValue(i), *(rec.getIndexRID(i)), pData, rid)) {
        searchLT(*(rec.getChild(i + 1)), pData, rid, res, pos);
        if (pos != -1) {
            return;
        }
    }
    searchLT(*(rec.getChild(0)), pData, rid, res, pos);
}


void IX_IndexHandle::searchNext(RID &rid, int &pos, bool direct) const {
    if (pos == -1) {
        return;
    }

    if (direct == 0) {
        if (pos == 0) {
            IX_Record rec;
            if (!getRec(rid, rec)) {
                rid = RID(-1, -1);
                pos = -1;
                return;
            }
            rid = *(rec.getPrev());
            
            if (!getRec(rid, rec)) {
                rid = RID(-1, -1);
                pos = -1;
                return;
            }
            pos = *(rec.getSize()) - 1;
        }
        else {
            --pos;
        }
    }
    else {
        IX_Record rec;
        if (!getRec(rid, rec)) {
            rid = RID(-1, -1);
            pos = -1;
            return;
        }
        
        if (pos == *(rec.getSize()) - 1) {
            rid = *(rec.getNext());
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


bool IX_IndexHandle::indexEQ(void *data1, RID rid1, void *data2, RID rid2) const {
    switch (attrType) {
        
    case INT: {
        int u = *(int *)data1, v = *(int *)data2;
        return u == v && rid1 == rid2;
    }

    case FLOAT: {
        float u = *(float *)data1, v = *(float *)data2;
        return u == v && rid1 == rid2;
    }

    case STRING: {
        char *u = (char *)data1, *v = (char *)data2;
        int cmp = strncmp(u, v, attrLength);
        return cmp == 0 && rid1 == rid2;
    }

    }
}


bool IX_IndexHandle::indexGE(void *data1, RID rid1, void *data2, RID rid2) const {
    return indexEQ(data1, rid1, data2, rid2) || indexGT(data1, rid1, data2, rid2);
}


bool IX_IndexHandle::indexGT(void *data1, RID rid1, void *data2, RID rid2) const {
    switch (attrType) {

    case INT: {
        int u = *(int *)data1, v = *(int *)data2;
        return u > v || (u == v && rid1 > rid2);
    }

    case FLOAT: {
        float u = *(float *)data1, v = *(float *)data2;
        return u > v || (u == v && rid1 > rid2);
    }

    case STRING: {
        char *u = (char *)data1, *v = (char *)data2;
        int cmp = strcmp(u, v, attrLength);
        return cmp > 0 || (cmp == 0 && rid1 > ri2);
    }
    
    }
}


bool IX_IndexHandle::indexLE(void *data1, RID rid1, void *data2, RID rid2) const {
    return indexEQ(data1, rid1, data2, rid2) || indexLT(data1, rid1, data2, rid2);
}


bool IX_IndexHandle::indexLT(void *data1, RID rid1, void *data2, RID rid2) const {
    switch (attrType) {

    case INT: {
        int u = *(int *)data1, v = *(int *)data2;
        return u < v || (u == v && rid1 < rid2);
    }

    case FLOAT: {
        float u = *(float *)data1, v = *(float *)data2;
        return u < v || (u == v && rid1 < rid2);
    }

    case STRING: {
        char *u = (char *)data1, *v = (char *)data2;
        int cmp = strcmp(u, v, attrLength);
        return cmp < 0 || (cmp == 0 && rid1 < ri2);
    }
    }
}


PageNum IX_IndexHandle::getFreePage() {
    IX_FileHeaderPage *header = (IX_FileHeaderPage *)getPageData(0, false);
    if (header->firstFree == NO_PAGE) {
        ++availPageCnt;
        header = (IX_FileHeaderPage *)getPageData(0, true);
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
    IX_FileHeaderPage *header = (IX_FileHeaderPage *)getPageData(0, true);
    IX_PageHeader *ph = (IX_PageHeader *)getPageData(pageNum, true);
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
        IX_PageHeader *fph = (IX_PageHeader *)getPageData(header->firstFree, true);
        fph->prevFree = pageNum;
        ph->nextFree = header->firstFree;
        ph->prevFree = NO_PAGE;
        header->firstFree = pageNum;
    }
    return true;
}


bool IX_IndexHandle::removeFreePage(PageNum) const {
    IX_FileHeaderPage *header = (IX_FileHeaderPage *)getPageData(0, true);
    IX_PageHeader *ph = (IX_PageHeader *)getPageData(pageNum, true);
    if (ph->prevFree == NO_PAGE && ph->nextFree == NO_PAGE && header->firstFree != pageNum) {
        return false;
    }
    if (ph->prevFree == NO_PAGE) {
        header->firstFree = ph->nextFree;
    } else {
        IX_PageHeader *pph = (IX_PageHeader *)getPageData(ph->prevFree, true);
        pph->nextFree = ph->nextFree;
        ph->prevFree = NO_PAGE;
    }
    if (ph->nextFree == NO_PAGE) {
        header->lastFree = ph->prevFree;
    } else {
        IX_PageHeader *nph = (IX_PageHeader *)getPageData(ph->nextFree, true);
        nph->prevFree = ph->prevFree;
        ph->nextFree = NO_PAGE;
    }
    return true;
}