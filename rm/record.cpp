#include "rm.h"


RM_Record::RM_Record() : size(0), pData(NULL) {}


RM_Record::RM_Record(int size, const RID &rid) : size(size), rid(rid), pData(new char[size]) {}


RM_Record::~RM_Record() {
    if (pData != NULL) {
        delete[] pData;
    }
}


RM_Record &RM_Record::operator=(const RM_Record &rec) {
    this->size = rec.size;
    if (this->pData != NULL) {
        delete[] this->pData;
    }
    this->pData = new char[rec.size];
    memcpy(this->pData, rec.pData, rec.size);
    this->rid = rec.rid;
    return *this;
}


char *RM_Record::getData() const {
    return pData;
}


RID RM_Record::getRid() const {
    return rid;
}


void RM_Record::setRid(const RID &rid) {
    this->rid = rid;
}
