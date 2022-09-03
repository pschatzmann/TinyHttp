# TinyHttp (Media) Server

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

I also added a small http client implementation by re-using the same functionality which was needed for the Server.

## Example: Basic Server

A simple demo that shows how to use the basic server functionality w/o extensions.

```
#include "HttpServer.h"

// setup server with initial dump address
WiFiServer wifi;
HttpServer server(wifi);
Url indexUrl("/index.html");

void setup() {
    Serial.begin(115200);
    Log.setLogger(Serial,Info);

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

    server.begin(80, "SSID", "password");
    
}

void loop(){
    server.doLoop();
}

```
## Extensions

In order to be able to support an unlimited number of more complex scenarios, I have designed the concept of __flexible "Extensions"__ and to proof the usefulness I have implemented the support for the following extensions:

- Publishing of Output from an Arduino Stream 
- Music Streaming to multiple clients
- Serving web pages from SD

### Example - Logging with Http 

You can "write log messages" to an Arduino stream and look at the result from multiple browser windows: 

```
#include "HttpServer.h"

// setup server 
WiFiServer wifi;
HttpServer server(wifi);
ExtensionLoggingStream stream("/log" );
Ticker ticker;


void setup() {
    Serial.begin(115200);
    Log.setLogger(Serial,Info);

    ticker.schedule(1000,&printMsg);
    server.addExtension(stream);
    server.begin(80,"SSID", "password" );
    
}

void printMsg(void*){
    static int count;
    char str[20];
    sprintf(str,"counter: %d",count++);
    stream.println(str); // log to server
    Serial.println(str); // log to Serial
}

void loop(){
    server.doLoop();
    ticker.doLoop();
}

```

Further extension examples can be found in the examples directory!
