//
// Created by ahlex on 20.11.2019.
//
#ifndef MYFS_MYFS_FILE_INFORMATION_MANAGER_H
#define MYFS_MYFS_FILE_INFORMATION_MANAGER_H
#define CURRENT_DIR_INFORMATION fileInformations[NUM_DIR_ENTRIES]

#include "myfs-structs.h"

class MyFsFileInformationManager {
private:
    MyFsFileInformation * fileInformations;
    int getFileDescriptor(const char *fileName);
    bool fileExists(int fileDescriptor);
    bool fileNameIsEqualTo(int fileDescriptor, const char *name);
    int getFreeFileDescriptor();

public:
    MyFsFileInformationManager();
    ~MyFsFileInformationManager();

    void init(MyFsFileInformation * fileInformations);

    bool fileInformationExists(const char *fileName);
    MyFsFileInformation getFileInformation(const char *fileName);
    struct stat getStat(const char *fileName);

    bool hasFreeSpace();
    void createFile(const char* fileName, mode_t mode);
};

#endif //MYFS_MYFS_FILE_INFORMATION_MANAGER_H
