#include "ix.h"


IX_Record::IX_Record() : size(0), attrLength(0), pData(NULL) {}


IX_Record::IX_Record(int size, int attrLength, const RID &rid) : size(size), attrLength(attrLength), rid(rid), pData(new char[size]) {}


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
