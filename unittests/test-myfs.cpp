//
//  test-myfs.cpp
//  testing
//
//  Created by Oliver Waldhorst on 15.12.17.
//  Copyright © 2017 Oliver Waldhorst. All rights reserved.
//

#include "catch.hpp"
#include "helper.hpp"
#include "myfs.h"
#include <cstring>
#include <iostream>

#define TEST_FILE "Testfile"
#define TEST_FILE2 "Testfile2"
#define TEST_FILE3 "Testfile3"
#define SMALL_FILE_SIZE 500
#define MEDIUM_FILE_SIZE 1500
#define LARGE_FILE_SIZE 15000
#define VERY_LARGE_FILE_SIZE 20971520

TEST_CASE("Write and read a file", "[MyFS]") {
    MyFS* myfs = new MyFS(true);
    fuse_file_info fileInfo = {};
    myfs->initInMemory();

    SECTION("Small file (500 byte)") {
        create(myfs, TEST_FILE);

        char writeBuf[SMALL_FILE_SIZE];
        gen_random(writeBuf, SMALL_FILE_SIZE);
        open_write_close(myfs, fileInfo, TEST_FILE, writeBuf, SMALL_FILE_SIZE, 0);

        char readBuf[SMALL_FILE_SIZE];
        open_read_close(myfs, fileInfo, TEST_FILE, readBuf, SMALL_FILE_SIZE, 0);

        REQUIRE(memcmp(readBuf, writeBuf, SMALL_FILE_SIZE) == 0);
    }

    SECTION("Medium file (1500 byte)") {
        create(myfs, TEST_FILE);

        char writeBuf[MEDIUM_FILE_SIZE];
        gen_random(writeBuf, MEDIUM_FILE_SIZE);
        open_write_close(myfs, fileInfo, TEST_FILE, writeBuf, MEDIUM_FILE_SIZE, 0);

        char readBuf[MEDIUM_FILE_SIZE];
        open_read_close(myfs, fileInfo, TEST_FILE, readBuf, MEDIUM_FILE_SIZE, 0);

        REQUIRE(memcmp(readBuf, writeBuf, MEDIUM_FILE_SIZE) == 0);
    }

    SECTION("Large file (15000 byte)") {
        create(myfs, TEST_FILE);

        char writeBuf[LARGE_FILE_SIZE];
        gen_random(writeBuf, LARGE_FILE_SIZE);
        open_write_close(myfs, fileInfo, TEST_FILE, writeBuf, LARGE_FILE_SIZE, 0);

        char readBuf[LARGE_FILE_SIZE];
        open_read_close(myfs, fileInfo, TEST_FILE, readBuf, LARGE_FILE_SIZE, 0);

        REQUIRE(memcmp(readBuf, writeBuf, LARGE_FILE_SIZE) == 0);
    }

    SECTION("Very Large file (20971520 byte)") {
        create(myfs, TEST_FILE);

        char* writeBuf = (char*) malloc(VERY_LARGE_FILE_SIZE * sizeof(char));
        gen_random(writeBuf, VERY_LARGE_FILE_SIZE);
        open_write_close(myfs, fileInfo, TEST_FILE, writeBuf, VERY_LARGE_FILE_SIZE, 0);

        char* readBuf = (char*) malloc(VERY_LARGE_FILE_SIZE * sizeof(char));
        open_read_close(myfs, fileInfo, TEST_FILE, readBuf, VERY_LARGE_FILE_SIZE, 0);

        REQUIRE(memcmp(readBuf, writeBuf, VERY_LARGE_FILE_SIZE) == 0);

        free(readBuf);
        free(writeBuf);
    }
}

TEST_CASE("Write and read a file without closing", "[MyFS]") {
    MyFS* myfs = new MyFS(true);
    fuse_file_info fileInfo = {};
    myfs->initInMemory();

    SECTION("Small file (500 byte)") {
        create(myfs, TEST_FILE);

        char writeBuf[SMALL_FILE_SIZE];
        gen_random(writeBuf, SMALL_FILE_SIZE);
        char readBuf[SMALL_FILE_SIZE];
        open(myfs, fileInfo, TEST_FILE);
        write(myfs, fileInfo, TEST_FILE, writeBuf, SMALL_FILE_SIZE, 0);
        read(myfs, fileInfo, TEST_FILE, readBuf, SMALL_FILE_SIZE, 0);

        REQUIRE(memcmp(readBuf, writeBuf, SMALL_FILE_SIZE) == 0);
    }

    SECTION("Medium file (1500 byte)") {
        create(myfs, TEST_FILE);

        char writeBuf[MEDIUM_FILE_SIZE];
        gen_random(writeBuf, MEDIUM_FILE_SIZE);
        char readBuf[MEDIUM_FILE_SIZE];
        open(myfs, fileInfo, TEST_FILE);
        write(myfs, fileInfo, TEST_FILE, writeBuf, MEDIUM_FILE_SIZE, 0);
        read(myfs, fileInfo, TEST_FILE, readBuf, MEDIUM_FILE_SIZE, 0);

        REQUIRE(memcmp(readBuf, writeBuf, MEDIUM_FILE_SIZE) == 0);
    }

    SECTION("Large file (15000 byte)") {
        create(myfs, TEST_FILE);

        char writeBuf[LARGE_FILE_SIZE];
        gen_random(writeBuf, LARGE_FILE_SIZE);
        char readBuf[LARGE_FILE_SIZE];
        open(myfs, fileInfo, TEST_FILE);
        write(myfs, fileInfo, TEST_FILE, writeBuf, LARGE_FILE_SIZE, 0);
        read(myfs, fileInfo, TEST_FILE, readBuf, LARGE_FILE_SIZE, 0);

        REQUIRE(memcmp(readBuf, writeBuf, LARGE_FILE_SIZE) == 0);
    }
}

TEST_CASE("Truncate a file", "[MyFS]") {
    MyFS *myfs = new MyFS(true);
    fuse_file_info fileInfo = {};
    myfs->initInMemory();

    SECTION("Small file (500 byte)") {
        create(myfs, TEST_FILE);
        const int truncatedSize = 100;
        char writeBuf[SMALL_FILE_SIZE];
        gen_random(writeBuf, SMALL_FILE_SIZE);
        char readBuf[truncatedSize];
        open_write_close(myfs, fileInfo, TEST_FILE, writeBuf, SMALL_FILE_SIZE, 0);
        truncate(myfs, TEST_FILE, truncatedSize);
        open_read_close(myfs, fileInfo, TEST_FILE, readBuf, truncatedSize, 0);
        char truncatedBuf[truncatedSize];
        memcpy(truncatedBuf, writeBuf, truncatedSize);

        REQUIRE(memcmp(readBuf, truncatedBuf, truncatedSize) == 0);
    }
}

TEST_CASE("Overwrite a file", "[MyFS]") {
    MyFS *myfs = new MyFS(true);
    fuse_file_info fileInfo = {};
    myfs->initInMemory();

    SECTION("Small file (500 byte)") {
        create(myfs, TEST_FILE);
        char writeBuf[SMALL_FILE_SIZE];
        gen_random(writeBuf, SMALL_FILE_SIZE);
        open_write_close(myfs, fileInfo, TEST_FILE, writeBuf, SMALL_FILE_SIZE, 0);

        char randomBuf[SMALL_FILE_SIZE];
        gen_random(randomBuf, SMALL_FILE_SIZE);
        open_write_close(myfs, fileInfo, TEST_FILE, randomBuf, SMALL_FILE_SIZE, 0);

        char readBuf[SMALL_FILE_SIZE];
        open_read_close(myfs, fileInfo, TEST_FILE, readBuf, SMALL_FILE_SIZE, 0);

        REQUIRE(memcmp(readBuf, randomBuf, SMALL_FILE_SIZE) == 0);
    }
}

TEST_CASE("Overwrite parts of a file", "[MyFS]") {
    MyFS *myfs = new MyFS(true);
    fuse_file_info fileInfo = {};
    myfs->initInMemory();

    SECTION("Small file (500 byte)") {
        create(myfs, TEST_FILE);
        char writeBuf[SMALL_FILE_SIZE];
        gen_random(writeBuf, SMALL_FILE_SIZE);
        open_write_close(myfs, fileInfo, TEST_FILE, writeBuf, SMALL_FILE_SIZE, 0);

        const off_t offset = 50;
        const size_t overwriteSize = 100;
        char randomBuf[overwriteSize];
        gen_random(randomBuf, overwriteSize);
        memcpy(writeBuf + offset, randomBuf, overwriteSize);
        open_write_close(myfs, fileInfo, TEST_FILE, randomBuf, overwriteSize, offset);

        char readBuf[SMALL_FILE_SIZE];
        open_read_close(myfs, fileInfo, TEST_FILE, readBuf, SMALL_FILE_SIZE, 0);

        REQUIRE(memcmp(readBuf, writeBuf, SMALL_FILE_SIZE) == 0);
    }
}

TEST_CASE("Append to a file", "[MyFS]") {
    MyFS *myfs = new MyFS(true);
    fuse_file_info fileInfo = {};
    myfs->initInMemory();

    SECTION("Small file (500 byte)") {
        create(myfs, TEST_FILE);
        char writeBuf[SMALL_FILE_SIZE];
        gen_random(writeBuf, SMALL_FILE_SIZE);
        open_write_close(myfs, fileInfo, TEST_FILE, writeBuf, SMALL_FILE_SIZE, 0);

        const off_t offset = SMALL_FILE_SIZE;
        const size_t appendSize = SMALL_FILE_SIZE / 2;
        char randomBuf[appendSize];
        gen_random(randomBuf, appendSize);
        char resultBuf[SMALL_FILE_SIZE + appendSize];
        memcpy(resultBuf, writeBuf, SMALL_FILE_SIZE);
        memcpy(resultBuf + SMALL_FILE_SIZE, randomBuf, appendSize);
        open_write_close(myfs, fileInfo, TEST_FILE, randomBuf, appendSize, offset);

        char readBuf[SMALL_FILE_SIZE + appendSize];
        open_read_close(myfs, fileInfo, TEST_FILE, readBuf, SMALL_FILE_SIZE + appendSize, 0);

        REQUIRE(memcmp(readBuf, resultBuf, SMALL_FILE_SIZE + appendSize) == 0);
    }
}

TEST_CASE("Multiple writes at different positions", "[MyFS]") {
    MyFS *myfs = new MyFS(true);
    fuse_file_info fileInfo = {};
    myfs->initInMemory();

    SECTION("Small file (500 byte)") {
        create(myfs, TEST_FILE);
        char writeBuf[SMALL_FILE_SIZE];
        gen_random(writeBuf, SMALL_FILE_SIZE);
        open_write_close(myfs, fileInfo, TEST_FILE, writeBuf, SMALL_FILE_SIZE, 0);

        const off_t offset1 = 100;
        const size_t overwriteSize1 = 50;
        const off_t offset2 = 20;
        const size_t overwriteSize2 = 30;
        const off_t offset3 = 350;
        const size_t overwriteSize3 = 75;
        char randomBuf1[overwriteSize1];
        gen_random(randomBuf1, overwriteSize1);
        char randomBuf2[overwriteSize2];
        gen_random(randomBuf2, overwriteSize2);
        char randomBuf3[overwriteSize3];
        gen_random(randomBuf3, overwriteSize3);
        memcpy(writeBuf + offset1, randomBuf1, overwriteSize1);
        memcpy(writeBuf + offset2, randomBuf2, overwriteSize2);
        memcpy(writeBuf + offset3, randomBuf3, overwriteSize3);

        open(myfs, fileInfo, TEST_FILE);
        write(myfs, fileInfo, TEST_FILE, randomBuf1, overwriteSize1, offset1);
        write(myfs, fileInfo, TEST_FILE, randomBuf2, overwriteSize2, offset2);
        write(myfs, fileInfo, TEST_FILE, randomBuf3, overwriteSize3, offset3);
        close(myfs, fileInfo, TEST_FILE);

        char readBuf[SMALL_FILE_SIZE];
        open_read_close(myfs, fileInfo, TEST_FILE, readBuf, SMALL_FILE_SIZE, 0);

        REQUIRE(memcmp(readBuf, writeBuf, SMALL_FILE_SIZE) == 0);
    }
}

TEST_CASE("Write to multiple open files", "[MyFS]") {
    MyFS *myfs = new MyFS(true);
    fuse_file_info fileInfo = {};
    myfs->initInMemory();

    SECTION("Small files (500 byte)") {
        char writeBuf[SMALL_FILE_SIZE];
        gen_random(writeBuf, SMALL_FILE_SIZE);

        create(myfs, TEST_FILE);
        create(myfs, TEST_FILE2);
        create(myfs, TEST_FILE3);

        open(myfs, fileInfo, TEST_FILE);
        open(myfs, fileInfo, TEST_FILE2);
        open(myfs, fileInfo, TEST_FILE3);

        fileInfo.fh = 0;
        write(myfs, fileInfo, TEST_FILE, writeBuf, SMALL_FILE_SIZE, 0);
        fileInfo.fh = 1;
        write(myfs, fileInfo, TEST_FILE2, writeBuf, SMALL_FILE_SIZE, 0);
        fileInfo.fh = 2;
        write(myfs, fileInfo, TEST_FILE3, writeBuf, SMALL_FILE_SIZE, 0);

        char readBuf1[SMALL_FILE_SIZE];
        fileInfo.fh = 0;
        read(myfs, fileInfo, TEST_FILE, readBuf1, SMALL_FILE_SIZE, 0);
        char readBuf2[SMALL_FILE_SIZE];
        fileInfo.fh = 1;
        read(myfs, fileInfo, TEST_FILE, readBuf2, SMALL_FILE_SIZE, 0);
        char readBuf3[SMALL_FILE_SIZE];
        fileInfo.fh = 2;
        read(myfs, fileInfo, TEST_FILE, readBuf3, SMALL_FILE_SIZE, 0);

        fileInfo.fh = 0;
        close(myfs, fileInfo, TEST_FILE);
        fileInfo.fh = 1;
        close(myfs, fileInfo, TEST_FILE2);
        fileInfo.fh = 2;
        close(myfs, fileInfo, TEST_FILE3);

        REQUIRE(memcmp(readBuf1, writeBuf, SMALL_FILE_SIZE) == 0);
        REQUIRE(memcmp(readBuf2, writeBuf, SMALL_FILE_SIZE) == 0);
        REQUIRE(memcmp(readBuf3, writeBuf, SMALL_FILE_SIZE) == 0);
    }
}