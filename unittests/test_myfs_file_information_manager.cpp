//
// Created by ahlex on 01.12.2019.
//

#include <unistd.h>
#include <cstring>
#include "catch.hpp"
#include "myfs-file-information-manager.h"

TEST_CASE("INITIALIZE FILE INFORMATIONS", "[myfs_file_information_manager]") {
    SECTION("Initialize file information manager") {
        MyFsFileInformationManager fileInformationManager;
        auto *fileInformations = new MyFsFileInformation[NUM_DIR_ENTRIES];
        fileInformationManager.set(fileInformations);
        fileInformationManager.init();
        fileInformations = fileInformationManager.getFileInformations();

        bool hasValidStructure = true;
        for (int i = 0; i < NUM_DIR_ENTRIES - 1; i++) {
            auto fileInformation = fileInformations[i];
            if (fileInformation.size != IS_FREE || fileInformation.nlink != 1 || fileInformation.uid != geteuid() ||
                fileInformation.gid != getegid()) {
                hasValidStructure = false;
                break;
            }
        }

        auto currentDir = fileInformations[NUM_DIR_ENTRIES - 1];

        if (currentDir.nlink != 2 || currentDir.size != 0 || strcmp(currentDir.name, ".") != 0 ||
            currentDir.mode != (S_IFDIR | 0775)) {
            hasValidStructure = false;
        }

        REQUIRE(hasValidStructure);
    }
}
TEST_CASE("GET FILE DESCRIPTOR", "[myfs_file_information_manager]") {
    SECTION("Get file descriptor") {
        MyFsFileInformationManager fileInformationManager;
        auto *fileInformations = new MyFsFileInformation[NUM_DIR_ENTRIES];
        fileInformationManager.set(fileInformations);
        fileInformationManager.init();
        fileInformations = fileInformationManager.getFileInformations();

        const int firstIndex = 0;
        const char* fileName = "test";
        fileInformations[firstIndex].size = 0;
        strcpy(fileInformations[firstIndex].name, fileName);
        fileInformationManager.set(fileInformations);

        int index = fileInformationManager.getFileDescriptor(fileName);
        REQUIRE(firstIndex == index);
    }
}
TEST_CASE("FILE INFORMATION EXISTS", "[myfs_file_information_manager]") {
    SECTION("File information exists for file name") {

    }
    SECTION("File information exists for file descriptor") {

    }
}