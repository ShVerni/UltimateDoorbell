/*
 * This file and associated .cpp file are licensed under the MIT Lesser General Public License Copyright (c) 2024 Sam Groveman
 * 
 * External libraries needed: https://github.com/alanswx/ESPAsyncWiFiManager
 * 
 * Contributors: Sam Groveman
 */
#pragma once
#include <ESPAsyncWiFiManager.h>
#include <ArduinoJson.h>
#include <LEDRing.h>

class WiFiConfig {
    public:
        WiFiConfig(AsyncWiFiManager* WiFiManager, LEDRing* ring);
        void connectWiFi();

    private:
        bool shouldSaveConfig;
        AsyncWiFiManager* wifiManager;
        LEDRing* led;
        void configModeCallback(AsyncWiFiManager *myWiFiManager);
        void configModeEndCallback(AsyncWiFiManager *myWiFiManager);
};