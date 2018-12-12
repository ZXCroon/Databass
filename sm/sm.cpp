#include <cstring>
#include "sm.h"


SM_Manager::SM_Manager(const IX_Manager *&ixm, const RM_Manager *&rmm) : ixm(ixm), rmm(rmm) {}


SM_Manager::~SM_Manager() {}


bool SM_Manager::createDb(const char *dbName) {
    if (!rmm->createDir(dbName)) {
        return false;
    }
    RC rc;
    rc = rmm->createFile(getPath(dbName, "relcat"), sizeof(RelcatLayout));
    if (rc) {
        return false;
    }
    rc = rmm->createFile(getPath(dbName, "attrcat"), sizeof(AttrcatLayout));
    if (rc) {
        return false;
    }
    return true;
}


bool SM_Manager::destroyDb(const char *dbName) {
    return rmm->deleteDir(dbName);
}


bool SM_Manager::openDb(const char *dbName) {
    strcpy(dbNameBuf, dbName);
    return rmm->openFile(getPath(dbName, "relcat"), relcatHandle) && rmm->openFile(getPath(dbName, "attrcat"), attrcatHandle);
}


bool SM_Manager::closeDb() {
    return rmm->closeFile(*relcatHandle) && rmm->closeFile(*attrcatHandle)
}


bool SM_Manager::createTable(const char *relName, int attrCount, AttrInfo *attributes) {
    RID rid;
    if (getRelcatRid(relName, rid)) {
        return false;
    }

    RelcatLayout relcat;
    strcpy(relcat.relName, relName);
    padName(relcat.relName);
    relcat.tupleLength = 0;
    relcat.attrCount = attrCount;
    relcat.indexCount = 0;
    AttrInfo *attr = attributes;
    for (int i = 0; i < attrCount; ++i, ++attr) {
        AttrcatLayout attrcat;
        strcpy(attrcat.relName, relName);
        strcpy(attrcat.attrName, attr->attrName);
        padName(attrcat.relName);
        padName(attrcat.attrName);
        attrcat.offset = relcat.tupleLength;
        attrcat.attrType = attr->attrType;
        attrcat.attrLength = attr->attrLength;
        attrcat.indexNo = -1;

        RID rid;
        if (!attrcatHandle->insertRec((char *)(&attrcat), rid)) {
            return false;
        }

        relcat.tupleLength += attr->attrLength;
    }

    RID rid;
    if (!relcatHandle->insertRec((char *)(&relcat), rid)) {
        return false;
    }

    if (rmm->createFile(getPath(dbName, relName), relcat.tupleLength) != 0) {
        return false;
    }

    return true;
}


bool SM_Manager::dropTable(const char *relName) {
    if (!rmm->deleteFile(getPath(dbName, relName))) {
        return false;
    }

    RID rid;
    if (!getRelcatRid(relName, rid)) {
        return false;
    }
    relcatHandle->deleteRec(rid);

    RID rids[MAXATTRS];
    int ridCount;
    getAttrcatRids(relName, NULL, rids, ridCount);
    for (int i = 0; i < ridCount; ++i) {
        attrcatHandle->deleteRec(rids[i]);
    }

    return true;
}


bool SM_Manager::createIndex(const char *relName, const char *attrName) {
    RID rid;
    if (!getRelcatRid(relName, rid)) {
        return false;
    }
    RM_Record relRec;
    relcatHandle->getRec(rid, relRec);
    RelcatLayout relcat;
    getRelcatFromRid(rid, relcat);

    RID rids[MAXATTRS];
    int ridCount;
    if (!getAttrcatRids(relName, attrName, rids, ridCount)) {
        return false;
    }
    RM_Record attrRec;
    attrcatHandle->getRec(rids[0], attrRec);
    AttrcatLayout attrcat;
    getAttrcatFromRid(rids[0], attrcat);
    if (attrcat.indexNo != -1) {
        return false;
    }

    attrcat.indexNo = relcat.nextIndex;
    ++relcat.nextIndex;

    memcpy(relRec.getData(), (char *)relcat, sizeof(RelcatLayout));
    memcpy(attrRec.getData(), (char *)attrcat, sizeof(AttrcatLayout));

    relcatHandle->updateRec(relRec);
    attrcatHandle->updateRec(attrRec);

    if (ixm->createIndex(getPath(dbName, relName), attrcat.indexNo, attrcat.attrType, attrcat.attrLength) != 0) {
        return false;
    }
    return true;
}


bool SM_Manager::dropIndex(const char *relName, const char *attrName) {
    RID rids[MAXATTRS];
    int ridCount;
    if (!getAttrcatRids(relName, attrName, rids, ridCount)) {
        return false;
    }
    RM_Record attrRec;
    attrcatHandle->getRec(rids[0], attrRec);
    AttrcatLayout attrcat;
    getAttrcatFromRid(rids[0], attrcat);
    if (attrcat.indexNo == -1) {
        return false;
    }

    attrcat.indexNo = -1;
    memcpy(attrRec.getData(), (char *)attrcat, sizeof(AttrcatLayout));
    attrcatHandle->updateRec(attrRec);

    if (ixm->deleteIndex(getPath(dbName, relName), attrcat.indexNo)) {
        return false;
    }
    return true;
}


char *SM_Manager::getPath(const char *dbName, const char *relName) {
    strcpy(pathBuf, dbName);
    strcat(pathBuf, "/");
    strcat(pathBuf, relName);
    return pathBuf;
}


void SM_Manager::padName(char name[MAXNAME + 1], char padding) {
    for (int k = strlen(name); k < MAXNAME + 1; ++k) {
        name[k] = padding;
    }
}


bool SM_Manager::getRelcatRid(const char *relName, RID &rid) {
    char relNamePad[MAXNAME + 1];
    strcpy(relNamePad, relName);
    padName(relNamePad);
    RM_FileScan scan;
    scan.openScan(&(*relcatHandle), STRING, MAXNAME + 1, 0, EQ_OP, relNamePad);
    RM_Record rec;
    if (scan.getNextRec(rec) == RM_FILESCAN_NONEXT) {
        return false;
    }
    rid = rec.getRid();
    return true;
}


bool SM_Manager::getAttrcatRids(const char *relName, const char *attrName, RID *rids, int &ridCount) {
    char relNamePad[MAXNAME + 1];
    strcpy(relNamePad, relName);
    padName(relNamePad);
    char attrNamePad[MAXNAME + 1];
    if (attrName) {
        strcpy(attrNamePad, attrName);
        padName(attrNamePad);
    }

    ridCount = 0;
    RM_FileScan scan;
    scan.openScan(&(*attrcatHandle), STRING, MAXNAME + 1, 0, EQ_OP, relNamePad);
    while (scan.getNextRec(rec) == 0) {
        rid = rec.getRid();
        if (!attrName || strcmp(getAttrcatRids(rid).attrName, attrNamePad) == 0) {
            rids[ridCount] = rid;
            ++ridCount;
        }
    }

    return ridCount != 0;
}


void SM_Manager::getRelcatFromRid(const RID &rid, RelcatLayout &relcat) {
    RM_Record rec;
    relcatHandle->getRec(rid, rec);
    memcpy(&relcat, rec.getData, sizeof(RelcatLayout))
}


void SM_Manager::getAttrcatFromRid(const RID &rid, AttrcatLayout &attrcat) {
    RM_Record rec;
    attrcatHandle->getRec(rid, rec);
    memcpy(&attrcat, rec.getData(), sizeof(AttrcatLayout))
}
