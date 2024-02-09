/*
 * This file and associated .cpp file are licensed under the MIT Lesser General Public License Copyright (c) 2024 Sam Groveman
 * 
 * Contributors: Sam Groveman
 */

#pragma once
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <SDCard.h>
#include <map>
#include <vector>

class Webhooks {
    public:
        Webhooks(SDCard* Card, String Settings);
        bool LoadSettings();
        bool SaveSettings();
        String GetSettings();
        bool UpdateSettings(String settings);
        bool AddEventToQueue(int event, String file = String());
        static void ProcessEventTaskWrapper(void* arg);

    private:
        /// @brief HTTPClient object
        HTTPClient client;

        /// @brief Queue to hold events
        QueueHandle_t EventQueue;

        /// @brief The path to the settings file
        String settings_file;

        /// @brief Reference to SDCard object
        SDCard* card;

        /// @brief Enable webhook calls
        bool enable;

        /// @brief HTTP methods accepted
        typedef enum {    
            HTTP_GET,
            HTTP_POST
        } HTTP_Method;

        /// @brief Structure representing a webhook request
        struct webhook {
            /// @brief URL of the request, can include port, :80 is default
            String url;

            /// @brief The method to use (HTTP_POST or HTTP_GET)
            HTTP_Method method;

            /// @brief Any GET/POST parameters to use.
            /// Use values %SOUND_FILE% or %EVENT% to get the sound file being played and/or the event triggering the hook
            std::map<String, String> parameters;
        };

        /// @brief Collection of all webhooks to call
        std::vector<webhook> hooks;

        void ProcessEvent();
        void FireHooks(String event, String sound_file);
        String ProcessParamTemplate(String param);
};