#include "error.h"
#include <iostream>


void Error::typeError(AttrType expected, AttrType actual) {
    head();
    std::cout << "Type mismatch: expected ";
    printAttrType(expected);
    std::cout << ", got ";
    printAttrType(actual);
    std::cout << "." << std::endl;
}


void Error::nullError(const char *attrName) {
    head();
    std::cout << "Key \"";
    print(attrName, VARSTRING, MAXNAME + 1);
    std::cout << "\" cannot be NULL." << std::endl;
}


void Error::primaryNotUniqueError(const char *attrName) {
    head();
    std::cout << "Primary key \"";
    print(attrName, VARSTRING, MAXNAME + 1);
    std::cout << "\" must be unique." << std::endl;
}


void Error::invalidDateError() {
    head();
    std::cout << "Invalid date." << std::endl;
}


void Error::head() {
    std::cout << "\033[1;31m[ERROR] \033[0m";
}
