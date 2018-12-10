#include "rm.h"


RM_Record::RM_Record() : size(0), pData(NULL) {}


RM_Record::RM_Record(int size, const RID &rid) : size(size), rid(rid), pData(new char[size]) {}


RM_Record::~RM_Record() {
    if (pData != NULL) {
        delete[] pData;
    }
}


char *RM_Record::getData() const {
    return pData;
}


RID RM_Record::getRid() const {
    return rid;
}


void RM_Record::nullify() {
    delete[] pData;
    pData = NULL;
}
