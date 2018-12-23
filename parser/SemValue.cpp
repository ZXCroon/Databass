#include "SemValue.h"
#define YYSTYPE SemValue

extern YYSTYPE yylval;

int SemValue::keyword(int code) {
    yylval = SemValue();
    yylval.code = code;
    return code;
}

CompOp SemValue::opt(char *code) {
    if (strcmp(code, "=") == 0) {
        return EQ_OP;
    }
    if (strcmp(code, "<") == 0) {
        return LT_OP;
    }
    if (strcmp(code, ">") == 0) {
        return GT_OP;
    }
    if (strcmp(code, "<=") == 0) {
        return LE_OP;
    }
    if (strcmp(code, ">=") == 0) {
        return GE_OP;
    }
    if (strcmp(code, "<>") == 0) {
        return NE_OP;
    }
    return NO_OP;
}