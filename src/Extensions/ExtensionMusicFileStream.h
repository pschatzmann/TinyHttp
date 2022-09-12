#pragma once

#include <SPI.h>
#include <SD.h>
#include "Extensions/Extension.h"
#include "Server/HttpServer.h"
#include "Extensions/ExtensionStreamBasic.h"
#include "Server/HttpStreamedMultiOutput.h"

namespace tinyhttp {

/**
 * @brief Extension which supports the Streaming of music files from a SD drive. 
 * 
 */
class ExtensionMusicFileStream : public Extension {
    public:
        ExtensionMusicFileStream(const char*url="/music", const char* startDir="/", const char* mime="audio/mpeg", const char* extension="mp3", int bufferSize=512, int cspin=-1, int delay=10){
            HttpLogger.log(Info,"ExtensionMusicFileStream", url);
            this->url = url;
            this->file_extension = extension;
            this->start_dir = startDir;
            this->buffer_size = bufferSize;
            this->buffer = new uint8_t[bufferSize];
            HttpStreamedMultiOutput *out = new HttpStreamedMultiOutput(mime, nullptr, nullptr, 0);
            this->streaming = new ExtensionStreamBasic(url,  *out, GET);
            this->sd_cs = cspin;
            this->delay_ms = delay;

        }

        ~ExtensionMusicFileStream(){
            HttpLogger.log(Info,"~ExtensionMusicFileStream");
            delete[] buffer;
            delete this->streaming;
        }

        virtual void open(HttpServer *server) {
            HttpLogger.log(Info,"ExtensionMusicFileStream", "open");
            setupSD();
            // setup handler
            streaming->open(server);
            // setup first music file
            directory = SD.open(start_dir);
            // find first music file - so that we are already ready to stream when a request arives
            getMusicFile();
        }

    protected:
        ExtensionStreamBasic *streaming;
        const char *file_extension;
        const char *start_dir;
        const char *url;
        File directory;
        File current_file;
        File empty;
        uint8_t* buffer;
        int buffer_size;
        int loop_limit = 10;
        int loop_count;
        int sd_cs;
        int delay_ms;
        bool is_open = false;

        // incremental pushing of the next buffer size to the open clients using chunked HttpStreamedMultiOutput
        virtual void doLoop()override{
            // we actually just need to do something if we have open clients
            if (streaming->isOpen()){
                File file = getMusicFile();
                if (file) {
                    // we just write the current data from the file to all open streams            
                    int len = file.read(buffer,buffer_size);
                    streaming->write(buffer, len);
                    delay(delay_ms);
                }
            }
        }

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

        // provides the current file if it is not finished yet otherwise we move to the 
        // next music file or restart at the start directory when we reach the end
        File &getMusicFile() {
            HttpLogger.log(Debug,"ExtensionMusicFileStream::getMusicFile",file_extension);
            if (current_file.available()>0){
                loop_count = 0;
                //HttpLogger.log(Debug,"ExtensionMusicFileStream::getMusicFile", current_file.name());
                return current_file;
            }
            current_file.close();
            int nextFileCount = 0;
            while(true){
                // prevent an endless loop
                if (loop_count > loop_limit){
                    return empty;
                }

                // check file extension
                current_file = directory.openNextFile();
                HttpLogger.log(Info,"processing", current_file.name());
                if (current_file){
                    Str name_str = Str(current_file.name());
                    if (name_str.endsWith(file_extension) && !name_str.contains("/.")){
                        HttpLogger.log(Info,"ExtensionMusicFileStream::getMusicFile", current_file.name());
                        loop_count = 0;
                        nextFileCount = 0;
                        break;
                    } 
                    HttpLogger.log(Warning,"ExtensionMusicFileStream::getMusicFile - not relevant", current_file.name());
                } else {
                    nextFileCount++;
                }

                // we restart when we did not find any vaild file in the last 20 entrie
                if (nextFileCount>20){
                    HttpLogger.log(Warning,"ExtensionMusicFileStream::getMusicFile", "restart");
                    // no file -> restart from the beginning
                    directory.rewindDirectory();
                    loop_count++;
                }
            }    

            return current_file;
        }

};

}

