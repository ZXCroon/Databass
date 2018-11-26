#ifndef DEF_H
#define DEF_H


typedef int PageNum;
const int ALL_PAGES = -1;
const int NO_PAGE = -2;
typedef int SlotNum;
typedef unsigned long long Bits;


//
// Comparison Operators
//
typedef int CompOp;
const int EQ_OP = 0;
const int LT_OP = 1;
const int GT_OP = 2;
const int LE_OP = 3;
const int GE_OP = 4;
const int NE_OP = 5;
const int NO_OP = 6;


//
// Attribute Types
//
typedef int AttrType;
const int INT = 0;
const int FLOAT = 1;
const int STRING = 2;


//
// Return Codes
//
typedef int RC;

/* RM */
const int RM_MANAGER_CREATEFAILED = 1;
const int RM_MANAGER_RECORDSIZEINVALID = RM_MANAGER_CREATEFAILED + 1;
const int RM_FILESCAN_NONEXT = RM_MANAGER_RECORDSIZEINVALID + 1;
const int RM_FILESCAN_NOTOPEN = RM_FILESCAN_NONEXT + 1;

/* IX */
const int IX_MANAGER_CREATEFAILED = RM_FILESCAN_NOTOPEN + 1;
const int IX_INDEXSCAN_NOTOPEN = IX_MANAGER_CREATEFAILED + 1;
const int IX_INDEXSCAN_INVALIDOP = IX_INDEXSCAN_NOTOPEN + 1;

//
// Constraints
//
const int RM_RECORD_MIN_SIZE = 32;
const int RM_RECORD_MAX_SIZE = 2048;


#endif
