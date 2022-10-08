#pragma once
#include "Server/Url.h"
#include "Server/HttpRequest.h"

namespace tinyhttp {

/**
 * @brief Forwards a request to a destination URL and provides a pointer to the result stream 
 * 
 */
class HttpTunnel {
    public:
        HttpTunnel(const char* url, const char* mime="text/html"){
            v_url.setUrl(url);
            v_mime = mime;
        }

        /// Executes the get request
        Stream *get() {
            if (isOk(v_request.get(v_url, v_mime))){
                return v_request.client();
            }
            return nullptr;
        }

        HttpRequest& request() {
            return v_request;
        }

        const char *mime() {
            return v_mime;
        }


    protected:
        Url v_url;
        HttpRequest v_request;
        const char* v_mime;

        bool isOk(int code){
            return code==200;
        }

};

}