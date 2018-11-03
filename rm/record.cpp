#include "rm.h"


RM_Record::RM_Record() {}


RM_Record::RM_Record(int size, const RID &rid) : size(size), rid(rid), pData(new char[size]) {}


RM_Record::~RM_Record() {
    delete[] pData;
}


char *RM_Record::getData() {
    return pData;
}


RID RM_Record::getRid() {
    return rid;
}
