/**
 * @file serverLoggingStream.ino
 * @author Phil Schatzmann
 * @brief Instead of logging messages to serial you can log them to the Server!
 * @version 0.1
 * @date 2022-09-03
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include "HttpServer.h"
#include "HttpExtensions.h"

const char* ssid = "SSID";
const char* password = "PASSWORD";
WiFiServer wifi;
HttpServer server(wifi);
ExtensionLoggingStream logger("/");
Ticker ticker; 

void printMsg(void*){
    static int count;
    char str[20];
    sprintf(str,"counter: %d",count++);
    // log to server
    stream.println(str);
    // log to serial
    Serial.println(str);
}

void setup() {
    Serial.begin(115200);
    // connect to WIFI
    Log.setLogger(Serial,Info);

    // generate test messages every second
    ticker.schedule(1000,&printMsg);

    // register logger extension and start server
    server.addExtension(logger);
    server.begin(80, ssid, password);
    
}

void loop(){
    server.doLoop();
    ticker.doLoop();
}