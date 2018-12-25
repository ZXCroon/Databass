#ifndef UTILS_H
#define UTILS_H

#include "defs.h"


bool isNull(void *value, int attrLength);

int cmpDate(char *val1, char *val2);

bool validate(char *pData, AttrType attrType, int attrLength, CompOp compOp, void *value);


#endif
