#ifndef FILE_MANAGER
#define FILE_MANAGER
#include <cstring>
#include <vector>
#include <stdio.h>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <dirent.h>
#include "../utils/MyLinkList.h"
#include "../../utils/utils.h"
#include "../utils/pagedef.h"
using namespace std;
class FileManager {
private:
	//FileTable* ftable;
	int fd[MAX_FILE_NUM];
    char baseDir[500];
    char pathBuf[500];
    BitMap fm;
    char *fullName(const char *name) {
        strcpy(pathBuf, baseDir);
        strcat(pathBuf, name);
        return pathBuf;
    }
	int _createFile(const char* name) {
		FILE* f = fopen(fullName(name), "a+");
		if (f == NULL) {
			cout << "fail" << endl;
			return -1;
		}
		fclose(f);
		return 0;
	}
	int _openFile(const char* name, int fileID) {
		int f = open(fullName(name), O_RDWR);
		if (f == -1) {
			return -1;
		}
		fd[fileID] = f;
		return 0;
	}
public:
	/*
	 * FilManager构造函数
	 */
	FileManager(const char* baseDir) {
        initBitMap(fm);
        strcpy(this->baseDir, baseDir);
        int len = strlen(baseDir);
        if (baseDir[len - 1] != '/') {
            this->baseDir[len] = '/';
            this->baseDir[len + 1] = '\0';
        }
	}
	/*
	 * @函数名writePage
	 * @参数fileID:文件id，用于区别已经打开的文件
	 * @参数pageID:文件的页号
	 * @参数buf:存储信息的缓存(4字节无符号整数数组)
	 * @参数off:偏移量
	 * 功能:将buf+off开始的2048个四字节整数(8kb信息)写入fileID和pageID指定的文件页中
	 * 返回:成功操作返回0
	 */
	int writePage(int fileID, int pageID, BufType buf, int off) {
		int f = fd[fileID];
		off_t offset = pageID;
		offset = (offset << PAGE_SIZE_IDX);
		off_t error = lseek(f, offset, SEEK_SET);
		if (error != offset) {
			return -1;
		}
		BufType b = buf + off;
		error = write(f, (void*) b, PAGE_SIZE);
        // std::cout << error << " " << strerror(errno) << std::endl;
		return 0;
	}
	/*
	 * @函数名readPage
	 * @参数fileID:文件id，用于区别已经打开的文件
	 * @参数pageID:文件页号
	 * @参数buf:存储信息的缓存(4字节无符号整数数组)
	 * @参数off:偏移量
	 * 功能:将fileID和pageID指定的文件页中2048个四字节整数(8kb)读入到buf+off开始的内存中
	 * 返回:成功操作返回0
	 */
	int readPage(int fileID, int pageID, BufType buf, int off) {
		//int f = fd[fID[type]];
		int f = fd[fileID];
		off_t offset = pageID;
		offset = (offset << PAGE_SIZE_IDX);
		off_t error = lseek(f, offset, SEEK_SET);
		if (error != offset) {
			return -1;
		}
		BufType b = buf + off;
		error = read(f, (void*) b, PAGE_SIZE);
		return 0;
	}
	/*
	 * @函数名closeFile
	 * @参数fileID:用于区别已经打开的文件
	 * 功能:关闭文件
	 * 返回:操作成功，返回0
	 */
	int closeFile(int fileID) {
        setBit(fm, fileID, 0);
		int f = fd[fileID];
		close(f);
		return 0;
	}
	/*
	 * @函数名createFile
	 * @参数name:文件名
	 * 功能:新建name指定的文件名
	 * 返回:操作成功，返回true
	 */
	bool createFile(const char* name) {
		return _createFile(name) == 0;
		// return true;
	}
	/*
	 * @函数名openFile
	 * @参数name:文件名
	 * @参数fileID:函数返回时，如果成功打开文件，那么为该文件分配一个id，记录在fileID中
	 * 功能:打开文件
	 * 返回:如果成功打开，在fileID中存储为该文件分配的id，返回true，否则返回false
	 */
	bool openFile(const char* name, int& fileID) {
        fileID = findRightMost(fm, 0);
        setBit(fm, fileID, 1);
		return _openFile(name, fileID) == 0;
	}
    bool deleteFile(const char*name) {
        return remove(fullName(name)) == 0;
    }
    bool deleteDir(const char*name) {
        std::vector<std::string> filenames = listDir(name);
        for (std::vector<std::string>::iterator it = filenames.begin(); it != filenames.end(); ++it) {
            deleteFile((std::string(name) + '/' + *it).c_str());
        }
        return rmdir(fullName(name)) == 0;
        return false;
    }
    bool createDir(const char* name) {
        struct stat st = {0};
        if (stat(fullName(name), &st) == -1) {
            mkdir(fullName(name), 0755);
            return true;
        } else {
            return false;
        }
    }
    std::vector<std::string> listDir(const char *name) {
        DIR *dirp;
        struct dirent *directory;
        vector<std::string> res;
        dirp = opendir(fullName(name));
        if (dirp) {
            while ((directory = readdir(dirp)) != NULL) {
                std::string str(directory->d_name);
                if (str[0] != '.') {
                    res.push_back(str);
                }
            }
            closedir(dirp);
        }
        return res;
    }
	void shutdown() {
	}
	~FileManager() {
		this->shutdown();
	}
};
#endif
