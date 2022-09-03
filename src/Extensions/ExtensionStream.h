#ifndef __EXTENSIONSTREAM_H__
#define __EXTENSIONSTREAM_H__

#include "Stream.h"
#include "Extensions/Extension.h"
#include "Server/HttpStreamedMultiOutput.h"
#include "Extensions/ExtensionStreamLean.h"
#include "Basic/RingBuffer.h"

namespace tinyhttp {

/**
 * @brief Extension which implements the Arduino Stream functionality. Instead of writing to Serial you can
 * write to a ExtensionStream object and look at the streamed output in your web browser. 
 * The data is managed with the help of a simple ring buffer.
 */

class ExtensionStream : public Stream, public Extension  {
    public:
        ExtensionStream(const char* url, MethodID action,  const char* mime, const char* startHtml=nullptr, const char* endHtml=nullptr, int bufferSize=256, int historySize=1024){
            Log.log(Info,"ExtensionStream");
            out = new HttpStreamedMultiOutput(mime, startHtml, endHtml, historySize);
            ext = new ExtensionStreamLean(url, *out, action);
            this->bufferSize = bufferSize;
            this->ringBuffer = new RingBuffer(bufferSize);
        }

        ~ExtensionStream(){
            Log.log(Info,"~ExtensionStream");
            delete out;
            delete ringBuffer;
        }

        // delegate processing to ExtensionStreamLean
        virtual void open(HttpServer *server){     
            Log.log(Info,"ExtensionStream","open");
            ext->open(server);
            is_open = true;
        }

        virtual void doLoop() {
            // only do something after open
            if (is_open){
                ext->doLoop();
            }
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
            int readLen = ringBuffer->available();
            if (readLen>0){
                Log.log(Info,"ExtensionStream","flush");
                uint8_t buffer[readLen];
                readLen = ringBuffer->read(buffer, readLen);
                out->write(buffer, readLen);
            }
        };

        size_t write(uint8_t chr) {
            if (ringBuffer->availableToWrite()==0){
                flush();
            }
            return ringBuffer->write(chr);            
        }

        size_t write(uint8_t *str, int len) {
            if (out->isOpen()) {
                Log.log(Info,"ExtensionStream","write");
                flush();
                return out->write(str, len);
            } 
            return 0;
        }

        size_t print(const char str[]){
            if (out->isOpen()) {
                Log.log(Info,"ExtensionStream","print");
                flush();
                return out->print(str);
            }
            return 0;
        }

        size_t println(const char str[]){
            if (out->isOpen()) {
                Log.log(Info,"ExtensionStream","println");
                flush();
                return out->println(str);
            }
            return 0;
        }

    protected:
        ExtensionStreamLean *ext;
        RingBuffer *ringBuffer; 
        HttpStreamedMultiOutput *out;
        int bufferSize;
        bool is_open=false;

};

}

#endif // __EXTENSIONSTREAM_H__