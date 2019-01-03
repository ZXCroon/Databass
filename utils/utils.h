#ifndef UTILS_H
#define UTILS_H

#include "defs.h"
#include "error.h"


bool isNull(const void *value, int attrLength);
int cmpDate(const char *val1, const char *val2);
bool convertToDate(char *value);
bool validate(const char *pData1, const char *pData2, AttrType attrType1, AttrType attrType2,
              int attrLength1, int attrLength2, CompOp compOp, bool strict);
bool validate(const char *pData, AttrType attrType, int attrLength, CompOp compOp, const void *value);
void print(const void *value, AttrType attrType, int attrLength);
void printAttrType(AttrType attrType, int attrLength = -1);


typedef unsigned long long ULL;
typedef unsigned int UI;
typedef unsigned short US;
typedef unsigned char UC;
/* 16 * 64 = 1024 */
struct BitMap {
    ULL leaf[16];
    US rootForEmpty, rootForFull;  // '1' indicates empty/full
};

void initBitMap(struct BitMap &bm);
int queryBit(struct BitMap bm, int index);
void setBit(struct BitMap &bm, int index, int b);
int findRightMost(struct BitMap bm, int b);
void print(const struct BitMap &bm);


#endif
