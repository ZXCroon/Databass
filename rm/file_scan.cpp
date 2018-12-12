#include <cstring>
#include "rm.h"
#include "../utils/utils.h"


RM_FileScan::RM_FileScan() : open(false) {}


RM_FileScan::~RM_FileScan() {}


void RM_FileScan::openScan(const RM_FileHandle &fileHandle, AttrType attrType,
                         int attrLength, int attrOffset, CompOp compOp, void *value) {
    this->fileHandle = &fileHandle;
    this->attrType = attrType;
    this->attrLength = attrLength;
    this->attrOffset = attrOffset;
    this->compOp = compOp;
    this->value = value;
    start = true;
    open = true;
}


RC RM_FileScan::closeScan() {
    if (!open) {
        return RM_FILESCAN_NOTOPEN;
    }
    start = true;
    open = false;
    return 0;
}


RC RM_FileScan::getNextRec(RM_Record &rec) {
    if (!open) {
        return RM_FILESCAN_NOTOPEN;
    }

    while (true) {
        if (start) {
            start = false;
            if (!fileHandle->getFirstRid(rid)) {
                return RM_FILESCAN_NONEXT;
            }
        } else {
            RID lastRid = rid;
            if (!fileHandle->getNextRid(lastRid, rid)) {
                return RM_FILESCAN_NONEXT;
            }
        }
        fileHandle->getRec(rid, rec);
        char *pData = rec.getData();
        if (pData) {
            pData += attrOffset;
        }
        if (validate(pData, attrType, attrLength, compOp, value)) {
            return 0;
        }
    }
}
