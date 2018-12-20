#include "ix.h"


IX_Record::IX_Record() : size(0), attrLength(0), pData(NULL) {}


IX_Record::IX_Record(int size, int attrLength, const RID &rid) : size(size), attrLength(attrLength), rid(rid), pData(new char[size]) {}


IX_Record::~IX_Record() {
    if (pData != NULL) {
        delete[] pData;
    }
}


bool *IX_Record::getIsLeaf() const {
    return (bool*)pData;
}


int *IX_Record::getSize() const {
    return (int*)(pData + sizeof(int));
}


RID *IX_Record::getIndexRID(int i) const {
    return (RID*)(pData + sizeof(int) + sizeof(int) + sizeof(RID) * i);
}


RID *IX_Record::getChild(int i) const {
    return (RID*)(pData + sizeof(int) + sizeof(int) + sizeof(RID) * (4 + i));
}


RID *IX_Record::getFather() const {
    return (RID*)(pData + sizeof(int) + sizeof(int) + sizeof(RID) * (4 + 5));
}


RID *IX_Record::getPrev() const {
    return (RID*)(pData + sizeof(int) + sizeof(int) + sizeof(RID) * (4 + 5 + 1));
}


RID *IX_Record::getNext() const {
    return (RID*)(pData + sizeof(int) + sizeof(int) + sizeof(RID) * (4 + 5 + 1 + 1));
}


void *IX_Record::getIndexValue(int i) const {
    return (void*)(pData + sizeof(int) + sizeof(int) + sizeof(RID) * (4 + 5 + 1 + 1 + 1) + attrLength * i);
}


char *IX_Record::getData() const {
    return pData;
}


RID IX_Record::getRid() const {
    return rid;
}