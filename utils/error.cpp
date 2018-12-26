#include "error.h"
#include <iostream>


void Error::typeError(AttrType expected, AttrType actual) {
    std::cout << "[ERROR] ";
    std::cout << "Type mismatch: expected ";
    printAttrType(expected);
    std::cout << ", got ";
    printAttrType(actual);
    std::cout << "." << std::endl;
}


void Error::primaryNullError(const char *attrName) {
    std::cout << "[ERROR] ";
    std::cout << "Primary key ";
    print(attrName, VARSTRING, MAXNAME + 1);
    std::cout << " cannot be NULL." << std::endl;
}


void Error::primaryNotUniqueError(const char *attrName) {
    std::cout << "[ERROR] ";
    std::cout << "Primary key ";
    print(attrName, VARSTRING, MAXNAME + 1);
    std::cout << " must be unique." << std::endl;
}


void Error::invalidDateError() {
    std::cout << "[ERROR] ";
    std::cout << "Invalid date." << std::endl;
}
