//
// Created by ahlex on 20.11.2019.
//

#ifndef MYFS_MYFS_OPEN_FILE_HANDLER_H
#define MYFS_MYFS_OPEN_FILE_HANDLER_H

#include "myfs-structs.h"

class MyFsOpenFileHandler {
private:
    MyFsOpenFileHandle *openFileHandles;
    static const int NOT_OPEN = -1;

public:
    MyFsOpenFileHandler();
    ~MyFsOpenFileHandler();

    void init(MyFsOpenFileHandle * openFileHandles);

    bool hasFreeSpace();

    int openFile(int fileDescriptor, MyFsFileAccessMode accessMode);

    bool isValidOpenFileHandle(int openFileHandleDescriptor);

    bool isOpen(int openFileHandleDescriptor);

    bool isRead(int openFileHandleDescriptor);

    bool isWrite(int openFileHandleDescriptor);

    int getFileDescriptor(int openFileHandleDescriptor);

    void release(int openFileHandleDescriptor);
};

#endif //MYFS_MYFS_OPEN_FILE_HANDLER_H
