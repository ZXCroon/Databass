#ifndef UTILS_H
#define UTILS_H

#include <cstring>
#include "defs.h"


bool isNull(void *value, int attrLength) {
    for (int i = 0; i < attrLength; ++i) {
        if (*((char *)value) != 0) {
            return false;
        }
    }
    return true;
}


bool validate(char *pData, AttrType attrType, int attrLength,
                           CompOp compOp, void *value) {
    if (compOp == NO_OP) {
        return true;
    }
    if (pData == NULL) {
        return false;
    }

    // ensure in top levels that IS/== is used correctly
    if (compOp == IS_OP) {
        compOp = EQ_OP;
    }

    switch (attrType) {

    case INT: {
        int u = *(int *)pData, v = *(int *)value;
        switch (compOp) {
        case EQ_OP: return u == v;
        case LT_OP: return u < v;
        case GT_OP: return u > v;
        case LE_OP: return u <= v;
        case GE_OP: return u >= v;
        case NE_OP: return u != v;
        }
        break;
    }

    case FLOAT: {
        float u = *(float *)pData, v = *(float *)value;
        switch (compOp) {
        case EQ_OP: return u == v;
        case LT_OP: return u < v;
        case GT_OP: return u > v;
        case LE_OP: return u <= v;
        case GE_OP: return u >= v;
        case NE_OP: return u != v;
        }
        break;
    }

    case STRING: {
        char *u = pData, *v = (char *)value;
        int cmp = strncmp(u, v, attrLength);
        switch (compOp) {
        case EQ_OP: return cmp == 0;
        case LT_OP: return cmp < 0;
        case GT_OP: return cmp > 0;
        case LE_OP: return cmp <= 0;
        case GE_OP: return cmp >= 0;
        case NE_OP: return cmp != 0;
        }
        break;
    }

    }
}


#endif
