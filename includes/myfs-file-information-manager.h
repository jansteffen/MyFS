//
// Created by ahlex on 20.11.2019.
//
#ifndef MYFS_MYFS_FILE_INFORMATION_MANAGER_H
#define MYFS_MYFS_FILE_INFORMATION_MANAGER_H
#define CURRENT_DIR_INFORMATION fileInformations[NUM_DIR_ENTRIES - 1]

#include "myfs-structs.h"

class MyFsFileInformationManager {
private:
    MyFsFileInformation * fileInformations;
    static const int IS_FREE = -1;
    static const int DOES_NOT_EXIST = -1;
    void initFileInformation(int fileDescriptor);
    bool fileNameIsEqualTo(int fileDescriptor, const char* name);
    int getFreeFileDescriptor();
    void clearFileInformation(int fileDescriptor);
    void reInitFileInformation(int fileDescriptor);
    void changeFileSize(int fileDescriptor, off_t size);
    void truncateFileData(int fileDescriptor, off_t size);

    MyFsFileAccessMode getUserAccess(int fileDescriptor, int flags);

    bool isUserAccessed(int fileDescriptor, int flags);

    MyFsFileAccessMode getGroupAccess(int fileDescriptor, int flags);

    bool isGroupAccessed(int fileDescriptor, int flags);

    MyFsFileAccessMode getOtherAccess(int fileDescriptor, int flags);

    bool isOtherAccessed(int fileDescriptor, int flags);

public:
    MyFsFileInformationManager();
    ~MyFsFileInformationManager();

    void init(MyFsFileInformation* fileInformations);

    int getFileDescriptor(const char* fileName);
    bool fileInformationExists(const char* fileName);

    bool fileInformationExists(int fileDescriptor);
    MyFsFileInformation getFileInformation(const char* fileName);

    MyFsFileInformation getFileInformation(int fileDescriptor);
    struct stat getStat(const char* fileName);

    bool hasFreeSpace();
    void createFile(const char* fileName, mode_t mode);
    void deleteFile(const char* fileName);
    void truncateFile(const char* fileName, off_t size);

    bool isAccessed(const char *fileName, int flags);

    MyFsFileAccessMode getAccess(const char *fileName, int flags);

    void update(MyFsFileInformation fileInformation);

    size_t read(int fileDescriptor, size_t size, off_t offset, char *buf);

    size_t write(int fileDescriptor, size_t size, off_t offset, const char *buf);


};

#endif //MYFS_MYFS_FILE_INFORMATION_MANAGER_H
