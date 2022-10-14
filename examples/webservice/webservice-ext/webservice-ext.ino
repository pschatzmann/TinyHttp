/**
 * @file webservice
 * @author Phil Schatzmann
 * @brief A simple webservice that supports T_GET and T_POST using ArduinoJson 
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
int16_t distortionControl = 4990;
int16_t tremoloDuration = 200;
float tremoloDepth = 0.5;

// Server
WiFiServer wifi;
HttpServer server(wifi);
const char *ssid = "SSID";
const char *password = "password";

void parameters2Json(Stream &out) {
    DynamicJsonDocument doc(1024);
    doc["volumeControl"] = volumeControl;
    doc["clipThreashold"]   = clipThreashold;
    doc["fuzzEffectValue"] = fuzzEffectValue;
    doc["distortionControl"] = distortionControl;
    doc["tremoloDuration"] = tremoloDuration;
    doc["tremoloDepth"] = tremoloDepth;
    serializeJson(doc, out);
}

void json2Parameters(Stream &in) {
    DynamicJsonDocument doc(1024);
    deserializeJson(doc, in);
    volumeControl = doc["volumeControl"];
    clipThreashold = doc["clipThreashold"];
    fuzzEffectValue = doc["fuzzEffectValue"];
    distortionControl = doc["distortionControl"];
    tremoloDuration = doc["tremoloDuration"];
    tremoloDepth = doc["tremoloDepth"];
}

// Arduino Setup
void setup(void) {
    Serial.begin(115200);
    HttpLogger.begin(Serial, Warning);
    
    auto getJson = [](HttpServer *server, const char*requestPath, HttpRequestHandlerLine *hl) { 
        // reply using callback 
        server->reply("text/json", parameters2Json, 200);
    };
    
    auto postJson = [](HttpServer *server, const char*requestPath, HttpRequestHandlerLine *hl) { 
        // read content from client
        json2Parameters(server->client());
        server->replyOK();
    };

    server.rewrite("/","/service");
    server.on("/service",T_GET, getJson);
    server.on("/service",T_POST, postJson);
    server.begin(80, ssid, password);

}

// Arduino loop - copy data
void loop() {
    server.copy();
}