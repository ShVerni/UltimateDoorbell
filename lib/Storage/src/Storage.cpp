#include "Storage.h"
bool Storage::useLittleFS = false;

/// @brief Mount and initiate the storage. Will format if necessary
/// @param clk The clock pin number
/// @param cmd The cmd pin number
/// @param d0 D0 pin number
/// @param d1 D1 pin number
/// @param d2 D2 pin number
/// @param d3 D3 pin number
/// @return True on success
bool Storage::begin(int clk, int cmd, int d0, int d1, int d2, int d3) {
	bool success = SD_MMC.setPins(clk, cmd, d0, d1, d2, d3);
	if (success) {
		Serial.println("Mounting storage...");
		if (!SD_MMC.begin("/sd", false, true, 10000)) {
			Serial.println("Card Mount Failed");
			success = false;
		}
		if (success) {
			uint8_t cardType = SD_MMC.cardType();

			if(cardType == CARD_NONE) {
				Serial.println("No SD_MMC card attached");
				success = false;
			}
			if (success) {
				Serial.print("SD_MMC Card Type: ");
				if (cardType == CARD_MMC) {
					Serial.println("MMC");
				} else if(cardType == CARD_SD) {
					Serial.println("SDSC");
				} else if(cardType == CARD_SDHC) {
					Serial.println("SDHC");
				} else {
					Serial.println("UNKNOWN");
				}
				uint64_t cardSize = SD_MMC.cardSize() / 1048576; // 1024 * 1024
				Serial.printf("SD_MMC Card Size: %lluMB\n", cardSize);
			}
		}
	}
	return success;
}

/// @brief Mount LittleFS and format if necessary
/// @return True on successful mount of LittleFS
bool Storage::begin() {
	Storage::useLittleFS = true;
	Serial.println("Mounting  LittleFS, this could take a while, please wait...");
	return LittleFS.begin(true, "/sd");
}

/// @brief List the files and folders in a directory
/// @param dirname The directory path to list
/// @param levels How many levels to recurse into the directory for listing
/// @return A collection of strings of paths of the files/folders found
std::vector<String> Storage::listDir(String dirname, uint8_t levels) {
	Serial.println("Listing directory: " + dirname);
	std::vector<String> folderContents;
	File root;
	if(useLittleFS)
		root = LittleFS.open(dirname);
	else
		root = SD_MMC.open(dirname);

	if(!root){
		Serial.println("Failed to open directory");
		return folderContents;
	}
	if (!root.isDirectory()) {
		Serial.println("Not a directory");
		return folderContents;
	}
	File file = root.openNextFile();
	while(file) {
		if(file.isDirectory()){
			Serial.print("  DIR : ");
			Serial.println(file.name());
			if(levels){
				listDir(file.path(), levels - 1);
			}
		} else {
			folderContents.insert(folderContents.end(), String(file.path()));
			Serial.println(file.name());
		}
		file = root.openNextFile();
	}
	return folderContents;
}

/// @brief Checks if a file or directory exists on the storage
/// @param path The path of the file or directory
/// @return True if it exists
bool Storage::fileExists(String path) {
	Serial.println("Checking for file: " + path);
	if(useLittleFS)
		return LittleFS.exists(path);
	else
		return SD_MMC.exists(path);

}

/// @brief Creates a directory on the storage
/// @param path The path of the directory to create
/// @return True on success
bool Storage::createDir(String path) {
	Serial.println("Creating Dir: " + path);
	if(useLittleFS)
		return LittleFS.mkdir(path);
	else
		return SD_MMC.mkdir(path);
}

/// @brief Removes a directory from the storage
/// @param path The path of the directory to remove
/// @return True on success
bool Storage::removeDir(String path) {
	Serial.println("Removing Dir:" + path);
	return rmdir(path.c_str());
}

/// @brief Reads the contents of a file from the storage
/// @param path The path of the file to read
/// @return A String of the file contents, empty string on failure
String Storage::readFile(String path) {
	Serial.println("Reading file: " + path);
	File file;
	if(useLittleFS)
		file = LittleFS.open(path);
	else
		file = SD_MMC.open(path);

	if (!file) {
		Serial.println("Failed to open file for reading");
		return "";
	}
	String output = "";
	while(file.available()){
		output += file.readString();
	}
	file.close();
	return output;
}

/// @brief Writes data to a file, creates a file if necessary
/// @param path The path of the file to write
/// @param content The content of the file to write
/// @return True on success
bool Storage::writeFile(String path, String content) {
	Serial.println("Writing file: " + path);
	File file;
	if(useLittleFS)
		file = LittleFS.open(path, FILE_WRITE);
	else
		file = SD_MMC.open(path, FILE_WRITE);
	if (!file) {
		Serial.println("Failed to open file for writing");
		return false;
	}
	return file.print(content) > 0;
}

/// @brief Appends data to a file
/// @param path The path of the file to append
/// @param content The content to append
/// @return True on success
bool Storage::appendFile(String path, String content) {
	Serial.println("Appending to file: " + path);
	File file;
	if(useLittleFS)
		file = LittleFS.open(path, FILE_APPEND);
	else
		file = SD_MMC.open(path, FILE_APPEND);
	if (!file) {
		Serial.println("Failed to open file for appending");
		return false;
	}
	return file.print(content) > 0;
}

/// @brief Renames/moves a file on the storage
/// @param path1 The original path/name of the file
/// @param path2 The new path/name of the file
/// @return True on success
bool Storage::renameFile(String path1, String path2) {
	Serial.println("Renaming file" + path1 + " to " + path2);
	if(useLittleFS)
		return LittleFS.rename(path1, path2);
	else
		return SD_MMC.rename(path1, path2);
}

/// @brief Deletes a file from the storage
/// @param path The path of the file to delete
/// @return True on success
bool Storage::deleteFile(String path) {
	Serial.println("Deleting file: " + path);
	if(useLittleFS)
		return LittleFS.remove(path);
	else
		return SD_MMC.remove(path);
}