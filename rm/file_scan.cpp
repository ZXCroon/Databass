#include <cstring>
#include "rm.h"
#include "../utils/utils.h"


RM_FileScan::RM_FileScan() : open(false) {}


RM_FileScan::~RM_FileScan() {}


void RM_FileScan::openScan(RM_FileHandle &fileHandle, AttrType attrType,
                         int attrLength, int attrOffset, CompOp compOp, const void *value) {
    this->fileHandle = &fileHandle;
    this->attrType = attrType;
    this->attrLength = attrLength;
    this->attrOffset = attrOffset;
    this->compOp = compOp;
    this->value = value;
    open = true;
    this->fileHandle->startVisiting();
}


RC RM_FileScan::closeScan() {
    if (!open) {
        return RM_FILESCAN_NOTOPEN;
    }
    open = false;
    return 0;
}


RC RM_FileScan::getNextRec(RM_Record &rec) {
    if (!open) {
        return RM_FILESCAN_NOTOPEN;
    }

    RID rid;
    while (true) {
        if (!fileHandle->getNextRid(rid)) {
            return RM_FILESCAN_NONEXT;
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
