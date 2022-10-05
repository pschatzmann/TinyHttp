#include <WiFi.h>
#include "Utils/WebCopy.h"
#include "WiFiClientSecure.h"

using namespace tinyhttp;

WiFiClientSecure client;

void setup() {
    Serial.begin(115200);
    // connect to WIFI
    WiFi.begin("SSID", "password");
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }

    // setup logger
    HttpLogger.begin(Serial, Info);

    // start the dump 
    WebCopy webCopy(client, false);
    webCopy.start("https://www.pschatzmann.ch/home");
}

void loop() {
}