#include "rm_rid.h"


RID::RID() {}


RID::RID(PageNum pageNum, SlotNum, slotNum) : pageNum(pageNum), slotNum(slotNum) {}


PageNum RID::getPageNum() {
    return pageNum;
}


SlotNum RID::getSlotNum() {
    return slotNum;
}
