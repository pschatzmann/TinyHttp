#ifndef __WEBDUMP_H__
#define __WEBDUMP_H__

#include <Arduino.h>
#include <SPI.h>
#include <SD.h>
#include "HttpRequest.h"
#include "MimeResolver.h"
#include "HttpLogger.h"
#include "SDFileNameMgr.h"
#include "Url.h"
#include "UrlExtractor.h"
#include "SDStack.h"

namespace tinyhttp {

/**
 * @brief Functionality which dumps a website to a SD drive. When the simulate flag is set we do the regular
 * processing but we do not write and information to the SD drive.
 */

class WebCopy {
  public:
    /// default constructor
    WebCopy(Client &client, bool inDoLoop=true, int cpin=-1, int bufferSize=512) {
      Log.log(Info,"WebCopy");
      this->processing_in_do_loop = inDoLoop;
      this->http.setClient(client);
      this->buffer_size = bufferSize;

      http.reply().put("Content-Type","");
      http.reply().setAutoCreateLines(false);

      // setup SD
      if (cpin==-1){
          SD.begin();
      } else {
          SD.begin(cpin);
      }
    }

    /// Destructor - we release the used memory
    ~WebCopy() {
      clear();
    }

    // defines an alternative file name manager. 
    virtual void setFileNameMgr(SDFileNameMgr &fileNameMgr) {
        this->file_name_mgr = fileNameMgr;
    }

    // starts the extraction for the indicated url
    virtual void start(const char* startUrlChar){
      char msg[100];
      sprintf(msg, "start %s",startUrlChar);
      Log.log(Info,"WebCopy", msg);
      this->start_url.setUrl(startUrlChar);
      const char* root = start_url.urlRoot();
      Log.log(Info,"WebCopy->root", root);


      this->file_name_mgr.setRootUrl(root);
      this->url_extractor.setRootUrl(root);
      // star the dump
      stack.push(start_url.url());
      active = true;
      if (!processing_in_do_loop){
        startDump();
      }
    }

    virtual void stop() {
      Log.log(Info,"WebCopy", "stop");
      active = false;
    }

    bool isActive() {
      return active;
    }

    // incremental processing in Loop
    void doLoop(){
      if (processing_in_do_loop){
        Log.log(Info, "startDump");
        Str url = stack.popStr();
        if(active && !url.isEmpty()){
          reportHeap();
          // extract the content
          processContent(url);
        } else {
          active = false;
          clear();
        }
      }
    }

  protected:
    Url start_url;
    Url url;
    HttpRequest http;
    SDFileNameMgr file_name_mgr;
    UrlExtractor url_extractor;
    SDStack stack = SDStack("/stack.txt");
    int max_level;
    int buffer_size;
    bool active;
    bool processing_in_do_loop;


    virtual void clear() {
      stack.reset();
      reportHeap();
    }

    /// dumps the url to a file while for all stack
    virtual void startDump(){
      Log.log(Info, "startDump");
      Str url = stack.popStr();
      while(active && !url.isEmpty()){
        reportHeap();
        // extract the content
        processContent(url);
      }
      clear();
    }

    void reportHeap() {
  #ifdef ESP32
      int freeHeap = ESP.getFreeHeap();
      char msg[80];
      sprintf(msg, "free memory: %d",  freeHeap);
      Log.log(Info, msg);
  #endif
    }

    // copies the content of the url to a file and collects the contained urls
    void processContent(Str urlStr) {
        Log.log(Info, "processContent", urlStr.c_str());
        if (!urlStr.isEmpty()){
          // Determine Mime
          url.setUrl(urlStr.c_str());
          Str mimeStr = getMime(url);
          // optional filter by requested mime type
          // create file 
          bool exists = false;
          File file = createFile(urlStr, mimeStr);
          if (file.size()==0){
              // get the data
              reportHeap();
              Log.log(Info, "processContent", url.url());
              http.get(url);
              processFile(file, mimeStr);
          }
          file.close();
        }
    }

    // read from URL to File
    void processFile(File &file, Str &mimeStr){
        Log.log(Info, "processFile", file.name());
        if (mimeStr.contains("htm")){
          processHtml(file);
        } else {
          processOthers(file);
        }
    }

    // determines the mime type
    Str getMime(Url &url) {
        Log.log(Info, "getMime", url.url());
        http.head(url);
        const char* mime = http.reply().get(CONTENT_TYPE);
        // text/html; charset=UTF-8 -> html
        Str mimeStr(mime);
        Log.log(Info, "getMime->", mimeStr.c_str());
        return mimeStr;
    }

    // creates an empty file for the url - returns true if an empty file was created
    bool createEmptyFile(Str urlStr){
        Url url(urlStr.c_str());
        Str mime = getMime(url);
        Str fileName = this->file_name_mgr.getName(urlStr.c_str(), mime.c_str());
        bool exists = SD.exists(fileName.c_str());
        if (!exists){
          File file = SD.open(fileName.c_str(), FILE_WRITE);
          file.close();
        }
        return !exists;
    }

    // creates the directoy and the file on the SD drive
    File createFile(Str &urlStr, Str &mime){
        Log.log(Info, "createFile", urlStr.c_str());
        // determine the file name which is valid for the SD card
        Str file_name = file_name_mgr.getName(urlStr.c_str(), mime.c_str());
        Log.log(Info, "createFile", file_name.c_str());
        // create directory - limit name to show only the path
        int pos = file_name.lastIndexOf("/");
        if (pos>1){
          file_name.setLength(pos);
          SD.mkdir(file_name.c_str());
          // remove limit to get the full name back
          file_name.setLengthUndo();
        }
        // return an real file only if it does 
        File file = SD.open(file_name.c_str());
        Log.log(Info, "createFile", file.name());

        return file;
    }

    // process a html file by saving the content to the file and extracting the 
    // contained urls
    void processHtml(File &file) {
        Log.log(Info, "processHtml", file.name());
        // process all lines
        uint8_t buffer[buffer_size];
        while(http.available()){
            int len = http.readln(buffer, buffer_size);
            extractReferences(buffer, len);
            // write line to file
            file.write((const uint8_t*)buffer, len);
        }
        file.close();
    }

    // just save the content to a file
    void processOthers(File &file) {
        Log.log(Info, "processOthers");
        uint8_t buffer[buffer_size];
        while(http.available()){
            // read a single line
            int len = http.readln(buffer, buffer_size);
            file.write((const uint8_t*)buffer, len);
        }
        file.close();
    }

    // extracts the stack and puts them on the stack Vector
    void extractReferences(uint8_t* buffer, int len) {
        Log.log(Info, "extractReferences");
        char url_buffer[200];
        Str url(url_buffer,200);
        // read a single line
        url_extractor.setString((const char*)buffer);
        
        // put all urls on stack vector
        bool found = url_extractor.nextUrl(url);
        while(found){
          if (!url.isEmpty()){
            if (createEmptyFile(url)) {
              Log.log(Info, "extractReferences",url.c_str());
              stack.push(url.c_str());
            }
          }
          found = url_extractor.nextUrl(url);
        }
    }

};

} // namespace

#endif // __WEBDUMP_H__