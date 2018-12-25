#include <cstring>
#include <iostream>
#include "utils.h"
#include "defs.h"


bool isNull(const void *value, int attrLength) {
    for (int i = 0; i < attrLength; ++i) {
        if (*((char *)value) != 0) {
            return false;
        }
    }
    return true;
}


int cmpDate(const char *val1, const char *val2) {
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


bool validate(const char *pData, AttrType attrType, int attrLength,
                           CompOp compOp, const void *value) {
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
        const char *u = pData, *v = (char *)value;
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


void print(const void *value, AttrType attrType, int attrLength) {
    switch (attrType) {

    case INT:
        std::cout << *(int *)value;
        break;
    case FLOAT:
        std::cout << *(float *)value;
        break;
    case STRING: {
        std::string str = std::string((char *)value, attrLength);
        int p = str.find_last_not_of(' ');
        str = str.substr(0, p + 1);
        std::cout << str;
        break;
    }
    case VARSTRING:
        std::cout << (char *)value;
        break;
    case DATE:
        unsigned short y = *(unsigned short *)value;
        unsigned char m = *(unsigned char *)((char *)value + 2);
        unsigned char d = *(unsigned char *)((char *)value + 3);
        std::cout << y << "-" << m << "-" << d;
        break;

    }
    std::cout.flush();
}
