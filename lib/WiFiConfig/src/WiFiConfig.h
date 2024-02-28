/*
 * This file and associated .cpp file are licensed under the GPLv3 License Copyright (c) 2024 Sam Groveman
 * 
 * External libraries needed:
 * ESPAsyncWiFiManager: https://github.com/alanswx/ESPAsyncWiFiManager
 * ArduinoJSON: https://arduinojson.org/
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
        AsyncWiFiManager* wifiManager;
        LEDRing* led;
        void configModeCallback(AsyncWiFiManager *myWiFiManager);
        void configModeEndCallback(AsyncWiFiManager *myWiFiManager);
};