#include "rm.h"
#include <bitset>


const int RM_Manager::MAX_POOL_SIZE = 64;


RM_Manager::RM_Manager(BufPageManager *bpm) : bpm(bpm) {}


RM_Manager::~RM_Manager() {
    std::cout << "DEBUG: RM_Manager flushed" << std::endl;
    clearPool();
}


RC RM_Manager::createFile(const char *fileName, int recordSize) {
    if (recordSize < RM_RECORD_MIN_SIZE || recordSize > RM_RECORD_MAX_SIZE || recordSize % 4) {
        return RM_MANAGER_RECORDSIZEINVALID;
    }
    if (!bpm->fileManager->createFile(fileName)) {
        return RM_MANAGER_CREATEFAILED;
    }
    int fileId;
    if (!bpm->fileManager->openFile(fileName, fileId)) {
        return RM_MANAGER_CREATEFAILED;
    }

    FileHeaderPage hp;
    hp.recordSize = recordSize;
    hp.firstFree = hp.lastFree = NO_PAGE;
    hp.availPageCnt = 0;
    char page[PAGE_SIZE];
    memcpy(page, &hp, sizeof(hp));
    if (bpm->fileManager->writePage(fileId, 0, (BufType)(page), 0) != 0 or
        bpm->fileManager->closeFile(fileId) != 0) {
        return RM_MANAGER_CREATEFAILED;
    }

    return 0;
}


bool RM_Manager::createDir(const char *dirName) {
    return bpm->fileManager->createDir(dirName);
}


bool RM_Manager::deleteFile(const char *fileName) {
    return bpm->fileManager->deleteFile(fileName);
}


bool RM_Manager::deleteDir(const char *dirName) {
    return bpm->fileManager->deleteDir(dirName);
}


bool RM_Manager::openFile(const char *fileName, RM_FileHandle *&fileHandle) {
    std::string name(fileName);
    auto it = handlePool.find(name);
    if (it != handlePool.end()) {
        fileHandle = it->second;
        return true;
    }
    int fileId;
    if (!bpm->fileManager->openFile(fileName, fileId)) {
        return false;
    }
    fileHandle = new RM_FileHandle(bpm, fileId);
    if (handlePool.size() == MAX_POOL_SIZE) {
        clearPool();
    }
    handlePool[name] = fileHandle;
    return true;
}


bool RM_Manager::closeFile(RM_FileHandle &fileHandle) {
    // bpm->close();
    // return !bpm->fileManager->closeFile(fileHandle.getFileId());
    return true;
}


std::vector<std::string> RM_Manager::listDir(const char *dirName) {
    return bpm->fileManager->listDir(dirName);
}


void RM_Manager::clearPool() {
    bpm->close();
    for (auto it = handlePool.begin(); it != handlePool.end(); ++it) {
        bpm->fileManager->closeFile(it->second->getFileId());
    }
    handlePool.clear();
}
