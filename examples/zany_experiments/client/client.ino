
// example for the http GET request

#include "HttpServer.h"

#ifdef ARDUINO
#include <WiFiClientSecure.h>
WiFiClientSecure client;
#else
WiFiClient client;
#endif

HttpRequest request(client);
Url url("https://www.pschatzmann.ch/home/category/arduino/");
UrlExtractor urlExtractor("https://www.pschatzmann.ch");

uint8_t buffer[1024];

void setup() {
    Serial.begin(115200);
    HttpLogger.begin(Serial,Info);
    // connect to WIFI
    WiFi.begin("SSID", "password");
    while (WiFi.status() != WL_CONNECTED) {        
      delay(500);
      Serial.print(".");
    }
    
    // Connect to server
    request.get(url);

    // Print some result headers
    Serial.print("Status Code: ");
    Serial.println(request.reply().statusCode());
    Serial.print("Status Message: ");
    Serial.println(request.reply().statusMessage());
    Serial.print("Mime: ");
    Serial.println(request.reply().get(CONTENT_TYPE));

    Serial.println("Result Data:");
    while(request.available()>0){
          // you can use read or readln
          int len = request.read(buffer, 1023);
          Serial.write(buffer, len);
    }
    
    Serial.println("DONE");

}

void loop(){

}