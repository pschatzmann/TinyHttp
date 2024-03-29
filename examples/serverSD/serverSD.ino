
/**
 * @file serverSD.ino
 * @author SSID (phil.schatzmann@gmail.com)
 * @brief  We start a Webserver which serves the pages from a SD card.
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
ExtensionSD sd("/*", PIN_AUDIO_KIT_SD_CARD_CS); // or use ExtensionSDStreamed

void setup() {
    Serial.begin(115200);
    HttpLogger.begin(Serial, Info);
 
    // If you use custom pins for the CD drive: Comment out if you use the standard pins
    SPI.begin(PIN_AUDIO_KIT_SD_CARD_CLK, PIN_AUDIO_KIT_SD_CARD_MISO, PIN_AUDIO_KIT_SD_CARD_MOSI, PIN_AUDIO_KIT_SD_CARD_CS);

    // start server
    server.rewrite("/","/index.html");
    server.addExtension(sd);
    server.begin(80, ssid, password);
    HttpLogger.log(Info,"server was started...");

    // create test files
    setupTestFiles();
}

void setupTestFiles(){
  HttpLogger.log(Info,"setupTestFiles");
  const char* index = "<html><body style='background-color:black; color:white'><h1>SD Example</h1> <a href='/test1.html'>test1</a><br/><a href='/test2.html'>test2</a><br/><a href='/test3.html'>test3</a></body></html>";
  const char* test1 = "<html><body style='background-color:black; color:white'><h1>Test1</h1> Lorem ipsum dolor sit amet, consectetur adipiscing elit. </body></html>";
  const char* test2 = "<html><body style='background-color:black; color:white'><h1>Test2</h1> Lorem ipsum dolor sit amet, consectetur adipiscing elit. </body></html>";
  const char* test3 = "<html><body style='background-color:black; color:white'><h1>Test3</h1> Lorem ipsum dolor sit amet, consectetur adipiscing elit. </body></html>";
  setupTestFile("/index.html",index);
  setupTestFile("/test1.html",test1);
  setupTestFile("/test2.html",test2);
  setupTestFile("/test3.html",test3);

}

void setupTestFile(const char* fileName, const char* content){
    HttpLogger.log(Info,"setupTestFile", fileName);
    File file = SD.open(fileName, FILE_WRITE);
    file.seek(0);
    file.write((uint8_t*)content, strlen(content));
    file.close();
}

void loop(){
    server.doLoop();
}