//
// Created by ahlex on 20.11.2019.
//

#include <cstring>
#include <unistd.h>
#include <ctime>
#include <iostream>
#include "myfs-file-information-manager.h"

MyFsFileInformationManager::MyFsFileInformationManager() = default;

MyFsFileInformationManager::~MyFsFileInformationManager() {
    delete[] this->fileInformations;
}

/* PRIVATE */
int MyFsFileInformationManager::getFileDescriptor(const char *fileName) {
    for(int i = 0; i < NUM_DIR_ENTRIES; i++) {
        if(fileExists(i) && fileNameIsEqualTo(i, fileName)) {
            return i;
        }
    }
    return -1;
}

bool MyFsFileInformationManager::fileExists(int fileDescriptor) {
    return fileInformations[fileDescriptor].size != -1;
}

bool MyFsFileInformationManager::fileNameIsEqualTo(int fileDescriptor, const char *name) {
    return strcmp(fileInformations[fileDescriptor].name, name) == 0;
}

bool MyFsFileInformationManager::fileInformationExists(const char *fileName) {
    return getFileDescriptor(fileName) != -1;
}

int MyFsFileInformationManager::getFreeFileDescriptor() {
    for(int i = 0; i < NUM_DIR_ENTRIES; i++) {
        if(!fileExists(i)) {
            return i;
        }
    }
    return -1;
}

/* PUBLIC */
void MyFsFileInformationManager::init(MyFsFileInformation *fileInformations) {
    this->fileInformations = fileInformations;

    // INITIALIZE ALL DIRECTORY / FILE INFORMATIONS
    for (int i = 0; i < NUM_DIR_ENTRIES; i++) {
        this->fileInformations[i].size = -1;
        this->fileInformations[i].nlink = 1;
        this->fileInformations[i].uid = geteuid(); // The owner of the file/directory is the user who mounted the filesystem
        this->fileInformations[i].gid = getegid(); // The group of the file/directory is the same as the group of the user who mounted the filesystem
    }

    // CURRENT DIRECTORY INFORMATION
    CURRENT_DIR_INFORMATION = {};
    CURRENT_DIR_INFORMATION.size = 0;
    strcpy(CURRENT_DIR_INFORMATION.name, ".");
    CURRENT_DIR_INFORMATION.mode = S_IFDIR | 0775;
    time_t currentTime = time(nullptr);
    CURRENT_DIR_INFORMATION.atime = currentTime; // The last "access of the file/directory is right now
    CURRENT_DIR_INFORMATION.ctime = currentTime; // The last "check of the file/directory is right now
    CURRENT_DIR_INFORMATION.mtime = currentTime; // The last "modification of the file/directory is right now
    CURRENT_DIR_INFORMATION.nlink = 2; // Why "two" hardlinks instead of "one"? The answer is here: http://unix.stackexchange.com/a/101536
}

MyFsFileInformation MyFsFileInformationManager::getFileInformation(const char *fileName) {
    int fileDescriptor = getFileDescriptor(fileName);
    return fileInformations[fileDescriptor];
}

struct stat MyFsFileInformationManager::getStat(const char *fileName) {
    MyFsFileInformation fileInformation = getFileInformation(fileName);
    struct stat statbuf = {};
    statbuf.st_uid = fileInformation.uid;
    statbuf.st_gid = fileInformation.gid;
    statbuf.st_mode = fileInformation.mode;
    statbuf.st_size = fileInformation.size;
    statbuf.st_atime = fileInformation.atime;
    statbuf.st_ctime = fileInformation.ctime;
    statbuf.st_mtime = fileInformation.mtime;
    statbuf.st_nlink = fileInformation.nlink;

    return statbuf;
}

bool MyFsFileInformationManager::hasFreeSpace() {
    return getFreeFileDescriptor() != -1;
}

void MyFsFileInformationManager::createFile(const char *fileName, mode_t mode) {
    int freeFileDescriptor = getFreeFileDescriptor();

    MyFsFileInformation fileInformation = {};
    strcpy(fileInformation.name, fileName);
    time_t currentTime = time(nullptr);
    fileInformation.atime = currentTime;
    fileInformation.ctime = currentTime;
    fileInformation.mtime = currentTime;
    fileInformation.mode = S_IFREG | mode;
    fileInformation.size = 0;
    fileInformations[freeFileDescriptor] = fileInformation;
}

