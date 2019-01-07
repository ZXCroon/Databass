#include "ix.h"


IX_Record::IX_Record() : size(0), attrLength(0), pData(NULL) {}


IX_Record::IX_Record(int size, int attrLength, const RID &rid) : size(size), attrLength(attrLength), rid(rid), pData(new char[size]) {
}


IX_Record::~IX_Record() {
    if (pData != NULL) {
        delete[] pData;
    }
}


IX_Record &IX_Record::operator=(const IX_Record &rec) {
    this->size = rec.size;
    this->attrLength = rec.attrLength;
    if (this->pData != NULL) {
        delete[] this->pData;
    }
    this->pData = new char[rec.size];
    memcpy(this->pData, rec.pData, rec.size);
    this->rid = rec.rid;
    return *this;
}


char *IX_Record::getData() const {
    return pData;
}


RID IX_Record::getRid() const {
    return rid;
}
