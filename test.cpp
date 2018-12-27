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

    AttrInfo attr1 = {"name", STRING, 20, NULL, NULL, false, false, false};
    AttrInfo attr2 = {"age", INT, 4, NULL, NULL, false, false, false};
    AttrInfo attrs[2] = {attr1, attr2};
    smm.createTable("student", 2, attrs);

    AttrInfo attr3 = {"name", STRING, 20, NULL, NULL, false, false, false};
    AttrInfo attr4 = {"id", INT, 4, NULL, NULL, false, false, false};
    AttrInfo attr5 = {"teacher", STRING, 20, NULL, NULL, false, false, false};
    AttrInfo attrs2[3] = {attr3, attr4, attr5};
    smm.createTable("class", 3, attrs2);

    AttrInfo attr6 = {"id", INT, 4, NULL, NULL, false, false, false};
    AttrInfo attr7 = {"number", INT, 4, NULL, NULL, false, false, false};
    AttrInfo attrs3[2] = {attr6, attr7};
    smm.createTable("sel", 2, attrs3);

    smm.showDbs();
    smm.dropTable("student");
    smm.showDb("trydb");

    smm.createTable("student", 2, attrs);

    // INSERT
    std::cout << "**** INSERT ****" << std::endl;
    char name1[] = "Compiler";
    int id1 = 2223425;
    char teacher1[] = "wangshengyuan";
    Value v11 = {STRING, name1};
    Value v12 = {INT, &id1};
    Value v13 = {STRING, &teacher1};
    Value values1[] = {v11, v12, v13};
    qlm.insert("class", 3, values1);
    char name2[] = "Image";
    int id2 = 234;
    char teacher2[] = "cuipeng";
    Value v21 = {STRING, name2};
    Value v22 = {INT, &id2};
    Value v23 = {STRING, &teacher2};
    Value values2[] = {v21, v22, v23};
    qlm.insert("class", 3, values2);
    int id3 = 5342;
    char teacher3[] = "liuweidong";
    Value v31 = {STRING, NULL};
    Value v32 = {INT, &id3};
    Value v33 = {STRING, &teacher3};
    Value values3[] = {v31, v32, v33};
    qlm.insert("class", 3, values3);

    int id4 = 3452;
    int number1 = 70, number2 = 100;
    Value v41 = {INT, &id1};
    Value v42 = {INT, &number1};
    Value values4[] = {v41, v42};
    qlm.insert("sel", 2, values4);
    Value v51 = {INT, &id4};
    Value v52 = {INT, &number2};
    Value values5[] = {v51, v52};
    qlm.insert("sel", 2, values5);

    // SELECT
    std::cout << "**** SELECT1 ****" << std::endl;
    RelAttr ra1 = {NULL, "id"}, ra2 = {"class", "name"}, ra3 = {"sel", "number"};
    RelAttr ras[] = {ra1, ra2};
    RelAttr cra1 = {NULL, "id"};
    int cid = 234;
    Value cv1 = {INT, &cid};
    RelAttr cra2 = {"class", "name"};
    // char cname[] = "Compiler        ";
    Value cv2 = {STRING, NULL};
    Condition cond1 = {cra1, GE_OP, 0, cra1, cv1};
    Condition cond2 = {cra2, NOTNULL_OP, 0, cra2, cv2};
    Condition conds[] = {cond1, cond2};
    qlm.select(2, ras, "class", NULL, NO_JOIN, 2, conds);
    std::cout << "**** SELECT2 ****" << std::endl;
    RelAttr ras_[] = {ra1, ra2, ra3};
    qlm.select(3, ras_, "sel", "class", FULL_JOIN, 0, NULL);

    smm.showTables();
    smm.closeDb();
    smm.showDbs();
}


void test2() {
    FileManager fm("test_dbfiles");
    BufPageManager bpm(&fm);
    RM_Manager rmm(&bpm);
    IX_Manager ixm(bpm);
    SM_Manager smm(&ixm, &rmm);
    QL_Manager qlm(&smm, &ixm, &rmm);

    smm.createDb("trydb2");
    smm.openDb("trydb2");
    
    AttrInfo attr1 = {"name", STRING, 20, {}, {}, false, false, false};
    AttrInfo attr2 = {"age", INT, 4, {}, {}, true, false, false};
    AttrInfo attr3 = {"id", INT, 4, {}, {}, true, true, false};
    AttrInfo attr4 = {"sex", STRING, 1, {}, {}, false, false, false};
    AttrInfo attr5 = {"date of birth", DATE, 4, {}, {}, false, false, false};
    AttrInfo attrs[] = {attr1, attr2, attr3, attr4, attr5};
    smm.createTable("people", 5, attrs);

    smm.showDb("trydb2");

    {
        char name[] = "Nima Wang";
        int age = 23;
        int id = 234112423;
        char sex[] = "M";
        char date_of_birth[] = "1995-01-28";
        Value v1 = {VARSTRING, name};
        Value v2 = {INT, &age};
        Value v3 = {INT, &id};
        Value v4 = {STRING, sex};
        Value v5 = {STRING, date_of_birth};
        Value values[] = {v1, v2, v3, v4, v5};
        qlm.insert("people", 5, values);
    }
    {
        char name[] = "Qiqi Wang";
        int age = 20;
        int id = 234112423;
        char sex[] = "F";
        char date_of_birth[] = "   1998- 02  - 22  ";
        Value v1 = {STRING, name};
        Value v2 = {INT, &age};
        Value v3 = {INT, &id};
        Value v4 = {STRING, sex};
        Value v5 = {STRING, date_of_birth};
        Value values[] = {v1, v2, v3, v4, v5};
        qlm.insert("people", 5, values);
    }
    {
        char name[] = "Qiqi Wang";
        int age = 20;
        int id = 2342423;
        char sex[] = "F";
        char date_of_birth[] = "   1998- 02  - 22  ";
        Value v1 = {STRING, name};
        Value v2 = {INT, &age};
        Value v3 = {INT, &id};
        Value v4 = {STRING, sex};
        Value v5 = {STRING, date_of_birth};
        Value values[] = {v1, v2, v3, v4, v5};
        qlm.insert("people", 5, values);
    }
    {
        char name[] = "Zixi Cai";
        // int age = 22;
        int id = 1523412;
        char sex[] = "M";
        char date_of_birth[] = "1996/10/24";
        Value v1 = {STRING, name};
        Value v2 = {INT, NULL};
        Value v3 = {INT, &id};
        Value v4 = {STRING, sex};
        Value v5 = {STRING, date_of_birth};
        Value values[] = {v1, v2, v3, v4, v5};
        qlm.insert("people", 5, values);
    }
    {
        char name[] = "Zemin Jiang";
        char age[] = "+1s";
        int id = 534164;
        char sex[] = "M";
        char date_of_birth[] = "1926/ 8/17 ";
        Value v1 = {STRING, name};
        Value v2 = {VARSTRING, age};
        Value v3 = {INT, &id};
        Value v4 = {STRING, sex};
        Value v5 = {STRING, date_of_birth};
        Value values[] = {v1, v2, v3, v4, v5};
        qlm.insert("people", 5, values);
    }
    {
        char name[] = "Zemin Jiang";
        int age = 92;
        int id = 534164;
        char sex[] = "M";
        char date_of_birth[] = "1926/ 8/17 ";
        Value v1 = {STRING, name};
        Value v2 = {INT, &age};
        Value v3 = {INT, &id};
        Value v4 = {STRING, sex};
        Value v5 = {STRING, date_of_birth};
        Value values[] = {v1, v2, v3, v4, v5};
        qlm.insert("people", 5, values);
    }
    {
        RelAttr ra = {NULL, "id"};
        RelAttr rra = {"people", "age"};
        RelAttr cra = {NULL, "sex"};
        char sex[] = "F";
        Value v = {STRING, sex};
        Condition cond = {cra, EQ_OP, false, cra, v};
        Condition conds[] = {cond};
        qlm.update("people", ra, false, rra, v, 1, conds);
    }
    {
        RelAttr ra1 = {NULL, "id"};
        RelAttr ra2 = {"people", "name"};
        RelAttr ra3 = {"people", "age"};
        RelAttr ra4 = {"people", "date of birth"};
        RelAttr ras[] = {ra1, ra2, ra3, ra4};
        qlm.select(4, ras, "people", NULL, NO_JOIN, 0, NULL);
    }

    smm.closeDb();
    smm.dropDb("trydb");
    smm.showDbs();
}


int main() {
    std::cout << std::endl;
    std::cout << "################" << std::endl;
    std::cout << "#    Test 1    #" << std::endl;
    std::cout << "################" << std::endl;
    test1();
    std::cout << std::endl;

    std::cout << std::endl;
    std::cout << "################" << std::endl;
    std::cout << "#    Test 2    #" << std::endl;
    std::cout << "################" << std::endl;
    test2();
    std::cout << std::endl;

    return 0;
}
