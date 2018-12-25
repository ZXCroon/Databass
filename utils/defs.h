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
const int IS_OP = 6;
const int NO_OP = 7;
const int AND_OP = 8;
const int ISNULL_OP = 9;
const int NOTNULL_OP = 10;


//
// Attribute Types
//
typedef int AttrType;
const int INT = 0;
const int FLOAT = 1;
const int STRING = 2;
const int VARSTRING = 3;
const int DATE = 4;


//
// JOIN Types
// 
typedef int JoinType;
const int NO_JOIN = 0;
const int INNER_JOIN = 1;
const int LEFT_JOIN = 2;
const int RIGHT_JOIN = 3;
const int FULL_JOIN = 4;


// 
// NOT NULL
// 
typedef int NotNull;
const int CAN_BE_NULL = 0;
const int NOT_NULL = 1;


// 
// IS PRIMARY
// 
typedef int IsPrimary;
const int NOT_PRIMARY = 0;
const int IS_PRIMARY = 1;


// 
// IS FOREIGN
// 
typedef int IsForeign;
const int NOT_FOREIGN = 0;
const int IS_FOREIGN = 1;


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
const int IX_INDEXSCAN_EOF = IX_INDEXSCAN_INVALIDOP + 1;
/* QL */
const int QL_NO_SUCH_RELATION = IX_INDEXSCAN_EOF + 1;
const int QL_NO_SUCH_ATTRIBUTE = QL_NO_SUCH_RELATION + 1;

//
// Constraints
//
const int MAXNAME = 24;
const int MAXATTRS = 40;
const int MAXSTRINGLEN = 255;

const int RM_RECORD_MIN_SIZE = 32;
const int RM_RECORD_MAX_SIZE = 2048;


#endif
