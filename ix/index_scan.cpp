#include "ix.h"

extern bool isDebug;

IX_IndexScan::IX_IndexScan() : open(false) {}


IX_IndexScan::~IX_IndexScan() {}


RC IX_IndexScan::openScan(const IX_IndexHandle &indexHandle, CompOp compOp, void *value) {
    if (compOp == NE_OP) {
        return IX_INDEXSCAN_INVALIDOP;
    }
    this->indexHandle = &indexHandle;
    this->compOp = compOp;
    this->value = value;
    RID none(-1, -1);
    RID inf(100000000, 100000000);
    if (isDebug) {
        printf("DEBUG entering scan!!!\n");
        printf("DEBUG scanning %d\n", compOp);
        printf("DEBUG scanning %d\n", *((int*)value));
    }
    switch (compOp){
        case EQ_OP:
            indexHandle.searchGE(indexHandle.getRoot(), value, none, res, pos);
            direct = 1;
            break;
        case LT_OP:
            indexHandle.searchLT(indexHandle.getRoot(), value, none, res, pos);
            direct = 0;
            break;
        case GT_OP:
            indexHandle.searchGE(indexHandle.getRoot(), value, inf, res, pos);
            direct = 1;
            break;
        case LE_OP:
            indexHandle.searchLT(indexHandle.getRoot(), value, inf, res, pos);
            direct = 0;
            break;
        case GE_OP:
            indexHandle.searchGE(indexHandle.getRoot(), value, none, res, pos);
            direct = 1;
            break;
        case NO_OP:
            indexHandle.searchFirst(indexHandle.getRoot(), res, pos);
            direct = 1;
            break;
    }
    start = true;
    open = true;
    return 0;
}


RC IX_IndexScan::closeScan() {
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

    if (pos == -1) {
        return IX_INDEXSCAN_EOF;
    }

    IX_Record rec;
    if (!indexHandle->getRec(res, rec)) {
        return IX_INDEXSCAN_EOF;
    }

    if (compOp == EQ_OP && !indexHandle->indexEQ(value, RID(-1, -1), indexHandle->getIndexValue(rec.getData(), pos), RID(-1, -1))) {
        return IX_INDEXSCAN_EOF;
    }
    //todo get the record of res in node
    rid = *(indexHandle->getIndexRID(rec.getData(), pos));
    indexHandle->searchNext(res, pos, direct);
    if (isDebug) {
        printf("DEBUG: scan using IX!!!\n");
    }
    return 0;
}