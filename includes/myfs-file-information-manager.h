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
    void initFileInformation(int fileDescriptor);
    bool fileExists(int fileDescriptor);
    bool fileNameIsEqualTo(int fileDescriptor, const char* name);
    int getFreeFileDescriptor();
    void clearFileInformation(int fileDescriptor);
    void reInitFileInformation(int fileDescriptor);
    void changeFileSize(int fileDescriptor, off_t size);
    void truncateFileData(int fileDescriptor, off_t size);
    MyFsFileAccessMode getUserAccess(int fileDescriptor, mode_t mode);
    bool isUserAccessed(int fileDescriptor, mode_t mode);
    MyFsFileAccessMode getGroupAccess(int fileDescriptor, mode_t mode);
    bool isGroupAccessed(int fileDescriptor, mode_t mode);
    MyFsFileAccessMode getOtherAccess(int fileDescriptor, mode_t mode);
    bool isOtherAccessed(int fileDescriptor, mode_t mode);

public:
    MyFsFileInformationManager();
    ~MyFsFileInformationManager();

    void init(MyFsFileInformation* fileInformations);

    int getFileDescriptor(const char* fileName);
    bool fileInformationExists(const char* fileName);
    MyFsFileInformation getFileInformation(const char* fileName);
    struct stat getStat(const char* fileName);

    bool hasFreeSpace();
    void createFile(const char* fileName, mode_t mode);
    void deleteFile(const char* fileName);
    void truncateFile(const char* fileName, off_t size);

    bool isAccessed(int fileDescriptor, mode_t mode);
    MyFsFileAccessMode getAccess(int fileDescriptor, mode_t mode);

};

#endif //MYFS_MYFS_FILE_INFORMATION_MANAGER_H
