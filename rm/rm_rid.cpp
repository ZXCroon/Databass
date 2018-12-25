#include "rm_rid.h"


RID::RID() {}


RID::RID(PageNum pageNum, SlotNum slotNum) : pageNum(pageNum), slotNum(slotNum) {}


PageNum RID::getPageNum() const {
    return pageNum;
}


SlotNum RID::getSlotNum() const {
    return slotNum;
}


RID &RID::operator = (const RID &r) {
    this->pageNum = r.pageNum;
    this->slotNum = r.slotNum;
    return *this;
}


bool RID::operator < (const RID &r) {
    return pageNum < r.pageNum || (pageNum == r.pageNum && slotNum < r.slotNum);
}


bool RID::operator > (const RID &r) {
    return pageNum > r.pageNum || (pageNum == r.pageNum && slotNum > r.slotNum);
}


bool RID::operator == (const RID &r) {
    return pageNum == r.pageNum && slotNum == r.slotNum;
}


bool RID::operator <= (const RID &r) {
    return !(*this > r);
}


bool RID::operator >= (const RID &r) {
    return !(*this < r);
}


bool RID::operator != (const RID &r) {
    return !(*this == r);
}
