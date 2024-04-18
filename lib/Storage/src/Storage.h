/*
 * This file and associated .cpp file are licensed under the GPLv3 License Copyright (c) 2024 Sam Groveman
 * 
 * Adapted from: https://docs.espressif.com/projects/arduino-esp32/en/latest/api/sdmmc.html
 * 
 * Contributors: Sam Groveman
 */

#pragma once
#include <FS.h>
#include <SD_MMC.h>
#include <LittleFS.h>
#include <vector>

class Storage {
	public:
		bool begin(int clk, int cmd, int d0, int d1, int d2, int d3);
		bool begin();
		/// @brief Checks if the current storage media is LittleFS or SD_MMC
		/// @return True if LittleFS is being used
		static bool isUsingLittleFS() { return useLittleFS; }
		std::vector<String> listDir(String dirname, uint8_t levels);
		bool fileExists(String path);
		bool createDir(String path);
		bool removeDir(String path);
		String readFile(String path);
		bool writeFile(String path, String content);
		bool appendFile(String path, String content);
		bool renameFile(String path1, String path2);
		bool deleteFile(String path);
		
	private:
		/// @brief Static variable storing what storage medium is being used
		static bool useLittleFS;
};