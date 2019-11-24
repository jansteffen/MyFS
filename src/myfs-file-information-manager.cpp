//
// Created by ahlex on 20.11.2019.
//

#include <cstring>
#include <unistd.h>
#include <ctime>
#include <iostream>
#include <fcntl.h>
#include "myfs-file-information-manager.h"

MyFsFileInformationManager::MyFsFileInformationManager() = default;

MyFsFileInformationManager::~MyFsFileInformationManager() {
    delete[] this->fileInformations;
}

/* PRIVATE */
void MyFsFileInformationManager::initFileInformation(int fileDescriptor) {
    fileInformations[fileDescriptor].size = IS_FREE;
    fileInformations[fileDescriptor].nlink = 1;
    fileInformations[fileDescriptor].uid = geteuid(); // The owner of the file/directory is the user who mounted the filesystem
    fileInformations[fileDescriptor].gid = getegid(); // The group of the file/directory is the same as the group of the user who mounted the filesystem
}

bool MyFsFileInformationManager::fileNameIsEqualTo(int fileDescriptor, const char* name) {
    return strcmp(fileInformations[fileDescriptor].name, name) == 0;
}

int MyFsFileInformationManager::getFreeFileDescriptor() {
    for(int i = 0; i < NUM_DIR_ENTRIES - 1; i++) {
        if (!fileInformationExists(i)) {
            return i;
        }
    }
    return DOES_NOT_EXIST;
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

MyFsFileAccessMode MyFsFileInformationManager::getUserAccess(int fileDescriptor, int flags) {
    MyFsFileInformation fileInformation = fileInformations[fileDescriptor];
    MyFsFileAccessMode accessMode = {};
    accessMode.read = false;
    accessMode.write = false;

    if (fileInformation.uid == geteuid()) {
        if ((flags & O_RDWR) != 0) {
            if ((fileInformation.mode & S_IRWXU) != 0) {
                accessMode.read = true;
                accessMode.write = true;
            }
        } else if ((flags & O_WRONLY) != 0) {
            if ((fileInformation.mode & S_IWUSR) != 0) {
                accessMode.write = true;
            }
        } else {
            if ((fileInformation.mode & S_IRUSR) != 0) {
                accessMode.read = true;
            }
        }
    }

    return accessMode;
}

bool MyFsFileInformationManager::isUserAccessed(int fileDescriptor, int flags) {
    MyFsFileAccessMode accessMode = getUserAccess(fileDescriptor, flags);
    return accessMode.write || accessMode.read;
}

MyFsFileAccessMode MyFsFileInformationManager::getGroupAccess(int fileDescriptor, int flags) {
    MyFsFileInformation fileInformation = fileInformations[fileDescriptor];
    MyFsFileAccessMode accessMode = {};
    accessMode.read = false;
    accessMode.write = false;

    if (fileInformation.gid == getegid()) {
        if ((flags & O_RDWR) != 0) {
            if ((fileInformation.mode & S_IRWXG) != 0) {
                accessMode.read = true;
                accessMode.write = true;
            }
        } else if ((flags & O_WRONLY) != 0) {
            if ((fileInformation.mode & S_IWGRP) != 0) {
                accessMode.write = true;
            }
        } else {
            if ((fileInformation.mode & S_IRGRP) != 0) {
                accessMode.read = true;
            }
        }
    }

    return accessMode;
}

bool MyFsFileInformationManager::isGroupAccessed(int fileDescriptor, int flags) {
    MyFsFileAccessMode accessMode = getGroupAccess(fileDescriptor, flags);
    return accessMode.write || accessMode.read;
}

MyFsFileAccessMode MyFsFileInformationManager::getOtherAccess(int fileDescriptor, int flags) {
    MyFsFileInformation fileInformation = fileInformations[fileDescriptor];
    MyFsFileAccessMode accessMode = {};
    accessMode.read = false;
    accessMode.write = false;

    if (fileInformation.uid != geteuid() && fileInformation.gid != getegid()) {
        if ((flags & O_RDWR) != 0) {
            if ((fileInformation.mode & S_IRWXO) != 0) {
                accessMode.read = true;
                accessMode.write = true;
            }
        } else if ((flags & O_WRONLY) != 0) {
            if ((fileInformation.mode & S_IWOTH) != 0) {
                accessMode.write = true;
            }
        } else {
            if ((fileInformation.mode & S_IROTH) != 0) {
                accessMode.read = true;
            }
        }
    }

    return accessMode;
}

bool MyFsFileInformationManager::isOtherAccessed(int fileDescriptor, int flags) {
    MyFsFileAccessMode accessMode = getOtherAccess(fileDescriptor, flags);
    return accessMode.write || accessMode.read;
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


bool MyFsFileInformationManager::fileInformationExists(int fileDescriptor) {
    return fileInformations[fileDescriptor].size != IS_FREE;
}

bool MyFsFileInformationManager::fileInformationExists(const char *fileName) {
    return getFileDescriptor(fileName) != DOES_NOT_EXIST;
}

int MyFsFileInformationManager::getFileDescriptor(const char* fileName) {
    for (int i = 0; i < NUM_DIR_ENTRIES; i++) {
        if (fileInformationExists(i) && fileNameIsEqualTo(i, fileName)) {
            return i;
        }
    }
    return DOES_NOT_EXIST;
}

MyFsFileInformation MyFsFileInformationManager::getFileInformation(int fileDescriptor) {
    return fileInformations[fileDescriptor];
}

MyFsFileInformation MyFsFileInformationManager::getFileInformation(const char* fileName) {
    int fileDescriptor = getFileDescriptor(fileName);
    return getFileInformation(fileDescriptor);
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
    return getFreeFileDescriptor() != DOES_NOT_EXIST;
}

void MyFsFileInformationManager::createFile(const char* fileName, mode_t mode) {
    int freeFileDescriptor = getFreeFileDescriptor();

    MyFsFileInformation fileInformation = fileInformations[freeFileDescriptor];
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

MyFsFileAccessMode MyFsFileInformationManager::getAccess(const char *fileName, int flags) {
    int fileDescriptor = getFileDescriptor(fileName);

    MyFsFileAccessMode accessMode = {};
    accessMode.write = false;
    accessMode.read = false;

    if (isUserAccessed(fileDescriptor, flags)) {
        accessMode = getUserAccess(fileDescriptor, flags);
    } else if (isGroupAccessed(fileDescriptor, flags)) {
        accessMode = getGroupAccess(fileDescriptor, flags);
    } else if (isOtherAccessed(fileDescriptor, flags)) {
        accessMode = getOtherAccess(fileDescriptor, flags);
    }

    return accessMode;
}

bool MyFsFileInformationManager::isAccessed(const char *fileName, int flags) {
    MyFsFileAccessMode accessMode = getAccess(fileName, flags);
    return accessMode.read || accessMode.write;
}

void MyFsFileInformationManager::update(MyFsFileInformation fileInformation) {
    int fileDescriptor = getFileDescriptor(fileInformation.name);
    MyFsFileInformation oldFileInformation = getFileInformation(fileDescriptor);
    CURRENT_DIR_INFORMATION.size -= oldFileInformation.size;
    CURRENT_DIR_INFORMATION.size += fileInformation.size;
    fileInformations[fileDescriptor] = fileInformation;
}

size_t MyFsFileInformationManager::read(int fileDescriptor, size_t size, off_t offset, char *buf) {
    MyFsFileInformation fileInformation = getFileInformation(fileDescriptor);

    if (offset < 0) {
        offset = 0;
    }

    if (fileInformation.size <= offset) {
        return 0; // EOF
    }

    if ((uint64_t) fileInformation.size < offset + size) {
        size = fileInformation.size - offset;
    }

    fileInformation.atime = time(nullptr);
    memcpy(buf, fileInformation.data + offset, size);
    update(fileInformation);

    return size;
}

size_t MyFsFileInformationManager::write(int fileDescriptor, size_t size, off_t offset, const char *buf) {
    if (offset < 0) {
        offset = 0;
    }

    MyFsFileInformation fileInformation = getFileInformation(fileDescriptor);

    if (fileInformation.size < offset) {
        size_t sizeZero = offset - fileInformation.size;
        char bufferZero[sizeZero];
        for (int i = 0; i < (int) sizeZero; i++) {
            bufferZero[i] = 0;
        }

        write(fileDescriptor, sizeZero, fileInformation.size, bufferZero);
    }

    if ((uint64_t) fileInformation.size < offset + size) {
        fileInformation.size = offset + size;
    }

    fileInformation.data = (char *) realloc(fileInformation.data, fileInformation.size);
    time_t currentTime = time(nullptr);
    fileInformation.atime = currentTime;
    fileInformation.mtime = currentTime;
    memcpy(fileInformation.data + offset, buf, size);
    update(fileInformation);

    return size;
}