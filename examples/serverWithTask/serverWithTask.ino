/**
 * @file server.ino
 * @author SSID (phil.schatzmann@gmail.com)
 * @brief  We start a Webserver. No processing is required in the loop! 
 * @version 0.1
 * @date 2020-11-30
 * 
 * @copyright Copyright (c) 2020
 * 
 */

#include "HttpServer.h"
#include "HttpExtensions.h"

const char* ssid = "SSID";
const char* password = "PASSWORD";
WiFiServer wifi;
HttpServer server(wifi);
ExtensionESPTask task;  // calls server.doLoop(); in an ESP32 task

void setup() {
    Serial.begin(115200);
    // connect to WIFI
    HttpLogger.setLogger(Serial,Info);

    const char*htmlHallo = 
        "<!DOCTYPE html>"
        "<html>"
        "<body style='background-color:black; color:white'>"
        "<h1>Arduino Http Server</h1>"
        "<p>Hallo world...</p>"
        "</body>"
        "</html>";

    // define urls
    server.rewrite("/","/hallo.html");
    server.rewrite("/index.html","/hallo.html");
    server.on("/hallo.html",GET,"text/html",htmlHallo);

    // start server with task
    server.addExtension(task);
    server.begin(80, ssid, password);
    
}

void loop(){
}