//
// Created by ahlex on 20.11.2019.
//

#include <fuse_common.h>
#include "myfs-open-file-handler.h"

MyFsOpenFileHandler::MyFsOpenFileHandler() = default;

MyFsOpenFileHandler::~MyFsOpenFileHandler() {
    delete[] this->openFileHandles;
}

void MyFsOpenFileHandler::init(MyFsOpenFileHandle * openFileHandles) {
    this->openFileHandles = openFileHandles;

    // INITIALIZE ALL OPEN FILE HANDLES
    for (int i = 0; i < NUM_OPEN_FILES; i++) {
        release(i);
    }
}

bool MyFsOpenFileHandler::hasFreeSpace() {
    for (int i = 0; i < NUM_OPEN_FILES; i++) {
        if (openFileHandles[i].fileDescriptor < 0) {
            return true;
        }
    }

    return false;
}

int MyFsOpenFileHandler::openFile(int fileDescriptor, MyFsFileAccessMode accessMode) {
    for (int i = 0; i < NUM_OPEN_FILES; i++) {
        openFileHandles[i].fileDescriptor = fileDescriptor;
        openFileHandles[i].accessMode = accessMode;
        return i;
    }
}

bool MyFsOpenFileHandler::isValidOpenFileHandle(int openFileHandleDescriptor) {
    return openFileHandleDescriptor >= 0 && openFileHandleDescriptor < NUM_OPEN_FILES;
}

bool MyFsOpenFileHandler::isOpen(int openFileHandleDescriptor) {
    MyFsOpenFileHandle openFileHandle = openFileHandles[openFileHandleDescriptor];
    return openFileHandle.fileDescriptor != NOT_OPEN;
}

bool MyFsOpenFileHandler::isRead(int openFileHandleDescriptor) {
    MyFsOpenFileHandle openFileHandle = openFileHandles[openFileHandleDescriptor];
    return openFileHandle.accessMode.read;
}

bool MyFsOpenFileHandler::isWrite(int openFileHandleDescriptor) {
    MyFsOpenFileHandle openFileHandle = openFileHandles[openFileHandleDescriptor];
    return openFileHandle.accessMode.write;
}

int MyFsOpenFileHandler::getFileDescriptor(int openFileHandleDescriptor) {
    return openFileHandles[openFileHandleDescriptor].fileDescriptor;
}

void MyFsOpenFileHandler::release(int openFileHandleDescriptor) {
    openFileHandles[openFileHandleDescriptor].fileDescriptor = NOT_OPEN;
    MyFsFileAccessMode accessMode = {};
    accessMode.read = false;
    accessMode.write = false;
    openFileHandles[openFileHandleDescriptor].accessMode = accessMode;
}
