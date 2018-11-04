#include "rm_rid.h"


RID::RID() {}


RID::RID(PageNum pageNum, SlotNum slotNum) : pageNum(pageNum), slotNum(slotNum) {}


PageNum RID::getPageNum() const {
    return pageNum;
}


SlotNum RID::getSlotNum() const {
    return slotNum;
}
