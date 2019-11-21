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
void MyFsFileInformationManager::initFileInformation(int fileDescriptor) {
    fileInformations[fileDescriptor].size = -1;
    fileInformations[fileDescriptor].nlink = 1;
    fileInformations[fileDescriptor].uid = geteuid(); // The owner of the file/directory is the user who mounted the filesystem
    fileInformations[fileDescriptor].gid = getegid(); // The group of the file/directory is the same as the group of the user who mounted the filesystem
}

bool MyFsFileInformationManager::fileExists(int fileDescriptor) {
    return fileInformations[fileDescriptor].size != -1;
}

bool MyFsFileInformationManager::fileNameIsEqualTo(int fileDescriptor, const char* name) {
    return strcmp(fileInformations[fileDescriptor].name, name) == 0;
}

bool MyFsFileInformationManager::fileInformationExists(const char* fileName) {
    return getFileDescriptor(fileName) != -1;
}

int MyFsFileInformationManager::getFreeFileDescriptor() {
    for(int i = 0; i < NUM_DIR_ENTRIES - 1; i++) {
        if(!fileExists(i)) {
            return i;
        }
    }
    return -1;
}

void MyFsFileInformationManager::clearFileInformation(int fileDescriptor) {
    fileInformations[fileDescriptor] = {};
}

void MyFsFileInformationManager::reInitFileInformation(int fileDescriptor) {
    clearFileInformation(fileDescriptor);
    initFileInformation(fileDescriptor);
}

void MyFsFileInformationManager::changeFileSize(int fileDescriptor, off_t size) {
    fileInformations[fileDescriptor].size = size;
}

void MyFsFileInformationManager::truncateFileData(int fileDescriptor, off_t size) {
    fileInformations[fileDescriptor].data = (char *)(realloc(fileInformations[fileDescriptor].data, size));
}

MyFsFileAccessMode MyFsFileInformationManager::getUserAccess(int fileDescriptor, mode_t mode) {
    return MyFsFileAccessMode();
}

bool MyFsFileInformationManager::isUserAccessed(int fileDescriptor, mode_t mode) {
    return false;
}

MyFsFileAccessMode MyFsFileInformationManager::getGroupAccess(int fileDescriptor, mode_t mode) {
    return MyFsFileAccessMode();
}

bool MyFsFileInformationManager::isGroupAccessed(int fileDescriptor, mode_t mode) {
    return false;
}

MyFsFileAccessMode MyFsFileInformationManager::getOtherAccess(int fileDescriptor, mode_t mode) {
    return MyFsFileAccessMode();
}

bool MyFsFileInformationManager::isOtherAccessed(int fileDescriptor, mode_t mode) {
    return false;
}

/* PUBLIC */
void MyFsFileInformationManager::init(MyFsFileInformation* fileInformations) {
    this->fileInformations = fileInformations;

    // INITIALIZE ALL FILE INFORMATIONS
    for (int i = 0; i < NUM_DIR_ENTRIES; i++) {
        initFileInformation(i);
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

int MyFsFileInformationManager::getFileDescriptor(const char* fileName) {
    for(int i = 0; i < NUM_DIR_ENTRIES - 1; i++) {
        if(fileExists(i) && fileNameIsEqualTo(i, fileName)) {
            return i;
        }
    }
    return -1;
}

MyFsFileInformation MyFsFileInformationManager::getFileInformation(const char* fileName) {
    int fileDescriptor = getFileDescriptor(fileName);
    return fileInformations[fileDescriptor];
}

struct stat MyFsFileInformationManager::getStat(const char* fileName) {
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

void MyFsFileInformationManager::createFile(const char* fileName, mode_t mode) {
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

void MyFsFileInformationManager::deleteFile(const char* fileName) {
    int fileDescriptor = getFileDescriptor(fileName);
    CURRENT_DIR_INFORMATION.size -= fileInformations[fileDescriptor].size;
    reInitFileInformation(fileDescriptor);
}

void MyFsFileInformationManager::truncateFile(const char* fileName, off_t size) {
    int fileDescriptor = getFileDescriptor(fileName);
    changeFileSize(fileDescriptor, size);
    truncateFileData(fileDescriptor, size);
}

bool MyFsFileInformationManager::isAccessed(int fileDescriptor, mode_t mode) {
    return false;
}

MyFsFileAccessMode MyFsFileInformationManager::getAccess(int fileDescriptor, mode_t mode) {
    return MyFsFileAccessMode();
}

