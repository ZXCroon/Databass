#include <cstring>
#include <iostream>
#include <iomanip>
#include <sstream>
#include "utils.h"
#include "defs.h"


bool isNull(const void *value, int attrLength) {
    return memcmp(value, ((const char *)value) + 1, attrLength - 1) == 0 && ((const unsigned char *)value)[0] == NULL_BYTE;
}


bool checkLike(std::string str, std::string pattern) {
    int m = str.length(), n = pattern.length();
    bool *match = new bool[m + 1], *sumMatch = new bool[m + 1];
    bool *lastMatch = new bool[m + 1], *lastSumMatch = new bool[m + 1];
    memset(lastMatch, 0, m + 1);
    lastMatch[0] = true;
    memset(lastSumMatch, -1, m + 1);
    for (int i = 0; i < n; ++i) {
        if (pattern[i] == '%') {
            match[0] = lastSumMatch[0];
        } else {
            match[0] = false;
        }
        sumMatch[0] = match[0];

        for (int j = 0; j < m; ++j) {
            if (str[j] == pattern[i] || pattern[i] == '_') {
                match[j + 1] = lastMatch[j];
            } else if (pattern[i] == '%') {
                match[j + 1] = lastSumMatch[j + 1];
            } else {
                match[j + 1] = false;
            }
            sumMatch[j + 1] = sumMatch[j] || match[j + 1];
        }

        bool *t = match;
        match = lastMatch;
        lastMatch = t;
        t = sumMatch;
        sumMatch = lastSumMatch;
        lastSumMatch = t;
    }

    bool ans = lastMatch[m];
    delete[] match;
    delete[] sumMatch;
    delete[] lastMatch;
    delete[] lastSumMatch;
    return ans;
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


bool convertToDate(char *value) {
    int len = strlen(value);
    int yyyy = -1, mm = -1, dd = -1;
    int phase = 0;
    char delim = ' ';
    for (int i = 0; i < len; ++i) {
        if (value[i] == ' ') {
            continue;
        } else if (phase == 3) {
            return false;
        }
        if (value[i] >= '0' && value[i] <= '9') {
            int v = value[i] - '0';
            for (++i; i < len && value[i] >= '0' && value[i] <= '9'; ++i) {
                v = v * 10 + (value[i] - '0');
                if (v > 10000) {
                    return false;
                }
            }
            if (phase < 2) {
                for (; value[i] == ' '; ++i);
                if (value[i] != '-' && value[i] != '/' || delim != ' ' && value[i] != delim) {
                    return false;
                }
                delim = value[i];
            } else {
                --i;
            }
            if (phase == 0) {
                yyyy = v;
            } else if (phase == 1) {
                mm = v;
            } else if (phase == 2) {
                dd = v;
            }
            ++phase;
        }
    }
    if (yyyy < 1 || yyyy > 9999 || mm < 1 || mm > 12 || dd < 1 || dd > 31) {
        return false;
    }
    int maxDay[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if (yyyy % 400 == 0 || (yyyy % 100 != 0 && yyyy % 4 == 0)) {
        maxDay[1] = 29;
    }
    if (dd > maxDay[mm - 1]) {
        return false;
    }
    *((unsigned short *)value) = yyyy;
    *((unsigned char *)(value) + 2) = mm;
    *((unsigned char *)(value) + 3) = dd;
    return true;
}


bool validate(const char *pData1, const char *pData2, AttrType attrType1, AttrType attrType2,
              int attrLength1, int attrLength2, CompOp compOp, bool strict) {
    if (compOp == ISNULL_OP || compOp == NOTNULL_OP) {
        return validate(pData1, attrType1, attrLength1, compOp, NULL);
    }
    if (attrType1 == attrType2) {
        return validate(pData1, attrType1, attrLength1, compOp, pData2);
    }
    if (attrType1 == INT && attrType2 == FLOAT) {
        char *pDatat = new char[attrLength2];
        *(float *)pDatat = (float)(*(int *)pData1);
        bool ans = validate(pDatat, FLOAT, attrLength2, compOp, pData2);
        delete[] pDatat;
        return ans;
    }
    if (attrType1 == FLOAT && attrType2 == INT) {
        char *pDatat = new char[attrLength1];
        *(float *)pDatat = (float)(*(int *)pData2);
        bool ans = validate(pData1, FLOAT, attrLength1, compOp, pDatat);
        delete[] pDatat;
        return ans;
    }
    if (strict) {
        Error::condTypeError();
        return false;
    }
    if (attrType1 == DATE && attrType2 == VARSTRING) {
        char *pDatat = new char[attrLength2 + 1];
        memcpy(pDatat, pData2, attrLength2 + 1);
        if (!convertToDate(pDatat)) {
            Error::invalidDateError();
            delete[] pDatat;
            return false;
        }
        bool ans = validate(pData1, DATE, attrLength1, compOp, pDatat);
        delete[] pDatat;
        return ans;
    }
    if (attrType1 == VARSTRING && attrType2 == DATE) {
        char *pDatat = new char[attrLength1 + 1];
        memcpy(pDatat, pData1, attrLength1 + 1);
        if (!convertToDate(pDatat)) {
            Error::invalidDateError();
            delete[] pDatat;
            return false;
        }
        bool ans = validate(pDatat, DATE, attrLength2, compOp, pData2);
        delete[] pDatat;
        return ans;
    }
    if (attrType1 == VARSTRING && attrType2 == STRING) {
        std::string str1(pData1);
        std::string str2(pData2, attrLength2);
        str1.erase(str1.find_last_not_of(" ") + 1);
        str2.erase(str2.find_last_not_of(" ") + 1);
        return validate(str1.c_str(), VARSTRING, 0, compOp, str2.c_str());
    }
    if (attrType1 == STRING && attrType2 == VARSTRING) {
        std::string str1(pData1, attrLength1);
        std::string str2(pData2);
        str1.erase(str1.find_last_not_of(" ") + 1);
        str2.erase(str2.find_last_not_of(" ") + 1);
        return validate(str1.c_str(), VARSTRING, 0, compOp, str2.c_str());
    }
    Error::condTypeError();
    return false;
}


bool validate(const char *pData, AttrType attrType, int attrLength,
              CompOp compOp, const void *value) {
    if (compOp == NO_OP) {
        return true;
    }
    if (pData == NULL) {
        return false;
    }

    if (compOp == ISNULL_OP) {
        return isNull(pData, attrLength);
    }
    if (compOp == NOTNULL_OP) {
        return !isNull(pData, attrLength);
    }

    if (compOp == LIKE_OP) {
        if (isNull(pData, attrLength) || isNull(value, attrLength)) {
            return false;
        }
        if (attrType == VARSTRING) {
            return checkLike(std::string(pData), std::string((char *)value));
        } else if (attrType == STRING) {
            std::string str(pData, attrLength), pattern((char *)value, attrLength);
            str.erase(str.find_last_not_of(" ") + 1);
            pattern.erase(pattern.find_last_not_of(" ") + 1);
            return checkLike(str, pattern);
        } else {
            return false;
        }
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
    if (isNull(value, attrLength)) {
        std::cout << "NULL";
        std::cout.flush();
        return;
    }

    switch (attrType) {

    case INT:
        std::cout << *(int *)value;
        break;
    case FLOAT:
        std::cout << std::fixed << std::setprecision(2) << *(float *)value;
        break;
    case STRING: {
        std::string str = std::string((char *)value, attrLength);
        str.erase(str.find_last_not_of(" ") + 1);
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
        std::cout << y << "-" << int(m) << "-" << int(d);
        break;

    }
    std::cout.flush();
}


void printAttrType(AttrType attrType, int attrLength) {
    std::string str;
    std::stringstream ss;
    switch (attrType) {

    case INT:
        str = "INT";
        break;
    case FLOAT:
        str = "FLOAT";
        break;
    case STRING:
        if (attrLength == -1) {
            str = "CHAR";
        } else {
            ss << "CHAR(";
            ss << attrLength;
            ss << ")";
            str = ss.str();
        }
        break;
    case VARSTRING:
        if (attrLength == -1) {
            str = "VARCHAR";
        } else {
            ss << "VARCHAR(";
            ss << attrLength - 1;
            ss << ")";
            str = ss.str();
        }
        break;
    case DATE:
        str = "DATE";
        break;
    }

    std::cout << str;
    std::cout.flush();
}
