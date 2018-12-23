#ifndef SEMVALUE_H
#define SEMVALUE_H

#include "../utils/defs.h"
#include <cstring>

class SemValue {

public:
    int code;
    CompOp compOp;
    char *id; /* name of identifier */
    static int keyword(int code);
    static CompOp opt(char *code);
};

#endif