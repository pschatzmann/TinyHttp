#pragma once

#include "Server/HttpHeader.h" 
#include "Platform/AltClient.h"
#include "Server/HttpChunkReader.h"
#include <WiFiClientSecure.h>

namespace tinyhttp {


/**
 * @brief Simple API to process get, put, post, del http requests
 * I tried to use Arduino HttpClient, but I  did not manage to extract the mime
 * type from streaming get requests.
 * 
 * The functionality is based on the Arduino Client class.
 * 
 */

class HttpRequest{
    public:
        HttpRequest() {
            HttpLogger.log(Info,"HttpRequest");
            default_client.setInsecure();
            setClient(default_client);
        }

        HttpRequest(Client &client){
            HttpLogger.log(Info,"HttpRequest");
            setClient(client);
        }

        void setClient(Client &client){
            this->client_ptr = &client;
        }

        // the requests usually need a host. This needs to be set if we did not provide a URL
        void setHost(const char* host){
            HttpLogger.log(Info,"setHost", host);
            this->host_name = host;
        }

        operator bool() {
            return client_ptr!=nullptr && (bool)*client_ptr;
        }

        virtual bool connected(){
            return client_ptr->connected();
        } 

        virtual int available() {
            if (reply_header.isChunked()){
                return chunk_reader.available();
            }
            return client_ptr->available();
        }

        virtual void stop(){
            HttpLogger.log(Info,"stop");
            client_ptr->stop();
        }

        virtual int post(Url &url, const char* mime, const char *data, int len=-1){
            HttpLogger.log(Info,"post %s", url.url());
            return process(T_POST, url, mime, data, len);
        }

        virtual int put(Url &url, const char* mime, const char *data, int len=-1){
            HttpLogger.log(Info,"put %s", url.url());
            return process(T_PUT, url, mime, data, len);
        }

        virtual int del(Url &url,const char* mime=nullptr, const char *data=nullptr, int len=-1) {
            HttpLogger.log(Info,"del %s", url.url());
            return process(T_DELETE, url, mime, data, len);
        }

        virtual int get(Url &url,const char* acceptMime=nullptr, const char *data=nullptr, int len=-1) {
            HttpLogger.log(Info,"get %s", str(url.url()));
            this->accept = acceptMime;
            return process(T_GET, url, nullptr, data, len);
        }

        virtual int head(Url &url,const char* acceptMime=nullptr, const char *data=nullptr, int len=-1) {
            HttpLogger.log(Info,"head %s", url.url());
            this->accept = acceptMime;
            return process(T_HEAD, url, nullptr, data, len);
        }

        // reads the reply data
        virtual int read(uint8_t* str, int len){            
            if (reply_header.isChunked()){
                return chunk_reader.read(*client_ptr, str, len);
            } else {
                return client_ptr->read(str, len);
            }
        }

        // read the reply data up to the next new line. For Chunked data we provide 
        // the full chunk!
        virtual int readln(uint8_t* str, int len, bool incl_nl=true){
            if (reply_header.isChunked()){
                return chunk_reader.readln(*client_ptr, str, len);
            } else {
                return chunk_reader.readlnInternal(*client_ptr, str, len, incl_nl);
            }
        }

        // provides the head information of the reply
        virtual HttpReplyHeader &reply(){
            return reply_header;
        }

        virtual void setAgent(const char* agent){
            this->agent = agent;
        }

        virtual void setConnection(const char* connection){
            this->connection = connection;
        }

        virtual void setAcceptsEncoding(const char* enc){
            this->accept_encoding = enc;
        }

        Client *client() {
            return client_ptr;
        }
   
    protected:
        WiFiClientSecure default_client;
        Client *client_ptr;
        Url url;
        HttpRequestHeader request_header;
        HttpReplyHeader reply_header;
        HttpChunkReader chunk_reader{reply_header};
        const char *agent = nullptr;
        const char *host_name=nullptr;
        const char *connection = CON_CLOSE;
        const char *accept = ACCEPT_ALL;
        const char *accept_encoding = nullptr;

        const char* str(const char* in){
            return in==nullptr ? "" : in;
        }

        // opens a connection to the indicated host
        virtual int connect(const char *ip, uint16_t port) {
            HttpLogger.log(Info,"connect %s", ip);
            return this->client_ptr->connect(ip, port);
        }

        // sends request and reads the reply_header from the server
        virtual int process(TinyMethodID action, Url &url, const char* mime, const char *data, int len=-1){
            if (!connected()){
                char msg[1024];
                sprintf(msg, "connecting to host %s port %d", url.host(), url.port());
                HttpLogger.log(Info,"process %s", msg);

                connect(url.host(), url.port());
                if (host_name==nullptr){
                    host_name = url.host();
                }
            }
            request_header.setValues(action, url.path());
            if (len==-1 && data!=nullptr){
                len = strlen(data);
                request_header.put(CONTENT_LENGTH, len);
            }
            if (host_name!=nullptr){
                request_header.put(HOST_C, host_name);    
            }            
            if (agent!=nullptr) {
                request_header.put(USER_AGENT, agent);
            }
            if(accept_encoding!=nullptr){
                request_header.put(ACCEPT_ENCODING, accept_encoding);
            }
            if (mime!=nullptr){
                request_header.put(CONTENT_TYPE, mime);
            }

            request_header.put(CONNECTION, connection);
            request_header.put(ACCEPT, accept);
            
            request_header.write(*client_ptr);

            if (len>0){
                HttpLogger.log(Info,"process - writing data");
                client_ptr->write((const uint8_t*)data,len);
            }

            reply_header.read(*client_ptr);

            // if we use chunked tranfer we need to read the first chunked length
            if (reply_header.isChunked()){
                chunk_reader.open(*client_ptr);
            };

            return reply_header.statusCode();
        }

};

}
