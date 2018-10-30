#ifndef RM_RID_H
#define RM_RID_H

#include "utils/defs.h"


class RID {

public:
    RID();
    ~RID();
    RID(PageNum pageNum, SlotNum slotNum);

    RC getPageNum(PageNum &pageNum) const;
    RC getSlotNum(SlotNum &slotNum) const;

};


#endif
