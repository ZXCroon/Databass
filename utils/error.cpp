#include "error.h"
#include <iostream>


bool Error::err = false;


void Error::typeError(AttrType expected, AttrType actual) {
    head();
    std::cout << "Type mismatch: expected ";
    printAttrType(expected);
    std::cout << ", got ";
    printAttrType(actual);
    std::cout << "." << std::endl;
}


void Error::condTypeError() {
    head();
    std::cout << "Type mismatch in conditions." << std::endl;
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


void Error::referenceError(const char *attrName, const char *refRelName, const char *refAttrName) {
    head();
    std::cout << "Foreign key \"";
    print(attrName, VARSTRING, MAXNAME + 1);
    std::cout << "\" references primary key \"";
    print(refAttrName, VARSTRING, MAXNAME + 1);
    std::cout << "\" in table \"";
    print(refRelName, VARSTRING, MAXNAME + 1);
    std::cout << "\"." << std::endl;
}


void Error::invalidDateError() {
    head();
    std::cout << "Invalid date." << std::endl;
}


void Error::notOpenDatabaseError() {
    head();
    std::cout << "No database is used." << std::endl;
}


void Error::ambiguousError(const char *attrName) {
    head();
    std::cout << "\"" << attrName << "\" is ambiguous." << std::endl;
}


void Error::noTableError(const char *dbName, const char *relName) {
    head();
    std::cout << "No table \"" << relName << "\" in database \"" << dbName << "\"." << std::endl;
}


void Error::noColumnError(const char *relName, const char *attrName) {
    head();
    std::cout << "No column \"" << attrName << "\" in table \"" << relName << "\"." << std::endl;
}


void Error::noColumnError(const char *relName1, const char *relName2, const char *attrName) {
    head();
    std::cout << "No column \"" << attrName << "\" in table \"" << relName1 << "\" and \"" << relName2 << "\"." << std::endl;
}

void Error::head() {
    err = true;
    std::cout << "\033[1;31m[ERROR] \033[0m";
}
