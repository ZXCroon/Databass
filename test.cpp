#include <iostream>
#include "fs/bufmanager/BufPageManager.h"
#include "rm/rm.h"
#include "rm/rm_rid.h"
#include "ix/ix.h"
#include "sm/sm.h"
#include "ql/ql.h"
#include "utils/defs.h"
#include "utils/utils.h"


void test1() {
    FileManager fm("test_dbfiles");
    BufPageManager bpm(&fm);
    RM_Manager rmm(&bpm);
    IX_Manager ixm(bpm);
    SM_Manager smm(&ixm, &rmm);
    QL_Manager qlm(&smm, &ixm, &rmm);

    smm.createDb("trydb");
    smm.openDb("trydb");

    AttrInfo attr1 = {"name", STRING, 10};
    AttrInfo attr2 = {"age", INT, 4};
    AttrInfo attrs[2] = {attr1, attr2};
    smm.createTable("student", 2, attrs);

    AttrInfo attr3 = {"name", STRING, 10};
    AttrInfo attr4 = {"id", INT, 4};
    AttrInfo attr5 = {"teacher", STRING, 10};
    AttrInfo attrs2[3] = {attr3, attr4, attr5};
    smm.createTable("class", 3, attrs2);

    smm.showCurrentDb();
    smm.dropTable("student");
    smm.showCurrentDb();

    smm.createTable("student", 2, attrs);

    // INSERT
    std::cout << "**** INSERT ****" << std::endl;
    char name[] = "Compiler";
    int id = 2223425;
    char teacher[] = "wangshengyuan";
    Value v1 = {STRING, name};
    Value v2 = {INT, &id};
    Value v3 = {STRING, teacher};
    Value values[] = {v1, v2, v3};
    qlm.insert("class", 3, values);

    // SELECT
    std::cout << "**** SELECT ****" << std::endl;
    RelAttr ra1 = {NULL, "id"}, ra2 = {"class", "name"};
    RelAttr ras[] = {ra1, ra2};
    qlm.select(2, ras, "class", NULL, NO_JOIN, 0, NULL);

    smm.closeDb();
}


int main() {
    test1();

    return 0;
}
