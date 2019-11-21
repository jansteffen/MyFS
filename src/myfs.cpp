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

#include <unistd.h>
#include <cstring>
#include <cerrno>

#include "macros.h"
#include "myfs.h"
#include "myfs-info.h"
#include "blockdevice.h"


static const char* GET_FILE_NAME(const char *path) {
    const char* name = path;
    if (*path == '/') {
        if (strlen(path) == 1) {
            name = ".";
        } else {
            name++;
        }
    }

    return name;
}

MyFS* MyFS::_instance = NULL;

MyFS* MyFS::Instance() {
    if(_instance == NULL) {
        _instance = new MyFS();
    }
    return _instance;
}

MyFS::MyFS() {
    this->logFile= stderr;
}

MyFS::~MyFS() {
    
}

int MyFS::fuseGetattr(const char *path, struct stat *statbuf) {
    LOGM();
    LOGF( "\tAttributes of %s requested\n", path );

    const char* fileName = GET_FILE_NAME(path);

    if(!fileInformationManager.fileInformationExists(fileName)) {
        RETURN(-ENOENT)
    }

    *statbuf = fileInformationManager.getStat(fileName);

    RETURN(0)
}

int MyFS::fuseMknod(const char *path, mode_t mode, dev_t dev) {
    LOGM();

    const char* fileName = GET_FILE_NAME(path);
    if(fileInformationManager.fileInformationExists(fileName)) {
        RETURN(-EEXIST);
    }

    if(!fileInformationManager.hasFreeSpace()) {
        RETURN(-ENOSPC);
    }

    fileInformationManager.createFile(fileName, mode);
    
    RETURN(0);
}

int MyFS::fuseUnlink(const char *path) {
    LOGM();

    const char* fileName = GET_FILE_NAME(path);

    if(!fileInformationManager.fileInformationExists(fileName)) {
        RETURN(-ENOENT)
    }

    fileInformationManager.deleteFile(fileName);
    
    RETURN(0);
}

int MyFS::fuseTruncate(const char *path, off_t newSize) {
    LOGM();

    const char* fileName = GET_FILE_NAME(path);

    if(!fileInformationManager.fileInformationExists(fileName)) {
        RETURN(-ENOENT)
    }

    fileInformationManager.truncateFile(fileName, newSize);

    return 0;
}

int MyFS::fuseOpen(const char *path, struct fuse_file_info *fileInfo) {
    LOGM();
    
    // TODO: Implement this!
    
    RETURN(0);
}

int MyFS::fuseRead(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fileInfo) {
    LOGM();
    
    // TODO: Implement this!

    LOGF( "--> Trying to read %s, %lu, %lu\n", path, (unsigned long) offset, size );

    char file54Text[] = "Hello World From File54!\n";
    char file349Text[] = "Hello World From File349!\n";
    char *selectedText = NULL;

    // ... //

    if ( strcmp( path, "/file54" ) == 0 )
        selectedText = file54Text;
    else if ( strcmp( path, "/file349" ) == 0 )
        selectedText = file349Text;
    else
        return -ENOENT;

    // ... //

    memcpy( buf, selectedText + offset, size );

    RETURN((int) (strlen( selectedText ) - offset));
}

int MyFS::fuseWrite(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fileInfo) {
    LOGM();
    
    // TODO: Implement this!
    
    RETURN(0);
}

int MyFS::fuseRelease(const char *path, struct fuse_file_info *fileInfo) {
    LOGM();
    
    // TODO: Implement this!
    
    RETURN(0);
}

int MyFS::fuseOpendir(const char *path, struct fuse_file_info *fileInfo) {
    LOGM();
    
    // (TODO: Implement this!)
    
    RETURN(0);
}

int MyFS::fuseReaddir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fileInfo) {
    LOGM();
    
    // TODO: Implement this!

    LOGF( "--> Getting The List of Files of %s\n", path );

    filler( buf, ".", NULL, 0 ); // Current Directory
    filler( buf, "..", NULL, 0 ); // Parent Directory

    if ( strcmp( path, "/" ) == 0 ) // If the user is trying to show the files/directories of the root directory show the following
    {
        filler( buf, "file54", NULL, 0 );
        filler( buf, "file349", NULL, 0 );
    }

    RETURN(0);
}

int MyFS::fuseReleasedir(const char *path, struct fuse_file_info *fileInfo) {
    LOGM();
    
    // (TODO: Implement this!)
    
    RETURN(0);
}

int MyFS::fuseCreate(const char *path, mode_t mode, struct fuse_file_info *fileInfo) {
    LOGM();
    
    // TODO: Implement this!
    
    RETURN(0);
}

void MyFS::fuseDestroy() {
    LOGM();
    // We don't need to do anything as the file information manager and the open file handler get destroyed anyways.
}

void* MyFS::fuseInit(struct fuse_conn_info *conn) {
    // Open logfile
    this->logFile= fopen(((MyFsInfo *) fuse_get_context()->private_data)->logFile, "w+");
    if(this->logFile == NULL) {
        fprintf(stderr, "ERROR: Cannot open logfile %s\n", ((MyFsInfo *) fuse_get_context()->private_data)->logFile);
    } else {
        //    this->logFile= ((MyFsInfo *) fuse_get_context()->private_data)->logFile;
        
        // turn of logfile buffering
        setvbuf(this->logFile, NULL, _IOLBF, 0);
        
        LOG("Starting logging...\n");
        LOGM();

        // Get in-memory flag
        this->inMemoryFs= (((MyFsInfo *) fuse_get_context()->private_data)->inMemoryFs == 1);

        if(this->inMemoryFs) {
            LOG("Using in-memory mode");
            auto *fileInformations = new MyFsFileInformation[NUM_DIR_ENTRIES];
            auto *openFileHandles = new MyFsOpenFileHandle[NUM_OPEN_FILES];

           fileInformationManager.init(fileInformations);
           openFileHandler.init(openFileHandles);

        } else {
            LOGF("Container file name: %s", ((MyFsInfo *) fuse_get_context()->private_data)->contFile);

            int ret= this->blockDevice.open(((MyFsInfo *) fuse_get_context()->private_data)->contFile);

            if(ret >= 0) {
                LOG("Container file does exist, reading");

                // TODO: Read existing structures form file

            } else if(ret == -ENOENT) {
                LOG("Container file does not exist, creating new one");

                ret = this->blockDevice.create(((MyFsInfo *) fuse_get_context()->private_data)->contFile);

                if (ret >= 0) {

                    // TODO: Create empty structures in file
                    
                }
            }

            if(ret < 0) {
                LOGF("ERROR: Access to container file failed with error %d", ret);
            }
        }
    }
    
    RETURN(0);
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

int MyFS::fuseRename(const char *path, const char *newpath) {
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
