/*
 * This file and associated .cpp file are licensed under the MIT Lesser General Public License Copyright (c) 2024 Sam Groveman
 * 
 * Adapted from: https://docs.espressif.com/projects/arduino-esp32/en/latest/api/sdmmc.html
 * 
 * Contributors: Sam Groveman
 */

#pragma once
#include <FS.h>
#include <SD_MMC.h>
#include <vector>

class SDCard {
    public:
        bool begin(int clk, int cmd, int d0, int d1, int d2, int d3);
        std::vector<String> listDir(String dirname, uint8_t levels);
        bool fileExists(String path);
        bool createDir(String path);
        bool removeDir(String path);
        String readFile(String path);
        bool writeFile(String path, String content);
        bool appendFile(String path, String content);
        bool renameFile(String path1, String path2);
        bool deleteFile(String path);
};