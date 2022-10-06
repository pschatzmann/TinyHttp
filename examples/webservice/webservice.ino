/**
 * @file webservice
 * @author Phil Schatzmann
 * @brief A very simple webservice using json
 * @version 0.1
 * @date 2022-10-05
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#include "HttpServer.h"
#include "ArduinoJson.h"

// json parameters
float volumeControl = 1.0;
int16_t clipThreashold = 4990;
float fuzzEffectValue = 6.5;

// Server
WiFiServer wifi;
HttpServer server(wifi);
const char *ssid = "SSID";
const char *password = "password";


// Arduino Setup
void setup(void) {
    Serial.begin(115200);
    HttpLogger.begin(Serial, Warning);
    
    auto getJson = [](HttpServer *server, const char*requestPath, HttpRequestHandlerLine *hl) { 
        // reply using callback 
        const int max_len = 160;
        char json[max_len];
        snprintf(json, max_len, "{volumeControl: %f, clipThreashold: %d, fuzzEffectValue: %f }", volumeControl, clipThreashold, fuzzEffectValue );
        server->reply("text/json", json, 200);
    };
    
    server.rewrite("/","/service");
    server.on("/service",GET, getJson);
    server.begin(80, ssid, password);
}

// Arduino loop - copy data
void loop() {
    server.copy();
}