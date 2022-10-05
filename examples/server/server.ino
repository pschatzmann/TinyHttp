/**
 * @file server.ino
 * @author SSID (phil.schatzmann@gmail.com)
 * @brief  We start a Webserver and demonstrate how to define the output by providing strings
 * or by defining a lambda function.
 * @version 0.1
 * @date 2020-11-30
 * 
 * @copyright Copyright (c) 2020
 * 
 */

#include "HttpServer.h"

const char* ssid = "SSID";
const char* password = "PASSWORD";
WiFiServer wifi;
HttpServer server(wifi);
Url indexUrl("/index.html");

void setup() {
    Serial.begin(115200);
    // connect to WIFI
    HttpLogger.begin(Serial,Info);

    const char*htmlHallo = 
        "<!DOCTYPE html>"
        "<html>"
        "<body style='background-color:black; color:white'>"
        "<h1>Arduino Http Server</h1>"
        "<p>Hallo world...</p>"
        "</body>"
        "</html>";

    auto countLambda = [](HttpServer *server, const char* requestPath, HttpRequestHandlerLine *hl) { 
        char buffer[2048];
        Str str(buffer, 2048);
        str += "<!DOCTYPE html>";
        str += "<html>";
        str += "<body style='background-color:black; color:white'>";
        str += "<h1>Counting Callback Example </h1> ";
        
        for (int j=0;j<100;j++){
          str+= "test ";
          str += j;
          str += "<br>";
        }
        str += "</body></html>";
        
        server->reply("text/html", str.c_str(), 200, SUCCESS);
    };

    server.rewrite("/","/hallo.html");
    server.rewrite("/index.html","/hallo.html");

    server.on("/hallo.html",GET,"text/html",htmlHallo);
    server.on("/count",GET, countLambda);
    server.on("/moved", GET, indexUrl);

    server.begin(80, ssid, password);
    
}

void loop(){
    server.doLoop();
}