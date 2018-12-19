#ifndef RM_RID_H
#define RM_RID_H

#include "../utils/defs.h"


class RID {

public:
    RID();
    RID(PageNum pageNum, SlotNum slotNum);

    PageNum getPageNum() const;
    SlotNum getSlotNum() const;

    bool operator < (const RID &r);
    bool operator > (const RID &r);
    bool operator == (const RID &r);
    bool operator <= (const RID &r);
    bool operator >= (const RID &r);
    bool operator != (const RID &r);

private:
    PageNum pageNum;
    SlotNum slotNum;

};


#endif
