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

const char* ssid = "SSID";
const char* password = "PASSWORD";
WiFiServer wifi;
HttpServer server(wifi);
ExtensionLoggingStream stream("/log");
Ticker ticker;


void setup() {
    Serial.begin(115200);
    // connect to WIFI
    Log.setLogger(Serial,Info);

    // generate test messages
    ticker.schedule(1000,&printMsg);

    server.addExtension(stream);
    server.begin(80, ssid, password);
    
}

void printMsg(void*){
    static int count;
    char str[20];
    sprintf(str,"counter: %d",count++);
    // log to server
    stream.println(str);
    // log to serial
    Serial.println(str);
}

void loop(){
    server.doLoop();
    ticker.doLoop();
}