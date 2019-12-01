//
//  myfs.cpp
//  myfs
//
//  Created by Oliver Waldhorst on 02.08.17.
//  Copyright © 2017 Oliver Waldhorst. All rights reserved.
//

// The functions fuseGettattr(), fuseRead(), and fuseReadDir() are taken from
// an example by Mohammed Q. Hussain. Here are original copyrights & licence:

/**
 * Simple & Stupid Filesystem.
 *
 * Mohammed Q. Hussain - http://www.maastaar.net
 *
 * This is an example of using FUSE to build a simple filesystem. It is a part of a tutorial in MQH Blog with the title "Writing a Simple Filesystem Using FUSE in C": http://www.maastaar.net/fuse/linux/filesystem/c/2016/05/21/writing-a-simple-filesystem-using-fuse/
 *
 * License: GNU GPL
 */

// For documentation of FUSE methods see https://libfuse.github.io/doxygen/structfuse__operations.html

#undef DEBUG

// TODO: Comment this to reduce debug messages
#define DEBUG
#define DEBUG_METHODS
#define DEBUG_RETURN_VALUES

#define OK 0

#include <unistd.h>
#include <cstring>
#include <cerrno>

#include "macros.h"
#include "myfs.h"
#include "myfs-info.h"
#include "blockdevice.h"


static const char *GET_FILE_NAME(const char *path) {
    const char *name = path;
    if (*path == '/') {
        if (strlen(path) == 1) {
            name = ".";
        } else {
            name++;
        }
    }

    return name;
}

MyFS *MyFS::_instance = NULL;

MyFS *MyFS::Instance() {
    if (_instance == NULL) {
        _instance = new MyFS();
    }
    return _instance;
}

MyFS::MyFS() {
    this->logFile = stderr;
}

MyFS::~MyFS() {

}

int MyFS::fuseGetattr(const char *path, struct stat *statbuf) {
    LOGM();

    const char *fileName = GET_FILE_NAME(path);
    LOGF("\tAttributes of %s requested\n", fileName);

    if (!fileInformationManager.fileInformationExists(fileName)) {
        RETURN(-ENOENT)
    }

    *statbuf = fileInformationManager.getStat(fileName);

    RETURN(OK)
}

int MyFS::fuseMknod(const char *path, mode_t mode, dev_t dev) {
    LOGM();

    const char *fileName = GET_FILE_NAME(path);
    if (fileInformationManager.fileInformationExists(fileName)) {
        RETURN(-EEXIST);
    }

    if (!fileInformationManager.hasFreeSpace()) {
        RETURN(-ENOSPC);
    }

    fileInformationManager.createFile(fileName, mode);

    RETURN(OK);
}

int MyFS::fuseUnlink(const char *path) {
    LOGM();

    const char *fileName = GET_FILE_NAME(path);

    if (!fileInformationManager.fileInformationExists(fileName)) {
        RETURN(-ENOENT)
    }

    fileInformationManager.deleteFile(fileName);

    RETURN(OK);
}

int MyFS::fuseTruncate(const char *path, off_t newSize) {
    LOGM();

    const char *fileName = GET_FILE_NAME(path);

    if (!fileInformationManager.fileInformationExists(fileName)) {
        RETURN(-ENOENT)
    }

    fileInformationManager.truncateFile(fileName, newSize);

    RETURN(OK);
}

int MyFS::fuseOpen(const char *path, struct fuse_file_info *fileInfo) {
    LOGM();

    const char *fileName = GET_FILE_NAME(path);

    if (!fileInformationManager.fileInformationExists(fileName)) {
        RETURN(-ENOENT)
    }

    if (!openFileHandler.hasFreeSpace()) {
        RETURN(-EMFILE);
    }

    LOGF("FLAGS: %d", fileInfo->flags);

    if (!fileInformationManager.isAccessed(fileName, fileInfo->flags)) {
        RETURN(-EACCES);
    }

    MyFsFileAccessMode accessMode = fileInformationManager.getAccess(fileName, fileInfo->flags);
    int fileDescriptor = fileInformationManager.getFileDescriptor(fileName);
    int openFileHandleDescriptor = openFileHandler.openFile(fileDescriptor, accessMode);
    fileInfo->fh = openFileHandleDescriptor;

    RETURN(OK);
}

int MyFS::fuseRead(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fileInfo) {
    LOGM();
    LOGF("--> Trying to read %s, %lu, %lu\n", path, (unsigned long) offset, size);

    int openFileHandleDescriptor = fileInfo->fh;
    if (!openFileHandler.isValidOpenFileHandle(openFileHandleDescriptor) ||
        !openFileHandler.isOpen(openFileHandleDescriptor) ||
        !openFileHandler.isRead(openFileHandleDescriptor)) {
        RETURN(-EBADF);
    }

    int fileDescriptor = openFileHandler.getFileDescriptor(openFileHandleDescriptor);
    if (!fileInformationManager.fileInformationExists(fileDescriptor)) {
        RETURN(-ENOENT);
    }

    size = fileInformationManager.read(fileDescriptor, size, offset, buf);

    RETURN((int) size);
}

int MyFS::fuseWrite(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fileInfo) {
    LOGM();
    LOGF("--> Trying to write %s with size %lu and offset %lu\n", buf, size, (unsigned long) offset);

    int openFileHandleDescriptor = fileInfo->fh;
    if (!openFileHandler.isValidOpenFileHandle(openFileHandleDescriptor) ||
        !openFileHandler.isOpen(openFileHandleDescriptor) ||
        !openFileHandler.isWrite(openFileHandleDescriptor)) {
        RETURN(-EBADF);
    }

    int fileDescriptor = openFileHandler.getFileDescriptor(openFileHandleDescriptor);
    if (!fileInformationManager.fileInformationExists(fileDescriptor)) {
        RETURN(-ENOENT);
    }

    size = fileInformationManager.write(fileDescriptor, size, offset, buf);

    RETURN((int) size);
}

int MyFS::fuseRelease(const char *path, struct fuse_file_info *fileInfo) {
    LOGM();

    int openFileHandleDescriptor = fileInfo->fh;
    if (!openFileHandler.isValidOpenFileHandle(openFileHandleDescriptor) ||
        !openFileHandler.isOpen(openFileHandleDescriptor)) {
        RETURN(-EBADF);
    }

    openFileHandler.release(openFileHandleDescriptor);

    RETURN(OK);
}

int MyFS::fuseOpendir(const char *path, struct fuse_file_info *fileInfo) {
    LOGM();
    RETURN(OK); // always grant access
}

int MyFS::fuseReaddir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fileInfo) {
    LOGM();
    LOGF("--> Getting The List of Files of %s\n", path);

    // Check if path is directory
    if (strcmp("/", path) == 0) {
        for (int i = 0; i < NUM_DIR_ENTRIES; i++) {
            if (fileInformationManager.fileInformationExists(i)) {
                struct stat s = {};
                LOGF("--> Getting attributes of File %s at index %d\n",
                     fileInformationManager.getFileInformation(i).name, i);

                fuseGetattr(fileInformationManager.getFileInformation(i).name, &s);
                filler(buf, fileInformationManager.getFileInformation(i).name, &s, 0);
            }
        }
        filler(buf, "..", nullptr, 0);
    } else {
        RETURN(-ENOTDIR);
    }

    RETURN(OK);
}

int MyFS::fuseReleasedir(const char *path, struct fuse_file_info *fileInfo) {
    LOGM();
    RETURN(OK);
}

int MyFS::fuseRename(const char *path, const char *newpath) {
    LOGM();

    const char *_old = path;
    if (*_old == '/' && strlen(_old) > 1) {
        _old++;
    }
    const char *_new = newpath;
    if (*_new == '/' && strlen(_new) > 1) {
        _new++;
    }

    if (strlen(_new) > NAME_LENGTH) {
        RETURN(-ENAMETOOLONG);
    }

    if (!fileInformationManager.fileInformationExists(_old)) {
        RETURN(-ENONET);
    }

    LOGF("renaming %s to %s\n", _old, _new);

    fileInformationManager.rename(_old, _new);
    MyFsFileInformation fileInformation = fileInformationManager.getFileInformation(_old);
    LOGF("new name %s\n", fileInformation.name);

    RETURN(OK);
}

int MyFS::fuseCreate(const char *path, mode_t mode, struct fuse_file_info *fileInfo) {
    LOGM();

    dev_t dev = 0;
    int ret = fuseMknod(path, mode, dev);
    if (ret < 0) {
        RETURN(ret);
    }
    ret = fuseOpen(path, fileInfo);
    RETURN(ret);
}

void MyFS::fuseDestroy() {
    LOGM();
    // We don't need to do anything as the file information manager and the open file handler get destroyed anyways.
}

void *MyFS::fuseInit(struct fuse_conn_info *conn) {
    // Open logfile
    this->logFile = fopen(((MyFsInfo *) fuse_get_context()->private_data)->logFile, "w+");
    if (this->logFile == nullptr) {
        fprintf(stderr, "ERROR: Cannot open logfile %s\n", ((MyFsInfo *) fuse_get_context()->private_data)->logFile);
    } else {
        //    this->logFile= ((MyFsInfo *) fuse_get_context()->private_data)->logFile;

        // turn of logfile buffering
        setvbuf(this->logFile, nullptr, _IOLBF, 0);

        LOG("Starting logging...\n");
        LOGM();

        // Get in-memory flag
        this->inMemoryFs = (((MyFsInfo *) fuse_get_context()->private_data)->inMemoryFs == 1);

        if (this->inMemoryFs) {
            LOG("Using in-memory mode");
            auto *fileInformations = new MyFsFileInformation[NUM_DIR_ENTRIES];
            auto *openFileHandles = new MyFsOpenFileHandle[NUM_OPEN_FILES];

            fileInformationManager.set(fileInformations);
            fileInformationManager.init();
            openFileHandler.init(openFileHandles);

        } else {
            LOGF("Container file name: %s", ((MyFsInfo *) fuse_get_context()->private_data)->contFile);

            int ret = this->blockDevice.open(((MyFsInfo *) fuse_get_context()->private_data)->contFile);

            if (ret >= 0) {
                LOG("Container file does exist, reading");

                // TODO: Read existing structures form file

            } else if (ret == -ENOENT) {
                LOG("Container file does not exist, creating new one");

                ret = this->blockDevice.create(((MyFsInfo *) fuse_get_context()->private_data)->contFile);

                if (ret >= 0) {

                    // TODO: Create empty structures in file

                }
            }

            if (ret < 0) {
                LOGF("ERROR: Access to container file failed with error %d", ret);
            }
        }
    }

    RETURN(OK);
}

// UNUSED
int MyFS::fuseReadlink(const char *path, char *link, size_t size) {
    LOGM();
    return 0;
}

// UNUSED
int MyFS::fuseUtime(const char *path, struct utimbuf *ubuf) {
    LOGM();
    return 0;
}

// UNUSED
int MyFS::fuseStatfs(const char *path, struct statvfs *statInfo) {
    LOGM();
    return 0;
}

// UNUSED
int MyFS::fuseFlush(const char *path, struct fuse_file_info *fileInfo) {
    LOGM();
    return 0;
}

// UNUSED
int MyFS::fuseFsync(const char *path, int datasync, struct fuse_file_info *fi) {
    LOGM();
    return 0;
}

// UNUSED
int MyFS::fuseListxattr(const char *path, char *list, size_t size) {
    LOGM();
    RETURN(0);
}

// UNUSED
int MyFS::fuseRemovexattr(const char *path, const char *name) {
    LOGM();
    RETURN(0);
}

// UNUSED
int MyFS::fuseFsyncdir(const char *path, int datasync, struct fuse_file_info *fileInfo) {
    LOGM();
    RETURN(0);
}

// UNUSED
int MyFS::fuseTruncate(const char *path, off_t offset, struct fuse_file_info *fileInfo) {
    LOGM();
    RETURN(0);
}

// UNUSED
int MyFS::fuseMkdir(const char *path, mode_t mode) {
    LOGM();
    return 0;
}

// UNUSED
int MyFS::fuseRmdir(const char *path) {
    LOGM();
    return 0;
}

// UNUSED
int MyFS::fuseSymlink(const char *path, const char *link) {
    LOGM();
    return 0;
}

// UNUSED
int MyFS::fuseLink(const char *path, const char *newpath) {
    LOGM();
    return 0;
}

// UNUSED
int MyFS::fuseChmod(const char *path, mode_t mode) {
    LOGM();
    return 0;
}

// UNUSED
int MyFS::fuseChown(const char *path, uid_t uid, gid_t gid) {
    LOGM();
    return 0;
}

#ifdef __APPLE__
int MyFS::fuseSetxattr(const char *path, const char *name, const char *value, size_t size, int flags, uint32_t x) {
#else

int MyFS::fuseSetxattr(const char *path, const char *name, const char *value, size_t size, int flags) {
#endif
    LOGM();
    RETURN(0);
}

#ifdef __APPLE__
int MyFS::fuseGetxattr(const char *path, const char *name, char *value, size_t size, uint x) {
#else

int MyFS::fuseGetxattr(const char *path, const char *name, char *value, size_t size) {
#endif
    LOGM();
    RETURN(0);
}
