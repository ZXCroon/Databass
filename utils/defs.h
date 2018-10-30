#ifndef DEF_H
#define DEF_H


typedef unsigned int PageNum;
const unsigned int ALL_PAGES = -1;
typedef unsigned int SlotNum;


//
// Comparison Operators
//
typedef unsigned int CompOp;
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
typedef unsigned int AttrType;
const int INT = 0;
const int FLOAT = 1;
const int STRING = 2;


//
// Return Codes
//
typedef unsigned int RC;

/* RM */



#endif DEF_H
