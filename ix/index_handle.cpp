#include <cstring>
#include "ix.h"


IX_IndexHandle::IX_IndexHandle(BufPageManager *&bpm, int fileId) : bpm(bpm), fileId(fileId) {
    IX_FileHeaderPage *header = (IX_FileHeaderPage *)getPageData(0, false);
    recordSize = header->recordSize;
    maxRecordCnt = (PAGE_SIZE - sizeof(IX_PageHeader)) / recordSize;
    availPageCnt = header->availPageCnt;
    root = header->root;
    attrType = header->attrType;
    attrLength = header->attrLength;

    nullRID = RID(-1, -1);
}


IX_IndexHandle::~IX_IndexHandle() {}


bool IX_IndexHandle::insertEntry(void *pData, const RID &rid) {
    //todo check if not an open index
    if (root.getPageNum() == -1 && root.getSlotNum() == -1) {
        char *tempData = new char[recordSize];
        memset(tempData, 0, recordSize);
        insertRec(tempData, root);
        IX_Record tempRec;
        assert(getRec(root, tempRec) == true);
        *(tempRec.getIsLeaf()) = 1;
        *(tempRec.getSize()) = 0;
        memcpy(tempRec.getPrev(), &nullRID, sizeof(RID));
        memcpy(tempRec.getNext(), &nullRID, sizeof(RID));
        updateRec(tempRec);
        IX_FileHeaderPage *header = (IX_FileHeaderPage *)getPageData(0, true);
        header->root = root;
    }

    RID res;
    int pos;
    if (findInsertPos(root, pData, rid, res, pos) == false) {
        return false;
    }

    IX_Record rec;
    assert(getRec(res, rec) == true);

    // update index
    if (pos == 0 && res != root) {
        void *replaced_value = rec.getIndexValue(0);
        RID *replaced_rid = rec.getIndexRID(0);
        IX_Record frec;
        assert(getRec(res, frec) == true);
        while (*(frec.getFather()) != root) {
            res = *(frec.getFather());
            assert(getRec(res, frec) == true);
            for (int i = 0; i < *(frec.getSize()); ++i) {
                if (indexEQ(replaced_value, *replaced_rid, frec.getIndexValue(i), *(frec.getIndexRID(i)))) {
                    memcpy(frec.getIndexValue(i), pData, attrLength);
                    memcpy(frec.getIndexRID(i), &rid, sizeof(RID));
                    updateRec(frec);
                    break;
                }
            }
        }
    }

    if (*(rec.getSize()) < 4) {
        for (int i = *(rec.getSize()); i > pos; --i) {
            memcpy(rec.getIndexValue(i), rec.getIndexValue(i - 1), attrLength);
            memcpy(rec.getIndexRID(i), rec.getIndexRID(i - 1), sizeof(RID));
        }
        memcpy(rec.getIndexValue(pos), pData, attrLength);
        memcpy(rec.getIndexRID(pos), &rid, sizeof(RID));
        ++(*(rec.getSize()));
        updateRec(rec);

        return true;
    }

    // split
    assert(*(rec.getSize()) == 4);
    void *tempIndexValue[5];
    RID tempIndexRID[5];
    int j = -1;
    for (int i = 0; i < pos; ++i) {
        ++j;
        tempIndexValue[j] = new char[attrLength];
        memcpy(tempIndexValue[j], rec.getIndexValue(i), attrLength);
        memcpy(&(tempIndexRID[j]), rec.getIndexRID(i), sizeof(RID));
    }

    ++j;
    tempIndexValue[j] = new char[attrLength];
    memcpy(tempIndexValue[j], pData, attrLength);
    memcpy(&(tempIndexRID[j]), &rid, sizeof(RID));

    for (int i = pos; i < *(rec.getSize()); ++i) {
        ++j;
        tempIndexValue[j] = new char[attrLength];
        memcpy(tempIndexValue[j], rec.getIndexValue(i), attrLength);
        memcpy(&(tempIndexRID[j]), rec.getIndexRID(i), sizeof(RID));
    }

    const char* tempData = new char[recordSize];
    RID newRID;
    assert(insertRec(tempData, newRID) == true);
    IX_Record newRec;
    assert(getRec(newRID, newRec) == true);
    RID nextRID;
    memcpy(&nextRID, rec.getNext(), sizeof(RID));

    // update rec
    *(rec.getSize()) = 3;
    for (int i = 0; i < 3; ++i) {
        memcpy(rec.getIndexValue(i), tempIndexValue[i], attrLength);
        memcpy(rec.getIndexRID(i), &(tempIndexRID[i]), sizeof(RID));
    }
    memcpy(rec.getNext(), &newRID, sizeof(RID));
    updateRec(rec);

    // update newRec
    *(newRec.getIsLeaf()) = 1;
    *(newRec.getSize()) = 2;
    for (int i = 0; i < 2; ++i) {
        memcpy(newRec.getIndexValue(i), tempIndexValue[3 + i], attrLength);
        memcpy(newRec.getIndexRID(i), &(tempIndexRID[3 + i]), sizeof(RID));
    }
    memcpy(newRec.getFather(), rec.getFather(), sizeof(RID));
    memcpy(newRec.getPrev(), &res, sizeof(RID));
    memcpy(newRec.getNext(), &nextRID, sizeof(RID));
    updateRec(newRec);
    
    // update nextRec
    if (nextRID != RID(-1, -1)) {
        IX_Record nextRec;
        assert(getRec(nextRID, nextRec) == true);
        memcpy(nextRec.getPrev(), &newRID, sizeof(RID));
        updateRec(nextRec);
    }

    combinePair(res, newRID, tempIndexValue[3], tempIndexRID[3]);

    return true;
}


bool IX_IndexHandle::deleteEntry(void *pData, const RID &rid) {
    //todo check if not an open index
    RID res;
    int pos;
    assert(findInsertPos(root, pData, rid, res, pos) == true);
    
    IX_Record rec;
    getRec(res, rec);
    for (int i = pos; i < *(rec.getSize()) - 1; ++i) {
        memcpy(rec.getIndexValue(i), rec.getIndexValue(i + 1), attrLength);
        memcpy(rec.getIndexRID(i), rec.getIndexRID(i + 1), sizeof(RID));
    }
    memset(rec.getIndexValue(*(rec.getSize()) - 1), 0, attrLength);
    memset(rec.getIndexRID(*(rec.getSize()) - 1), 0, sizeof(RID));
    --(*(rec.getSize()));
    updateRec(rec);

    // update indexValue in ancestor node
    if (pos == 0 && *(rec.getSize()) > 0 && res != root) {
        void *next_value = rec.getIndexValue(0);
        RID *next_rid = rec.getIndexRID(0);
        while (*(rec.getFather()) != root) {
            res = *(rec.getFather());
            assert(getRec(res, rec) == true);
            for (int i = 0; i < *(rec.getSize()); ++i)
            if (indexEQ(pData, rid, rec.getIndexValue(i), *(rec.getIndexRID(i)))) {
                memcpy(rec.getIndexValue(i), next_value, attrLength);
                memcpy(rec.getIndexRID(i), next_rid, sizeof(RID));
                updateRec(rec);
                return true;
            }
        }
        return true;
    }

    if (*(rec.getSize()) == 0) {
        RID prev, next;
        memcpy(&prev, rec.getPrev(), sizeof(RID));
        memcpy(&next, rec.getNext(), sizeof(RID));
        if (prev != RID(-1, -1)) {
            IX_Record prec;
            assert(getRec(prev, prec) == true);
            memcpy(prec.getNext(), &next, sizeof(RID));
            updateRec(prec);
        }
        if (next != RID(-1, -1)) {
            IX_Record nrec;
            assert(getRec(next, nrec) == true);
            memcpy(nrec.getPrev(), &prev, sizeof(RID));
            updateRec(nrec);
        }
        deleteNode(*(rec.getFather()), res);
        deleteRec(res);
    }
    return true;
}


bool IX_IndexHandle::getRec(const RID &rid, IX_Record &rec) const {
    PageNum pageNum = rid.getPageNum();
    SlotNum slotNum = rid.getSlotNum();
    char *page = getPageData(pageNum, false);
    rec = IX_Record(recordSize, attrLength, rid);
    if (queryBit(((IX_PageHeader *)page)->bitmap, slotNum) == 1) {
        memcpy(rec.getData(), page + getSlotOffset(slotNum), recordSize);
        return true;
    } else {
        return false;
    }
}


bool IX_IndexHandle::insertRec(const char *pData, RID &rid) {
    PageNum pageNum = getFreePage();
    IX_PageHeader *ph = (IX_PageHeader *)getPageData(pageNum, false);
    SlotNum slotNum = findRightMost(ph->bitmap, 0);
    
    ph = (IX_PageHeader *)getPageData(pageNum, true);
    rid = RID(pageNum, slotNum);
    memcpy(((char *)ph) + getSlotOffset(slotNum), pData, recordSize);
    setBit(ph->bitmap, slotNum, 1);
    int tmp = findRightMost(ph->bitmap, 0);
    if (tmp == -1 || tmp >= maxRecordCnt) {
        return removeFreePage(pageNum);
    }
    return true;
}


bool IX_IndexHandle::deleteRec(const RID &rid) {
    PageNum pageNum = rid.getPageNum();
    SlotNum slotNum = rid.getSlotNum();
    IX_PageHeader *ph = (IX_PageHeader *)getPageData(pageNum, false);
    bool oriFull = findRightMost(ph->bitmap, 0) == -1;
    if (queryBit(ph->bitmap, slotNum) == 0) {
        return false;
    }
    ph = (IX_PageHeader *)getPageData(pageNum, true);
    setBit(ph->bitmap, slotNum, 0);
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
    
    if (*(rec.getIsLeaf()) == 1) {
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

    if (*(rec.getIsLeaf()) == 1) {
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

    if (*(rec.getIsLeaf()) == 1) {
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
    searchGE(*(rec.getChild(*(rec.getSize()))), pData, rid, res, pos);
}


void IX_IndexHandle::searchLT(RID u, void *pData, const RID &rid, RID &res, int &pos) const {
    IX_Record rec;
    if (!getRec(u, rec)) {
        res = RID(-1, -1);
        pos = -1;
        return;
    }
    
    if (*(rec.getIsLeaf()) == 1) {
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


/* v, a child of u is deleted */
void IX_IndexHandle::deleteNode(RID &u, RID & v) {
    IX_Record rec;
    assert(getRec(u, rec) == true);
    
    for (int pos = 0; pos <= *(rec.getSize()); ++pos)
    if (*(rec.getChild(pos)) == v) {
        // delete child
        for (int i = pos; i < *(rec.getSize()); ++i) {
            memcpy(rec.getChild(i), rec.getChild(i + 1), sizeof(RID));
        }
        memset(rec.getChild(*(rec.getSize())), 0, sizeof(RID));

        // delete index
        for (int i = max(pos - 1, 0); i < *(rec.getSize()) - 1; ++i) {
            memcpy(rec.getIndexValue(i), rec.getIndexValue(i + 1), attrLength);
            memcpy(rec.getIndexRID(i), rec.getIndexRID(i + 1), sizeof(RID));
        }
        memset(rec.getIndexValue(*(rec.getSize()) - 1), 0, attrLength);
        memset(rec.getIndexRID(*(rec.getSize()) - 1), 0, sizeof(RID));

        --(*(rec.getSize()));
        updateRec(rec);
        break;
    }

    if (*(rec.getSize()) == 0) {
        deleteNode(*(rec.getFather()), u);
        deleteRec(u);
    }
}


/* let v be the right neighbor of u with lowerbound (pData, rid) */
void IX_IndexHandle::combinePair(RID &u, RID &v, void *pData, const RID &rid) {
    if (u == root) {
        char *tempData = new char[recordSize];
        memset(tempData, 0, recordSize);
        insertRec(tempData, root);
        IX_Record tempRec;
        assert(getRec(root, tempRec) == true);
        *(tempRec.getIsLeaf()) = 0;
        *(tempRec.getSize()) = 1;
        memcpy(tempRec.getIndexValue(0), pData, attrLength);
        memcpy(tempRec.getIndexRID(0), &rid, sizeof(RID));
        memcpy(tempRec.getChild(0), &u, sizeof(RID));
        memcpy(tempRec.getChild(1), &v, sizeof(RID));
        *(tempRec.getPrev()) = RID(-1, -1);
        *(tempRec.getNext()) = RID(-1, -1);
        updateRec(tempRec);
        IX_FileHeaderPage *header = (IX_FileHeaderPage *)getPageData(0, true);
        header->root = root;

        IX_Record uRec, vRec;
        assert(getRec(u, uRec) == true);
        memcpy(uRec.getFather(), &root, sizeof(RID));
        updateRec(uRec);

        assert(getRec(v, vRec) == true);
        memcpy(vRec.getFather(), &root, sizeof(RID));
        updateRec(vRec);
        return;
    }

    RID f;
    IX_Record uRec, faRec;
    assert(getRec(u, uRec) == true);
    memcpy(&f, uRec.getFather(), sizeof(RID));
    assert(getRec(f, faRec) == true);
    
    int pos = -1;
    for (int i = 0; i <= *(faRec.getSize()); ++i)
    if (*(faRec.getChild(i)) == u) {
        pos = i;
        break;
    }

    if (*(faRec.getSize()) < 4) {
        for (int i = *(faRec.getSize()); i >= pos + 1; --i) {
            memcpy(faRec.getIndexValue(i), faRec.getIndexValue(i - 1), attrLength);
            memcpy(faRec.getIndexRID(i), faRec.getIndexRID(i - 1), sizeof(RID));
        }
        memcpy(faRec.getIndexValue(pos), pData, attrLength);
        memcpy(faRec.getIndexRID(pos), &rid, sizeof(RID));

        for (int i = *(faRec.getSize()) + 1; i >= pos + 2; --i) {
            memcpy(faRec.getChild(i), faRec.getChild(i - 1), sizeof(RID));
        }
        memcpy(faRec.getChild(pos + 1), &v, sizeof(RID));
        ++(*(faRec.getSize()));
        updateRec(faRec);

        return;
    }

    // split
    assert(*(faRec.getSize()) == 4);
    void *tempIndexValue[5];
    RID tempIndexRID[5];
    RID tempChild[6];
    // get index list
    int j = -1;
    for (int i = 0; i < pos; ++i) {
        ++j;
        tempIndexValue[j] = new char[attrLength];
        memcpy(tempIndexValue[j], faRec.getIndexValue(i), attrLength);
        memcpy(&(tempIndexRID[j]), faRec.getIndexRID(i), sizeof(RID));
    }
    ++j;
    tempIndexValue[j] = new char[attrLength];
    memcpy(tempIndexValue[j], pData, attrLength);
    memcpy(&(tempIndexRID[j]), &rid, sizeof(RID));
    for (int i = pos; i < *(faRec.getSize()); ++i) {
        ++j;
        tempIndexValue[j] = new char[attrLength];
        memcpy(tempIndexValue[j], faRec.getIndexValue(i), attrLength);
        memcpy(&(tempIndexRID[j]), faRec.getIndexRID(i), sizeof(RID));
    }
    // get child list
    int k = -1;
    for (int i = 0; i <= pos; ++i) {
        ++k;
        memcpy(&(tempChild[k]), faRec.getChild(i), sizeof(RID));
    }
    ++k;
    memcpy(&(tempChild[k]), &v, sizeof(RID));
    for (int i = pos + 1; i <= *(faRec.getSize()); ++i) {
        ++k;
        memcpy(&(tempChild[k]), faRec.getChild(i), sizeof(RID));
    }

    const char* tempData = new char[recordSize];
    RID newRID;
    assert(insertRec(tempData, newRID) == true);
    IX_Record newRec;
    assert(getRec(newRID, newRec) == true);
    
    // updata faRec
    *(faRec.getSize()) = 2;
    for (int i = 0; i <= 2; ++i) {
        if (i != 2) {
            memcpy(faRec.getIndexValue(i), tempIndexValue[i], attrLength);
            memcpy(faRec.getIndexRID(i), &(tempIndexRID[i]), sizeof(RID));
        }
        memcpy(faRec.getChild(i), &(tempChild[i]), sizeof(RID));
    }
    updateRec(faRec);

    // updata newRec
    *(newRec.getIsLeaf()) = 0;
    *(newRec.getSize()) = 2;
    for (int i = 0; i <= 2; ++i) {
        if (i != 2) {
            memcpy(newRec.getIndexValue(i), tempIndexValue[3 + i], attrLength);
            memcpy(newRec.getIndexRID(i), &(tempIndexRID[3 + i]), sizeof(RID));
        }
        memcpy(newRec.getChild(i), &(tempChild[3 + i]), sizeof(RID));
    }
    memcpy(newRec.getFather(), &f, sizeof(RID));
    updateRec(newRec);

    combinePair(f, newRID, tempIndexValue[2], tempIndexRID[2]);
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
        int cmp = strncmp(u, v, attrLength);
        return cmp > 0 || (cmp == 0 && rid1 > rid2);
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
        int cmp = strncmp(u, v, attrLength);
        return cmp < 0 || (cmp == 0 && rid1 < rid2);
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
        IX_PageHeader *fph = (IX_PageHeader *)getPageData(header->firstFree, true);
        fph->prevFree = pageNum;
        ph->nextFree = header->firstFree;
        ph->prevFree = NO_PAGE;
        header->firstFree = pageNum;
    }
    return true;
}


bool IX_IndexHandle::removeFreePage(PageNum pageNum) const {
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
