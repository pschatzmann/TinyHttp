
/**
 * @file serverMP3.ino
 * @author SSID (phil.schatzmann@gmail.com)
 * @brief  We start a Webserver which streams MP3 files from a SD card. The same data is pushed to potentially multiple clients
 * @version 0.1
 * @date 2020-11-30
 * 
 * @copyright Copyright (c) 2020
 * 
 */

#include <WiFi.h>
#include "HttpServer.h"
#include "Extensions/ExtensionMusicFileStream.h"

WiFiServer wifi;
HttpServer server(wifi);
ExtensionMusicFileStream sdMp3("/music/mp3", "/", "audio/mpeg", ".mp3", 512);

void setup() {
    Serial.begin(115200);
    // connect to WIFI
    WiFi.begin("SSID", "password");
    while (WiFi.status() != WL_CONNECTED) {        
      delay(500);
      Serial.print(".");
    }

    Log.setLogger(Serial, Info);

    server.rewrite("/","/music/mp3");
    server.addExtension(sdMp3);
    server.begin(80);
    Log.log(Info,"server was started...");

}

void loop(){
    server.doLoop();
}