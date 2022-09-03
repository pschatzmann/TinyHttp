#pragma once

#include "ExtensionStream.h"

#define HTML_HEADER "<html><body style='background-color:black; color:white'><h1>Web Logger</h1>"
#define HTML_END "</body></html>"

namespace tinyhttp {

/**
 * @brief A Simple Logger
 * 
 */
class ExtensionLoggingStream : public ExtensionStream {

  public:
    ExtensionLoggingStream(const char* url, MethodID action=GET,  const char* mime="text/html", const char* startHtml=HTML_HEADER, const char* endHtml=HTML_END, int bufferSize=256, int historySize=1024)
    : ExtensionStream (url, action, mime, startHtml, endHtml, bufferSize, historySize){
    }
};

}