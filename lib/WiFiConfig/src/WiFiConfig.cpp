#include "WiFiConfig.h"

/// @brief Connects to a saved WiFi network, or configures WiFi.
/// @param WiFiManager The WifManager object to use
/// @param ring The LEDRing object to use
WiFiConfig::WiFiConfig (AsyncWiFiManager* WiFiManager, LEDRing* ring) {
	wifiManager = WiFiManager;
	led = ring;
}

/// @brief Callback notifying that the access point has started
/// @param myWiFiManager the AsyncWiFiManager making the call
void WiFiConfig::configModeCallback(AsyncWiFiManager *myWiFiManager)
{
	Serial.println("Access point started");
	neopixelWrite(RGB_DATA, 0, 0, 32);
	led->AddEventToQueue(LEDRing::Events::WIFI_CONFIG_START);
}

/// @brief Callback notifying that new settings were saved and connection successful
/// @param myWiFiManager the AsyncWiFiManager making the call
void WiFiConfig::configModeEndCallback(AsyncWiFiManager *myWiFiManager)
{
	Serial.println("Access point started");
	Serial.print("IP address: ");
	Serial.println(WiFi.localIP());
	led->AddEventToQueue(LEDRing::Events::WIFI_CONFIG_END);
}

/// @brief Attempts to connect to Wi-Fi network
void WiFiConfig::connectWiFi() {    
	// Set entered AP mode callback
	wifiManager->setAPCallback(std::bind(&WiFiConfig::configModeCallback, this, wifiManager));
	wifiManager->setSaveConfigCallback(std::bind(&WiFiConfig::configModeEndCallback, this, wifiManager));
	// Fetches ssid and password and tries to connect
	// if it does not connect it starts an access point with the specified name
	// and goes into a blocking loop awaiting configuration
	if (!wifiManager->autoConnect("UltimateBell")) {
		Serial.println("Failed to connect, we should reset as see if it connects");
		delay(3000);
		ESP.restart();
	}
}