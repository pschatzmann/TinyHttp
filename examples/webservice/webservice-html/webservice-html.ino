/**
 * @file webservice
 * @author Phil Schatzmann
 * @brief We extend the webservice into an application with a form
 * @version 0.1
 * @date 2022-10-05
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#include "HttpServer.h"
#include "ArduinoJson.h"

// json parameters
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
            <script src = \"https://ajax.googleapis.com/ajax/libs/jquery/2.1.3/jquery.min.js\"></script>\
            <script>\
                // load values\
                $(document).ready(function() {\
                    $.getJSON('./service', function(data) {\
                        $(\"input[name='volumeControl']\").val(data.volumeControl);\
                        $(\"input[name='clipThreashold']\").val(data.clipThreashold);\
                        $(\"input[name='fuzzEffectValue']\").val(data.fuzzEffectValue);\
                        $(\"input[name='distortionControl']\").val(data.distortionControl);\
                        $(\"input[name='tremoloDuration']\").val(data.tremoloDuration);\
                    });\
                });\
                // submit form\
                $( '#effect-form' ).submit(function( event ) {\
                    // Stop form from submitting normally\
                    event.preventDefault();\
                    // Get the values from elements on the page:\
                    var $form = $( this ),\
                        clipThreashold = $form.find( \"input[name='clipThreashold']\" ).val(),\
                        fuzzEffectValue = $form.find( \"input[name='fuzzEffectValue']\" ).val(),\
                        distortionControl = $form.find( \"input[name='distortionControl']\" ).val(),\
                        tremoloDuration = $form.find( \"input[name='tremoloDuration']\" ).val(),\
                        tremoloDepth = $form.find( \"input[name='tremoloDepth']\" ).val(),\
                        url = $form.attr( 'action' );\
                    // Send the data using post\
                    var posting = $.post( url, { volumeControl: volumeControl,\ 
                        clipThreashold: clipThreashold,\
                        fuzzEffectValue : fuzzEffectValue,\
                        distortionControl: distortionControl,\
                        tremoloDuration: tremoloDuration,\
                        tremoloDepth: tremoloDepth\
                    } );\
                });\
            </script>\
        </head>\
        <body>\
            <h1>Effect Parameters:</h1>\
            <form id='effect-form' action='/service' >\
                <div>\
                    <input type='range' id='volume' name='volumeControl'\
                            min='0' max='1' step='0.01' value='0'>\
                    <label for='volume'>Volume</label>\
                </div>\
                <div>\
                    <input type='range' id='clipThreashold' name='clipThreashold' \
                            min='0' max='6000' step='100' value='0'>\
                    <label for='clipThreashold'>Clip Threashold</label>\
                </div>\
                <div>\
                    <input type='range' id='fuzzEffectValue' name='clipThfuzzEffectValuereashold' \
                            min='0' max='12' step='0.1' value='0'>\
                    <label for='fuzzEffectValue'>Fuzz</label>\
                </div>\
                <div>\
                    <input type='range' id='distortionControl' name='distortionControl' \
                            min='0' max='8000' step='100' value='0'>\
                    <label for='distortionControl'>Distortion</label>\
                </div>\
                <div>\
                    <input type='range' id='tremoloDuration' name='tremoloDuration' \
                            min='0' max='500' step='1' value='0'>\
                    <label for='tremoloDuration'>Tremolo Duration</label>\
                </div>\
                <div>\
                    <input type='range' id='tremoloDepth' name='tremoloDepth' \
                            min='0' max='1' step='0.1' value='0'>\
                    <label for='tremoloDepth'>Tremolo Depth</label>\
                </div>\
                <button type='submit'>Submit</button>\
            </form>\
        </body>\
    </html>";

void parameters2Json(Stream &out) {
    DynamicJsonDocument doc(1024);
    doc["volumeControl"] = volumeControl;
    doc["clipThreashold"]   = clipThreashold;
    doc["fuzzEffectValue"] = fuzzEffectValue;
    doc["distortionControl"] = distortionControl;
    doc["tremoloDuration"] = tremoloDuration;
    doc["tremoloDepth"] = tremoloDepth;
    serializeJson(doc, out);
}

void json2Parameters(Stream &in) {
    DynamicJsonDocument doc(1024);
    deserializeJson(doc, in);
    volumeControl = doc["volumeControl"];
    clipThreashold = doc["clipThreashold"];
    fuzzEffectValue = doc["fuzzEffectValue"];
    distortionControl = doc["distortionControl"];
    tremoloDuration = doc["tremoloDuration"];
    tremoloDepth = doc["tremoloDepth"];
}

// Arduino Setup
void setup(void) {
    Serial.begin(115200);
    HttpLogger.begin(Serial, Warning);
    
    auto getJson = [](HttpServer *server, const char*requestPath, HttpRequestHandlerLine *hl) { 
        // provide data as json using callback 
        server->reply("text/json", parameters2Json, 200);
    };
    
    auto postJson = [](HttpServer *server, const char*requestPath, HttpRequestHandlerLine *hl) { 
        // post json to server
        json2Parameters(server->client());
        server->replyOK();
    };

    server.on("/",GET,"text/html", htmlForm);
    server.on("/service",GET, getJson);
    server.on("/service",POST, postJson);
    server.begin(80, ssid, password);

}

// Arduino loop - copy data
void loop() {
    server.copy();
}