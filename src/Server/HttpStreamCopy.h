#ifndef __HttpStreamCopy_H__
#define __HttpStreamCopy_H__


/**
 * @brief Processing of a single stream to a single client. 
 * In the loop we can simply provide individual small chunks
 * until we are done.
 * 
 * We work on a copy of the WiFiClient
 */
#include "Server/HttpChunkWriter.h"

class HttpStreamCopy {
    public:
        HttpStreamCopy(Stream &input, WiFiClient &client, int outputSize=215){
            this->is_open=true;
            this->input_ptr = &input;
            this->client = client;
            this->output_size = outputSize;
        }

        ~HttpStreamCopy(){
            close();
        }

        bool isOpen(){
            is_open;
        }

        void doLoop(){
            if (is_open) {
                char buffer[outputSize];
                if(input.available()>0 && client.connected()){
                    int len = input.read(buffer, outputSize);
                    writer.writeChunk(client, buffer, len);
                } else {
                    close();
                }
            }
        }

    protected:
        HttpChunkWriter writer;
        Stream *input_ptr;
        Client client;
        int output_size;
        bool is_open;


        void close() {
            if (is_open){
                is_open = false;
                input.close();
                chunk_writer.writeEnd(client_ptr);
                client_ptr = nullptr;    
            }            
        }

}
#endif // __HttpStreamCopy_H__