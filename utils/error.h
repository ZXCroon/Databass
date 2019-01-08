#ifndef ERROR_H
#define ERROR_H

#include "defs.h"
#include "utils.h"


class Error {
public:
    static void typeError(AttrType expected, AttrType actual);
    static void condTypeError();
    static void nullError(const char *attrName);
    static void primaryNotUniqueError(const char *attrName);
    static void referenceError(const char *attrName, const char *refRelName, const char *refAttrName);
    static void invalidDateError();
    static void notOpenDatabaseError();
    static bool err;
private:
    static void head();
};


#endif 
