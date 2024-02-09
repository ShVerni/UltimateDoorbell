#include "Webserver.h"

/// @brief Creates a Webserver object
/// @param webserver An AsyncWebServer object reference.
/// @param LEDs An LEDRing object
/// @param Player A SoundPlayer object
/// @param Card An SDCard object
/// @param Hooks A Webhook object
/// @param Ringing Reference to a bool that can be used to indicate the bell is ringing
Webserver::Webserver(AsyncWebServer* webserver, LEDRing* LEDs, SoundPlayer* Player, SDCard* Card, Webhooks* Hooks, bool* Ringing) {
    server = webserver;
    leds = LEDs;
    player = Player;
    card = Card;
    hooks = Hooks;
    ringing = Ringing;
}

/// @brief Starts the update server
bool Webserver::ServerStart() {
    Serial.println("Starting web server");

    // Create root directory if needed
    if (!card->fileExists("/www"))
        if (!card->createDir("/www"))
            return false;

    // Add request handler
    if (card->fileExists("/www/index.html")) {
        server->serveStatic("/", SD_MMC, "/www/").setDefaultFile("index.html");
    } else {
        server->on("/", HTTP_GET, [this](AsyncWebServerRequest *request) {
            request->send_P(HTTP_CODE_OK, "text/html", index_page);
        });
    }

    // Handle file uploads
    server->on("/upload-www", HTTP_POST, [](AsyncWebServerRequest *request) { request->send(HTTP_CODE_ACCEPTED); }, onUpload_www);
    server->on("/upload-settings", HTTP_POST, [](AsyncWebServerRequest *request) { request->send(HTTP_CODE_ACCEPTED); }, onUpload_settings);
    server->on("/upload-chimes", HTTP_POST, [](AsyncWebServerRequest *request) { request->send(HTTP_CODE_ACCEPTED); }, onUpload_chimes);

    // Retrieve sound settings
    server->on("/audioSettings", HTTP_GET, [this](AsyncWebServerRequest *request) {
        Serial.println("Getting audio settings");
        request->send(HTTP_CODE_OK, "text/json", player->getSettings());
    });

    // Saves the sound settings
    server->on("/audioSettings", HTTP_POST, [this](AsyncWebServerRequest *request) {
        Serial.println("Updating audio settings");
        if (request->hasParam("settings", true)) {
            String settings = request->getParam("settings", true)->value();
            if (player->updateSettings(settings)) {
                request->send(HTTP_CODE_OK);
                player->saveSettings();
            } else {
                request->send(HTTP_CODE_BAD_REQUEST, "text/plain", "Could not parse JSON.");
            }
        } else {
            request->send(HTTP_CODE_BAD_REQUEST, "text/plain", "New settings required.");
        }
    });

    // Retrieve webhook settings
    server->on("/webhookSettings", HTTP_GET, [this](AsyncWebServerRequest *request) {
        Serial.println("Getting webhook settings");
        request->send(HTTP_CODE_OK, "text/json", hooks->GetSettings());
    });

    // Saves the webhook settings
    server->on("/webhookSettings", HTTP_POST, [this](AsyncWebServerRequest *request) {
        Serial.println("Updating webhook settings");
        if (request->hasParam("settings", true)) {
            String settings = request->getParam("settings", true)->value();
            if (hooks->UpdateSettings(settings)) {
                request->send(HTTP_CODE_OK);
                hooks->SaveSettings();
            } else {
                request->send(HTTP_CODE_BAD_REQUEST, "text/plain", "Could not parse JSON.");
            }
        } else {
            request->send(HTTP_CODE_BAD_REQUEST, "text/plain", "New settings required.");
        }
    });

    // Retrieve animations
    server->on("/animationSettings", HTTP_GET, [this](AsyncWebServerRequest *request) {
        Serial.println("Getting LED animations");
        request->send(HTTP_CODE_OK, "text/json", leds->GetAnimations());
    });

    // Saves the animations
    server->on("/animationSettings", HTTP_POST, [this](AsyncWebServerRequest *request) {
        Serial.println("Updating LED animations");
        if (request->hasParam("settings", true)) {
            String settings = request->getParam("settings", true)->value();
            if (leds->UpdateAnimations(settings)) {
                request->send(HTTP_CODE_OK);
            } else {
                request->send(HTTP_CODE_BAD_REQUEST, "text/plain", "Bad animations data");
            }
        } else {
            request->send(HTTP_CODE_BAD_REQUEST, "text/plain", "New animations required.");
        }
    });

    // Play sound file
    server->on("/ring", HTTP_POST, [this](AsyncWebServerRequest *request) {
        Serial.println("Ringing bell from API");
        String sound = String();
        if (request->hasParam("sound", true))
            sound = request->getParam("sound", true)->value();
        if (!player->isPlaying()) {
            bool success;
            if (sound.isEmpty()) {
                sound = player->playChimeSound();
                success = !sound.isEmpty();
            } else { 
                success = player->playChimeSound(sound);
            }
            leds->AddEventToQueue(LEDRing::Events::BELL_RING_START, sound.substring(sound.lastIndexOf('/') + 1, sound.indexOf('.')));
            hooks->AddEventToQueue(LEDRing::Events::BELL_RING_START, sound);
            if (success) {
                *ringing = true;
                request->send(HTTP_CODE_OK);
            } else {
                request->send(HTTP_CODE_BAD_REQUEST, "text/plain", "Could not play file.");
            }
        } else {
            request->send(HTTP_CODE_SERVICE_UNAVAILABLE, "text/plain", "Already playing a sound");
        }
    });

    // Handle deletion of files
    server->on("/delete", HTTP_POST, [this](AsyncWebServerRequest *request) {
        if(request->hasParam("path", true)) {
            String path = request->getParam("path", true)->value();
            Serial.println("Deleting " + path);
            if (card->fileExists(path)) {
                bool success = card->deleteFile(path);
                request->send(HTTP_CODE_OK, "text/plain", success ? "OK" : "FAIL");
            } else {
                request->send(HTTP_CODE_BAD_REQUEST, "text/plain", "File doesn't exist");
            }
        } else {
            request->send(HTTP_CODE_BAD_REQUEST, "text/plain", "Bad request data");
        }
    });

    // Handle reset request
    server->on("/reset", HTTP_PUT, [this](AsyncWebServerRequest *request) {
        Serial.println("Resetting WiFi settings");
         if (card->fileExists("/www/reset.html")) {
            request->send(SD_MMC, "/www/reset.html", "text/html");
        } else {
            request->send(HTTP_CODE_OK, "text/plain", "OK");
        }
        WiFi.mode(WIFI_AP_STA); // cannot erase if not in STA mode !
        WiFi.persistent(true);
        WiFi.disconnect(true, true);
        WiFi.persistent(false);
        shouldReboot = true;
    });

    // Handle reboot request
    server->on("/reboot", HTTP_PUT, [this](AsyncWebServerRequest *request) {
        if (card->fileExists("/www/reboot.html")) {
            request->send(SD_MMC, "/www/reboot.html", "text/html");
        } else {
            request->send(HTTP_CODE_OK, "text/plain", "OK");
        }
        this->shouldReboot = true;
    });

    // Handle listing files
    server->on("/list", HTTP_GET, [this](AsyncWebServerRequest *request) {
        if (request->hasParam("path")) {
            String path = request->getParam("path")->value();
            if (card->fileExists(path)) {
                std::vector<String> file_list = card->listDir(path, 0);
                DynamicJsonDocument files(2048);
                for (int i = 0; i < file_list.size(); i++) {
                    files["files"][i] = file_list[i];
                }
                files.shrinkToFit();
                String files_string;
                serializeJson(files, files_string);
                request->send(HTTP_CODE_OK, "text/json", files_string);
            } else {
                request->send(HTTP_CODE_BAD_REQUEST, "text/plain", "Folder doesn't exist");
            }
        } else {
            request->send(HTTP_CODE_BAD_REQUEST, "text/plain", "Bad request data");
        }
    });

    // Handle downloads
    server->on("/download", HTTP_GET, [this](AsyncWebServerRequest *request) {
        if (request->hasParam("path")) {
            String path = request->getParam("path")->value();
            if (SD_MMC.exists(path)) {
                request->send(SD_MMC, path, "application/octet-stream");
            } else {
                request->send(HTTP_CODE_BAD_REQUEST, "text/plain", "File doesn't exist");
            }
        } else {
            request->send(HTTP_CODE_BAD_REQUEST, "text/plain", "Bad request data");
        }
    });

    // Update page is special and hard-coded to always be available
    server->on("/update", HTTP_GET, [this](AsyncWebServerRequest *request) {
        request->send_P(HTTP_CODE_OK, "text/html", update_page);
    });

    // Update firmware
    server->on("/update", HTTP_POST, [this](AsyncWebServerRequest *request) {
        delay(50); // Let update start
        shouldReboot = !Update.hasError();
        if (shouldReboot) {
            leds->AddEventToQueue(LEDRing::Events::UPDATED);
        }
        AsyncWebServerResponse *response = request->beginResponse(HTTP_CODE_ACCEPTED, "text/plain", this->shouldReboot ? "OK" : "FAIL");
        response->addHeader("Connection", "close");
        request->send(response);
    }, onUpdate);    

    // 404 handler
    server->onNotFound([](AsyncWebServerRequest *request) { 
        request->send(HTTP_CODE_NOT_FOUND); 
    });

    server->begin();
    return true;
}

/// @brief Stops the update server
void Webserver::ServerStop() {
    Serial.println("Stopping web server");
    server->reset();
    server->end();
}

/// @brief Wraps the reboot checker task for static access.
/// @param arg The CommandProcessor object.
void Webserver::RebootCheckerTaskWrapper(void* arg) {
    static_cast<Webserver*>(arg)->RebootChecker();
}

/// @brief Checks if a reboot was requested
void Webserver::RebootChecker() {
    while (true) {
        if (shouldReboot) {
            Serial.println("Rebooting...");
            // Delay to show LED animation and let server send response
            delay(3000);
            ESP.restart();
        }
        // This loop doesn't need to be tight
        delay(100);
    }
}

/// @brief Handle file uploads to www folder. Adapted from https://github.com/smford/esp32-asyncwebserver-fileupload-example
/// @param request
/// @param filename
/// @param index
/// @param data
/// @param len
/// @param final
void Webserver::onUpload_www(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
    if (!index) {    
        request->_tempFile = SD_MMC.open("/www/" + filename, "w", true);
        Serial.println("Uploading file /www/" + filename);
    }
    if (len) {
        // Stream the incoming chunk to the opened file
        request->_tempFile.write(data, len);
    }
    if (final) {
        // Close the file handle as the upload is now done
        request->_tempFile.close();
    }
}

/// @brief Handle file uploads to settings folder.
/// @param request
/// @param filename
/// @param index
/// @param data
/// @param len
/// @param final
void Webserver::onUpload_settings(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
    if (!index) {    
        request->_tempFile = SD_MMC.open("/settings/" + filename, "w", true);
        Serial.println("Uploading file /settings/" + filename);
    }
    if (len) {
        // Stream the incoming chunk to the opened file
        request->_tempFile.write(data, len);
    }
    if (final) {
        // Close the file handle as the upload is now done
        request->_tempFile.close();
    }
}

/// @brief Handle file uploads to chimes folder.
/// @param request
/// @param filename
/// @param index
/// @param data
/// @param len
/// @param final
void Webserver::onUpload_chimes(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
    if (!index) {    
        request->_tempFile = SD_MMC.open("/chimes/" + filename, "w", true);
        Serial.println("Uploading file /chimes/" + filename);
    }
    if (len) {
        // Stream the incoming chunk to the opened file
        request->_tempFile.write(data, len);
    }
    if (final) {
        // Close the file handle as the upload is now done
        request->_tempFile.close();
    }
}

/// @brief Handle firmware update
/// @param request
/// @param filename
/// @param index
/// @param data
/// @param len
/// @param final
void Webserver::onUpdate(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
    if (!index)
    {
        Serial.printf("Update Start: %s\n", filename.c_str());
        // Ensure firmware will fit into flash space
        if (!Update.begin((ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000))
        {
            Update.printError(Serial);
        }
    }
    if (!Update.hasError())
    {
        if (Update.write(data, len) != len)
        {
            Update.printError(Serial);
        }
    }
    if (final)
    {
        if (Update.end(true))
        {
            Serial.printf("Update Success: %uB\n", index + len);
        }
        else
        {
            Update.printError(Serial);
        }
    }
}