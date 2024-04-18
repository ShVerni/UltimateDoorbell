/*
 * This file and associated .cpp file are licensed under the GPLv3 License Copyright (c) 2024 Sam Groveman
 * 
 * External libraries needed:
 * Adafruit NeoPixel: https://github.com/adafruit/Adafruit_NeoPixel
 * ArduinoJSON: https://arduinojson.org/
 * 
 * Contributors: Sam Groveman
 */

#pragma once
#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include <ArduinoJson.h>
#include <Storage.h>
#include <vector>
#include <map>

class LEDRing {
	public:
		#define LED_COUNT 16
		#define LED_PIN 1

		/// @brief Events that trigger the display
		enum Events { BELL_RING_START, BELL_RING_END, WIFI_CONFIG_START, WIFI_CONFIG_END, UPDATED, STORAGE_ERROR, I2S_PLAYER_ERROR, WEBHOOK_ERROR, DOORBELL_READY };

		LEDRing(Storage* Storage, String Animations_file);
		void begin();
		String GetAnimations();
		bool UpdateAnimations(String newAnimations);
		bool LoadAnimations();
		bool AddEventToQueue(Events event, String file = String());
		static void ProcessEventTaskWrapper(void* arg);
		
	private:
		/// @brief Queue to hold events to be processed.
		QueueHandle_t EventQueue;

		/// @brief Holds the name of events
		String event_names[9];

		/// @brief Reference to storage object
		Storage* storage;

		/// @brief LED  driver
		Adafruit_NeoPixel leds;

		/// @brief File storing the JSON encodings of the animations
		String animations_file;
		
		/// @brief Represents a single animation frame
		struct frame {
			/// @brief  The duration of this frame in milliseconds
			int duration;
			/// @brief the RGB value for each LED in this frame
			std::vector<uint32_t> colors;
		};

		/// @brief Represents an animation
		struct animation
		{
			/// @brief How many times this animation should repeat
			uint32_t repetitions;
			
			/// @brief Whether the LEDs should be turned off after the animation or left on the last frame
			bool clearOnDone;

			/// @brief A collection of frames for this animation
			std::vector<frame> frames;
		};
		
		/// @brief Collects animations for each event type and/or sound
		std::map<String, animation> animations;

		void ProcessEvent();
		void PlayAnimation(String name);
		bool LoadAnimationsHelper(const char* input);; 
};

/// @brief Default LED ring animations. Has been minified, see repo for non-minified version
const char default_animations[] = R"({"animations":{"BELL_RING_START":{"repetitions":1,"clearOnDone":true,"frames":[{"duration":75,"colors":["0","0x00317F","0x00317F","0x00317F","0x00317F","0x00317F","0x00317F","0x00317F","0x00317F","0x00317F","0x00317F","0x00317F","0x00317F","0x00317F","0x00317F","0x00317F"]},{"duration":75,"colors":["0x00317F","0","0x00317F","0x00317F","0x00317F","0x00317F","0x00317F","0x00317F","0x00317F","0x00317F","0x00317F","0x00317F","0x00317F","0x00317F","0x00317F","0x00317F"]},{"duration":75,"colors":["0x00317F","0x00317F","0","0x00317F","0x00317F","0x00317F","0x00317F","0x00317F","0x00317F","0x00317F","0x00317F","0x00317F","0x00317F","0x00317F","0x00317F","0x00317F"]},{"duration":75,"colors":["0x00317F","0x00317F","0x00317F","0","0x00317F","0x00317F","0x00317F","0x00317F","0x00317F","0x00317F","0x00317F","0x00317F","0x00317F","0x00317F","0x00317F","0x00317F"]},{"duration":75,"colors":["0x00317F","0x00317F","0x00317F","0x00317F","0","0x00317F","0x00317F","0x00317F","0x00317F","0x00317F","0x00317F","0x00317F","0x00317F","0x00317F","0x00317F","0x00317F"]},{"duration":75,"colors":["0x00317F","0x00317F","0x00317F","0x00317F","0x00317F","0","0x00317F","0x00317F","0x00317F","0x00317F","0x00317F","0x00317F","0x00317F","0x00317F","0x00317F","0x00317F"]},{"duration":75,"colors":["0x00317F","0x00317F","0x00317F","0x00317F","0x00317F","0x00317F","0","0x00317F","0x00317F","0x00317F","0x00317F","0x00317F","0x00317F","0x00317F","0x00317F","0x00317F"]},{"duration":75,"colors":["0x00317F","0x00317F","0x00317F","0x00317F","0x00317F","0x00317F","0x00317F","0","0x00317F","0x00317F","0x00317F","0x00317F","0x00317F","0x00317F","0x00317F","0x00317F"]},{"duration":75,"colors":["0x00317F","0x00317F","0x00317F","0x00317F","0x00317F","0x00317F","0x00317F","0x00317F","0","0x00317F","0x00317F","0x00317F","0x00317F","0x00317F","0x00317F","0x00317F"]},{"duration":75,"colors":["0x00317F","0x00317F","0x00317F","0x00317F","0x00317F","0x00317F","0x00317F","0x00317F","0x00317F","0","0x00317F","0x00317F","0x00317F","0x00317F","0x00317F","0x00317F"]},{"duration":75,"colors":["0x00317F","0x00317F","0x00317F","0x00317F","0x00317F","0x00317F","0x00317F","0x00317F","0x00317F","0x00317F","0","0x00317F","0x00317F","0x00317F","0x00317F","0x00317F"]},{"duration":75,"colors":["0x00317F","0x00317F","0x00317F","0x00317F","0x00317F","0x00317F","0x00317F","0x00317F","0x00317F","0x00317F","0x00317F","0","0x00317F","0x00317F","0x00317F","0x00317F"]},{"duration":75,"colors":["0x00317F","0x00317F","0x00317F","0x00317F","0x00317F","0x00317F","0x00317F","0x00317F","0x00317F","0x00317F","0x00317F","0x00317F","0","0x00317F","0x00317F","0x00317F"]},{"duration":75,"colors":["0x00317F","0x00317F","0x00317F","0x00317F","0x00317F","0x00317F","0x00317F","0x00317F","0x00317F","0x00317F","0x00317F","0x00317F","0x00317F","0","0x00317F","0x00317F"]},{"duration":75,"colors":["0x00317F","0x00317F","0x00317F","0x00317F","0x00317F","0x00317F","0x00317F","0x00317F","0x00317F","0x00317F","0x00317F","0x00317F","0x00317F","0x00317F","0","0x00317F"]},{"duration":75,"colors":["0x00317F","0x00317F","0x00317F","0x00317F","0x00317F","0x00317F","0x00317F","0x00317F","0x00317F","0x00317F","0x00317F","0x00317F","0x00317F","0x00317F","0x00317F","0"]}]},"BELL_RING_END":{"repetitions":0,"clearOnDone":true,"frames":[{"duration":0,"colors":["0","0","0","0","0","0","0","0","0","0","0","0","0","0","0","0"]}]},"WIFI_CONFIG_START":{"repetitions":1,"clearOnDone":false,"frames":[{"duration":50,"colors":["0x00007F","0","0","0","0","0","0","0","0x00007F","0","0","0","0","0","0","0"]},{"duration":50,"colors":["0","0x00007F","0","0","0","0","0","0","0","0x00007F","0","0","0","0","0","0"]},{"duration":50,"colors":["0","0","0x00007F","0","0","0","0","0","0","0","0x00007F","0","0","0","0","0"]},{"duration":50,"colors":["0","0","0","0x00007F","0","0","0","0","0","0","0","0x00007F","0","0","0","0"]},{"duration":50,"colors":["0","0","0","0","0x00007F","0","0","0","0","0","0","0","0x00007F","0","0","0"]},{"duration":50,"colors":["0","0","0","0","0","0x00007F","0","0","0","0","0","0","0","0x00007F","0","0"]},{"duration":50,"colors":["0","0","0","0","0","0","0x00007F","0","0","0","0","0","0","0","0x00007F","0"]},{"duration":50,"colors":["0","0","0","0","0","0","0","0x00007F","0","0","0","0","0","0","0","0x00007F"]},{"duration":0,"colors":["0","0x00007F","0","0x00007F","0","0x00007F","0","0x00007F","0","0x00007F","0","0x00007F","0","0x00007F","0","0x00007F"]}]},"WIFI_CONFIG_END":{"repetitions":1,"clearOnDone":true,"frames":[{"duration":50,"colors":["0xFF7F00","0","0","0","0","0","0","0","0xFF7F00","0","0","0","0","0","0","0"]},{"duration":50,"colors":["0","0xFF7F00","0","0","0","0","0","0","0","0xFF7F00","0","0","0","0","0","0"]},{"duration":50,"colors":["0","0","0xFF7F00","0","0","0","0","0","0","0","0xFF7F00","0","0","0","0","0"]},{"duration":50,"colors":["0","0","0","0xFF7F00","0","0","0","0","0","0","0","0xFF7F00","0","0","0","0"]},{"duration":50,"colors":["0","0","0","0","0xFF7F00","0","0","0","0","0","0","0","0xFF7F00","0","0","0"]},{"duration":50,"colors":["0","0","0","0","0","0xFF7F00","0","0","0","0","0","0","0","0xFF7F00","0","0"]},{"duration":50,"colors":["0","0","0","0","0","0","0xFF7F00","0","0","0","0","0","0","0","0xFF7F00","0"]},{"duration":50,"colors":["0","0","0","0","0","0","0","0xFF7F00","0","0","0","0","0","0","0","0xFF7F00"]}]},"UPDATED":{"repetitions":3,"clearOnDone":true,"frames":[{"duration":50,"colors":["0xFF00C4","0","0","0","0xFF00C4","0","0","0","0xFF00C4","0","0","0","0xFF00C4","0","0","0"]},{"duration":50,"colors":["0","0xFF00C4","0","0","0","0xFF00C4","0","0","0","0xFF00C4","0","0","0","0xFF00C4","0","0"]},{"duration":50,"colors":["0","0","0xFF00C4","0","0","0","0xFF00C4","0","0","0","0xFF00C4","0","0","0","0xFF00C4","0"]},{"duration":50,"colors":["0","0","0","0xFF00C4","0","0","0","0xFF00C4","0","0","0","0xFF00C4","0","0","0","0xFF00C4"]}]},"STORAGE_ERROR":{"repetitions":0,"clearOnDone":false,"frames":[{"duration":50,"colors":["0xFF0000","0","0","0","0","0","0","0","0","0","0","0","0","0","0","0"]}]},"I2S_PLAYER_ERROR":{"repetitions":0,"clearOnDone":false,"frames":[{"duration":50,"colors":["0xFF0000","0xFF0000","0","0","0","0","0","0","0","0","0","0","0","0","0","0"]}]},"WEBHOOK_ERROR":{"repetitions":0,"clearOnDone":false,"frames":[{"duration":50,"colors":["0xFF0000","0xFF0000","0xFF0000","0","0","0","0","0","0","0","0","0","0","0","0","0"]}]},"DOORBELL_READY":{"repetitions":0,"clearOnDone":true,"frames":[{"duration":50,"colors":["0x007F00","0","0","0","0","0","0","0","0","0","0","0","0","0","0","0"]},{"duration":50,"colors":["0","0x007F00","0","0","0","0","0","0","0","0","0","0","0","0","0","0"]},{"duration":50,"colors":["0","0","0x007F00","0","0","0","0","0","0","0","0","0","0","0","0","0"]},{"duration":50,"colors":["0","0","0","0x007F00","0","0","0","0","0","0","0","0","0","0","0","0"]},{"duration":50,"colors":["0","0","0","0","0x007F00","0","0","0","0","0","0","0","0","0","0","0"]},{"duration":50,"colors":["0","0","0","0","0","0x007F00","0","0","0","0","0","0","0","0","0","0"]},{"duration":50,"colors":["0","0","0","0","0","0","0x007F00","0","0","0","0","0","0","0","0","0"]},{"duration":50,"colors":["0","0","0","0","0","0","0","0x007F00","0","0","0","0","0","0","0","0"]},{"duration":50,"colors":["0","0","0","0","0","0","0","0","0x007F00","0","0","0","0","0","0","0"]},{"duration":50,"colors":["0","0","0","0","0","0","0","0","0","0x007F00","0","0","0","0","0","0"]},{"duration":50,"colors":["0","0","0","0","0","0","0","0","0","0","0x007F00","0","0","0","0","0"]},{"duration":50,"colors":["0","0","0","0","0","0","0","0","0","0","0","0x007F00","0","0","0","0"]},{"duration":50,"colors":["0","0","0","0","0","0","0","0","0","0","0","0","0x007F00","0","0","0"]},{"duration":50,"colors":["0","0","0","0","0","0","0","0","0","0","0","0","0","0x007F00","0","0"]},{"duration":50,"colors":["0","0","0","0","0","0","0","0","0","0","0","0","0","0","0x007F00","0"]},{"duration":50,"colors":["0","0","0","0","0","0","0","0","0","0","0","0","0","0","0","0x007F00"]}]}}})";