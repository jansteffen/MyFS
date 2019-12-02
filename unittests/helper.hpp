//
//  helper.hpp
//  testing
//
//  Created by Oliver Waldhorst on 15.12.17.
//  Copyright © 2017 Oliver Waldhorst. All rights reserved.
//

#ifndef helper_hpp
#define helper_hpp

#include "myfs.h"
#include "blockdevice.h"

void create(MyFS* myfs, const char* fileName);
void open(MyFS* myfs, fuse_file_info fileInfo, const char* fileName);
void read(MyFS* myfs, fuse_file_info fileInfo, const char* fileName, char* readBuf, size_t size, off_t offset);
void write(MyFS* myfs, fuse_file_info fileInfo, const char* fileName, char* writeBuf, size_t size, off_t offset);
void close(MyFS* myfs, fuse_file_info fileInfo, const char* fileName);
void open_write_close(MyFS* myfs, fuse_file_info fileInfo, const char* fileName, char* writeBuf, size_t size, off_t offset);
void open_read_close(MyFS* myfs, fuse_file_info fileInfo, const char* fileName, char* readBuf, size_t size, off_t offset);
void truncate(MyFS* myfs, const char* fileName, off_t newSize);

void gen_random(char *s, const int len);
void bdWriteRead(BlockDevice *bd, int noBlocks= 1);

#endif /* helper_hpp */
