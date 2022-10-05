#pragma once

#include "Platform/AltStream.h"

namespace tinyhttp {

/**
 * @brief Supported log levels
 * 
 */
enum LogLevel { 
    Debug,
    Info, 
    Warning, 
    Error
};

/**
 * @brief Logger that writes messages dependent on the log level
 * 
 */

class HttpLoggerClass {
    public:
        HttpLoggerClass()= default;
        
        // activate the logging
        virtual void begin(Stream& out, LogLevel level=Error){
            this->log_stream_ptr = &out;
            this->log_level = level;
        }

        void setLevel(LogLevel l){
            log_level = l;
        }

        // checks if the logging is active
        virtual bool isLogging(){
            return log_stream_ptr!=nullptr;
        }

        // write an message to the log
        virtual void log(LogLevel current_level, const char *str, const char* str1=nullptr, const char* str2=nullptr){
            if (log_stream_ptr!=nullptr){
                if (current_level >= log_level){
                    log_stream_ptr->print((char*)str);
                    if (str1!=nullptr){
                        log_stream_ptr->print(" ");
                        log_stream_ptr->print((char*)str1);
                    }
                    if (str2!=nullptr){
                        log_stream_ptr->print(" ");
                        log_stream_ptr->print((char*)str2);
                    }
                    log_stream_ptr->println();
                    log_stream_ptr->flush();
                }
            }
        }

    protected:
        Stream *log_stream_ptr=&Serial;
        LogLevel log_level = Warning;  

};

HttpLoggerClass HttpLogger;

}

