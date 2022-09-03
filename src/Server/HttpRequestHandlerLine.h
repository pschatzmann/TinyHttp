#ifndef __HTTPREQUESTHANDLERLINE__
#define __HTTPREQUESTHANDLERLINE__


#include "Server/HttpHeader.h"

namespace tinyhttp {


// forward declarations for the callback
class HttpServer;
class HttpRequestHandlerLine;

// Callback function which provides result
typedef void (*web_callback_fn)(HttpServer *server, const char* requestPath, HttpRequestHandlerLine *handlerLine);

/**
 * @brief used to register and process callbacks 
 * 
 */
class HttpRequestHandlerLine {
    public:
        HttpRequestHandlerLine(int ctxSize=0){
            Log.log(Info,"HttpRequestHandlerLine");
            contextCount = ctxSize;
            context = new void*[ctxSize];
        }

        ~HttpRequestHandlerLine(){
            Log.log(Info,"~HttpRequestHandlerLine");
            for (int j=0;j<contextCount;j++){
                // if (context[j]!=nullptr){
                //     Log.log(Info,"HttpRequestHandlerLine","delete context");
                //     delete[]  context[j];
                // }
            }
            if (contextCount>0){
                Log.log(Info,"HttpRequestHandlerLine","free");
                delete[] context;
            }
        }

        MethodID method;
        const char* path;
        web_callback_fn fn;
        void** context;
        int contextCount;
};

}

#endif // __HTTPREQUESTHANDLERLINE__