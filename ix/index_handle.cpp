#include <cstring>
#include "ix.h"

extern bool isDebug;


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
    if (isDebug) {
        printf("%d %d\n", rid.getPageNum(), rid.getSlotNum());
    }
    if (root.getPageNum() == -1 && root.getSlotNum() == -1) {
        char *tempData = new char[recordSize];
        memset(tempData, 0, recordSize);
        *(getIsLeaf(tempData)) = 1;
        *(getSize(tempData)) = 0;
        memcpy(getPrev(tempData), &nullRID, sizeof(RID));
        memcpy(getNext(tempData), &nullRID, sizeof(RID));

        insertRec(tempData, root);
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
        void *replaced_value = getIndexValue(rec.getData(), 0);
        RID *replaced_rid = getIndexRID(rec.getData(), 0);
        IX_Record frec;
        assert(getRec(res, frec) == true);
        while (*(getFather(frec.getData())) != root) {
            res = *(getFather(frec.getData()));
            assert(getRec(res, frec) == true);
            for (int i = 0; i < *(getSize(frec.getData())); ++i) {
                if (indexEQ(replaced_value, *replaced_rid, getIndexValue(frec.getData(), i), *(getIndexRID(frec.getData(), i)))) {
                    memcpy(getIndexValue(frec.getData(), i), pData, attrLength);
                    memcpy(getIndexRID(frec.getData(), i), &rid, sizeof(RID));
                    updateRec(frec);
                    break;
                }
            }
        }
    }

    if (*(getSize(rec.getData())) < 4) {
        for (int i = *(getSize(rec.getData())); i > pos; --i) {
            memcpy(getIndexValue(rec.getData(), i), getIndexValue(rec.getData(), i - 1), attrLength);
            memcpy(getIndexRID(rec.getData(), i), getIndexRID(rec.getData(), i - 1), sizeof(RID));
        }
        memcpy(getIndexValue(rec.getData(), pos), pData, attrLength);
        memcpy(getIndexRID(rec.getData(), pos), &rid, sizeof(RID));
        ++(*(getSize(rec.getData())));
        updateRec(rec);

        return true;
    }

    // split
    assert(*(getSize(rec.getData())) == 4);
    void *tempIndexValue[5];
    RID tempIndexRID[5];
    int j = -1;
    for (int i = 0; i < pos; ++i) {
        ++j;
        tempIndexValue[j] = new char[attrLength];
        memcpy(tempIndexValue[j], getIndexValue(rec.getData(), i), attrLength);
        memcpy(&(tempIndexRID[j]), getIndexRID(rec.getData(), i), sizeof(RID));
    }

    ++j;
    tempIndexValue[j] = new char[attrLength];
    memcpy(tempIndexValue[j], pData, attrLength);
    memcpy(&(tempIndexRID[j]), &rid, sizeof(RID));

    for (int i = pos; i < *(getSize(rec.getData())); ++i) {
        ++j;
        tempIndexValue[j] = new char[attrLength];
        memcpy(tempIndexValue[j], getIndexValue(rec.getData(), i), attrLength);
        memcpy(&(tempIndexRID[j]), getIndexRID(rec.getData(), i), sizeof(RID));
    }

    const char* tempData = new char[recordSize];
    RID newRID;
    assert(insertRec(tempData, newRID) == true);
    IX_Record newRec;
    assert(getRec(newRID, newRec) == true);
    RID nextRID;
    memcpy(&nextRID, getNext(rec.getData()), sizeof(RID));

    // update rec
    *(getSize(rec.getData())) = 3;
    for (int i = 0; i < 3; ++i) {
        memcpy(getIndexValue(rec.getData(), i), tempIndexValue[i], attrLength);
        memcpy(getIndexRID(rec.getData(), i), &(tempIndexRID[i]), sizeof(RID));
    }
    memcpy(getNext(rec.getData()), &newRID, sizeof(RID));
    updateRec(rec);

    // update newRec
    *(getIsLeaf(newRec.getData())) = 1;
    *(getSize(newRec.getData())) = 2;
    for (int i = 0; i < 2; ++i) {
        memcpy(getIndexValue(newRec.getData(), i), tempIndexValue[3 + i], attrLength);
        memcpy(getIndexRID(newRec.getData(), i), &(tempIndexRID[3 + i]), sizeof(RID));
    }
    memcpy(getFather(newRec.getData()), getFather(rec.getData()), sizeof(RID));
    memcpy(getPrev(newRec.getData()), &res, sizeof(RID));
    memcpy(getNext(newRec.getData()), &nextRID, sizeof(RID));
    updateRec(newRec);
    
    // update nextRec
    if (nextRID != RID(-1, -1)) {
        IX_Record nextRec;
        assert(getRec(nextRID, nextRec) == true);
        memcpy(getPrev(nextRec.getData()), &newRID, sizeof(RID));
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
    for (int i = pos; i < *(getSize(rec.getData())) - 1; ++i) {
        memcpy(getIndexValue(rec.getData(), i), getIndexValue(rec.getData(), i + 1), attrLength);
        memcpy(getIndexRID(rec.getData(), i), getIndexRID(rec.getData(), i + 1), sizeof(RID));
    }
    memset(getIndexValue(rec.getData(), *(getSize(rec.getData())) - 1), 0, attrLength);
    memset(getIndexRID(rec.getData(), *(getSize(rec.getData())) - 1), 0, sizeof(RID));
    --(*(getSize(rec.getData())));
    updateRec(rec);

    // update indexValue in ancestor node
    if (pos == 0 && *(getSize(rec.getData())) > 0 && res != root) {
        void *next_value = getIndexValue(rec.getData(), 0);
        RID *next_rid = getIndexRID(rec.getData(), 0);
        while (*(getFather(rec.getData())) != root) {
            res = *(getFather(rec.getData()));
            assert(getRec(res, rec) == true);
            for (int i = 0; i < *(getSize(rec.getData())); ++i)
            if (indexEQ(pData, rid, getIndexValue(rec.getData(), i), *(getIndexRID(rec.getData(), i)))) {
                memcpy(getIndexValue(rec.getData(), i), next_value, attrLength);
                memcpy(getIndexRID(rec.getData(), i), next_rid, sizeof(RID));
                updateRec(rec);
                return true;
            }
        }
        return true;
    }

    if (*(getSize(rec.getData())) == 0) {
        RID prev, next;
        memcpy(&prev, getPrev(rec.getData()), sizeof(RID));
        memcpy(&next, getNext(rec.getData()), sizeof(RID));
        if (prev != RID(-1, -1)) {
            IX_Record prec;
            assert(getRec(prev, prec) == true);
            memcpy(getNext(prec.getData()), &next, sizeof(RID));
            updateRec(prec);
        }
        if (next != RID(-1, -1)) {
            IX_Record nrec;
            assert(getRec(next, nrec) == true);
            memcpy(getPrev(nrec.getData()), &prev, sizeof(RID));
            updateRec(nrec);
        }
        deleteNode(*(getFather(rec.getData())), res);
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
    
    if (*(getIsLeaf(rec.getData())) == 1) {
        res = u;
        for (int i = 0; i < *(getSize(rec.getData())); ++i) {
            if (indexEQ(pData, rid, getIndexValue(rec.getData(), i), *(getIndexRID(rec.getData(), i)))) {
                pos = i;
                return false;
            }
            if (indexLT(pData, rid, getIndexValue(rec.getData(), i), *(getIndexRID(rec.getData(), i)))) {
                pos = i;
                return true;
            }
        }
        pos = *(getSize(rec.getData()));
        return true;
    }

    for (int i = 0; i < *(getSize(rec.getData())); ++i)
    if (indexLT(pData, rid, getIndexValue(rec.getData(), i), *(getIndexRID(rec.getData(), i)))) {
        return findInsertPos(*(getChild(rec.getData(), i)), pData, rid, res, pos);
    }
    return findInsertPos(*(getChild(rec.getData(), *(getSize(rec.getData())))), pData, rid, res, pos);
}


void IX_IndexHandle::searchFirst(RID u, RID &res, int &pos) const {
    IX_Record rec;
    if (!getRec(u, rec)) {
        res = RID(-1, -1);
        pos = -1;
        return;
    }

    if (*(getIsLeaf(rec.getData())) == 1) {
        res = u;
        pos = 0;
        return;
    }
    searchFirst(*(getChild(rec.getData(), 0)), res, pos);
}


void IX_IndexHandle::searchGE(RID u, void *pData, const RID &rid, RID &res, int &pos) const {
    IX_Record rec;
    if (!getRec(u, rec)) {
        res = RID(-1, -1);
        pos = -1;
        return;
    }

    if (*(getIsLeaf(rec.getData())) == 1) {
        res = u;
        for (int i = 0; i < *(getSize(rec.getData())); ++i)
        if (indexGE(getIndexValue(rec.getData(), i), *(getIndexRID(rec.getData(), i)), pData, rid)) {
            pos = i;
            return;
        }
        res = RID(-1, -1);
        pos = -1;
        return;
    }

    for (int i = 0; i < *(getSize(rec.getData())); ++i)
    if (indexGT(getIndexValue(rec.getData(), i), *(getIndexRID(rec.getData(), i)), pData, rid)) {
        searchGE(*(getChild(rec.getData(), i)), pData, rid, res, pos);
        if (pos != -1) {
            return;
        }
    }
    searchGE(*(getChild(rec.getData(), *(getSize(rec.getData())))), pData, rid, res, pos);
}


void IX_IndexHandle::searchLT(RID u, void *pData, const RID &rid, RID &res, int &pos) const {
    IX_Record rec;
    if (!getRec(u, rec)) {
        res = RID(-1, -1);
        pos = -1;
        return;
    }
    
    if (*(getIsLeaf(rec.getData())) == 1) {
        res = u;
        for (int i = *(getSize(rec.getData())) - 1; i >= 0; --i)
        if (indexLT(getIndexValue(rec.getData(), i), *(getIndexRID(rec.getData(), i)), pData, rid)) {
            pos = i;
            return;
        }
        res = RID(-1, -1);
        pos = -1;
        return;
    }

    for (int i = *(getSize(rec.getData())) - 1; i >= 0; --i)
    if (indexLT(getIndexValue(rec.getData(), i), *(getIndexRID(rec.getData(), i)), pData, rid)) {
        searchLT(*(getChild(rec.getData(), i + 1)), pData, rid, res, pos);
        if (pos != -1) {
            return;
        }
    }
    searchLT(*(getChild(rec.getData(), 0)), pData, rid, res, pos);
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
            rid = *(getPrev(rec.getData()));
            
            if (!getRec(rid, rec)) {
                rid = RID(-1, -1);
                pos = -1;
                return;
            }
            pos = *(getSize(rec.getData())) - 1;
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
        
        if (pos == *(getSize(rec.getData())) - 1) {
            rid = *(getNext(rec.getData()));
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
    
    for (int pos = 0; pos <= *(getSize(rec.getData())); ++pos)
    if (*(getChild(rec.getData(), pos)) == v) {
        // delete child
        for (int i = pos; i < *(getSize(rec.getData())); ++i) {
            memcpy(getChild(rec.getData(), i), getChild(rec.getData(), i + 1), sizeof(RID));
        }
        memset(getChild(rec.getData(), *(getSize(rec.getData()))), 0, sizeof(RID));

        // delete index
        for (int i = max(pos - 1, 0); i < *(getSize(rec.getData())) - 1; ++i) {
            memcpy(getIndexValue(rec.getData(), i), getIndexValue(rec.getData(), i + 1), attrLength);
            memcpy(getIndexRID(rec.getData(), i), getIndexRID(rec.getData(), i + 1), sizeof(RID));
        }
        memset(getIndexValue(rec.getData(), *(getSize(rec.getData())) - 1), 0, attrLength);
        memset(getIndexRID(rec.getData(), *(getSize(rec.getData())) - 1), 0, sizeof(RID));

        --(*(getSize(rec.getData())));
        updateRec(rec);
        break;
    }

    if (*(getSize(rec.getData())) == 0) {
        deleteNode(*(getFather(rec.getData())), u);
        deleteRec(u);
    }
}


/* let v be the right neighbor of u with lowerbound (pData, rid) */
void IX_IndexHandle::combinePair(RID &u, RID &v, void *pData, const RID &rid) {
    if (u == root) {
        char *tempData = new char[recordSize];
        memset(tempData, 0, recordSize);
        *getIsLeaf(tempData) = 0;
        *getSize(tempData) = 1;
        memcpy(getIndexValue(tempData, 0), pData, attrLength);
        memcpy(getIndexRID(tempData, 0), &rid, sizeof(RID));
        memcpy(getChild(tempData, 0), &u, sizeof(RID));
        memcpy(getChild(tempData, 1), &v, sizeof(RID));
        *(getPrev(tempData)) = RID(-1, -1);
        *(getNext(tempData)) = RID(-1, -1);
        insertRec(tempData, root);

        IX_FileHeaderPage *header = (IX_FileHeaderPage *)getPageData(0, true);
        header->root = root;

        IX_Record uRec, vRec;
        assert(getRec(u, uRec) == true);
        memcpy(getFather(uRec.getData()), &root, sizeof(RID));
        updateRec(uRec);

        assert(getRec(v, vRec) == true);
        memcpy(getFather(vRec.getData()), &root, sizeof(RID));
        updateRec(vRec);
        return;
    }

    RID f;
    IX_Record uRec, faRec;
    assert(getRec(u, uRec) == true);
    memcpy(&f, getFather(uRec.getData()), sizeof(RID));
    assert(getRec(f, faRec) == true);
    
    int pos = -1;
    for (int i = 0; i <= *(getSize(faRec.getData())); ++i)
    if (*(getChild(faRec.getData(), i)) == u) {
        pos = i;
        break;
    }

    if (*(getSize(faRec.getData())) < 4) {
        for (int i = *(getSize(faRec.getData())); i >= pos + 1; --i) {
            memcpy(getIndexValue(faRec.getData(), i), getIndexValue(faRec.getData(), i - 1), attrLength);
            memcpy(getIndexRID(faRec.getData(), i), getIndexRID(faRec.getData(), i - 1), sizeof(RID));
        }
        memcpy(getIndexValue(faRec.getData(), pos), pData, attrLength);
        memcpy(getIndexRID(faRec.getData(), pos), &rid, sizeof(RID));

        for (int i = *(getSize(faRec.getData())) + 1; i >= pos + 2; --i) {
            memcpy(getChild(faRec.getData(), i), getChild(faRec.getData(), i - 1), sizeof(RID));
        }
        memcpy(getChild(faRec.getData(), pos + 1), &v, sizeof(RID));
        ++(*(getSize(faRec.getData())));
        updateRec(faRec);

        return;
    }

    // split
    assert(*(getSize(faRec.getData())) == 4);
    void *tempIndexValue[5];
    RID tempIndexRID[5];
    RID tempChild[6];
    // get index list
    int j = -1;
    for (int i = 0; i < pos; ++i) {
        ++j;
        tempIndexValue[j] = new char[attrLength];
        memcpy(tempIndexValue[j], getIndexValue(faRec.getData(), i), attrLength);
        memcpy(&(tempIndexRID[j]), getIndexRID(faRec.getData(), i), sizeof(RID));
    }
    ++j;
    tempIndexValue[j] = new char[attrLength];
    memcpy(tempIndexValue[j], pData, attrLength);
    memcpy(&(tempIndexRID[j]), &rid, sizeof(RID));
    for (int i = pos; i < *(getSize(faRec.getData())); ++i) {
        ++j;
        tempIndexValue[j] = new char[attrLength];
        memcpy(tempIndexValue[j], getIndexValue(faRec.getData(), i), attrLength);
        memcpy(&(tempIndexRID[j]), getIndexRID(faRec.getData(), i), sizeof(RID));
    }
    // get child list
    int k = -1;
    for (int i = 0; i <= pos; ++i) {
        ++k;
        memcpy(&(tempChild[k]), getChild(faRec.getData(), i), sizeof(RID));
    }
    ++k;
    memcpy(&(tempChild[k]), &v, sizeof(RID));
    for (int i = pos + 1; i <= *(getSize(faRec.getData())); ++i) {
        ++k;
        memcpy(&(tempChild[k]), getChild(faRec.getData(), i), sizeof(RID));
    }

    const char* tempData = new char[recordSize];
    RID newRID;
    assert(insertRec(tempData, newRID) == true);
    IX_Record newRec;
    assert(getRec(newRID, newRec) == true);
    
    // updata faRec
    *(getSize(faRec.getData())) = 2;
    for (int i = 0; i <= 2; ++i) {
        if (i != 2) {
            memcpy(getIndexValue(faRec.getData(), i), tempIndexValue[i], attrLength);
            memcpy(getIndexRID(faRec.getData(), i), &(tempIndexRID[i]), sizeof(RID));
        }
        memcpy(getChild(faRec.getData(), i), &(tempChild[i]), sizeof(RID));
    }
    updateRec(faRec);

    // updata newRec
    *(getIsLeaf(newRec.getData())) = 0;
    *(getSize(newRec.getData())) = 2;
    for (int i = 0; i <= 2; ++i) {
        if (i != 2) {
            memcpy(getIndexValue(newRec.getData(), i), tempIndexValue[3 + i], attrLength);
            memcpy(getIndexRID(newRec.getData(), i), &(tempIndexRID[3 + i]), sizeof(RID));
        }
        memcpy(getChild(newRec.getData(), i), &(tempChild[3 + i]), sizeof(RID));
    }
    memcpy(getFather(newRec.getData()), &f, sizeof(RID));
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


int *IX_IndexHandle::getIsLeaf(char *pData) const {
    return (int*)pData;
}


int *IX_IndexHandle::getSize(char *pData) const {
    return (int*)(pData + sizeof(int));
}


RID *IX_IndexHandle::getIndexRID(char *pData, int i) const {
    return (RID*)(pData + sizeof(int) + sizeof(int) + sizeof(RID) * i);
}


RID *IX_IndexHandle::getChild(char *pData, int i) const {
    return (RID*)(pData + sizeof(int) + sizeof(int) + sizeof(RID) * (4 + i));
}


RID *IX_IndexHandle::getFather(char *pData) const {
    return (RID*)(pData + sizeof(int) + sizeof(int) + sizeof(RID) * (4 + 5));
}


RID *IX_IndexHandle::getPrev(char *pData) const {
    return (RID*)(pData + sizeof(int) + sizeof(int) + sizeof(RID) * (4 + 5 + 1));
}


RID *IX_IndexHandle::getNext(char *pData) const {
    return (RID*)(pData + sizeof(int) + sizeof(int) + sizeof(RID) * (4 + 5 + 1 + 1));
}


void *IX_IndexHandle::getIndexValue(char *pData, int i) const {
    return (void*)(pData + sizeof(int) + sizeof(int) + sizeof(RID) * (4 + 5 + 1 + 1 + 1) + attrLength * i);
}