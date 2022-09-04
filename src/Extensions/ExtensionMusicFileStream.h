#pragma once

#include <SPI.h>
#include <SD.h>
#include "Extensions/Extension.h"
#include "Server/HttpServer.h"
#include "Extensions/ExtensionStreamShared.h"
#include "Server/HttpStreamedMultiOutput.h"

namespace tinyhttp {

/**
 * @brief Extension which supports the Streaming of music files from a SD drive. 
 * 
 */
class ExtensionMusicFileStream : public Extension {
    public:
        ExtensionMusicFileStream(const char*url="/music", const char* startDir="/", const char* mime="audio/mpeg", const char* extension="mp3", int bufferSize=512, int cspin=-1){
            Log.log(Info,"ExtensionMusicFileStream", url);
            this->url = url;
            this->file_extension = extension;
            this->start_dir = startDir;
            this->buffer_size = bufferSize;
            this->buffer = new uint8_t[bufferSize];
            HttpStreamedMultiOutput *out = new HttpStreamedMultiOutput(mime, nullptr, nullptr, 0);
            this->streaming = new ExtensionStreamShared(url,  *out, GET);
            this->sd_cs = cspin;

        }

        ~ExtensionMusicFileStream(){
            Log.log(Info,"~ExtensionMusicFileStream");
            delete[] buffer;
            delete this->streaming;
        }

        virtual void open(HttpServer *server) {
            Log.log(Info,"ExtensionMusicFileStream", "open");
            setupSD();
            // setup handler
            streaming->open(server);
            // setup first music file
            current_file = SD.open(start_dir);
            // find first music file - so that we are already ready to stream when a request arives
            getNextMusicFile();
        }

        // incremental pushing of the next buffer size to the open clients using chunked HttpStreamedMultiOutput
        virtual void doLoop(){
            // we actually just need to do something if we have open clients
            if (streaming->isOpen()){
                File file = getNextMusicFile();
                if (file) {
                    // we just write the current data from the file to all open streams            
                    int len = file.read(buffer,buffer_size);
                    streaming->write(buffer, len);
                }
            }
        }

    protected:
        ExtensionStreamShared *streaming;
        const char *file_extension;
        const char *start_dir;
        const char *url;
        File current_file;
        File empty;
        uint8_t* buffer;
        int buffer_size;
        int loop_limit = 2;
        int loop_count;
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

        // provides the current file if it is not finished yet otherwise we move to the 
        // next music file or restart at the start directory when we reach the end
        File &getNextMusicFile() {
            Log.log(Info,"ExtensionMusicFileStream::getNextMusicFile",file_extension);
            if ((current_file).available()>0){
                Log.log(Info,"ExtensionMusicFileStream::getNextMusicFile", current_file.name());
                return current_file;
            }
            current_file.close();
            while(true){
                // prevent an endless loop
                if (loop_count > loop_limit){
                    return empty;
                }

                // check file extension
                current_file = current_file.openNextFile();
                if (current_file){
                    Str name_str = Str(current_file.name());
                    if (name_str.endsWith(file_extension) && !name_str.contains("/.")){
                        break;
                    }
                    Log.log(Warning,"ExtensionMusicFileStream::getNextMusicFile - not relevant", current_file.name());
                } else {
                    Log.log(Warning,"ExtensionMusicFileStream::getNextMusicFile", "restart");
                    // no file -> restart from the beginning
                    current_file = SD.open(start_dir);
                    loop_count++;
                }
            }    

            Log.log(Info,"ExtensionMusicFileStream::getNextMusicFile", current_file.name());
            return current_file;
        }

};

}

