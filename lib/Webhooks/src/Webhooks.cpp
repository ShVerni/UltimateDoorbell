#include "Webhooks.h"

/// @brief Creates an HTTPRequests object
/// @param Settings Reference to a SDCard object
/// @param Settings Path to the JSON settings file
Webhooks::Webhooks(SDCard* Card, String Settings) {
    card = Card;
    settings_file = Settings;
    EventQueue = xQueueCreate(10, sizeof(String*));
}

 /// @brief Add event to the webhook queue
 /// @param event The type of the event
 /// @param file The full path to the sound file being played, if any
 /// @return True on success
 bool Webhooks::AddEventToQueue(int event, String file) {
    String *params = new String(String(event) + ":" + file);
    if (xQueueSendToBack(EventQueue, &params, 10) == errQUEUE_FULL) {
        Serial.println("Webhook event queue full");
        return false;
    }
    return true;
 }

/// @brief Wraps the event processor task for static access.
/// @param arg The Webhooks object.
void Webhooks::ProcessEventTaskWrapper(void* arg) {
    static_cast<Webhooks*>(arg)->ProcessEvent();
}

/// @brief Process each event in the queue as an infinite loop
void Webhooks::ProcessEvent() {
    String *params_ptr = NULL;
    while(true) 
    {
        if (xQueueReceive(EventQueue, &params_ptr, 10) == pdTRUE) {
            if (enable) {
                String params = *params_ptr;
                FireHooks(params.substring(0, params.indexOf(":")), params.substring(params.indexOf(":") + 1));
            }
            delete params_ptr;
        }
    }
}

/// @brief Load settings from file
/// @return True on success
bool Webhooks::LoadSettings() {
    Serial.println("Loading webhook settings....");
    String content = card->readFile(settings_file);
    if (content != "") {
        Serial.println("Webhook settings loaded.");
        return UpdateSettings(content);
    } else {
        return SaveSettings();
    }
}

/// @brief Saves current settings to file
/// @return True on success
bool Webhooks::SaveSettings() {
    Serial.println("Saving webhook settings....");
    String test;
    return card->writeFile(settings_file, GetSettings());
}

/// @brief Get current settings
/// @return The settings as a JSON string
String Webhooks::GetSettings() {
    DynamicJsonDocument settings(2048);
    settings["enable"] = enable;
    if (hooks.empty()) {
        settings["webhooks"] = NULL;
    } else {
        for (int i = 0; i < hooks.size(); i++) {
            settings["webhooks"][i]["url"] = hooks[i].url;
            settings["webhooks"][i]["method"] = hooks[i].method;
            if (hooks[i].parameters.empty()) {
                settings["webhooks"][i]["parameters"] = NULL;
            } else {
                for (std::pair<String, String> const& param : hooks[i].parameters) {
                    settings["webhooks"][i]["parameters"][param.first] = param.second;
                }
            }
        }
    }    
    settings.shrinkToFit();
    String settings_string;
    serializeJson(settings, settings_string);
    Serial.println(settings_string);
    return settings_string;
}

/// @brief Update current settings
/// @param settings JSON string of new settings
/// @return True on success
bool Webhooks::UpdateSettings(String settings) {
    if (settings != "") {
        // Parse settings string
        settings.trim();
        Serial.println("New settings : " + settings);
        DynamicJsonDocument new_settings(2048);
        DeserializationError error = deserializeJson(new_settings, settings);
        if (error) {
            Serial.println("Bad settings data received");
            return false;
        }
        new_settings.shrinkToFit();
        enable = new_settings["enable"];
        // Remove old webhooks list
        hooks.clear();
        // Create new webhooks
        for (JsonObject hook : new_settings["webhooks"].as<JsonArray>()) {;
            std::map<String,String> params;
            for (JsonPair kv : hook["parameters"].as<JsonObject>()) {
                params[kv.key().c_str()] = kv.value().as<String>();
            }
            hooks.push_back(webhook {
                hook["url"].as<String>(),
                hook["method"].as<HTTP_Method>(),
                params
            });
        }
        return true;
    }
    return false;
}

/// @brief Fires all registered webhooks
/// @param event The event triggering the hook
/// @param sound_file The full path to the sound file, if any, being played
void Webhooks::FireHooks(String event, String sound_file) {
    Serial.println("Firing webhooks");
    for (webhook hook : hooks) {
        String query = "";
        String url = hook.url;
        int response_code;
        if (!hook.parameters.empty()) {
            client.addHeader("Content-Type", "application/x-www-form-urlencoded");
            bool first = true;
            for (std::pair<String, String> param : hook.parameters) {
                // Skip this parameter if it needs a sound file and none is provided
                if (sound_file.isEmpty() && param.second.indexOf("%SOUND_FILE%") != -1){
                    break;
                }
                if (first) {
                    first = false;
                } else {
                    query += '&';
                }
                query += param.first + '=' + param.second;
                // Process template
                query.replace("%EVENT%", event);
                query.replace("%SOUND_FILE%", sound_file);
            }
        }
        Serial.println("URL: " + url);
        Serial.println("Parameters: " + query);
        if (hook.method == HTTP_GET) {
            if (query != "")
                url += '?' + query;
            client.begin(url);
            response_code = client.GET();
        } else if (hook.method == HTTP_POST) {
            client.begin(url);
            response_code = client.POST(query);
        } else {
            Serial.println("ERROR: Unrecognized HTTP method");
            client.end();
            break;
        }
        bool success = false;
        if (response_code > 0 ) {
            if (response_code == HTTP_CODE_OK || response_code == HTTP_CODE_ACCEPTED) {
                String payload = client.getString();
                Serial.println(payload);
            } else {
                Serial.printf("Unexpected response code: %d\n", response_code);
            }
        } else {
            Serial.printf("Webhook request failed failed, error: %s\n", client.errorToString(response_code).c_str());
        }
        client.end();
    }
}
