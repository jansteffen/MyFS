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
#include <string.h>
#include <errno.h>

#include "macros.h"
#include "myfs.h"
#include "myfs-info.h"

MyFS* MyFS::_instance = NULL;
MyFsFileInfo files[NUM_DIR_ENTRIES];
int64_t fileHandles[NUM_OPEN_FILES];


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
    
    // TODO: Implement this!

    LOGF( "\tAttributes of %s requested\n", path );

    // GNU's definitions of the attributes (http://www.gnu.org/software/libc/manual/html_node/Attribute-Meanings.html):
    // 		st_uid: 	The user ID of the file’s owner.
    //		st_gid: 	The group ID of the file.
    //		st_atime: 	This is the last access time for the file.
    //		st_mtime: 	This is the time of the last modification to the contents of the file.
    //		st_mode: 	Specifies the mode of the file. This includes file type information (see Testing File Type) and the file permission bits (see Permission Bits).
    //		st_nlink: 	The number of hard links to the file. This count keeps track of how many directories have entries for this file. If the count is ever decremented to zero, then the file itself is discarded as soon
    //						as no process still holds it open. Symbolic links are not counted in the total.
    //		st_size:	This specifies the size of a regular file in bytes. For files that are really devices this field isn’t usually meaningful. For symbolic links this specifies the length of the file name the link refers to.

    statbuf->st_uid = getuid(); // The owner of the file/directory is the user who mounted the filesystem
    statbuf->st_gid = getgid(); // The group of the file/directory is the same as the group of the user who mounted the filesystem
    statbuf->st_atime = time( NULL ); // The last "access of the file/directory is right now

    int ret= 0;

    if ( strcmp( path, "/" ) == 0 )
    {
        statbuf->st_mode = S_IFDIR | 0755;
        statbuf->st_nlink = 2; // Why "two" hardlinks instead of "one"? The answer is here: http://unix.stackexchange.com/a/101536
    }
    else {
        for (int i = 0; i < NUM_DIR_ENTRIES; i++) {
            if(strcmp(path+1, files[i].name) == 0) {
                statbuf->st_mode = S_IFREG | 0644;
                statbuf->st_nlink = 1;
                statbuf->st_size = files[i].size;
                statbuf->st_mtime = files[i].mtime;
                statbuf->st_ctime = files[i].ctime;
                RETURN(ret);
            }
        }
        ret = -ENOENT;
    }
    RETURN(ret);
}


int MyFS::fuseReadlink(const char *path, char *link, size_t size) {
    LOGM();
    return 0;
}

int MyFS::fuseMknod(const char *path, mode_t mode, dev_t dev) {
    LOGM();
    // TODO: Implement this!
    int freeSpaceIndex = -1;
    for (int i = 0; i < NUM_DIR_ENTRIES; i++) {
        if(strcmp(path+1,files[i].name) == 0) {
            RETURN(-EEXIST);
        }
        if (strlen(files[i].name) == 0 && freeSpaceIndex == -1) {
            freeSpaceIndex = i;
        }
    }
    if(freeSpaceIndex == -1) {
        RETURN(-ENOSPC);
    }
    strcpy(files[freeSpaceIndex].name, path+1);
    files[freeSpaceIndex].mode=mode;
    files[freeSpaceIndex].ctime = time(NULL);
    files[freeSpaceIndex].atime = time(NULL);
    files[freeSpaceIndex].mtime = time(NULL);
    RETURN(0);
}

int MyFS::fuseMkdir(const char *path, mode_t mode) {
    LOGM();
    return 0;
}

int MyFS::fuseUnlink(const char *path) {
    LOGM();
    
    // TODO: Implement this!
    int foundIndex = -1;
    for (int i = 0; i < NUM_DIR_ENTRIES; i++) {
        if(strcmp(path+1,files[i].name) == 0) {
            foundIndex = i;
            break;
        }
    }
    if(foundIndex == -1)  {
        RETURN(-ENOENT);
    }
    for (int j = 0; j < NUM_OPEN_FILES; j++) {
        if(fileHandles[j] == foundIndex) {
            fileHandles[j] = -1;
        }
    }
    //free(files[foundIndex].name);
    free(files[foundIndex].data);
	files[foundIndex].data = (char*)malloc((files[foundIndex].size+1)*sizeof(char));
    files[foundIndex].name[0] = '\0';

    RETURN(0);
}

int MyFS::fuseRmdir(const char *path) {
    LOGM();
    return 0;
}

int MyFS::fuseSymlink(const char *path, const char *link) {
    LOGM();
    return 0;
}

int MyFS::fuseRename(const char *path, const char *newpath) {
    LOGM();
    return 0;
}

int MyFS::fuseLink(const char *path, const char *newpath) {
    LOGM();
    return 0;
}

int MyFS::fuseChmod(const char *path, mode_t mode) {
    LOGM();
    return 0;
}

int MyFS::fuseChown(const char *path, uid_t uid, gid_t gid) {
    LOGM();
    return 0;
}

int MyFS::fuseTruncate(const char *path, off_t newSize) {
    LOGM();
    for (int i = 0; i < NUM_DIR_ENTRIES; i++) {
        if(strcmp(files[i].name, path + 1) == 0) {
            files[i].size = newSize;
            files[i].data = (char *)(realloc(files[i].data, newSize));
            RETURN(0);
        }
    }
    RETURN(-ENOENT);
}

int MyFS::fuseUtime(const char *path, struct utimbuf *ubuf) {
    LOGM();
    return 0;
}

int MyFS::fuseOpen(const char *path, struct fuse_file_info *fileInfo) {
    LOGM();

    int foundIndex = -1;
    for (int i = 0; i < NUM_DIR_ENTRIES; i++) {
        if(strcmp(path+1,files[i].name) == 0) {
            foundIndex = i;
            break;
        }
    }
    if(foundIndex == -1)  {
        RETURN(-ENOENT);
    }

    for (int j = 0; j < NUM_OPEN_FILES; j++) {
        if(fileHandles[j] == foundIndex) {
            RETURN(-EMFILE);
        }
    }

    for(int k = 0; k < NUM_OPEN_FILES; k++) {
        if(fileHandles[k] == -1) {
            fileHandles[k] = foundIndex;
            fileInfo -> fh = k;
            files[foundIndex].atime = time(NULL);
            RETURN(0);
        }
    }

    RETURN(-EMFILE);
}

int MyFS::fuseRead(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fileInfo) {
    LOGM();
    
    // TODO: Implement this!
    if(fileInfo -> fh < 0 || fileInfo -> fh >= NUM_OPEN_FILES || fileHandles[fileInfo -> fh] == -1) {
        RETURN(-EBADF);
    }

    LOGF( "--> Trying to read %s, %lu, %lu\n", path, (unsigned long) offset, size );
    memcpy( buf, files[fileHandles[fileInfo->fh]].data + offset, size );
    files[fileHandles[fileInfo -> fh]].atime = time(NULL);
    RETURN(strlen( files[fileHandles[fileInfo->fh]].data ) - offset);
}

int MyFS::fuseWrite(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fileInfo) {
    LOGM();
    
    // TODO: Implement this!
    if(fileInfo -> fh < 0 || fileInfo -> fh >= NUM_OPEN_FILES || fileHandles[fileInfo -> fh] == -1) {
        RETURN(-EBADF);
    }
	LOGF("--> Trying to write %s with size %lu and offset %lu\n", buf, size, (unsigned long) offset);
    
    if(offset+size > CONTAINER_SIZE) {
        RETURN (-ENOSPC);
    }
    files[fileHandles[fileInfo -> fh]].atime = time(NULL);
    files[fileHandles[fileInfo -> fh]].mtime = time(NULL);
	files[fileHandles[fileInfo -> fh]].data = (char *)realloc(files[fileHandles[fileInfo -> fh]].data, files[fileHandles[fileInfo -> fh]].size + size);
	files[fileHandles[fileInfo -> fh]].size += size;
    memcpy(files[fileHandles[fileInfo->fh]].data + offset , buf, size );
    RETURN(size);
}

int MyFS::fuseStatfs(const char *path, struct statvfs *statInfo) {
    LOGM();
    return 0;
}

int MyFS::fuseFlush(const char *path, struct fuse_file_info *fileInfo) {
    LOGM();
    return 0;
}

int MyFS::fuseRelease(const char *path, struct fuse_file_info *fileInfo) {
    LOGM();
    
    // TODO: Implement this!
    if(fileInfo -> fh < 0 || fileInfo -> fh >= NUM_OPEN_FILES) {
        RETURN(-EBADF);
    }
    fileHandles[fileInfo->fh] = -1;
    RETURN(0);
}

int MyFS::fuseFsync(const char *path, int datasync, struct fuse_file_info *fi) {
    LOGM();
    return 0;
}

int MyFS::fuseListxattr(const char *path, char *list, size_t size) {
    LOGM();
    RETURN(0);
}

int MyFS::fuseRemovexattr(const char *path, const char *name) {
    LOGM();
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
        for (int i = 0; i < NUM_DIR_ENTRIES; i++) {
            if(files[i].name[0] == '\0') {
                continue;
            }
            filler( buf, files[i].name, NULL, 0 );
        }
        //filler( buf, "file54", NULL, 0 );
        //filler( buf, "file349", NULL, 0 );
    }

    RETURN(0);
}

int MyFS::fuseReleasedir(const char *path, struct fuse_file_info *fileInfo) {
    LOGM();
    
    // (TODO: Implement this!)
    
    RETURN(0);
}

int MyFS::fuseFsyncdir(const char *path, int datasync, struct fuse_file_info *fileInfo) {
    LOGM();
    RETURN(0);
}

int MyFS::fuseTruncate(const char *path, off_t offset, struct fuse_file_info *fileInfo) {
    LOGM();
LOGF("--> Trying to truncate %s with offset %lu\n", path, (unsigned long) offset);
    for (int i = 0; i < NUM_DIR_ENTRIES; i++) {
        if(strcmp(files[i].name, path + 1) == 0) {
            files[i].size -= offset;
            files[i].data = (char *)(realloc(files[i].data, files[i].size - offset));
            RETURN(0);
        }
    }
    RETURN(-ENOENT);
}

int MyFS::fuseCreate(const char *path, mode_t mode, struct fuse_file_info *fileInfo) {
    LOGM();
    
    // TODO: Implement this!
    
    RETURN(0);
}

void MyFS::fuseDestroy() {
    LOGM();
    for (int i = 0; i < NUM_DIR_ENTRIES; i++) {
        free(files[i].data);
        files[i].size= 0;
    }
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
        
        // you can get the containfer file name here:
        LOGF("Container file name: %s", ((MyFsInfo *) fuse_get_context()->private_data)->contFile);
        
        // TODO: Implement your initialization methods here!
        for (int i = 0; i < NUM_DIR_ENTRIES; i++) {
            files[i].size = 0;
            //Evlt aendern auf dynamic cast
            files[i].data = (char *)malloc(sizeof(char));
            files[i].data[0] = '\0';
            files[i].name[0] = '\0';
        }

        for (int j = 0; j < NUM_OPEN_FILES; j++) {
            fileHandles[j] = -1;
        }
    }
    
    RETURN(0);
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
        
// TODO: Add your own additional methods here!

