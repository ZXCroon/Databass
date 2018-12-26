#include <cstring>
#include <vector>
#include "ql.h"


QL_Manager::QL_Manager(SM_Manager *smm, IX_Manager *ixm, RM_Manager *rmm) :
        smm(smm), ixm(ixm), rmm(rmm) {}


QL_Manager::~QL_Manager() {}


void QL_Manager::insert(const char *relName, int nValues, Value values[]) {
    Catalog cat;
    getCatalog(relName, cat);
    if (nValues != cat.relcat.attrCount) {
        // TODO
    }
    for (int i = 0; i < cat.relcat.attrCount; ++i) {
        if (!filterValue(values[i], &(cat.attrcats[i]))) {
            return;
        }
    }

    RM_FileHandle *handle;
    rmm->openFile(getPath(smm->dbName, relName), handle);
    for (int i = 0; i < cat.relcat.attrCount; ++i) {
        if (!checkUnique(values[i], &(cat.attrcats[i]), handle) || !checkReference(values[i], &(cat.attrcats[i]))) {
            rmm->closeFile(*handle);
            return;
        }
    }

    char *data = new char[cat.relcat.tupleLength];
    for (int i = 0; i < cat.relcat.attrCount; ++i) {
        memcpy(data + cat.attrcats[i].offset, padValue(values[i].data, values[i].type, cat.attrcats[i].attrLength),
               cat.attrcats[i].attrLength);
    }
    RID rid;
    handle->insertRec(data, rid);
    IX_IndexHandle *ixHandle;
    for (int i = 0; i < cat.relcat.attrCount; ++i) {
        if (cat.attrcats[i].indexNo != -1) {
            ixm->openIndex(getPath(smm->dbName, relName), cat.attrcats[i].indexNo, ixHandle);
            ixHandle->insertEntry(data + cat.attrcats[i].offset, rid);
            ixm->closeIndex(*ixHandle);
        }
    }
    delete[] data;
    rmm->closeFile(*handle);
    // int t;
    // if (strcmp(relName, "class") == 0) {
        // cin >> t;
    // }
}


void QL_Manager::del(const char *relName, int nConditions, const Condition conditions[]) {
    Catalog cat;
    getCatalog(relName, cat);
    RM_FileHandle *handle;
    IX_IndexHandle *ixHandle;
    RM_Record rec;
    RID rid;
    vector<RID> rids;
    rmm->openFile(getPath(smm->dbName, relName), handle);
    RM_FileScan fileScan;
    IX_IndexScan indexScan;

    SelectStrategy strat;
    decideStrategy(relName, relName, cat, cat, NO_JOIN, nConditions, conditions, strat);

    if (strat.strat1.attrcat != NULL) {
        ixm->openIndex(getPath(smm->dbName, relName), strat.strat1.attrcat->indexNo, ixHandle);
        indexScan.openScan(*ixHandle, strat.strat1.compOp,
                padValue(strat.strat1.value.data, strat.strat1.attrcat->attrType, strat.strat1.attrcat->attrLength));
        while (true) {
            RC rc = indexScan.getNextEntry(rid);
            if (rc == IX_INDEXSCAN_EOF) {
                break;
            }
            handle->getRec(rid, rec);
            if (singleValidate(relName, cat, nConditions, conditions, rec)) {
                rids.push_back(rid);
            }
        }
        indexScan.closeScan();
        ixm->closeIndex(*ixHandle);
    } else {
        fileScan.openScan(*handle, 0, 0, 0, NO_OP, NULL);
        while (true) {
            RC rc = fileScan.getNextRec(rec);
            if (rc == RM_FILESCAN_NONEXT) {
                break;
            }
            if (singleValidate(relName, cat, nConditions, conditions, rec)) {
                rids.push_back(rid);
            }
        }
        fileScan.closeScan();
    }

    for (int i = 0; i < cat.relcat.attrCount; ++i) {
        if (cat.attrcats[i].indexNo != -1) {
            ixm->openIndex(getPath(smm->dbName, relName), cat.attrcats[i].indexNo, ixHandle);
            for (int k = 0; k < rids.size(); ++k) {
                handle->getRec(rids[k], rec);
                ixHandle->deleteEntry(rec.getData() + cat.attrcats[i].offset, rids[k]);
            }
            ixm->closeIndex(*ixHandle);
        }
    }
    for (int i = 0; i < rids.size(); ++i) {
        handle->deleteRec(rids[i]);
    }
    rmm->closeFile(*handle);
}


void QL_Manager::update(const char *relName, const RelAttr &updAttr, const int bIsValue,
                        const RelAttr &rhsRelAttr, Value &rhsValue,
                        int nConditions, const Condition conditions[]) {
    Catalog cat;
    getCatalog(relName, cat);
    RM_FileHandle *handle;
    IX_IndexHandle *ixHandle;
    RM_Record rec;
    RID rid;
    vector<RID> rids;
    rmm->openFile(getPath(smm->dbName, relName), handle);
    RM_FileScan fileScan;
    IX_IndexScan indexScan;

    SelectStrategy strat;
    decideStrategy(relName, relName, cat, cat, NO_JOIN, nConditions, conditions, strat);

    if (strat.strat1.attrcat != NULL) {
        ixm->openIndex(getPath(smm->dbName, relName), strat.strat1.attrcat->indexNo, ixHandle);
        indexScan.openScan(*ixHandle, strat.strat1.compOp,
                padValue(strat.strat1.value.data, strat.strat1.attrcat->attrType, strat.strat1.attrcat->attrLength));
        while (true) {
            RC rc = indexScan.getNextEntry(rid);
            if (rc == IX_INDEXSCAN_EOF) {
                break;
            }
            handle->getRec(rid, rec);
            if (singleValidate(relName, cat, nConditions, conditions, rec)) {
                rids.push_back(rid);
            }
        }
        indexScan.closeScan();
        ixm->closeIndex(*ixHandle);
    } else {
        fileScan.openScan(*handle, 0, 0, 0, NO_OP, NULL);
        while (true) {
            RC rc = fileScan.getNextRec(rec);
            if (rc == RM_FILESCAN_NONEXT) {
                break;
            }
            if (singleValidate(relName, cat, nConditions, conditions, rec)) {
                rids.push_back(rec.getRid());
            }
        }
        fileScan.closeScan();
    }

    const AttrcatLayout *updAc = locateAttrcat(relName, cat, updAttr), *rhsAc;
    if (updAc == NULL) {
        return;
    }
    Value rhsValue_ = rhsValue;
    if (bIsValue) {
        rhsValue_ = rhsValue;
        if (!filterValue(rhsValue_, updAc) || !checkUnique(rhsValue, updAc, handle) || !checkReference(rhsValue, updAc)) {
            return;
        }
    } else {
        rhsAc = locateAttrcat(relName, cat, rhsRelAttr);
        rhsValue_.type = rhsAc->attrType;
        rhsValue_.data = new char[rhsAc->attrLength];
        memcpy(rhsValue_.data, rhsValue.data, rhsAc->attrLength);
    }
    if (bIsValue && !filterValue(rhsValue_, updAc)) {
        return;
    }

    if (updAc->indexNo != -1) {
        ixm->openIndex(getPath(smm->dbName, relName), updAc->indexNo, ixHandle);
        for (int k = 0; k < rids.size(); ++k) {
            handle->getRec(rids[k], rec);
            ixHandle->deleteEntry(rec.getData() + updAc->offset, rids[k]);
            if (bIsValue) {
                memcpy(rec.getData() + updAc->offset,
                        padValue(rhsValue_.data, updAc->attrType, updAc->attrLength), updAc->attrLength);
            } else {
                memcpy(rhsValue_.data, rec.getData() + rhsAc->offset, rhsAc->attrLength);
                if (!filterValue(rhsValue_, updAc)) {
                    delete[] (char *)rhsValue_.data;
                    return;
                }
                memcpy(rec.getData() + updAc->offset, rhsValue_.data, updAc->attrLength);
            }
            ixHandle->insertEntry(rec.getData() + updAc->offset, rids[k]);
        }
        ixm->closeIndex(*ixHandle);
    }
    for (int i = 0; i < rids.size(); ++i) {
        handle->getRec(rids[i], rec);
        if (bIsValue) {
            memcpy(rec.getData() + updAc->offset,
                    padValue(rhsValue_.data, updAc->attrType, updAc->attrLength), updAc->attrLength);
        } else {
            memcpy(rhsValue_.data, rec.getData() + rhsAc->offset, rhsAc->attrLength);
            if (!filterValue(rhsValue_, updAc)) {
                delete[] (char *)rhsValue_.data;
                return;
            }
            memcpy(rec.getData() + updAc->offset, rhsValue_.data, updAc->attrLength);
        }
        handle->updateRec(rec);
    }
    rmm->closeFile(*handle);
    if (!bIsValue) {
        delete[] (char *)rhsValue_.data;
    }
}


bool QL_Manager::getCatalog(const char *relName, Catalog &cat) {
    RID rid;
    if (!smm->getRelcatRid(relName, rid)) {
        return false;
    }
    RM_Record relRec;
    smm->relcatHandle->getRec(rid, relRec);
    smm->getRelcatFromRid(rid, cat.relcat);

    RID rids[MAXATTRS];
    int attrCount;
    if (!smm->getAttrcatRids(relName, NULL, rids, attrCount)) {
        return false;
    }

    for (int i = 0; i < attrCount; ++i) {
        smm->getAttrcatFromRid(rids[i], cat.attrcats[i]);
    }

    return true;
}


const AttrcatLayout *QL_Manager::locateAttrcat(const char *relName, const Catalog &cat, const RelAttr &ra) {
    if (ra.relName != NULL && strcmp(ra.relName, relName) != 0) {
        return NULL;
    }
    for (int i = 0; i < cat.relcat.attrCount; ++i) {
        if (strcmp(ra.attrName, cat.attrcats[i].attrName) == 0) {
            return &(cat.attrcats[i]);
        }
    }
    return NULL;
}


char *QL_Manager::getPath(const char *dbName, const char *relName) {
    strcpy(pathBuf, dbName);
    strcat(pathBuf, "/");
    strcat(pathBuf, relName);
    return pathBuf;
}


void *QL_Manager::padValue(void *value, AttrType attrType, int attrLength) {
    if (value == NULL) {
        memset(valBuf, NULL_BYTE, attrLength);
        return valBuf;
    }
    if (attrType != STRING) {
        return value;
    }
    int len = strlen((const char *)value);
    memset(valBuf, ' ', attrLength);
    strcpy(valBuf, (char *)value);
    if (len < attrLength) {
        valBuf[len] = ' ';
    }
    return valBuf;
}


bool QL_Manager::filterValue(Value &value, const AttrcatLayout *attrcat) {
    if (value.data == NULL) {
        if ((attrcat->constrFlag & 3) != 0) {
            Error::nullError(attrcat->attrName);
            return false;
        }
        value.type = attrcat->attrType;
        return true;
    }
    if (attrcat->attrType == value.type) {
        return true;
    }
    if (attrcat->attrType == STRING && value.type == VARSTRING) {
        value.type = STRING;
        return true;
    }
    if (attrcat->attrType == VARSTRING && value.type == STRING) {
        value.type = VARSTRING;
        return true;
    }
    if (attrcat->attrType == FLOAT && value.type == INT) {
        value.type = FLOAT;
        *((float *)value.data) = float(*((int *)value.data));
        return true;
    }
    if (attrcat->attrType == DATE && (value.type == STRING || value.type == VARSTRING)) {
        if (!convertToDate((char *)value.data)) {
            Error::invalidDateError();
            return false;
        }
        value.type = DATE;
        return true;
    }
    Error::typeError(attrcat->attrType, value.type);
    return false;
}


bool QL_Manager::checkUnique(Value &value, const AttrcatLayout *attrcat, RM_FileHandle *handle) {
    if (value.data == NULL || ((attrcat->constrFlag >> 1) & 1) == 0) {
        return true;
    }
    RM_FileScan scan;
    scan.openScan(*handle, attrcat->attrType, attrcat->attrLength, attrcat->offset, EQ_OP, value.data);
    RC rc;
    RM_Record rec;
    rc = scan.getNextRec(rec);
    scan.closeScan();
    if (rc == 0) {
        Error::primaryNotUniqueError(attrcat->attrName);
        return false;
    }
    return true;
}


bool QL_Manager::checkReference(Value &value, const AttrcatLayout *attrcat) {
    if (((attrcat->constrFlag >> 2) & 1) == 0) {
        return true;
    }
    RM_FileHandle *refHandle;
    Catalog refCat;
    getCatalog(attrcat->refRelName, refCat);
    RelAttr ra = {NULL, attrcat->refAttrName};
    const AttrcatLayout *refAc = locateAttrcat(attrcat->refRelName, refCat, ra);
    rmm->openFile(getPath(smm->dbName, attrcat->refRelName), refHandle);

    RM_FileScan scan;
    scan.openScan(*refHandle, refAc->attrType, refAc->attrLength, refAc->offset, EQ_OP, value.data);
    RC rc;
    RM_Record rec;
    rc = scan.getNextRec(rec);
    scan.closeScan();
    if (rc == RM_FILESCAN_NONEXT) {
        Error::referenceError(attrcat->attrName, refAc->relName, refAc->attrName);
        rmm->closeFile(*refHandle);
        return false;
    }
    return true;
}
