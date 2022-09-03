#pragma once

#include "Server/HttpStreamedOutput.h"
//#include "Server/HttpHeader.h"
//#include "Server/HttpHandler.h"
#include "Server/HttpServer.h"
#include "Client.h"

namespace tinyhttp {


/**
 * @brief Extension which supports streaming to multiple clients. It makes sure that same content is streamed 
 * to multiple clients when you call the writes or printlns methods in your loop();
 * Only a subset of the stream output functions is avaiblable!
 */

class ExtensionStreamShared : public Extension  {
    public:
        ExtensionStreamShared( const char* url,  HttpStreamedOutput &out, MethodID method=GET){
            Log.log(Info,"ExtensionStreamShared");
            this->url = url;
            this->method = method;
            this->out = &out;
        }

        virtual void open(HttpServer *server){
            Log.log(Info,"ExtensionStreamShared","open");

            auto lambda = [](HttpServer *server_ptr, const char*requestPath, HttpRequestHandlerLine *hl){ 
                HttpReplyHeader reply_header;
                ExtensionStreamShared *ext = static_cast<ExtensionStreamShared*>(hl->context[0]);
                HttpStreamedOutput *out = ext->getOutput();
                if (out==nullptr){
                    Log.log(Error,"ExtensionStreamShared","out must not be null");
                    return;
                }
                Log.log(Error,"mime",out->mime());

                reply_header.setValues(200, "OK");
                reply_header.put(TRANSFER_ENCODING,CHUNKED);
                reply_header.put(CONTENT_TYPE,out->mime());
                reply_header.put(CONNECTION,CON_KEEP_ALIVE);
                reply_header.write(server_ptr->client());
                out->open(server_ptr->client());
            };
     
            HttpRequestHandlerLine *hl = new HttpRequestHandlerLine(1);
            hl->context[0] = this;

            // register new handler
            hl->path = url;
            hl->fn = lambda;
            hl->method = method;
            server->addHandler(hl);
        }

        void doLoop(){
            // get the actual client_ptr
            if (out->isOpen()){
                out->doLoop();
            } 
        }   

        // checks if the output is currently open in order to determie if we need to wwrite any data
        HttpStreamedOutput *getOutput(){
             return out;
        }

        // checks if we have any active clients for the id
        bool isOpen(){
            return out!=nullptr && out->isOpen();
        }

        // closes the ouptut
        void close(){
            if (out!=nullptr)
                out->close();
        }

        // write the content to the ouptut
        int write(uint8_t *content, int len){
            if (out!=nullptr)
                out->write(content, len);
        }

        // writes a line 
        int print(const char* str){
            if (out!=nullptr){
                int len = strlen(str);
                out->write((uint8_t*)str, len);
            }
        }

        // writes a line which terminates with a html line break
        int println(const char* str){
            if (out!=nullptr)
                out->println(str);
        }

    protected:
        MethodID method;
        HttpStreamedOutput *out = nullptr;
        const char* url = nullptr;
};

}

