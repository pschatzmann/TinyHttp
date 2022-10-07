#include "HttpServer.h"
#include "ArduinoJson.h"

// parameters
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
const char* htmlForm = 
    "<!DOCTYPE html>\
    <html>\
        <head>\
            <title>Effect Parameters</title>\
        </head>\
        <body>\
            <h1>Effect Parameters:</h1>\
            <form id='effect-form' action='/service' >\
                <div>\
                    <input type='range' id='volumeControl' name='volumeControl'\
                            min='0' max='1' step='0.01' value='%volumeControl%'>\
                    <label for='volumeControl'>Volume</label>\
                </div>\
                <div>\
                    <input type='range' id='clipThreashold' name='clipThreashold' \
                            min='0' max='6000' step='100' value='%clipThreashold%'>\
                    <label for='clipThreashold'>Clip Threashold</label>\
                </div>\
                <div>\
                    <input type='range' id='fuzzEffectValue' name='fuzzEffectValue' \
                            min='0' max='12' step='0.1' value='%fuzzEffectValue%'>\
                    <label for='fuzzEffectValue'>Fuzz</label>\
                </div>\
                <div>\
                    <input type='range' id='distortionControl' name='distortionControl' \
                            min='0' max='8000' step='100' value='%distortionControl%'>\
                    <label for='distortionControl'>Distortion</label>\
                </div>\
                <div>\
                    <input type='range' id='tremoloDuration' name='tremoloDuration' \
                            min='0' max='500' step='1' value='%tremoloDuration%'>\
                    <label for='tremoloDuration'>Tremolo Duration</label>\
                </div>\
                <div>\
                    <input type='range' id='tremoloDepth' name='tremoloDepth' \
                            min='0' max='1' step='0.1' value='%tremoloDepth%'>\
                    <label for='tremoloDepth'>Tremolo Depth</label>\
                </div>\
                <button type='submit'>Submit</button>\
            </form>\
        </body>\
    </html>";


void printValues() {
    char msg[120];
    snprintf(msg, 120, "====> updated values %f %d %f %d %d %f",volumeControl, clipThreashold, fuzzEffectValue, distortionControl, tremoloDuration,tremoloDepth);
    Serial.println(msg);        
}

// Arduino Setup
void setup(void) {
    Serial.begin(115200);
    HttpLogger.begin(Serial, Warning);
    
    auto getHtml = [](HttpServer *server, const char*requestPath, HttpRequestHandlerLine *hl) { 
        // provide data as json using callback 
        StrExt html(1024);
        html.set(htmlForm);
        html.replace("%volumeControl%",volumeControl);
        html.replace("%clipThreashold%",clipThreashold);
        html.replace("%fuzzEffectValue%",fuzzEffectValue);
        html.replace("%distortionControl%",distortionControl);
        html.replace("%tremoloDuration%",tremoloDuration);
        html.replace("%tremoloDepth%",tremoloDepth);
        server->reply("text/html", html.c_str(), 200);
    };
    
    auto postData = [](HttpServer *server, const char*requestPath, HttpRequestHandlerLine *hl) { 
        HttpParameters parameters;
        parameters.parse(server->client());
        volumeControl = parameters.getFloat("volumeControl");
        clipThreashold = parameters.getFloat("clipThreashold");
        fuzzEffectValue = parameters.getFloat("fuzzEffectValue");
        distortionControl = parameters.getFloat("distortionControl");
        tremoloDuration = parameters.getFloat("tremoloDuration");
        tremoloDepth = parameters.getFloat("tremoloDepth");        
        server->replyOK();
        printValues();
    };

    server.on("/",GET, getHtml);
    server.on("/",POST, postData);
    server.begin(80, ssid, password);

}

// Arduino loop - copy data
void loop() {
    server.copy();
}