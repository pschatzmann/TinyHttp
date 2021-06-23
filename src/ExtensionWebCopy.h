#ifndef __EXTENSIONWEBDUMP_H__
#define __EXTENSIONWEBDUMP_H__

#include "ExtensionSD.h"
#include "WebCopy.h"

namespace tinyhttp {

/**
 * @brief Extension which starts the serving of a website from the SD Drive (using the SD extension). 
 * In addition it supports the dump post request which starts the download of an external web site to the SD drive.
 * 
 */

class ExtensionWebCopy : public Extension {
    public:
        ExtensionWebCopy(const char* startUrl, bool autoStart=false,  SDFileNameMgr *fileNamePtr = nullptr) {
            this->web_copy = new WebCopy(true, fileNamePtr);
            this->start_url = startUrl;
            this->auto_start = autoStart;
        }

        ~ExtensionWebCopy(){
            delete web_copy;
        }

        virtual void open(HttpServer *server) {
            Log.log(Info,"ExtensionWebCopy","open");
            // register SD logic
            sd.open(server);

            // add handler for /dump post request
            auto lambda = [](HttpServer *server,const char*requestPath, HttpRequestHandlerLine *hl) {   
                ExtensionWebCopy *ext = static_cast<ExtensionWebCopy*>(hl->context[0]);
                WebCopy* web_copy = ext->webCopy();
                const char* reply = web_copy->isActive() ? "<html><body>Dump is already running...</body></html>" : "<html><body>Dump will be started...</body></html>" ;            
                server->reply("text/html", reply, 200, SUCCESS);
                // start the dump
                if (!web_copy->isActive()) {
                    web_copy->start(ext->startUrl());
                }
            };

            HttpRequestHandlerLine *hl = new HttpRequestHandlerLine(1);
            hl->context[0] = this;
            hl->path = "/dump";
            hl->method = POST;
            hl->fn = lambda;
            server->addHandler(hl);

            // start the download by the registration
            if (this->auto_start){
                web_copy->start(start_url);
            }

        };

        virtual const char* startUrl() {
            return start_url;
        }

        virtual WebCopy *webCopy(){
            return web_copy;
        }

        virtual void doLoop() {
            sd.doLoop();
            web_copy->doLoop();
        };

    protected:
        ExtensionSD sd;  
        WebCopy *web_copy;
        const char* start_url;
        bool auto_start;
};

}

#endif // __EXTENSIONWEBDUMP_H__