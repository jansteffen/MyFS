//
//  myfs-structs.h
//  myfs
//
//  Created by Oliver Waldhorst on 07.09.17.
//  Copyright © 2017 Oliver Waldhorst. All rights reserved.
//

#ifndef myfs_structs_h
#define myfs_structs_h

#include <sys/stat.h>
#include <sys/types.h>
#include <cstdint>

#include "myfs-constants.h"

typedef struct {
    char name[NAME_LENGTH];
    off_t size; // This specifies the size of a regular file in bytes. For files that are really devices this field isn’t usually meaningful. For symbolic links this specifies the length of the file name the link refers to.
    uid_t uid; // The user ID of the file’s owner.
    gid_t gid; // The group ID of the file.
    time_t atime; // This is the last access time for the file.
    time_t mtime; // This is the time of the last modification to the contents of the file.
    time_t ctime; // last change
    char *  data;
    mode_t mode; // Specifies the mode of the file. This includes file type information (see Testing File Type) and the file permission bits (see Permission Bits).
    nlink_t nlink; // The number of hard links to the file. This count keeps track of how many directories have entries for this file. If the count is ever decremented to zero, then the file itself is discarded as soon
                   // as no process still holds it open. Symbolic links are not counted in the total.
} MyFsFileInformation;

typedef struct {
    int fileDescriptor;
} MyFsOpenFileHandle;

#endif /* myfs_structs_h */
