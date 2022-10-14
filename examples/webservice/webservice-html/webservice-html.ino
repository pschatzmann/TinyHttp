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
    "<!DOCTYPE html>\n\
    <html>\n\
        <head>\n\
            <title>Effects</title>\n\
            <script src = \"https://ajax.googleapis.com/ajax/libs/jquery/2.1.3/jquery.min.js\"></script>\n\
            <script>\n\
                $(document).ready(function() {\n\
                    // load values\n\
                    $.getJSON('./service', function(data) {\n\
                        $('#volumeControl').val(data.volumeControl);\n\
                        $('#clipThreashold').val(data.clipThreashold);\n\
                        $('#fuzzEffectValue').val(data.fuzzEffectValue);\n\
                        $('#distortionControl').val(data.distortionControl);\n\
                        $('#tremoloDuration').val(data.tremoloDuration);\n\
                    });\n\
                    // submit form\n\
                    $( '#effect-form' ).submit(function( event ) {\n\
                        event.preventDefault();\n\
                        const data = new FormData(event.target);\n\
                        const value = Object.fromEntries(data.entries());\n\
                        const json = JSON.stringify(value);\n\
                        // send ajax\n\
                        $.ajax({\n\
                            url: './service',\n\
                            type: 'T_POST',\n\
                            dataType: 'json',\n\
                            data: json,\n\
                            contentType: 'text/json',\n\
                            success : function(result) {console.log(json);},\n\
                            error: function(xhr, resp, text) {\n\
                                console.log(json, xhr, resp, text);\n\
                                alert(text);\n\
                            }\n\
                        });\
                    });\
                });\
            </script>\
        </head>\n\
        <body>\n\
            <h1>Effects:</h1>\
            <form id='effect-form' method='post' >\n\
                <div>\
                    <input type='range' id='volumeControl' name='volumeControl'\
                            min='0' max='1' step='0.01' value='0' onchange=\"$('#effect-form').submit();\">\
                    <label for='volume'>Volume</label>\
                </div>\n\
                <div>\
                    <input type='range' id='clipThreashold' name='clipThreashold' \
                            min='0' max='6000' step='100' value='0' onchange=\"$('#effect-form').submit();\">\
                    <label for='clipThreashold'>Clip Threashold</label>\
                </div>\n\
                <div>\
                    <input type='range' id='fuzzEffectValue' name='fuzzEffectValue' \
                            min='0' max='12' step='0.1' value='0' onchange=\"$('#effect-form').submit();\">\
                    <label for='fuzzEffectValue'>Fuzz</label>\
                </div>\n\
                <div>\
                    <input type='range' id='distortionControl' name='distortionControl' \
                            min='0' max='8000' step='100' value='0' onchange=\"$('#effect-form').submit();\">\
                    <label for='distortionControl'>Distortion</label>\
                </div>\n\
                <div>\
                    <input type='range' id='tremoloDuration' name='tremoloDuration' \
                            min='0' max='500' step='1' value='0' onchange=\"$('#effect-form').submit();\">\
                    <label for='tremoloDuration'>Tremolo Duration</label>\
                </div>\n\
                <div>\
                    <input type='range' id='tremoloDepth' name='tremoloDepth' \
                            min='0' max='1' step='0.1' value='0' onchange=\"$('#effect-form').submit();\">\
                    <label for='tremoloDepth'>Tremolo Depth</label>\
                </div>\n\
            </form>\n\
        </body>\n\
    </html>\n";


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

void addCors(HttpReplyHeader &header){
    header.put("Access-Control-Allow-Origin","*");
    header.put("Access-Control-Allow-Credentials", "true");
    header.put("Access-Control-Allow-Methods", "T_GET,HEAD,OPTIONS,T_POST,PUT");
    header.put("Access-Control-Allow-Headers", "Access-Control-Allow-Headers, Origin,Accept, X-Requested-With, Content-Type, Access-Control-Request-Method, Access-Control-Request-Headers");
}

void printValues() {
    char msg[120];
    snprintf(msg, 120, "====> updated values %f %d %f %d %d %f",volumeControl, clipThreashold, fuzzEffectValue, distortionControl, tremoloDuration,tremoloDepth);
    Serial.println(msg);        
}

// Arduino Setup
void setup(void) {
    Serial.begin(115200);
    HttpLogger.begin(Serial, Warning);
    
    auto getJson = [](HttpServer *server, const char*requestPath, HttpRequestHandlerLine *hl) { 
        // provide data as json using callback 
        addCors(server->replyHeader());
        server->reply("text/json", parameters2Json, 200);
    };
    
    auto postJson = [](HttpServer *server, const char*requestPath, HttpRequestHandlerLine *hl) { 
        // post json to server
        json2Parameters(server->client());
        addCors(server->replyHeader());
        server->reply("text/json","{}",200);

        // log updated result
        printValues();
    };

    auto replyOK = [](HttpServer *server, const char*requestPath, HttpRequestHandlerLine *hl) { 
        addCors(server->replyHeader());
        server->replyOK();
    };

    server.on("/",T_GET,"text/html", htmlForm);
    server.on("/service",T_OPTIONS, replyOK);
    server.on("/service",T_GET, getJson);
    server.on("/service",T_POST, postJson);
    server.on("/favicon.ico",T_GET, replyOK);
    server.begin(80, ssid, password);

}

// Arduino loop - copy data
void loop() {
    server.copy();
}