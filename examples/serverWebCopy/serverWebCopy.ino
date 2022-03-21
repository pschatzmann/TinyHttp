
/**
 * @file serverDump.ino
 * @author SSID (phil.schatzmann@gmail.com)
 * @brief  We start a Webserver which serves the pages from a SD card and which also supports the download of an external
 * website to the SD.
 * @version 0.1
 * @date 2020-11-30
 * 
 * @copyright Copyright (c) 2020
 * 
 */

#include <WiFi.h>
#include "HttpServer.h"
#include "ExtensionWebCopy.h"

using namespace tinyhttp;

WiFiServer wifi;
HttpServer server(wifi);
Extension sd = ExtensionWebCopy("https://www.pschatzmann.ch/home/", true);

void setup() {
    Serial.begin(115200);
    // connect to WIFI
    WiFi.begin("network name", "password");
    while (WiFi.status() != WL_CONNECTED) {        
      delay(500);
      Serial.print(".");
    }

    server.addExtension(sd);
    server.begin(80);
    
}

void loop(){
    server.doLoop();
}