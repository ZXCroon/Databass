#include "ix.h"


IX_Record::IX_Record(int size, const RID &rid) : size(size), rid(rid), pData(new char[size]) {}


IX_Record::~IX_Record() {
    if (pData != NULL) {
        delete[] pData;
    }
}


char *IX_Record::getData() const {
    return pData;
}


RID IX_Record::getRid() const {
    return rid;
}


void IX_Record::nullify() {
    delete[] pData;
    pData = NULL;
}
