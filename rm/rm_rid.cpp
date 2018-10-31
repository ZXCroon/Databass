#include "rm_rid.h"


RID::RID() : valid(false) {}


RID::~RID() {}


RID::RID(PageNum pageNum, SlotNum, slotNum) : pageNum(pageNum), slotNum(slotNum), valid(true) {}


RC RID::getPageNum(PageNum &pageNum) {
    if (valid) {
        pageNum = this->pageNum;
        return 0;
    } else {
        return RID_INVALILD;
    }
}


RC RID::getSlotNum(SlotNum &slotNum) {
    if (valid) {
        slotNum = this->slotNum;
        return 0;
    } else {
        return RID_INVALILD;
    }
}
