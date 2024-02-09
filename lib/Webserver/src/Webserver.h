/*
 * This file and associated .cpp file are licensed under the MIT Lesser General Public License Copyright (c) 2024 Sam Groveman
 * 
 * External libraries needed:
 * ESPAsyncWebServer: https://github.com/esphome/ESPAsyncWebServer
 * 
 * Contributors: Sam Groveman
 */

#pragma once
#include <ESPAsyncWebServer.h>
#include <Update.h>
#include <SD_MMC.h>
#include <HTTPClient.h>
#include <LEDRing.h>
#include <SDCard.h>
#include <ArduinoJson.h>
#include <SoundPlayer.h>
#include <Webhooks.h>
#include <vector>

/// @brief Local web server.
class Webserver {
    public:
        /// @brief Reboot on firmware update flag
        bool shouldReboot = false;
        
        Webserver(AsyncWebServer* webserver, LEDRing* LEDs, SoundPlayer* Player, SDCard* Card, Webhooks* Hooks, bool* Ringing);
        bool ServerStart();
        void ServerStop();
        static void RebootCheckerTaskWrapper(void* arg);
        
    private:
        #define FIRMWARE_VERSION "0.5.0"
        /// @brief Pointer to the Webserver object
        AsyncWebServer* server;

        /// @brief Pointer to the LEDRing object
        LEDRing* leds;

        /// @brief Pointer to the SDCard object
        SDCard* card;

        /// @brief Pointer to the SoundPlayer object
        SoundPlayer* player;

         /// @brief Pointer to the Webhooks object
        Webhooks* hooks;

        /// @brief Reference to a bool that can be used to indicate the bell is ringing
        bool* ringing;

        static void onUpload_www(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final);
        static void onUpload_settings(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final);
        static void onUpload_chimes(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final);
        static void onUpdate(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final);
        void RebootChecker();
};

// @brief Text of update webpage
const char update_page[] = R"(<!DOCTYPE html>
<html lang='en-us'>
<head>
<title>Firmware Updater</title>
</head>
<body>
<div id='up-wrap'>
<h1>Ultimate Doorbell</h1>
<h2>Upload Firmware</h2>
<h3>Current version: 0.5.0</h3>
<div id='up-progress'>
    <div id='up-bar'></div>
    <div id='up-percent'>0%</div>
</div>
<input type='file' id='up-file' disabled>
<label for='up-file' id='up-label'>
    Update
</label>
<div id='message'></div>
</div>
<script>
var uprog = {
    hBar : null,
    hPercent : null,
    hFile : null,
    init : () => {
        uprog.hBar = document.getElementById('up-bar');
        uprog.hPercent = document.getElementById('up-percent');
        uprog.hFile = document.getElementById('up-file');
        uprog.hFile.disabled = false;
        document.getElementById('up-label').onclick = uprog.upload;
    },
    update : (percent) => {
    percent = percent + '%';
    uprog.hBar.style.width = percent;
    uprog.hPercent.innerHTML = percent;
    if (percent == '100%') { uprog.hFile.disabled = false; }
    },
    upload : () => {
    if(uprog.hFile.files.length == 0 ){
    return;
    }
    let file = uprog.hFile.files[0];
    uprog.hFile.disabled = true;
    uprog.hFile.value = '';
    let xhr = new XMLHttpRequest(), data = new FormData();
    data.append('upfile', file);
    xhr.open('POST', '/update');
    let percent = 0;
    xhr.upload.onloadstart = (evt) => { uprog.update(0); };
    xhr.upload.onloadend = (evt) => { uprog.update(100); };
    xhr.upload.onprogress = (evt) => {
        percent = Math.ceil((evt.loaded / evt.total) * 100);
        uprog.update(percent);
    };
    xhr.onload = function () {
        if (this.response != 'OK' || this.status != 202) {
        document.getElementById('message').innerHTML = 'ERROR!';
        } else {
        uprog.update(100);
        document.getElementById('message').innerHTML = 'Success, rebooting!';
        }
    };
    xhr.send(data);
    }
};
window.addEventListener('load', uprog.init);
</script>
<style>
#message{font-size:18px;font-weight:bolder}
#up-file,#up-label{width:100%;height:44px;border-radius:4px;margin:10px auto;font-size:17px}
#up-label{background:#f1f1f1;border:0;display:block;line-height:44px}
body{background:#3498db;font-family:sans-serif;font-size:14px;color:#777}
#up-file{padding:0;border:1px solid #ddd;line-height:44px;text-align:left;display:block;cursor:pointer}
#up-bar,#up-progress{background-color:#f1f1f1;border-radius:10px;position:relative}
#up-bar{background-color:#3498db;width:0%;height:30px}
#up-wrap{background:#fff;max-width:258px;margin:75px auto;padding:30px;border-radius:5px;text-align:center}
#up-label{background:#3498db;color:#fff;cursor:pointer}
#up-percent{position:absolute;top:6px;left:0;width:100%;display:flex;align-items:center;justify-content:center;text-shadow:-1px 1px 0 #000,1px 1px 0 #000,1px -1px 0 #000,-1px -1px 0 #000;color:#fff}</style>
</body>
</html>)";

// @brief Text of default index webpage
const char index_page[] = R"(<!DOCTYPE html>
<html lang='en-us'>
<head>
<title>Default Server Page</title>
</head>
<body>
<div id='up-wrap'>
<h1>Default Server Setup</h1>
<p>No index page found, this page will allow you to upload new web server files for your doorbell. You will need to reboot to use a new index page.</p>
<p>Default files are: index.html, update.html, reset.html, the latter two are optional.</p>
<p><strong>Upload ALL necessary web files (.html/.css/.html) before rebooting</strong>.</p>
<div id='up-progress'>
    <div id='up-bar'></div>
    <div id='up-percent'>0%</div>
</div>
<div id='message'></div>
<input type='file' id='up-file' disabled>
<label class='def-button' for='up-file' id='up-label'>
    Upload
</label>
<a class='def-button' id='update' href='/update'>Update Firmware</a>
<button class='def-button' id='reboot'>Reboot Device</button>
<button class='def-button' id='reset'>Reset WiFi Settings</button>
</div>
<script>
var uprog = {
    hBar : null,
    hPercent : null,
    hFile : null,
    init : () => {
        uprog.hBar = document.getElementById('up-bar');
        uprog.hPercent = document.getElementById('up-percent');
        uprog.hFile = document.getElementById('up-file');
        uprog.hFile.disabled = false;
        document.getElementById('up-label').onclick = uprog.upload;
    },
    update : (percent) => {
    percent = percent + '%';
    uprog.hBar.style.width = percent;
    uprog.hPercent.innerHTML = percent;
    if (percent == '100%') { uprog.hFile.disabled = false; }
    },
    upload : () => {
    if(uprog.hFile.files.length == 0 ){
    return;
    }
    let file = uprog.hFile.files[0];
    uprog.hFile.disabled = true;
    uprog.hFile.value = '';
    let xhr = new XMLHttpRequest(), data = new FormData();
    data.append('upfile', file);
    xhr.open('POST', '/upload-www');
    let percent = 0;
    xhr.upload.onloadstart = (evt) => { uprog.update(0); };
    xhr.upload.onloadend = (evt) => { uprog.update(100); };
    xhr.upload.onprogress = (evt) => {
        percent = Math.ceil((evt.loaded / evt.total) * 100);
        uprog.update(percent);
    };
    xhr.onload = function () {
        if (this.status != 202) {
        document.getElementById('message').innerHTML = 'ERROR!';
        } else {
        uprog.update(100);
        document.getElementById('message').innerHTML = 'File uploaded!';
        }
    };
    xhr.send(data);
    }
};
window.addEventListener('load', uprog.init);
document.getElementById("reboot").onclick = function() {
    let xhr = new XMLHttpRequest();
    xhr.open('PUT', '/reboot');
    xhr.onload = function () {
        if (this.status != 200) {
        document.getElementById('message').innerHTML = 'ERROR!';
        } else {
        document.getElementById('message').innerHTML = 'Success, rebooting!';
        }
    };
    xhr.send();
};
document.getElementById("reset").onclick = function() {
    let xhr = new XMLHttpRequest();
    xhr.open('PUT', '/reset');
    xhr.onload = function () {
        if (this.response != 200) {
        document.getElementById('message').innerHTML = 'ERROR!';
        } else {
        document.getElementById('message').innerHTML = 'Success, rebooting!';
        }
    };
    xhr.send();
};
</script>
<style>
#message{font-size:18px;font-weight:bolder}
#up-file,.def-button{width:100%;height:44px;border-radius:4px;margin:10px auto;font-size:17px}
body{background:#3498db;font-family:sans-serif;font-size:14px;color:#777}
#up-file{padding:0;border:1px solid #ddd;line-height:44px;text-align:left;display:block;cursor:pointer}
#up-bar,#up-progress{background-color:#f1f1f1;border-radius:10px;position:relative}
#up-bar{background-color:#3498db;width:0%;height:30px}
#up-wrap{background:#fff;max-width:258px;margin:75px auto;padding:30px;border-radius:5px;text-align:center}
#up-percent{position:absolute;top:6px;left:0;width:100%;display:flex;align-items:center;justify-content:center;text-shadow:-1px 1px 0 #000,1px 1px 0 #000,1px -1px 0 #000,-1px -1px 0 #000;color:#fff}
.def-button{background:#3498db;color:#fff;cursor:pointer;border:0;display:block;line-height:44px;text-decoration:none}</style>
</body>
</html>)";