//
// Created by ahlex on 20.11.2019.
//

#include "myfs-open-file-handler.h"

MyFsOpenFileHandler::MyFsOpenFileHandler() = default;

MyFsOpenFileHandler::~MyFsOpenFileHandler() {
    delete[] this->openFileHandles;
}

void MyFsOpenFileHandler::init(MyFsOpenFileHandle * openFileHandles) {
    this->openFileHandles = openFileHandles;

    // INITIALIZE ALL OPEN FILE HANDLES
    for (int i = 0; i < NUM_OPEN_FILES; i++) {
        openFileHandles[i].fileDescriptor = -1;
    }
}

