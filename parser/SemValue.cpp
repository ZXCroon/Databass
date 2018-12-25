#include "SemValue.h"
#define YYSTYPE SemValue

extern YYSTYPE yylval;

SemValue::SemValue() {}

int SemValue::keyword(int code) {
    yylval = SemValue();
    yylval.code = code;
    return code;
}