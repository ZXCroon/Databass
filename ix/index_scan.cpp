#include "ix.h"


IX_IndexScan::IX_IndexScan() : open(false) {}



IX_IndexScan::~IX_IndexScan() {}


RC IX_IndexScan::openScan(const IX_IndexHandle &indexHandle, CompOp compOp, void *value) {
    if (compOp == NE_OP) {
        return IX_INDEXSCAN_INVALIDOP;
    }
    this->indexHandle = &indexHandle;
    //todo after IX_IndexHandle
    // this->attrType = indexHandle.
    // this->attrLength = indexHandle.
    this->compOp = compOp;
    this->value = value;
    start = true;
    open = true;
    return 0;
}


RC IX_IndexScan::closeScan::closeScan() {
    if (!open) {
        return IX_INDEXSCAN_NOTOPEN;
    }
    start = true;
    open = false;
    return 0;
}


RC IX_IndexScan::getNextEntry(RID &rid) {
    if (!open) {
        return IX_INDEXSCAN_NOTOPEN;
    }
    //todo B+ tree search
    return 0;
}