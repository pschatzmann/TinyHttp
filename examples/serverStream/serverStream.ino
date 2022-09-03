/**
 * @file serverStream.ino
 * @author Phil Schatzmann
 * @brief Instead of logging messages to serial you can log them to the Server!
 * @version 0.1
 * @date 2022-09-03
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include "HttpServer.h"


// setup server 
const char* ssid = "SSID";
const char* password = "PASSWORD";
WiFiServer wifi;
HttpServer server(wifi);
const char* htmlHeader = "<html><body style='background-color:black; color:white'><h1>Streaming Example</h1>";
const char* htmlEnd = "</body></html>";
const char* mime = "text/html";
ExtensionStream stream("/stream",GET, mime, htmlHeader, htmlEnd );
Ticker ticker;


void setup() {
    Serial.begin(115200);
    Log.setLogger(Serial,Info);

    server.addExtension(stream);
    server.begin(80, ssid, password);

    // generate test messages
    ticker.schedule(1000,&printMsg);

}

void printMsg(void*){
    static int count;
    char str[20];
    sprintf(str,"counter: %d",count++);
    // log to server
    stream.println((const char*) str);
    // log to serial
    Serial.println(str);
}

void loop(){
    server.doLoop();
    ticker.doLoop();
}