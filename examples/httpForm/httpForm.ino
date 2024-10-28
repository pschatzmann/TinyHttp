#include "HttpServer.h"

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
HttpParameters parameters;

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
            <form id='effect-form' method='T_POST' >\
                <div>\
                    <input type='range' id='volumeControl' name='volumeControl'\
                            onchange='this.form.submit()'\
                            min='0' max='1' step='0.01' value='%volumeControl%'>\
                    <label for='volumeControl'>Volume</label>\
                </div>\
                <div>\
                    <input type='range' id='clipThreashold' name='clipThreashold' \
                            onchange='this.form.submit()'\
                            min='0' max='6000' step='100' value='%clipThreashold%'>\
                    <label for='clipThreashold'>Clip Threashold</label>\
                </div>\
                <div>\
                    <input type='range' id='fuzzEffectValue' name='fuzzEffectValue' \
                            onchange='this.form.submit()'\
                            min='0' max='12' step='0.1' value='%fuzzEffectValue%'>\
                    <label for='fuzzEffectValue'>Fuzz</label>\
                </div>\
                <div>\
                    <input type='range' id='distortionControl' name='distortionControl' \
                            onchange='this.form.submit()'\
                            min='0' max='8000' step='100' value='%distortionControl%'>\
                    <label for='distortionControl'>Distortion</label>\
                </div>\
                <div>\
                    <input type='range' id='tremoloDuration' name='tremoloDuration' \
                            onchange='this.form.submit()'\
                            min='0' max='500' step='1' value='%tremoloDuration%'>\
                    <label for='tremoloDuration'>Tremolo Duration</label>\
                </div>\
                <div>\
                    <input type='range' id='tremoloDepth' name='tremoloDepth' \
                            onchange='this.form.submit()'\
                            min='0' max='1' step='0.1' value='%tremoloDepth%'>\
                    <label for='tremoloDepth'>Tremolo Depth</label>\
                </div>\
            </form>\
        </body>\
    </html>";


void printValues() {
    char msg[120];
    snprintf(msg, 120, "====> updated values %f %d %f %d %d %f",volumeControl, clipThreashold, fuzzEffectValue, distortionControl, tremoloDuration,tremoloDepth);
    Serial.println(msg);        
}

void getHtml(HttpServer *server, const char*requestPath, HttpRequestHandlerLine *hl) { 
    // provide html and replace variables with actual values
    Str html(2500);
    html.set(htmlForm);
    html.replace("%volumeControl%",volumeControl);
    html.replace("%clipThreashold%",clipThreashold);
    html.replace("%fuzzEffectValue%",fuzzEffectValue);
    html.replace("%distortionControl%",distortionControl);
    html.replace("%tremoloDuration%",tremoloDuration);
    html.replace("%tremoloDepth%",tremoloDepth);
    server->reply("text/html", html.c_str(), 200);
};

void postData(HttpServer *server, const char*requestPath, HttpRequestHandlerLine *hl) { 
    // parse the parameters
    parameters.parse(server->client());
    // update parameters
    volumeControl = parameters.getFloat("volumeControl");
    clipThreashold = parameters.getInt("clipThreashold");
    fuzzEffectValue = parameters.getFloat("fuzzEffectValue");
    distortionControl = parameters.getInt("distortionControl");
    tremoloDuration = parameters.getInt("tremoloDuration");
    tremoloDepth = parameters.getFloat("tremoloDepth"); 
    // return updated html       
    getHtml(server, requestPath, hl);
    printValues();
};

// Arduino Setup
void setup(void) {
    Serial.begin(115200);
    HttpLogger.begin(Serial, Warning);
    
    server.on("/",T_GET, getHtml);
    server.on("/",T_POST, postData);
    server.begin(80, ssid, password);
}

// Arduino loop - copy data
void loop() {
    server.copy();
}