#include "SoundPlayer.h"

/// @brief Create an audio player object
/// @param Storage Reference to an SDCard object
/// @param LEDs Reference to an LEDRing object
/// @param Hooks Reference to an Webhook object
/// @param Settings_file Path to settings file
SoundPlayer::SoundPlayer(Storage* Storage, String Settings_file) {
	storage = Storage;
	settings_file = Settings_file;
}

/// @brief Initializes the audio player
/// @param I2S_BCLK BCLK pin number
/// @param I2S_LRC LRC pin number
/// @param I2S_DOUT DOUT pin number
/// @return True on success
bool SoundPlayer::begin(int I2S_BCLK, int I2S_LRC, int I2S_DOUT) {
	if(player.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT)) {
		player.setVolume(10); // default 0...21
		return true;
	}
	return false;
}

/* For debugging
/// @brief Called by Audio library, shows file information
/// @param info Pointer to info object
void audio_info(const char *info){
	Serial.print("info        "); 
	Serial.println(info);
}
*/

/// @brief Calls the audio player loop function (should be done in the main loop or equivalent)
void SoundPlayer::callLoop() {
	player.loop();
}

/// @brief Play a random chime sound from the selected options
/// @return The full path to the random sound file chosen to be played
String SoundPlayer::playChimeSound() {
	if (_files.size() == 0)
		return "";
	String sound = _files[random(0, _files.size())];
	playFile(sound);
	return sound;
}

/// @brief Play a random chime sound from the selected options
/// @param sound The full path of the sound file to played
/// @return True on success
bool SoundPlayer::playChimeSound(String sound) {
	return playFile(sound);
}

/// @brief Checks to see if a sound is playing
/// @return True while a sound is playing
bool SoundPlayer::isPlaying() {
	return player.isRunning();
}

/// @brief Gets the currently saved settings of the sound player
/// @return A JSON string of the settings
String SoundPlayer::getSettings() {
	String settings = "{\"volume\":" + String(player.getVolume()) + ",\"files\": [";
	bool first = true;
	for (String path : _files) {
		if (first) {
			settings += '"' + path + '"';
			first = false;
		} else {
			settings += ", \"" + path + '"';
		}
	}
	settings += "]}";
	return settings;
}

/// @brief Loads the saved settings for the sound player
/// @return True on success
bool SoundPlayer::loadSettings() {
	Serial.println("Loading audio settings....");
	String content = storage->readFile(settings_file);
	if (content != "") {
		Serial.println("Audio settings loaded.");
		return updateSettings(content);
	} else {
		return saveSettings();
	}
}

/// @brief Saves the current sound player settings to the file system
/// @return True on success
bool SoundPlayer::saveSettings() {
	Serial.println("Saving audio settings....");    
	return storage->writeFile(settings_file, getSettings());
}

/// @brief Called when there are new audio settings
/// @param settings A JSON object of new parameters.
/// @return True on success.
bool SoundPlayer::updateSettings(String settings) {
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
		// Set volume
		player.setVolume(new_settings["volume"].as<int>()); 
		// Remove old file list
		for (int i = 0; i < _files.size(); i++) {
			_files[i].clear();
		}
		_files.clear();
		// Create new file list
		for (String path : new_settings["files"].as<JsonArray>()) {
			_files.push_back(path);
		}
		return true;
	}
	return false;
}

/// @brief Play a specific audio file
/// @param file The full path of the audio file
/// @return True on success
bool SoundPlayer::playFile(String file) {
	Serial.println("Playing: " + file);
	if (Storage::isUsingLittleFS())
		return player.connecttoFS(LittleFS, file.c_str());
	else
		return player.connecttoFS(SD_MMC, file.c_str());
}