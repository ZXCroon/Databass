#include "rm.h"


RM_Manager::RM_Manager(BufPageManager &bpm) : bpm(bpm) {}


RM_Manager::~RM_Manager() {}


RC RM_Manager::createFile(const char *fileName, int recordSize) {
    if (recordSize < RM_RECORD_MIN_SIZE || recordSize > RM_RECORD_MAX_SIZE || recordSize % 4) {
        return RM_MANAGER_RECORDSIZEINVALID;
    }
    if (!bpm.fileManager->createFile(fileName)) {
        return RM_MANAGER_CREATEFAILED;
    }
    int fileId;
    if (!bpm.fileManager->openFile(fileName, fileId)) {
        return RM_MANAGER_CREATEFAILED;
    }

    FileHeaderPage hp;
    hp.recordSize = recordSize;
    if (bpm.fileManager->writePage(fileId, 0, (BufType)(&hp), 0) != 0 or
        bpm.fileManager->closeFile(fileId) != 0) {
        return RM_MANAGER_CREATEFAILED;
    }

    return 0;
}


RC RM_Manager::openFile(const char *fileName, RM_FileHandle &fileHandle) {
    int fileId;
    if (!bpm.fileManager->openFile(fileName, fileId)) {
        return RM_MANAGER_OPENFAILED;
    }
    fileHandle = RM_FileHandle(fileId);
    return 0;
}

RC RM_Manager::closeFile(RM_FileHandle &fileHandle) {
    bpm.close();
    if (bpm.fileManager->closeFile(fileHandle.fileId)) {
        return RM_MANAGER_CLOSEFAILED;
    } else {
        reutn 0;
    }
}
