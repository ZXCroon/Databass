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
    AttrInfo attr1 = {"name", STRING, 10};
    AttrInfo attr2 = {"age", INT, 4};
    AttrInfo attrs[2] = {attr1, attr2};
    smm.openDb("trydb");
    smm.createTable("student", 2, attrs);
    smm.showCurrentDb();
    smm.closeDb();
}


int main() {
    test1();

    return 0;
}
