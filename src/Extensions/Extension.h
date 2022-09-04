#pragma once

namespace tinyhttp {

// forward declaration
class HttpServer;

/**
 * @brief The HttpServer supports the registraiton of Extensions to 
 * implement some additional functionalry which is not supported in
 * the basic impelementation. E.g SD support.
 * 
 */
class Extension {
    friend class HttpServer;

    public:
        Extension(){}
        virtual void open(HttpServer *server) {};
    
    protected:
        virtual void doLoop() {};

};

}

