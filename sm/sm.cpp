#include <cstring>
#include "sm.h"


SM_Manager::SM_Manager(IX_Manager *ixm, RM_Manager *rmm) : ixm(ixm), rmm(rmm), relcatHandle(NULL), attrcatHandle(NULL) {}


SM_Manager::~SM_Manager() {
    closeDb();
}


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


bool SM_Manager::dropDb(const char *dbName) {
    return rmm->deleteDir(dbName);
}


bool SM_Manager::openDb(const char *dbName) {
    if (relcatHandle != NULL || attrcatHandle != NULL) {
        if (!closeDb()) {
            return false;
        }
    }
    strcpy(this->dbName, dbName);
    return rmm->openFile(getPath(dbName, "relcat"), relcatHandle) && rmm->openFile(getPath(dbName, "attrcat"), attrcatHandle);
}


bool SM_Manager::showDb(const char *dbName) {
    // TODO
}


bool SM_Manager::closeDb() {
    if (relcatHandle == NULL || attrcatHandle == NULL) {
        return false;
    }
    if (!(rmm->closeFile(*relcatHandle) && rmm->closeFile(*attrcatHandle))) {
        return false;
    }
    relcatHandle = attrcatHandle = NULL;
    return true;
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
    relcat.nextIndex = 0;
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
        attrcatHandle->insertRec((char *)(&attrcat), rid);

        relcat.tupleLength += attr->attrLength;
    }

    relcatHandle->insertRec((char *)(&relcat), rid);

    rmm->createFile(getPath(dbName, relName), relcat.tupleLength);

    return true;
}


bool SM_Manager::showTable(const char *relName) {
    // TODO
}


bool SM_Manager::dropTable(const char *relName) {
    RID rid;
    if (!getRelcatRid(relName, rid)) {
        return false;
    }
    relcatHandle->deleteRec(rid);
    rmm->deleteFile(getPath(dbName, relName));

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

    memcpy(relRec.getData(), (char *)&relcat, sizeof(RelcatLayout));
    memcpy(attrRec.getData(), (char *)&attrcat, sizeof(AttrcatLayout));

    relcatHandle->updateRec(relRec);
    attrcatHandle->updateRec(attrRec);

    ixm->createIndex(getPath(dbName, relName), attrcat.indexNo, attrcat.attrType, attrcat.attrLength);
    IX_IndexHandle *ixHandle;
    ixm->openIndex(getPath(dbName, relName), attrcat.indexNo, ixHandle);
    RM_FileHandle *handle;
    rmm->openFile(getPath(dbName, relName), handle);
    RM_FileScan scan;
    scan.openScan(*handle, 0, 0, 0, NO_OP, NULL);
    RM_Record rec;
    RC rc;
    while (true) {
        rc = scan.getNextRec(rec);
        if (rc == RM_FILESCAN_NONEXT) {
            break;
        }
        RID rid;
        ixHandle->insertEntry(rec.getData() + attrcat.offset, rid);
    }
    ixm->closeIndex(*ixHandle);
    rmm->closeFile(*handle);
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
    memcpy(attrRec.getData(), (char *)&attrcat, sizeof(AttrcatLayout));
    attrcatHandle->updateRec(attrRec);

    ixm->deleteIndex(getPath(dbName, relName), attrcat.indexNo);
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
    scan.openScan(*relcatHandle, STRING, MAXNAME + 1, 0, EQ_OP, relNamePad);
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
    RM_Record rec;
    RID rid;
    scan.openScan(*attrcatHandle, STRING, MAXNAME + 1, 0, EQ_OP, relNamePad);
    while (scan.getNextRec(rec) == 0) {
        rid = rec.getRid();
        AttrcatLayout attrcat;
        getAttrcatFromRid(rid, attrcat);
        if (!attrName || strcmp(attrcat.attrName, attrNamePad) == 0) {
            rids[ridCount] = rid;
            ++ridCount;
        }
    }

    return ridCount != 0;
}


void SM_Manager::getRelcatFromRid(const RID &rid, RelcatLayout &relcat) {
    RM_Record rec;
    relcatHandle->getRec(rid, rec);
    memcpy(&relcat, rec.getData(), sizeof(RelcatLayout));
}


void SM_Manager::getAttrcatFromRid(const RID &rid, AttrcatLayout &attrcat) {
    RM_Record rec;
    attrcatHandle->getRec(rid, rec);
    memcpy(&attrcat, rec.getData(), sizeof(AttrcatLayout));
}
