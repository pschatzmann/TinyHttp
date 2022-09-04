
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

#include "HttpServer.h"
#include "HttpExtensions.h"

// I was using the AudioKit to test: It has quite some strange SD pin setup
#define PIN_AUDIO_KIT_SD_CARD_CS 13
#define PIN_AUDIO_KIT_SD_CARD_MISO 2
#define PIN_AUDIO_KIT_SD_CARD_MOSI 15
#define PIN_AUDIO_KIT_SD_CARD_CLK  14

const char* ssid = "SSID";
const char* password = "PASSWORD";
WiFiServer wifi;
HttpServer server(wifi);
ExtensionMusicFileStream sdMp3("/music/mp3", "/", "audio/mpeg", ".mp3", 512, PIN_AUDIO_KIT_SD_CARD_CS);

void setup() {
    Serial.begin(115200);
    Log.setLogger(Serial, Info);

    // If you use custom pins for the CD drive: Comment out if you use the standard pins
    SPI.begin(PIN_AUDIO_KIT_SD_CARD_CLK, PIN_AUDIO_KIT_SD_CARD_MISO, PIN_AUDIO_KIT_SD_CARD_MOSI, PIN_AUDIO_KIT_SD_CARD_CS);

    server.rewrite("/","/music/mp3");
    server.addExtension(sdMp3);
    server.begin(80, ssid, password);
    Log.log(Info,"server was started...");

}

void loop(){
    server.doLoop();
}