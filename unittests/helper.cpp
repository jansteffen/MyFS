//
//  helper.cpp
//  testing
//
//  Created by Oliver Waldhorst on 15.12.17.
//  Copyright © 2017 Oliver Waldhorst. All rights reserved.
//

#include <cstdlib>
#include <string.h>

#include "catch.hpp"
#include "helper.hpp"

#define RW 0666
#define DEVICE_NUMBERS 0
#define OK 0

void gen_random(char *s, const int len) {
    static const char alphanum[] =
    "0123456789"
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz";
    
    for (int i = 0; i < len; ++i) {
        s[i] = alphanum[rand() % (sizeof(alphanum) - 1)];
    }
}

void create(MyFS* myfs, const char* fileName) {
    REQUIRE(myfs->fuseMknod(fileName, RW, DEVICE_NUMBERS) == OK);
}
void open(MyFS* myfs, fuse_file_info fileInfo, const char* fileName) {
    REQUIRE(myfs->fuseOpen(fileName, &fileInfo) == OK);
}
void read(MyFS* myfs, fuse_file_info fileInfo, const char* fileName, char* readBuf, size_t size, off_t offset) {
    REQUIRE(myfs->fuseRead(fileName, readBuf, size, offset,  &fileInfo) == size);
}
void write(MyFS* myfs, fuse_file_info fileInfo, const char* fileName, char* writeBuf, size_t size, off_t offset) {
    REQUIRE(myfs->fuseWrite(fileName, writeBuf, size, offset, &fileInfo) == size);
}
void close(MyFS* myfs, fuse_file_info fileInfo, const char* fileName) {
    REQUIRE(myfs->fuseRelease(fileName, &fileInfo) == OK);
}
void open_write_close(MyFS* myfs, fuse_file_info fileInfo, const char* fileName, char* writeBuf, size_t size, off_t offset) {
    open(myfs, fileInfo, fileName);
    write(myfs, fileInfo, fileName, writeBuf, size, offset);
    close(myfs, fileInfo, fileName);
}
void open_read_close(MyFS* myfs, fuse_file_info fileInfo, const char* fileName, char* readBuf, size_t size, off_t offset) {
    open(myfs, fileInfo, fileName);
    read(myfs, fileInfo, fileName, readBuf, size, offset);
    close(myfs, fileInfo, fileName);
}

void truncate(MyFS* myfs, const char* fileName, off_t newSize) {
    REQUIRE(myfs->fuseTruncate(fileName, newSize) == OK);
}

void bdWriteRead(BlockDevice *bd, int noBlocks) {
    char* r= new char[BD_BLOCK_SIZE * noBlocks];
    memset(r, 0, BD_BLOCK_SIZE * noBlocks);

    char* w= new char[BD_BLOCK_SIZE * noBlocks];
    gen_random(w, BD_BLOCK_SIZE * noBlocks);

    // write all blocks
    for(int b= 0; b < noBlocks; b++) {
        REQUIRE(bd->write(b, w + b*BD_BLOCK_SIZE) == 0);
    }

    // read all blocks
    for(int b= 0; b < noBlocks; b++) {
        REQUIRE(bd->read(b, r + b*BD_BLOCK_SIZE) == 0);
    }

    REQUIRE(memcmp(w, r, BD_BLOCK_SIZE * noBlocks) == 0);

    delete [] r;
    delete [] w;
}

// TODO: Implement you helper functions here
