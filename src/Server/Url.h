#pragma once

#include "Basic/Str.h"
#include "Server/HttpLogger.h"

namespace tinyhttp {

/**
 * @brief URL parser which breaks a full url string up into its individual parts
 * 
 * http://pschatzmann.ch:80/path1/path2
 * -> protocol: http
 * -> host: pschatzmann.ch
 * -> port: 80
 * -> url: http://pschatzmann.ch:80/path1/path2
 * -> root: http://pschatzmann.ch:80
 */
class Url {
    public:
        // empty url
        Url() {
            HttpLogger.log(Info,"Url");
        }
        
        ~Url() {
            HttpLogger.log(Info,"~Url");
            pathStr.clear();
            hostStr.clear();
            protocolStr.clear();
            urlRootStr.clear();
            urlStr.clear();
        }

        // setup url with string
        Url(const char *url){
            HttpLogger.log(Info,"Url %s",url);
            setUrl(url);
        }

        // copy constructor
        Url(Url &url){
            HttpLogger.log(Info,"Url %s",url.url());
            setUrl(url.url());
        }

        const char* url() {return urlStr.c_str();}
        const char* path() { return pathStr.c_str(); }
        const char* host() { return hostStr.c_str();}
        const char* protocol() {return protocolStr.c_str();}
        const char* urlRoot() {return urlRootStr.c_str();} // prefix w/o path -> https://host:port
        int port() {return portInt;}

        void setUrl(const char* url){
            HttpLogger.log(Info,"setUrl %s",url);
            this->urlStr = url;
            parse();
        }

    protected:
        Str pathStr = Str(40);
        Str hostStr = Str(20);
        Str protocolStr = Str(6);
        Str urlRootStr = Str(40);
        Str urlStr = Str(40);
        int portInt;

        void parse() {
            HttpLogger.log(Info,"Url::parse");
            
            int protocolEnd = urlStr.indexOf("://");
            if (protocolEnd==-1){
                return;
            }
            protocolStr.substring(urlStr, 0,protocolEnd);
            int portStart = urlStr.indexOf(":",protocolEnd+3);            
            int hostEnd = portStart!=-1 ? portStart : urlStr.indexOf("/",protocolEnd+4);
            // we have not path -> so then host end is the end of string
            if (hostEnd==-1){
                hostEnd = urlStr.length();                
            }
            hostStr.substring(urlStr, protocolEnd+3,hostEnd);
            if (portStart>0){
                portInt = atoi(urlStr.c_str()+portStart+1);
            } else {
                if (protocolStr.startsWith("https"))
                    portInt = 443;
                else if (protocolStr.startsWith("http"))
                    portInt = 80;
                else if (protocolStr.startsWith("ftp"))
                    portInt = 21;
                else 
                    portInt = -1; // undefined
            }
            int pathStart = urlStr.indexOf("/",protocolEnd+4);
            if (pathStart==0){
                // we have no path
                pathStr = "/";
                urlRootStr = urlStr;
            } else {
                pathStr.substring(urlStr, pathStart, urlStr.length());
                pathStr.trim();
                urlRootStr.substring(urlStr, 0, pathStart);
            }
            HttpLogger.log(Info,"url-> %s",url());
            HttpLogger.log(Info,"path-> %s",path());
           
        }

};

}

