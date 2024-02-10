/*
 * This file and associated .cpp file are licensed under the GPLv3 License Copyright (c) 2024 Sam Groveman
 * 
 * External libraries needed:
 * ESP32-audioI2S : https://github.com/schreibfaul1/ESP32-audioI2S
 * ArduinoJSON: https://arduinojson.org/
 * 
 * Contributors: Sam Groveman
 */

#pragma once
#include <Audio.h>
#include <vector>
#include <SDCard.h>
#include <ArduinoJson.h>
#include <LEDRing.h>
#include <Webhooks.h>
#include <SD_MMC.h>

class SoundPlayer {
    public:
        SoundPlayer(SDCard* card, String Settings_file);
        bool begin(int I2S_BCLK, int I2S_LRC, int I2S_DOUT);
        void callLoop();
        String playChimeSound();
        bool playChimeSound(String sound);
        bool isPlaying();
        bool loadSettings();
        String getSettings();
        bool saveSettings();
        bool updateSettings(String settings);
        
    private:
        /// @brief The audio player object
        Audio player;

        /// @brief Reference to the SDCard object
        SDCard* card;

        /// @brief Collection of audio files that can be played
        std::vector<String> _files;

        /// @brief Path to settings file
        String settings_file;

        bool playFile(String file);
};