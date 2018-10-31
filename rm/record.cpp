#include "rm.h"


RM_Record::RM_Record() : pData(NULL) {}


RM_Record::~RM_Record() {}


RC RM_Record::getData(char *&pData) {
    if (this->pData) {
        pData = this->pData;
        return 0;
    } else {
        return RM_RECORD_INVALID;
    }
}


RC RM_Record::getRid(RID &rid) {
    rid = this->rid;
    return 0;
}
