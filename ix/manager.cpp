#include <string>
#include "ix.h"
#include <cstdio>


const int IX_Manager::MAX_POOL_SIZE = 64;


IX_Manager::IX_Manager(BufPageManager &bpm) : bpm(&bpm) {}


IX_Manager::~IX_Manager() {
    std::cout << "DEBUG: IX_Manager flushed" << std::endl;
    clearPool();
}


RC IX_Manager::createIndex(const char *filename, int indexNo, AttrType attrType, int attrLength) {
    char *indexFilename = getIndexFilename(filename, indexNo);
    if (!bpm->fileManager->createFile(indexFilename)) {
        return IX_MANAGER_CREATEFAILED;
    }
    int fileId;
    if (!bpm->fileManager->openFile(indexFilename, fileId)) {
        return IX_MANAGER_CREATEFAILED;
    }

    IX_FileHeaderPage hp;
    hp.recordSize = sizeof(int) + sizeof(int) + sizeof(RID) * (4 + 5 + 1 + 1 + 1) + attrLength * 4;
    hp.recordSize = max(RM_RECORD_MIN_SIZE, (hp.recordSize + 3) / 4 * 4);
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
    char *indexFilename = getIndexFilename(filename, indexNo);
    return bpm->fileManager->deleteFile(indexFilename);
}


bool IX_Manager::openIndex(const char *filename, int indexNo, IX_IndexHandle *&indexHandle) {
    char *indexFilename = getIndexFilename(filename, indexNo);
    std::string name(indexFilename);
    auto it = handlePool.find(name);
    if (it != handlePool.end()) {
        indexHandle = it->second;
        return true;
    }
    int fileId;
    if (!bpm->fileManager->openFile(indexFilename, fileId)) {
        return false;
    }
    indexHandle = new IX_IndexHandle(bpm, fileId);
    if (handlePool.size() == MAX_POOL_SIZE) {
        clearPool();
    }
    handlePool[name] = indexHandle;
    return true;
}


bool IX_Manager::closeIndex(IX_IndexHandle &indexHandle) {
    // bpm->close();
    // return !bpm->fileManager->closeFile(indexHandle.getFileId());
    return true;
}


char* IX_Manager::getIndexFilename(const char* filename, int indexNo) {
    char *indexNum = new char[256];
    sprintf(indexNum, "%d", indexNo);
    char *ans = new char[strlen(filename) + strlen(indexNum) + 1];
    strcpy(ans, filename);
    strcat(ans, ".");
    strcat(ans, indexNum);
    return ans;
}


void IX_Manager::clearPool() {
    bpm->close();
    for (auto it = handlePool.begin(); it != handlePool.end(); ++it) {
        bpm->fileManager->closeFile(it->second->getFileId());
    }
    handlePool.clear();
}
