#ifndef __HTTPSDSERVER_H__
#define __HTTPSDSERVER_H__

#include "HttpServer.h"
#include "MimeResolver.h"
#include <SPI.h>
#include <SD.h>

namespace tinyhttp {

/**
 * @brief Extension which serves the files which are available on the SD card
 * We return the full file content - so use this class only for small files!
 * You need to call SD.begin() yourself before registering the extension.
 */
class ExtensionSD : public Extension {
    public:    
        ExtensionSD(const char* path="/*", int cpin=-1){
            Log.log(Info,"ExtensionSD", path);
            this->path = path;
            if (cpin==-1){
                SD.begin();
            } else {
                SD.begin(cpin);
            }
        }

        virtual void open(HttpServer *server) {
            Log.log(Info,"ExtensionSD", "open");
            // define the file handler
            auto lambda = [](HttpServer *server_ptr,const char*requestPath, HttpRequestHandlerLine *hl) { 
                const char* path = requestPath;
                Log.log(Info,"ExtensionSD::lambda", path);
                if (SD.exists(path)){
                    File file = SD.open(path);
                    int size = file.size();
                    MimeResolver resolver;
                    const char* mime = resolver.getMime(file.name());
                    //  void reply(char* contentType, Stream &inputStream, int size, int status=200, char* msg=SUCCESS){
                    server_ptr->reply(mime, file, size, 200);
                } else {
                    Log.log(Error, "ExtensionSD::open - file does not exist",path);
                }
            };
            // register default SD File handler
            server->on(path, GET, lambda);
        }
        virtual void doLoop(){}

    protected:
        const char* path;

};

}

#endif // __HTTPSDSERVER_H__