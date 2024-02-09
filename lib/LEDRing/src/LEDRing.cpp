#include "LEDRing.h"

/// @brief Controls an LEDRing. Define LED pin and LED count in header file.
/// @param Animations_file Path to the file storing animations
LEDRing::LEDRing(SDCard* Card, String Animations_file) : leds(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800),
event_names {"BELL_RING_START", "BELL_RING_END", "WIFI_CONFIG_START", "WIFI_CONFIG_END", "UPDATED", "SD_ERROR", "I2S_PLAYER_ERROR", "WEBHOOK_ERROR", "DOORBELL_READY"}
{
    animations_file = Animations_file;
    card = Card;
    // Create queue
    EventQueue = xQueueCreate(10, sizeof(String*));
}

/// @brief Initializes the LED ring
void LEDRing::begin() {
    // Start LEDs
    leds.begin();
    leds.fill(0x7F1500); // Show while booting
    leds.show();

    // Load default animations
    LoadAnimationsHelper(default_animations);
}

/// @brief Reads the list of current custom animations
/// @return The JSON formatted animations, or empty string on failure
String LEDRing::GetAnimations() {
    String animation_settings = String();
    if (card->fileExists(animations_file))
        animation_settings = card->readFile(animations_file);
    return animation_settings;
}

/// @brief Updates and saves custom animations
/// @param newAnimations JSON string of new custom animations
/// @return True on success
bool LEDRing::UpdateAnimations(String newAnimations) {
    if (LoadAnimationsHelper(newAnimations.c_str())) {
        return card->writeFile(animations_file, newAnimations);
    }
    return false;
}

/// @brief Loads the animations from the animation file
/// @return True on success
bool LEDRing::LoadAnimations() {
    Serial.println("Loading animations");
    if (card->fileExists(animations_file)) {
        String settings = card->readFile(animations_file);
        // Parse settings string
        settings.trim();
        Serial.println("Loaded animations file");
        return LoadAnimationsHelper(settings.c_str());
    } else {
        Serial.println("Animations files doesn't exist");
        return false;
    }
}

/// @brief Processes animations from a C-type string
/// @param input The JSON formatted string
/// @return True on success
bool LEDRing::LoadAnimationsHelper(const char* input) {
    DynamicJsonDocument new_settings(1048576); // Allocate a big 1MiB document to load animations (requires PSRAM)
    DeserializationError error = deserializeJson(new_settings, input);
    if (error) {
        Serial.println("Bad settings data loaded");
        Serial.println(error.c_str());
        return false;
    }
    new_settings.shrinkToFit();
    // Add animations
    for (JsonPair kv : new_settings["animations"].as<JsonObject>()) {
        JsonObject new_animation = kv.value().as<JsonObject>();
        // Create frames
        std::vector<frame> new_frames;
        for (JsonObject new_frame : new_animation["frames"].as<JsonArray>()) {
            // Build color array
            JsonArray colors = new_frame["colors"].as<JsonArray>();
            std::vector<uint32_t> new_colors; 
            for (int i = 0; i < colors.size(); i++) {
                new_colors.push_back(std::strtoul(colors[i].as<String>().c_str(), NULL, 0)); // Gamma correction could go here
            }
            // Add frame to animation
            new_frames.push_back(frame {
                new_frame["duration"].as<int>(),
                new_colors
            });
        }
        // Add or replace animation in map
        animations[kv.key().c_str()] = animation {
            new_animation["repetitions"].as<uint32_t>(),
            new_animation["clearOnDone"].as<bool>(),
            new_frames 
        };
    }
    return true;
}

// @brief Adds a command to the queue
/// @param event The event to add
/// @param file The full path the the sound file playing for this event, if any
/// @return True on success.
bool LEDRing::AddEventToQueue(Events event, String file) {
    String *event_string = new String(event_names[event] + ':' + file);
    if (xQueueSendToBack(EventQueue, (void*) &event_string, 10) == errQUEUE_FULL) {
        Serial.println("LED event queue full");
        return false;
    }
    return true;
}

/// @brief Wraps the event processor task for static access.
/// @param arg The LEDRing object.
void LEDRing::ProcessEventTaskWrapper(void* arg) {
    static_cast<LEDRing*>(arg)->ProcessEvent();
}

/// @brief Process each event in the queue as an infinite loop
void LEDRing::ProcessEvent() {
    String *event_ptr = NULL;
    while(true) 
    {
        if (xQueueReceive(EventQueue, &event_ptr, 10) == pdTRUE) {
            Serial.print("Processing event ");
            Serial.println(*event_ptr);
            String event = *event_ptr;
            // Get event name
            String animation_to_play = event.substring(0, event.indexOf(":"));
            // Get file name, if any
            String file = event.substring(event.indexOf(":") + 1);
            if (!file.isEmpty() && animations.find(file) != animations.end()) {
                animation_to_play = file;
            }
            PlayAnimation(animation_to_play);
            delete event_ptr;
        }
    }
}

/// @brief Shows an animation on the LEDs
/// @param event The name of the animation to play
void LEDRing::PlayAnimation(String name) {
    Serial.println("Playing animation " + name);
    animation* playing = &animations[name];
    for (int i = 0; i <= playing->repetitions; i++) {
        // Loop through each frame
        for (frame f : playing->frames) {
            // Set the color of each LED
            for (uint16_t j = 0; j < f.colors.size(); j++) {
                leds.setPixelColor(j, f.colors[j]);
            }
            leds.show();
            delay(f.duration);
        }
    }
    if (playing->clearOnDone) {
        leds.clear();
        leds.show();
    }
}
