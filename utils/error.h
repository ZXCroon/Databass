#ifndef ERROR_H
#define ERROR_H

#include "defs.h"
#include "utils.h"


class Error {
public:
    static void typeError(AttrType expected, AttrType actual);
    static void nullError(const char *attrName);
    static void primaryNotUniqueError(const char *attrName);
    static void invalidDateError();
private:
    static void head();
};


#endif 