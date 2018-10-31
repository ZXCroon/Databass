#include <cstring>
#include "rm.h"


RM_FileScan::RM_FileScan() : open(false) {}


RM_FileScan::~RM_FileScan() {}


RC RM_FileScan::openScan(const RM_FileHandle &fileHandle, AttrType attrType,
                         int attrLength, int attrOffset, CompOp compOp, void *value) {
    this->fileHandle = &fileHandle;
    this->attrType = attrType;
    this->attrLength = attrLength;
    this->attrOffset = attrOffset;
    this->compOp = compOp;
    this->value = value;
    start = true;
    open = true;
    return 0;
}


RC RM_FileScan::closeScan() {
    open = false;
    return 0;
}


RC RM_FileScan::getNextRec(RM_Record &rec) {
    if (!open) {
        return RM_FILESCAN_NOTOPEN;
    }

    RC rc;
    while (true) {
        if (start) {
            start = false;
            rc = fileHandle->getFirstRid(rid);
        } else {
            RID lastRid = rid;
            rc = fileHandle->getNextRid(lastRid, rid);
        }
        if (rc == RM_FILEHANDLE_NORID) {
            return RM_FILESCAN_NONEXT;
        }
        rc = fileHandle->getRec(rid, rec);
        char *pData;
        rc = rec.getData(pData);
        pData += attrOffset;
        if (validate(pData, attrType, attrLength, compOp, value) {
            return 0;
        }
    }
}


bool RM_FileScan::validate(char *pData, AttrType attrType, int attrLength,
                           CompOp compOp, void *value) {
    if (pData == NULL) {
        return compOp == NO_OP && value == NULL;
    }

    switch (attrType) {

    case INT: {
        int u = *(int *)pData, v = *(int *)value;
        switch (compOp) {
        case EQ_OP: return u == v;
        case LT_OP: return u < v;
        case GT_OP: return u > v;
        case LE_OP: return u <= v;
        case GE_OP: return u >= v;
        case NE_OP: return u != v;
        }
        break;
    }

    case FLOAT: {
        float u = *(float *)pData, v = *(float *)value;
        switch (compOp) {
        case EQ_OP: return u == v;
        case LT_OP: return u < v;
        case GT_OP: return u > v;
        case LE_OP: return u <= v;
        case GE_OP: return u >= v;
        case NE_OP: return u != v;
        }
        break;
    }

    case STRING: {
        char *u = pData, *v = (char *)value;
        int cmp = strncmp(u, v, attrLength);
        switch (compOp) {
        case EQ_OP: return cmp == 0
        case LT_OP: return cmp < 0;
        case GT_OP: return cmp > 0;
        case LE_OP: return cmp <= 0;
        case GE_OP: return cmp >= 0;
        case NE_OP: return cmp != 0;
        }
        break;
    }

    }
}
