
/**
 * @file ticker.ino
 * @author Phil Schatzmann
 * @brief The ticker executes a scheduled function in a defined interval
 * @version 0.1
 * @date 2022-09-03
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include "HttpServer.h"

Ticker ticker;

void hallo(void*) {
    Serial.println("hallo...");
    HttpLogger.begin(Serial,Info);
}

void setup(){
    Serial.begin(115200);
    ticker.schedule(1000, &hallo, millis()+2000, millis()+10000);

    auto lambda = [](void*) { Serial.println("hallo from lambda"); };
    ticker.schedule(10000, lambda);
}

void loop(){
    ticker.doLoop();
    delay(100);
}