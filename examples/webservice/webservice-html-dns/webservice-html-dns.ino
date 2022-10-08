/**
 * @file webservice
 * @author Phil Schatzmann
 * @brief We extend the webservice into an application with a form
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

void addCors(HttpReplyHeader &header){
    header.put("Access-Control-Allow-Origin","*");
    header.put("Access-Control-Allow-Credentials", "true");
    header.put("Access-Control-Allow-Methods", "GET,HEAD,OPTIONS,POST,PUT");
    header.put("Access-Control-Allow-Headers", "Access-Control-Allow-Headers, Origin,Accept, X-Requested-With, Content-Type, Access-Control-Request-Method, Access-Control-Request-Headers");
}

void printValues() {
    char msg[120];
    snprintf(msg, 120, "====> updated values %f %d %f %d %d %f",volumeControl, clipThreashold, fuzzEffectValue, distortionControl, tremoloDuration,tremoloDepth);
    Serial.println(msg);        
}

// Arduino Setup
void setup(void) {
    Serial.begin(115200);
    HttpLogger.begin(Serial, Warning);

    if (!MDNS.begin("esp32-service")) {
        Serial.println("Could not set up DNS");
        return;
    }

    
    auto getJson = [](HttpServer *server, const char*requestPath, HttpRequestHandlerLine *hl) { 
        // provide data as json using callback 
        addCors(server->replyHeader());
        server->reply("text/json", parameters2Json, 200);
    };
    
    auto postJson = [](HttpServer *server, const char*requestPath, HttpRequestHandlerLine *hl) { 
        // post json to server
        json2Parameters(server->client());
        addCors(server->replyHeader());
        server->reply("text/json","{}",200);

        // log updated result
        printValues();
    };

    auto replyOK = [](HttpServer *server, const char*requestPath, HttpRequestHandlerLine *hl) { 
        addCors(server->replyHeader());
        server->replyOK();
    };

    Url url("");
    server.on("/",GET,"text/html", url);
    server.on("/service",OPTIONS, replyOK);
    server.on("/service",GET, getJson);
    server.on("/service",POST, postJson);
    server.on("/favicon.ico",GET, replyOK);
    server.begin(80, ssid, password);

}

// Arduino loop - copy data
void loop() {
    server.copy();
}