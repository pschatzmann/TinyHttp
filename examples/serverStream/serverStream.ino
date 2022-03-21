#include <WiFi.h>
#include "HttpServer.h"
#include "ExtensionStream.h"
#include "Ticker.h"

using namespace tinyhttp;

// setup server 
WiFiServer wifi;
HttpServer server(wifi);
const char* htmlHeader = "<html><body style='background-color:black; color:white'><h1>Streaming Example</h1>";
const char* htmlEnd = "</body></html>";
const char* mime = "text/html";
ExtensionStream stream("/stream",GET, mime, htmlHeader, htmlEnd );
Ticker ticker;


void setup() {
    Serial.begin(115200);
    // connect to WIFI
    Log.setLogger(Serial,Info);

    WiFi.begin("SSID", "password");
    while (WiFi.status() != WL_CONNECTED) {        
      delay(500);
      Serial.print(".");
    }

    ticker.schedule(1000,&printMsg);

    server.addExtension(stream);
    server.begin(80);
    
}

void printMsg(){
    static int count;
    char str[20];
    sprintf(str,"counter: %d",count++);
    stream.println((const char*) str);
    Serial.println(str);
}

void loop(){
    server.doLoop();
    ticker.doLoop();
}