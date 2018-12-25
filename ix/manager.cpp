#include <string>
#include <sstream>
#include "ix.h"


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

    IX_FileHeaderPage hp;
    hp.recordSize = sizeof(int) + sizeof(int) + sizeof(RID) * (4 + 5 + 1 + 1 + 1) + attrLength * 4;
    hp.firstFree = hp.lastFree = NO_PAGE;
    hp.availPageCnt = 0;
    hp.root = RID(-1, -1);
    hp.attrType = attrType;
    hp.attrLength = attrLength;
    char page[PAGE_SIZE];
    memcpy(page, &hp, sizeof(hp));
    if (bpm->fileManager->writePage(fileId, 0, (BufType)(page), 0) != 0 or
        bpm->fileManager->closeFile(fileId) != 0) {
            return IX_MANAGER_CREATEFAILED;
    }

    return 0;
}


bool IX_Manager::deleteIndex(const char *filename, int indexNo) {
    const char *indexFilename = getIndexFilename(filename, indexNo);
    return bpm->fileManager->deleteFile(indexFilename);
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
    std::stringstream ss;
    ss << indexNo;
    temp += '.' + ss.str();
    return temp.c_str();
}
