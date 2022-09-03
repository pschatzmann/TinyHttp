#pragma once

#include "Server/HttpServer.h"
#include "Server/HttpStreamCopy.h"
#include "Utils/MimeResolver.h"
#include <SPI.h>
#include <SD.h>

namespace tinyhttp {

/**
 * @brief Extension which serves the files which are available on the SD card
 * The content is returned in chunks via the processing loop
 */
class ExtensionSDStreamed : public Extension {
    public:    
        ExtensionSDStreamed(const char* path="/*", int cspin=-1){
        Log.log(Info,"ExtensionSDStreamed", path);
        this->path = path;
        this->sd_cs = cspin;
       }

        virtual void open(HttpServer *server) {
            setupSD();
            // define the file handler
            auto lambda = [this](HttpServer *server_ptr, const char*requestPath, HttpRequestHandlerLine *hl) { 
                Url url(server_ptr->requestHeader().urlPath());
                const char* path = url.path();
                Log.log(Info,"ExtensionSD::lambda", path);
                if (SD.exists(path)){
                    // provide reply header
                    MimeResolver resolver;
                    const char* mime = resolver.getMime(path);
                    server_ptr->replyChunked(mime, 200);

                    // setup incremental reply chunks
                    File file = SD.open(path);
                    HttpStreamCopy *out = new HttpStreamCopy(file, server_ptr->client());
                    output->push_back(out);

                } 
            };
            // register default SD File handler
            server->on(path, GET, lambda);
        }

        // process chunk for each open request
        virtual void doLoop(){
            for (auto i = output.begin(); i != output.end(); ++i) {
                HttpStreamCopy *out = (*i);
                if (out->isOpen()){ 
                    out->doLoop();
                }
            }  
            cleanup();      
        }

        // remove closed output from further processing
        void cleanup() {
            for (auto i = output.begin(); i != output.end(); ++i) {
                HttpStreamCopy *out = (*i);
                if (!out->isOpen()){ 
                    output.erase(i);
                    delete out;
                }
            }        
        }

    protected:
        Vector<HttpStreamCopy*> output;
        const char* path;
        int sd_cs;
        bool is_open = false;

        void setupSD() {
            if (!is_open) {
                if (sd_cs==-1){
                    SD.begin();
                } else {
                    SD.begin(sd_cs);
                }
                is_open = true;
            }
        }

};

}

