#include "ix.h"

#include <string.h>


IX_Manager::IX_Manager(BufPageManager &bpm) : bpm(&bpm) {}


IX_Manager::~IX_Manager() {}


RC IX_Manager::createIndex(const char *filename, int indexNo, AttrType attrType, int attrLength) {
    const char *indexFilename = getIndexFilename(filename, indexNo);
    if (!bpm->fileManager->createFile(indexFilename)) {
        return IX_MANAGER_CREATEFAILED;
    }
    int fileId;
    if (!bpm->fileManager->openFile(indexFilename, fileId)) {
        return IX_MANAGER_CREATEFAILED;
    }

    FileHeaderPage hp;
    /* to be updated */
    hp.recordSize = attrLength;
    hp.firstFree = hp.lastFree = NO_PAGE;
    hp.availPageCnt = 0;
    if (bpm->fileManager->writePage(fileId, 0, (BufType)(&hp), 0) != 0 or
        bpm->fileManager->closeFile(fileId) != 0) {
            return IX_MANAGER_CREATEFAILED;
    }

    return 0;
}


bool IX_Manager::openIndex(const char *filename, int indexNo, IX_IndexHandle *&indexHandle) {
    const char *indexFilename = getIndexFilename(filename, indexNo);
    int fileId;
    if (!bpm->fileManager->openFile(indexFilename, fileId)) {
        return false;
    }
    indexHandle = new IX_IndexHandle(bpm, fileId);
    return true;
}


bool IX_Manager::closeIndex(IX_IndexHandle &indexHandle) {
    bpm->close();
    return !bpm->fileManager->closeFile(indexHandle.getFileId());
}


const char* IX_Manager::getIndexFilename(const char* filename, int indexNo) {
    string temp(filename);
    temp += '.' + to_string(indexNo);
    return temp.c_str();
}