\mainpage
# TinyHttp

There is a good choice of different Http Server implementations for Arduino. Somehow however I was always fighting with some restrictions: E.g the server was crashing because it was running out of memory.

So I decided to to investigate if it is possible to achieve the following goals:

- Serving of a potentially __unbounded amount of data__ 
- __concurrent__ handling of __multiple requests__.
- Support for __Extensions__ to add additional functionality

In my alternative design I try to split the processing up into many small pieces: So instead of sending a single big reply we can use the chunked http transfer encoding to feed many clients with small replys.  

This should resolve also the memory restrictions for good because we do not have any upper totoal limit for the reply any more! 

The basic HttpServer class just provides the functionality to

- receive / interpret requests
- define rewrites
- register reply handlers (callbacks)
- register extensions
- provide replys 

In order to be able to support an unlimited number of more complex scenarios, I have designed the concept of flexible "Extensions" and to proof the usefulness I have implemented the support for the following extensions:

- Publishing of Output from an Arduino Stream 
- Music Streaming
- Serving web pages from SD
- Extracting an external website and save it to SD

I also added a small http client implementation by re-using the same functionality which was needed for the Server.

## Example - Basic Server

A simple demo that shows how to use the basic server functionality w/o extensions.

```
#include <WiFi.h>
#include "HttpServer.h"

using namespace tinyhttp;

// setup server with initial dump address
WiFiServer wifi;
HttpServer server(wifi);
Url indexUrl("/index.html");

void setup() {
    Serial.begin(115200);
    // connect to WIFI
    Log.setLogger(Serial,Info);

    WiFi.begin("SSID", "password");
    while (WiFi.status() != WL_CONNECTED) {        
      delay(500);
      Serial.print(".");
    }

    const char*htmlHallo = 
        "<!DOCTYPE html>"
        "<html>"
        "<body style='background-color:black; color:white'>"
        "<h1>Arduino Http Server</h1>"
        "<p>Hallo world...</p>"
        "</body>"
        "</html>";

    auto countLambda = [](HttpServer *server, HttpRequestHandlerLine *hl) { 
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

    server.begin(80);
    
}

int count;
void loop(){
    server.doLoop();
}
```

## Example - Streamed Http 

You can "publish" to an Arduino stream and look at the result from multiple browser windows: 

```
#include <WiFi.h>
#include "HttpServer.h"
#include "ExtensionStream.h"
#include "Ticker.h"

using namespace tinyhttp;

// setup server 
WiFiServer wifi;
HttpServer server(wifi);
const char* htmlHeader = "<html><body style='background-color:black; color:white'><h1>Streaming Example</h1>";
const char* htmlEnd = "</body></html>";
const char* mime = "text/html";
ExtensionStream stream("/stream",GET, mime, htmlHeader, htmlEnd );
Ticker Ticker;


void setup() {
    Serial.begin(115200);
    // connect to WIFI
    Log.setLogger(Serial,Info);

    WiFi.begin("SSID", "password");
    while (WiFi.status() != WL_CONNECTED) {        
      delay(500);
      Serial.print(".");
    }

    Ticker.schedule(1000,&printMsg);
    server.addExtension(stream);
    server.begin(80);
    
}

void printMsg(void*){
    static int count;
    char str[20];
    sprintf(str,"counter: %d",count++);
    stream.println((const char*) str);
    Serial.println(str);
}

void loop(){
    server.doLoop();
    Ticker.doLoop();
}

```
## Example - Server from SD

The following  example shows how you can setup a server that uses the SD drive as data souce:

```
#include <WiFi.h>
#include "HttpServer.h"
#include "ExtensionSD.h"

using namespace tinyhttp;

WiFiServer wifi;
HttpServer server(wifi);
Extension sd = ExtensionSD();

void setup() {
    Serial.begin(115200);
    // connect to WIFI
    WiFi.begin("network name", "password");
    while (WiFi.status() != WL_CONNECTED) {        
      delay(500);
      Serial.print(".");
    }

    server.addExtension(sd);
    server.begin(80);
    
}

void loop(){
    server.doLoop();
}
```