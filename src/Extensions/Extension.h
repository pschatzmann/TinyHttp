#ifndef __EXTENSION_H__
#define __EXTENSION_H__

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
    public:
        Extension(){}
        virtual void open(HttpServer *server) {};
        virtual void doLoop() {};

};

}

#endif // __EXTENSION_H__