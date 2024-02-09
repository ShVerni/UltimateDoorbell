#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <ESPAsyncWiFiManager.h>
#include <WebServer.h>
#include <WiFiConfig.h>
#include <SDCard.h>
#include <LEDRing.h>
#include <Webhooks.h>
#include <SoundPlayer.h>
#include <UMS3.h>

/// @brief Doorbell button pin number
#define BUTTON_PIN 2

/// @brief TinyS3 helper
UMS3 ums3;

/// @brief Set true while the bell is ringing
bool ringing = false;

/// @brief AsyncWebServer object (passed to WfiFiConfig and WebServer)
AsyncWebServer server(80);

/// @brief SDCard object 
SDCard card;

/// @brief LED ring
LEDRing leds(&card, "/settings/animations.json");

/// @brief Contains webhooks to call on ring
Webhooks hooks(&card, "/settings/webhooks.json");

/// @brief Player for ringer sounds
SoundPlayer player(&card, "/settings/audio_settings.json");

/// @brief Webserver handling all requests, needs access to all data
Webserver webserver(&server, &leds, &player, &card, &hooks, &ringing);

// put function declarations here:
void IRAM_ATTR RING_ISR();

void setup() {
    ums3.begin();
    Serial.begin(115200);
    digitalWrite(RGB_PWR, HIGH);

    // Show red light for start up
    neopixelWrite(RGB_DATA, 128, 0, 0);
    delay(2000);

    // Configure button pin
    pinMode(BUTTON_PIN, INPUT_PULLUP);

    // Start LEDs
    Serial.println("Starting LEDs");
    leds.begin();

    // Start event processor loop
    xTaskCreate(LEDRing::ProcessEventTaskWrapper, "Event Processor Loop", 2000, &leds, 1, NULL);

    Serial.print("PSRAM: ");
    Serial.println(ESP.getPsramSize());
    delay(250);
    // Mount and start the SD card: clk, cmd, d0, d1, d2, d3
    if (!card.begin(8, 34, 9, 37, 35, 36)) {
        Serial.println("Could not connect to SD card, aborting.");
        leds.AddEventToQueue(LEDRing::Events::SD_ERROR);
        while(true) {delay(500);}
    }

    // Create the settings directory if needed
    if (!card.fileExists("/settings")) {
        if (!card.createDir("/settings")) {
            Serial.println("Could not create settings directory, aborting.");
            leds.AddEventToQueue(LEDRing::Events::SD_ERROR);
            while(true) {delay(500);}
        }
    }

    // Load saved animations now that SD card is ready
    leds.LoadAnimations();

    // Configure WiFi
    DNSServer dns;
    AsyncWiFiManager manager(&server, &dns);
    WiFiConfig configurator(&manager, &leds);
    configurator.connectWiFi();

    // Clear server settings just in case
    webserver.ServerStop();

    // Start the update server
    webserver.ServerStart();
    xTaskCreate(Webserver::RebootCheckerTaskWrapper, "Reboot Checker Loop", 1000, &webserver, 1, NULL);

    if (!player.begin(5, 4, 21)) {
        Serial.println("Could not initialize audio device, aborting.");
        leds.AddEventToQueue(LEDRing::Events::I2S_PLAYER_ERROR);
        while(true) {delay(500);}
    }

    // Load audio player settings
    if (!player.loadSettings()) {
        Serial.println("Could not load audio device settings, aborting.");
        leds.AddEventToQueue(LEDRing::Events::I2S_PLAYER_ERROR);
        while(true) {delay(500);}
    }

    // Load webhooks
    if (!hooks.LoadSettings()) {
        Serial.println("Could not load webhook settings, aborting.");
        leds.AddEventToQueue(LEDRing::Events::WEBHOOK_ERROR);
        while(true) {delay(500);}
    }
    // Start webhook task
    xTaskCreate(Webhooks::ProcessEventTaskWrapper, "Webhook Processor Loop", 4000, &hooks, 1, NULL);

    // Attach interrupt handler
    attachInterrupt(BUTTON_PIN, RING_ISR, FALLING);
    Serial.println("Ready!");
    leds.AddEventToQueue(LEDRing::Events::DOORBELL_READY);

    // Show green LED
    neopixelWrite(RGB_DATA, 0, 64, 0);
    delay(1000); // Delay 1 second to allow serial monitor to connect for debugging
    digitalWrite(RGB_PWR, LOW);
}

void loop() {
    // Maintenance call to sound player loop
    player.callLoop();
    // Check if bell should be ringing
    if (ringing) { // || !digitalRead(BUTTON_PIN)
        // Check if button has been pushed for a sufficient amount of time (prevents false positives)
        delay(25);
        if (!digitalRead(BUTTON_PIN)) {
            // Check if bell is not already ringing
            if (!player.isPlaying()) {
                String file = player.playChimeSound();
                leds.AddEventToQueue(LEDRing::Events::BELL_RING_START, file.substring(file.lastIndexOf('/') + 1, file.indexOf('.')));
                hooks.AddEventToQueue(LEDRing::Events::BELL_RING_START, file);
            }
            // Wait for sound to finish playing
            do {
                player.callLoop();
            } while (player.isPlaying());
            leds.AddEventToQueue(LEDRing::Events::BELL_RING_END);
            hooks.AddEventToQueue(LEDRing::Events::BELL_RING_END);
            ringing = false;
        } else {
            // False positive
            ringing = false;
        }
    }
}

// put function definitions here:

/// @brief Interrupt service routine for doorbell button pushed 
void IRAM_ATTR RING_ISR() {
    if (ringing)
        return;
    ringing = true;
}