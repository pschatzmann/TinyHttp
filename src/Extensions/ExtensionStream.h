#pragma once

#include "Stream.h"
#include "Extensions/Extension.h"
#include "Server/HttpStreamedMultiOutput.h"
#include "Extensions/ExtensionStreamBasic.h"
#include "Basic/RingBuffer.h"

namespace tinyhttp {

/**
 * @brief Extension which implements the Arduino Stream functionality. Instead of writing to Serial you can
 * write to a ExtensionStream object and look at the streamed output in your web browser. 
 * The data is managed with the help of a simple ring buffer.
 */

class ExtensionStream : public Stream, public Extension  {
    public:
        /// Default Constructor
        ExtensionStream(const char* url, MethodID action,  const char* mime, const char* startHtml=nullptr, const char* endHtml=nullptr, int bufferSize=256, int historySize=1024){
            HttpLogger.log(Info,"ExtensionStream");
            out = new HttpStreamedMultiOutput(mime, startHtml, endHtml, historySize);
            ext = new ExtensionStreamBasic(url, *out, action);
            this->bufferSize = bufferSize;
            this->ringBuffer = new RingBuffer(bufferSize);
        }

        /// Alternative way to provide data by reading from another stream
        ExtensionStream(const char* url, const char* mime, Stream &source){
            out = new HttpStreamedMultiOutput(mime);
            ext = new ExtensionStreamBasic(url, *out, GET);
            pAltSource = &source;
        }

        ~ExtensionStream(){
            HttpLogger.log(Info,"~ExtensionStream");
            delete out;
            delete ringBuffer;
        }

        /// Defines a standard reply header - which can be binary data
        void setReplyHeader(Str &header){
            ext->setReplyHeader(header);
        }

        // delegate processing to ExtensionStreamBasic
        virtual void open(HttpServer *server){     
            HttpLogger.log(Info,"ExtensionStream %s","open");
            ext->open(server);
            is_open = true;
        }

        int available() {
            return 0;
        }
        int read() {
            return 0;
        }
        int read(char* str, int len){
            return 0;
        }

        int peek() {
            return -1;
        };

        void flush() {
            if (out!=nullptr && out->isOpen()) {
                int readLen = ringBuffer->available();
                if (readLen>0){
                    HttpLogger.log(Info,"ExtensionStream %s","flush");
                    uint8_t buffer[readLen];
                    readLen = ringBuffer->read(buffer, readLen);
                    out->write(buffer, readLen);
                }
            }
        };

        size_t write(uint8_t chr) {
            if (ringBuffer!=nullptr && ringBuffer->availableToWrite()==0){
                flush();
            }
            return ringBuffer->write(chr);            
        }

        size_t write(uint8_t *str, int len) {
            if (out!=nullptr && out->isOpen()) {
                HttpLogger.log(Info,"ExtensionStream %s","write");
                flush();
                return out->write(str, len);
            } 
            return 0;
        }

        size_t print(const char str[]){
            if (out!=nullptr && out->isOpen()) {
                HttpLogger.log(Info,"ExtensionStream %s","print");
                flush();
                return out->print(str);
            }
            return 0;
        }

        size_t println(const char str[]){
            if (out!=nullptr && out->isOpen()) {
                HttpLogger.log(Info,"ExtensionStream %s","println");
                flush();
                return out->println(str);
            }
            return 0;
        }

        int availableForWrite() {
            return out->availableForWrite();
        }

        /// Checks if we have any open clients
        virtual bool isOpen(){
            return is_open && out==nullptr ? false : out->isOpen();
        }

    protected:
        ExtensionStreamBasic *ext=nullptr;
        RingBuffer *ringBuffer=nullptr; 
        HttpStreamedMultiOutput *out=nullptr;
        Stream *pAltSource=nullptr;
        int bufferSize;
        bool is_open=false;

        virtual void doLoop() override {
            // only do something after open
            if (isOpen()){
                copyFromAltSource(); // optinally
                ext->doLoop();
            }
        }

        void copyFromAltSource() {
            if (pAltSource!=nullptr){
                uint8_t buffer[1024];
                size_t bytesRead = pAltSource->readBytes(buffer, 1024);
                out->write(buffer, bytesRead);
            }
        }

};

}

