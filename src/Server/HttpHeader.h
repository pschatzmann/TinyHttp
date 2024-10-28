#pragma once

#include "Platform/AltClient.h"
#include "Basic/Str.h"
#include "Basic/List.h"
#include "Server/Url.h"
#include "Server/HttpLogger.h" 
#include "Server/HttpLineReader.h" 

namespace tinyhttp {

// Class Configuration
const int MaxHeaderLineLength = 200;

// Define relevant header content
const char* CONTENT_TYPE = "Content-Type";
const char* CONTENT_LENGTH = "Content-Length";
const char* CONNECTION = "Connection";
const char* CON_CLOSE = "close";
const char* CON_KEEP_ALIVE = "keep-alive";
const char* TRANSFER_ENCODING = "Transfer-Encoding";
const char* CHUNKED = "chunked";
const char* ACCEPT = "Accept";
const char* ACCEPT_ALL = "*/*";
const char* SUCCESS = "Success";
const char* USER_AGENT = "User-Agent";
const char* DEFAULT_AGENT = "Mozilla/5.0 (compatible; Googlebot/2.1; +http://www.google.com/bot.html)";
const char* HOST_C = "Host";
const char* ACCEPT_ENCODING = "Accept-Encoding";
const char* IDENTITY = "identity";
const char* LOCATION = "Location";


// Http methods
enum TinyMethodID {T_UNDEFINED, T_GET, T_HEAD, T_POST, T_PUT, T_DELETE, T_TRACE, T_OPTIONS, T_CONNECT, T_PATCH};
const char* methods[] = {"?","GET","HEAD","POST","PUT","DELETE","TRACE","OPTIONS","CONNECT","PATCH",nullptr};

/**
 * @brief A individual key - value header line 
 * 
 */
struct HttpHeaderLine {
    Str key;
    Str value;
    bool active;
};

/**
 * @brief In a http request and reply we need to process header information. With this API
 * we can define and query the header information. The individual header lines are stored
 * in a list. This is the common functionality for the HttpRequest and HttpReplyHeader
 * subclasses
 * 
 */
class HttpHeader {
    public:
        HttpHeader(){
            HttpLogger.log(Debug,"HttpHeader");
           // set default values
            protocol_str = "HTTP/1.1";
            url_path = "/";
            status_msg = "";
        }
        ~HttpHeader(){
            HttpLogger.log(Debug,"~HttpHeader");
            clear(false);
        }

        /// clears the data - usually we do not delete but we just set the active flag
        HttpHeader& clear(bool activeFlag=true) {
            is_written = false;
            is_chunked = false;
            url_path = "/";
            for (auto it = lines.begin() ; it != lines.end(); ++it){
                if (it!=nullptr){
                    if (activeFlag){
                        (*it)->active = false;
                    } else {
                        delete *it;
                    }
                }
            }
            if (!activeFlag){
                lines.clear();
            }
            return *this;
        }

        HttpHeader& put(const char* key, const char* value){
            if (value!=nullptr){
                HttpHeaderLine *hl = headerLine(key);
                if (hl==nullptr){
                    HttpLogger.log(Error,"HttpHeader::put - did not add HttpHeaderLine for %s", key);
                    return *this;
                }

                // log entry
                HttpLogger.log(Debug,"HttpHeader::put '%s' : %s", key, value);
                hl->value = value;
                hl->active = true;

                if (StrView(key) == TRANSFER_ENCODING &&  StrView(value) == CHUNKED){
                    HttpLogger.log(Info,"HttpHeader::put -> is_chunked!!!");
                    this->is_chunked = true;
                }
            } else {
                HttpLogger.log(Info,"HttpHeader::put - value ignored because it is null for %s", key);
            }
            return *this;
        }

        /// adds a new line to the header - e.g. for content size
        HttpHeader& put(const char* key, int value){
            HttpLogger.log(Debug,"HttpHeader::put %s %d", key, value);
            HttpHeaderLine *hl = headerLine(key);

            if (value>1000){
                HttpLogger.log(Warning,"value is > %d for %s",value, key);
            }

            // add value
            hl->value = value;
            hl->active = true;
            HttpLogger.log(Info,key, hl->value.c_str());
            return *this;
        }

        /// adds a  received new line to the header
        HttpHeader& put(const char* line){
            HttpLogger.log(Debug,"HttpHeader::put -> %s", (const char*) line);
            StrView keyStr(line);
            int pos = keyStr.indexOf(":");
            char *key = (char*)line;
            key[pos] = 0;

            // usually there is a leading space - but unfurtunately not always
            const char *value = line+pos+1;
            if (value[0]==' '){
                value = line+pos+2;
            }
            return put((const char*)key, value);
        }

        // determines a header value with the key
        const char* get(const char* key){
            for (auto it = lines.begin() ; it != lines.end(); ++it){
                HttpHeaderLine *line = *it;
                line->key.trim();
                if (line->key.equalsIgnoreCase(key)){
                    const char* result = line->value.c_str();
                    return line->active ? result : nullptr;
                }
            }
            return nullptr;
        }

        // reads a single header line 
        void readLine(Client &in, char* str, int len){
            reader.readlnInternal(in, (uint8_t*) str, len, false);
            HttpLogger.log(Info,"HttpHeader::readLine -> %s",str);
        }

        // writes a lingle header line
        void writeHeaderLine(Client &out,HttpHeaderLine *header ){
            if (header==nullptr){
                HttpLogger.log(Info,"HttpHeader::writeHeaderLine", "the value must not be null");
                return;
            }
            if (!header->active){
                HttpLogger.log(Info,"HttpHeader::writeHeaderLine %s - not active", header->key.c_str());
                return;
            }
            if (header->value.c_str() == nullptr){
                HttpLogger.log(Info,"HttpHeader::writeHeaderLine - ignored because value is null");
                return;
            }

            char msg[200];
            StrView msg_str(msg,200);
            msg_str = header->key.c_str();
            msg_str += ": ";
            msg_str += header->value.c_str();
            msg_str += CRLF;
            out.print(msg);

            // remove crlf from log
            int len = strnlen(msg,200);
            msg[len-2] = 0;
            HttpLogger.log(Info,"writeHeaderLine -> %s", msg);

            // marke as processed
            header->active = false;
        }
        

        const char* urlPath() {
            return url_path.c_str(); 
        }

        const char* protocol() {
            return protocol_str.c_str(); 
        }

        TinyMethodID method(){
            return method_id;
        }

        const char* accept(){
            return get(ACCEPT);
        }

        int statusCode() {
            return status_code;
        }

        const char* statusMessage() {
            return status_msg.c_str();
        }

        bool isChunked(){
            // the value is automatically set from the reply
            return is_chunked;
        }

        // reads the full header from the request (stream)
        void read(Client &in) {
            HttpLogger.log(Info,"HttpHeader::read");
            // remove all existing value
            clear();

            char line[MaxHeaderLineLength];   
            if (in.connected()){
                if (in.available()==0) {
                    HttpLogger.log(Warning, "Waiting for data...");
                    while(in.available()==0){
                        delay(500);
                    }
                }
                readLine(in, line, MaxHeaderLineLength);
                parse1stLine(line);
                while (in.available()){
                    readLine(in, line, MaxHeaderLineLength);
                    StrView lineStr(line);
                    if (lineStr.isEmpty()||lineStr.isNewLine()){
                        break;
                    }
                    if (isValidStatus() || isRedirectStatus()){
                        lineStr.ltrim();
                        put(line); 
                    }  
                }
            }
        }

        // writes the full header to the indicated HttpStreamedMultiOutput stream
        void write(Client &out){
            HttpLogger.log(Info,"HttpHeader::write");
            write1stLine(out);
            for (auto it = lines.begin() ; it != lines.end(); ++it){
                writeHeaderLine(out, *it);
            }
            // print empty line
            crlf(out);
            out.flush();
            is_written = true;
        }

        // automatically create new lines
        void setAutoCreateLines(bool is_auto_line){
            create_new_lines = is_auto_line;
        }
        /// returns true if status code >=200 and < 300
        bool isValidStatus() {
            return status_code >= 200 && status_code < 300;
        }

        bool isRedirectStatus() {
            return status_code >= 300 && status_code < 400;
        }


    protected:
        int status_code = T_UNDEFINED;
        bool is_written = false;
        bool is_chunked = false;
        bool create_new_lines = true;
        TinyMethodID method_id;
        // we store the values on the heap. this is acceptable because we just have one instance for the
        // requests and one for the replys: which needs about 2*100 bytes 
        Str protocol_str = Str(10);
        Str url_path = Str(70);
        Str status_msg = Str(20);
        List<HttpHeaderLine*> lines;
        HttpLineReader reader;
        const char* CRLF = "\r\n";

        // the headers need to delimited with CR LF
        void crlf(Client &out) {
            out.print(CRLF);
            HttpLogger.log(Info," -> %s", "<CR LF>");

        }

        // gets or creates a header line by key
        HttpHeaderLine *headerLine(const char* key) {
            if (key!=nullptr){
                for (auto it = lines.begin() ; it != lines.end(); ++it){
                    HttpHeaderLine *pt = (*it);
                    if (pt!=nullptr && pt->key.c_str()!=nullptr){
                        if (pt->key.equalsIgnoreCase(key)){
                            pt->active = true;
                            return pt;
                        }
                    }
                }
                if (create_new_lines){
                    HttpHeaderLine *newLine = new HttpHeaderLine();
                    HttpLogger.log(Debug,"HttpHeader::headerLine - new line created for %s", key);
                    newLine->active = true;
                    newLine->key = key;
                    lines.push_back(newLine);
                    return newLine;
                }
            } else {
                HttpLogger.log(Error,"HttpHeader::headerLine", "The key must not be null");
            }
            return nullptr;            
        }

        TinyMethodID getMethod(const char* line){
            // set action
            for (int j=0; methods[j]!=nullptr;j++){
                const char *action = methods[j];
                int len = strlen(action);
                if (strncmp(action,line,len)==0){
                    return (TinyMethodID) j;
                }
            }
            return (TinyMethodID)0;
        }


        virtual void write1stLine(Client &out) = 0;
        virtual void parse1stLine(const char *line) = 0;

     
};

/**
 * @brief Reading and writing of Http Requests
 * 
 */
class HttpRequestHeader : public HttpHeader {
    public:
        // Defines the action id, url path and http version for an request
        HttpHeader& setValues(TinyMethodID id, const char* urlPath, const char* protocol=nullptr){
            this->method_id = id;
            this->url_path = urlPath;
            
            HttpLogger.log(Info,"HttpRequestHeader::setValues - path: %s",this->url_path.c_str());
            if (protocol!=nullptr){
                this->protocol_str = protocol;
            }
            return *this;
        }

        // action path protocol
        void write1stLine(Client &out){
            char msg[201]={0};

            const char* method_str = methods[this->method_id];
            strncat(msg, method_str, 200);
            strncat(msg, " ", 200);
            strncat(msg, this->url_path.c_str(), 200);
            strncat(msg, " ", 200);
            strncat(msg, this->protocol_str.c_str(), 200);
            strncat(msg, CRLF, 200);
            out.print(msg);
            HttpLogger.log(Info,"HttpRequestHeader::write1stLine:  %s", msg);
        }

        // parses the requestline 
        // Request-Line = Method SP Request-URI SP HTTP-Version CRLF
        void parse1stLine(const char *line){
            HttpLogger.log(Info,"HttpRequestHeader::parse1stLine %s", line);
            StrView line_str(line);
            int space1 = line_str.indexOf(" ");
            int space2 = line_str.indexOf(" ", space1+1);

            this->method_id = getMethod(line);
            this->protocol_str.substring(line_str, space2+1, line_str.length());
            this->url_path.substring(line_str,space1+1,space2);
            this->url_path.trim();
  
            HttpLogger.log(Info,"->method: %s", methods[this->method_id]);
            HttpLogger.log(Info,"->protocol: %s", protocol_str.c_str());
            HttpLogger.log(Info,"->url_path: %s", url_path.c_str());
        }

};

/**
 * @brief Reading and Writing of Http Replys
 * 
 */
class HttpReplyHeader : public HttpHeader  {
    public:
        // defines the values for the rely
        void setValues(int statusCode, const char* msg="", const char* protocol=nullptr){
            HttpLogger.log(Info,"HttpReplyHeader::setValues %d", statusCode);
            status_msg = msg;
            status_code = statusCode;
            if (protocol!=nullptr){
                this->protocol_str = protocol;
            }
        }

        // reads the final chunked reply headers 
        void readExt(Client &in) {
            HttpLogger.log(Info,"HttpReplyHeader::readExt");
            char line[MaxHeaderLineLength];   
            readLine(in, line, MaxHeaderLineLength);
            while(strlen(line)!=0){
                put(line);                
                readLine(in, line, MaxHeaderLineLength);
            }
        }

        // HTTP-Version SP Status-Code SP Reason-Phrase CRLF
        void write1stLine(Client &out){
            char msg[200];
            StrView msg_str(msg,200);
            msg_str = this->protocol_str.c_str();
            msg_str += " ";
            msg_str += this->status_code;
            msg_str += " ";
            msg_str += this->status_msg.c_str();
            HttpLogger.log(Info,"HttpReplyHeader::write1stLine: %s", msg);
            out.print(msg);
            crlf(out);
        }


        // HTTP-Version SP Status-Code SP Reason-Phrase CRLF
        // we just update the pointers to point to the correct position in the
        // http_status_line
        void parse1stLine(const char *line){
            HttpLogger.log(Info,"HttpReplyHeader::parse1stLine %s",line);
            StrView line_str(line);
            int space1 = line_str.indexOf(' ',0);
            int space2 = line_str.indexOf(' ',space1+1);

            // save http version 
            protocol_str.substring(line_str,0,space1);

            // find response status code after the first space
            char status_c[6];
            StrView status(status_c,6);
            status.substring(line_str, space1+1, space2);
            status_code = atoi(status_c);

            // get reason-phrase after last SP
            status_msg.substring(line_str, space2+1, line_str.length());
        }

};

}
