
#include "Ticker.h"
using namespace tinyhttp;

Ticker ticker;

void hallo(void*) {
    Serial.println("hallo...");
    Log.setLogger(Serial,Info);
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