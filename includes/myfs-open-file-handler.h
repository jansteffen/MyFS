//
// Created by ahlex on 20.11.2019.
//

#ifndef MYFS_MYFS_OPEN_FILE_HANDLER_H
#define MYFS_MYFS_OPEN_FILE_HANDLER_H

#include "myfs-structs.h"

class MyFsOpenFileHandler {
private:
    MyFsOpenFileHandle *openFileHandles;
public:
    MyFsOpenFileHandler();
    ~MyFsOpenFileHandler();

    void init(MyFsOpenFileHandle * openFileHandles);
};

#endif //MYFS_MYFS_OPEN_FILE_HANDLER_H
