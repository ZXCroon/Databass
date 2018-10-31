#ifndef DEF_H
#define DEF_H


typedef unsigned int PageNum;
const unsigned int ALL_PAGES = -1;
typedef unsigned int SlotNum;


//
// Comparison Operators
//
typedef unsigned int CompOp;
const unsigned int EQ_OP = 0;
const unsigned int LT_OP = 1;
const unsigned int GT_OP = 2;
const unsigned int LE_OP = 3;
const unsigned int GE_OP = 4;
const unsigned int NE_OP = 5;
const unsigned int NO_OP = 6;


//
// Attribute Types
//
typedef unsigned int AttrType;
const unsigned int INT = 0;
const unsigned int FLOAT = 1;
const unsigned int STRING = 2;


//
// Return Codes
//
typedef unsigned int RC;

/* RM */
const unsigned int RID_INVALILD = 1;
const unsigned int RM_MANAGER_OPENFAILED = RID_INVALILD + 1;
const unsigned int RM_MANAGER_CLOSEFAILED = RM_MANAGER_OPENFAILED + 1;
const unsigned int RM_MANAGER_CREATEFAILED = RM_MANAGER_CLOSEFAILED + 1;
const unsigned int RM_RECORD_INVALID = RM_MANAGER_CREATEFAILED + 1;
const unsigned int RM_FILEHANDLE_NORID = RM_RECORD_INVALID + 1;
const unsigned int RM_FILESCAN_NONEXT = RM_FILEHANDLE_NORID + 1;
const unsigned int RM_FILESCAN_NOTOPEN = RM_FILESCAN_NONEXT + 1;



#endif DEF_H
