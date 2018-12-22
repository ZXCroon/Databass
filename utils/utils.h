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


int cmpDate(char *val1, char *val2) {
  unsigned short y1 = *(unsigned short *)val1;
  unsigned char m1 = *(unsigned char *)(val1 + 2);
  unsigned char d1 = *(unsigned char *)(val1 + 3);
  unsigned short y2 = *(unsigned short *)val2;
  unsigned char m2 = *(unsigned char *)(val2 + 2);
  unsigned char d2 = *(unsigned char *)(val2 + 3);

  if (y1 < y2) {
    return -1;
  } else if (y1 > y2) {
    return 1;
  }

  if (m1 < m2) {
    return -1;
  } else if (m1 > m2) {
    return 1;
  }

  if (d1 < d2) {
    return -1;
  } else if (d1 > d2) {
    return 1;
  }

  return 0;
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

    case STRING:
    case VARSTRING:
    case DATE: {
        int cmp;
        char *u = pData, *v = (char *)value;
        if (attrType == STRING) {
            cmp = strncmp(u, v, attrLength);
        } else if (attrType == VARSTRING) {
            cmp = strcmp(u, v);
        } else {
            cmp = cmpDate((char *)pData, (char *)value);
        }
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

    throw;
}


#endif
