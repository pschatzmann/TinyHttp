#pragma once

#include "Server/HttpStreamedOutput.h"
#include "Server/HttpServer.h"
#include "Client.h"

namespace tinyhttp {


/**
 * @brief Extension which supports streaming to multiple clients. It makes sure that same content is streamed 
 * to multiple clients when you call the writes or printlns methods in your loop();
 * Only a subset of the stream output functions is avaiblable!
 */

class ExtensionStreamBasic : public Extension  {
    friend class ExtensionStream;
    public:
        /// Default Constructor
        ExtensionStreamBasic( const char* url,  HttpStreamedOutput &out, MethodID method=GET){
            HttpLogger.log(Info,"ExtensionStreamBasic");
            this->url = url;
            this->method = method;
            this->out = &out;
        }

        /// Defines a standard reply header
        void setReplyHeader(Str &header){
            hl.header = &header;
        }

        virtual void open(HttpServer *server){
            HttpLogger.log(Info,"ExtensionStreamBasic %s","open");

            auto lambda = [](HttpServer *server_ptr, const char*requestPath, HttpRequestHandlerLine *hl){ 
                HttpReplyHeader reply_header;
                ExtensionStreamBasic *ext = static_cast<ExtensionStreamBasic*>(hl->context[0]);
                HttpStreamedOutput *out = ext->getOutput();
                if (out==nullptr){
                    HttpLogger.log(Error,"ExtensionStreamBasic %s","out must not be null");
                    return;
                }
                HttpLogger.log(Error,"mime",out->mime());

                reply_header.setValues(200, "OK");
                reply_header.put(TRANSFER_ENCODING,CHUNKED);
                reply_header.put(CONTENT_TYPE,out->mime());
                reply_header.put(CONNECTION,CON_KEEP_ALIVE);
                reply_header.write(server_ptr->client());

                out->open(server_ptr->client());

                // if a replay header is defined we write it out
                if (hl->header!=nullptr){
                    HttpLogger.log(Info,"ExtensionStreamBasic %s","writing content header");
                    out->write((uint8_t*)hl->header->c_str(), hl->header->length());
                }
                
            };
     
            // register new handler
            hl.context[0] = this;
            hl.path = url;
            hl.fn = lambda;
            hl.method = method;
            server->addHandler(&hl);
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
            int result = 0;
            if (out!=nullptr)
                result = out->write(content, len);
            return result;
        }

        // writes a line 
        int print(const char* str){
            int result = 0;
            if (out!=nullptr){
                int len = strlen(str);
                result = out->write((uint8_t*)str, len);
            }
            return result;
        }

        // writes a line which terminates with a html line break
        int println(const char* str){
            int result = 0;
            if (out!=nullptr)
                result = out->println(str);
            return result;
        }

    protected:
        MethodID method;
        HttpStreamedOutput *out = nullptr;
        HttpRequestHandlerLine hl{1};
        const char* url = nullptr;

        void doLoop() override {
            // get the actual client_ptr
            if (out->isOpen()){
                out->doLoop();
            } 
        }   

};

}

