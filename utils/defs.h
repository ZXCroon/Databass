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



#endif DEF_H
